//  Copyright (c) 2024 Ethan Delage

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>

static struct icmp get_icmp_header(u_char icmp_type);
static u_int16_t    get_checksum(void* data, size_t len);

char* get_ping_message(size_t packet_size) {
    struct icmp* icmp_header;
    char* msg;

    msg = calloc(1, sizeof(struct icmp) + packet_size);
    icmp_header = (struct icmp*) msg;
    *icmp_header = get_icmp_header(ICMP_ECHO);
    icmp_header->icmp_cksum = get_checksum(msg, sizeof(struct icmp) + packet_size);
    return msg;
}

static struct icmp get_icmp_header(u_char icmp_type) {
    static n_short seq = 0;
    struct icmp icmp_header;

    bzero(&icmp_header, sizeof(struct icmp));
    icmp_header.icmp_type = icmp_type;
    icmp_header.icmp_seq = seq;
    icmp_header.icmp_id = getpid();
    ++seq;
    return icmp_header;
}

static u_int16_t    get_checksum(void* data, size_t len) {
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
