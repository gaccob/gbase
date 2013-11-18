#ifndef PROCESS_LOCK_H_
#define PROCESS_LOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// process lock
// for linux/unix: system V semaphore
// for windows: mutex
//

#include "core/os_def.h"

struct process_lock_t;

struct process_lock_t* process_lock_init(int key);
void process_lock_release(struct process_lock_t* pl);
void process_lock_destroy(struct process_lock_t* pl);

int process_lock_lock(struct process_lock_t* pl);
int process_lock_try_lock(struct process_lock_t* pl);
int process_lock_unlock(struct process_lock_t* pl);

#ifdef __cplusplus
}
#endif

#endif
