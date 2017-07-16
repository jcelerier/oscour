#include "oscour.hpp"
#include <chrono>
#include <iostream>
#include <oscour/ostream.hpp>
#include <oscour/outbound_stream.hpp>
#include <type_traits>
#include <oscour/message_buffer.hpp>
int main()
{
  oscour::osc_strict_receiver recv;
  recv.on_message("/foo/bar", [](int f, int c) {
    std::cerr << "case 1: " << f << " " << c << std::endl;
  });
  recv.on_message("/foo/bar", [](float f, float c) {
    std::cerr << "case 2: " << f << " " << c << std::endl;
  });
  recv.on_message("/foo/bar", [](float f, oscour::string c) {
    std::cerr << "case 3: " << f << " " << c << std::endl;
  });
  recv.on_message("/foo/baa", [](const oscour::message_view&) {});

  oscour::osc_lax_receiver recv2;
  recv2.on_message("/foo/bar", [](int f, int c) {
    std::cerr << "case 1: " << f << " " << c << std::endl;
  });
  recv2.on_message("/foo/bar", [](float f, float c) {
    std::cerr << "case 2: " << f << " " << c << std::endl;
  });
  recv2.on_message("/foo/bar", [](float f, std::string c) {
    std::cerr << "case 3: " << f << " " << c << std::endl;
  });
  recv2.on_message("/foo/baa", [](const oscour::message_view& m) {
    std::cerr << m.address_pattern().data() << std::endl;
  });

  oscour::udp_server s(
      [&](auto m) {
        std::cerr << "strict: \n";
        recv(m);
        std::cerr << "lax: \n";
        recv2(m);
      },
      1234);

  oscour::async_runner r{s};
  r.run();

  oscour::udp_client clt{"localhost", 1234};
  while (true)
  {
    using namespace std::literals;
    std::this_thread::sleep_for(500ms);

    // Raw
    clt.send("/foo/bar\0\0\0\0,ii\0\0\0\0\1\0\0\0\2"s);

    // Manual stream
    char buf[64] = { 0 };
    oscour::outbound_stream stream(buf, 64);
    stream << oscour::begin_message{"/foo/bar"}
           << 1
           << 2
           << oscour::end_message{};
    clt.send({stream.data(), gsl::narrow_cast<std::ptrdiff_t>(stream.size())});

    // Simple
    clt.send(oscour::message{"/foo/bar", 1234.f, "fooo"});
  }

  oscour::tcp_server t([](auto f) {}, 5678);
  ::unlink("/tmp/foo");
  oscour::unix_server u([](auto f) {}, "/tmp/foo");
  while (true)
    ;
}
