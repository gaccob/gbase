#ifndef REACTOR_H_
#define REACTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"

struct reactor_impl_t;
typedef struct reactor_t reactor_t;

typedef struct handler_t {
    sock_t fd;
    // return < 0 means to close handle fd
    int (*in_func)(struct handler_t*);
    int (*out_func)(struct handler_t*);
    int (*close_func)(struct handler_t*);
} handler_t;

#define EVENT_IN 1
#define EVENT_OUT 2

reactor_t* reactor_create();
void reactor_release(reactor_t*);
int reactor_register(reactor_t*, handler_t*, int events);
int reactor_unregister(reactor_t*, handler_t*);
int reactor_modify(reactor_t*, handler_t*, int events);

//  return = 0, success & process
//  return < 0, fail
//  return > 0, noting to do
int reactor_dispatch(reactor_t*, int ms);

#ifdef __cplusplus
}
#endif

#endif // REACTOR_H_

