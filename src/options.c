#include "options.h"
#include "common.h"
#include "rtt.h"

#include <argp.h>
#include <stdlib.h>

extern ping_rtt_t rtt_g;

const char *args_doc = "HOST ...";
const char *doc = "Send ICMP ECHO_REQUEST packets to network hosts.";

static struct argp_option options[] = {
        {"count", 'c', "NUMBER", 0, "stop_ping after sending NUMBER packets", 0},
        {"debug", 'd', NULL, 0, "set the SO_DEBUG option", 0},
        {"interval", 'i', "NUMBER", 0, "wait NUMBER seconds between sending each packet", 0},
        {"verbose", 'v', 0, 0, "verbose output", 0},
        {"linger", 'W', "N", 0, "number of seconds to wait for response", 0},
        {"size", 's', "NUMBER", 0, "send NUMBER data octets", 0},
        {"ttl", OPT_TTL_KEY, "N", 0, "specify N as time-to-live", 0},
        {0}
};

static error_t argp_parser(int key, char *arg, struct argp_state *state);

void parse_opt(int argc, char **argv, ping_option_t *ping_opt) {
    struct argp argp = {options, argp_parser, args_doc, doc, 0, 0, 0};

    ping_opt->linger_tv.tv_sec = DEFAULT_LINGER_SEC;
    ping_opt->interval_tv = convert_double_to_timeval(DEFAULT_INTERVAL);
    ping_opt->packet_size = DEFAULT_PACKET_SIZE;
    argp_parse(&argp, argc, argv, 0, NULL, ping_opt);
}

static error_t argp_parser(int key, char *arg, struct argp_state *state) {
    ping_option_t *ping_opt = state->input;

    switch (key) {
        case 'c':
            ping_opt->count = convert_arg_to_size_t(arg, 0, 1);
            break;
        case 'd':
            ping_opt->debug = true;
            break;
        case 'i':
            ping_opt->interval_tv = convert_double_to_timeval(convert_arg_to_double(arg));
            break;
        case 'v':
            ping_opt->verbose = true;
            break;
        case 'W':
            ping_opt->linger_tv.tv_sec = (time_t) convert_arg_to_size_t(arg, INT_MAX, 1);
            break;
        case 's':
            ping_opt->packet_size = convert_arg_to_size_t(arg, MAX_PACKET_SIZE, 1);
            break;
        case OPT_TTL_KEY:
            ping_opt->ttl = (int) convert_arg_to_size_t(arg, 255, 0);
            break;
        case ARGP_KEY_ARG:
            ping_opt->host = arg;
            rtt_g.host = arg;
            break;
        case ARGP_KEY_NO_ARGS:
            argp_error(state, "missing host operand");
            // FALLTHROUGH
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}