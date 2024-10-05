//  Copyright (c) 2024 Ethan Delage
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "ft_ping.h"

int resolve_host(const char* host, ping_params_t* ping_params) {
    struct addrinfo hints;
    struct addrinfo *res;
    int error_code;
    char fqdn[NI_MAXHOST];

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = IPPROTO_ICMP;

    error_code = getaddrinfo(host, NULL, &hints, &res);
    if (error_code != 0) {
        dprintf(STDERR_FILENO, "ping: getaddrinfo: %s\n", gai_strerror(error_code));
        return error_code;
    }
    ping_params->sock_addr = *(struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &ping_params->sock_addr.sin_addr, ping_params->ip, sizeof(ping_params->ip));
    error_code = getnameinfo(res->ai_addr, res->ai_addrlen, ping_params->fqdn, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
    freeaddrinfo(res);
    if (error_code != 0) {
        dprintf(STDERR_FILENO, "ping: getnameinfo: %s\n", gai_strerror(error_code));
        return error_code;
    }
    printf("FQDN: %s\n", fqdn);
    return 0;
}

