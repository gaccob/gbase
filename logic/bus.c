#include <assert.h>
#include "mm/shm.h"
#include "core/atom.h"
#include "core/lock.h"
#include "base/idtable.h"
#include "base/rbuffer.h"
#include "logic/bus.h"

typedef struct bus_pipe_head {
    int32_t key;
    size_t size;
    bus_addr_t from;
    bus_addr_t to;
} bus_pipe_head;

typedef struct bus_pipe_t{
    bus_pipe_head* head;
    struct rbuffer_t* r;
} bus_pipe_t;

typedef struct bus_head {
    int32_t key;
    int16_t ckey;
    size_t size;
    // terminals info
    atom_t tver;
    int tcount;
    bus_addr_t terms[BUS_MAX_TERMINAL_COUNT];
    // channel info
    atom_t pver;
    int pcount;
    bus_pipe_head pipes[BUS_MAX_PIPE_COUNT];
} bus_head;

typedef struct bus_t {
    bus_head* head;
    bus_addr_t self;
    struct lock_t* lock;
    uint32_t tver;
    uint32_t pver;
    int tcount;
    bus_addr_t terms[BUS_MAX_TERMINAL_COUNT];
    struct idtable_t* opipes;
    struct idtable_t* ipipes;
} bus_t;

static void
_bus_pipe_head_assign(bus_pipe_head* head, int32_t key, bus_addr_t from,
                      bus_addr_t to, size_t sz) {
    if (head) {
        head->key = key; 
        head->size = sz;
        head->from = from;
        head->to = to;
    }
}

static bus_pipe_t*
_bus_pipe_create(bus_pipe_head* head, int32_t create) {
    if (!head) return NULL;
    bus_pipe_t* bp = (bus_pipe_t*)MALLOC(sizeof(*bp));
    assert(bp);
    bp->head = head;
    // extend ring-buffer head
    struct shm_t* shm = shm_create(head->key, head->size + rbuffer_head_size(), create);
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
_bus_pipe_release(bus_pipe_t* bp) {
    if (bp) FREE(bp);
}

static struct rbuffer_t*
_bus_pipe_rbuffer(bus_pipe_t* bp) {
    return bp ? bp->r : NULL;
}

static bus_pipe_head*
_bus_pipe_head(bus_pipe_t* bp) {
    return bp ? bp->head : NULL;
}

static void
_bus_head_init(bus_head* head, int32_t key, bus_addr_t creator) {
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

static int32_t
_bus_head_create(bus_t* bt, int16_t key) {
    int32_t buskey = (key << 16);
    struct shm_t* shm = shm_create(buskey, sizeof(bus_head), 0);
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
    int32_t i;
    assert(bt && bt->head);
    bt->tver = bt->head->tver;
    bt->tcount = bt->head->tcount;
    for (i = 0; i < bt->tcount; ++ i) {
        bt->terms[i] = bt->head->terms[i];
    }
}

static void
_bus_update_pipes(bus_t* bt) {
    assert(bt && bt->head);
    bt->pver = bt->head->pver;
    for (int i = 0; i < bt->head->pcount; ++ i) {
        bus_pipe_head* phead = &bt->head->pipes[i]; 
        if (phead->from == bt->self && !idtable_get(bt->opipes, phead->to)) {
            bus_pipe_t* bp = _bus_pipe_create(phead, 1);
            assert(bp);
            int ret = idtable_add(bt->opipes, phead->to, bp);
            assert(0 == ret);
        } else if (phead->to == bt->self && !idtable_get(bt->ipipes, phead->from)) {
            bus_pipe_t* bp = _bus_pipe_create(phead, 1);
            assert(bp);
            int ret = idtable_add(bt->ipipes, phead->from, bp);
            assert(0 == ret);
        }
    }
}

static int32_t
_bus_create_attach(bus_t* bt, int16_t key) {
    int32_t buskey = (key << 16);
    int32_t i;
    struct shm_t* shm;
    int32_t registered = -1;
    // attach exsit shm
    shm = shm_create(buskey, sizeof(bus_head), 1);
    assert(shm);
    bt->head = (bus_head*)shm_mem(shm);
    if (bt->head->key != buskey || bt->head->size != sizeof(bus_head)) {
        return -1;
    }
    // register terminal info
    for (i = 0; i < bt->head->tcount; ++ i) {
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
    _bus_pipe_release((bus_pipe_t*)data);
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
    uint32_t tver, pver;
    if (!bt) return;
    // update terminals
    tver = bt->head->tver;
    if (tver != bt->tver) {
        lock_lock(bt->lock);
        _bus_update_terminals(bt);
        lock_unlock(bt->lock);
    }
    // udpate pipe 
    pver = bt->head->pver;
    if (pver != bt->pver) {
        lock_lock(bt->lock);
        _bus_update_pipes(bt);
        lock_unlock(bt->lock);
    }
}

static bus_pipe_t* 
_bus_register_pipe(bus_t* bt, bus_addr_t to, size_t sz) {
    if (!bt) return NULL;
    // make sure same version
    bus_poll(bt);

    // add lock
    bus_head* head = bt->head;
    lock_lock(bt->lock);
    if (bt->pver != head->pver) goto PIPE_CREATE_FAIL;

    // validate
    if (head->pcount >= BUS_MAX_PIPE_COUNT) goto PIPE_CREATE_FAIL;
    int exist = -1;
    for (int i = 0; i < head->tcount; ++ i) {
        if (head->terms[i] == to) {
            exist = 0;
            break;
        }
    }
    if (exist != 0 || to == bt->self) goto PIPE_CREATE_FAIL;

    // create pipe 
    int key = head->key + (++ head->ckey);
    bus_pipe_head* bph = &head->pipes[head->pcount];
    _bus_pipe_head_assign(bph, key, bt->self, to, sz);
    bus_pipe_t* bp = _bus_pipe_create(bph, 0);
    if (!bp) goto PIPE_CREATE_FAIL;
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

int32_t
bus_send(bus_t* bt, const char* buf, size_t bufsz, bus_addr_t to) {
    struct bus_pipe_t* bp;
    int32_t ret;
    if (!bt) return BUS_ERR_FAIL;
    bp = (bus_pipe_t*)idtable_get(bt->opipes, to);
    if (!bp) {
        bp = _bus_register_pipe(bt, to, BUS_PIPE_DEFAULT_SIZE);
        if (!bp) return BUS_ERR_PIPE_FAIL;
    }
    ret = rbuffer_write(_bus_pipe_rbuffer(bp), buf, bufsz);
    return ret == 0 ? BUS_OK : BUS_ERR_SEND_FAIL;
}

int32_t
bus_send_by_type(bus_t* bt, const char* buf, size_t bufsz, int type) {
    int i, ret = -1;
    for (i = 0; i < bt->tcount; ++ i) {
        if (bt->terms[i] != bt->self && bus_addr_type(bt->terms[i]) == type) {
            ret = bus_send(bt, buf, bufsz, bt->terms[i]);
            if (ret) return ret;
        }
    }
    return ret ? BUS_ERR_PEER_NOT_FOUND : BUS_OK;
}

int32_t
bus_send_all(bus_t* bt, const char* buf, size_t bufsz) {
    int32_t i, ret;
    for (i = 0; i < bt->tcount; ++ i) {
        if (bt->terms[i] != bt->self) {
            ret = bus_send(bt, buf, bufsz, bt->terms[i]);
            if (ret) { return ret; }
        }
    }
    return BUS_OK;
}

int32_t
bus_recv(bus_t* bt, char* buf, size_t* bufsz, bus_addr_t from) {
    struct bus_pipe_t* bp;
    int32_t ret;
    if (!bt) return BUS_ERR_FAIL;
    bp = (struct bus_pipe_t*)idtable_get(bt->ipipes, from);
    if (!bp) return BUS_ERR_PEER_NOT_FOUND;
    ret = rbuffer_read(_bus_pipe_rbuffer(bp), buf, bufsz);
    return ret == 0 ? BUS_ERR_EMPTY : BUS_OK;
}

typedef struct bus_loop_param_t {
    size_t* size;
    char* buf;
    bus_addr_t* from;
} bus_loop_param_t;

static int
_bus_recv_loop(void* data, void* arg) {
    bus_pipe_t* bp = (bus_pipe_t*)data;
    bus_pipe_head* head = _bus_pipe_head(bp);
    bus_loop_param_t* param = (bus_loop_param_t*)arg;
    int ret = rbuffer_read(_bus_pipe_rbuffer(bp), param->buf, param->size);
    if (ret == 0) {
        *(param->from) = head->from;
        return 1;
    }
    return 0;
}

int32_t
bus_recv_all(bus_t* bt, char* buf, size_t* bufsz, bus_addr_t* from) {
    static int index = 0;
    if (!bt || !buf || !bufsz || !from) {
        return BUS_ERR_FAIL;
    }
    bus_loop_param_t param;
    param.size = bufsz;
    param.buf = buf;
    param.from = from;
    int ret = idtable_loop(bt->ipipes, _bus_recv_loop, &param, index ++);
    return ret == 0 ? BUS_ERR_EMPTY : BUS_OK;
}

typedef struct bus_dump_param_t {
    char* debug;
    size_t sz;
} bus_dump_param_t;

static int
_bus_dump_loop(void* data, void* arg) {
    bus_pipe_t* bp = (bus_pipe_t*)data;
    bus_pipe_head* head = _bus_pipe_head(bp);
    struct rbuffer_t* r = _bus_pipe_rbuffer(bp);
    bus_dump_param_t* param = (bus_dump_param_t*)arg;
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
    bus_dump_param_t param;
    param.debug = debug;
    param.sz = debugsz;
    idtable_loop(bt->ipipes, _bus_dump_loop, &param, 0);
    idtable_loop(bt->opipes, _bus_dump_loop, &param, 0);
}

uint32_t
bus_send_bytes(bus_t* bt, bus_addr_t to) {
    assert(bt);
    bus_pipe_t* bp = (bus_pipe_t*)idtable_get(bt->opipes, to);
    return bp ? rbuffer_write_bytes(_bus_pipe_rbuffer(bp)) : 0;
}

uint32_t
bus_recv_bytes(bus_t* bt, bus_addr_t from) {
    assert(bt);
    bus_pipe_t* bp = (bus_pipe_t*)idtable_get(bt->ipipes, from);
    return bp ? rbuffer_read_bytes(_bus_pipe_rbuffer(bp)) : 0;
}

