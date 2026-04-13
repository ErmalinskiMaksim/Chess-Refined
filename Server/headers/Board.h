#ifndef BOARD_H
#define BOARD_H

#include "GameStateCache.h"

class Board {
    friend consteval BoardType initBoard();
public:
    using Directions = const std::array<Pos, 4>;
    using RoundDirections = const std::array<Pos, 8>;

    Board();

    // return a copy of a board
    inline BoardType get() const noexcept {
        return m_board;
    }

    // commit a move and return a score of the taken piece
    uint8_t commitMove(Pos, Pos, Piece::Color) noexcept;
    // calculate a list  of available moves for a piece 
    // with a given position and color
    Moves calculatePieceMoves(Pos, Piece::Color) noexcept;
    // promote a piece
    void promote(Pos, Piece::Type) noexcept;

private:
    // convert a board index to a logic board position
    static inline Pos indexToPos(size_t idx) noexcept {
        return Pos(idx % 8, static_cast<int8_t>(idx) / 8);
    } 
    // convert a logical board position to the board array index
    static inline size_t posToIndex(Pos pos) noexcept {
        return static_cast<size_t>(pos.x + pos.y*8);
    }
    // find a position ahead of a piece with a given position and color
    static inline Pos posAhead(Pos p, Piece::Color col) noexcept {
        return Pos(p.x, static_cast<int8_t>(p.y + 2*static_cast<int8_t>(col) - 1));
    }
    // check if a piece from a precomputed position exists
    static inline bool isEmpty(Piece p) noexcept {
        return p.type == Piece::Type::NONE;
    }
    // check if a move is in legal numeric boundaries
    static inline bool inLegalBounds(Pos moveTo) noexcept {
        return (moveTo.x >= 0 && moveTo.x <= 7 && moveTo.y >= 0 && moveTo.y <= 7);
    }
    // get a copy of a piece at a position
    inline Piece at(Pos pos) const noexcept {
        return m_board[posToIndex(pos)];
    }

    // move calculation 
    Moves calculatePawnMoves   (Pos, Piece::Color) const noexcept;
    Moves calculateSlidingMoves(const Directions&, Pos, Piece::Color) const noexcept;
    Moves calculateRoundMoves  (const RoundDirections&, Pos, Piece::Color) const noexcept;

    // king security checks
    bool isKingEndangered       (Pos, Piece::Color, Pos, Pos) noexcept;
    bool isKingAttacked         (Pos, Piece::Color) const noexcept;
    bool isKingAttackedByPawn   (Pos, Piece::Color) const noexcept;
    bool isKingAttackedBySliding(const Directions&, Pos, Piece) const noexcept;
    bool isKingAttackedByRound  (const RoundDirections&, Pos, Piece) const noexcept;

    // castling
    bool canCastle(Pos, Piece::Color, bool) const noexcept;

    // check if a team with of a specified color has any moves left
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
