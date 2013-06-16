#ifndef REACTOR_INNER_H_
#define REACTOR_INNER_H_

#include "net/reactor.h"

typedef struct reactor_t
{
    const char* name;
    void* data;
    const struct reactor_impl_t* impl;
}reactor_t;

typedef struct reactor_impl_t
{
    int (*init)(struct reactor_t*);
    void (*release)(struct reactor_t*);
    int (*add)(struct reactor_t*, struct handler_t*, int);
    int (*remove)(struct reactor_t*, struct handler_t*);
    int (*modify)(struct reactor_t*, struct handler_t*, int);

    /*
    *    return = 0, success & process
    *    return < 0, fail
    *    return > 0, noting to do
    */
    int (*dispatch)(struct reactor_t*, int);
}reactor_impl_t;

#if defined(OS_LINUX)
extern struct reactor_impl_t reactor_epoll;
#elif defined(OS_MAC)
extern struct reactor_impl_t reactor_kqueue;
#endif
extern struct reactor_impl_t reactor_select;

#endif // REACTOR_INNER_H_

