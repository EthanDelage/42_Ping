#ifndef PING_H
#define PING_H

#include "options.h"

#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/time.h>

#define RESPONSE_OFFSET (sizeof(iphdr_t) + sizeof(struct icmphdr) + sizeof(iphdr_t))

typedef struct ping_data_s {
    char *ip;
    struct sockaddr_in sock_addr;

    u_char icmp_type;
    uint16_t id;
    uint16_t seq;
    ping_option_t opt;

    struct timeval start_tv;
    struct timeval last_send_tv;
} ping_data_t;

typedef struct ping_response_s {
    char *packet;
    size_t packet_size;
    struct sockaddr_in sock_addr;
    struct timeval receive_tv;
    long trip_time;

    uint8_t type;
    uint8_t code;
    uint16_t id;
    uint16_t seq;
    uint8_t ttl;
} ping_response_t;


void init_ping(ping_data_t *ping_data);
void run_ping(ping_data_t *ping_data);

#endif //PING_H
