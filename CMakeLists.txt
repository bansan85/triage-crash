cmake_minimum_required(VERSION 3.8.0)

project(triage-crash)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_GDB)
set(LIBRARY_GDB)
set(INCLUDE_HEADER)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include(2lgc)

add_subdirectory(src)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cc *.h)
add_custom_target(check COMMAND clang-format -style=Google -i ${ALL_SOURCE_FILES})
