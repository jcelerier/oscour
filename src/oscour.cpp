#include "oscour.hpp"

int main()
{
    oscour::udp_server s([] (auto f) { }, 1234);
    oscour::tcp_server t([] (auto f) { }, 1234);
    ::unlink("/tmp/foo");
    oscour::unix_server u([] (auto f) { }, "/tmp/foo");
    while(true);

}
