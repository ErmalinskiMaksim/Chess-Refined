#include "ChessClient.h"
#include "Serializer.h"
#include <cstring>
// #include <print>

// Responsibilities:
// * Perform a graceful shutdown of all the networking side
ChessClient::~ChessClient() {
    // stop the incoming message queue
    g_MQ.gui2net.stop();

    // close the connection
    if (m_connection) 
        m_connection->close();

    // close the io
    m_workGuard.reset();
    m_io.stop();

    // join the threads
    m_ioThread.join();
    m_mainThread.join();

    // destroy the connection
    m_connection.reset();
}

// Responsibilities:
// * Initialize the network io
void ChessClient::init() {
    if (m_initialized) return;

    // std::println("[CLI]: initializing...");
    m_workGuard = std::make_unique<Executor>(m_io.get_executor());

    m_initialized = true;
}

// Responsibilities:
// * Connect to the server
// * Lauch the network io thread 
// * Lauch the main thread
void ChessClient::connect(std::string_view ipAddr) {
    // std::println("[CLI]: connecting...");
    if (!m_initialized) init();

    // create a connection
    m_connection = std::make_shared<Connection>(m_io, *this);
    // start the connction
    m_connection->start(ipAddr);
    // start the network io thread
    m_ioThread = std::thread([this]{ m_io.run(); });
    // start main networking thread
    m_mainThread = std::thread([this] {
        while (true) {
            // wait for messages (blocking) in the gui->net queue
            auto msg = g_MQ.gui2net.waitPop();
            if (!msg) break; // when the queue is stopped, finish`

            // dispatch a received request
            std::visit([&](auto&& req) { send(std::move(req)); }, std::move(*msg));
        }
    });
}

// Responsibilities:
// * Update internal state with the data from an incoming package
// * Optionally put data to the net->gui queue
void ChessClient::onPacket(Packet resp) {
    switch (resp.type) {
        case ReqType::CALCULATE_MOVES: {
            Moves moves{resp.payload.size() / sizeof(Pos)};
            // std::println("[CLI]: received moves: {}", moves.size());
            std::memcpy(moves.data(), resp.payload.data(), resp.payload.size());
            g_MQ.net2gui.push(std::move(moves));
            break;
        }
        case ReqType::COMMIT_MOVE: {
            // std::println("[CLI]: committing a move");
            std::memcpy(&m_cache.state, resp.payload.data(), resp.payload.size());
            g_MQ.net2gui.push(StateUpdateMsg{m_cache.state, m_cache.teamColor});
            break;
        }
        case ReqType::RESTART: {
            // std::println("[CLI]: committing restart");
            std::memcpy(&m_cache.state, resp.payload.data(), resp.payload.size());
            g_MQ.net2gui.push(RestartMsg{m_cache.state, m_cache.teamColor});
            break;
        }
        case ReqType::PROMOTE: {
            // std::println("[CLI]: received promotion response");
            auto piece = static_cast<Piece::Type>(resp.payload[0]);
            auto pos = Pos{static_cast<int8_t>(resp.payload[1]), static_cast<int8_t>(resp.payload[2])};
            m_cache.state.board[posToIndex(pos)].type = piece;
            g_MQ.net2gui.push(StateUpdateMsg{m_cache.state, m_cache.teamColor});
            break;
        }
        case ReqType::ASSIGN_COLOR: {
            // std::println("[CLI]: received team color: {}", resp.payload[0]);
            m_cache.teamColor = static_cast<Piece::Color>(resp.payload[0]);
            break;
        }
        default: {
            // std::println("[CLI]: received a shutdown request");
            g_MQ.net2gui.push(ShutdownMsg{});
            break;
        }
    }
}

// Responsibilities:
// * Passes a serialized move request to the connection
void ChessClient::send(AvailableMovesReq req) {
    // std::println("[CLI]: requesting moves...");
    m_cache.selectedPiece = req.moveFrom; // store the position for move commit
    m_connection->send(Serializer::serialize(req.moveFrom, ReqType::CALCULATE_MOVES));
}

// Responsibilities:
// * Passes a serialized move commit request to the connection
void ChessClient::send(MoveCommitReq req) {
    // std::println("[CLI]: requesting move commit...");
    m_connection->send(Serializer::serialize(m_cache.selectedPiece, req.moveTo, ReqType::COMMIT_MOVE));
    m_cache.selectedPiece = req.moveTo; // store the new position for promotion handling
}

// Responsibilities:
// * Passes a serialized restart request to the connection
void ChessClient::send(RestartReq) {
    // std::println("[CLI]: requesting restart...");
    m_connection->send(Serializer::serialize(ReqType::RESTART));
}

// Responsibilities:
// * Passes a serialized promotion request to the connection
void ChessClient::send(PromotionReq req) {
    // std::println("[CLI]: requesting promotion...");
    m_connection->send(Serializer::serialize(static_cast<uint8_t>(req.type), m_cache.selectedPiece, ReqType::PROMOTE));
}
