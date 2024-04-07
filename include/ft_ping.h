//  Copyright (c) 2024 Ethan Delage

#ifndef FT_PING_H_
#define FT_PING_H_

#include <stdbool.h>
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

#define OPTIONS_WITHOUT_ARG "v"
#define OPTIONS_WITH_ARG    ""

#define USAGE_MESSAGE   "usage: ./ft_ping [-v] host\n"
#define INVALID_OPTION_MESSAGE  "ping: invalid option -- "
#define OPTION_REQUIRES_ARG_MESSAGE "ping: option requires an argument --"

#define DEFAULT_PACKET_SIZE 56

typedef struct ping_params_s {
    char*   host;
    char*   ip;
    struct sockaddr_in sock_addr;
    bool    verbose;
    size_t  packet_size;
    u_int16_t seq;
} ping_params_t;

bool validate_params(int argc, char** argv, ping_params_t* ping_params);
bool validate_option(int argc, char** argv, int* index,
                     ping_params_t* ping_params);

int init_socket();
int resolve_host(const char* host, ping_params_t* ping_params);

char* get_ping_message(size_t packet_size, uint16_t seq);
int icmp_ping(int sock_fd, ping_params_t* ping_params);

long get_timestamp(struct timeval start_tv, struct timeval end_tv);

void print_params(ping_params_t ping_params);
void print_timestamp(long timestamp);

#endif //FT_PING_H_
