#ifndef CONNECTION_H
#define CONNECTION_H

#include "PacketSerializer.h"
#include "boost/asio.hpp"
#include <deque>

class Connection;
class ChessClient;

using ContextType = boost::asio::io_context;
using SocketType = boost::asio::local::stream_protocol::socket;
using ConnectionPtr = std::shared_ptr<Connection>;
using ClientView = std::reference_wrapper<ChessClient>;

class Connection : public std::enable_shared_from_this<Connection>{
public:
    Connection(ContextType&, ClientView);

    void connect(std::string_view);
    void send(StreamType&&);
private:
    void readHeader();
    void readPayload(uint8_t);
    void write();

    SocketType m_socket;
    std::deque<StreamType> m_sendQueue;
    ClientView r_client;

    uint8_t m_headerBuffer{};
    uint8_t m_payloadBuffer[134]{};
};

#endif // !CONNECTION_H
