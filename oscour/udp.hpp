#pragma once
#include <oscour/utils.hpp>
#include <asio.hpp>
namespace oscour
{
using namespace asio;
using namespace asio::ip;

class udp_client
{
public:
  udp_client(std::string host, uint16_t port)
      : m_resolver{m_service}
      , m_query{udp::v4(), host, std::to_string(port)}
      , m_endpoint{*m_resolver.resolve(m_query)}
      , m_socket{m_service}
  {
    m_socket.open(udp::v4());
  }

  void send(span s)
  {
    m_socket.send_to(asio::buffer(s.data(), s.size()), m_endpoint);
  }

private:
  io_service m_service;
  udp::resolver m_resolver;
  udp::resolver::query m_query;

  udp::endpoint m_endpoint;
  udp::socket m_socket;
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
