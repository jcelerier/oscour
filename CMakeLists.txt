cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(oscour)

add_executable(oscour
    src/oscour.hpp
    src/oscour.cpp)

set_target_properties(oscour PROPERTIES CXX_STANDARD 17)
