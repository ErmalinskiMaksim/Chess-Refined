#ifndef PIECE_H
#define PIECE_H

#include <cstdint>

struct Piece {
    enum class Type : uint8_t {
        NONE, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING
    };
    enum class Color : uint8_t {
        BLACK = 0x00, WHITE = 0x01
    };

    Type type;
    Color col;
};

#endif // !PIECE_H
