#pragma once
#include <oscour/utils.hpp>
#include <oscour/message_buffer.hpp>
#include <asio.hpp>
#include <iostream>
namespace oscour
{
using namespace asio;
using namespace asio::ip;

class tcp_client
{
public:
  tcp_client(std::string ip, uint16_t port)
  {
    asio::connect(m_socket, m_resolver.resolve({ip, std::to_string(port)}));
  }

  void send(const oscour::message<Any>& m)
  {
    span sp{m};

    char n[8];
    to_net(n, sp.size());

    m_socket.write_some(asio::buffer(n, 8));
    m_socket.write_some(asio::buffer(sp.data(), sp.size()));
  }

  void send(const oscour::bundle& m)
  {

  }

  io_service m_service;

  tcp::socket m_socket{m_service};
  tcp::resolver m_resolver{m_service};
};

template <typename OnMessage>
class tcp_server
{
public:
  tcp_server(OnMessage m, uint16_t port)
      : m_acceptor{m_service, tcp::endpoint{tcp::v4(), port}}
      , m_socket{m_service}
      , m_cb{std::move(m)}
  {
    m_acceptor.async_accept(m_socket, [this] (const asio::error_code& ec) {
      if (!ec)
      {
        start_receive();
      }
    });
  }

  auto& service() { return m_service; }

private:
  void start_receive()
  {
    asio::async_read(
          m_socket, m_buf, asio::transfer_exactly(8),
          [=](const asio::error_code& error, std::size_t size) {
      if (!error || error == error::message_size)
      {
        const char* data = asio::buffer_cast<const char*>(m_buf.data());
        continue_receive(from_net<std::ptrdiff_t>(data));
      }
    });
  }

  void continue_receive(std::ptrdiff_t size)
  {
    m_buf.consume(size);
    asio::async_read(
          m_socket, m_buf, asio::transfer_exactly(size),
          [=](const asio::error_code& error, std::size_t size) {
      if (!error || error == error::message_size)
      {
        const char* data = asio::buffer_cast<const char*>(m_buf.data());
        m_cb(span{data, gsl::narrow_cast<std::ptrdiff_t>(m_buf.size())});
        m_buf.consume(size);
        start_receive();
      }
    });
  }

  io_service m_service;
  tcp::acceptor m_acceptor;
  tcp::socket m_socket;
  asio::streambuf m_buf;
  OnMessage m_cb;
};
}
