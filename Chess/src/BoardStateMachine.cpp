#include "BoardStateMachine.h"
#include "BoardHandlers.h"
#include "BoardInteractor.h"
#include "Piece.h"
#include <algorithm>
#include <print>

constexpr Color MOVE_TILE_COLOR = Color{0xFF, 0x00, 0xFF, 0x77};

///////// LOGIC //////////////////
OptState IdleBoardState::process(const MouseLeftDownEvent& event) {
    auto& c = m_context.get();
    const auto& w = c.r_widget.get();
    auto& chess = c.m_chess;
    // get pos 
    Pos boardPos = w.guiPosToLogical(event.x, event.y);
    if (!chess.board.compareColor(boardPos, chess.currentTeam))
        return std::nullopt; // if wrong team is selected
    // get moves 
    chess.moves = chess.board.calculatePieceMoves(boardPos, chess.currentTeam);
    // doesn't have moves, so cannot be selected
    if(chess.moves.empty()) return std::nullopt;
    // select
    chess.selectedPiece = boardPos;
    // fill gui move tiles
    for (auto&& move : chess.moves) 
        c.m_moveTiles.emplace_back(w.rectFromPos(move));

    return MoveSelectionBoardState{c};   
}

OptState MoveSelectionBoardState::process(const MouseLeftDownEvent& event) {
    auto& c = m_context.get();
    auto& chess = c.m_chess;
    // get pos 
    Pos boardPos = c.r_widget.get().guiPosToLogical(event.x, event.y);
    // if the selected piece is clicked, it gets disselected
    if (boardPos == chess.selectedPiece)  {
        chess.moves.clear();
        c.m_moveTiles.clear();
        return IdleBoardState{c};
    }
    // if a tile outside the move list was selected
    auto move = std::find(chess.moves.begin(), chess.moves.end(), boardPos);
    if(move == chess.moves.end()) return std::nullopt;
    // commit move 
    auto score = chess.board.commitMove(chess.selectedPiece, *move, chess.currentTeam);
    chess.selectedPiece = *move;
    // check for promotion
    if (chess.board.isPromotionPossible(chess.selectedPiece)) {
        BoardPromotionHandler<HandlerContext>::requestMainMenu(
            HandlerContext{c.r_widget, c.r_layersRequest, c.getOperation()});
    }
    // update
    PieceColor nextTeam;
    if (chess.currentTeam == PieceColor::WHITE) {
        chess.whiteScore += score;
        nextTeam = PieceColor::BLACK;
    }
    else { 
        chess.blackScore+= score;
        nextTeam = PieceColor::WHITE;
    }

    // check for game over
    if(!chess.board.anyMovesAvailable(nextTeam)) {
        if (chess.board.isKingChecked(chess.currentTeam)) {
            std::println("{} WIN! CHECK MATE", (chess.currentTeam == PieceColor::WHITE) ? "WHITE" : "BLACK");
        } else { 
            std::println("STALE MATE!");
        } 
        std::println("WHITE SCORE: {}, BLACK SCORE: {}", chess.whiteScore, chess.blackScore);
        GameOverBoardState{c};
    }
    chess.moves.clear();
    c.m_moveTiles.clear();
    chess.currentTeam = nextTeam;

    return IdleBoardState{c};
}

OptState GameOverBoardState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

///////////////// RENDERING //////////////////// 
void IdleBoardState::draw(const Renderer&, const Font&) const {}
void MoveSelectionBoardState::draw(const Renderer& renderer, const Font&) const {
    const auto& c = m_context.get();
    renderer.setBlendMode();
    for (auto&& tile : c.m_moveTiles)
        renderer.renderFillRect(&tile, MOVE_TILE_COLOR);
    renderer.resetBlendMode();
}
void GameOverBoardState::draw(const Renderer&, const Font&) const {}
