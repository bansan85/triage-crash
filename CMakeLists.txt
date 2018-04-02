cmake_minimum_required(VERSION 3.8.0)

project(triage-crash)

include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

set(LIB2LGC_INCLUDE)
set(LIB2LGC_LIB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (LIB2LGC_INCLUDE)
  set(CMAKE_MODULE_PATH "${LIB2LGC_INCLUDE}/util/cmake;${CMAKE_MODULE_PATH}")
endif()

include(2lgc)

if (CHECK_CODE)
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(NOT iwyu_path)
    message(FATAL_ERROR "Could not find the program include-what-you-use")
  endif()
  if (LIB2LGC_INCLUDE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path} -Xiwyu
      --mapping_file=${LIB2LGC_INCLUDE}/util/iwyu/mapping.imp)
  endif()
endif()

add_subdirectory(src)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cc *.h)

add_custom_target(check)

if (CHECK_CODE)
  add_custom_command(TARGET check
    COMMAND clang-format -style='{BasedOnStyle: google, BreakBeforeBraces: Custom, BraceWrapping: { AfterClass: true, AfterControlStatement: true, AfterEnum : true, AfterFunction : true, AfterNamespace : true, AfterObjCDeclaration: true, AfterStruct : true, AfterUnion : true, BeforeCatch : true, BeforeElse : true, IndentBraces : false } }' -i ${ALL_SOURCE_FILES}
    #-build/include: implementation of template with template class in source instead of header needs include .cc.
    COMMAND cpplint.py --filter=-whitespace/braces,-whitespace/newline,-build/include ${ALL_SOURCE_FILES}
    COMMAND cppcheck --language=c++ --std=c++14 --inconclusive --enable=all -I${LIB2LGC_INCLUDE}/src ${ALL_SOURCE_FILES})
endif()
