#ifndef ICMP_H
#define ICMP_H

#include "ping.h"

#ifdef __APPLE__

typedef struct ip iphdr_t;

struct icmphdr
{
    u_int8_t type;                /* message type */
    u_int8_t code;                /* type sub-code */
    u_int16_t checksum;
    union
    {
        struct
        {
            u_int16_t        id;
            u_int16_t        sequence;
        } echo;                        /* echo datagram */
    } un;
};

#endif
#ifdef __linux__

#include <netinet/ip_icmp.h>

typedef struct iphdr iphdr_t;

#endif

char *create_icmp_packet(ping_data_t *ping_data);
u_int16_t    calculate_checksum(void* data, size_t len);

#endif //ICMP_H
