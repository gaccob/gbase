#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "base/connbuffer.h"
#include "net/sock.h"
#include "net/reactor.h"

struct connector_t;
typedef struct connector_t connector_t;

// return buffer size processed, return -1 means process fail, reactor will close it
typedef int32_t (*connector_read_func)(sock_t sock,
                                       void* arg,
                                       const char* buffer,
                                       int32_t buflen);

typedef void (*connector_close_func)(sock_t sock,
                                     void* arg);

connector_t* connector_create(reactor_t* r);

void connector_set_read_func(connector_t*, connector_read_func, void*);
void connector_set_close_func(connector_t*, connector_close_func, void*);

int32_t connector_release(connector_t* con);

// set customized buffer (before start)
int32_t connector_set_rbuf(connector_t*, connbuffer_t*);
int32_t connector_set_wbuf(connector_t*, connbuffer_t*);

sock_t connector_fd(connector_t* con);
void connector_set_fd(connector_t* con, sock_t sock);

int32_t connector_start(connector_t* con);
int32_t connector_stop(connector_t* con);

//  return = 0 success
//  return < 0, fail, maybe full
int32_t connector_send(connector_t* con, const char* buffer, int buflen);

#ifdef __cplusplus
}
#endif

#endif // CONNECTOR_H_

