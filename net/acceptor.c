#include "net/acceptor.h"
#include "net/sock.h"

typedef struct acceptor_t {
    // handler must be at head
    struct handler_t h;
    struct reactor_t* r;
    acceptor_read_func on_read;
    void* read_arg;
    acceptor_close_func on_close;
    void* close_arg;
} acceptor_t;

//  return -1, means fail, reactor will remove & close acceptor
static int
_acceptor_read(struct handler_t* h) {
    acceptor_t* a = (acceptor_t*)h;
    struct sockaddr addr;
    sock_t nsock = sock_accept(a->h.fd, &addr);
    if (nsock == INVALID_SOCK)
        return -1;
    if (a->on_read)
        return a->on_read(nsock, a->read_arg);
    return 0;
}

static int
_acceptor_close(struct handler_t* h) {
    acceptor_t* a = (acceptor_t*)h;
    if (a->on_close)
        a->on_close(a->h.fd, a->close_arg);
    return acceptor_release(a);
}

acceptor_t*
acceptor_create(struct reactor_t* r) {
    acceptor_t* a;
    if (!r) return NULL;
    a = (acceptor_t*)MALLOC(sizeof(acceptor_t));
    if (!a) return NULL;
    memset(a, 0, sizeof(acceptor_t));
    a->r = r;
    a->h.fd = sock_tcp();
    a->h.in_func = _acceptor_read;
    a->h.close_func = _acceptor_close;
    return a;
}

inline void
acceptor_set_read_func(acceptor_t* a,
                       acceptor_read_func on_read,
                       void* arg) {
    if (a) {
        a->on_read = on_read;
        a->read_arg = arg;
    }
}

inline void
acceptor_set_close_func(acceptor_t* a,
                        acceptor_close_func on_close,
                        void* arg) {
    if (a) {
        a->on_close = on_close;
        a->close_arg = arg;
    }
}

int
acceptor_release(acceptor_t* a) {
    acceptor_stop(a);
    if (a) FREE(a);
    return 0;
}

int
acceptor_start(acceptor_t* a, struct sockaddr* laddr) {
    int res;
    if (!a || a->h.fd == INVALID_SOCK)
        return -1;
    res = sock_listen(a->h.fd, laddr);
    if (res < 0) return -1;
    res = reactor_register(a->r, &a->h, EVENT_IN);
    if (res < 0) return -1;
    return 0;
}

int
acceptor_stop(acceptor_t* a) {
    if (!a || a->h.fd == INVALID_SOCK)
        return -1;
    reactor_unregister(a->r, &a->h);
    sock_close(a->h.fd);
    return 0;
}

