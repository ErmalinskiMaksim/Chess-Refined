#ifndef BOARD_INTERACTOR_H
#define BOARD_INTERACTOR_H

#include "RTWgui/Interactors/Interactor.h"
#include "BoardStateMachine.h"
#include "TextureMap.h"

constexpr Color HOVERED_TILE_COLOR = {0x00, 0xFF, 0xFF, 0x77};
constexpr Color MOVE_TILE_COLOR = Color{0xFF, 0x00, 0xFF, 0x77};

template<WidgetType MainWidget, typename ContextType, typename... Handlers>
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
    static constexpr bool hasOperations = true;

    BoardInteractor(NonModalLayerCreateRequest::Payload&&, BoardWidgetRef board, RequestView req)
        : m_moveTiles{}
        , m_hoveredTile{}
        , r_widget{board}
        , r_layersRequest{req}
        , m_fsm{IdleBoardState{std::ref(*this)}}
        , m_operation(EmptyOperation{})
    {
        ChessClient::get().registerInteractor(this);
    }

    ~BoardInteractor() {
        ChessClient::get().unregisterInteractor();
    }
    
    void dispatchEvents(const LayerEvent& event) {
        std::visit([&](auto&& ev) { process(ev); }, event);
    }

    void onNetworkMessage(NetworkMessage&& msg) {
        if (std::holds_alternative<RestartMsg>(msg)) [[unlikely]] {
            m_fsm = FSM{IdleBoardState{std::ref(*this)}};
            m_moves.clear();
            m_moveTiles.clear();
        }
        std::visit([&](auto&& m) { m_fsm.process(std::move(m)); }, msg);
    }
    
    OperationView getOperation() {
        return std::ref(m_operation);
    } 

    void processOperation() {
        std::visit([&](auto&& op) { 
            perform(op); 
            m_operation = EmptyOperation{};
        }, m_operation);
    }

    void render(const Renderer& renderer, const Font&) const {
        ChessClient::get().poll();
        // // draw hovered
        renderer.renderRect(&m_hoveredTile, HOVERED_TILE_COLOR);

        // draw move tiles 
        renderer.setBlendMode();
        for (auto&& tile : m_moveTiles)
            renderer.renderFillRect(&tile, MOVE_TILE_COLOR);
        renderer.resetBlendMode();

        // draw pieces
        if (m_pieces.get().empty()) [[unlikely]] {
            m_pieces = TextureMap{Texture{renderer.get(), IMAGE_PIECE_MAP_PATH}};
        }
        const auto& board = ChessClient::get().getBoardCache();
        const auto& boardWidget = r_widget.get();
        for (auto i = 0uz; i < board.size(); ++i) {
            auto boardRect = boardWidget.rectFromPos(ChessClient::indexToPos(i));
            auto pieceRect = m_pieces.getTile(board[i]);
            renderer.renderTexture(m_pieces.get().get(), &pieceRect, &boardRect);
        }
    }

private:
    template<LayerEventType Event>
    void process(const Event&) {}
    void process(const KeyUpEvent& event) {
        if (event.key == KEY_R)
            ChessClient::get().requestRestart();
    }
    void process(const MouseMotionEvent& event) {
        const auto& w = r_widget.get();
        m_hoveredTile = w.rectFromPos(w.guiPosToLogical(event.x, event.y));
    }
    void process(const MouseLeftDownEvent& event) {
        m_fsm.process(event);
    }

    void perform(PromotionOperation op) {
        ChessClient::get().requestPromotion(op.type);
    }
    void perform(EmptyOperation) {}


    Moves m_moves;
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
