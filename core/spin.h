#ifndef SPIN_H_
#define SPIN_H_

//
// spin lock type, unlike a mutex, it's running under user-level
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct spin_lock_t spin_t;

spin_t* spin_create();
void spin_release(spin_t* lock);
void spin_lock(spin_t* lock);
int spin_trylock(spin_t* lock);
void spin_unlock(spin_t* lock);

#ifdef __cplusplus
}
#endif

#endif // SPIN_H_

