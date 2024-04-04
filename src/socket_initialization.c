//  Copyright (c) 2024 Ethan Delage

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

int init_socket() {
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_fd < 0) {
        dprintf(STDERR_FILENO, "ping: %s\n", strerror(errno));
        return -1;
    }
    return sock_fd;
}
