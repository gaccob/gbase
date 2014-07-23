#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "base/buffer.h"
#include "net/sock.h"
#include "net/reactor.h"

typedef struct connector_t con_t;

// return buffer size processed
// return -1 means process fail, reactor will close it
typedef int (*con_read_func)(sock_t, void* arg, const char* buffer, int buflen);
typedef void (*con_close_func)(sock_t, void* arg);

con_t* con_create(reactor_t*);
int con_release(con_t*);

void con_set_read_func(con_t*, con_read_func, void*);
void con_set_close_func(con_t*, con_close_func, void*);

// set customized buffer (before start)
int con_set_rbuf(con_t*, buffer_t*);
int con_set_wbuf(con_t*, buffer_t*);

sock_t con_sock(con_t*);
void con_set_sock(con_t*, sock_t);

int con_start(con_t*);
int con_stop(con_t*);

//  return = 0 success
//  return < 0, fail, maybe full
int con_send(con_t*, const char* buffer, int buflen);

#ifdef __cplusplus
}
#endif

#endif // CONNECTOR_H_

