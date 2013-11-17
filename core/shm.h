#ifndef _SHM_H_
#define _SHM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct shm_t;

#if defined (OS_WIN)
typedef HANDLE shm_id_t
#else
typedef int shm_id_t;
#endif

#define SHM_INVALID_ID 0

// if exist, attach
struct shm_t* shm_create(int shmkey, size_t size);

shm_id_t shm_id(struct shm_t* shm);

size_t shm_size(struct shm_t* shm);

// in different process, same share memory may get different address
// it's OK, as it's virtual address -->  same physical address is ensured
void* shm_mem(struct shm_t* shm);

void shm_destroy(struct shm_t* shm);

void shm_release(struct shm_t* shm);

#ifdef __cplusplus
}
#endif

#endif
