project(xtc-length)

if(CMAKE_VERSION VERSION_GREATER "3.1")
    set(CMAKE_C_STANDARD 99)
endif()

add_executable(xtc-length xtc-length.c)
