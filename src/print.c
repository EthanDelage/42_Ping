#include "print.h"
#include "icmp.h"

#include <math.h>
#include <stdio.h>
#include <arpa/inet.h>

#define NITEMS(p) sizeof(p)/sizeof((p)[0])

typedef struct icmp_code_description_s {
    int type;
    int code;
    char *diag;
} icmp_code_description_t;

icmp_code_description_t icmp_code_description[] = {
        {ICMP_DEST_UNREACH, ICMP_NET_UNREACH, "Destination Net Unreachable"},
        {ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, "Destination Host Unreachable"},
        {ICMP_DEST_UNREACH, ICMP_PROT_UNREACH, "Destination Protocol Unreachable"},
        {ICMP_DEST_UNREACH, ICMP_PORT_UNREACH, "Destination Port Unreachable"},
        {ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, "Fragmentation needed and DF set"},
        {ICMP_DEST_UNREACH, ICMP_SR_FAILED, "Source Route Failed"},
        {ICMP_DEST_UNREACH, ICMP_NET_UNKNOWN, "Network Unknown"},
        {ICMP_DEST_UNREACH, ICMP_HOST_UNKNOWN, "Host Unknown"},
        {ICMP_DEST_UNREACH, ICMP_HOST_ISOLATED, "Host Isolated"},
        {ICMP_DEST_UNREACH, ICMP_NET_UNR_TOS, "Destination Network Unreachable At This TOS"},
        {ICMP_DEST_UNREACH, ICMP_HOST_UNR_TOS, "Destination Host Unreachable At This TOS"},
        {ICMP_REDIRECT, ICMP_REDIR_NET, "Redirect Network"},
        {ICMP_REDIRECT, ICMP_REDIR_HOST, "Redirect Host"},
        {ICMP_REDIRECT, ICMP_REDIR_NETTOS, "Redirect Type of Service and Network"},
        {ICMP_REDIRECT, ICMP_REDIR_HOSTTOS, "Redirect Type of Service and Host"},
        {ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, "Time to live exceeded"},
        {ICMP_TIME_EXCEEDED, ICMP_EXC_FRAGTIME, "Frag reassembly time exceeded"}};

static void print_icmp_description(const ping_response_t *ping_response, bool verbose);
static void print_ip_data(const ping_response_t *ping_response);
static void print_ip_header(struct ip *ip);
static char *get_icmp_description(int type, int code);

void print_ping_info(const ping_data_t *ping_data) {
    printf("PING %s (%s): %zu data bytes",
           ping_data->opt.host, inet_ntoa(ping_data->sock_addr.sin_addr), ping_data->opt.packet_size);
    if (ping_data->opt.verbose) {
        printf(", id 0x%04x = %d", ping_data->id, ping_data->id);
    }
    printf("\n");
}

void print_response(const ping_response_t *ping_response, bool verbose) {
    if (ping_response->type != ICMP_ECHOREPLY) {
        return print_icmp_description(ping_response, verbose);
    }
    printf(
           "%zu bytes from %s: icmp_seq=%d ttl=%d",
           ping_response->packet_size - sizeof(iphdr_t),
           inet_ntoa(*(struct in_addr *)&ping_response->sock_addr.sin_addr.s_addr),
           ntohs(ping_response->seq),
           ping_response->ttl
    );
    print_trip_time(ping_response->trip_time);
    printf("\n");
}

void print_rtt(const ping_rtt_t *rtt) {
    double trip_time_avg;
    double trip_time_stddev;
    struct timeval end_tv;

    gettimeofday(&end_tv, NULL);
    printf("--- %s ping statistics ---\n", rtt->host);
    printf("%zu packets transmitted, ", rtt->n_transmitted);
    printf("%zu received", rtt->n_received);
    if (rtt->n_transmitted) {
        printf(", %g%% packet loss,",
               (((double) (rtt->n_transmitted - rtt->n_received) * 100.0)
                / rtt->n_transmitted));
    }
    printf("\n");
    if (rtt->n_received) {
        trip_time_avg = (double) rtt->trip_time_sum / rtt->n_received;
        trip_time_stddev = sqrt(((double) rtt->trip_time_square_sum / rtt->n_received)
                                - (trip_time_avg * trip_time_avg));
        printf(
                "rtt min/avg/max/mdev = "
                "%ld.%03ld/%lu.%03ld/%ld.%03ld/%ld.%03ld ms\n",
                (long)rtt->min_trip_time / 1000, (long)rtt->min_trip_time % 1000,
                (long)(trip_time_avg / 1000), (long)(trip_time_avg) % 1000,
                (long)rtt->max_trip_time / 1000, (long)rtt->max_trip_time % 1000,
                (long)trip_time_stddev / 1000, (long)trip_time_stddev % 1000
        );
    }
}

void print_trip_time(long timestamp) {
    if (timestamp >= 100000 - 50)
        printf(" time=%ld ms", (timestamp + 500) / 1000);
    else if (timestamp >= 10000 - 5)
        printf(" time=%ld.%01ld ms", (timestamp + 50) / 1000,
               ((timestamp + 50) % 1000) / 100);
    else if (timestamp >= 1000)
        printf(" time=%ld.%02ld ms", (timestamp + 5) / 1000,
               ((timestamp + 5) % 1000) / 10);
    else
        printf(" time=%ld.%03ld ms", timestamp / 1000, timestamp % 1000);
}

static void print_icmp_description(const ping_response_t *ping_response, bool verbose) {
    char host[INET_ADDRSTRLEN];
    int ret;

    ret = getnameinfo((struct sockaddr *)&ping_response->sock_addr, sizeof(ping_response->sock_addr), host, INET_ADDRSTRLEN, NULL, 0, 0);
    printf("%zu bytes ", ping_response->packet_size - sizeof(iphdr_t));
    if (ret == 0) {
        printf("from %s (%s): ", host, inet_ntoa(ping_response->sock_addr.sin_addr));
    } else {
        printf("from %s: ", inet_ntoa(ping_response->sock_addr.sin_addr));
    }
    printf("%s\n", get_icmp_description(ping_response->type, ping_response->code));
    if (verbose == true) {
        print_ip_data(ping_response);
    }
}

static void print_ip_data(const ping_response_t *ping_response) {
    int hlen;
    unsigned char *cp;
    struct ip *ip;

    ip = (struct ip *)(ping_response->packet + sizeof(iphdr_t) + sizeof(struct icmphdr));
    print_ip_header(ip);

    hlen = ip->ip_hl << 2;
    cp = (unsigned char *)ip + hlen;

    if (ip->ip_p == IPPROTO_TCP)
        printf("TCP: from port %u, to port %u (decimal)\n", (*cp * 256 + *(cp + 1)),
               (*(cp + 2) * 256 + *(cp + 3)));
    else if (ip->ip_p == IPPROTO_UDP)
        printf("UDP: from port %u, to port %u (decimal)\n", (*cp * 256 + *(cp + 1)),
               (*(cp + 2) * 256 + *(cp + 3)));
    else if (ip->ip_p == IPPROTO_ICMP) {
        int type = *cp;
        int code = *(cp + 1);

        printf("ICMP: type %u, code %u, size %u", type, code,
               ntohs(ip->ip_len) - hlen);
        if (type == ICMP_ECHOREPLY || type == ICMP_ECHO)
            printf(", id 0x%04x, seq 0x%04x", *(cp + 4) * 256 + *(cp + 5),
                   *(cp + 6) * 256 + *(cp + 7));
        printf("\n");
    }

}

static void print_ip_header(struct ip *ip) {
    size_t hlen;
    unsigned char *cp;

    hlen = ip->ip_hl << 2;
    cp = (unsigned char *)ip + sizeof(*ip); /* point to options */

    size_t j;

    printf("IP Hdr Dump:\n ");
    for (j = 0; j < sizeof(*ip); ++j)
        printf("%02x%s", *((unsigned char *)ip + j),
               (j % 2) ? " " : ""); /* Group bytes two by two.  */
    printf("\n");

    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
    printf(" %1x  %1x  %02x", ip->ip_v, ip->ip_hl, ip->ip_tos);
    printf(" %04x %04x", (ip->ip_len > 0x2000) ? ntohs(ip->ip_len) : ip->ip_len,
           ntohs(ip->ip_id));
    printf("   %1x %04x", (ntohs(ip->ip_off) & 0xe000) >> 13,
           ntohs(ip->ip_off) & 0x1fff);
    printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ntohs(ip->ip_sum));
    printf(" %s ", inet_ntoa(ip->ip_src));
    printf(" %s ", inet_ntoa(ip->ip_dst));
    while (hlen-- > sizeof(*ip))
        printf("%02x", *cp++);

    printf("\n");
}

static char *get_icmp_description(int type, int code) {
    icmp_code_description_t *p;
    for (p = icmp_code_description; p < icmp_code_description + NITEMS(icmp_code_description); p++) {
        if (p->type == type && p->code == code) {
            return p->diag;
        }
    }
    return NULL;
}
