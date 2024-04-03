//  Copyright (c) 2024 Ethan Delage

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ft_ping.h"

static char* get_option_argument(int argc, char** argv, int* option_index);
static void set_option(char option, char* argument,
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
            set_option(options[i], argument, ping_params);
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

static void set_option(char option, char* argument,
                       ping_params_t* ping_params) {
    switch (option) {
        case 'v':
            ping_params->verbose = true;
            break;
    }
    (void) argument;
}
