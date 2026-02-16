#ifndef PACKET_SERIALIZER_H
#define PACKET_SERIALIZER_H

#include "Common.h"
#include <span>
#include <print>

using StreamType = std::vector<uint8_t>;
using StreamTypeView = std::span<uint8_t>;
class PacketSerializer {
public:
    static StreamType serialize(Request::ReqType type) {
        std::println("Serializing type: {}", static_cast<uint8_t>(type));
        StreamType buff(2);
        buff[0] = 1;
        buff[1] = static_cast<uint8_t>(type);
        return buff;
    }

    static StreamType serialize(uint8_t data, Request::ReqType type) {
        std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(type) + 2;
        StreamType buff(sz);
        buff[0] = sz - 1;
        buff[1] = static_cast<uint8_t>(type);
        buff[2] = data;
        return buff;
    }

    static StreamType serialize(uint8_t flag, Pos pos, Request::ReqType type) {
        std::println("Serializing type: {}", static_cast<uint8_t>(type));
        uint8_t sz = sizeof(type) + sizeof(pos) + 2;
        StreamType buff(sz);
        buff[0] = sz - 1;
        buff[1] = static_cast<uint8_t>(type);
        buff[2] = flag;
        buff[3] = static_cast<uint8_t>(pos.x);
        buff[4] = static_cast<uint8_t>(pos.y);
        return buff;
    }

    static StreamType serialize(const Moves& moves, Request::ReqType type) {
        std::println("Serializing moves");
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

    static StreamType serialize(const ChessClientCache& cache, Request::ReqType type) {
        std::println("Serializing cache");
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

    static Request deserialize(StreamTypeView data) {
        std::println("Deserializing type: {}, len: {}", data[0], data.size());
        using RT = Request::ReqType;
        // [len]|[req_type]|[payload]
        auto type = static_cast<RT>(data[0]); 
        switch(type) {
            case RT::CALCULATE_MOVES:
                if (data.size() != 3) return parseShutDown();
                return { 
                    .moveFrom = Pos{static_cast<int8_t>(data[1]), static_cast<int8_t>(data[2])}
                    , .type = type
                }; 
            case RT::COMMIT_MOVE:
                if (data.size()!= 5) return parseShutDown();
                else return { 
                    .moveFrom = Pos{static_cast<int8_t>(data[1]), static_cast<int8_t>(data[2])}
                    , .moveTo = Pos{static_cast<int8_t>(data[3]), static_cast<int8_t>(data[4])}
                    , .type = type
                };
            case RT::PROMOTE:
                if (data.size() != 4) return parseShutDown();
                else return { 
                    .moveFrom = Pos{static_cast<int8_t>(data[1]), static_cast<int8_t>(data[2])}
                    , .promotion = static_cast<PieceType>(data[3])
                    , .type = type
                };
            case RT::RESTART: [[fallthrough]];
            case RT::GET_CACHE: 
                if (data.size() != 1) return parseShutDown();
                else return { .type = type };
            default: return parseShutDown();
        }
    }

    static Request parseShutDown() {
        return Request{.type = Request::ReqType::SHUT_DOWN};
    }
};

#endif
