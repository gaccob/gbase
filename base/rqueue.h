#ifndef RQUEUE_H_
#define RQUEUE_H_

// it's a ring queue
// likes a ring buffer, it's also lock-free

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct rqueue_t rqueue_t;

rqueue_t* rqueue_create(size_t size);
void rqueue_release(rqueue_t* q);
void* rqueue_push_back(rqueue_t* q, void* data);
void* rqueue_pop_front(rqueue_t* q);
void* rqueue_head(rqueue_t* q);
int rqueue_is_empty(rqueue_t* q);
int rqueue_is_full(rqueue_t* q);

#ifdef __cplusplus
}
#endif

#endif // RQUEUE_H_


