#include "ping.h"
#include "rtt.h"
#include "socket.h"
#include "icmp.h"
#include "common.h"
#include "print.h"

#include <unistd.h>
#include <stdio.h>
#include <netinet/ip_icmp.h>
#include <error.h>
#include <errno.h>
#include <arpa/inet.h>

extern ping_rtt_t rtt_g;
extern int sock_fd_g;

static void send_ping(int sock_fd, ping_data_t *ping_data);
static ping_response_t receive_ping(int sock_fd, ping_data_t *ping_data);
static int process_response(ping_response_t *ping_response, ping_data_t *ping_data);
static int ping_select_handler(int sock_fd, ping_data_t *ping_data);
static struct timeval calculate_select_timeout(ping_data_t *ping_data);
static void stop_ping();

void init_ping(ping_data_t *ping_data) {
    resolve_host(ping_data->opt.host, ping_data);
    ping_data->id = getpid();
    ping_data->icmp_type = ICMP_ECHO;
    sock_fd_g = init_socket(*ping_data);
    atexit(stop_ping);
}

void run_ping(ping_data_t *ping_data) {
    size_t n_resp = 0;
    ping_response_t ping_response;

    print_ping_info(ping_data);
    send_ping(sock_fd_g, ping_data);
    while (1) {
        if (ping_select_handler(sock_fd_g, ping_data)) {
            ping_response = receive_ping(sock_fd_g, ping_data);
            if (process_response(&ping_response, ping_data) == 0) {
                print_response(&ping_response, ping_data->opt.verbose);
                n_resp++;
            }
            free(ping_response.packet);
        } else {
            send_ping(sock_fd_g, ping_data);
        }
        if (ping_data->opt.count != 0 && n_resp == ping_data->opt.count) {
            break ;
        }
    }
}

static void send_ping(int sock_fd, ping_data_t *ping_data) {
    struct timeval send_tv;
    char *packet;
    ssize_t packet_len;
    ssize_t ret;

    packet = create_icmp_packet(ping_data);
    packet_len = sizeof(struct icmphdr) + ping_data->opt.packet_size;

    gettimeofday(&send_tv, NULL);
    ret = sendto(sock_fd, packet, packet_len, 0,
                 (struct sockaddr *) &ping_data->sock_addr, sizeof(struct sockaddr_in));
    free(packet);
    if (ret < 0) {
        error(EXIT_FAILURE, errno, "sending packet");
    } else if (ret != packet_len) {
        error(0, errno, "wrote %s %zu chars, ret=%zu", ping_data->opt.host, packet_len, ret);
    }
    if (ping_data->seq == 0) {
        ping_data->start_tv = send_tv;
    }
    ping_data->last_send_tv = send_tv;
    ping_data->seq++;
    rtt_g.n_transmitted++;
}

static ping_response_t receive_ping(int sock_fd, ping_data_t *ping_data) {
    ping_response_t ping_response = {0};
    size_t buffer_len;
    socklen_t addr_len = sizeof(ping_response.sock_addr);
    ssize_t ret;

    buffer_len = RESPONSE_OFFSET + sizeof(struct icmphdr) + ping_data->opt.packet_size;
    ping_response.packet = malloc(buffer_len);
    if (ping_response.packet == NULL) {
        error(EXIT_FAILURE, errno, "malloc failed");
    }
    ret = recvfrom(
            sock_fd,
            ping_response.packet,
            buffer_len,
            0,
            (struct sockaddr *) &ping_response.sock_addr,
            &addr_len
    );
    gettimeofday(&ping_response.receive_tv, NULL);
    if (ret < 0) {
        free(ping_response.packet);
        error(EXIT_FAILURE, errno, "recvfrom failed");
    }
    ping_response.packet_size = ret;
    return ping_response;
}

/**
 * @return 0 if response is printable, -1 otherwise
 */
static int process_response(ping_response_t *ping_response, ping_data_t *ping_data) {
    iphdr_t *ip_hdr;
    struct icmphdr *icmp_hdr;
    u_int16_t checksum;

    ip_hdr = (iphdr_t *) ping_response->packet;
#ifdef __APPLE__
    ping_response->ttl = ip_hdr->ip_ttl;
#endif
#ifdef __linux__
    ping_response->ttl = ip_hdr->ttl;
#endif
    icmp_hdr = (struct icmphdr *) (ping_response->packet + sizeof(iphdr_t));
    ping_response->type = icmp_hdr->type;
    ping_response->code = icmp_hdr->code;
    checksum = icmp_hdr->checksum;
    icmp_hdr->checksum = 0;
    if (calculate_checksum(icmp_hdr, ping_response->packet_size - sizeof(iphdr_t)) != checksum) {
        error(0, 0, "checksum mismatch from %s", inet_ntoa(ping_response->sock_addr.sin_addr));
        return -1;
    }
    if (ping_response->type != ICMP_ECHOREPLY) {
        icmp_hdr = (struct icmphdr *) (ping_response->packet + sizeof(iphdr_t) + sizeof(struct icmphdr) + sizeof(iphdr_t));
    }
    ping_response->id = icmp_hdr->un.echo.id;
    ping_response->seq = icmp_hdr->un.echo.sequence;
    if (ping_response->id != ping_data->id) {
        return -1;
    }
    if (ping_response->type == ICMP_ECHOREPLY) {
        ping_response->trip_time = calculate_trip_time(ping_data->last_send_tv, ping_response->receive_tv);
        update_rtt(ping_response->trip_time);
    } else if (ping_response->type == ICMP_ECHO) {
        return -1;
    }
    return 0;
}

/**
 * @param sock_fd the socket fd which is watched to see if it is ready for reading
 * @return 1 if sock_fd is readable, and 0 if the timeout expired
 */
static int ping_select_handler(int sock_fd, ping_data_t *ping_data) {
    struct timeval timeout_tv;
    fd_set read_fds;
    int ret;

    FD_ZERO(&read_fds);
    FD_SET(sock_fd, &read_fds);
    timeout_tv = calculate_select_timeout(ping_data);
    ret = select(sock_fd + 1, &read_fds, NULL, NULL, &timeout_tv);
    if (ret < 0) {
        error(EXIT_FAILURE, errno, "select failed");
    } else if (ret > 0) {
        return 1;
    }
    return 0;
}

static struct timeval calculate_select_timeout(ping_data_t *ping_data) {
    struct timeval current_tv;
    struct timeval interval_tv;
    struct timeval timeout_tv;

    if (ping_data->opt.count == 0 || ping_data->opt.count != rtt_g.n_transmitted) {
        interval_tv = ping_data->opt.interval_tv;
    } else {
        interval_tv = ping_data->opt.linger_tv;
    }
    gettimeofday(&current_tv, NULL);
    timeout_tv.tv_sec = ping_data->last_send_tv.tv_sec + interval_tv.tv_sec - current_tv.tv_sec;
    timeout_tv.tv_usec = ping_data->last_send_tv.tv_usec + interval_tv.tv_usec - current_tv.tv_usec;
    normalize_timeval(&timeout_tv);
    if (timeout_tv.tv_sec < 0) {
        timeout_tv.tv_sec = 0;
        timeout_tv.tv_usec = 0;
    }
    return timeout_tv;
}

static void stop_ping() {
    print_rtt(&rtt_g);
    close(sock_fd_g);
}