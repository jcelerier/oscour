cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(oscour)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-std=c++17 -fconcepts")
find_package(Threads REQUIRED)
add_executable(oscour
  oscour/bundle.hpp
  oscour/message.hpp
  oscour/types.hpp
  oscour/traits.hpp
  oscour/utils.hpp
  oscour/exceptions.hpp
  oscour/udp.hpp
  oscour/tcp.hpp
  oscour/unix.hpp
  oscour/serial.hpp
  oscour/websocket.hpp
  oscour/strict_receiver.hpp
  oscour/lax_receiver.hpp
  oscour/outbound_stream.hpp
  oscour/message_buffer.hpp
  oscour/ostream.hpp
  oscour/runner.hpp
  oscour/oscour.hpp
  oscour/oscour.cpp)
target_link_libraries(oscour PRIVATE Threads::Threads)
# set_target_properties(oscour PROPERTIES CXX_STANDARD 17)
