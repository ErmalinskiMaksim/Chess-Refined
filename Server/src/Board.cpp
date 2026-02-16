#include "Board.h"
#include <utility>
#include <algorithm>

consteval BoardType initBoard() {
    BoardType board;
    board[0] = { PieceType::ROOK,   PieceColor::WHITE};
    board[1] = { PieceType::KNIGHT, PieceColor::WHITE};
    board[2] = { PieceType::BISHOP, PieceColor::WHITE};
    board[3] = { PieceType::QUEEN,  PieceColor::WHITE};
    board[4] = { PieceType::KING,   PieceColor::WHITE};
    board[5] = { PieceType::BISHOP, PieceColor::WHITE};
    board[6] = { PieceType::KNIGHT, PieceColor::WHITE};
    board[7] = { PieceType::ROOK,   PieceColor::WHITE};

    auto i = 8uz;
    while (i < 16uz) board[i++] = { PieceType::PAWN, PieceColor::WHITE };
    while (i < 48uz) board[i++] = { PieceType::NONE, PieceColor::BLACK }; 
    while (i < 56uz) board[i++] = { PieceType::PAWN, PieceColor::BLACK };

    board[56] = { PieceType::ROOK,   PieceColor::BLACK};
    board[57] = { PieceType::KNIGHT, PieceColor::BLACK};
    board[58] = { PieceType::BISHOP, PieceColor::BLACK};
    board[59] = { PieceType::QUEEN,  PieceColor::BLACK};
    board[60] = { PieceType::KING,   PieceColor::BLACK};
    board[61] = { PieceType::BISHOP, PieceColor::BLACK};
    board[62] = { PieceType::KNIGHT, PieceColor::BLACK};
    board[63] = { PieceType::ROOK,   PieceColor::BLACK};
    
    return board;
}

constexpr Board::Directions rookDirections = {
      Pos {0, +1}
    , Pos {0, -1}
    , Pos {+1, 0}
    , Pos {-1, 0}
};
constexpr Board::Directions bishopDirections = {
      Pos {+1, +1}
    , Pos {+1, -1}
    , Pos {-1, -1}
    , Pos {-1, +1}
};
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

Board::Board()
    : m_board{initBoard()}
    , m_blackData{{4, 7}, true, true}
    , m_whiteData{{4, 0}, true, true}
    , m_enPassantTarget{}
    , m_enPassantEnabled{false}
{}

uint8_t Board::commitMove(Pos moveFrom, Pos moveTo, PieceColor teamColor) noexcept {
    // reset en passant flags
    GameFlags flags = GameFlags::NONE;
    bool oldEnPassantEnabled = m_enPassantEnabled;
    Pos oldEnPassantTarget = m_enPassantTarget;
    m_enPassantEnabled = false;

    PieceType capturedType = m_board[posToIndex(moveTo)].type;
    auto movedPiece = at(moveFrom);
    m_board[posToIndex(moveTo)] = 
        std::exchange(m_board[posToIndex(moveFrom)], {PieceType::NONE, PieceColor::BLACK}); 

    PieceColor oppositeTeamColor = (teamColor == PieceColor::WHITE) 
        ? PieceColor::BLACK : PieceColor::WHITE;
    auto& team = (teamColor == PieceColor::WHITE) ? m_whiteData : m_blackData;

    // update castling
    if (team.QSideCastling) { // left rook
        auto rook = at(Pos(0, (movedPiece.col == PieceColor::WHITE) ? 0 : 7));
        team.QSideCastling = (rook.type == PieceType::ROOK && rook.col == movedPiece.col);
    }
    if (team.KSideCastling) { // right rook
        auto rook = at(Pos(7, (movedPiece.col == PieceColor::WHITE) ? 0 : 7));
        team.KSideCastling = (rook.type == PieceType::ROOK && rook.col == movedPiece.col);
    }
    if (movedPiece.type == PieceType::KING) {
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
    if (movedPiece.type == PieceType::PAWN) {
        // update enpassant 
        if (oldEnPassantEnabled && moveTo == oldEnPassantTarget) {
            m_board[posToIndex(Pos(moveTo.x, moveFrom.y))].type = PieceType::NONE;
        } 
        if (std::abs(moveFrom.y - moveTo.y) == 2) {
            m_enPassantEnabled = true;
            m_enPassantTarget = posAhead(moveFrom, movedPiece.col);
        }  
        // check promotion
        if (moveTo.y == ((movedPiece.col == PieceColor::BLACK) ? 0 : 7))
            flags = GameFlags::PROMOTION;
    }

    if (!hasAnyMoves(oppositeTeamColor)) {
        auto& oppositeTeam = (oppositeTeamColor == PieceColor::WHITE)
            ? m_whiteData : m_blackData;
        flags = (isKingAttacked(oppositeTeam.kingPos, oppositeTeamColor))
            ? GameFlags::CHECK_MATE : GameFlags::STALE_MATE;
    }

    // get score
    uint8_t score;
    switch(capturedType) {
        case PieceType::PAWN:   score = 1u; break;
        case PieceType::ROOK:   score = 5u; break;
        case PieceType::KNIGHT: score = 3u; break;
        case PieceType::BISHOP: score = 3u; break;
        case PieceType::QUEEN:  score = 9u; break;
        default:                score = 0u; break;
    }
    return score + static_cast<uint8_t>(flags);
}

void Board::promote(Pos pos, PieceType type) noexcept {
    m_board[posToIndex(pos)].type = type; 
}

bool Board::hasAnyMoves(PieceColor teamColor) noexcept {
    for (auto i = 0uz; i < m_board.size(); ++i) {
        if (m_board[i].col != teamColor || m_board[i].type == PieceType::NONE)
            continue;
        if (!(calculatePieceMoves(indexToPos(i), teamColor).empty())) return true;
    }
    return false;
}

Moves Board::calculatePieceMoves(Pos moveFrom, PieceColor teamColor) noexcept {
    Moves moves{};
    auto movingPiece = at(moveFrom);
    // wrong team
    if (movingPiece.col != teamColor) return {};

    auto& team = (teamColor == PieceColor::WHITE) ? m_whiteData : m_blackData;
    switch (movingPiece.type) {
        case PieceType::PAWN:   
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
        case PieceType::ROOK:   
            moves = calculateSlidingMoves(rookDirections, moveFrom, teamColor);
            break;
        case PieceType::KNIGHT: 
            moves = calculateRoundMoves  (knightDirections, moveFrom, teamColor);
            break;
        case PieceType::BISHOP: 
            moves = calculateSlidingMoves(bishopDirections, moveFrom, teamColor);
            break;
        case PieceType::QUEEN:  
            moves = [&]() { 
                auto mvs = calculateSlidingMoves(rookDirections, moveFrom, teamColor);
                auto bishopMoves = calculateSlidingMoves(bishopDirections, moveFrom, teamColor);
                mvs.insert(mvs.end(),
                    std::make_move_iterator(bishopMoves.begin()),
                    std::make_move_iterator(bishopMoves.end()));
                return mvs;
            }();
            break;
        case PieceType::KING:   
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
    filteredMoves.shrink_to_fit();
    return filteredMoves;
}

Moves Board::calculatePawnMoves(Pos pos, PieceColor pawnColor) const noexcept {
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

    moves.shrink_to_fit();
    return moves;
}

Moves Board::calculateSlidingMoves(const Directions& dirs, Pos pos, PieceColor myColor) const noexcept {
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

    moves.shrink_to_fit();
    return moves;
}

Moves Board::calculateRoundMoves(const RoundDirections& dirs, Pos pos, PieceColor myColor) const noexcept {
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

    moves.shrink_to_fit();
    return moves;
}

bool Board::canCastle(Pos kingPos, PieceColor kingCol, bool isQSide) const noexcept {
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

bool Board::isKingEndangered(Pos kingPos, PieceColor kingCol, Pos moveFrom, Pos moveTo) noexcept {
    const auto movedPiece = std::exchange(m_board[posToIndex(moveFrom)], {PieceType::NONE, PieceColor::BLACK});
    const auto replacedPiece = std::exchange(m_board[posToIndex(moveTo)], movedPiece);

    if (movedPiece.type == PieceType::KING) kingPos = moveTo;
    bool isAttacked = isKingAttacked(kingPos, kingCol);

    m_board[posToIndex(moveFrom)] = movedPiece;
    m_board[posToIndex(moveTo)] = replacedPiece; 
    
    return isAttacked;
}

bool Board::isKingAttacked(Pos kingPos, PieceColor kingCol) const noexcept {
    return   isKingAttackedByPawn   (kingPos, kingCol)
          || isKingAttackedBySliding(rookDirections,   kingPos, {PieceType::ROOK,   kingCol})
          || isKingAttackedBySliding(bishopDirections, kingPos, {PieceType::BISHOP, kingCol})
          || isKingAttackedByRound  (knightDirections, kingPos, {PieceType::KNIGHT, kingCol})
          || isKingAttackedByRound  (kingDirections,   kingPos, {PieceType::KING,   kingCol});
}

bool Board::isKingAttackedByPawn(Pos kingPos, PieceColor kingCol) const noexcept {
    auto leftAhead = posAhead(Pos(kingPos.x-1, kingPos.y), kingCol);
    auto rightAhead = posAhead(Pos(kingPos.x+1, kingPos.y), kingCol);

    if (inLegalBounds(leftAhead)) {
        auto pieceleftAhead = at(leftAhead);
        if (pieceleftAhead.type == PieceType::PAWN && pieceleftAhead.col != kingCol)
            return true;
    }
    if (inLegalBounds(rightAhead)) {
        auto pieceRightAhead = at(rightAhead);
        if (pieceRightAhead.type == PieceType::PAWN && pieceRightAhead.col != kingCol)
            return true;
    }
    return false;
}

// target = {enemy type; friend color}
bool Board::isKingAttackedBySliding(const Directions& dirs, Pos kingPos, Piece target) const noexcept {
    for (auto&& dir : dirs) {
        for (Pos i = kingPos + dir; inLegalBounds(i); i = i + dir) {
            auto piece = at(i);
            if (!isEmpty(piece)) {
                if ((piece.type == PieceType::QUEEN || piece.type == target.type)
                        && piece.col != target.col) 
                    return true;
                break;
            }
        }
    }
    return false;
}

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
