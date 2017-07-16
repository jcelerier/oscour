#pragma once
#include <oscour/utils.hpp>
#include <experimental/filesystem>
#include <asio.hpp>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
namespace oscour
{
using namespace asio;
using namespace asio::ip;

class unix_client
{
public:
  unix_client(std::string endpoint)
      : m_endpoint{std::move(endpoint)}
      , m_socket{m_service}
  {
    m_socket.open();
  }

  void send(span s)
  {
    m_socket.send_to(asio::buffer(s.data(), s.size()), m_endpoint);
  }

private:
  io_service m_service;

  local::datagram_protocol::endpoint m_endpoint;
  local::datagram_protocol::socket m_socket;
};

struct unix_endpoint_cleaner
{
  unix_endpoint_cleaner(const std::string& ep)
  {
    auto fd = ::open(ep.c_str(), O_RDONLY);
    if(fd == -1)
    {
      ::unlink(ep.c_str());
    }
    else
    {
      struct stat statbuf;
      ::fstat(fd, &statbuf);
      if(S_ISSOCK(statbuf.st_mode))
      {
        ::close(fd);
        ::unlink(ep.c_str());
      }
    }
  }
};

template <typename OnMessage>
class unix_server: unix_endpoint_cleaner
{
public:
  unix_server(OnMessage m, const std::string& endpoint)
      : unix_endpoint_cleaner{endpoint}
      , m_endpoint{endpoint}
      , m_socket{m_service, m_endpoint}
      , m_cb{std::move(m)}
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
            m_cb(oscour::span{m_buf.data(), gsl::narrow_cast<std::ptrdiff_t>(m_buf.size())});
            start_receive();
          }
        });
  }
  io_service m_service;

  local::datagram_protocol::endpoint m_endpoint;
  local::datagram_protocol::socket m_socket;
  std::array<char, 16384> m_buf;
  OnMessage m_cb;
};
}
