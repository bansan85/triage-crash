if (LIB2LGC_INCLUDE)
  include_directories(${LIB2LGC_INCLUDE}/src)
endif()
if (LIB2LGC_LIB)
  # For protobuf header
  include_directories(${LIB2LGC_LIB}/src)
  link_directories(${LIB2LGC_LIB}/src)
endif()

find_package(Threads)

add_executable(triage_crash triage_crash.cc)
target_link_libraries(triage_crash 2lgc ${Protobuf_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARY})
if (CHECK_CODE)
  set_target_properties(triage_crash PROPERTIES CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()
