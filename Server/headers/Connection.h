#ifndef CONNECTION_H
#define CONNECTION_H

#include "boost/asio.hpp"
#include <deque>
#include "PacketSerializer.h"

class ChessServer;
class Connection;

using SocketType = boost::asio::local::stream_protocol::socket;
using ConnectionPtr = std::shared_ptr<Connection>;
using ServerView = std::reference_wrapper<ChessServer>;

class Connection : public std::enable_shared_from_this<Connection> {
   // asio::ip::tcp::socket;
public:
    explicit Connection(SocketType, ServerView);

    void send(StreamType);
    void start();
    void disconnect();
private:
    void write();
    void readHeader();
    void readPayload(uint8_t);

    SocketType m_socket;
    boost::asio::strand<typename SocketType::executor_type>
    m_strand;
    std::deque<StreamType> m_sendQueue;  
    uint8_t m_payloadBuffer[6];
    ServerView r_server;
    uint8_t m_headerBuffer;
    bool m_disconnected = false;
};

#endif
