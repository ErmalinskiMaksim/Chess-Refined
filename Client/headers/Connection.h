#ifndef CONNECTION_H
#define CONNECTION_H

#include "boost/asio.hpp"
#include <deque>
#include <memory>
#include "IConnection.h"

class ChessClient;
class Connection;

using Context = boost::asio::io_context;
using Executor = boost::asio::executor_work_guard<Context::executor_type>;
using SocketType = boost::asio::local::stream_protocol::socket;

using ConnectionPtr = std::shared_ptr<Connection>;
using ClientView = std::reference_wrapper<ChessClient>;

class Connection : public IConnection
                 , public std::enable_shared_from_this<Connection> {
public:
    Connection(Context&, ClientView);

    void start(std::string_view);
    void send(StreamType) override;
    void close() override;
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
