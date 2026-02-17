#include "Connection.h"
#include "ChessClient.h"
#include "Serializer.h"

Connection::Connection(Context& io, ClientView client)
    : m_socket{io}
    , r_client{client}
{}

void Connection::start(std::string_view path) {
    boost::asio::local::stream_protocol::endpoint ep(path);
    m_socket.async_connect(ep,
        [self = shared_from_this()](auto ec) {
            if (!ec) { 
                self->readHeader();
            }
        });
}

void Connection::send(StreamType data) {
    boost::asio::post(m_socket.get_executor(),
        [self = shared_from_this(), data = std::move(data)]() mutable {
            bool writing = !self->m_sendQueue.empty();
            self->m_sendQueue.push_back(std::move(data));

            if (!writing)
                self->write();
        });
}

void Connection::close() {

}

void Connection::readHeader() {
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(&m_headerBuffer, sizeof(m_headerBuffer)),
        [self = shared_from_this()](auto ec, size_t) {
            if (!ec)
                self->readPayload(self->m_headerBuffer);
        });
}

void Connection::readPayload(uint8_t len) {
    if (len > sizeof(m_payloadBuffer)) return;

    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_payloadBuffer, len),
        [self = shared_from_this(), len](auto ec, size_t) {
            if (!ec) {
                self->r_client.get().onPacket(Serializer::deserialize(
                        { self->m_payloadBuffer, len }));

                self->readHeader();
            }
        });
}

void Connection::write() {
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_sendQueue.front()),
        [self = shared_from_this()](auto ec, size_t) {
            if (!ec) {
                self->m_sendQueue.pop_front();
                if (!self->m_sendQueue.empty())
                    self->write();
            }
        });
}
