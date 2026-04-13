#ifndef BOARD_INTERACTOR_H
#define BOARD_INTERACTOR_H

#include "RTWgui/Interactors/Interactor.h"
#include "ChessClient.h"
#include "BoardStateMachine.h"
#include "TextureMap.h"

constexpr Color HOVERED_TILE_COLOR = {0x00, 0xFF, 0xFF, 0x77};
constexpr Color MOVE_TILE_COLOR = Color{0xFF, 0x00, 0xFF, 0x77};

// The main GUI-side class that is responsible for handling GUI logic of the chess board
template<WidgetType MainWidget, HandlerContextType ContextType, typename... Handlers>
class BoardInteractor : public Interactor{
    friend IdleBoardState;
    friend MoveWaitingState;
    friend MoveSelectionBoardState;
    friend MoveCommitWaitingState;
    friend PromotionCommitWaitingState;
    friend GameOverBoardState;

    using BoardWidgetRef = std::reference_wrapper<MainWidget>;
    using FSM = BoardStateMachine<IdleBoardState, MoveWaitingState, MoveSelectionBoardState
        , MoveCommitWaitingState, PromotionCommitWaitingState, GameOverBoardState>; 
public:
    BoardInteractor(NonModalLayerCreateRequest::Payload&&, BoardWidgetRef board, RequestView req)
        : m_gameCache{}
        , m_moveTiles{}
        , m_hoveredTile{}
        , r_widget{board}
        , r_layersRequest{req}
        , m_fsm{IdleBoardState{std::ref(*this)}}
        , m_operation(EmptyOperation{})
    {}

    void dispatchEvents(const LayerEvent& event) {
        std::visit([&](auto&& ev) { process(ev); }, event);
    }

    // Responsibility: poll (non-blockingly) the message queue for any new 
    // messages coming from the network thread and process it if found any. 
    void update() {
        while (auto msg = g_MQ.net2gui.tryPop())
            onNetworkMessage(*msg);
    }

    OperationView getOperation() noexcept {
        return std::ref(m_operation);
    } 

    void processOperation() {
        std::visit([&](auto&& op) { 
            perform(op); 
            m_operation = EmptyOperation{};
        }, m_operation);
    }

    void render(const Renderer& renderer, const Font&) const {
        // // draw hovered
        renderer.renderRect(&m_hoveredTile, HOVERED_TILE_COLOR);

        // draw move tiles 
        renderer.setBlendMode();
        for (auto&& tile : m_moveTiles)
            renderer.renderFillRect(&tile, MOVE_TILE_COLOR);
        renderer.resetBlendMode();

        // draw pieces
        if (m_pieces.empty()) [[unlikely]] {
            m_pieces = TextureMap{Texture{renderer.get(), IMAGE_PIECE_MAP_PATH}};
        }
        const auto& boardWidget = r_widget.get();
        for (auto i = 0uz; i < m_gameCache.board.size(); ++i) {
            auto boardRect = boardWidget.rectFromPos(ChessClient::indexToPos(i));
            auto pieceRect = m_pieces.getTile(m_gameCache.board[i]);
            renderer.renderTexture(m_pieces.get(), &pieceRect, &boardRect);
        }
    }

private:
    template<LayerEventType Event>
    void process(const Event&) {}
    void process(const KeyUpEvent& event) {
        // request game restart when R key is pressed
        if (event.key == KEY_R)
            g_MQ.gui2net.push(RestartReq{});
    }
    void process(const MouseMotionEvent& event) {
        // update the hovered tile
        const auto& w = r_widget.get();
        m_hoveredTile = w.rectFromPos(w.guiPosToLogical(event.x, event.y));
    }
    void process(const MouseLeftDownEvent& event) {
        // state-dependant processing
        m_fsm.process(event);
    }

    // Responsibility: request promotion
    void perform(PromotionOperation op) {
        g_MQ.gui2net.push(PromotionReq{op.type});
    }
    void perform(EmptyOperation) {}

    // Responsibility: process network messages
    void onNetworkMessage(const NetworkMessage& msg) {
        if (auto* restartMsg = std::get_if<RestartMsg>(&msg)) {
            m_gameCache = {
                .board = restartMsg->cache.board,
                .moves = {},
                .selectedPiece = {},
                .myColor = restartMsg->myColor,
                .currTeam = restartMsg->cache.state.currentTeam
            };
            m_moveTiles.clear();
            m_fsm = FSM{IdleBoardState{std::ref(*this)}};
        } else if (std::holds_alternative<ShutdownMsg>(msg))  {
            triggerQuitEvent();
        } else std::visit([&](const auto& m) { m_fsm.process(m); }, msg);
    }
    
    // TODO: GUI knows too much about game logic
    struct GameCache {
        BoardType board;
        Moves moves;
        Pos selectedPiece;
        Piece::Color myColor;
        Piece::Color currTeam;
    } m_gameCache;

    std::vector<Rect> m_moveTiles;
    // texture map
    mutable TextureMap m_pieces;
    // selection rectangle of the hovered-on tile 
    Rect m_hoveredTile;
    // a reference to the table widget
    BoardWidgetRef r_widget;
    // a reference to the layer's request slot
    RequestView r_layersRequest;
    // state machine
    FSM m_fsm;
    // table operations
    OperationRegister m_operation;
};

#endif
