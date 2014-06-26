#include "net/acceptor.h"
#include "net/sock.h"

typedef struct acceptor_t {
    // handler must be at head
    struct handler_t h;
    struct reactor_t* r;
    acceptor_read_func read_cb;
    acceptor_close_func close_cb;
} acceptor_t;

//  return -1, means fail, reactor will remove & close acceptor
static int
_acceptor_read(struct handler_t* h) {
    acceptor_t* a = (acceptor_t*)h;
    struct sockaddr addr;
    sock_t nsock = sock_accept(a->h.fd, &addr);
    if (nsock == INVALID_SOCK)
        return -1;
    if (a->read_cb) return a->read_cb(nsock);
    return 0;
}

static int
_acceptor_close(struct handler_t* h) {
    acceptor_t* a = (acceptor_t*)h;
    if (a->close_cb) a->close_cb(a->h.fd);
    return acceptor_release(a);
}

acceptor_t*
acceptor_create(struct reactor_t* r,
                acceptor_read_func read_cb,
                acceptor_close_func close_cb) {
    acceptor_t* a;
    if (!r) return NULL;
    a = (acceptor_t*)MALLOC(sizeof(acceptor_t));
    if (!a) return NULL;
    a->r = r;
    a->read_cb = read_cb;
    a->close_cb = close_cb;
    a->h.fd = sock_tcp();
    a->h.in_func = _acceptor_read;
    a->h.out_func = NULL;
    a->h.close_func = _acceptor_close;
    return a;
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
    if (!a || a->h.fd < 0) return -1;
    res = sock_listen(a->h.fd, laddr);
    if (res < 0) return -1;
    res = reactor_register(a->r, &a->h, EVENT_IN);
    if (res < 0) return -1;
    return 0;
}

int
acceptor_stop(acceptor_t* a) {
    if (!a || a->h.fd < 0) return -1;
    reactor_unregister(a->r, &a->h);
    sock_close(a->h.fd);
    return 0;
}

