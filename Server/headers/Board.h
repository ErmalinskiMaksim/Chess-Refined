#ifndef BOARD_H
#define BOARD_H

#include "Common.h"

class Board {
    friend consteval BoardType initBoard();
public:
    using Directions = const std::array<Pos, 4>;
    using RoundDirections = const std::array<Pos, 8>;

    Board();
    inline BoardType get() const noexcept {
        return m_board;
    }
    static inline Pos indexToPos(size_t idx) noexcept {
        return Pos(idx % 8, static_cast<int8_t>(idx) / 8);
    } 

    uint8_t commitMove(Pos, Pos, PieceColor) noexcept;
    Moves calculatePieceMoves(Pos, PieceColor) noexcept;
    void promote(Pos, PieceType) noexcept;

private:
    static inline Pos posAhead(Pos p, PieceColor col) noexcept {
        return Pos(p.x, static_cast<int8_t>(p.y + 2*static_cast<int8_t>(col) - 1));
    }
    static inline bool isEmpty(Piece p) noexcept {
        return p.type == PieceType::NONE;
    }
    static inline size_t posToIndex(Pos pos) noexcept {
        return static_cast<size_t>(pos.x + pos.y*8);
    }
    static inline bool inLegalBounds(Pos moveTo) noexcept {
        return (moveTo.x >= 0 && moveTo.x <= 7 && moveTo.y >= 0 && moveTo.y <= 7);
    }

    inline Piece at(Pos pos) const noexcept {
        return m_board[posToIndex(pos)];
    }

    Moves calculatePawnMoves   (Pos, PieceColor) const noexcept;
    Moves calculateSlidingMoves(const Directions&, Pos, PieceColor) const noexcept;
    Moves calculateRoundMoves  (const RoundDirections&, Pos, PieceColor) const noexcept;

    bool isKingEndangered       (Pos, PieceColor, Pos, Pos) noexcept;
    bool isKingAttacked         (Pos, PieceColor) const noexcept;
    bool isKingAttackedByPawn   (Pos, PieceColor) const noexcept;
    bool isKingAttackedBySliding(const Directions&, Pos, Piece) const noexcept;
    bool isKingAttackedByRound  (const RoundDirections&, Pos, Piece) const noexcept;

    bool canCastle(Pos, PieceColor, bool) const noexcept;
    bool hasAnyMoves(PieceColor) noexcept;

    struct TeamData {
        Pos kingPos;
        bool QSideCastling;
        bool KSideCastling;
    };
    BoardType m_board;
    TeamData m_blackData;
    TeamData m_whiteData;
    Pos m_enPassantTarget;    
    bool m_enPassantEnabled;
};
#endif
