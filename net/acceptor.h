#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"

typedef int (*acceptor_read_func)(int fd);
typedef void (*acceptor_close_func)(int fd);

struct acceptor_t;
struct acceptor_t* acceptor_init(struct reactor_t* r,
        acceptor_read_func read_cb,
        acceptor_close_func close_cb);
int acceptor_release(struct acceptor_t* a);
int acceptor_start(struct acceptor_t* a, struct sockaddr* laddr);
int acceptor_stop(struct acceptor_t* a);

#ifdef __cplusplus
}
#endif

#endif // ACCEPTOR_H_


