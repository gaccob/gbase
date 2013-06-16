#ifndef THREAD_H_
#define THREAD_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"

#if defined(OS_WIN)
    #include <winsock2.h>
    #include <process.h>
    #include <sys/locking.h>
    #define THREAD_SPIN_COUNT 2000

    typedef HANDLE thread_t;
    #define THREAD_FUNC unsigned __stdcall
    #define THREAD_CREATE(threadvar, fn, arg) do { \
        uintptr_t threadhandle = _beginthreadex(NULL,0,fn,(arg),0,NULL); \
        (threadvar) = (thread_t) threadhandle; \
    } while (0)
    #define THREAD_JOIN(th) WaitForSingleObject(th, INFINITE)
    #define THREAD_RETURN return (0)

#elif defined(OS_LINUX) || defined(OS_MAC)
    #include <pthread.h>
    #include <sys/time.h>
    #include <errno.h>

    typedef pthread_t thread_t;
    #define THREAD_FUNC void*
    #define THREAD_CREATE(threadvar, fn, arg) \
        pthread_create(&(threadvar), NULL, fn, arg)
    #define THREAD_JOIN(th) pthread_join(th, NULL)
    #define THREAD_RETURN return (NULL)
#endif

void* thread_lock_alloc();
void thread_lock_free(void* lock);
int thread_lock(void* lock);
int thread_unlock(void* lock);

void* thread_cond_alloc();
void thread_cond_free(void* cond);
int thread_cond_signal(void* cond, int broadcast);
int thread_cond_wait(void* cond, void* lock, const struct timeval* tv);

#ifdef __cplusplus
}
#endif

#endif // THREAD_H_


