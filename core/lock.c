#include "lock.h"

#if defined(OS_WIN)
#else
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#endif

typedef struct lock_t {
    int key;
#if defined(OS_WIN)
    char name[64];
    HANDLE mutex;
#else
    int semid;
#endif
} lock_t;

struct lock_t*
lock_create(int key) {
    lock_t* l = (lock_t*)MALLOC(sizeof(*l));
    l->key = key;
#if defined (OS_WIN)
    snprintf(l->name, sizeof(l->name), "gbase_mutex_%d", key);
    l->mutex = CreateMutex(NULL, FALSE, l->name);
    if (!l->mutex) {
        FREE(l);
        return NULL;
    }
#else
    l->semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (l->semid < 0) {
        if (errno == EEXIST) {
            l->semid = semget(key, 1, IPC_CREAT | 0666);
            if (l->semid < 0) {
                FREE(l);
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
        if (semctl(l->semid, 0, SETVAL, sm)) {
            FREE(l);
            return NULL;
        }
    }
#endif
    return l;
}

int
lock_lock(struct lock_t* l) {
    if (l) {
#if defined (OS_WIN)
        return WaitForSingleObject(l->mutex, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
#else
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = -1;
        sm.sem_flg = 0;
        return semop(l->semid, &sm, 1);
#endif
    }
    return -1;
}

int
lock_try_lock(struct lock_t* l) {
    if (l) {
#if defined (OS_WIN)
        return WaitForSingleObject(l->mutex, 0) == WAIT_OBJECT_0 ? 0 : -1;
#else
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = -1;
        sm.sem_flg = IPC_NOWAIT;
        return semop(l->semid, &sm, 1);
#endif
    }
    return -1;

}

int
lock_unlock(struct lock_t* l) {
    if (l) {
#if defined (OS_WIN)
        return ReleaseMutex(l->mutex) == TRUE ? 0 : -1;
#else
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = 1;
        sm.sem_flg = 0;
        return semop(l->semid, &sm, 1);
#endif
    }
    return -1;
}

void
lock_release(struct lock_t* l) {
    if (l) {
        FREE(l);
    }
}

void
lock_destroy(struct lock_t* l) {
    if (l) {
#if defined (OS_WIN)
        // nothing to do
#else
        semctl(l->semid, 0, IPC_RMID, 1);
#endif
    }
}

