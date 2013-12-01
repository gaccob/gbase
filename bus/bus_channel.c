#include <assert.h>
#include "bus/bus_channel.h"
#include "core/rbuffer.h"
#include "mm/shm.h"

typedef struct bus_terminal_channel_t
{
    bus_channel_t* bc;
    struct rbuffer_t* r;
} bus_terminal_channel_t;

// if create == 0: create new share-memory channel
// else: attach exist share-memory channel and verify data
bus_terminal_channel_t* bus_terminal_channel_init(int32_t shmkey, bus_addr_t from,
                                                  bus_addr_t to, size_t size,
                                                  int32_t create)
{
    struct shm_t* shm;
    bus_terminal_channel_t* btc;
    size_t alloc_size = size + sizeof(bus_channel_t) + rbuffer_head_size();

    // init terminal channel
    btc = (bus_terminal_channel_t*)MALLOC(sizeof(*btc));
    if (!btc) {
        return NULL;
    }

    // share memory
    if (create == 0) {
        shm = shm_create(shmkey, alloc_size, 0);
    } else {
        shm = shm_create(shmkey, alloc_size, 1);
    }
    if (!shm) {
        FREE(btc);
        return NULL;
    }

    // init channel or verify channel data
    btc->bc = shm_mem(shm);
    assert(btc->bc);
    if (create == 0) {
        btc->bc->shmkey = shmkey;
        btc->bc->from = from;
        btc->bc->to = to;
        btc->bc->channel_size = size;
    } else {
        if (btc->bc->shmkey != shmkey || btc->bc->from != from
            || btc->bc->to != to || btc->bc->channel_size != size) {
            FREE(btc);
            return NULL;
        }
    }

    // init ring buffer
    btc->r = rbuffer_init_mem((char*)btc->bc + sizeof(bus_channel_t),
        btc->bc->channel_size + rbuffer_head_size());
    if (!btc->r) {
        FREE(btc);
        return NULL;
    }
    return btc;
}

void bus_terminal_channel_release(bus_terminal_channel_t* btc)
{
    if (btc) {
        // rbuffer no need to release as inited from memory
        FREE(btc);
    }
}

struct rbuffer_t* bus_terminal_channel_rbuffer(bus_terminal_channel_t* btc)
{
    return btc ? btc->r : NULL;
}

bus_channel_t* bus_terminal_channel(bus_terminal_channel_t* btc)
{
    return btc ? btc->bc : NULL;
}
