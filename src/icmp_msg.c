//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>

#include "ft_ping.h"

static struct icmphdr get_icmp_header(u_char icmp_type, uint16_t seq);

char* get_ping_packet(size_t packet_size, uint16_t seq) {
    struct icmphdr* icmp_header;
    char* msg;

    msg = calloc(1, sizeof(struct icmphdr) + packet_size);
    if (msg == NULL) {
        dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        return NULL;
    }
    icmp_header = (struct icmphdr*) msg;
    *icmp_header = get_icmp_header(ICMP_ECHO, seq);
    icmp_header->checksum = get_checksum(msg, sizeof(struct icmphdr) + packet_size);
    return msg;
}

static struct icmphdr get_icmp_header(u_char icmp_type, uint16_t seq) {
    struct icmphdr icmp_header;

    bzero(&icmp_header, sizeof(struct icmphdr));
    icmp_header.type = icmp_type;
    icmp_header.un.echo.sequence = htons(seq);
    icmp_header.un.echo.id = getpid();
    return icmp_header;
}

u_int16_t    get_checksum(void* data, size_t len) {
    u_int16_t* buffer;
    u_int32_t sum;

    sum = 0;
    buffer = (uint16_t*) data;
    while (len > 1) {
        sum += *buffer;
        if (sum > 0xFFFF) {
            sum = (sum >> 16) + (sum & 0xFFFF);
        }
        ++buffer;
        len -= 2;
    }
    if (len == 1) {
        sum += *(u_int8_t*) buffer;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    return (u_int16_t) ~sum;
}
