#ifndef CHESS_CLIENT
#define CHESS_CLIENT

#include "Connection.h"
#include "BoardTypes.h"
#include "BoardWidget.h"
#include "BoardHandlers.h"

struct RestartMsg{ bool confirmed; };
struct PromotionMsg{};
using NetworkMessage = std::variant<Moves, GameState::Flags, RestartMsg, PromotionMsg>;
template<typename T>
concept NetworkMessageType = std::is_same_v<T, Moves>
                            || std::is_same_v<T, GameState::Flags>
                            || std::is_same_v<T, RestartMsg>
                            || std::is_same_v<T, PromotionMsg>;

template<WidgetType, typename, typename...>
class BoardInteractor;
using DefaultBoardInteractor = BoardInteractor<BoardWidget
                             , HandlerContext
                             , BoardPromotionHandler<HandlerContext>
                             , GameOverHandler<HandlerContext>
>;
using DefaultBoardInteractorView = std::reference_wrapper<DefaultBoardInteractor>;

struct Packet;
class ChessClient {
public:
    ChessClient(const ChessClient&) = delete;
    ChessClient(ChessClient&&) = delete;
    ChessClient& operator=(const ChessClient&) = delete;
    ChessClient& operator=(ChessClient&&) = delete;
    ~ChessClient() = default;

    static ChessClient& get() {
        static ChessClient client;
        return client;
    }
    inline static Pos indexToPos(size_t idx) {
        return Pos(idx % 8, static_cast<int8_t>(idx) / 8);
    }

    // connection
    void connect(std::string_view);
    void init();
    void poll();
    void onDisconnect();
    void onPacket(Packet);

    // interactor registration 
    void registerInteractor(DefaultBoardInteractor*);
    void unregisterInteractor();

    // request server data
    void requestRestart();
    void requestShutDown();
    void requestAvailableMoves(Pos);
    void requestPromotion(Piece::Type);
    void requestCommitMove(Pos);

    // get local cache
    const BoardType& getBoardCache() const noexcept;
    Pos getSelectedPieceCache() const noexcept;
    GameState getCache() const noexcept;
    bool canMove(Pos) const noexcept;
    
private:
    inline static size_t posToIndex(Pos pos) {
        return static_cast<size_t>(pos.x + pos.y*8);
    }

    void requestCache();

    ChessClient(); 

    struct GameCache {
        BoardType board;
        GameState state;
        // this part is not updated on restart
        Pos selectedPiece;
        Piece::Color teamColor;
    } m_cache;

    Context m_io;
    std::unique_ptr<Executor> m_workGuard;
    ConnectionPtr m_connection;

    DefaultBoardInteractor* p_interactor;
    bool m_initialized = false;
};

#endif // !CHESS_CLIENT
