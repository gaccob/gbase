#include <assert.h>
#include "mm/shm.h"
#include "core/atom.h"
#include "core/lock.h"
#include "base/idtable.h"
#include "base/rbuffer.h"
#include "logic/bus.h"

typedef struct bus_pipe_head {
    int key;
    size_t size;
    bus_addr_t from;
    bus_addr_t to;
} head_t;

typedef struct bus_pipe_t{
    head_t* head;
    rbuffer_t* r;
} pipe_t;

typedef struct bus_head {
    int key;
    int16_t ckey;
    size_t size;
    // terminals info
    atom_t tver;
    int tcount;
    bus_addr_t terms[BUS_MAX_TERMINAL_COUNT];
    // channel info
    atom_t pver;
    int pcount;
    head_t pipes[BUS_MAX_PIPE_COUNT];
} bus_head;

struct bus_t {
    bus_head* head;
    bus_addr_t self;
    lock_t* lock;
    uint32_t tver;
    uint32_t pver;
    int tcount;
    bus_addr_t terms[BUS_MAX_TERMINAL_COUNT];
    idtable_t* opipes;
    idtable_t* ipipes;
};

static void
_head_t_assign(head_t* head, int key, bus_addr_t from,
                      bus_addr_t to, size_t sz) {
    if (head) {
        head->key = key;
        head->size = sz;
        head->from = from;
        head->to = to;
    }
}

static pipe_t*
_bus_pipe_create(head_t* head, int create) {
    if (!head)
        return NULL;
    pipe_t* bp = (pipe_t*)MALLOC(sizeof(*bp));
    assert(bp);
    bp->head = head;
    // extend ring-buffer head
    shm_t* shm = shm_create(head->key, head->size + rbuffer_head_size(), create);
    if (!shm) {
        FREE(bp);
        return NULL;
    }
    shm_mem(shm);
    bp->r = rbuffer_attach((char*)shm_mem(shm), head->size + rbuffer_head_size());
    if (!bp->r) {
        FREE(bp);
        return NULL;
    }
    return bp;
}

static void
_bus_pipe_release(pipe_t* bp) {
    if (bp) FREE(bp);
}

static rbuffer_t*
_bus_pipe_rbuffer(pipe_t* bp) {
    return bp ? bp->r : NULL;
}

static head_t*
_head_t(pipe_t* bp) {
    return bp ? bp->head : NULL;
}

static void
_bus_head_init(bus_head* head, int key, bus_addr_t creator) {
    if (head) {
        head->key = key;
        head->ckey = 0;
        head->size = sizeof(bus_head);
        atom_set(&head->tver, 1);
        head->tcount = 1;
        head->terms[0] = creator;
        atom_set(&head->pver, 1);
        head->pcount = 0;
    }
}

static int
_bus_head_create(bus_t* bt, int16_t key) {
    int buskey = (key << 16);
    shm_t* shm = shm_create(buskey, sizeof(bus_head), 0);
    if (shm) {
        bt->head = (bus_head*)shm_mem(shm);
        assert(bt->head);
        _bus_head_init(bt->head, buskey, bt->self);
        return 0;
    }
    return -1;
}

static void
_bus_update_terminals(bus_t* bt) {
    assert(bt && bt->head);
    bt->tver = bt->head->tver;
    bt->tcount = bt->head->tcount;
    for (int i = 0; i < bt->tcount; ++ i) {
        bt->terms[i] = bt->head->terms[i];
    }
}

static void
_bus_update_pipes(bus_t* bt) {
    assert(bt && bt->head);
    bt->pver = bt->head->pver;
    for (int i = 0; i < bt->head->pcount; ++ i) {
        head_t* phead = &bt->head->pipes[i];
        pipe_t* bp;
        int ret;
        if (phead->from == bt->self && !idtable_get(bt->opipes, phead->to)) {
            bp = _bus_pipe_create(phead, 1);
            assert(bp);
            ret = idtable_add(bt->opipes, phead->to, bp);
            assert(0 == ret);
        } else if (phead->to == bt->self && !idtable_get(bt->ipipes, phead->from)) {
            bp = _bus_pipe_create(phead, 1);
            assert(bp);
            ret = idtable_add(bt->ipipes, phead->from, bp);
            assert(0 == ret);
        }
    }
}

static int
_bus_create_attach(bus_t* bt, int16_t key) {
    // attach exsit shm
    int buskey = (key << 16);
    shm_t* shm = shm_create(buskey, sizeof(bus_head), 1);
    assert(shm);
    bt->head = (bus_head*)shm_mem(shm);
    if (bt->head->key != buskey || bt->head->size != sizeof(bus_head)) {
        return -1;
    }
    // register terminal info
    int registered = -1;
    for (int i = 0; i < bt->head->tcount; ++ i) {
        if (bt->head->terms[i] == bt->self) {
            registered = 0;
            break;
        }
    }
    if (registered != 0) {
        if (bt->head->tcount >= BUS_MAX_TERMINAL_COUNT) {
            return -1;
        } else {
            bt->head->terms[bt->head->tcount ++] = bt->self;
            atom_inc(&bt->head->tver);
        }
    }
    return 0;
}

bus_t*
bus_create(int16_t key, bus_addr_t addr) {
    bus_t* bt = (bus_t*)MALLOC(sizeof(*bt));
    if (!bt) return NULL;
    bt->head = NULL;
    bt->self = addr;
    bt->tver = 0;
    bt->pver = 0;
    bt->lock = lock_create(key);
    assert(bt->lock);
    bt->opipes = idtable_create(BUS_MAX_TERMINAL_COUNT);
    assert(bt->opipes);
    bt->ipipes = idtable_create(BUS_MAX_TERMINAL_COUNT);
    assert(bt->ipipes);

    lock_lock(bt->lock);
    if (_bus_head_create(bt, key) == 0 || _bus_create_attach(bt, key) == 0) {
        _bus_update_terminals(bt);
        _bus_update_pipes(bt);
        lock_unlock(bt->lock);
        return bt;
    }
    lock_unlock(bt->lock);
    FREE(bt);
    return NULL;
}

static int
_bus_release_pipe(void* data, void* arg) {
    _bus_pipe_release((pipe_t*)data);
    return 0;
}

void
bus_release(bus_t* bt) {
    if (bt) {
        idtable_loop(bt->opipes, _bus_release_pipe, NULL, 0);
        idtable_release(bt->opipes);
        bt->opipes = NULL;
        idtable_loop(bt->ipipes, _bus_release_pipe, NULL, 0);
        idtable_release(bt->ipipes);
        bt->ipipes = NULL;
        FREE(bt);
    }
}

// check bus version and do update
void
bus_poll(bus_t* bt) {
    if (!bt)
        return;
    // update terminals
    uint32_t tver = bt->head->tver;
    if (tver != bt->tver) {
        lock_lock(bt->lock);
        _bus_update_terminals(bt);
        lock_unlock(bt->lock);
    }
    // udpate pipe
    uint32_t pver = bt->head->pver;
    if (pver != bt->pver) {
        lock_lock(bt->lock);
        _bus_update_pipes(bt);
        lock_unlock(bt->lock);
    }
}

static pipe_t*
_bus_register_pipe(bus_t* bt, bus_addr_t to, size_t sz) {
    if (!bt)
        return NULL;
    // make sure same version
    bus_poll(bt);

    // add lock
    bus_head* head = bt->head;
    lock_lock(bt->lock);
    if (bt->pver != head->pver) goto PIPE_CREATE_FAIL;

    // validate
    if (head->pcount >= BUS_MAX_PIPE_COUNT)
        goto PIPE_CREATE_FAIL;
    int exist = -1;
    for (int i = 0; i < head->tcount; ++ i) {
        if (head->terms[i] == to) {
            exist = 0;
            break;
        }
    }
    if (exist != 0 || to == bt->self)
        goto PIPE_CREATE_FAIL;

    // create pipe
    int key = head->key + (++ head->ckey);
    head_t* bph = &head->pipes[head->pcount];
    _head_t_assign(bph, key, bt->self, to, sz);
    pipe_t* bp = _bus_pipe_create(bph, 0);
    if (!bp)
        goto PIPE_CREATE_FAIL;
    int ret = idtable_add(bt->opipes, bph->to, bp);
    assert(0 == ret);
    ++ bt->head->pcount;

    // version set. unlock
    atom_inc(&bt->head->pver);
    bt->pver = bt->head->pver;
    lock_unlock(bt->lock);
    return bp;

PIPE_CREATE_FAIL:
    lock_unlock(bt->lock);
    return NULL;
}

int
bus_send(bus_t* bt, const char* buf, size_t bufsz, bus_addr_t to) {
    if (!bt)
        return BUS_ERR_FAIL;
    pipe_t* bp = (pipe_t*)idtable_get(bt->opipes, to);
    if (!bp) {
        bp = _bus_register_pipe(bt, to, BUS_PIPE_DEFAULT_SIZE);
        if (!bp)
            return BUS_ERR_PIPE_FAIL;
    }
    int ret = rbuffer_write(_bus_pipe_rbuffer(bp), buf, bufsz);
    return ret == 0 ? BUS_OK : BUS_ERR_SEND_FAIL;
}

int
bus_send_by_type(bus_t* bt, const char* buf, size_t bufsz, int type) {
    int ret = -1;
    for (int i = 0; i < bt->tcount; ++ i) {
        if (bt->terms[i] != bt->self && bus_addr_type(bt->terms[i]) == type) {
            ret = bus_send(bt, buf, bufsz, bt->terms[i]);
            if (ret)
                return ret;
        }
    }
    return ret ? BUS_ERR_PEER_NOT_FOUND : BUS_OK;
}

int
bus_send_all(bus_t* bt, const char* buf, size_t bufsz) {
    for (int i = 0; i < bt->tcount; ++ i) {
        if (bt->terms[i] != bt->self) {
            int ret = bus_send(bt, buf, bufsz, bt->terms[i]);
            if (ret)
                return ret;
        }
    }
    return BUS_OK;
}

int
bus_recv(bus_t* bt, char* buf, size_t* bufsz, bus_addr_t from) {
    if (!bt)
        return BUS_ERR_FAIL;
    pipe_t* bp = (pipe_t*)idtable_get(bt->ipipes, from);
    if (!bp)
        return BUS_ERR_PEER_NOT_FOUND;
    int ret = rbuffer_read(_bus_pipe_rbuffer(bp), buf, bufsz);
    return ret == 0 ? BUS_ERR_EMPTY : BUS_OK;
}

typedef struct bus_loop_param_t {
    size_t* size;
    char* buf;
    bus_addr_t* from;
} loop_t;

static int
_bus_recv_loop(void* data, void* arg) {
    pipe_t* bp = (pipe_t*)data;
    head_t* head = _head_t(bp);
    loop_t* param = (loop_t*)arg;
    int ret = rbuffer_read(_bus_pipe_rbuffer(bp), param->buf, param->size);
    if (ret == 0) {
        *(param->from) = head->from;
        return 1;
    }
    return 0;
}

// TODO: not thread safe, as with static index
int
bus_recv_all(bus_t* bt, char* buf, size_t* bufsz, bus_addr_t* from) {
    if (!bt || !buf || !bufsz || !from)
        return BUS_ERR_FAIL;
    loop_t param;
    param.size = bufsz;
    param.buf = buf;
    param.from = from;
    static int index = 0;
    int ret = idtable_loop(bt->ipipes, _bus_recv_loop, &param, index ++);
    return ret == 0 ? BUS_ERR_EMPTY : BUS_OK;
}

typedef struct bus_dump_param_t {
    char* debug;
    size_t sz;
} dump_t;

static int
_bus_dump_loop(void* data, void* arg) {
    pipe_t* bp = (pipe_t*)data;
    head_t* head = _head_t(bp);
    rbuffer_t* r = _bus_pipe_rbuffer(bp);
    dump_t* param = (dump_t*)arg;
    size_t len = strnlen(param->debug, param->sz);
    snprintf(param->debug + len, param->sz - len,
        "%d->%d: size=%d, read bytes %u, write bytes %d\n", head->from,
        head->to, (int)head->size, rbuffer_read_bytes(r), rbuffer_write_bytes(r));
    return 0;
}

void
bus_dump(bus_t* bt, char* debug, size_t debugsz) {
    assert(bt);
    debug[0] = 0;
    // terminals
    for (int i = 0; i < bt->tcount; ++ i) {
        size_t len = strnlen(debug, debugsz);
        snprintf(debug + len, debugsz - len, "terminal: %d\n", bt->terms[i]);
    }
    // pipes
    dump_t param;
    param.debug = debug;
    param.sz = debugsz;
    idtable_loop(bt->ipipes, _bus_dump_loop, &param, 0);
    idtable_loop(bt->opipes, _bus_dump_loop, &param, 0);
}

uint32_t
bus_send_bytes(bus_t* bt, bus_addr_t to) {
    assert(bt);
    pipe_t* bp = (pipe_t*)idtable_get(bt->opipes, to);
    return bp ? rbuffer_write_bytes(_bus_pipe_rbuffer(bp)) : 0;
}

uint32_t
bus_recv_bytes(bus_t* bt, bus_addr_t from) {
    assert(bt);
    pipe_t* bp = (pipe_t*)idtable_get(bt->ipipes, from);
    return bp ? rbuffer_read_bytes(_bus_pipe_rbuffer(bp)) : 0;
}

