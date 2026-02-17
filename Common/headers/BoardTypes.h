#ifndef BOARD_TYPES_H
#define BOARD_TYPES_H

#include "Pos.h"
#include "Piece.h"
#include <array>
#include <vector>

using BoardType = std::array<Piece, 64>;
using Moves = std::vector<Pos>;

#endif
