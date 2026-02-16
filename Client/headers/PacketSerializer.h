#ifndef PACKET_SERIALIZER_H
#define PACKET_SERIALIZER_H

#include "Common.h"
#include <print>
#include <span>

using StreamType = std::vector<uint8_t>;
using StreamTypeView = std::span<uint8_t>;

enum class ReqType : uint8_t {
    CALCULATE_MOVES     = 0x00
    , COMMIT_MOVE       = 0x01
    , PROMOTE           = 0x02
    , RESTART           = 0x03
    , GET_CACHE         = 0x04
    , SHUT_DOWN         = 0x05
    , ASSIGN_COLOR      = 0x06
    , PLAYER_DISCONNECT = 0x07
};

struct ServerResponse {
    StreamTypeView payload;
    ReqType type;
};

class Serializer {
public:
    static ServerResponse deserialize(StreamTypeView data) {
        ReqType type = static_cast<ReqType>(data[0]);
        std::println("Deserializing type: {}", data[0]);
        if (data.size() == 1) return {.payload = {}, .type = type};
        else return { .payload = {data.data() + 1, data.size() - 1}, .type = type};
    }
    static StreamType serialize(Pos pos, ReqType type) {
        std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(pos) + sizeof(type) + 1;
        StreamType stream(sz);
        stream[0] = sz - 1;
        stream[1] = static_cast<uint8_t>(type);
        stream[2] = static_cast<uint8_t>(pos.x);
        stream[3] = static_cast<uint8_t>(pos.y);
        return stream;
    }
    static StreamType serialize(Pos pos, Pos pos2, ReqType type) {
        std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(pos) + sizeof(pos2) + sizeof(type) + 1;
        StreamType stream(sz);
        stream[0] = sz - 1;
        stream[1] = static_cast<uint8_t>(type);
        stream[2] = static_cast<uint8_t>(pos.x);
        stream[3] = static_cast<uint8_t>(pos.y);
        stream[4] = static_cast<uint8_t>(pos2.x);
        stream[5] = static_cast<uint8_t>(pos2.y);
        return stream;
    }
    static StreamType serialize(Pos pos, PieceType ptype, ReqType type) {
        std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(pos) + sizeof(ptype) + sizeof(type) + 1;
        StreamType stream(sz);
        stream[0] = sz - 1;
        stream[1] = static_cast<uint8_t>(type);
        stream[2] = static_cast<uint8_t>(pos.x);
        stream[3] = static_cast<uint8_t>(pos.y);
        stream[4] = static_cast<uint8_t>(ptype);
        return stream;
    }
    static StreamType serialize(ReqType type) {
        std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(type) + 1;
        StreamType stream(sz);
        stream[0] = sz - 1;
        stream[1] = static_cast<uint8_t>(type);
        return stream;
    }
};

#endif
