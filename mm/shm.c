#include <sys/shm.h>
#include <sys/ipc.h>
#include <assert.h>
#include <unistd.h>
#include "mm/shm.h"

struct shm_t {
    shm_id_t id;
    size_t size;
    void* mem;
};

// excl == 0: means return null if shm exists
shm_t*
shm_create(int shmkey, size_t size, int excl) {
    // malloc shm
    shm_t* shm = (shm_t*)MALLOC(sizeof(*shm));
    if (!shm) return NULL;
    shm->mem = NULL;

    // size round up, pagesize must be 2^n
    shm->size = ROUNDUP2(size, getpagesize());
    shm->id = shmget(shmkey, shm->size, 0666 | IPC_CREAT | IPC_EXCL);
    if (shm->id < 0) {
        if (errno == EEXIST && excl) {
            // already exsit, validate shm size
            shm->id = shmget(shmkey, 0, 0666);
            struct shmid_ds sd;
            int ret = shmctl(shm->id, IPC_STAT, &sd);
            if (ret < 0 || (int)sd.shm_segsz != shm->size) {
                FREE(shm);
                return NULL;
            }
        } else {
            FREE(shm);
            return NULL;
        }
    }
    shm->mem = shmat(shm->id, NULL, 0);
    assert(shm->mem);
    return shm;
}

inline shm_id_t
shm_id(shm_t* shm) {
    return shm ? shm->id : SHM_INVALID_ID;
}

inline size_t
shm_size(shm_t* shm) {
    return shm ? shm->size : 0;
}

inline void*
shm_mem(shm_t* shm) {
    return shm ? shm->mem : NULL;
}

void
shm_destroy(shm_t* shm) {
    if (shm) {
        shmctl(shm->id, IPC_RMID, NULL);
        shm->id = -1;
    }
}

inline void
shm_release(shm_t* shm) {
    if (shm) FREE(shm);
}

