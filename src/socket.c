#include "socket.h"

#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

static void set_sockopt_debug(int sock_fd);
static void set_sockopt_broadcast(int sock_fd);

int init_socket(ping_data_t ping_data) {
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_fd < 0) {
        error(EXIT_FAILURE, errno, "socket creation failed");
    }
    if (ping_data.opt.debug == true) {
        set_sockopt_debug(sock_fd);
    }
    set_sockopt_broadcast(sock_fd);
    return sock_fd;
}

static void set_sockopt_debug(int sock_fd) {
    int one = 1;

    if (setsockopt(sock_fd, SOL_SOCKET, SO_DEBUG, (char *) &one, sizeof(one)) < 0) {
        error(0, errno, "setsockopt SO_DEBUG (ignored)");
        errno = 0;
    }
}

static void set_sockopt_broadcast(int sock_fd) {
    int one = 1;

    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (char *) &one, sizeof(one)) < 0) {
        error(EXIT_FAILURE, errno, "setsockopt SO_BROADCAST");
    }
}

void resolve_host(const char *host, ping_data_t *ping_data) {
    struct addrinfo hints = {0};
    struct addrinfo *res;
    int status;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    status = getaddrinfo(host, NULL, &hints, &res);
    if (status != 0) {
        error(EXIT_FAILURE, 0, "unknown host");
    }
    ping_data->sock_addr = *(struct sockaddr_in *)res->ai_addr;
    ping_data->ip = inet_ntoa(ping_data->sock_addr.sin_addr);
    freeaddrinfo(res);
}
