#include "ChessServer.h"
#include "Common.h"
#include "Connection.h"
#include "PacketSerializer.h"
#include <print>

ChessServer::ChessServer(std::string_view path)
    : m_sessions{}
    , m_io{}
    , m_acceptor{m_io, EndpointType{path}}
    , m_connections{}
    , m_waitingPlayer{}
{}

ServerView ChessServer::getView() noexcept {
    return std::ref(*this);
}

void ChessServer::doAccept() {
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, SocketType socket) {
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
    std::println("[SVR]: Chess server started");
    m_io.run();
    std::println("[SVR]: Chess server finished execution"); 
}

void ChessServer::onConnect(ConnectionPtr conn) {
    if (m_waitingPlayer == nullptr) {
        m_waitingPlayer = conn;
        notifyColor(m_waitingPlayer, PieceColor::WHITE);
    }
    else {
        auto opponent = m_waitingPlayer;
        m_waitingPlayer = nullptr;
        notifyColor(conn, PieceColor::BLACK);

        createSession(opponent, conn);
    }
}

void ChessServer::onDisconnect(ConnectionPtr conn) {
    removeConnection(conn);

    if (m_waitingPlayer == conn) {
        m_waitingPlayer.reset();
        return;
    }
    removeFromSession(conn);
}

void ChessServer::dispatchRequests(ConnectionPtr conn, Request req) {
    using RT = Request::ReqType;

    switch(req.type) {
        case RT::CALCULATE_MOVES: 
            std::println("[SVR]: performing move calculation");
            performAvailableMoves(conn, req.moveFrom); 
            break;
        case RT::COMMIT_MOVE:
            std::println("[SVR]: performing move commit");
            performCommitMove(conn, req.moveFrom, req.moveTo);
            break;
        case RT::PROMOTE:
            std::println("[SVR]: performing promotion");
            performPromotion(conn, req.moveFrom, req.promotion);
            break;
        case RT::RESTART:
            std::println("[SVR]: performing restart");
            performRestart(conn);
            break;
        case RT::GET_CACHE:
            std::println("[SVR]: performing cache transfer");
            performGetCache(conn);
            break;
        case RT::SHUT_DOWN:
            std::println("[SVR]: performing shut down");
            performShutdown(conn);
            break;
        default: break;
    }
}

void ChessServer::removeFromSession(ConnectionPtr conn) {
    std::println("[SVR]: Removing a player from a session");
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

void ChessServer::createSession(ConnectionPtr white, ConnectionPtr black) {
    std::println("[SVR]: Creating a session");
    m_sessions.push_back({{}, white, black});
    const auto& session = m_sessions.back();
    white->send(PacketSerializer::serialize(
        ChessClientCache{session.state.board.get()
        , ::GameState{session.state.score
        , GameFlags::NONE
        , PieceColor::WHITE}}
        , Request::ReqType::GET_CACHE
    ));
    black->send(PacketSerializer::serialize(
        ChessClientCache{session.state.board.get()
        , ::GameState{session.state.score
        , GameFlags::NONE
        , PieceColor::BLACK}}
        , Request::ReqType::GET_CACHE
    ));
}

void ChessServer::addConnection(ConnectionPtr conn) {
    std::println("[SVR]: performing adding connection...");
    m_connections.insert(conn);
}

void ChessServer::removeConnection(ConnectionPtr conn) {
    std::println("[SVR]: performing removing connection...");
    m_connections.erase(conn);
}

void ChessServer::performShutdown(ConnectionPtr) {
    for (auto& conn : m_connections)
        conn->disconnect();

    m_connections.clear();
}

void ChessServer::performAvailableMoves(ConnectionPtr conn, Pos pos) {
    if (auto idx = findSession(conn)) {
        auto& state = m_sessions[*idx].state;
        conn->send(PacketSerializer::serialize(
            state.board.calculatePieceMoves(pos, state.currentTeam)
            , Request::ReqType::CALCULATE_MOVES));
    }
}

void ChessServer::performPromotion(ConnectionPtr conn, Pos pos, PieceType type) {
    if (auto idx = findSession(conn)) {
        auto& session = m_sessions[*idx];
        session.state.board.promote(pos, type);
        auto opponent = (session.white == conn) ? session.black : session.white;
        auto packet = PacketSerializer::serialize(static_cast<uint8_t>(type), pos, Request::ReqType::PROMOTE);
        conn->send(packet);
        opponent->send(std::move(packet));
    }
}

void ChessServer::performRestart(ConnectionPtr conn) {
    if (auto idx = findSession(conn)) {
        auto& session = m_sessions[*idx];
        auto opponent = (session.white == conn) ? session.black : session.white;
        session.state = GameSession::GameState{};

        auto packet = PacketSerializer::serialize(true, Request::ReqType::RESTART);
        conn->send(packet);
        opponent->send(std::move(packet));
    }
}

void ChessServer::performCommitMove(ConnectionPtr conn, Pos moveFrom, Pos moveTo) {
    if (auto idx = findSession(conn)) {
        auto& session = m_sessions[*idx];
        auto opponent = (session.white == conn) ? session.black : session.white;
        auto& state = session.state;
        uint8_t resp = state.board.commitMove(moveFrom, moveTo, state.currentTeam);
        GameFlags flag = static_cast<GameFlags>(resp & 0xF0);
        if (state.currentTeam == PieceColor::WHITE) {
            state.score.white += (resp & 0x0F);
            state.currentTeam = PieceColor::BLACK;
        } else { 
            state.score.black += (resp & 0x0F);
            state.currentTeam = PieceColor::WHITE;
        }

        auto packet = PacketSerializer::serialize(
            ChessClientCache{state.board.get(), ::GameState{state.score, flag, state.currentTeam}}
            , Request::ReqType::COMMIT_MOVE);
        conn->send(packet);
        opponent->send(std::move(packet));
    }
}

void ChessServer::performGetCache(ConnectionPtr conn) {
    if (auto idx = findSession(conn)) {
        auto& state = m_sessions[*idx].state;
        conn->send(PacketSerializer::serialize(
            ChessClientCache{state.board.get(), ::GameState{state.score, GameFlags::NONE, state.currentTeam}}
            , Request::ReqType::GET_CACHE));
    }
}

void ChessServer::notifyColor(ConnectionPtr conn, PieceColor col) {
    conn->send(PacketSerializer::serialize(static_cast<uint8_t>(col), Request::ReqType::ASSIGN_COLOR));
} 

void ChessServer::notifyDisconnect(ConnectionPtr conn) {
    conn->send(PacketSerializer::serialize(Request::ReqType::PLAYER_DISCONNECT));
}

std::optional<size_t> ChessServer::findSession(ConnectionPtr conn) {
    for (auto i = 0uz; i < m_sessions.size(); ++i) 
        if (m_sessions[i].white == conn || m_sessions[i].black == conn)
            return i;
    return std::nullopt;
}
