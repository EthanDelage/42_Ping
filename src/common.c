#include "common.h"

#include <error.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @brief convert a string to a size_t
 * @param arg string to convert
 * @param max maximum value, if max = 0, there is no maximum value
 * @return result of the conversion
 */
size_t convert_arg_to_size_t(char *arg, size_t max) {
    char *end;
    unsigned long value;

    value = strtoul(arg, &end, 0);
    if (*end) {
        error (EXIT_FAILURE, 0, "invalid value (`%s' near `%s')", arg, end);
    }
    if (max && value > max) {
        error (EXIT_FAILURE, 0, "option value too big: %s", arg);
    }
    return value;
}

/**
 * @brief convert a string to a double
 * @param arg string to convert
 * @return result of the conversion
 */
double convert_arg_to_double(char *arg) {
    char *end;
    double value;

    value = strtod(arg, &end);
    if (*end) {
        error (EXIT_FAILURE, 0, "invalid value (`%s' near `%s')", arg, end);
    }
    return value;
}

/**
 * @brief convert double value (in seconds) into struct timeval
 * @param seconds value to convert (in seconds)
 * @return value converted as struct timeval
 */
struct timeval convert_double_to_timeval(double seconds) {
    double integral_part;
    double fractional_part;
    struct timeval result;

    fractional_part = modf(seconds, &integral_part);
    result.tv_sec = (time_t) integral_part;
    result.tv_usec = (suseconds_t)(fractional_part * 1e6);

    if (result.tv_usec >= 1000000) {
        result.tv_sec++;
        result.tv_usec -= 1000000;
    }
    return result;
}

/**
 * @brief Adjusts a timeval structure to ensure valid fields.
 *
 * This function normalizes a `struct timeval` to guarantee that:
 * - The `tv_usec` field is always in the range [0, 999999].
 * - Any overflow or underflow in the `tv_usec` field is properly
 *   carried over to the `tv_sec` field.
 * @param tv pointer to a struct timeval to adjust
 */
void normalize_timeval(struct timeval *tv) {
    if (tv == NULL) {
        return;
    }

    if (tv->tv_usec >= 1000000) {
        tv->tv_sec += tv->tv_usec / 1000000;
        tv->tv_usec %= 1000000;
    }
    if (tv->tv_usec < 0) {
        tv->tv_sec -= 1 + (-tv->tv_usec) / 1000000;
        tv->tv_usec = 1000000 - (-tv->tv_usec) % 1000000;
    }
}

long calculate_trip_time(struct timeval start_tv, struct timeval end_tv) {
    struct timeval  interval_tv;
    long interval;

    interval_tv.tv_sec = end_tv.tv_sec - start_tv.tv_sec;
    interval_tv.tv_usec = end_tv.tv_usec - start_tv.tv_usec;
    interval = interval_tv.tv_sec * 1000000 + interval_tv.tv_usec;
    if (interval < 0) {
        error(0, 0, "Warning: time of day goes back (%ldus), " \
                "taking countermeasures", interval);
        interval = 0;
    }
    return interval;
}
