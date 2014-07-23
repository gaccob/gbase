#ifndef LOCK_H_
#define LOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

// process lock: system V semaphore

#include "core/os_def.h"

typedef struct lock_t lock_t;

lock_t* lock_create(int key);
void lock_release(lock_t*);
void lock_destroy(lock_t*);

int lock_lock(lock_t*);
int lock_try_lock(lock_t*);
int lock_unlock(lock_t*);

#ifdef __cplusplus
}
#endif

#endif
