cmake_minimum_required(VERSION 3.8.0)

project(triage-crash)

set(INCLUDE_GDB)
set(LIBRARY_GDB)
set(INCLUDE_HEADER)
set(CHECK_CODE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include(2lgc)

if (CHECK_CODE)
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(NOT iwyu_path)
    message(FATAL_ERROR "Could not find the program include-what-you-use")
  endif()
endif()

add_subdirectory(src)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cc *.h)
add_custom_target(check)
if (CHECK_CODE)
  add_custom_command(TARGET check
    COMMAND clang-format -style='{BasedOnStyle: google, BreakBeforeBraces: Custom, BraceWrapping: { AfterClass: true, AfterControlStatement: true, AfterEnum : true, AfterFunction : true, AfterNamespace : true, AfterObjCDeclaration: true, AfterStruct : true, AfterUnion : true, BeforeCatch : true, BeforeElse : true, IndentBraces : false } }' -i ${ALL_SOURCE_FILES}
    COMMAND cpplint.py --filter=-whitespace/braces,-whitespace/newline ${ALL_SOURCE_FILES})
endif()
