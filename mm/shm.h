#ifndef _SHM_H_
#define _SHM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct shm_t shm_t;

#if defined (OS_WIN)
typedef HANDLE shm_id_t;
#else
typedef int shm_id_t;
#endif

#define SHM_INVALID_ID 0

// excl == 0: means return null if shm exists
shm_t* shm_create(int shmkey, size_t size, int excl);

shm_id_t shm_id(shm_t*);

size_t shm_size(shm_t*);

// in different process, same share memory may get different address
// it's OK, as it's virtual address -->  same physical address is ensured
void* shm_mem(shm_t*);

void shm_destroy(shm_t*);

void shm_release(shm_t*);

#ifdef __cplusplus
}
#endif

#endif
