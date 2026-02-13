#ifndef BOARD_INTERACTOR_H
#define BOARD_INTERACTOR_H

#include "RTWgui/Interactors/Interactor.h"
#include "BoardStateMachine.h"
#include "Board.h"
#include "TextureMap.h"

constexpr Color HOVERED_TILE_COLOR = {0x00, 0xFF, 0xFF, 0x77};

template<WidgetType MainWidget = BoardWidget
        , typename ContextType = HandlerContext 
        , typename BoardOps = BoardPromotionHandler<HandlerContext>>
class BoardInteractor : public Interactor{
    friend IdleBoardState;
    friend MoveSelectionBoardState;
    friend GameOverBoardState;

    using BoardWidgetRef = std::reference_wrapper<MainWidget>;
    using FSM = BoardStateMachine<IdleBoardState, MoveSelectionBoardState, GameOverBoardState>; 
public:
    static constexpr bool hasOperations = true;

    BoardInteractor(NonModalLayerCreateRequest::Payload&&, BoardWidgetRef board, RequestView req)
        : m_moveTiles{}
        , m_chess{}
        , m_hoveredTile{}
        , r_widget{board}
        , r_layersRequest{req}
        , m_fsm{IdleBoardState{std::ref(*this)}}
        , m_operation(EmptyOperation{})
    {}
    
    void dispatchEvents(const LayerEvent& event) {
        std::visit([&](auto&& ev) { process(ev); }, event);
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

    void render(const Renderer& renderer, const Font& font) const {
        // // draw hovered
        renderer.renderRect(&m_hoveredTile, HOVERED_TILE_COLOR);

        // draw move tiles 
        m_fsm.draw(renderer, font);

        // draw pieces
        if (m_pieces.get().empty()) [[unlikely]] {
            m_pieces = TextureMap{Texture{renderer.get(), IMAGE_PIECE_MAP_PATH}};
        }
        const auto& board = m_chess.board.get();
        const auto& boardWidget = r_widget.get();
        for (auto i = 0uz; i < board.size(); ++i) {
            auto boardRect = boardWidget.rectFromPos(Board::indexToPos(i));
            auto pieceRect = m_pieces.getTile(board[i].type, board[i].col);
            renderer.renderTexture(m_pieces.get().get(), &pieceRect, &boardRect);
        }
    }

private:
    template<LayerEventType Event>
    void process(const Event&) {}
    void process(const KeyUpEvent& event) {
        if (event.key == KEY_R) {
            m_fsm = FSM{IdleBoardState{std::ref(*this)}};
            m_chess = ChessLogic{};
            m_moveTiles.clear();
        }
    }
    void process(const MouseMotionEvent& event) {
        const auto& w = r_widget.get();
        m_hoveredTile = w.rectFromPos(w.guiPosToLogical(event.x, event.y));
    }
    void process(const MouseLeftDownEvent& event) {
        m_fsm.process(event);
    }

    void perform(ChooseOperation op) {
         m_chess.board.promote(m_chess.selectedPiece, op.type);
    }
    void perform(EmptyOperation) {}

    struct ChessLogic {
        Board board{};
        std::vector<Pos> moves;
        Pos selectedPiece{};
        uint8_t blackScore;
        uint8_t whiteScore;
        PieceColor currentTeam = PieceColor::WHITE;
    };

    std::vector<Rect> m_moveTiles;
    // chess logic
    ChessLogic m_chess;
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
