#include "ChessServer.h"
#include "Serializer.h"
// #include <print>

// Responsibilities:
// * Initialize the network io context
// * Create and bind an acceptor socket
ChessServer::ChessServer()
    : m_sessions{}
    , m_io{}
    , m_acceptor{m_io, Endpoint{boost::asio::ip::tcp::v4(), 5000}}
    , m_connections{}
    , m_waitingPlayer{}
{}

// Responsibilities:
// * Return a view of itself
ServerView ChessServer::getView() noexcept {
    return std::ref(*this);
}

// Responsibilities:
// * Accept new connections
// * Initialize and save new connections 
void ChessServer::doAccept() {
    m_acceptor.async_accept([this](auto ec, Socket socket) {
        if (!ec) {
            auto conn = std::make_shared<Connection>(std::move(socket), getView());

            addConnection(conn);
            conn->start();
            onConnect(conn);
        }
        // continue accepting
        doAccept(); 
    });
}

// Responsibilities:
// * Run the main loop
void ChessServer::run() {
    // std::println("[SVR]: Chess server started");
    m_io.run();
    // std::println("[SVR]: Chess server finished execution"); 
}

// Responsibilities:
// * Notify a newly joined client about their color 
// * If there already is another player waiting, then create a 
//   session for them.
void ChessServer::onConnect(const ConnectionPtr& conn) {
    if (m_waitingPlayer == nullptr) { // no players waiting
        m_waitingPlayer = conn;
        notifyColor(m_waitingPlayer, Piece::Color::WHITE);
    }
    else { // there is another player waiting
        auto opponent = m_waitingPlayer;
        m_waitingPlayer = nullptr;
        notifyColor(conn, Piece::Color::BLACK);

        // create a new game session
        createSession(opponent, conn);
    }
}

// Responsibilities:
// * Remove an expired connection from the active connection list 
// * Remove an expired Connection from an active session
void ChessServer::onDisconnect(const ConnectionPtr& conn) {
    removeConnection(conn);

    if (m_waitingPlayer == conn) {
        m_waitingPlayer.reset();
        return;
    }
    // if a player is in a game session, then remove the session
    removeSession(conn);
}

// Responsibilities:
// * Accept a data packet from a client connection 
// * Determine the session, which session the connection belongs to 
// * Dispatch the packet into an appropriate handler based on the request type
void ChessServer::onPacket(const ConnectionPtr& conn, Packet req) {
    // get a session, where the connection belongs
    auto session = [&] {
        for (auto& s : m_sessions) 
            if (s.white == conn || s.black == conn)
                return std::optional<std::reference_wrapper<GameSession>>{std::ref(s)};
        return std::optional<std::reference_wrapper<GameSession>>{std::nullopt};
    }();

    if (!session) return; // if the connection is not in a session, ignore

    switch(req.type) {
        case ReqType::CALCULATE_MOVES: 
            // std::println("[SVR]: performing move calculation");
            performAvailableMoves(session->get(), conn
                , Pos{static_cast<int8_t>(req.payload[0]), static_cast<int8_t>(req.payload[1])}); 
            break;
        case ReqType::COMMIT_MOVE:
            // std::println("[SVR]: performing move commit");
            performCommitMove(session->get()
                    , Pos{static_cast<int8_t>(req.payload[0]), static_cast<int8_t>(req.payload[1])}
                    , Pos{static_cast<int8_t>(req.payload[2]), static_cast<int8_t>(req.payload[3])});
            break;
        case ReqType::PROMOTE:
            // std::println("[SVR]: performing promotion; len: {}", req.payload.size());
            performPromotion(session->get(), static_cast<Piece::Type>(req.payload[0])
                    , Pos{static_cast<int8_t>(req.payload[1]), static_cast<int8_t>(req.payload[2])});
            break;
        case ReqType::RESTART:
            // std::println("[SVR]: performing restart");
            performRestart(session->get());
            break;
        case ReqType::SHUT_DOWN:
            // std::println("[SVR]: performing shut down");
            performShutdown();
            break;
        default: break;
    }
}

// Responsibilities:
// * Find the session, where a connection belongs 
// * Find the connection of player's opponent 
// * Notify the opponent about disconnection 
// * Remove the session
void ChessServer::removeSession(const ConnectionPtr& conn) {
    // std::println("[SVR]: Removing a player from a session");
    if (auto session = std::find_if(m_sessions.begin(), m_sessions.end()
        , [&](const auto& s){ return (s.white == conn || s.black == conn); }); session != m_sessions.end()) {

        if (const auto& opponent = (session->white == conn) ? session->black : session->white) {
            notifyDisconnect(opponent);
            m_waitingPlayer = opponent;
        }
        m_sessions.erase(session);
    }
}

// Responsibilities:
// * Create a session from two connections 
// * Add a session to the active session list 
// * Perform a restart operation
void ChessServer::createSession(const ConnectionPtr& white, const ConnectionPtr& black) {
    // std::println("[SVR]: Creating a session");
    m_sessions.push_back({{}, white, black});
    performRestart(m_sessions.back());
}

// Responsibilities:
// * Insert a connection to the active connection list
void ChessServer::addConnection(const ConnectionPtr& conn) {
    // std::println("[SVR]: performing adding connection...");
    m_connections.insert(conn);
}

// Responsibilities:
// * Remove a connection from the active connection list
void ChessServer::removeConnection(const ConnectionPtr& conn) {
    // std::println("[SVR]: performing removing connection...");
    m_connections.erase(conn);
}

// Responsibilities:
// * Close all connections 
// * Destroy all connections
void ChessServer::performShutdown() {
    for (auto& conn : m_connections)
        conn->close();

    m_connections.clear();
}

// Responsibilities:
// * Calculate the moves 
// * Pass a serialized packet to the connection
void ChessServer::performAvailableMoves(GameSession& session, const ConnectionPtr& conn, Pos pos) {
    auto& state = session.state;
    conn->send(Serializer::serialize(state.board.calculatePieceMoves(pos, state.currentTeam)
        , ReqType::CALCULATE_MOVES));
}

// Responsibilities:
// * Perform promotion 
// * Send a serialized packet to both players' connections 
void ChessServer::performPromotion(GameSession& session, Piece::Type type, Pos pos) {
    session.state.board.promote(pos, type);
    auto packet = Serializer::serialize(static_cast<uint8_t>(type), pos, ReqType::PROMOTE);
    session.black->send(packet);
    session.white->send(std::move(packet));
}

// Responsibilities:
// * Perform restart 
// * Send a serialized packet to both players' connections 
void ChessServer::performRestart(GameSession& session) {
    session.state = GameSession::State{};
    auto packet = Serializer::serialize(GameStateCache{session.state.board.get()
                                    , ::GameState{session.state.score, GameState::Flags::NONE, Piece::Color::WHITE}}
                                    , ReqType::RESTART);
    session.black->send(packet);
    session.white->send(std::move(packet));
}

// Responsibilities:
// * Perform a move commit 
// * Send a serialized packet to both players' connections 
void ChessServer::performCommitMove(GameSession& session, Pos moveFrom, Pos moveTo) {
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

    auto packet = Serializer::serialize(GameStateCache{state.board.get()
                                    , ::GameState{state.score, flag, state.currentTeam}}
                                    , ReqType::COMMIT_MOVE);
    session.black->send(packet);
    session.white->send(std::move(packet));
}

// Responsibilities:
// * Pass a serialized color message to the connection
void ChessServer::notifyColor(const ConnectionPtr& conn, Piece::Color col) const {
    conn->send(Serializer::serialize(static_cast<uint8_t>(col), ReqType::ASSIGN_COLOR));
} 

// Responsibilities:
// * Pass a serialized disconnect message to the connection
void ChessServer::notifyDisconnect(const ConnectionPtr& conn) const {
    conn->send(Serializer::serialize(ReqType::PLAYER_DISCONNECT));
}
