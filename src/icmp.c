#include "icmp.h"

#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

static void fill_icmp_header(struct icmphdr *icmp_header, const ping_data_t *ping_data);

char *create_icmp_packet(ping_data_t *ping_data) {
    struct icmphdr* icmp_header;
    char* packet;

    packet = malloc(sizeof(struct icmphdr) + ping_data->opt.packet_size);
    if (packet == NULL) {
        error(EXIT_FAILURE, errno, "malloc failed");
    }
    icmp_header = (struct icmphdr *) packet;
    fill_icmp_header(icmp_header, ping_data);
    icmp_header->checksum = calculate_checksum(packet, sizeof(struct icmphdr) + ping_data->opt.packet_size);
    return packet;
}

static void fill_icmp_header(struct icmphdr *icmp_header, const ping_data_t *ping_data) {
    bzero(icmp_header, sizeof(struct icmphdr));
    icmp_header->type = ping_data->icmp_type;
    icmp_header->code = 0;
    icmp_header->un.echo.sequence = htons(ping_data->seq);
    icmp_header->un.echo.id = ping_data->id;
}

u_int16_t    calculate_checksum(void* data, size_t len) {
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
