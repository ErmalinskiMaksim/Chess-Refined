#ifndef SERIALIZER_H
#define SERIALIZER_H

// #include <print>
#include "GameStateCache.h"
#include "IConnection.h"

// Request tags exchanged between client and server
enum class ReqType : uint8_t {
      CALCULATE_MOVES   = 0x00
    , COMMIT_MOVE       = 0x01
    , PROMOTE           = 0x02
    , RESTART           = 0x03
    , SHUT_DOWN         = 0x04
    , ASSIGN_COLOR      = 0x05
    , PLAYER_DISCONNECT = 0x06
};

// A data format that the client/server expect from the connection
struct Packet {
    StreamTypeView payload;
    ReqType type;
};

// Main data serializer-deserializer.
// * Fulfils the role of an adapter between the client/server and connection
class Serializer {
public:
    static Packet deserialize(StreamTypeView data) {
        auto type = static_cast<ReqType>(data[0]);
        // std::println("Deserializing type: {}", data[0]);
        if (data.size() == 1) return {.payload = {}, .type = type};
        else return { .payload = {data.data() + 1, data.size() - 1}, .type = type};
    }

    static StreamType serialize(Pos pos, ReqType type) {
        // std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(pos) + sizeof(type) + 1;
        StreamType stream(sz);
        stream[0] = sz - 1;
        stream[1] = static_cast<uint8_t>(type);
        stream[2] = static_cast<uint8_t>(pos.x);
        stream[3] = static_cast<uint8_t>(pos.y);
        return stream;
    }
    static StreamType serialize(Pos pos, Pos pos2, ReqType type) {
        // std::println("Serializing type: {}", static_cast<uint8_t>(type));
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
    static StreamType serialize(uint8_t data, Pos pos, ReqType type) {
        // std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(pos) + sizeof(data) + sizeof(type) + 1;
        StreamType stream(sz);
        stream[0] = sz - 1;
        stream[1] = static_cast<uint8_t>(type);
        stream[2] = static_cast<uint8_t>(data);
        stream[3] = static_cast<uint8_t>(pos.x);
        stream[4] = static_cast<uint8_t>(pos.y);
        return stream;
    }

    static StreamType serialize(ReqType type) {
        // std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(type) + 1;
        StreamType stream(sz);
        stream[0] = sz - 1;
        stream[1] = static_cast<uint8_t>(type);
        return stream;
    }

    static StreamType serialize(uint8_t data, ReqType type) {
        // std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(type) + 2;
        StreamType buff(sz);
        buff[0] = sz - 1;
        buff[1] = static_cast<uint8_t>(type);
        buff[2] = data;
        return buff;
    }

    static StreamType serialize(const Moves& moves, ReqType type) {
        // std::println("Serializing moves");
        // maximum amount of moves: ~28. Max length = ~28 * 2 = ~56
        auto sz = static_cast<uint8_t>(moves.size() * sizeof(moves[0]) + sizeof(type) + 1);
        StreamType buff(sz);
        buff[0] = sz - 1;
        buff[1] = static_cast<uint8_t>(type);
        auto i = 2uz;
        for (auto&& move : moves) {
            buff[i++] = static_cast<uint8_t>(move.x);
            buff[i++] = static_cast<uint8_t>(move.y);
        }
        return buff;
    }

    static StreamType serialize(const GameStateCache& cache, ReqType type) {
        // std::println("Serializing cache");
        // board = 64 *sizeof(Piece) = 128 bytes; state = 4 bytes
        auto sz = static_cast<uint8_t>(
                cache.board.size() * sizeof(cache.board[0]) + sizeof(cache.state) + sizeof(type)+  1);
        StreamType buff(sz);
        buff[0] = sz - 1;
        buff[1] = static_cast<uint8_t>(type);
        auto i = 2uz;
        for (auto&& piece : cache.board) {
            buff[i++] = static_cast<uint8_t>(piece.type);
            buff[i++] = static_cast<uint8_t>(piece.col);
        }
        buff[i++] = cache.state.score.white;
        buff[i++] = cache.state.score.black;
        buff[i++] = static_cast<uint8_t>(cache.state.flag);
        buff[i++] = static_cast<uint8_t>(cache.state.currentTeam);
        return buff;
    }
};

#endif
