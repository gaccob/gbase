#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"

typedef struct acceptor_t acc_t;
typedef int (*acc_read_func)(sock_t, void* arg);
typedef void (*acc_close_func)(sock_t, void* arg);

acc_t* acc_create(reactor_t*);
int acc_release(acc_t* a);

void acc_set_read_func(acc_t*, acc_read_func, void*);
void acc_set_close_func(acc_t*, acc_close_func, void*);

int acc_start(acc_t*, sockaddr_t* laddr);
int acc_stop(acc_t*);

#ifdef __cplusplus
}
#endif

#endif // ACCEPTOR_H_


