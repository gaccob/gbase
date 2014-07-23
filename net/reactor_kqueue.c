#include "core/os_def.h"
#include "net/reactor.h"
#include "net/reactor_inner.inl"
#include "base/slist.h"

#if defined(OS_MAC)
#include <sys/event.h>

#define KQUEUE_SIZE 1024

typedef struct kevent event_t;

typedef struct kqueue_t {
    int kqueue_fd;
    event_t events[KQUEUE_SIZE];
    slist_t* expired;
} kqueue_t;

static const char* KQUEUE_NAME = "kqueue";

int
kqueue_create(reactor_t* reactor) {
    if (!reactor || reactor->data)
        return -1;
    kqueue_t* kq = (kqueue_t*)MALLOC(sizeof(kqueue_t));
    assert(kq);
    kq->kqueue_fd = kqueue();
    assert(kq->kqueue_fd >= 0);
    memset(kq->events, 0, sizeof(kq->events));
    kq->expired = slist_create();
    assert(kq->expired);

    reactor->data = (void*)kq;
    reactor->name = KQUEUE_NAME;
    return 0;
}

int
kqueue_register(reactor_t* reactor, handler_t* h, int events) {
    if (!reactor || !reactor->data || !h)
        return -1;
    kqueue_t* kq = (kqueue_t*)reactor->data;
    event_t ke_read, ke_write;
    if (EVENT_IN & events) {
        EV_SET(&ke_read, h->fd, EVFILT_READ, EV_ADD, 0, 0, h);
    } else {
        EV_SET(&ke_read, h->fd, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, h);
    }
    kevent(kq->kqueue_fd, &ke_read, 1, NULL, 0, NULL);
    if (EVENT_OUT & events) {
        EV_SET(&ke_write, h->fd, EVFILT_WRITE, EV_ADD, 0, 0, h);
    } else {
        EV_SET(&ke_write, h->fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, h);
    }
    kevent(kq->kqueue_fd, &ke_write, 1, NULL, 0, NULL);
    return 0;
}

int
kqueue_unregister(reactor_t* reactor, handler_t* h) {
    if (!reactor || !reactor->data || !h)
        return -1;

    kqueue_t* kq = (kqueue_t*)reactor->data;
    slist_push_front(kq->expired, h);

    event_t ke;
    EV_SET(&ke, h->fd, 0, EV_DELETE, 0, 0, h);
    return kevent(kq->kqueue_fd, &ke, 1, NULL, 0, NULL);
}

int
kqueue_modify(reactor_t* reactor, handler_t* h, int events) {
    if (!reactor || !reactor->data || !h)
        return -1;

    kqueue_t* kq = (kqueue_t*)reactor->data;
    event_t ke;
    if (EVENT_OUT & events) {
        EV_SET(&ke, h->fd, EVFILT_WRITE, EV_ENABLE, 0, 0, h);
    } else {
        EV_SET(&ke, h->fd, EVFILT_WRITE, EV_DISABLE, 0, 0, h);
    }

    kevent(kq->kqueue_fd, &ke, 1, NULL, 0, NULL);
    if (EVENT_IN & events) {
        EV_SET(&ke, h->fd, EVFILT_READ, EV_ENABLE, 0, 0, h);
    } else {
        EV_SET(&ke, h->fd, EVFILT_READ, EV_DISABLE, 0, 0, h);
    }
    kevent(kq->kqueue_fd, &ke, 1, NULL, 0, NULL);
    return 0;
}

//  return = 0, success & process
//  return < 0, fail
//  return > 0, noting to do
int
kqueue_dispatch(reactor_t* reactor, int ms) {
    if (!reactor || !reactor->data)
        return -1;

    kqueue_t* kq = (kqueue_t*)(reactor->data);
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    int res = kevent(kq->kqueue_fd, NULL, 0, kq->events, KQUEUE_SIZE, &ts);
    if (res < 0) {
        return ERR_EINTR == ERRNO ? 0 : -ERRNO;
    } else if (0 == res) {
        return 1;
    }
   
    // get events
    for (int i = 0; i < res; ++ i) {
        int type = kq->events[i].filter;
        handler_t* h = (handler_t*)kq->events[i].udata;
        // check if expired
        if (0 == slist_find(kq->expired, h))
            continue;
        if (kq->events[i].flags & EV_ERROR) {
            printf("kqueue error flag=%d\n", kq->events[i].flags);
            h->close_func(h);
            continue;
        }
        if (EVFILT_READ == type) {
            res = h->in_func(h);
            if (res < 0) {
                h->close_func(h);
                continue;
            }
        } else if (EVFILT_WRITE == type) {
            res = h->out_func(h);
            if (res < 0) {
                h->close_func(h);
                continue;
            }
        }
    }
    slist_clean(kq->expired);
    return 0;
}

void
kqueue_release(reactor_t* reactor) {
    if (reactor && reactor->data) {
        kqueue_t* kq = (kqueue_t*)(reactor->data);
        slist_release(kq->expired);
        close(kq->kqueue_fd);
        kq->kqueue_fd = -1;
        FREE(kq);
        reactor->data = 0;
        reactor->name = NULL;
    }
}

reactor_impl_t reactor_kqueue =
{
    kqueue_create,
    kqueue_release,
    kqueue_register,
    kqueue_unregister,
    kqueue_modify,
    kqueue_dispatch,
};

#endif

