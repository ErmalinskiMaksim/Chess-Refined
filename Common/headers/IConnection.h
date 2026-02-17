#ifndef ICONNECTION_H
#define ICONNECTION_H

#include <vector>
#include <span>
#include <cstdint>

using StreamType = std::vector<uint8_t>;
using StreamTypeView = std::span<uint8_t>;

class IConnection {
public:
    virtual void close() = 0;
    virtual void send(StreamType) = 0;
    virtual ~IConnection() = default;
};

#endif
