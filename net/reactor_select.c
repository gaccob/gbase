#include "core/os_def.h"
#include "net/reactor.h"
#include "net/reactor_inner.h"
#include "core/hash.h"
#include "core/slist.h"

static const char* SELECT_NAME = "select";

#if defined(OS_LINUX) || defined(OS_MAC)
    #include <sys/select.h>
#elif defined(OS_WIN)
    #include <Winsock2.h>
#else
    #error other platform not support now
#endif

typedef struct select_t
{
    int nfds;
    fd_set in_prepare;
    fd_set out_prepare;
    fd_set in_set;
    fd_set out_set;
    struct hash_t* handler_table;
    struct slist_t* expired;
} select_t;

uint32_t _handle_hash(const void* data)
{
    return ((const handler_t*)data)->fd;
}

int32_t _handle_cmp(const void* data1, const void* data2)
{
    return ((const handler_t*)data1)->fd -((const handler_t*)data2)->fd;
}

#define SELECT_SIZE (sizeof(fd_set)  * 8)
int select_init(struct reactor_t* reactor)
{
    select_t* s;
    if (!reactor || reactor->data) goto SELECT_FAIL;

    s = (select_t*)MALLOC(sizeof(select_t));
    if (!s) goto SELECT_FAIL;
    memset(s, 0, sizeof(select_t));

    s->handler_table = hash_init(_handle_hash, _handle_cmp, SELECT_SIZE * 3);
    if (!s->handler_table) goto SELECT_FAIL1;

    s->expired = slist_init();
    if (!s->expired) goto SELECT_FAIL2;

    printf("select support max fd=%d\n", (int)SELECT_SIZE);
    reactor->name = SELECT_NAME;
    reactor->data = (void*)s;
    return 0;

SELECT_FAIL2:
    hash_release(s->handler_table);
SELECT_FAIL1:
    FREE(s);
SELECT_FAIL:
    return -1;
}

int select_register(struct reactor_t* reactor, struct handler_t* h, int events)
{
    select_t* s;
    if (!reactor || !reactor->data || !h || h->fd < 0) return -1;

    // exceeds fd max
    if (h->fd > (int)sizeof(fd_set)  * 8) return -1;

    // add handle fd
    s = (select_t*)reactor->data;
    if (s->nfds < (int)h->fd)
        s->nfds = h->fd;
    if (EVENT_IN & events)
        FD_SET(h->fd, &s->in_prepare);
    if (EVENT_OUT & events)
        FD_SET(h->fd, &s->out_prepare);

    // add to hash table, ignore fail case(duplicate)
    hash_insert(s->handler_table, h);
    return -1;
}

int select_unregister(struct reactor_t* reactor, struct handler_t* h)
{
    select_t* s;
    if (!reactor || !reactor->data || !h) return -1;

    s = (select_t*)reactor->data;
    FD_CLR(h->fd, &s->in_prepare);
    FD_CLR(h->fd, &s->out_prepare);
    hash_remove(s->handler_table, h);
    slist_push_front(s->expired, h);
    return 0;
}

int select_modify(struct reactor_t* reactor, struct handler_t* h, int events)
{
    select_t* s;
    if (!reactor || !reactor->data || !h) return -1;

    s = (select_t*)reactor->data;
    if (EVENT_IN & events)
        FD_SET(h->fd, &s->in_prepare);
    if (EVENT_OUT & events)
        FD_SET(h->fd, &s->out_prepare);
    // add to hash table, ignore fail case(duplicate)
    hash_insert(s->handler_table, (void*)h);
    return 0;
}

void _select_callback(select_t* s, int fd, int events)
{
    int res;
    struct handler_t tmp;
    struct handler_t* h;
    tmp.fd = fd;
    h = (struct handler_t*)hash_find(s->handler_table, (void*)&tmp);
    if (!h) return;

    // expired
    res = slist_find(s->expired, h);
    if (0 == res) return;

    if (EVENT_IN & events) {
        res = h->in_func(h);
        if (res < 0) {
            h->close_func(h);
            return;
        }
    }
    if (EVENT_OUT & events) {
        res = h->out_func(h);
        if (res < 0) {
            h->close_func(h);
            return;
        }
    }
}

//  return = 0, success & process
//  return < 0, fail
//  return > 0, noting to do
int select_dispatch(struct reactor_t* reactor, int ms)
{
    int i, j, res;
    select_t* s;
    struct timeval tv;
    if (!reactor || !reactor->data) return -1;

    // select
    s = (select_t*)reactor->data;
    memcpy(&s->in_set, &s->in_prepare, sizeof(fd_set));
    memcpy(&s->out_set, &s->out_prepare, sizeof(fd_set));
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    res = select(s->nfds, &s->in_set, &s->out_set, NULL, &tv);
    if (res == -1) {
        if (errno != EINTR) return -1;
        return 1;
    }

    // callback, random for balance efficiency
    i = rand() % s->nfds;
    for (j = 0; j < s->nfds; ++j) {
        if (++i >= s->nfds) i = 0;
        res = 0;
        if (FD_ISSET(i, &s->in_set)) res |= EVENT_IN;
        if (FD_ISSET(i, &s->out_set)) res |= EVENT_OUT;
        if (res == 0) continue;
        _select_callback(s, i, res);
    }

    // clean expired list
    slist_clean(s->expired);

    return 0;
}

void select_release(struct reactor_t* reactor)
{
    select_t* s;
    if (reactor && reactor->data) {
        s = (select_t*)reactor->data;
        hash_release(s->handler_table);
        FREE(s);
        reactor->data = 0;
        reactor->name = NULL;
    }
}

struct reactor_impl_t reactor_select =
{
    select_init,
    select_release,
    select_register,
    select_unregister,
    select_modify,
    select_dispatch,
};

