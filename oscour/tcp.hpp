#pragma once
#include <asio.hpp>
namespace oscour
{
using namespace asio;
using namespace asio::ip;

class tcp_client
{
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
    m_acceptor.async_accept(m_socket, [this](const asio::error_code& ec) {
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
        buffer(m_buf), [=](const asio::error_code& error, std::size_t size) {
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
}
