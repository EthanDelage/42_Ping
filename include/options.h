#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdlib.h>

#define DEFAULT_INTERVAL 1.
#define DEFAULT_LINGER_SEC 10
#define DEFAULT_PACKET_SIZE 56
#define MAX_PACKET_SIZE 65535

typedef struct ping_option_s {
    bool verbose;
    bool debug;
    size_t count;
    struct timeval linger_tv;
    struct timeval interval_tv;
    size_t packet_size;
    char *host;
} ping_option_t;

void parse_opt(int argc, char **argv, ping_option_t *ping_opt);

#endif //OPTIONS_H
