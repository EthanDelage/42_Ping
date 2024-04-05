//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ft_ping.h"

static ping_params_t init_ping_params();

int main(int argc, char** argv) {
    ping_params_t ping_params;
    int sock_fd;

    ping_params = init_ping_params();
    if (validate_params(argc, argv, &ping_params) == false) {
        dprintf(STDERR_FILENO, USAGE_MESSAGE);
        return 1;
    }
    if (resolve_host(ping_params.host, &ping_params) != 0) {
        return 1;
    }
    print_params(ping_params);
    sock_fd = init_socket();
    if (sock_fd < 0) {
        return errno;
    }
    if (icmp_ping(sock_fd, &ping_params) != 0) {
        return errno;
    }
    close(sock_fd);
    return 0;
}

static ping_params_t init_ping_params() {
    ping_params_t ping_params;

    bzero(&ping_params, sizeof(ping_params_t));
    ping_params.packet_size = DEFAULT_PACKET_SIZE;
    return ping_params;
}
