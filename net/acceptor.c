#include "net/acceptor.h"
#include "net/sock.h"

struct acceptor_t {
    // handler must be at head
    handler_t h;
    reactor_t* r;
    acc_read_func on_read;
    void* read_arg;
    acc_close_func on_close;
    void* close_arg;
};

//  return -1, means fail, reactor will remove & close acceptor
static int
_acc_read(handler_t* h) {
    acc_t* a = (acc_t*)h;
    sockaddr_t addr;
    sock_t nsock = sock_accept(a->h.fd, &addr);
    if (nsock == INVALID_SOCK)
        return -1;
    if (a->on_read)
        return a->on_read(nsock, a->read_arg);
    return 0;
}

static int
_acc_close(handler_t* h) {
    acc_t* a = (acc_t*)h;
    if (a->on_close)
        a->on_close(a->h.fd, a->close_arg);
    return acc_release(a);
}

acc_t*
acc_create(reactor_t* r) {
    acc_t* a;
    if (!r) return NULL;
    a = (acc_t*)MALLOC(sizeof(acc_t));
    if (!a) return NULL;
    memset(a, 0, sizeof(acc_t));
    a->r = r;
    a->h.fd = sock_tcp();
    a->h.in_func = _acc_read;
    a->h.close_func = _acc_close;
    return a;
}

inline void
acc_set_read_func(acc_t* a, acc_read_func on_read, void* arg) {
    if (a) {
        a->on_read = on_read;
        a->read_arg = arg;
    }
}

inline void
acc_set_close_func(acc_t* a, acc_close_func on_close, void* arg) {
    if (a) {
        a->on_close = on_close;
        a->close_arg = arg;
    }
}

int
acc_release(acc_t* a) {
    if (a) {
        acc_stop(a);
        FREE(a);
    }
    return 0;
}

int
acc_start(acc_t* a, sockaddr_t* laddr) {
    if (!a || a->h.fd == INVALID_SOCK) {
        return -1;
    }
    int res = sock_listen(a->h.fd, laddr);
    if (res < 0) {
        return res;
    }
    return reactor_register(a->r, &a->h, EVENT_IN);
}

int
acc_stop(acc_t* a) {
    if (!a || a->h.fd == INVALID_SOCK) {
        return -1;
    }
    reactor_unregister(a->r, &a->h);
    sock_close(a->h.fd);
    return 0;
}

