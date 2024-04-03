//  Copyright (c) 2024 Ethan Delage

#include <stdio.h>

#include "ft_ping.h"

void print_params(ping_params_t ping_params) {
    printf("host: `%s`\n", ping_params.host);
    printf("verbose: %d\n", ping_params.verbose);
}
