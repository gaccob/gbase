#include <assert.h>

#include "ds/rqueue.h"
#include "core/atom.h"

typedef struct rqueue_t
{
    void** data;
    volatile atom_t read_pos;
    volatile atom_t write_pos;
    size_t size;
} rqueue_t;

struct rqueue_t* rqueue_init(size_t size)
{
    struct rqueue_t* q = (struct rqueue_t*)MALLOC(sizeof(struct rqueue_t));
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

void rqueue_release(struct rqueue_t* q)
{
    if (q) {
        FREE(q->data);
        FREE(q);
    }
}

void* rqueue_push_back(struct rqueue_t* q, void* data)
{
    if (rqueue_is_full(q)) return NULL;
    q->data[q->write_pos] = data;
    atom_set(&q->write_pos, (q->write_pos + 1) % q->size);
    return data;
}

void* rqueue_pop_front(struct rqueue_t* q)
{
    void* data;
    if (rqueue_is_empty(q)) return NULL;
    data = q->data[q->read_pos];
    atom_set(&q->read_pos, (q->read_pos + 1) % q->size);
    return data;
}

void* rqueue_head(struct rqueue_t* q)
{
    if (rqueue_is_empty(q)) return NULL;
    return q->data[q->read_pos];
}

int rqueue_is_empty(struct rqueue_t* q)
{
    assert(q);
    return q->read_pos == q->write_pos ? 0 : -1;
}

int rqueue_is_full(struct rqueue_t* q)
{
    assert(q);
    return q->read_pos == ((q->write_pos + 1) % q->size) ? 0 : -1;
}


