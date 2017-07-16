cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(oscour)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-std=c++17 -fconcepts -fsanitize=address -fsanitize=undefined")
set(CMAKE_EXE_LINKER_FLAGS "-std=c++17 -fconcepts -fsanitize=address -fsanitize=undefined")
add_subdirectory(uWebSockets)

find_package(Threads REQUIRED)
add_library(oscour INTERFACE)
target_sources(oscour INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/bundle.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/message.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/types.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/traits.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/utils.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/exceptions.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/udp.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/tcp.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/unix.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/serial.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/websocket.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/strict_receiver.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/lax_receiver.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/outbound_stream.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/message_buffer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/ostream.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/runner.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/oscour/oscour.hpp)

target_include_directories(oscour INTERFACE oscour/)
target_link_libraries(oscour INTERFACE Threads::Threads uWS)

add_executable(udp examples/udp.cpp)
target_link_libraries(udp PRIVATE oscour)

add_executable(tcp examples/tcp.cpp)
target_link_libraries(tcp PRIVATE oscour)

add_executable(serial examples/serial.cpp)
target_link_libraries(serial PRIVATE oscour)

add_executable(unix examples/unix.cpp)
target_link_libraries(unix PRIVATE oscour)

add_executable(websocket examples/websocket.cpp)
target_link_libraries(websocket PRIVATE oscour)

add_executable(complete examples/complete.cpp)
target_link_libraries(complete PRIVATE oscour)
# set_target_properties(oscour PROPERTIES CXX_STANDARD 17)
