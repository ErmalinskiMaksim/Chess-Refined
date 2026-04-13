#ifndef GAME_STATE_CACHE_H
#define GAME_STATE_CACHE_H

#include "Pos.h"
#include "GameState.h"

#include <array>
#include <vector>

using BoardType = std::array<Piece, 64>;
using Moves = std::vector<Pos>;

// A comprehensive game state
struct GameStateCache {
    BoardType board;
    GameState state;
};

#endif
