#include <assert.h>

#include "core/atom.h"
#include "rqueue.h"

struct rqueue_t {
    void** data;
    volatile atom_t read_pos;
    volatile atom_t write_pos;
    size_t size;
};

rqueue_t*
rqueue_create(size_t size) {
    rqueue_t* q = (rqueue_t*)MALLOC(sizeof(rqueue_t));
    if (!q) {
        return NULL;
    }
    q->data = (void**)MALLOC(sizeof(void*) * size);
    if (!q->data) {
        FREE(q);
        return NULL;
    }
    atom_set(&q->read_pos, 0);
    atom_set(&q->write_pos, 0);
    q->size = size;
    return q;
}

void
rqueue_release(rqueue_t* q) {
    if (q) {
        FREE(q->data);
        FREE(q);
    }
}

void*
rqueue_push_back(rqueue_t* q, void* data) {
    if (rqueue_is_full(q)) return NULL;
    q->data[q->write_pos] = data;
    atom_set(&q->write_pos, (q->write_pos + 1) % q->size);
    return data;
}

void*
rqueue_pop_front(rqueue_t* q) {
    void* data;
    if (rqueue_is_empty(q)) return NULL;
    data = q->data[q->read_pos];
    atom_set(&q->read_pos, (q->read_pos + 1) % q->size);
    return data;
}

void*
rqueue_head(rqueue_t* q) {
    if (rqueue_is_empty(q)) return NULL;
    return q->data[q->read_pos];
}

int
rqueue_is_empty(rqueue_t* q) {
    assert(q);
    return q->read_pos == q->write_pos ? 0 : -1;
}

int
rqueue_is_full(rqueue_t* q) {
    assert(q);
    return q->read_pos == ((q->write_pos + 1) % q->size) ? 0 : -1;
}

