#include "process_lock.h"

#if defined(OS_WIN)
#else
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#endif

typedef struct process_lock_t {
    int key;
#if defined(OS_WIN)
    char name[64];
    HANDLE mutex;
#else
    int semid;
#endif
} process_lock_t;

struct process_lock_t*
process_lock_create(int key) {
    process_lock_t* pl = (process_lock_t*)MALLOC(sizeof(*pl));
    pl->key = key;
#if defined (OS_WIN)
    snprintf(pl->name, sizeof(pl->name), "gbase_mutex_%d", key);
    pl->mutex = CreateMutex(NULL, FALSE, pl->name);
    if (!pl->mutex) {
        FREE(pl);
        return NULL;
    }
#else
    pl->semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (pl->semid < 0) {
        if (errno == EEXIST) {
            pl->semid = semget(key, 1, IPC_CREAT | 0666);
            if (pl->semid < 0) {
                FREE(pl);
                return NULL;
            }
        }
    } else {
        union {
            int val;
            struct semid_ds* buf;
            unsigned short* arrary;
        } sm;
        sm.val = 1;
        if (semctl(pl->semid, 0, SETVAL, sm)) {
            FREE(pl);
            return NULL;
        }
    }
#endif
    return pl;
}

int
process_lock_lock(struct process_lock_t* pl) {
    if (pl) {
#if defined (OS_WIN)
        return WaitForSingleObject(pl->mutex, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
#else
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = -1;
        sm.sem_flg = 0;
        return semop(pl->semid, &sm, 1);
#endif
    }
    return -1;
}

int
process_lock_try_lock(struct process_lock_t* pl) {
    if (pl) {
#if defined (OS_WIN)
        return WaitForSingleObject(pl->mutex, 0) == WAIT_OBJECT_0 ? 0 : -1;
#else
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = -1;
        sm.sem_flg = IPC_NOWAIT;
        return semop(pl->semid, &sm, 1);
#endif
    }
    return -1;

}

int
process_lock_unlock(struct process_lock_t* pl) {
    if (pl) {
#if defined (OS_WIN)
        return ReleaseMutex(pl->mutex) == TRUE ? 0 : -1;
#else
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = 1;
        sm.sem_flg = 0;
        return semop(pl->semid, &sm, 1);
#endif
    }
    return -1;
}

void
process_lock_release(struct process_lock_t* pl) {
    if (pl) {
        FREE(pl);
    }
}

void
process_lock_destroy(struct process_lock_t* pl) {
    if (pl) {
#if defined (OS_WIN)
        // nothing to do
#else
        semctl(pl->semid, 0, IPC_RMID, 1);
#endif
    }
}

