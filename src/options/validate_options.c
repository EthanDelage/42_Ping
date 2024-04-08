//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ft_ping.h"

static char* get_option_argument(int argc, char** argv, int* option_index);
static int set_option(char option, char* argument,
                       ping_params_t* ping_params);

bool validate_option(int argc, char** argv, int* index,
                     ping_params_t* ping_params) {
    char* options;
    char* argument;

    options = &argv[*index][1];
    for (size_t i = 0; i < strlen(options); ++i) {
        if (strchr(OPTIONS_WITHOUT_ARG, options[i]) != NULL) {
            set_option(options[i], NULL, ping_params);
        } else if (strchr(OPTIONS_WITH_ARG, options[i]) != NULL) {
            argument = get_option_argument(argc, argv, index);
            if (argument == NULL) {
                dprintf(STDERR_FILENO, "%s%c\n",
                        OPTION_REQUIRES_ARG_MESSAGE, options[i]);
                return false;
            }
            if (set_option(options[i], argument, ping_params) != 0) {
                exit(1);
            }
        } else {
            dprintf(STDERR_FILENO, "%s%c\n", INVALID_OPTION_MESSAGE, options[i]);
            return false;
        }
    }
    return true;
}

static char* get_option_argument(int argc, char** argv, int* option_index) {
    if (*option_index < argc - 1) {
        ++(*option_index);
        return argv[*option_index];
    }
    return NULL;
}

static int set_option(char option, char* argument,
                       ping_params_t* ping_params) {
    char* rest;

    switch (option) {
        case 'v':
            ping_params->verbose = true;
            break;
        case '?':
            ping_params->help = true;
            break;
        case 'd':
            ping_params->so_debug = true;
            break;
        case 'c':
            ping_params->count = true;
            ping_params->count_arg = strtol(argument, &rest, 10);
            if (*rest != '\0' || errno == ERANGE
                || ping_params->count_arg < 1) {
                printf("ping: invalid count of packets to transmit: `%s'\n",
                       argument);
                return -1;
            }
            break;
        case 'i':
            ping_params->interval = strtod(argument, &rest);
            if (*rest != '\0' || errno == ERANGE
                || isless(ping_params->interval, 0)
                || isgreater(ping_params->interval, INT_MAX / 1000)) {
                printf("ping: bad interval time: `%s'\n", argument);
                ping_params->interval = 1;
                return -1;
            }
            break;
        case 'W':
            ping_params->linger = true;
            ping_params->linger_arg = strtod(argument, &rest);
            if (*rest != '\0' || errno == ERANGE
                || isless(ping_params->linger_arg, 0)
                || isgreater(ping_params->linger_arg, INT_MAX / 1000)) {
                printf("ping: bad linger time: `%s'\n", argument);
                return -1;
            }
            break;
    }
    return 0;
}
