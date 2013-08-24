#ifndef SPIN_H_
#define SPIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct spin_lock_t;
struct spin_lock_t* spin_init();
void spin_release(struct spin_lock_t* lock);
void spin_lock(struct spin_lock_t* lock);
int spin_trylock(struct spin_lock_t* lock);
void spin_unlock(struct spin_lock_t* lock);

#ifdef __cplusplus
}
#endif

#endif // SPIN_H_

