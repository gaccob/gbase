#include <assert.h>
#include "mm/shm.h"

#if defined(OS_WIN)
    #include <windows.h>
#else
    #include <sys/shm.h>
    #include <sys/ipc.h>
#endif

typedef struct shm_t {
    shm_id_t id;
    size_t size;
    void* mem;
} shm_t;

// excl == 0: means return null if shm exists
struct shm_t*
shm_create(int shmkey, size_t size, int excl) {
#if defined(OS_WIN)
    char name[64];
#else
    struct shmid_ds sd;
    int ret;
#endif
    // malloc shm
    struct shm_t* shm = (shm_t*)MALLOC(sizeof(*shm));
    if (!shm) return NULL;
    shm->mem = NULL;

    // size round up, pagesize must be 2^n
    shm->size = ROUNDUP2(size, getpagesize());

#if defined(OS_WIN)
    snprintf(name, sizeof(name), "gbase_shm_%d", shmkey);
    shm->id = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (shm->id) {
        if (excl == 0) {
            FREE(shm);
            return NULL;
        }
    } else {
        shm->id = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
            PAGE_READWRITE, 0, shm->size, name);
        if (!shm->id) {
            FREE(shm);
            return NULL;
        }
    }
    shm->mem = MapViewOfFile(shm->id, FILE_MAP_ALL_ACCESS, 0, 0, shm->size);
    if (!shm->mem) {
        CloseHandle(shm->id);
        FREE(shm);
        return NULL;
    }
    return shm;

#else
    shm->id = shmget(shmkey, shm->size, 0666 | IPC_CREAT | IPC_EXCL);
    if (shm->id < 0) {
        if (errno == EEXIST && excl) {
            // already exsit, validate shm size
            shm->id = shmget(shmkey, 0, 0666);
            ret = shmctl(shm->id, IPC_STAT, &sd);
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
#endif
}

inline shm_id_t
shm_id(struct shm_t* shm) {
    return shm ? shm->id : SHM_INVALID_ID;
}

inline size_t
shm_size(struct shm_t* shm) {
    return shm ? shm->size : 0;
}

inline void*
shm_mem(struct shm_t* shm) {
    return shm ? shm->mem : NULL;
}

void
shm_destroy(struct shm_t* shm) {
#if defined(OS_WIN)
    if (shm) {
        UnmapViewOfFile(shm->mem);
        shm->mem = NULL;
        CloseHandle(shm->id);
        shm->id = NULL;
    }
#else
    if (shm) {
        shmctl(shm->id, IPC_RMID, NULL);
        shm->id = -1;
    }
#endif
}

inline void
shm_release(struct shm_t* shm) {
    if (shm) FREE(shm);
}

