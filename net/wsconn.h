#ifndef WSCONN_H_
#define WSCONN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "core/connbuffer.h"

// return buffer size processed, return -1 means process fail, reactor will close it
typedef int32_t (*wsconn_read_func)(int32_t fd,
                                    const char* buffer,
                                    int32_t buflen);
typedef void (*wsconn_build_func)(int32_t fd);
typedef void (*wsconn_close_func)(int32_t fd);

struct wsconn_t;
struct wsconn_t* wsconn_init(struct reactor_t* r,
                             wsconn_build_func build_cb,
                             wsconn_read_func read_cb,
                             wsconn_close_func close_cb,
                             struct connbuffer_t* read_buf,
                             struct connbuffer_t* real_read_buf,
                             struct connbuffer_t* write_buf,
                             int32_t fd);
int32_t wsconn_release(struct wsconn_t* con);
int32_t wsconn_fd(struct wsconn_t* con);
int32_t wsconn_start(struct wsconn_t* con);
int32_t wsconn_stop(struct wsconn_t* con);
int32_t wsconn_established(struct wsconn_t* con);

//  return = 0 success
//  return < 0 fail, maybe full
int32_t wsconn_send(struct wsconn_t* con, const char* buffer, int buflen);


#ifdef __cplusplus
}
#endif

#endif // WSCONN_H_

