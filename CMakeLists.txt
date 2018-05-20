cmake_minimum_required(VERSION 3.8.0)

project(triage_crash)

include(FindProtobuf)
include(FindOpenSSL)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

set(LIB2LGC_INCLUDE)
set(LIB2LGC_LIB)

if (LIB2LGC_INCLUDE)
  set(CMAKE_MODULE_PATH "${LIB2LGC_INCLUDE}/util/cmake;${CMAKE_MODULE_PATH}")
endif()

include(2lgc)

llgc_init_cflags()
llgc_iwyu()

add_subdirectory(src)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cc *.h)

add_custom_target(check)

llgc_check_all("${ALL_SOURCE_FILES}" FALSE "")
