//  Copyright (c) 2024 Ethan Delage

#ifndef FT_PING_H_
#define FT_PING_H_

#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/time.h>

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

#define OPTIONS_WITHOUT_ARG "dv?"
#define OPTIONS_WITH_ARG    "cisW"

#define USAGE_MESSAGE   "usage: ./ft_ping [-dv?] [-c count] [-i interval] " \
                        "[-s packet_size] [-W wait_time] host\n"
#define INVALID_OPTION_MESSAGE  "ping: invalid option -- "
#define OPTION_REQUIRES_ARG_MESSAGE "ping: option requires an argument -- "

#define DEFAULT_PACKET_SIZE 56
#define DEFAULT_INTERVAL 1

typedef struct ping_params_s {
    char*   host;
    char    ip[INET_ADDRSTRLEN];
    char    fqdn[NI_MAXHOST];
    struct sockaddr_in sock_addr;

    size_t  packet_size;
    u_int16_t seq;
    u_int16_t id;

    bool    verbose;
    bool    help;
    bool    so_debug;
    bool    count;
    long    count_arg;
    bool    linger;
    double  linger_arg;
    double interval;
} ping_params_t;

// ping runtime state
typedef struct ping_rts_s {
    struct timeval  start_tv;
    char* host;
    int sock_fd;
    size_t n_transmitted;
    size_t n_received;
    long min_timestamp;
    long max_timestamp;
    long last_timestamp;
    double timestamp_sum;
    double timestamp_square_sum;
} ping_rts_t;

bool validate_params(int argc, char** argv, ping_params_t* ping_params);
bool validate_option(int argc, char** argv, int* index,
                     ping_params_t* ping_params);

int init_socket(const ping_params_t* ping_params);
int resolve_host(const char* host, ping_params_t* ping_params);

char* get_ping_packet(ping_params_t ping_params);
int icmp_ping(int sock_fd, ping_params_t* ping_params);
u_int16_t get_checksum(void* data, size_t len);

long get_timestamp(struct timeval start_tv, struct timeval end_tv);
int add_timestamp(long timestamp);

void print_params(ping_params_t ping_params);
void print_timestamp(long timestamp);

#endif //FT_PING_H_
