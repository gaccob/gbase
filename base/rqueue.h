#ifndef RQUEUE_H_
#define RQUEUE_H_

// it's a ring queue
// likes a ring buffer, it's also lock-free

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct rqueue_t;

struct rqueue_t* rqueue_create(size_t size);
void rqueue_release(struct rqueue_t* q);
void* rqueue_push_back(struct rqueue_t* q, void* data);
void* rqueue_pop_front(struct rqueue_t* q);
void* rqueue_head(struct rqueue_t* q);
int rqueue_is_empty(struct rqueue_t* q);
int rqueue_is_full(struct rqueue_t* q);

#ifdef __cplusplus
}
#endif

#endif // RQUEUE_H_


