#ifndef REACTOR_INNER_INL_
#define REACTOR_INNER_INL_

#ifdef __cpluplus
extern "C" {
#endif

#include "net/reactor.h"

struct reactor_t {
    const char* name;
    void* data;
    const struct reactor_impl_t* impl;
};

typedef struct reactor_impl_t {
    int (*create)(reactor_t*);
    void (*release)(reactor_t*);
    int (*add)(reactor_t*, handler_t*, int);
    int (*remove)(reactor_t*, handler_t*);
    int (*modify)(reactor_t*, handler_t*, int);

    //  return = 0, success & process
    //  return < 0, fail
    //  return > 0, noting to do
    int (*dispatch)(reactor_t*, int);
} reactor_impl_t;

#if defined(OS_LINUX)
extern reactor_impl_t reactor_epoll;
#elif defined(OS_CYGWIN)
extern reactor_impl_t reactor_select;
#elif defined(OS_MAC)
extern reactor_impl_t reactor_kqueue;
#endif

#ifdef __cpluplus
}
#endif

#endif

