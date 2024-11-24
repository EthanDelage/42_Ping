#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>

size_t convert_arg_to_size_t(char *arg, size_t max, int allow_zero);
double convert_arg_to_double(char *arg);
struct timeval convert_double_to_timeval(double seconds);

void normalize_timeval(struct timeval *tv);
long calculate_trip_time(struct timeval start_tv, struct timeval end_tv);

#endif //COMMON_H
