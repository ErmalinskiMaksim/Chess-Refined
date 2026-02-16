#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <cstdint>
#include <array>

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

struct Pos {
    bool operator==(Pos other) const noexcept {
        return (other.x == x && other.y == y);
    }
    int8_t x;
    int8_t y;
};

inline Pos operator+(Pos p1, Pos p2) {
    return Pos(p1.x + p2.x, p1.y + p2.y);
}

using BoardType = std::array<Piece, 64>;
using Moves = std::vector<Pos>;

enum class GameFlags : uint8_t {
    NONE = 0x10, STALE_MATE = 0x20, CHECK_MATE = 0x30, PROMOTION = 0x40 
};

struct Score {
    uint8_t white; uint8_t black;
};

struct GameState {
    Score score;
    GameFlags flag;
    PieceColor currentTeam;
};

struct ChessClientCache {
    BoardType board;
    GameState state;
};

struct Request {
    enum class ReqType : uint8_t {
          CALCULATE_MOVES   = 0x00
        , COMMIT_MOVE       = 0x01
        , PROMOTE           = 0x02
        , RESTART           = 0x03
        , GET_CACHE         = 0x04
        , SHUT_DOWN         = 0x05
        , ASSIGN_COLOR      = 0x06
        , PLAYER_DISCONNECT = 0x07
    };
    Pos moveFrom = {};
    Pos moveTo = {};
    PieceType promotion = PieceType::NONE;
    ReqType type;
};
#endif
