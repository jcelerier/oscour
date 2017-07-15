#pragma once
#include <asio.hpp>
#include <thread>
namespace oscour
{
using namespace asio;
using namespace asio::ip;

class udp_client
{
};

template <typename OnMessage>
class udp_server
{
public:
  udp_server(OnMessage m, uint16_t port)
      : m_socket{m_service, udp::endpoint{udp::v4(), port}}, m_cb{std::move(m)}
  {
    start_receive();
  }

  io_service& service()
  {
    return m_service;
  }

private:
  void start_receive()
  {
    m_socket.async_receive_from(
        buffer(m_buf), m_endpoint,
        [=](const asio::error_code& error, std::size_t size) {
          if (!error || error == error::message_size)
          {
            m_cb(span(m_buf.data(), size));
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
}
