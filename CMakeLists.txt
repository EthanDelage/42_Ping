cmake_minimum_required(VERSION 3.30)
project(ping C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(ping
    include/common.h
    include/icmp.h
    include/options.h
    include/ping.h
    include/print.h
    include/rtt.h
    include/socket.h
    src/common.c
    src/icmp.c
    src/main.c
    src/options.c
    src/ping.c
    src/print.c
    src/rtt.c
    src/socket.c)

target_link_libraries(ping m)
