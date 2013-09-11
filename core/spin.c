#include "spin.h"

#if defined(OS_WIN)
    #if !defined(_MSC_VER)
        #error  windows but not MSC
    #else
        #if !defined(SPIN_IMPL_INTLOCK)
        #define SPIN_IMPL_INTLOCK 1
        #endif
    #endif
#elif defined(OS_LINUX) || defined(OS_MAC)
    // from 4.1.2
    #if (GCC_VERSION < 40102)
        #if !defined(SPIN_IMPL_PTHREAD)
        #define SPIN_IMPL_PTHREAD 1
        #endif
    #else
        #if !defined(SPIN_IMPL_BUILTIN)
        #define SPIN_IMPL_BUILTIN 1
        #endif
    #endif
#endif

typedef struct spin_lock_t
{
    #if defined(SPIN_IMPL_PTHREAD)
        pthread_spinlock_t spin;
    #elif defined(SPIN_IMPL_BUILTIN)
        volatile uint32_t spin;
    #else
        volatile LONG spin;
    #endif
}spin_lock_t;

struct spin_lock_t* spin_init()
{
    struct spin_lock_t* lock = (struct spin_lock_t*)MALLOC(sizeof(struct spin_lock_t));
    if(!lock)
        return NULL;

#if defined(SPIN_IMPL_INTLOCK)
    InterlockedExchange(&lock->spin, 0);
#elif defined(SPIN_IMPL_BUILTIN)
    __sync_lock_test_and_set(&lock->spin, 0);
#elif defined(SPIN_IMPL_PTHREAD)
    pthread_spin_init(&lock->spin, PTHREAD_PROCESS_SHARED);
#endif

    return lock;
}

void spin_release(struct spin_lock_t* lock)
{
    if(lock)
    {
    #if defined(SPIN_IMPL_PTHREAD)
        pthread_spin_destroy(&lock->spin);
    #else
        spin_lock(lock);
    #endif
        FREE(lock);
    }
}

void spin_lock(struct spin_lock_t* lock)
{
#if defined(SPIN_IMPL_INTLOCK)
    while(InterlockedExchange(&lock->spin, 1)) {}
#elif defined(SPIN_IMPL_BUILTIN)
    while(__sync_lock_test_and_set(&lock->spin, 1)) {}
#elif defined(SPIN_IMPL_PTHREAD)
    pthread_spin_lock(&lock->spin);
#endif
}

int spin_trylock(struct spin_lock_t* lock)
{
#if defined(SPIN_IMPL_INTLOCK)
    return !InterlockedExchange(&lock->spin, 1);
#elif defined(SPIN_IMPL_BUILTIN)
    return !__sync_lock_test_and_set(&lock->spin, 1);
#elif defined(SPIN_IMPL_PTHREAD)
    pthread_spin_trylock(&lock->spin);
#endif
}

void spin_unlock(struct spin_lock_t* lock)
{
#if defined(SPIN_IMPL_INTLOCK)
    InterlockedExchange(&lock->spin, 0);
#elif defined(SPIN_IMPL_BUILTIN)
    __sync_lock_release(&lock->spin);
#elif defined(SPIN_IMPL_PTHREAD)
    pthread_spin_unlock(&lock->spin);
#endif
}

