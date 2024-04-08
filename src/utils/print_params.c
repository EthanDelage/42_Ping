//  Copyright (c) 2024 Ethan Delage

#include <stdio.h>

#include "ft_ping.h"

void print_params(ping_params_t ping_params) {
    printf("host: `%s`\n", ping_params.host);
    printf("ip: %s\n", ping_params.ip);
    printf("verbose: %d\n", ping_params.verbose);
    printf("debug: %d\n", ping_params.so_debug);
    printf("count: %d, value: %ld\n", ping_params.count, ping_params.count_arg);
}
