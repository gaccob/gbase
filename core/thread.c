#include "thread.h"

void*
thread_lock_alloc() {
    pthread_mutex_t* lock = (pthread_mutex_t*)MALLOC(sizeof(pthread_mutex_t));
    if(!lock) return NULL;
    if(pthread_mutex_init(lock, NULL)) {
        FREE(lock);
        return NULL;
    }
    return lock;
}

void
thread_lock_free(void* lock) {
    pthread_mutex_t* thread_lock = (pthread_mutex_t*)lock;
    if(thread_lock) {
        pthread_mutex_destroy(thread_lock);
        FREE(thread_lock);
    }
}

int
thread_lock(void* lock) {
    pthread_mutex_t* thread_lock = (pthread_mutex_t*)lock;
    return pthread_mutex_lock(thread_lock);
}

int
thread_unlock(void* lock) {
    pthread_mutex_t* thread_lock = (pthread_mutex_t*)lock;
    return pthread_mutex_unlock(thread_lock);
}

void*
thread_cond_alloc() {
    pthread_cond_t* cond = (pthread_cond_t*)MALLOC(sizeof(pthread_cond_t));
    if(!cond) return NULL;
    if(pthread_cond_init(cond, NULL)) {
        FREE(cond);
        return NULL;
    }
    return cond;
}

void
thread_cond_free(void* cond) {
    pthread_cond_t* thread_cond = (pthread_cond_t*)cond;
    pthread_cond_destroy(thread_cond);
    FREE(cond);
}

int
thread_cond_signal(void* cond, int broadcast) {
    int32_t ret;
    pthread_cond_t* thread_cond = (pthread_cond_t*)cond;
    if(broadcast)
        ret = pthread_cond_broadcast(thread_cond);
    else
        ret = pthread_cond_signal(thread_cond);
    return ret ? -1 : 0;
}

int
thread_cond_wait(void* cond, void* lock, const struct timeval* tv) {
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
}


