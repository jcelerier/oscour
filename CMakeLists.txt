cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(oscour)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-std=c++17")
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
  oscour/oscour.hpp
  oscour/oscour.cpp)
# set_target_properties(oscour PROPERTIES CXX_STANDARD 17)
