#ifndef PRINT_H
#define PRINT_H

#include "ping.h"
#include "rtt.h"

#define HOST_SIZE 256

void print_ping_info(const ping_data_t *ping_data);
void print_response(const ping_response_t *ping_response, bool verbose);
void print_rtt(const ping_rtt_t *rtt);
void print_trip_time(long timestamp);

#endif //PRINT_H
