#include <assert.h>
#include "core/shm.h"

#if defined(OS_WIN)
    #include <windows.h>
#else
    #include <sys/shm.h>
    #include <sys/ipc.h>
#endif

typedef struct shm_t
{
#if defined(OS_WIN)
    HANDLE h;
#else
    int shmid;
#endif
    size_t size;
    void* mem;
} shm_t;

// if exist, attach
struct shm_t* shm_create(int shmkey, size_t size)
{
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
    shm->h = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, shm->size, name);
    if (!shm->h) {
        FREE(shm);
        return NULL;
    }
    shm->mem = MapViewOfFile(shm->h, FILE_MAP_ALL_ACCESS, 0, 0, shm->size);
    if (!shm->mem) {
        CloseHandle(shm->h);
        FREE(shm);
        return NULL;
    }
    return shm;
#else
    shm->shmid = shmget(shmkey, shm->size, 0666 | IPC_CREAT | IPC_EXCL);
    if (shm->shmid < 0) {
        if (errno != EEXIST) {
            FREE(shm);
            return NULL;
        }
        // already exsit, validate shm size
        shm->shmid = shmget(shmkey, 0, 0666);
        ret = shmctl(shm->shmid, IPC_STAT, &sd);
        if (ret < 0 || (int)sd.shm_segsz != shm->size) {
            FREE(shm);
            return NULL;
        }
    }
    shm->mem = shmat(shm->shmid, NULL, 0);
    assert(shm->mem);
    return shm;
#endif
}

size_t shm_size(struct shm_t* shm)
{
    return shm ? shm->size : 0;
}

void* shm_mem(struct shm_t* shm)
{
    return shm ? shm->mem : NULL;
}

void shm_destroy(struct shm_t* shm)
{
#if defined(OS_WIN)
    if (shm) {
        UnmapViewOfFile(shm->mem);
        shm->mem = NULL;
        CloseHandle(shm->h);
        shm->h = NULL;
    }
#else
    if (shm) {
        shmctl(shm->shmid, IPC_RMID, NULL);
        shm->shmid = -1;
    }
#endif
}

void shm_release(struct shm_t* shm)
{
    if (shm) FREE(shm);
}

