#include "Connection.h"
#include "ChessServer.h"
#include <print>

Connection::Connection(SocketType sock, ServerView svr) 
    : m_socket(std::move(sock))
    , m_strand{m_socket.get_executor()}
    , r_server(svr)
{}

void Connection::start() {
    readHeader();
}

void Connection::send(StreamType stream) {
    boost::asio::post(
        m_strand
        , [self = this->shared_from_this(), data = std::move(stream)]() mutable {
            bool writing = !self->m_sendQueue.empty();
            self->m_sendQueue.push_back(std::move(data));

            if (!writing)
                self->write();
        });
}

void Connection::readHeader() {
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(&m_headerBuffer, sizeof(m_headerBuffer)),
        boost::asio::bind_executor(
            m_strand
            , [self = shared_from_this()](boost::system::error_code ec, size_t) {
                if (!ec) {
                    uint8_t length = self->m_headerBuffer;
                    self->readPayload(length);
                } else self->disconnect();
    }));
}

void Connection::readPayload(uint8_t payloadLen) {
    if (!payloadLen || payloadLen > sizeof(m_payloadBuffer)) {
        disconnect();
        return;
    }

    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_payloadBuffer, payloadLen),
        boost::asio::bind_executor(
            m_strand 
            , [self = shared_from_this(), payloadLen](boost::system::error_code ec, size_t) {
               if (!ec) {
                self->r_server.get().dispatchRequests(self, PacketSerializer::deserialize(
                            {self->m_payloadBuffer, payloadLen}));
                self->readHeader(); 
               } else self->disconnect();
    }));
}

void Connection::write() {
    boost::asio::async_write(
        m_socket
        , boost::asio::buffer(m_sendQueue.front()),
        [self = shared_from_this()](boost::system::error_code ec, size_t)
        {
            if (!ec) {
                self->m_sendQueue.pop_front();
                if (!self->m_sendQueue.empty()) 
                    self->write();
            } else {
                self->disconnect();
            }
        });
}

void Connection::disconnect() {
    if (m_disconnected) return;

    m_disconnected = true;

    boost::system::error_code ignored;
    m_socket.shutdown(boost::asio::socket_base::shutdown_both, ignored);
    m_socket.close(ignored);

    r_server.get().onDisconnect(shared_from_this());
}
