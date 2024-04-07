//  Copyright (c) 2024 Ethan Delage

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "ft_ping.h"

extern ping_rts_t* rts_g;

long get_timestamp(struct timeval start_tv, struct timeval end_tv) {
    struct timeval  timestamp_tv;
    long timestamp;

    timestamp_tv.tv_sec = end_tv.tv_sec - start_tv.tv_sec;
    timestamp_tv.tv_usec = end_tv.tv_usec - start_tv.tv_usec;
    timestamp = timestamp_tv.tv_sec * 1000000 + timestamp_tv.tv_usec;
    if (timestamp < 0) {
        dprintf(STDERR_FILENO, "Warning: time of day goes back (%ldus), " \
                "taking countermeasures\n", timestamp);
        timestamp = 0;
    }
    return timestamp;
}

int add_timestamp(long timestamp) {
    if (rts_g->n_received == 0) {
        rts_g->max_timestamp = timestamp;
        rts_g->min_timestamp = timestamp;
    } else if (timestamp < rts_g->min_timestamp) {
        rts_g->min_timestamp = timestamp;
    } else if (timestamp > rts_g->max_timestamp) {
        rts_g->max_timestamp = timestamp;
    }
    rts_g->timestamp_sum += timestamp;
    rts_g->timestamp_square_sum += (timestamp * timestamp);
    ++rts_g->n_received;
    return 0;
}

void print_timestamp(long timestamp) {
    if (timestamp >= 100000 - 50)
        printf(" time=%ld ms", (timestamp + 500) / 1000);
    else if (timestamp >= 10000 - 5)
        printf(" time=%ld.%01ld ms", (timestamp + 50) / 1000,
               ((timestamp + 50) % 1000) / 100);
    else if (timestamp >= 1000)
        printf(" time=%ld.%02ld ms", (timestamp + 5) / 1000,
               ((timestamp + 5) % 1000) / 10);
    else
        printf(" time=%ld.%03ld ms", timestamp / 1000, timestamp % 1000);
}
