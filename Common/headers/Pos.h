#ifndef POS_H
#define POS_H

#include <cstdint>

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

#endif
