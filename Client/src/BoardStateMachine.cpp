#include "BoardStateMachine.h"
#include "BoardInteractor.h"
#include <algorithm>

OptState IdleBoardState::process(const MouseLeftDownEvent& event) {
    auto& cli = ChessClient::get();
    auto boardPos = m_context.get().r_widget.get().guiPosToLogical(event.x, event.y);
    if (cli.canMove(boardPos)) {
        cli.requestAvailableMoves(boardPos);
        return MoveWaitingState{m_context};  
    }
    return std::nullopt;
}

OptState IdleBoardState::process(GameState::Flags flag) {
    auto& c = m_context.get();
    if (flag == GameState::Flags::STALE_MATE 
        || flag == GameState::Flags::CHECK_MATE) {
            GameOverHandler<HandlerContext>::requestMainMenu(
                HandlerContext{ c.r_widget, c.r_layersRequest, std::nullopt 
                    , ChessClient::get().getCache()});
            return GameOverBoardState{c};
    }
    return std::nullopt;
}

OptState MoveWaitingState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

OptState MoveWaitingState::process(Moves moves) {
    auto& c = m_context.get();
    const auto& w = c.r_widget.get();
    // get moves 
    c.m_moves = std::move(moves);
    // doesn't have moves, so cannot be selected
    if(c.m_moves.empty()) return IdleBoardState{c};
    // fill gui move tiles
    for (auto&& move : c.m_moves) 
        c.m_moveTiles.emplace_back(w.rectFromPos(move));

    return MoveSelectionBoardState{c};
}

OptState MoveSelectionBoardState::process(const MouseLeftDownEvent& event) {
    auto& c = m_context.get();
    // get pos 
    auto boardPos = c.r_widget.get().guiPosToLogical(event.x, event.y);
    // if the selected piece is clicked, it gets disselected
    if (boardPos == ChessClient::get().getSelectedPieceCache())  {
        c.m_moves.clear();
        c.m_moveTiles.clear();
        return IdleBoardState{c};
    }
    // if a tile outside the move list was selected
    auto move = std::find(c.m_moves.begin(), c.m_moves.end(), boardPos);
    if(move == c.m_moves.end()) return std::nullopt;
    // request a move commit
    ChessClient::get().requestCommitMove(*move);
    return MoveCommitWaitingState{c};
}

OptState MoveCommitWaitingState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

OptState MoveCommitWaitingState::process(GameState::Flags flag) {
    auto& c = m_context.get();
    c.m_moves.clear();
    c.m_moveTiles.clear();

    switch (flag) {
        case GameState::Flags::NONE:
            return IdleBoardState{c};
        case GameState::Flags::PROMOTION:
            BoardPromotionHandler<HandlerContext>::requestMainMenu(
                HandlerContext{ c.r_widget, c.r_layersRequest, c.getOperation()});
            return PromotionCommitWaitingState{c};
        default:
            GameOverHandler<HandlerContext>::requestMainMenu(
                HandlerContext{ c.r_widget, c.r_layersRequest, std::nullopt 
                    , ChessClient::get().getCache()});
            return GameOverBoardState{c};
    }
}

OptState PromotionCommitWaitingState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

OptState PromotionCommitWaitingState::process(PromotionMsg) {
    return IdleBoardState{m_context};
}

OptState GameOverBoardState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

