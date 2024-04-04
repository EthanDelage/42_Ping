//  Copyright (c) 2024 Ethan Delage
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ft_ping.h"

int resolve_host(const char* host, ping_params_t* ping_params) {
    struct hostent* host_info;
    struct in_addr* host_addr;

    host_info = gethostbyname(host);
    if (host_info == NULL) {
        dprintf(STDERR_FILENO,
                "ping: cannot resolve test: %s\n",
                hstrerror(h_errno));
        return h_errno;
    }
    host_addr = (struct in_addr*) host_info->h_addr_list[0];
    ping_params->sock_addr.sin_addr = *host_addr;
    ping_params->sock_addr.sin_family = AF_INET;
    ping_params->ip = inet_ntoa(*host_addr);
    return 0;
}

