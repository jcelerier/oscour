#include "oscour.hpp"
#include <iostream>
#include <type_traits>
#include <oscour/ostream.hpp>


int main()
{
  oscour::osc_strict_receiver recv;
  recv.on_message("/foo/bar", [] (int f, int c) {
    std::cerr << "case 1: " << f << " " << c << std::endl;
  });
  recv.on_message("/foo/bar", [] (float f, float c) {
    std::cerr << "case 2: " << f << " " << c << std::endl;
  });
  recv.on_message("/foo/bar", [] (float f, oscour::symbol c) {
    std::cerr << "case 3: " << f << " " << c << std::endl;
  });
  recv.on_message("/foo/baa", [] (const oscour::message_view&) {
  });

  oscour::osc_lax_receiver recv2;
  recv2.on_message("/foo/bar", [] (int f, int c) {
    std::cerr << "case 1: " << f << " " << c << std::endl;
  });
  recv2.on_message("/foo/bar", [] (float f, float c) {
    std::cerr << "case 2: " << f << " " << c << std::endl;
  });
  recv2.on_message("/foo/bar", [] (float f, std::string c) {
    std::cerr << "case 3: " << f << " " << c << std::endl;
  });
  recv2.on_message("/foo/baa", [] (const oscour::message_view& m) {
    std::cerr << m.address_pattern().data() << std::endl;
  });

  oscour::udp_server s([&] (auto m) {
    std::cerr << "strict: \n";
    recv(m);
    std::cerr << "lax: \n";
    recv2(m);
  } , 1234);

  oscour::sync_runner r{s};
  r.run();

  oscour::tcp_server t([](auto f) {}, 5678);
  ::unlink("/tmp/foo");
  oscour::unix_server u([](auto f) {}, "/tmp/foo");
  while (true)
    ;
}
