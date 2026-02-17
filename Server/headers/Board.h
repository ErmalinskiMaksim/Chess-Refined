#ifndef BOARD_H
#define BOARD_H

#include "BoardTypes.h"

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

    uint8_t commitMove(Pos, Pos, Piece::Color) noexcept;
    Moves calculatePieceMoves(Pos, Piece::Color) noexcept;
    void promote(Pos, Piece::Type) noexcept;

private:
    static inline Pos posAhead(Pos p, Piece::Color col) noexcept {
        return Pos(p.x, static_cast<int8_t>(p.y + 2*static_cast<int8_t>(col) - 1));
    }
    static inline bool isEmpty(Piece p) noexcept {
        return p.type == Piece::Type::NONE;
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

    Moves calculatePawnMoves   (Pos, Piece::Color) const noexcept;
    Moves calculateSlidingMoves(const Directions&, Pos, Piece::Color) const noexcept;
    Moves calculateRoundMoves  (const RoundDirections&, Pos, Piece::Color) const noexcept;

    bool isKingEndangered       (Pos, Piece::Color, Pos, Pos) noexcept;
    bool isKingAttacked         (Pos, Piece::Color) const noexcept;
    bool isKingAttackedByPawn   (Pos, Piece::Color) const noexcept;
    bool isKingAttackedBySliding(const Directions&, Pos, Piece) const noexcept;
    bool isKingAttackedByRound  (const RoundDirections&, Pos, Piece) const noexcept;

    bool canCastle(Pos, Piece::Color, bool) const noexcept;
    bool hasAnyMoves(Piece::Color) noexcept;

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
