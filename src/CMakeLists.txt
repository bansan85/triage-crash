include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../inc)

if (INCLUDE_GDB)
        include_directories(${INCLUDE_GDB})
endif()
if (LIBRARY_GDB)
        link_directories(${LIBRARY_GDB})
endif()

find_package(Threads)

add_executable(triage-crash triage-crash.cc)
target_link_libraries(triage-crash 2lgcgdb stdc++fs ${CMAKE_THREAD_LIBS_INIT})
