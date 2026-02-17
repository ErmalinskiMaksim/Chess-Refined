#include "ChessServer.h"
#include "Serializer.h"
// #include <print>

ChessServer::ChessServer(std::string_view path)
    : m_sessions{}
    , m_io{}
    , m_acceptor{m_io, Endpoint{path}}
    , m_connections{}
    , m_waitingPlayer{}
{}

ServerView ChessServer::getView() noexcept {
    return std::ref(*this);
}

void ChessServer::doAccept() {
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, Socket socket) {
            if (!ec) {
                auto conn = std::make_shared<Connection>(
                    std::move(socket),
                    getView() 
                );

                addConnection(conn);
                conn->start();
                onConnect(conn);
            }
            // continue accepting
            doAccept(); 
        });
}

void ChessServer::run() {
    // std::println("[SVR]: Chess server started");
    m_io.run();
    // std::println("[SVR]: Chess server finished execution"); 
}

void ChessServer::onConnect(const ConnectionPtr& conn) {
    if (m_waitingPlayer == nullptr) {
        m_waitingPlayer = conn;
        notifyColor(m_waitingPlayer, Piece::Color::WHITE);
    }
    else {
        auto opponent = m_waitingPlayer;
        m_waitingPlayer = nullptr;
        notifyColor(conn, Piece::Color::BLACK);

        createSession(opponent, conn);
    }
}

void ChessServer::onDisconnect(const ConnectionPtr& conn) {
    removeConnection(conn);

    if (m_waitingPlayer == conn) {
        m_waitingPlayer.reset();
        return;
    }
    removeFromSession(conn);
}

void ChessServer::onPacket(const ConnectionPtr& conn, Packet req) {
    switch(req.type) {
        case ReqType::CALCULATE_MOVES: 
            // std::println("[SVR]: performing move calculation");
            performAvailableMoves(conn
                , Pos{static_cast<int8_t>(req.payload[0]), static_cast<int8_t>(req.payload[1])}); 
            break;
        case ReqType::COMMIT_MOVE:
            // std::println("[SVR]: performing move commit");
            performCommitMove(conn
                    , Pos{static_cast<int8_t>(req.payload[0]), static_cast<int8_t>(req.payload[1])}
                    , Pos{static_cast<int8_t>(req.payload[2]), static_cast<int8_t>(req.payload[3])});
            break;
        case ReqType::PROMOTE:
            // std::println("[SVR]: performing promotion; len: {}", req.payload.size());
            performPromotion(conn, static_cast<Piece::Type>(req.payload[0])
                    , Pos{static_cast<int8_t>(req.payload[1]), static_cast<int8_t>(req.payload[2])});
            break;
        case ReqType::RESTART:
            // std::println("[SVR]: performing restart");
            performRestart(conn);
            break;
        case ReqType::GET_CACHE:
            // std::println("[SVR]: performing cache transfer");
            performGetCache(conn);
            break;
        case ReqType::SHUT_DOWN:
            // std::println("[SVR]: performing shut down");
            performShutdown(conn);
            break;
        default: break;
    }
}

void ChessServer::removeFromSession(const ConnectionPtr& conn) {
    // std::println("[SVR]: Removing a player from a session");
    if (auto idx = findSession(conn)) {
        auto opponent = (m_sessions[*idx].white == conn)
            ? m_sessions[*idx].black : m_sessions[*idx].white;

        if (opponent) {
            notifyDisconnect(opponent);
            m_waitingPlayer = opponent;
        }

        m_sessions.erase(m_sessions.begin() + static_cast<ssize_t>(*idx));
    }
}

void ChessServer::createSession(const ConnectionPtr& white, const ConnectionPtr& black) {
    // std::println("[SVR]: Creating a session");
    m_sessions.push_back({{}, white, black});
    const auto& session = m_sessions.back();
    white->send(Serializer::serialize(
        ChessClientCache{session.state.board.get()
        , ::GameState{session.state.score
        , GameState::Flags::NONE
        , Piece::Color::WHITE}}
        , ReqType::GET_CACHE
    ));
    black->send(Serializer::serialize(
        ChessClientCache{session.state.board.get()
        , ::GameState{session.state.score
        , GameState::Flags::NONE
        , Piece::Color::BLACK}}
        , ReqType::GET_CACHE
    ));
}

void ChessServer::addConnection(const ConnectionPtr& conn) {
    // std::println("[SVR]: performing adding connection...");
    m_connections.insert(conn);
}

void ChessServer::removeConnection(const ConnectionPtr& conn) {
    // std::println("[SVR]: performing removing connection...");
    m_connections.erase(conn);
}

void ChessServer::performShutdown(const ConnectionPtr&) {
    for (auto& conn : m_connections)
        conn->close();

    m_connections.clear();
}

void ChessServer::performAvailableMoves(const ConnectionPtr& conn, Pos pos) {
    if (auto idx = findSession(conn)) {
        auto& state = m_sessions[*idx].state;
        conn->send(Serializer::serialize(
            state.board.calculatePieceMoves(pos, state.currentTeam)
            , ReqType::CALCULATE_MOVES));
    }
}

void ChessServer::performPromotion(const ConnectionPtr& conn, Piece::Type type, Pos pos) {
    if (auto idx = findSession(conn)) {
        auto& session = m_sessions[*idx];
        session.state.board.promote(pos, type);
        auto opponent = (session.white == conn) ? session.black : session.white;
        auto packet = Serializer::serialize(static_cast<uint8_t>(type), pos, ReqType::PROMOTE);
        conn->send(packet);
        opponent->send(std::move(packet));
    }
}

void ChessServer::performRestart(const ConnectionPtr& conn) {
    if (auto idx = findSession(conn)) {
        auto& session = m_sessions[*idx];
        auto opponent = (session.white == conn) ? session.black : session.white;
        session.state = GameSession::State{};

        auto packet = Serializer::serialize(true, ReqType::RESTART);
        conn->send(packet);
        opponent->send(std::move(packet));
    }
}

void ChessServer::performCommitMove(const ConnectionPtr& conn, Pos moveFrom, Pos moveTo) {
    if (auto idx = findSession(conn)) {
        auto& session = m_sessions[*idx];
        auto opponent = (session.white == conn) ? session.black : session.white;
        auto& state = session.state;
        uint8_t resp = state.board.commitMove(moveFrom, moveTo, state.currentTeam);
        GameState::Flags flag = static_cast<GameState::Flags>(resp & 0xF0);
        if (state.currentTeam == Piece::Color::WHITE) {
            state.score.white += (resp & 0x0F);
            state.currentTeam = Piece::Color::BLACK;
        } else { 
            state.score.black += (resp & 0x0F);
            state.currentTeam = Piece::Color::WHITE;
        }

        auto packet = Serializer::serialize(
            ChessClientCache{state.board.get(), ::GameState{state.score, flag, state.currentTeam}}
            , ReqType::COMMIT_MOVE);
        conn->send(packet);
        opponent->send(std::move(packet));
    }
}

void ChessServer::performGetCache(const ConnectionPtr& conn) {
    if (auto idx = findSession(conn)) {
        auto& state = m_sessions[*idx].state;
        conn->send(Serializer::serialize(
            ChessClientCache{state.board.get(), ::GameState{state.score, GameState::Flags::NONE, state.currentTeam}}
            , ReqType::GET_CACHE));
    }
}

void ChessServer::notifyColor(const ConnectionPtr& conn, Piece::Color col) const {
    conn->send(Serializer::serialize(static_cast<uint8_t>(col), ReqType::ASSIGN_COLOR));
} 

void ChessServer::notifyDisconnect(const ConnectionPtr& conn) const {
    conn->send(Serializer::serialize(ReqType::PLAYER_DISCONNECT));
}

std::optional<size_t> ChessServer::findSession(const ConnectionPtr& conn) const noexcept {
    for (auto i = 0uz; i < m_sessions.size(); ++i) 
        if (m_sessions[i].white == conn || m_sessions[i].black == conn)
            return i;
    return std::nullopt;
}
