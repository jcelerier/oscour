#include <oscour/oscour.hpp>
#include <chrono>
#include <iostream>

int main()
{
  // Setup a server
  oscour::osc_lax_receiver recv;
  recv.on_message("/foo/bar", [] (float f, std::string_view c) {
    std::cerr << f << " " << c << std::endl;
  });

  oscour::udp_server s{recv, 1234};
  oscour::async_runner r{s};
  r.run();

  // Setup a client
  oscour::udp_client clt{"localhost", 1234};

  // Send stuff to the client
  for(int i = 0; i < 5; i++)
  {
    using namespace std::literals;
    std::this_thread::sleep_for(500ms);

    clt.send(oscour::message{"/foo/bar", float(i), "fooo"});
  }
}
