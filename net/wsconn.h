#ifndef WSCONN_H_
#define WSCONN_H_

// web socket wrapper

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "base/buffer.h"

// return buffer size processed, return -1 means process fail, reactor will close it
typedef int (*wsconn_read_func)(sock_t, void* arg, const char* buffer, int buflen);
typedef void (*wsconn_build_func)(sock_t, void* arg);
typedef void (*wsconn_close_func)(sock_t, void* arg);

typedef struct wsconn_t wsconn_t;

wsconn_t* wsconn_create(reactor_t*);
int wsconn_release(wsconn_t*);

void wsconn_set_build_func(wsconn_t*, wsconn_build_func, void*);
void wsconn_set_read_func(wsconn_t*, wsconn_read_func, void*);
void wsconn_set_close_func(wsconn_t*, wsconn_close_func, void*);

int wsconn_set_buffer(wsconn_t*, buffer_t* rb, buffer_t* wb, buffer_t* rrb);

sock_t wsconn_fd(wsconn_t*);
void wsconn_set_fd(wsconn_t*, sock_t);

int wsconn_start(wsconn_t*);
int wsconn_stop(wsconn_t*);
int wsconn_established(wsconn_t*);

//  return = 0 success
//  return < 0 fail, maybe full
int wsconn_send(wsconn_t* con, const char* buffer, int buflen);

#ifdef __cplusplus
}
#endif

#endif // WSCONN_H_

