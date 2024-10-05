//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

#include "ft_ping.h"

extern ping_rts_t* rts_g;

static ssize_t send_ping(int sock_fd, ping_params_t* ping_params,
                         struct timeval* start_tv);
static char* receive_ping(int sock_fd, ping_params_t* ping_params,
                          struct timeval* end_tv);
static int validate_reply(char* reply, ping_params_t* ping_params,
                          long timestamp);
static void print_bad_reply_type(iphdr_t* ip);

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
    return 0;
}

static ssize_t send_ping(int sock_fd, ping_params_t* ping_params,
                         struct timeval* start_tv) {
    char* ping_message;
    ssize_t message_len;
    ssize_t ret;

    message_len = sizeof(struct icmphdr) + ping_params->packet_size;
    ping_message = get_ping_packet(ping_params->packet_size, ping_params->seq);
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
    struct icmphdr *icmp_hdr;

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
    icmp_hdr = (struct icmphdr*) (buffer + sizeof(iphdr_t));
    if (icmp_hdr->type == ICMP_ECHO
        || ntohs(icmp_hdr->un.echo.sequence) != ping_params->seq) {
        free(buffer);
        return receive_ping(sock_fd, ping_params, end_tv);
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
        printf("Bad checksum for reply icmp_seq %d\n",
               ntohs(icmp_hdr->un.echo.sequence));
        return 1;
    }
    if (icmp_hdr->type == ICMP_ECHOREPLY) {
        printf("%zu bytes from %s (%s): icmp_seq=%d", message_len, ping_params->fqdn,
               ping_params->ip, ntohs(icmp_hdr->un.echo.sequence));
#ifdef __APPLE__
        printf(" ttl=%d", ip_hdr->ip_ttl);
#endif
#ifdef __linux__
        printf(" ttl=%d", ip_hdr->ttl);
#endif
        print_timestamp(timestamp);
        printf("\n");
    } else {
        if (ping_params->verbose) {
            print_bad_reply_type(ip_hdr);
        } else if ((ping_params->linger && ping_params->linger >= 1.)
                   || !ping_params->linger) {
            printf("Bad type for reply icmp_seq %d\n", ping_params->seq);
        }
        return 1;
    }
    return 0;
}

#ifdef __APPLE__
static void print_bad_reply_type(iphdr_t* ip) {
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
    printf(" %1x  %1x  %02x %04x %04x",
           ip->ip_v, ip->ip_hl, ip->ip_tos, ip->ip_len, ip->ip_id);
    printf("   %1x %04x", ((ip->ip_off) & 0xe000) >> 13,
           (ip->ip_off) & 0x1fff);
    printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ip->ip_sum);
    printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_src));
    printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_dst));
    printf("\n");
}
#endif

#ifdef __linux__
static void print_bad_reply_type(iphdr_t* ip) {
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
    printf(" %1x  %1x  %02x %04x %04x",
           ip->version, ip->ihl, ip->tos, ip->tot_len, ip->id);
    printf("   %1x %04x", ((ip->frag_off) & 0xe000) >> 13,
           (ip->frag_off) & 0x1fff);
    printf("  %02x  %02x %04x", ip->ttl, ip->protocol, ip->check);
    printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->saddr));
    printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->daddr));
    printf("\n");
}
#endif
