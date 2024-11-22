#ifndef RTT_H
#define RTT_H

#include <stdlib.h>

typedef struct ping_rtt_s {
    char* host;
    size_t n_transmitted;
    size_t n_received;
    long min_trip_time;
    long max_trip_time;
    size_t trip_time_sum;
    size_t trip_time_square_sum;
} ping_rtt_t;

void update_rtt(long new_trip_time);

#endif //RTT_H
