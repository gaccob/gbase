#ifndef REACTOR_H_
#define REACTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "net/sock.h"

struct reactor_impl_t;
struct reactor_t;

typedef struct handler_t
{
    sock_t fd;
    // return < 0 means to close handle fd
    int (*in_func)(struct handler_t*);
    int (*out_func)(struct handler_t*);
    int (*close_func)(struct handler_t*);
} handler_t;

#define EVENT_IN 1
#define EVENT_OUT 2

struct reactor_t* reactor_init();
void reactor_release(struct reactor_t* reactor);
int reactor_register(struct reactor_t* reactor, struct handler_t* h, int events);
int reactor_unregister(struct reactor_t* reactor, struct handler_t* h);
int reactor_modify(struct reactor_t* reactor, struct handler_t* h, int events);
int reactor_dispatch(struct reactor_t* reactor, int ms);

#ifdef __cplusplus
}
#endif

#endif // REACTOR_H_

