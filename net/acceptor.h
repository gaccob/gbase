#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"

typedef int (*acceptor_read_func)(sock_t sock, void* arg);
typedef void (*acceptor_close_func)(sock_t sock, void* arg);

struct acceptor_t;
typedef struct acceptor_t acceptor_t;

acceptor_t* acceptor_create(reactor_t* r);
int acceptor_release(acceptor_t* a);

void acceptor_set_read_func(acceptor_t*, acceptor_read_func, void*);
void acceptor_set_close_func(acceptor_t*, acceptor_close_func, void*);

int acceptor_start(acceptor_t* a, struct sockaddr* laddr);
int acceptor_stop(acceptor_t* a);

#ifdef __cplusplus
}
#endif

#endif // ACCEPTOR_H_


