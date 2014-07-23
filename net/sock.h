#ifndef SOCK_H_
#define SOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <errno.h>

#include "core/os_def.h"

typedef int sock_t;
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddrin_t;
#define INVALID_SOCK -1

#define SOCK_SNDBUF_SIZE (256 * 1024)
#define SOCK_RCVBUF_SIZE (256 * 1024)

sock_t sock_tcp();
sock_t sock_udp();

int sock_nonblock_connect(sock_t sock, const char* ip_str, uint16_t port);
int sock_listen(sock_t sock, sockaddr_t* addr);
sock_t sock_accept(sock_t sock, sockaddr_t* addr);
int sock_close(sock_t sock);

int sock_set_nonblock(sock_t sock);
int sock_set_block(sock_t sock);
int sock_set_reuseaddr(sock_t sock);
int sock_set_nodelay(sock_t sock);
int sock_set_sndbuf(sock_t sock, int size);
int sock_set_rcvbuf(sock_t sock, int size);

int sock_addr_aton(const char* ip_str, uint16_t port, sockaddrin_t* addr);
int sock_addr_ntoa(const sockaddrin_t* addr, char* addr_str, size_t len);

#ifdef __cplusplus
}
#endif

#endif // SOCK_H_

