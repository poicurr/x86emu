cmake_minimum_required(VERSION 3.1)

project(x86emu CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

add_subdirectory(src)
