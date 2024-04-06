//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ft_ping.h"

static int send_ping(int sock_fd, ping_params_t* ping_params);
static char* receive_ping(int sock_fd, ping_params_t* ping_params);

int icmp_ping(int sock_fd, ping_params_t* ping_params) {
    char* reply_packet;
    struct icmphdr* icmp_header;

    if (send_ping(sock_fd, ping_params) != 0) {
        return -1;
    }
    reply_packet = receive_ping(sock_fd, ping_params);
    if (reply_packet == NULL) {
        return -1;
    }
    icmp_header = (struct icmphdr*) (reply_packet+ 20);
    // TODO: remove print
    printf("type: %d, code: %d, seq: %d, id: %d\n", icmp_header->type,
           icmp_header->code, icmp_header->un.echo.sequence,
           icmp_header->un.echo.id);
    printf("ttl: %d\n", ((iphdr_t*) reply_packet)->ip_ttl);
    free(reply_packet);
    return 0;
}

static int send_ping(int sock_fd, ping_params_t* ping_params) {
    char* ping_message;
    ssize_t message_len;
    ssize_t ret;

    message_len = sizeof(struct icmphdr) + ping_params->packet_size;
    ping_message = get_ping_message(ping_params->packet_size, ping_params->seq);
    if (ping_message == NULL) {
        return -1;
    }
    ret = sendto(sock_fd, ping_message, message_len, 0,
                 (struct sockaddr*) &ping_params->sock_addr,
                 sizeof(struct sockaddr_in));
    free(ping_message);
    if (ret != message_len) {
        dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static char* receive_ping(int sock_fd, ping_params_t* ping_params) {
    char* buffer;
    ssize_t ret;
    ssize_t message_len;
    socklen_t addr_len;

    message_len = sizeof(iphdr_t) + sizeof(struct icmphdr)
            + ping_params->packet_size;
    buffer = calloc(1, message_len + 1);
    if (buffer == NULL) {
        dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        return NULL;
    }
    addr_len = sizeof(ping_params->sock_addr);
    ret = recvfrom(sock_fd, buffer, message_len, 0,
                   (struct sockaddr*) &ping_params->sock_addr, &addr_len);
    if (ret <= 0) {
        dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        free(buffer);
        return NULL;
    }
    return buffer;
}
