#pragma once
#include <asio.hpp>
namespace oscour
{
using namespace asio;
using namespace asio::ip;

class unix_client
{
};

template <typename OnMessage>
class unix_server
{
public:
  unix_server(OnMessage m, std::string endpoint)
      : m_endpoint{std::move(endpoint)}
      , m_acceptor{m_service, m_endpoint}
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
  local::stream_protocol::endpoint m_endpoint;
  local::stream_protocol::acceptor m_acceptor;
  local::stream_protocol::socket m_socket;
  std::array<char, 16384> m_buf;
  OnMessage m_cb;
};
}
