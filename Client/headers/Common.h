#ifndef COMMON_H
#define COMMON_H

#include <vector>

#include "Piece.h"
#include "Pos.h"

using Moves = std::vector<Pos>;

struct GameState {
    enum class Flags : uint8_t {
        NONE = 0x10, STALE_MATE = 0x20, CHECK_MATE = 0x30, PROMOTION = 0x40 
    };

    struct Score {
        uint8_t white; uint8_t black;
    };

    Score score;
    Flags flag;
    PieceColor currentTeam;
};

#endif
