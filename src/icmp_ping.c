//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "ft_ping.h"

static ssize_t send_ping(int sock_fd, ping_params_t* ping_params,
                         struct timeval* start_tv);
static char* receive_ping(int sock_fd, ping_params_t* ping_params,
                          struct timeval* end_tv);

int icmp_ping(int sock_fd, ping_params_t* ping_params) {
    ssize_t ret;
    char* reply_packet;
    struct icmphdr* icmp_header;
    struct timeval start_tv;
    struct timeval end_tv;
    long timestamp;

    ret = send_ping(sock_fd, ping_params, &start_tv);
    if (ret > 0) {
        return -1;
    }
    reply_packet = receive_ping(sock_fd, ping_params, &end_tv);
    if (reply_packet == NULL) {
        if (errno == EAGAIN)
            return 0;
        return -1;
    }
    icmp_header = (struct icmphdr*) (reply_packet + sizeof(iphdr_t));
    // TODO: remove print
    timestamp = get_timestamp(start_tv, end_tv);
    printf("type: %d, code: %d, seq: %d, id: %d", icmp_header->type,
           icmp_header->code, icmp_header->un.echo.sequence,
           icmp_header->un.echo.id);
    print_timestamp(timestamp);
    printf("ttl: %d\n", ((iphdr_t*) reply_packet)->ip_ttl);
    free(reply_packet);
    return 0;
}

static ssize_t send_ping(int sock_fd, ping_params_t* ping_params,
                         struct timeval* start_tv) {
    char* ping_message;
    ssize_t message_len;
    ssize_t ret;

    message_len = sizeof(struct icmphdr) + ping_params->packet_size;
    ping_message = get_ping_message(ping_params->packet_size, ping_params->seq);
    if (ping_message == NULL) {
        return -1;
    }
    gettimeofday(start_tv, NULL);
    ret = sendto(sock_fd, ping_message, message_len, 0,
                 (struct sockaddr*) &ping_params->sock_addr,
                 sizeof(struct sockaddr_in));
    free(ping_message);
    if (ret != message_len) {
        dprintf(STDERR_FILENO, "ping: sendto: %s\n", strerror(errno));
        return ret;
    }
    return 0;
}

static char* receive_ping(int sock_fd, ping_params_t* ping_params,
                          struct timeval* end_tv) {
    char* buffer;
    ssize_t ret;
    ssize_t message_len;
    socklen_t addr_len;

    message_len = sizeof(iphdr_t) + sizeof(struct icmphdr)
            + ping_params->packet_size;
    buffer = malloc(message_len + 1);
    if (buffer == NULL) {
        dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        return NULL;
    }
    addr_len = sizeof(ping_params->sock_addr);
    ret = recvfrom(sock_fd, buffer, message_len, 0,
                   (struct sockaddr*) &ping_params->sock_addr, &addr_len);
    gettimeofday(end_tv, NULL);
    if (ret <= 0) {
        if (errno == EAGAIN) {
            printf("Request timeout for icmp_seq %d\n", ping_params->seq);
        } else {
            dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        }
        free(buffer);
        return NULL;
    }
    buffer[ret] = '\0';
    return buffer;
}
