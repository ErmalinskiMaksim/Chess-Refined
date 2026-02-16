#ifndef PIECE_H
#define PIECE_H

#include <cstdint>

enum class PieceType : uint8_t {
    NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING
};

enum class PieceColor : uint8_t {
    BLACK = 0x00, WHITE = 0x01
};

struct Piece {
    PieceType type;
    PieceColor col;
};

#endif
