#include "net/acceptor.h"
#include "net/sock.h"

typedef struct acceptor_t
{
    /* handler must be at head */
    struct handler_t h;
    struct reactor_t* r;
    acceptor_read_func read_cb;
    acceptor_close_func close_cb;
}acceptor_t;

/*
*    return -1, means fail, reactor will remove & close acceptor
*/
int _acceptor_read(struct handler_t* h)
{
    struct acceptor_t* a = (struct acceptor_t*)h;
    struct sockaddr addr;
    int new_fd = sock_accept(a->h.fd, &addr);
    if(new_fd < 0)
        return -1;
    if(a->read_cb)
        return a->read_cb(new_fd);
    return 0;
}

int _acceptor_close(struct handler_t* h)
{
    struct acceptor_t* a = (struct acceptor_t*)h;
    if(a->close_cb)
        a->close_cb(a->h.fd);
    return acceptor_release(a);
}

struct acceptor_t* acceptor_init(struct reactor_t* r,
        acceptor_read_func read_cb,
        acceptor_close_func close_cb)
{
    struct acceptor_t* a;
    if(!r)
        return NULL;
    a = (struct acceptor_t*)MALLOC(sizeof(struct acceptor_t));
    if(!a)
        return NULL;
    a->r = r;
    a->read_cb = read_cb;
    a->close_cb = close_cb;
    a->h.fd = sock_tcp();
    a->h.in_func = _acceptor_read;
    a->h.out_func = NULL;
    a->h.close_func = _acceptor_close;
    return a;
}

int acceptor_release(struct acceptor_t* a)
{
    acceptor_stop(a);
    if(a)
        FREE(a);
    return 0;
}

int acceptor_start(struct acceptor_t* a, struct sockaddr* laddr)
{
    int res;
    if(!a || a->h.fd < 0)
        return -1;
    res = sock_listen(a->h.fd, laddr);
    if(res < 0)
        return -1;
    res = reactor_register(a->r, &a->h, EVENT_IN);
    if(res < 0)
        return -1;
    return 0;
}

int acceptor_stop(struct acceptor_t* a)
{
    if(!a || a->h.fd < 0)
        return -1;
    reactor_unregister(a->r, &a->h);
    sock_close(a->h.fd);
    return 0;
}


