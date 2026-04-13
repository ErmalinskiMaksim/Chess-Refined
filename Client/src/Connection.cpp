#include "Connection.h"
#include "ChessClient.h"
#include "Serializer.h"
#include <print>

// Initialize the socket without binding it
Connection::Connection(Context& io, ClientView client)
    : m_socket{io}
    , r_client{client}
{}

// Responsibilities:
// * Bind the socket to the server endpoint
// * Try to asynchoronously connect to the server 
// * Upon a successful connection, start reading server responses
void Connection::start(std::string_view ipAddr) {
    Resolver resolver(m_socket.get_executor()); 
    auto endpoints = resolver.resolve(ipAddr, "5000"); 
    boost::asio::async_connect(m_socket, endpoints,         
        [self = shared_from_this()](auto ec, const Endpoint&) {
            if (!ec) { 
                self->m_socket.set_option(boost::asio::ip::tcp::no_delay(true)); 
                self->readHeader();
            } else std::println("failed to connect");
        });
}

// Responsibilities:
// * Start writing to the socket as soon as the writing queue is available
void Connection::send(StreamType data) {
    boost::asio::post(m_socket.get_executor(),
        [self = shared_from_this(), data = std::move(data)]() mutable {
            bool writing = !self->m_sendQueue.empty();
            self->m_sendQueue.push_back(std::move(data));

            if (!writing)
                self->write();
        });
}

// Responsibilities:
// * Notify the server about shutdown 
// * Close the socket
// * Clear bufferes
void Connection::close() {
    boost::system::error_code ec;

    m_socket.shutdown(boost::asio::socket_base::shutdown_both, ec);
    m_socket.close(ec);

    m_sendQueue.clear();
}

// Responsibilities:
// * Asynchoronously read the header of an incoming packet
// * If the header is valid, start reading the payload
void Connection::readHeader() {
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(&m_headerBuffer, sizeof(m_headerBuffer)),
        [self = shared_from_this()](auto ec, size_t) {
            if (!ec)
                self->readPayload(self->m_headerBuffer);
        });
}

// Responsibilities:
// * Asynchoronously read the body of an incoming packet
// * Pass a deserialized packet to the client
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

// Responsibilities:
// * asynchoronously write to the socket
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
