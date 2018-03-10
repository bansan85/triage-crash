cmake_minimum_required(VERSION 3.8.0)

project(triage-crash)

set(LIB2LGC ${CMAKE_BINARY_DIR}
  CACHE STRING "Folder where all submodules are.")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (${LIB2LGC} MATCHES ${CMAKE_BINARY_DIR})
  execute_process(COMMAND git clone https://github.com/bansan85/2lgcutils.git
    ${CMAKE_BINARY_DIR}/utils)
endif()
set(CMAKE_MODULE_PATH "${LIB2LGC}/utils/cmake;${CMAKE_MODULE_PATH}")

include(2lgc)

if (CHECK_CODE)
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(NOT iwyu_path)
    message(FATAL_ERROR "Could not find the program include-what-you-use")
  endif()
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path} -Xiwyu
    --mapping_file=${LIB2LGC}/utils/iwyu/mapping.imp)
endif()

if (${LIB2LGC} MATCHES ${CMAKE_BINARY_DIR})
  include(ExternalProject)

  ExternalProject_Add(libheader
    GIT_REPOSITORY https://github.com/bansan85/lib2lgcheader.git
    GIT_TAG master
    CMAKE_ARGS "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS};-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/libheader
    BINARY_DIR ${CMAKE_BINARY_DIR}/libheader-build
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND ""
  )

  ExternalProject_Add(libgdb
    DEPENDS libheader
    GIT_REPOSITORY https://github.com/bansan85/lib2lgcgdb.git
    GIT_TAG master
    CMAKE_ARGS "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS};-DLIB2LGC=${LIB2LGC};-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER};-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/libgdb
    BINARY_DIR ${CMAKE_BINARY_DIR}/libgdb-build
    TEST_COMMAND ""
    INSTALL_COMMAND ""
)
else()
  add_custom_target(libgdb)
endif()

add_subdirectory(src)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cc *.h)
add_custom_target(check)
if (CHECK_CODE)
  if (LIB2LGC)
    set(INCLUDE_CPPCHECK "-I${LIB2LGC}/libheader/inc;-I${LIB2LGC}/libgdb/inc")
  else()
    set(INCLUDE_CPPCHECK "")
  endif()
  add_custom_command(TARGET check
    COMMAND clang-format -style='{BasedOnStyle: google, BreakBeforeBraces: Custom, BraceWrapping: { AfterClass: true, AfterControlStatement: true, AfterEnum : true, AfterFunction : true, AfterNamespace : true, AfterObjCDeclaration: true, AfterStruct : true, AfterUnion : true, BeforeCatch : true, BeforeElse : true, IndentBraces : false } }' -i ${ALL_SOURCE_FILES}
    COMMAND cpplint.py --filter=-whitespace/braces,-whitespace/newline ${ALL_SOURCE_FILES}
    COMMAND cppcheck --check-config --language=c++ --std=c++14 --inconclusive --enable=all ${INCLUDE_CPPCHECK} ${ALL_SOURCE_FILES})
endif()
