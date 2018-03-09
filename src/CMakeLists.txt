if (LIB2LGC)
  include_directories(${LIB2LGC}/libgdb/inc ${LIB2LGC}/libheader/inc)
  link_directories(${LIB2LGC}/libgdb-build/src)
endif()

find_package(Threads)

add_executable(triage-crash triage-crash.cc)
add_dependencies(triage-crash libgdb)
target_link_libraries(triage-crash 2lgcgdb stdc++fs ${CMAKE_THREAD_LIBS_INIT})
if (CHECK_CODE)
  set_target_properties(triage-crash PROPERTIES CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()
