cmake_minimum_required(VERSION 3.27)
project(ft_ping C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(ft_ping
        include/ft_ping.h
        src/main.c
        src/options/validate_options.c
        src/validate_params.c
        src/utils/print_params.c
        src/socket_initialization.c
        src/resolve_host.c
        src/icmp_msg.c
        src/icmp_ping.c
        src/utils/timestamp.c)
