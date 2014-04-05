#include "thread.h"

void*
thread_lock_alloc() {
#if defined(OS_WIN)
    CRITICAL_SECTION* lock = (CRITICAL_SECTION*)MALLOC(sizeof(CRITICAL_SECTION));
    if (!lock) return NULL;
    if (InitializeCriticalSectionAndSpinCount(lock, THREAD_SPIN_COUNT) == 0) {
        FREE(lock);
        return NULL;
    }
    return lock;
#elif defined(OS_LINUX) || defined(OS_MAC)
    pthread_mutex_t* lock = (pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
    if(!lock) return NULL;
    if(pthread_mutex_init(lock, NULL)) {
        FREE(lock);
        return NULL;
    }
    return lock;
#endif
    return NULL;
}

void
thread_lock_free(void* lock) {
#if defined(OS_WIN)
    CRITICAL_SECTION* thread_lock = (CRITICAL_SECTION*)lock;
    DeleteCriticalSection(thread_lock);
    FREE(thread_lock);
#elif defined(OS_LINUX) || defined(OS_MAC)
    pthread_mutex_t* thread_lock = (pthread_mutex_t*)lock;
    if(thread_lock) {
        pthread_mutex_destroy(thread_lock);
        FREE(thread_lock);
    }
#endif
}

int
thread_lock(void* lock) {
#if defined(OS_WIN)
    CRITICAL_SECTION* thread_lock = (CRITICAL_SECTION*)lock;
    EnterCriticalSection(thread_lock);
    return 0;
#elif defined(OS_LINUX) || defined(OS_MAC)
    pthread_mutex_t* thread_lock = (pthread_mutex_t*)lock;
    return pthread_mutex_lock(thread_lock);
#endif
    return -1;
}

int
thread_unlock(void* lock) {
#if defined(OS_WIN)
    CRITICAL_SECTION* thread_lock = (CRITICAL_SECTION*)lock;
    LeaveCriticalSection(thread_lock);
    return 0;
#elif defined(OS_LINUX) || defined(OS_MAC)
    pthread_mutex_t* thread_lock = (pthread_mutex_t*)lock;
    return pthread_mutex_unlock(thread_lock);
#endif
    return -1;
}

#if defined(OS_WIN)
struct Win32ThreadCond {
    HANDLE event;
    CRITICAL_SECTION lock;
    int n_waiting;
    int n_to_wake;
    int generation;
};
#endif

void*
thread_cond_alloc() {
#if defined(OS_WIN)
    int32_t ret;
    struct Win32ThreadCond* cond;
    cond = (struct Win32ThreadCond*)MALLOC(sizeof(struct Win32ThreadCond));
    if (!cond) return NULL;
    ret = InitializeCriticalSectionAndSpinCount(&cond->lock, THREAD_SPIN_COUNT);
    if (0 == ret) {
        FREE(cond);
        return NULL;
    }
    cond->event = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (NULL == cond->event) {
        DeleteCriticalSection(&cond->lock);
        FREE(cond);
        return NULL;
    }
    cond->n_waiting = cond->n_to_wake = cond->generation = 0;
    return cond;

#elif defined(OS_LINUX) || defined(OS_MAC)
    pthread_cond_t* cond = (pthread_cond_t*)MALLOC(sizeof(pthread_cond_t));
    if(!cond) return NULL;
    if(pthread_cond_init(cond, NULL)) {
        FREE(cond);
        return NULL;
    }
    return cond;
#endif
}

void
thread_cond_free(void* cond) {
#if defined(OS_WIN)
    struct Win32ThreadCond* thread_cond = (struct Win32ThreadCond*)cond;
    DeleteCriticalSection(&thread_cond->lock);
    CloseHandle(thread_cond->event);
    FREE(thread_cond);
#elif defined(OS_LINUX) || defined(OS_MAC)
    pthread_cond_t* thread_cond = (pthread_cond_t*)cond;
    pthread_cond_destroy(thread_cond);
    FREE(cond);
#endif
}

int
thread_cond_signal(void* cond, int broadcast) {
#if defined(OS_WIN)
    struct Win32ThreadCond* thread_cond = (struct Win32ThreadCond*)cond;
    EnterCriticalSection(&thread_cond->lock);
    if (broadcast)
        thread_cond->n_to_wake = thread_cond->n_waiting;
    else
        ++thread_cond->n_to_wake;
    thread_cond->generation++;
    SetEvent(thread_cond->event);
    LeaveCriticalSection(&thread_cond->lock);
    return 0;
#elif defined(OS_LINUX) || defined(OS_MAC)
    int32_t ret;
    pthread_cond_t* thread_cond = (pthread_cond_t*)cond;
    if(broadcast)
        ret = pthread_cond_broadcast(thread_cond);
    else
        ret = pthread_cond_signal(thread_cond);
    return ret ? -1 : 0;
#endif
    return -1;
}

int
thread_cond_wait(void* cond, void* lock, const struct timeval* tv) {
#if defined(OS_WIN)
    struct Win32ThreadCond* thread_cond = (struct Win32ThreadCond*)cond;
    CRITICAL_SECTION* thread_lock = (CRITICAL_SECTION*)lock;
    int generation_at_start;
    int waiting = 1;
    int result = -1;
    DWORD ms = INFINITE, ms_orig = INFINITE, startTime, endTime;
    if(tv)
        ms_orig = ms =  (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);

    EnterCriticalSection(&thread_cond->lock);
    ++thread_cond->n_waiting;
    generation_at_start = thread_cond->generation;
    LeaveCriticalSection(&thread_cond->lock);

    LeaveCriticalSection(thread_lock);
    startTime = GetTickCount();
    do {
        DWORD res;
        res = WaitForSingleObject(thread_cond->event, ms);
        EnterCriticalSection(&thread_cond->lock);
        if(thread_cond->n_to_wake && thread_cond->generation != generation_at_start) {
            -- thread_cond->n_to_wake;
            -- thread_cond->n_waiting;
            result = 0;
            waiting = 0;
            goto FINISH;
        } else if(res != WAIT_OBJECT_0) {
            result = (res == WAIT_TIMEOUT) ? 1 : -1;
            --thread_cond->n_waiting;
            waiting = 0;
            goto FINISH;
        } else if(ms != INFINITE) {
            endTime = GetTickCount();
            if (startTime + ms_orig <= endTime) {
                result = 1;
                --thread_cond->n_waiting;
                waiting = 0;
                goto FINISH;
            } else {
                ms = startTime + ms_orig - endTime;
            }
        }
        // If we make it here, we are still waiting.
        if (thread_cond->n_to_wake == 0) {
            // There is nobody else who should wake up; reset the event.
            ResetEvent(thread_cond->event);
        }
    FINISH:
        LeaveCriticalSection(&thread_cond->lock);
    } while(waiting);

    EnterCriticalSection(thread_lock);
    EnterCriticalSection(&thread_cond->lock);
    if (!thread_cond->n_waiting)
        ResetEvent(thread_cond->event);
    LeaveCriticalSection(&thread_cond->lock);
    return result;

#elif defined(OS_LINUX) || defined(OS_MAC)
    int32_t ret;
    pthread_cond_t* thread_cond = (pthread_cond_t*)cond;
    pthread_mutex_t* thread_lock = (pthread_mutex_t*)lock;
    if (tv) {
        // calculate end-time
        struct timeval now;
        struct timespec ts;
        gettimeofday(&now, NULL);
        ts.tv_sec = now.tv_sec + tv->tv_sec;
        ts.tv_nsec = (now.tv_usec + tv->tv_usec) * 1000;
        while (ts.tv_nsec >= 1000000000) {
            ts.tv_sec ++;
            ts.tv_nsec -= 1000000000;
        }
        ret = pthread_cond_timedwait(thread_cond, thread_lock, &ts);
        if (ETIMEDOUT == ret) return 1;
        else if (ret) return -1;
        else return 0;
    } else {
        ret = pthread_cond_wait(thread_cond, thread_lock);
        return ret ? -1 : 0;
    }
#endif
    return -1;
}


