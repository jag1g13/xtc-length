project(xtc-length)
cmake_minimum_required(VERSION 2.8)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -O2")
endif()

add_executable(xtc-length xtc-length.c)

install(TARGETS xtc-length RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
