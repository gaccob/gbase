#ifndef LOCK_H_
#define LOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

// process lock
// for linux/unix: system V semaphore
// for windows: mutex

#include "core/os_def.h"

struct lock_t;

struct lock_t* lock_create(int key);
void lock_release(struct lock_t* l);
void lock_destroy(struct lock_t* l);

int lock_lock(struct lock_t* l);
int lock_try_lock(struct lock_t* l);
int lock_unlock(struct lock_t* l);

#ifdef __cplusplus
}
#endif

#endif
