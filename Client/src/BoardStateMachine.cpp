#include "BoardStateMachine.h"
#include "BoardInteractor.h"
#include <algorithm>

/// STATE MACHINE:
///   ___RESTART_________________________________________________________________
///  |            |            |                |                   |           |
///  V            |            |                |                   |           |
/// IDLE -> MOVE WAITING -> MOVE SELECT -> MOVE COMMIT WAITING | -> GAMEOVER    |
///  ^            |            |                               | -> PROMOTION WAITING   
///  |____________|            |                               |     |
///  |_________________________|                               |     |
///  |_________________________________________________________|     |     
///  |_______________________________________________________________|

// Responsibilities:
// * Handle the events in the default idle board state 
// -> on mouse click, check if it's the clients turn to move 
// -> if the client can move, send a request to get moves.
//          (idle state -> move waiting state)
// -> else do nothing
OptState IdleBoardState::process(const MouseLeftDownEvent& event) {
    auto& c = m_context.get();

    // get clicked tile
    auto boardPos = c.r_widget.get().guiPosToLogical(event.x, event.y);

    if (c.m_gameCache.myColor == c.m_gameCache.currTeam
        && c.m_gameCache.board[ChessClient::posToIndex(boardPos)].col == c.m_gameCache.myColor) {
        g_MQ.gui2net.push(AvailableMovesReq{boardPos});
        c.m_gameCache.selectedPiece = boardPos;
        return MoveWaitingState{m_context};  
    } 
    
    return std::nullopt;
}

// Responsibilities:
// * Handle incoming state update messages;
// -> update the local game state cache 
// -> check if the current game state is gameover 
//          (idle state -> game over state)
//  XXX necessary for the client that didn't move.
// -> else remain in the idle state
// 
// - This method may be called in the following circumstances:
// = On game start up 
// = On turn start 
OptState IdleBoardState::process(const StateUpdateMsg& msg) {
    auto& c = m_context.get();
    
    // update the cache 
    c.m_gameCache.board = msg.cache.board;
    c.m_gameCache.currTeam = msg.cache.state.currentTeam;
    c.m_gameCache.myColor = msg.myColor;

    // if gameover flag is set, request a gameover menu and change to gameover state
    // FIXME: this if used to be hidden in the client side before multithreading, now 
    // the GUI is forced to handle it (for performance reasons)
    if (msg.cache.state.flag == GameState::Flags::STALE_MATE 
        || msg.cache.state.flag == GameState::Flags::CHECK_MATE) {
            GameOverHandler<HandlerContext>::requestMainMenu(
                HandlerContext{ c.r_widget, c.r_layersRequest, std::nullopt, msg.cache.state});
            return GameOverBoardState{c};
    }
    return std::nullopt;
}

// Responsibilities:
// * Ignore any clicks while waiting for the moves 
OptState MoveWaitingState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

// Responsibilities:
// * React to an incoming available moves message 
// * If no moves are available, roll back to idle state 
//      (move waiting state -> idle state)
// * Else update local moves cache and generate move rectangles;  
//      (move waiting state -> select state)
OptState MoveWaitingState::process(const Moves& moves) {
    auto& c = m_context.get();
    
    // get moves 
    c.m_gameCache.moves = std::move(moves);
    
    // doesn't have moves, so cannot be selected
    if(c.m_gameCache.moves.empty()) return IdleBoardState{c};

    // fill gui move tiles
    const auto& w = c.r_widget.get();
    for (auto&& move : c.m_gameCache.moves) 
        c.m_moveTiles.emplace_back(w.rectFromPos(move));

    return MoveSelectionBoardState{c};
}

// Responsibilities:
// * Process piece selection procedure
//
// -> if the piece was unselected, cancel the move selection 
//      (move selection state -> idle state)
// -> if a tile outside the move list was chosen, ignore 
// -> if a legal move was chosen, request a move commit 
//      (move select state -> move commit waiting state)
OptState MoveSelectionBoardState::process(const MouseLeftDownEvent& event) {
    auto& c = m_context.get();

    // get clicked tile
    auto boardPos = m_context.get().r_widget.get().guiPosToLogical(event.x, event.y);

    // undo select if the select piece is clicked while selecting a move
    // FIXME: this if used to be hidden in the client before multithreading. 
    // Now, it's here for the performance reasons
    if (boardPos == c.m_gameCache.selectedPiece) {
        c.m_gameCache.moves.clear();
        c.m_moveTiles.clear();
        return IdleBoardState{c};
    }

    // find a selected tile in the move list
    auto move = std::find(c.m_gameCache.moves.begin(), c.m_gameCache.moves.end(), boardPos);
    
    // if a tile is no in the move list ignore the click
    if(move == c.m_gameCache.moves.end()) return std::nullopt;

    // request a move commit, if the move is legal
    g_MQ.gui2net.push(MoveCommitReq{*move});
    return MoveCommitWaitingState{c};
}

// Responsibilities:
// * ignore any clicks while the client is waiting for move commit
OptState MoveCommitWaitingState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

// Responsibilities:
// * Update the local cache as soon as the updated game state comes in
// * Check for flags at the end of the move.
//
// -> If a promotion flag is set, initiate promotion 
//      (move commit waiting state -> promotion state)
// -> If a gameover flag is set, change to gameover state 
//      (move commit state -> gameover state)
OptState MoveCommitWaitingState::process(const StateUpdateMsg& msg) {
    auto& c = m_context.get();

    // update the cache
    c.m_gameCache.board = msg.cache.board;
    c.m_gameCache.currTeam = msg.cache.state.currentTeam;
    c.m_gameCache.moves.clear();
    c.m_moveTiles.clear();

    // check for promotion or gameover at of the move end
    switch (msg.cache.state.flag) {
        case GameState::Flags::NONE: // finish the move
            return IdleBoardState{c};
        case GameState::Flags::PROMOTION: // request a promotion menu
            BoardPromotionHandler<HandlerContext>::requestMainMenu(
                HandlerContext{ c.r_widget, c.r_layersRequest, c.getOperation()});
            return PromotionCommitWaitingState{c};
        default: // request a gameover menu
            GameOverHandler<HandlerContext>::requestMainMenu(
                HandlerContext{ c.r_widget, c.r_layersRequest, std::nullopt, msg.cache.state});
            return GameOverBoardState{c};
    }
}

// Responsibilities:
// * ignore any clicks while waiting for promotion confirmation
OptState PromotionCommitWaitingState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}

// Responsibilities:
// * Update the board, when the state is updated after the promotion is committed
OptState PromotionCommitWaitingState::process(const StateUpdateMsg& msg) {
    // FIXME: previously, it didn't copy the whole board but changed a single piece.
    // The problem with doing so is giving the GUI part too much game handling knowledge.
    m_context.get().m_gameCache.board = msg.cache.board;
    return IdleBoardState{m_context};
}

// Responsibilities:
// * Ignore all clicks
OptState GameOverBoardState::process(const MouseLeftDownEvent&) {
    return std::nullopt;
}
