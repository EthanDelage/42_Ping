//  Copyright (c) 2024 Ethan Delage

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "ft_ping.h"

int init_socket(const ping_params_t* ping_params) {
    int sock_fd;
    struct timeval tv;

    sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_fd < 0) {
        dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        return -1;
    }
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ping_params->so_debug) {
        setsockopt(sock_fd, SOL_SOCKET, SO_DEBUG, &tv, sizeof(tv));
    }
    return sock_fd;
}
