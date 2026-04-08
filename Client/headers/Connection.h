#ifndef CONNECTION_H
#define CONNECTION_H

#include "boost/asio.hpp"
#include <deque>
#include <memory>
#include "IConnection.h"

class ChessClient;
class Connection;

using Context   = boost::asio::io_context;
using Executor  = boost::asio::executor_work_guard<Context::executor_type>;
using Resolver  = boost::asio::ip::tcp::resolver;
using Endpoint  = boost::asio::ip::tcp::endpoint;
using Socket    = boost::asio::ip::tcp::socket;

using ConnectionPtr = std::shared_ptr<Connection>;
using ClientView    = std::reference_wrapper<ChessClient>;

class Connection : public IConnection
                 , public std::enable_shared_from_this<Connection> {
public:
    Connection(Context&, ClientView);

    void start();
    void send(StreamType) override;
    void close() override;
private:
    void readHeader();
    void readPayload(uint8_t);
    void write();

    Socket m_socket;
    std::deque<StreamType> m_sendQueue;
    ClientView r_client;

    uint8_t m_headerBuffer{};
    uint8_t m_payloadBuffer[134]{};
};

#endif // !CONNECTION_H
