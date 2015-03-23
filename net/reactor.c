#include "net/reactor.h"
#include "net/reactor_inner.inl"

reactor_t*
reactor_create() {
    reactor_t* reactor = (reactor_t*)MALLOC(sizeof(reactor_t));
    if (!reactor)
        return NULL;
    reactor->name = NULL;
    reactor->data = 0;
#if defined(OS_LINUX)
    reactor->impl = &reactor_epoll;
#elif defined(OS_MAC)
    reactor->impl = &reactor_kqueue;
#else
    reactor->impl = &reactor_select;
#endif
    int ret = reactor->impl->create(reactor);
    if (ret < 0) {
        FREE(reactor);
        return NULL;
    }
    return reactor;
}

void
reactor_release(reactor_t* reactor) {
    if (reactor) {
        reactor->impl->release(reactor);
        FREE(reactor);
    }
}

int
reactor_register(reactor_t* reactor, struct handler_t* h, int events) {
    return reactor ? reactor->impl->add(reactor, h, events) : -1;
}

int
reactor_unregister(reactor_t* reactor, struct handler_t* h) {
    return reactor ? reactor->impl->remove(reactor, h) : -1;
}

int
reactor_modify(reactor_t* reactor, struct handler_t* h, int events) {
    return reactor ? reactor->impl->modify(reactor, h, events) : -1;
}

int
reactor_dispatch(reactor_t* reactor, int ms) {
    return reactor ? reactor->impl->dispatch(reactor, ms) : -1;
}

