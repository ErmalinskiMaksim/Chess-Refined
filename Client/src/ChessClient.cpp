#include "ChessClient.h"
#include "BoardInteractor.h"
#include "Serializer.h"
#include <cstring>
// #include <print>

ChessClient::ChessClient() 
    : m_cache{}
{}

void ChessClient::connect(/*std::string_view path*/) {
    // std::println("[CLI]: connecting...");
    if (!m_initialized)
        init();

    m_connection = std::make_shared<Connection>(m_io, *this);
    m_connection->start(/*path*/);
}

void ChessClient::poll() {
    m_io.poll();
}

void ChessClient::init() {
    if (m_initialized)
        return;

    // std::println("[CLI]: initializing...");
    m_workGuard = std::make_unique<Executor>(m_io.get_executor());

    m_initialized = true;
}

void ChessClient::onDisconnect() {
    // std::println("[CLI]: disconnection...");
    m_workGuard.reset();
    m_io.stop();
    m_connection.reset();
}

void ChessClient::onPacket(Packet resp) {
    switch (resp.type) {
        case ReqType::CALCULATE_MOVES: {
            Moves moves{resp.payload.size() / sizeof(Pos)};
            // std::println("[CLI]: received moves: {}", moves.size());
            std::memcpy(moves.data(), resp.payload.data(), resp.payload.size());
            p_interactor->onNetworkMessage(std::move(moves));
            break;
        }
        case ReqType::GET_CACHE: 
            // std::println("[CLI]: cache size: {}", resp.payload.size());
            [[fallthrough]];
        case ReqType::COMMIT_MOVE: {
            // std::println("[CLI]: committing a move");
            std::memcpy(&m_cache, resp.payload.data(), resp.payload.size());
            p_interactor->onNetworkMessage(m_cache.state.flag);
            break;
        }
        case ReqType::RESTART: {
            // std::println("[CLI]: received restart response");
            bool isConfirmed = resp.payload[0];
            if (isConfirmed)  {
                // std::println("[CLI]: committing restart");
                requestCache();
            }
            p_interactor->onNetworkMessage(RestartMsg{isConfirmed});
            break;
        }
        case ReqType::PROMOTE: {
            // std::println("[CLI]: received promotion response");
            auto piece = static_cast<Piece::Type>(resp.payload[0]);
            auto pos = Pos{static_cast<int8_t>(resp.payload[1]), static_cast<int8_t>(resp.payload[2])};
            m_cache.board[posToIndex(pos)].type = piece;
            p_interactor->onNetworkMessage(PromotionMsg{});
            break;
        }
        case ReqType::ASSIGN_COLOR: {
            // std::println("[CLI]: received team color: {}", resp.payload[0]);
            m_cache.teamColor = static_cast<Piece::Color>(resp.payload[0]);
            break;
        }
        default: requestShutDown();
    }
}

void ChessClient::registerInteractor(DefaultBoardInteractor* inter) {
    p_interactor = inter;
}

void ChessClient::unregisterInteractor() {
    p_interactor = nullptr;
}

void ChessClient::requestRestart() {
    // std::println("[CLI]: requesting restart...");
    m_connection->send(Serializer::serialize(ReqType::RESTART));
}

void ChessClient::requestShutDown() {
    // std::println("[CLI]: requesting shutdown...");
    m_connection->send(Serializer::serialize(ReqType::SHUT_DOWN));
    onDisconnect();
}

void ChessClient::requestAvailableMoves(Pos pos) {
    // std::println("[CLI]: requesting moves...");
    m_cache.selectedPiece = pos;
    m_connection->send(Serializer::serialize(pos, ReqType::CALCULATE_MOVES));
}

void ChessClient::requestPromotion(Piece::Type type) {
    // std::println("[CLI]: requesting promotion...");
    m_connection->send(Serializer::serialize(static_cast<uint8_t>(type), m_cache.selectedPiece, ReqType::PROMOTE));
}

void ChessClient::requestCommitMove(Pos moveTo) {
    // std::println("[CLI]: requesting move commit...");
    m_connection->send(Serializer::serialize(m_cache.selectedPiece, moveTo, ReqType::COMMIT_MOVE));
    m_cache.selectedPiece = moveTo;
}

void ChessClient::requestCache() {
    // std::println("[CLI]: requesting cache...");
    m_connection->send(Serializer::serialize(ReqType::GET_CACHE));
}

const BoardType& ChessClient::getBoardCache() const noexcept {
    return m_cache.board;
}

Pos ChessClient::getSelectedPieceCache() const noexcept {
    return m_cache.selectedPiece;
}

GameState ChessClient::getCache() const noexcept {
    return m_cache.state;
}

bool ChessClient::canMove(Pos moveFrom) const noexcept {
    return m_cache.state.currentTeam == m_cache.teamColor
        && m_cache.board[posToIndex(moveFrom)].col == m_cache.teamColor;
}
