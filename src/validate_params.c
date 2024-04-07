//  Copyright (c) 2024 Ethan Delage

#include <unistd.h>
#include <stdbool.h>

#include "ft_ping.h"

extern ping_rts_t* rts_g;

bool validate_params(int argc, char** argv, ping_params_t* ping_params) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (validate_option(argc, argv, &i, ping_params) == false) {
                return false;
            }
        } else if (ping_params->host == NULL) {
            ping_params->host = argv[i];
            rts_g->host = argv[i];
        } else {
            return false;
        }
    }
    if (ping_params->host == NULL) {
        return false;
    }
    return true;
}
