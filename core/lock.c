#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "lock.h"

typedef struct lock_t {
    int key;
    int semid;
} lock_t;

struct lock_t*
lock_create(int key) {
    lock_t* l = (lock_t*)MALLOC(sizeof(*l));
    l->key = key;
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
    return l;
}

int
lock_lock(struct lock_t* l) {
    if (l) {
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = -1;
        sm.sem_flg = 0;
        return semop(l->semid, &sm, 1);
    }
    return -1;
}

int
lock_try_lock(struct lock_t* l) {
    if (l) {
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = -1;
        sm.sem_flg = IPC_NOWAIT;
        return semop(l->semid, &sm, 1);
    }
    return -1;
}

int
lock_unlock(struct lock_t* l) {
    if (l) {
        struct sembuf sm;
        sm.sem_num = 0;
        sm.sem_op = 1;
        sm.sem_flg = 0;
        return semop(l->semid, &sm, 1);
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
        semctl(l->semid, 0, IPC_RMID, 1);
    }
}

