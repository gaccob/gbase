#include "core/os_def.h"
#include "net/reactor.h"
#include "net/reactor_inner.inl"
#include "base/slist.h"

#if defined(OS_LINUX)
#include <sys/epoll.h>

typedef struct epoll_event event_t;

#define EPOLL_SIZE 10240
typedef struct epoll_t {
    int epoll_fd;
    event_t events[EPOLL_SIZE];
    slist_t* expired;
} epoll_t;

static const char* EPOLL_NAME = "epoll";

int
epoll_init(reactor_t* reactor) {
    if (!reactor || reactor->data)
        return -1;

    epoll_t* epoll = (epoll_t*)MALLOC(sizeof(*epoll));
    if (!epoll)
        goto EPOLL_FAIL;

    epoll->epoll_fd = epoll_create(EPOLL_SIZE);
    if (epoll->epoll_fd < 0)
        goto EPOLL_FAIL1;

    memset(epoll->events, 0, sizeof(epoll->events));

    epoll->expired = slist_create();
    if (!epoll->expired)
        goto EPOLL_FAIL2;

    reactor->data = (void*)epoll;
    reactor->name = EPOLL_NAME;
    return 0;

EPOLL_FAIL2:
    close(epoll->epoll_fd);
EPOLL_FAIL1:
    FREE(epoll);
EPOLL_FAIL:
    return -1;
}

static int
_epoll_set(epoll_t* epoll, handler_t* h, int option, int events) {
    if (!epoll || epoll->epoll_fd < 0 || !h)
        return -1;

    if (EPOLL_CTL_DEL == option)
        return epoll_ctl(epoll->epoll_fd, EPOLL_CTL_DEL, h->fd, 0);

    event_t ep_event;
    ep_event.events = 0;
    ep_event.data.ptr = h;
    if (EVENT_IN & events)
        ep_event.events |= (EPOLLIN | EPOLLERR);
    if (EVENT_OUT & events)
        ep_event.events |= EPOLLOUT;
    return epoll_ctl(epoll->epoll_fd, option, h->fd, &ep_event);
}

int
epoll_register(reactor_t* reactor, handler_t* h, int events) {
    if (!reactor || !reactor->data || !h)
        return -1;
    return _epoll_set((epoll_t*)reactor->data, h, EPOLL_CTL_ADD, events);
}

int
epoll_unregister(reactor_t* reactor, handler_t* h) {
    if (!reactor || !reactor->data || !h)
        return -1;
    epoll_t* epoll = (epoll_t*)reactor->data;
    // add to expire-list
    slist_push_front(epoll->expired, h);
    return _epoll_set(epoll, h, EPOLL_CTL_DEL, 0);
}

int
epoll_modify(reactor_t* reactor, handler_t* h, int events) {
    if (!reactor || !reactor->data || !h)
        return -1;
    return _epoll_set((epoll_t*)reactor->data, h, EPOLL_CTL_MOD, events);
}

//  return = 0, success & process
//  return < 0, fail
//  return > 0, noting to do
int
epoll_dispatch(reactor_t* reactor, int ms) {
    if (!reactor || !reactor->data)
        return -1;
    epoll_t* epoll = (epoll_t*)(reactor->data);
    int res = epoll_wait(epoll->epoll_fd, epoll->events, EPOLL_SIZE, ms);
    if (res < 0) {
        return EINTR == errno ? 0 : -errno;
    } else if (0 == res) {
        return 1;
    } else {
        for (int i = 0; i < res; i++) {
            int type = epoll->events[i].events;
            handler_t* h = (handler_t*)epoll->events[i].data.ptr;
            // check if expired
            if (0 == slist_find(epoll->expired, h))
                continue;
            if ((EPOLLIN & type) || (EPOLLHUP & type)) {
                res = h->in_func(h);
                if (res < 0) {
                    h->close_func(h);
                    continue;
                }
            }
            if (EPOLLOUT & type) {
                res = h->out_func(h);
                if (res < 0) {
                    h->close_func(h);
                    continue;
                }
            }
            if (EPOLLERR & type) {
                h->close_func(h);
            }
        }
    }
    // clean expired list
    slist_clean(epoll->expired);
    return 0;
}

void
epoll_release(reactor_t* reactor) {
    if (reactor && reactor->data) {
        epoll_t* epoll = (epoll_t*)(reactor->data);
        slist_release(epoll->expired);
        close(epoll->epoll_fd);
        epoll->epoll_fd = -1;
        FREE(epoll);
        reactor->data = 0;
        reactor->name = NULL;
    }
}

reactor_impl_t reactor_epoll = {
    epoll_init,
    epoll_release,
    epoll_register,
    epoll_unregister,
    epoll_modify,
    epoll_dispatch,
};

#endif

