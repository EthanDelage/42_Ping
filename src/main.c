#include "options.h"
#include "ping.h"
#include "socket.h"

#include <errno.h>
#include <signal.h>
#include <error.h>

int sock_fd_g = -1;

static void sigint_handler(int signum);

int main(int argc, char **argv) {
    ping_data_t ping_data = {0};
    parse_opt(argc, argv, &ping_data.opt);
    init_ping(&ping_data);
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        error(EXIT_FAILURE, errno, "signal");
    }
    run_ping(&ping_data);
    return 0;
}

static void sigint_handler(int signum) {
    (void) signum;
    exit(errno);
}