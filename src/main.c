//  Copyright (c) 2024 Ethan Delage

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ft_ping.h"

int main(int argc, char** argv) {
    ping_params_t ping_params;

    bzero(&ping_params, sizeof(ping_params_t));
    if (validate_params(argc, argv, &ping_params) == false) {
        dprintf(STDERR_FILENO, USAGE_MESSAGE);
        return 1;
    }
    if (resolve_host(ping_params.host, &ping_params) != 0) {
        close(sock_fd);
        return 1;
    }
    print_params(ping_params);
    return 0;
}