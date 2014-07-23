#include <stdint.h>
#include "spin.h"

// from 4.1.2
#if (GCC_VERSION > 40102)
    #if !defined(SPIN_GCC)
    #define SPIN_GCC 1
    #endif
#endif

typedef struct spin_lock_t {
#if defined(SPIN_GCC)
    volatile uint32_t spin;
#else
    pthread_spinlock_t spin;
#endif
} spin_t;

spin_t*
spin_create() {
    spin_t* lock = (spin_t*)MALLOC(sizeof(spin_t));
    if (!lock) return NULL;
#if defined(SPIN_GCC)
    __sync_lock_test_and_set(&lock->spin, 0);
#else
    pthread_spin_init(&lock->spin, PTHREAD_PROCESS_SHARED);
#endif
    return lock;
}

void
spin_release(spin_t* lock) {
    if (lock) {
    #if defined(SPIN_GCC)
        spin_unlock(lock);
    #else
        pthread_spin_destroy(&lock->spin);
    #endif
        FREE(lock);
    }
}

void
spin_lock(spin_t* lock) {
    if (lock) {
    #if defined(SPIN_GCC)
        while (__sync_lock_test_and_set(&lock->spin, 1)) {}
    #else
        pthread_spin_lock(&lock->spin);
    #endif
    }
}

int
spin_trylock(spin_t* lock) {
    if (lock) {
    #if defined(SPIN_GCC)
        return !__sync_lock_test_and_set(&lock->spin, 1);
    #else
        return pthread_spin_trylock(&lock->spin);
    #endif
    }
    return -1;
}

void
spin_unlock(spin_t* lock) {
    if (lock) {
    #if defined(SPIN_GCC)
        __sync_lock_release(&lock->spin);
    #else
        pthread_spin_unlock(&lock->spin);
    #endif
    }
}

