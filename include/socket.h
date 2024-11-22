#ifndef SOCKET_H
#define SOCKET_H

#include "ping.h"

int init_socket(ping_data_t ping_data);
void resolve_host(const char *host, ping_data_t *ping_data);

#endif //SOCKET_H
