#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "Piece.h"

struct Score {
    uint8_t white; uint8_t black;
};

struct GameState {
    enum class Flags : uint8_t {
        NONE = 0x10, STALE_MATE = 0x20, CHECK_MATE = 0x30, PROMOTION = 0x40 
    };

    Score score;
    Flags flag;
    Piece::Color currentTeam;
};

#endif
