#include <assert.h>
#include <string.h>
#include "sock.h"

//  create tcp socket
//  return >= 0, success
//  return < 0, fail
sock_t sock_tcp()
{
#if defined(OS_WIN)
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return socket(AF_INET, SOCK_STREAM, 0);
#endif
    return -1;
}

//  create udp socket
//  return >= 0, success
//  return < 0, fail
sock_t sock_udp()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}

//  connect, timeout default = 1 second
//  return =0, success
//  return <0, fail
int sock_connect(sock_t sock, const char* ip_str, uint16_t port)
{
    struct sockaddr_in addr;
    socklen_t addr_len, len;
    int ret, err;
    fd_set read_events, write_events, exec_events;
    struct timeval tv;

    if(sock < 0) return -1;
    if(sock_set_nonblock(sock) < 0) return -1;

    // set buffer size, before connect
    if(sock_set_sndbuf(sock, SOCK_SNDBUF_SIZE) < 0
        || sock_set_rcvbuf(sock, SOCK_RCVBUF_SIZE) < 0)
        return -1;

    // parse address
    addr_len = sizeof(addr);
    if(sock_addr_aton(ip_str, port, &addr) < 0)
        return -1;

    // try connect
    ret = connect(sock, (const struct sockaddr*)&addr, addr_len);
    if(ret < 0)
    {
        // connect fail
        if(ERR_EINPROGRESS != ERRNO && ERR_EWOULDBLOCK != ERRNO)
            return ret;

        // try select
        FD_ZERO(&read_events);
        FD_SET(sock, &read_events);
        write_events = read_events;
        exec_events = read_events;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        ret = select(sock + 1, &read_events, &write_events, &exec_events, &tv);
        if(ret <= 0)
            return ret;
        if(!FD_ISSET(sock, &read_events) && !FD_ISSET(sock, &write_events))
            return -1;

        // make sure socket no error
        len = sizeof(err);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
        if(0 != err) return -1;
    }

    return 0;
}

int sock_listen(sock_t sock, struct sockaddr* addr)
{
    // reuse listen address
    if(sock_set_reuseaddr(sock) < 0)
        return -1;

    // set buffer size, before listen
    if(sock_set_sndbuf(sock, SOCK_SNDBUF_SIZE) < 0
        || sock_set_rcvbuf(sock, SOCK_RCVBUF_SIZE) < 0)
        return -1;

    if(bind(sock, addr, sizeof(struct sockaddr)) < 0)
        return -1;
    return listen(sock, 1024);
}

int sock_accept(sock_t sock, struct sockaddr* addr)
{
    socklen_t addr_len = sizeof(struct sockaddr);
    return accept(sock, addr, &addr_len);
}

//  return =0, success
//  return <0, fail
int sock_close(sock_t sock)
{
    if(sock < 0) return -1;

#if defined(OS_WIN)
    return closesocket(sock);
#elif defined(OS_LINUX) || defined(OS_MAC)
    int ret = 0;
    do{
        ret = close(sock);
    }while((ret == -1) && (ERRNO == ERR_EINTR));
    return ret;
#endif
    return -1;
}

//  return >0:    success read byts
//  return <=0:    read fail
int sock_read(sock_t sock, char* buffer, size_t buffer_len)
{
#if defined(OS_WIN)
    return recv(sock, buffer, buffer_len, 0);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return read(sock, buffer, buffer_len);
#endif
    return -1;
}

//  return >0:    success write byts
//  return <=0:    write fail
int sock_write(sock_t sock, const char* buffer, size_t buffer_len)
{
#if defined(OS_WIN)
    return send(sock, buffer, buffer_len, 0);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return write(sock, buffer, buffer_len);
#endif
    return -1;
}

//  return >0:    success recv byts
//  return <=0:    read fail
int sock_recvfrom(sock_t sock, char* buffer, size_t buffer_len, struct sockaddr* addr, socklen_t addrlen)
{
    return recvfrom(sock, buffer, buffer_len, 0, addr, &addrlen);
}

//  return >0:    success send byts
//  return <=0:    write fail
int sock_sendto(sock_t sock, const char* buffer, size_t buffer_len, struct sockaddr* addr, socklen_t addrlen)
{
    return sendto(sock, buffer, buffer_len, 0, addr, addrlen);
}

int sock_set_nonblock(sock_t sock)
{
    if(sock < 0) return -1;

#if defined(OS_WIN)
    {
        u_long nonblocking = 1;
        if (ioctlsocket(sock, FIONBIO, &nonblocking) == SOCKET_ERROR)
            return -1;
        return 0;
    }
#elif defined(OS_LINUX) || defined(OS_MAC)
    {
        int flags;
        if ((flags = fcntl(sock, F_GETFL, NULL)) < 0)
            return -1;
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
            return -1;
        return 0;
    }
#endif
    return -1;
}

int sock_set_block(sock_t sock)
{
    if(sock < 0) return -1;

#if defined(OS_WIN)
    {
        u_long nonblocking = 0;
        if (ioctlsocket(sock, FIONBIO,  &nonblocking) == SOCKET_ERROR)
            return -1;
        return 0;
    }
#elif defined(OS_LINUX) || defined(OS_MAC)
    {
        int flags;
        if ((flags = fcntl(sock, F_GETFL, NULL)) < 0)
            return -1;
        if (fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) == -1)
            return -1;
        return 0;
    }
#endif
    return -1;
}

int sock_set_reuseaddr(sock_t sock)
{
    int optval;
    socklen_t optlen;

    if(sock < 0) return -1;
    optval = 1;
    optlen = sizeof(optval);
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, optlen);
}

int sock_set_nodelay(sock_t sock)
{
    int optval;
    socklen_t optlen;

    if(sock < 0) return -1;
    optval = 1;
    optlen = sizeof(optval);
    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, optlen);
}

int sock_set_sndbuf(sock_t sock, int size)
{
    if(sock < 0) return -1;
    return setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const void*)&size, sizeof(size));
}

int sock_set_rcvbuf(sock_t sock, int size)
{
    if(sock < 0) return -1;
    return setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const void*)&size, sizeof(size));
}


//  ipv4 only
int sock_addr_aton(const char* ip_str, uint16_t port, struct sockaddr_in* addr)
{
    if(!ip_str || !addr) return -1;

    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    if(sock_inet_aton(ip_str, &addr->sin_addr) < 0)
        return -1;
    addr->sin_port = htons(port);
    return 0;
}

//  ipv4 only
int sock_addr_ntoa(const struct sockaddr_in* addr, char* addr_str, size_t len)
{
    size_t used;
    int32_t ret;

    if(!addr_str || !addr) return -1;
    if(sock_inet_ntoa(&addr->sin_addr, addr_str, len) < 0)
        return -1;
    used = strlen(addr_str);
    assert(len >= used);
    ret = snprintf(addr_str + used, len - used, ":%d", ntohs(addr->sin_port));
    if(ret < 0 || (size_t)ret >= len - used)
        return -1;
    return 0;
}

//  ipv4 only
int sock_inet_aton(const char *ip_str, struct in_addr *addr)
{
    int a, b, c, d;
    char more;

    if(!ip_str || !addr) return -1;
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
int sock_inet_ntoa(const struct in_addr* addr, char* ip_str, size_t len)
{
    uint32_t a;
    int32_t ret;

    if(!ip_str || !addr) return -1;
    a = ntohl(addr->s_addr);
    ret = snprintf(ip_str, len, "%d.%d.%d.%d",
        (uint8_t)((a >> 24) & 0xff),
        (uint8_t)((a >> 16) & 0xff),
        (uint8_t)((a >> 8) & 0xff),
        (uint8_t)(a & 0xff));
    if(ret < 0 || (size_t)ret >= len)
        return -1;
    return 0;
}


