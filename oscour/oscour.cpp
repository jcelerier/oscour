#include "oscour.hpp"

int main()
{
  oscour::udp_server s([](oscour::span f) {
    if(oscour::is_bundle(f))
    {

    }
    else
    {
      oscour::message_view m(f);


    }
  }, 1234);
  oscour::tcp_server t([](auto f) {}, 1234);
  ::unlink("/tmp/foo");
  oscour::unix_server u([](auto f) {}, "/tmp/foo");
  while (true)
    ;
}
