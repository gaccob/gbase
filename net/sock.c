#include <assert.h>
#include <string.h>
#include "sock.h"

inline sock_t
sock_tcp() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

inline sock_t
sock_udp() {
    return socket(AF_INET, SOCK_DGRAM, 0);
}

//  timeout default 1 second
int
sock_nonblock_connect(sock_t sock, const char* ip_str, uint16_t port) {
    // nonblock
    if (sock < 0 || sock_set_nonblock(sock) < 0)
        return -1;

    // parse address
    sockaddrin_t addr;
    socklen_t addr_len = sizeof(addr);
    if (sock_addr_aton(ip_str, port, &addr) < 0)
        return -1;

    // try connect
    int ret = connect(sock, (const sockaddr_t*)&addr, addr_len);
    if (ret < 0) {
        if (EINPROGRESS != errno && EWOULDBLOCK != errno)
            return errno;

        // try select
        fd_set read_events, write_events, exec_events;
        FD_ZERO(&read_events);
        FD_SET(sock, &read_events);
        write_events = exec_events = read_events;

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        ret = select(sock + 1, &read_events, &write_events, &exec_events, &tv);
        if (ret <= 0)
            return ret;
        if (!FD_ISSET(sock, &read_events) && !FD_ISSET(sock, &write_events))
            return -1;

        // make sure socket no error
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
        if (0 != err)
            return err;
    }
    return 0;
}

inline int
sock_listen(sock_t sock, sockaddr_t* addr) {
    // reuse listen address
    if (sock_set_reuseaddr(sock) < 0)
        return -1;
    if (bind(sock, addr, sizeof(sockaddr_t)) < 0)
        return -1;
    return listen(sock, 1024);
}

inline sock_t
sock_accept(sock_t sock, sockaddr_t* addr) {
    socklen_t addr_len = sizeof(sockaddr_t);
    return accept(sock, addr, &addr_len);
}

inline int
sock_close(sock_t sock) {
    if (sock < 0)
        return -1;
    int ret = 0;
    do {
        ret = close(sock);
    } while (ret < 0 && errno == EINTR);
    return ret;
}

inline int
sock_set_nonblock(sock_t sock) {
    if (sock < 0)
        return -1;
    int flags;
    if ((flags = fcntl(sock, F_GETFL, NULL)) < 0)
        return -1;
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
        return -1;
    return 0;
}

inline int
sock_set_block(sock_t sock) {
    if (sock < 0)
        return -1;
    int flags;
    if ((flags = fcntl(sock, F_GETFL, NULL)) < 0)
        return -1;
    if (fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) < 0)
        return -1;
    return 0;
}

inline int
sock_set_reuseaddr(sock_t sock) {
    if (sock < 0)
        return -1;
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, optlen);
}

inline int
sock_set_nodelay(sock_t sock) {
    if (sock < 0)
        return -1;
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, optlen);
}

inline int
sock_set_sndbuf(sock_t sock, int size) {
    if (sock < 0)
        return -1;
    return setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const void*)&size, sizeof(size));
}

inline int
sock_set_rcvbuf(sock_t sock, int size) {
    if (sock < 0)
        return -1;
    return setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const void*)&size, sizeof(size));
}

//  ipv4 only
static int
_sock_inet_aton(const char *ip_str, struct in_addr *addr) {
    if (!ip_str || !addr)
        return -1;
    int a, b, c, d;
    char more;
    if (sscanf(ip_str, "%d.%d.%d.%d%c", &a, &b, &c, &d, &more)  !=  4)
        return -1;
    if (a < 0 || a > 255) return -1;
    if (b < 0 || b > 255) return -1;
    if (c < 0 || c > 255) return -1;
    if (d < 0 || d > 255) return -1;
    addr->s_addr = htonl((a<<24) | (b<<16) | (c<<8) | d);
    return 0;
}

//  ipv4 only
static int
_sock_inet_ntoa(const struct in_addr* addr, char* ip_str, size_t len) {
    if (!ip_str || !addr)
        return -1;
    uint32_t a = ntohl(addr->s_addr);
    int ret = snprintf(ip_str, len, "%d.%d.%d.%d",
        (uint8_t)((a >> 24) & 0xff),
        (uint8_t)((a >> 16) & 0xff),
        (uint8_t)((a >> 8) & 0xff),
        (uint8_t)(a & 0xff));
    if (ret < 0 || (size_t)ret >= len)
        return -1;
    return 0;
}

//  ipv4 only
int
sock_addr_aton(const char* ip_str, uint16_t port, sockaddrin_t* addr) {
    if (!ip_str || !addr)
        return -1;
    memset(addr, 0, sizeof(sockaddrin_t));
    addr->sin_family = AF_INET;
    if (_sock_inet_aton(ip_str, &addr->sin_addr) < 0)
        return -1;
    addr->sin_port = htons(port);
    return 0;
}

//  ipv4 only
int
sock_addr_ntoa(const sockaddrin_t* addr, char* addr_str, size_t len) {
    if (!addr_str || !addr)
        return -1;
    if (_sock_inet_ntoa(&addr->sin_addr, addr_str, len) < 0)
        return -1;
    size_t used = strlen(addr_str);
    assert(len >= used);
    int ret = snprintf(addr_str + used, len - used, ":%d", ntohs(addr->sin_port));
    if (ret < 0 || (size_t)ret >= len - used)
        return -1;
    return 0;
}

