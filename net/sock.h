#ifndef SOCK_H_
#define SOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

// win32
#if defined(OS_WIN)
    #include <winsock2.h>
    #pragma comment(lib,"WS2_32.lib")

    typedef SOCKET sock_t;
    #define INVALID_SOCK INVALID_SOCKET

    typedef int socklen_t;

    #define SOCK_INIT()  \
        do { \
            WSADATA wsadata;\
            WSAStartup(MAKEWORD(2, 2), &wsadata);\
        } while(0)

    #define SOCK_RELEASE()  \
        do { \
            WSACleanup(); \
        } while(0)

// linux or unix
#elif defined(OS_LINUX) || defined(OS_MAC)
    #include <unistd.h>
    #include <fcntl.h>
    #include <arpa/inet.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/tcp.h>
    #include <errno.h>

    typedef int sock_t;
    #define INVALID_SOCK -1

    #define SOCK_INIT() do {} while(0)
    #define SOCK_RELEASE() do {} while(0)

#endif

#define SOCK_SNDBUF_SIZE (256 * 1024)
#define SOCK_RCVBUF_SIZE (256 * 1024)

sock_t sock_tcp();
sock_t sock_udp();

int sock_connect(sock_t sock, const char* ip_str, uint16_t port);
int sock_listen(sock_t sock, struct sockaddr* addr);
int sock_accept(sock_t sock, struct sockaddr* addr);
int sock_close(sock_t sock);

int sock_read(sock_t sock, char* buffer, size_t buffer_len);
int sock_write(sock_t sock, const char* buffer, size_t buffer_len);
int sock_recvfrom(sock_t sock, char* buffer, size_t buffer_len, struct sockaddr* addr, socklen_t addrlen);
int sock_sendto(sock_t sock, const char* buffer, size_t buffer_len, struct sockaddr* addr, socklen_t addrlen);

int sock_set_nonblock(sock_t sock);
int sock_set_block(sock_t sock);
int sock_set_reuseaddr(sock_t sock);
int sock_set_nodelay(sock_t sock);
int sock_set_sndbuf(sock_t sock, int size);
int sock_set_rcvbuf(sock_t sock, int size);

int sock_addr_aton(const char* ip_str, uint16_t port, struct sockaddr_in* addr);
int sock_addr_ntoa(const struct sockaddr_in* addr, char* addr_str, size_t len);
int sock_inet_aton(const char *ip_str, struct in_addr *addr);
int sock_inet_ntoa(const struct in_addr* addr, char* ip_str, size_t len);

#ifdef __cplusplus
}
#endif

#endif // SOCK_H_

