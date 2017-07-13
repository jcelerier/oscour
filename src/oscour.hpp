#pragma once
#include <asio.hpp>
namespace oscour
{
using namespace asio;
using namespace asio::ip;


class message
{
    template<typename Buf>
    message(const Buf& buffer)
    {

    }
};

class udp_client
{

};

template<typename OnMessage>
class udp_server
{
public:
    udp_server(OnMessage m, uint16_t port)
        : m_socket{m_service, udp::endpoint{udp::v4(), port}}
        , m_cb{std::move(m)}
    {
        start_receive();
    }

private:
    void start_receive()
    {
        m_socket.async_receive_from(
                    buffer(m_buf), m_endpoint, [=] (
                    const asio::error_code& error,
                    std::size_t size) {
            if (!error || error == error::message_size)
            {
                m_cb(m_buf);
                start_receive();
            }
        });
    }

    io_service m_service;
    udp::socket m_socket;
    udp::endpoint m_endpoint;
    std::array<char, 16384> m_buf;
    OnMessage m_cb;
};

class tcp_client
{

};


template<typename OnMessage>
class tcp_server
{
public:
    tcp_server(OnMessage m, uint16_t port)
        : m_acceptor{m_service, tcp::endpoint{tcp::v4(), port}}
        , m_socket{m_service}
        , m_cb{std::move(m)}
    {
        m_acceptor.async_accept(
                    m_socket,
                    [this] (const asio::error_code& ec) {
            if (!ec)
            {
                start_receive();
            }
        });
    }

private:
    void start_receive()
    {
        m_socket.async_receive(
                    buffer(m_buf),
                    [=] (const asio::error_code& error,
                         std::size_t size) {
            if (!error || error == error::message_size)
            {
                m_cb(m_buf);
                start_receive();
            }
        });
    }

    io_service m_service;
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    std::array<char, 16384> m_buf;
    OnMessage m_cb;
};


class unix_client
{

};


template<typename OnMessage>
class unix_server
{
public:
    unix_server (OnMessage m, std::string endpoint)
        : m_endpoint{std::move(endpoint)}
        , m_acceptor{m_service, m_endpoint}
        , m_socket{m_service}
        , m_cb{std::move(m)}
    {
        m_acceptor.async_accept(
                    m_socket,
                    [this] (const asio::error_code& ec) {
            if (!ec)
            {
                start_receive();
            }
        });
    }

private:
    void start_receive()
    {
        m_socket.async_receive(
                    buffer(m_buf),
                    [=] (const asio::error_code& error,
                         std::size_t size) {
            if (!error || error == error::message_size)
            {
                m_cb(m_buf);
                start_receive();
            }
        });
    }
    io_service m_service;
    local::stream_protocol::endpoint m_endpoint;
    local::stream_protocol::acceptor m_acceptor;
    local::stream_protocol::socket m_socket;
    std::array<char, 16384> m_buf;
    OnMessage m_cb;
};

class serial_client
{

};

class serial_server
{

};
}
