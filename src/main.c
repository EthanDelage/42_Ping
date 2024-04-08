//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ft_ping.h"

ping_rts_t* rts_g;

static ping_params_t init_ping_params();
static ping_rts_t init_ping_rts();
static void sigint_handler(int signum);
static void finish(ping_rts_t* rts);

int main(int argc, char** argv) {
    static ping_rts_t rts;
    ping_params_t ping_params;
    double interval;
    int sock_fd;

    rts = init_ping_rts();
    rts_g = &rts;
    ping_params = init_ping_params();
    if (validate_params(argc, argv, &ping_params) == false) {
        printf(USAGE_MESSAGE);
        return 1;
    }
    if (ping_params.help) {
        printf(USAGE_MESSAGE);
        return 0;
    }
    if (resolve_host(ping_params.host, &ping_params) != 0) {
        return 1;
    }
    sock_fd = init_socket(&ping_params);
    if (sock_fd < 0) {
        return errno;
    }
    rts.sock_fd = sock_fd;
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        close(sock_fd);
        return errno;
    }
    printf("PING %s (%s): %zu(%zu) data bytes\n",
           ping_params.host, ping_params.ip, ping_params.packet_size,
           sizeof(iphdr_t) + sizeof(struct icmphdr) + ping_params.packet_size);
    while (ping_params.count == false || ping_params.count_arg != 0) {
        if (icmp_ping(sock_fd, &ping_params) != 0) {
            finish(&rts);
            return errno;
        }
        interval = ping_params.interval - rts.last_timestamp / 1000000;
        if ((ping_params.count == false || ping_params.count_arg != 1)
            && interval > 0) {
            if (interval > 1) {
                sleep(floor(interval));
            }
            usleep((interval - floor(interval)) * 1000000);
        }
        ++ping_params.seq;
        if (ping_params.count) {
            --ping_params.count_arg;
        }
    }
    finish(&rts);
}

static ping_params_t init_ping_params() {
    ping_params_t ping_params;

    bzero(&ping_params, sizeof(ping_params_t));
    ping_params.packet_size = DEFAULT_PACKET_SIZE;
    return ping_params;
}

static ping_rts_t init_ping_rts() {
    ping_rts_t ping_rts;

    bzero(&ping_rts, sizeof(ping_rts_t));
    return ping_rts;
}

static void sigint_handler(int signum) {
    (void) signum;
    finish(rts_g);
    exit(0);
}

static void finish(ping_rts_t* rts) {
    double timestamp_avg;
    double timestamp_stddev;

    printf("\n--- %s ping statistics ---\n", rts->host);
    printf("%zu packets transmitted, ", rts->n_transmitted);
    printf("%zu received", rts->n_received);
    if (rts->n_transmitted) {
        printf(", %g%% packet loss,",
               (float)(((rts->n_transmitted - rts->n_received) * 100.0)
                       / rts->n_transmitted));
        print_timestamp(rts->timestamp_sum);
    }
    printf("\n");
    if (rts->n_received) {
        timestamp_avg = rts->timestamp_sum / rts->n_received;
        timestamp_stddev = sqrt((rts->timestamp_square_sum / rts->n_received)
                - (timestamp_avg * timestamp_avg));
        printf("round-trip min/avg/max/stddev = " \
               "%ld.%03ld/%lu.%03ld/%ld.%03ld/%ld.%03ld ms\n",
               (long)rts->min_timestamp / 1000, (long)rts->min_timestamp % 1000,
               (long)(timestamp_avg / 1000), (long)(timestamp_avg) % 1000,
               (long)rts->max_timestamp / 1000, (long)rts->max_timestamp % 1000,
               (long)timestamp_stddev / 1000, (long)timestamp_stddev % 1000);
    }
    close(rts->sock_fd);
}
