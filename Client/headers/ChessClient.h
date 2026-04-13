#ifndef CHESS_CLIENT
#define CHESS_CLIENT

#include "Connection.h"
#include "MessageQueues.h"

struct Packet;

// The main network and client-side game logic manager
class ChessClient {
public:
    ChessClient() = default; 
    ~ChessClient(); 

    inline static Pos indexToPos(size_t idx) noexcept {
        return Pos(idx % 8, static_cast<int8_t>(idx) / 8);
    }
    inline static size_t posToIndex(Pos pos) noexcept {
        return static_cast<size_t>(pos.x + pos.y*8);
    }

    // starts the network client
    void init();
    // connects the network client to the server
    void connect(std::string_view);
    // a callback on packet arival
    void onPacket(Packet);
private:
    // send client requests
    void send(AvailableMovesReq);
    void send(MoveCommitReq);
    void send(RestartReq);
    void send(PromotionReq);

    // local game cache
    struct GameCache {
        GameStateCache state;
        Pos selectedPiece;
        Piece::Color teamColor;
    } m_cache;

    // network io context
    Context m_io;
    // main gui <-> network interactor thread
    std::thread m_mainThread;
    // network io thread
    std::thread m_ioThread;
    std::unique_ptr<Executor> m_workGuard;
    // a shared connection to the server
    ConnectionPtr m_connection;
    bool m_initialized = false;
};

#endif // !CHESS_CLIENT
