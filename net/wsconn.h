#ifndef WSCONN_H_
#define WSCONN_H_

// web socket wrapper
// reference: http://www.gaccob.com/?p=1148

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "base/connbuffer.h"

// return buffer size processed, return -1 means process fail, reactor will close it
typedef int32_t (*wsconn_read_func)(sock_t fd,
                                    void* arg,
                                    const char* buffer,
                                    int32_t buflen);

typedef void (*wsconn_build_func)(sock_t fd, void* arg);
typedef void (*wsconn_close_func)(sock_t fd, void* arg);

struct wsconn_t;
typedef struct wsconn_t wsconn_t;

wsconn_t* wsconn_create(reactor_t*);
int32_t wsconn_release(wsconn_t*);

void wsconn_set_build_func(wsconn_t*, wsconn_build_func, void*);
void wsconn_set_read_func(wsconn_t*, wsconn_read_func, void*);
void wsconn_set_close_func(wsconn_t*, wsconn_close_func, void*);

int32_t wsconn_set_buffer(wsconn_t*,
                          connbuffer_t* rbuf,
                          connbuffer_t* wbuf,
                          connbuffer_t* rrbuf);

sock_t wsconn_fd(wsconn_t*);
void wsconn_set_fd(wsconn_t*, sock_t);

int32_t wsconn_start(wsconn_t*);
int32_t wsconn_stop(wsconn_t*);
int32_t wsconn_established(wsconn_t*);

//  return = 0 success
//  return < 0 fail, maybe full
int32_t wsconn_send(wsconn_t* con, const char* buffer, int buflen);


#ifdef __cplusplus
}
#endif

#endif // WSCONN_H_

