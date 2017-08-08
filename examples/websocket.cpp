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

  oscour::ws_server s{recv, 1234};
  oscour::async_runner r{s};
  r.run();

  while(true);
}
