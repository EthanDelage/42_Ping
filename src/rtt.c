#include "rtt.h"
#include "common.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>


ping_rtt_t rtt_g = {0};

void update_rtt(long new_trip_time) {
    if (rtt_g.n_received == 0) {
        rtt_g.max_trip_time = new_trip_time;
        rtt_g.min_trip_time = new_trip_time;
    } else if (new_trip_time < rtt_g.min_trip_time) {
        rtt_g.min_trip_time = new_trip_time;
    } else if (new_trip_time > rtt_g.max_trip_time) {
        rtt_g.max_trip_time = new_trip_time;
    }
    rtt_g.trip_time_sum += new_trip_time;
    rtt_g.trip_time_square_sum += (new_trip_time * new_trip_time);
    ++rtt_g.n_received;
}
