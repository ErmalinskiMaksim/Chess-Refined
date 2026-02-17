#ifndef CONNECTION_H
#define CONNECTION_H

#include "boost/asio.hpp"
#include <deque>
#include <memory>
#include "IConnection.h"

class ChessServer;
class Connection;

using Context = boost::asio::io_context;
// UNIX
// using Acceptor = boost::asio::local::stream_protocol::acceptor;
// using Endpoint = boost::asio::local::stream_protocol::endpoint;
// using Socket = boost::asio::local::stream_protocol::socket;
// TCP
using Acceptor = boost::asio::ip::tcp::acceptor;
using Socket = boost::asio::ip::tcp::socket;
using Endpoint  = boost::asio::ip::tcp::endpoint;
using Strand = boost::asio::strand<Socket::executor_type>;

using ConnectionPtr = std::shared_ptr<Connection>;
using ServerView = std::reference_wrapper<ChessServer>;

class Connection : public IConnection 
                 , public std::enable_shared_from_this<Connection> {
public:
    explicit Connection(Socket, ServerView);

    void start();
    void send(StreamType) override;
    void close() override;
private:
    void write();
    void readHeader();
    void readPayload(uint8_t);

    Socket m_socket;
    Strand m_strand;
    std::deque<StreamType> m_sendQueue;  
    ServerView r_server;

    uint8_t m_payloadBuffer[6];
    uint8_t m_headerBuffer;
    bool m_disconnected = false;
};

#endif
