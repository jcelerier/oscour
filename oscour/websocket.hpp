#pragma once
#include <oscour/utils.hpp>
#include <uWS/WebSocket.h>
#include <uWS/Hub.h>

namespace oscour
{
class ws_client
{
public:
  ws_client(std::string host, uint16_t port)
  {
  }

  void send(span s)
  {
  }

private:
};

template <typename OnMessage>
class ws_server
{
public:
  ws_server(OnMessage m, uint16_t port)
    : m_cb{std::move(m)}
  {
    h.onMessage([this] (uWS::WebSocket<uWS::SERVER>*, char *message, size_t length, uWS::OpCode) {
      m_cb({message, gsl::narrow_cast<std::ptrdiff_t>(length)});
    });

    h.listen(port);
  }

  auto service()
  {
    struct runner {
      uWS::Hub& h;
      void run() { h.run(); }
      void stop() { h.getDefaultGroup<uWS::SERVER>().close(); }
    };

    return runner{h};
  }

private:
  uWS::Hub h;
  OnMessage m_cb;
};
}
