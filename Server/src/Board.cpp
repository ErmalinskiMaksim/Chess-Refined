#include "Board.h"
#include <utility>
#include <algorithm>

// Responsibilities:
// * Default-initialize a chess board
consteval BoardType initBoard() {
    BoardType board;
    board[0] = { Piece::Type::ROOK,   Piece::Color::WHITE};
    board[1] = { Piece::Type::KNIGHT, Piece::Color::WHITE};
    board[2] = { Piece::Type::BISHOP, Piece::Color::WHITE};
    board[3] = { Piece::Type::QUEEN,  Piece::Color::WHITE};
    board[4] = { Piece::Type::KING,   Piece::Color::WHITE};
    board[5] = { Piece::Type::BISHOP, Piece::Color::WHITE};
    board[6] = { Piece::Type::KNIGHT, Piece::Color::WHITE};
    board[7] = { Piece::Type::ROOK,   Piece::Color::WHITE};

    auto i = 8uz;
    while (i < 16uz) board[i++] = { Piece::Type::PAWN, Piece::Color::WHITE };
    while (i < 48uz) board[i++] = { Piece::Type::NONE, Piece::Color::BLACK }; 
    while (i < 56uz) board[i++] = { Piece::Type::PAWN, Piece::Color::BLACK };

    board[56] = { Piece::Type::ROOK,   Piece::Color::BLACK};
    board[57] = { Piece::Type::KNIGHT, Piece::Color::BLACK};
    board[58] = { Piece::Type::BISHOP, Piece::Color::BLACK};
    board[59] = { Piece::Type::QUEEN,  Piece::Color::BLACK};
    board[60] = { Piece::Type::KING,   Piece::Color::BLACK};
    board[61] = { Piece::Type::BISHOP, Piece::Color::BLACK};
    board[62] = { Piece::Type::KNIGHT, Piece::Color::BLACK};
    board[63] = { Piece::Type::ROOK,   Piece::Color::BLACK};
    
    return board;
}

// Vectors of rook's attack
constexpr Board::Directions rookDirections = {
      Pos {0, +1}
    , Pos {0, -1}
    , Pos {+1, 0}
    , Pos {-1, 0}
};
// Vectors of bishop's attack
constexpr Board::Directions bishopDirections = {
      Pos {+1, +1}
    , Pos {+1, -1}
    , Pos {-1, -1}
    , Pos {-1, +1}
};
// Vectors of king's attack
constexpr Board::RoundDirections kingDirections = {
      Pos{-1, -1}
    , Pos{-1,  0}
    , Pos{-1, +1}
    , Pos{ 0, -1}
    , Pos{ 0, +1}
    , Pos{ 1, -1}
    , Pos{+1,  0}
    , Pos{+1, +1}
};
// Vectors of knight's attack
constexpr Board::RoundDirections knightDirections = {
      Pos{-2, +1}
    , Pos{-2, -1}
    , Pos{-1, +2}
    , Pos{-1, -2}
    , Pos{+1, +2}
    , Pos{+1, -2}
    , Pos{+2, +1}
    , Pos{+2, -1}
};

// Responsibilities:
// * Initialize the board 
// * Set default flags
Board::Board()
    : m_board{initBoard()}
    , m_blackData{{4, 7}, true, true}
    , m_whiteData{{4, 0}, true, true}
    , m_enPassantTarget{}
    , m_enPassantEnabled{false}
{}

// Responsibilities:
// * Perform move commit logic according to chess rules
// * Calculate and return a score of the captured piece
uint8_t Board::commitMove(Pos moveFrom, Pos moveTo, Piece::Color teamColor) noexcept {
    // reset en passant flags
    auto flags = GameState::Flags::NONE;
    bool oldEnPassantEnabled = m_enPassantEnabled;
    Pos oldEnPassantTarget = m_enPassantTarget;
    m_enPassantEnabled = false;

    auto capturedType = at(moveTo).type;
    auto movedPiece = at(moveFrom);

    // NOTE: at() method is not called because it returns a copy
    m_board[posToIndex(moveTo)] = 
        std::exchange(m_board[posToIndex(moveFrom)], {Piece::Type::NONE, Piece::Color::BLACK}); 

    // determine the colors of a current and opponent teams' colors
    auto oppositeTeamColor = teamColor == Piece::Color::WHITE ? Piece::Color::BLACK : Piece::Color::WHITE;
    auto& team = teamColor == Piece::Color::WHITE ? m_whiteData : m_blackData;

    // update castling
    if (team.QSideCastling) { // left rook
        auto rook = at(Pos(0, (movedPiece.col == Piece::Color::WHITE) ? 0 : 7));
        team.QSideCastling = (rook.type == Piece::Type::ROOK && rook.col == movedPiece.col);
    }
    if (team.KSideCastling) { // right rook
        auto rook = at(Pos(7, (movedPiece.col == Piece::Color::WHITE) ? 0 : 7));
        team.KSideCastling = (rook.type == Piece::Type::ROOK && rook.col == movedPiece.col);
    }
    if (movedPiece.type == Piece::Type::KING) { // king movement while castling
        if (std::abs(team.kingPos.x - moveTo.x) == 2) {
            Pos oldRookPos; 
            Pos newRookPos;
            if (moveTo.x == 2) { // left castling
                oldRookPos = {0, team.kingPos.y};
                newRookPos = {3, team.kingPos.y};
            } else { // right castling
                oldRookPos = {7, team.kingPos.y};
                newRookPos = {5, team.kingPos.y};
            }
            std::swap(m_board[posToIndex(oldRookPos)], m_board[posToIndex(newRookPos)]);
        }
        team.kingPos = moveTo;
        team.QSideCastling = team.KSideCastling = false;
    }
    // enpassant and promotion
    if (movedPiece.type == Piece::Type::PAWN) {
        // update enpassant 
        if (oldEnPassantEnabled && moveTo == oldEnPassantTarget) {
            m_board[posToIndex(Pos(moveTo.x, moveFrom.y))].type = Piece::Type::NONE;
        } 
        if (std::abs(moveFrom.y - moveTo.y) == 2) {
            m_enPassantEnabled = true;
            m_enPassantTarget = posAhead(moveFrom, movedPiece.col);
        }  
        // check promotion
        if (moveTo.y == ((movedPiece.col == Piece::Color::BLACK) ? 0 : 7))
            flags = GameState::Flags::PROMOTION;
    }

    // check if a team has any moves to set a check mate or stale mate flags
    if (!hasAnyMoves(oppositeTeamColor)) {
        auto& oppositeTeam = (oppositeTeamColor == Piece::Color::WHITE)
            ? m_whiteData : m_blackData;
        flags = (isKingAttacked(oppositeTeam.kingPos, oppositeTeamColor))
            ? GameState::Flags::CHECK_MATE : GameState::Flags::STALE_MATE;
    }

    // get score
    uint8_t score;
    switch(capturedType) {
        case Piece::Type::PAWN:   score = 1u; break;
        case Piece::Type::ROOK:   score = 5u; break;
        case Piece::Type::KNIGHT: score = 3u; break;
        case Piece::Type::BISHOP: score = 3u; break;
        case Piece::Type::QUEEN:  score = 9u; break;
        default:                score = 0u; break;
    }
    return score + static_cast<uint8_t>(flags);
}

// Responsibilities:
// * perform promotion
void Board::promote(Pos pos, Piece::Type type) noexcept {
    m_board[posToIndex(pos)].type = type; 
}

// Responsibilities:
// * Check if a team of a specified color has any moves left
bool Board::hasAnyMoves(Piece::Color teamColor) noexcept {
    for (auto i = 0uz; i < m_board.size(); ++i) {
        if (m_board[i].col != teamColor || isEmpty(m_board[i])) continue;
        if (!(calculatePieceMoves(indexToPos(i), teamColor).empty())) return true;
    }
    return false;
}

// Responsibilities:
// * Calculate and return legal moves for a piece at a given position with a given color
Moves Board::calculatePieceMoves(Pos moveFrom, Piece::Color teamColor) noexcept {
    Moves moves{};
    auto movingPiece = at(moveFrom);

    auto& team = (teamColor == Piece::Color::WHITE) ? m_whiteData : m_blackData;
    switch (movingPiece.type) {
        case Piece::Type::PAWN:   
            moves = [&]() {
                auto mvs = calculatePawnMoves(moveFrom, teamColor);
                // en passant 
                if (m_enPassantEnabled) {
                    auto ahead = posAhead(moveFrom, movingPiece.col);
                    if (std::abs(ahead.x - m_enPassantTarget.x) == 1 && ahead.y == m_enPassantTarget.y)
                        mvs.emplace_back(m_enPassantTarget);
                }
                return mvs;
            }();
            break;
        case Piece::Type::ROOK:   
            moves = calculateSlidingMoves(rookDirections, moveFrom, teamColor);
            break;
        case Piece::Type::KNIGHT: 
            moves = calculateRoundMoves  (knightDirections, moveFrom, teamColor);
            break;
        case Piece::Type::BISHOP: 
            moves = calculateSlidingMoves(bishopDirections, moveFrom, teamColor);
            break;
        case Piece::Type::QUEEN:  
            moves = [&]() { 
                auto mvs = calculateSlidingMoves(rookDirections, moveFrom, teamColor);
                auto bishopMoves = calculateSlidingMoves(bishopDirections, moveFrom, teamColor);
                mvs.insert(mvs.end(),
                    std::make_move_iterator(bishopMoves.begin()),
                    std::make_move_iterator(bishopMoves.end()));
                return mvs;
            }();
            break;
        case Piece::Type::KING:   
            moves = [&]() {
                auto mvs = calculateRoundMoves  (kingDirections, moveFrom, teamColor);
                // castling 
                if (team.QSideCastling && canCastle(moveFrom, teamColor, true)) 
                    mvs.emplace_back(moveFrom + Pos{ -2, 0});
                if (team.KSideCastling && canCastle(moveFrom, teamColor, false))
                    mvs.emplace_back(moveFrom + Pos{ +2, 0});
                return mvs;
            }();
            break;
        default: break;
    }

    // filter out moves that are illegal if king is pinned 
    Moves filteredMoves{};
    filteredMoves.reserve(moves.size());
    for (auto&& move : moves) {
        if (!isKingEndangered(team.kingPos, teamColor, moveFrom, move))
            filteredMoves.emplace_back(move);
    }
    return filteredMoves;
}

// Responsibilities:
// * Calculate pawn moves
Moves Board::calculatePawnMoves(Pos pos, Piece::Color pawnColor) const noexcept {
    Moves moves;
    moves.reserve(4);

	auto ahead = posAhead(pos, pawnColor);
	if (ahead.y >= 0 && ahead.y <= 7)
	{
        // ahead
		if (isEmpty(at(ahead))) 
		{
			moves.emplace_back(ahead);
            // double ahead
            bool movesFirstTime = (pos.y == (6 - static_cast<int8_t>(pawnColor)*5));
            auto aheadOfAhead = posAhead(ahead, pawnColor);
			if (movesFirstTime && isEmpty(at(aheadOfAhead)))
				moves.emplace_back(aheadOfAhead);
		}
        // left ahead
        if (pos.x > 0) {
            auto leftAhead = Pos(pos.x - 1, ahead.y);
            auto pieceLeftAhead = at(leftAhead);
            if (!isEmpty(pieceLeftAhead) && pieceLeftAhead.col != pawnColor)
                moves.emplace_back(leftAhead);
        } 
        // right ahead
        if (pos.x < 7) {
            auto rightAhead = Pos(pos.x + 1, ahead.y);
            auto pieceRightAhead = at(rightAhead);
            if (!isEmpty(pieceRightAhead) && pieceRightAhead.col != pawnColor)
                moves.emplace_back(rightAhead);
        }
	}

    return moves;
}

// Responsibilities:
// * Calculate rook/bishop/queen moves
Moves Board::calculateSlidingMoves(const Directions& dirs, Pos pos, Piece::Color myColor) const noexcept {
    Moves moves;
    moves.reserve(14);

    for (auto&& dir : dirs)
        for (Pos i = pos + dir; inLegalBounds(i); i = i + dir) {
            auto piece = at(i);
            if (isEmpty(piece)) moves.emplace_back(i);
            else {
                if (piece.col != myColor) moves.emplace_back(i);
                break;
            }
        }

    return moves;
}

// Responsibilities:
// * Calculate king/knight moves
Moves Board::calculateRoundMoves(const RoundDirections& dirs, Pos pos, Piece::Color myColor) const noexcept {
    Moves moves;
    moves.reserve(8);

    for (auto&& dir : dirs) {
        auto currentPos = pos + dir;
        if (inLegalBounds(currentPos)) {
            auto piece = at(currentPos);
            if (isEmpty(piece) || piece.col != myColor)
                moves.emplace_back(currentPos);
        }
    }

    return moves;
}

// Responsibilities:
// * check if a king at a given pos, of given color can castle to a given side
bool Board::canCastle(Pos kingPos, Piece::Color kingCol, bool isQSide) const noexcept {
    Pos closeTile = {1, 0};
    if (isQSide) {
        if (!isEmpty(at(kingPos + Pos{-3, 0}))) return false;
        closeTile.x = -1;
    }
    Pos farTile = closeTile + closeTile;
    closeTile = kingPos + closeTile;
    farTile = kingPos + farTile;
    if (isEmpty(at(closeTile)) && isEmpty(at(farTile))) 
        return !isKingAttacked(kingPos, kingCol) 
            && !isKingAttacked(closeTile, kingCol) 
            && !isKingAttacked(farTile, kingCol);
    return false;
}

// Responsibilities:
// * Check if king is attacked by any piece if a move is committed
bool Board::isKingEndangered(Pos kingPos, Piece::Color kingCol, Pos moveFrom, Pos moveTo) noexcept {
    const auto movedPiece = std::exchange(m_board[posToIndex(moveFrom)], {Piece::Type::NONE, Piece::Color::BLACK});
    const auto replacedPiece = std::exchange(m_board[posToIndex(moveTo)], movedPiece);

    if (movedPiece.type == Piece::Type::KING) kingPos = moveTo;
    bool isAttacked = isKingAttacked(kingPos, kingCol);

    m_board[posToIndex(moveFrom)] = movedPiece;
    m_board[posToIndex(moveTo)] = replacedPiece; 
    
    return isAttacked;
}

// Responsibilities:
// * Check if king is attacked by any piece
bool Board::isKingAttacked(Pos kingPos, Piece::Color kingCol) const noexcept {
    return   isKingAttackedByPawn   (kingPos, kingCol)
          || isKingAttackedBySliding(rookDirections,   kingPos, {Piece::Type::ROOK,   kingCol})
          || isKingAttackedBySliding(bishopDirections, kingPos, {Piece::Type::BISHOP, kingCol})
          || isKingAttackedByRound  (knightDirections, kingPos, {Piece::Type::KNIGHT, kingCol})
          || isKingAttackedByRound  (kingDirections,   kingPos, {Piece::Type::KING,   kingCol});
}

// Responsibilities:
// * Check if king is attacked by pawns
bool Board::isKingAttackedByPawn(Pos kingPos, Piece::Color kingCol) const noexcept {
    auto leftAhead = posAhead(Pos(kingPos.x-1, kingPos.y), kingCol);
    auto rightAhead = posAhead(Pos(kingPos.x+1, kingPos.y), kingCol);

    if (inLegalBounds(leftAhead)) {
        auto pieceleftAhead = at(leftAhead);
        if (pieceleftAhead.type == Piece::Type::PAWN && pieceleftAhead.col != kingCol)
            return true;
    }
    if (inLegalBounds(rightAhead)) {
        auto pieceRightAhead = at(rightAhead);
        if (pieceRightAhead.type == Piece::Type::PAWN && pieceRightAhead.col != kingCol)
            return true;
    }
    return false;
}

// Responsibilities:
// * Check if king is attacked by rooks/bishops/queens
// target = {enemy type; friend color}
bool Board::isKingAttackedBySliding(const Directions& dirs, Pos kingPos, Piece target) const noexcept {
    for (auto&& dir : dirs) {
        for (Pos i = kingPos + dir; inLegalBounds(i); i = i + dir) {
            auto piece = at(i);
            if (!isEmpty(piece)) {
                if ((piece.type == Piece::Type::QUEEN || piece.type == target.type)
                        && piece.col != target.col) 
                    return true;
                break;
            }
        }
    }
    return false;
}

// Responsibilities:
// * Check if king is attcked by king/knights
// target = {enemy type; friend color}
bool Board::isKingAttackedByRound(const RoundDirections& dirs, Pos kingPos, Piece target) const noexcept {
    for (auto&& dir : dirs) {
        auto i = kingPos + dir;
        if (inLegalBounds(i)) {
            auto piece = at(i);
            if (piece.type == target.type && piece.col != target.col) 
                return true;
        }
    }
    return false;
}
