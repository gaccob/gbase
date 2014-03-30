#include "core/os_def.h"
#include "net/reactor.h"
#include "net/reactor_inner.h"
#include "base/slist.h"

#if defined(OS_MAC)
#include <sys/event.h>

#define KQUEUE_SIZE 1024
typedef struct kqueue_t
{
    int kqueue_fd;
    struct kevent events[KQUEUE_SIZE];
    struct slist_t* expired;
} kqueue_t;

static const char* KQUEUE_NAME = "kqueue";

int kqueue_init(struct reactor_t* reactor)
{
    if (!reactor || reactor->data) return -1;

    kqueue_t* kq = (kqueue_t*)MALLOC(sizeof(kqueue_t));
    if (!kq) goto KQ_FAIL;

    kq->kqueue_fd = kqueue();
    if (kq->kqueue_fd < 0) goto KQ_FAIL1;
    memset(kq->events, 0, sizeof(kq->events));

    kq->expired = slist_create();
    if (!kq->expired) goto KQ_FAIL2;

    reactor->data = (void*)kq;
    reactor->name = KQUEUE_NAME;
    return 0;

KQ_FAIL2:
    close(kq->kqueue_fd);
KQ_FAIL1:
    FREE(kq);
KQ_FAIL:
    return -1;
}

int kqueue_register(struct reactor_t* reactor, struct handler_t* h, int events)
{
    struct kevent ke_read, ke_write;
    kqueue_t* kq;
    if (!reactor || !reactor->data || !h) return -1;

    kq = (kqueue_t*)reactor->data;
    if (EVENT_IN & events)
        EV_SET(&ke_read, h->fd, EVFILT_READ, EV_ADD, 0, 0, h);
    else
        EV_SET(&ke_read, h->fd, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, h);
    kevent(kq->kqueue_fd, &ke_read, 1, NULL, 0, NULL);
    if (EVENT_OUT & events)
        EV_SET(&ke_write, h->fd, EVFILT_WRITE, EV_ADD, 0, 0, h);
    else
        EV_SET(&ke_write, h->fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, h);
    kevent(kq->kqueue_fd, &ke_write, 1, NULL, 0, NULL);
    return 0;
}

int kqueue_unregister(struct reactor_t* reactor, struct handler_t* h)
{
    kqueue_t* kq;
    struct kevent ke;
    if (!reactor || !reactor->data || !h) return -1;

    kq = (kqueue_t*)reactor->data;
    slist_push_front(kq->expired, h);
    EV_SET(&ke, h->fd, 0, EV_DELETE, 0, 0, h);
    return kevent(kq->kqueue_fd, &ke, 1, NULL, 0, NULL);
}

int kqueue_modify(struct reactor_t* reactor, struct handler_t* h, int events)
{
    kqueue_t* kq;
    struct kevent ke;
    if (!reactor || !reactor->data || !h) return -1;

    kq = (kqueue_t*)reactor->data;
    if (EVENT_OUT & events)
        EV_SET(&ke, h->fd, EVFILT_WRITE, EV_ENABLE, 0, 0, h);
    else
        EV_SET(&ke, h->fd, EVFILT_WRITE, EV_DISABLE, 0, 0, h);
    kevent(kq->kqueue_fd, &ke, 1, NULL, 0, NULL);
    if (EVENT_IN & events)
        EV_SET(&ke, h->fd, EVFILT_READ, EV_ENABLE, 0, 0, h);
    else
        EV_SET(&ke, h->fd, EVFILT_READ, EV_DISABLE, 0, 0, h);
    kevent(kq->kqueue_fd, &ke, 1, NULL, 0, NULL);
    return 0;
}

//  return = 0, success & process
//  return < 0, fail
//  return > 0, noting to do
int kqueue_dispatch(struct reactor_t* reactor, int ms)
{
    int res, i, type;
    struct handler_t* h;
    kqueue_t* kq;
    struct timespec ts;
    if (!reactor || !reactor->data) return -1;

    kq = (kqueue_t*)(reactor->data);
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    res = kevent(kq->kqueue_fd, NULL, 0, kq->events, KQUEUE_SIZE, &ts);
    if (res < 0) {
        if (EINTR != errno) return -errno;
        return 0;
    }

    // nothing to do
    if (0 == res) return 1;

    // get events
    for (i = 0; i < res; ++ i) {
        type = kq->events[i].filter;
        h = (struct handler_t*)kq->events[i].udata;

        // check if expired
        if (0 == slist_find(kq->expired, h)) continue;

        if (kq->events[i].flags & EV_ERROR) {
            printf("kqueue error flag=%d\n", kq->events[i].flags);
            h->close_func(h);
            continue;
        }

        // IO callback
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

    // clean expired list
    slist_clean(kq->expired);
    return 0;
}

void kqueue_release(struct reactor_t* reactor)
{
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

struct reactor_impl_t reactor_kqueue =
{
    kqueue_init,
    kqueue_release,
    kqueue_register,
    kqueue_unregister,
    kqueue_modify,
    kqueue_dispatch,
};

#endif

