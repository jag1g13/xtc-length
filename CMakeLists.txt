project(xtc-length)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")
endif()

add_executable(xtc-length xtc-length.c)
