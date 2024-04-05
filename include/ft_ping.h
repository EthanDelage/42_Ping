//  Copyright (c) 2024 Ethan Delage

#ifndef FT_PING_H_
#define FT_PING_H_

#include <stdbool.h>
#include <netinet/in.h>

#ifdef __APPLE__

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

#define OPTIONS_WITHOUT_ARG "v"
#define OPTIONS_WITH_ARG    ""

#define USAGE_MESSAGE "usage: ./ft_ping [-v] host\n"
#define INVALID_OPTION_MESSAGE  "ping: invalid option -- "
#define OPTION_REQUIRES_ARG_MESSAGE "ping: option requires an argument --"

typedef struct ping_params_s {
    char*   host;
    char*   ip;
    struct sockaddr_in sock_addr;
    bool    verbose;
} ping_params_t;

bool validate_params(int argc, char** argv, ping_params_t* ping_params);
bool validate_option(int argc, char** argv, int* index,
                     ping_params_t* ping_params);

int init_socket();
int resolve_host(const char* host, ping_params_t* ping_params);

char* get_ping_message(size_t packet_size);

void print_params(ping_params_t ping_params);

#endif //FT_PING_H_
