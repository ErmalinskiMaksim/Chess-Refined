#ifndef CONNECTION_H
#define CONNECTION_H

#include "IConnection.h"
#include "boost/asio.hpp"
#include <deque>
#include <memory>

class ChessServer;
class Connection;

using Context   = boost::asio::io_context;
using Acceptor  = boost::asio::ip::tcp::acceptor;
using Socket    = boost::asio::ip::tcp::socket;
using Endpoint  = boost::asio::ip::tcp::endpoint;
using Strand    = boost::asio::strand<Socket::executor_type>;

using ConnectionPtr = std::shared_ptr<Connection>;
using ServerView    = std::reference_wrapper<ChessServer>;

// Implementation of IConnection interface. 
// * Represents a boost::asio TCP connection (doesn't create it but destroys it)
// * Performs asynchronous read() and write() operations to the client
class Connection final : public IConnection 
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
