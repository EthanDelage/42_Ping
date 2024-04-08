//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>

#include "ft_ping.h"

extern ping_rts_t* rts_g;

static ssize_t send_ping(int sock_fd, ping_params_t* ping_params,
                         struct timeval* start_tv);
static char* receive_ping(int sock_fd, ping_params_t* ping_params,
                          struct timeval* end_tv);
static int validate_reply(char* reply, ping_params_t* ping_params,
                          long timestamp);

int icmp_ping(int sock_fd, ping_params_t* ping_params) {
    ssize_t ret;
    char* reply_packet;
    struct timeval start_tv;
    struct timeval end_tv;
    long timestamp;

    ret = send_ping(sock_fd, ping_params, &start_tv);
    if (ret > 0) {
        return -1;
    }
    ++rts_g->n_transmitted;
    reply_packet = receive_ping(sock_fd, ping_params, &end_tv);
    if (reply_packet == NULL) {
        if (errno == EAGAIN)
            return 0;
        return -1;
    }
    timestamp = get_timestamp(start_tv, end_tv);
    if (validate_reply(reply_packet, ping_params, timestamp) == 0) {
        add_timestamp(timestamp);
    }
    free(reply_packet);
    usleep(1000000 - timestamp);
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
    buffer = malloc(message_len);
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
    return buffer;
}

static int validate_reply(char* reply, ping_params_t* ping_params,
                          long timestamp) {
    iphdr_t* ip_hdr;
    struct icmphdr* icmp_hdr;
    u_int16_t checksum;
    ssize_t message_len;

    message_len = sizeof(struct icmphdr) + ping_params->packet_size;
    ip_hdr = (iphdr_t*) reply;
    icmp_hdr = (struct icmphdr*) (reply + sizeof(iphdr_t));
    checksum = icmp_hdr->checksum;
    icmp_hdr->checksum = 0;

    if (get_checksum(icmp_hdr, sizeof(struct icmphdr)) != checksum) {
        printf("Bad checksum for reply icmp_seq %d\n", ping_params->seq);
        return 1;
    }
    if (icmp_hdr->type == ICMP_ECHOREPLY) {
        printf("%zu bytes from %s: icmp_seq=%d ttl=%d",
               message_len, ping_params->ip,
               ping_params->seq, ip_hdr->ip_ttl);
        print_timestamp(timestamp);
        printf("\n");
    } else {
        printf("Bad type for reply icmp_seq %d\n", ping_params->seq);
        return 1;
    }
    return 0;
}
