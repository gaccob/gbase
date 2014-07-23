#ifndef THREAD_H_
#define THREAD_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "core/os_def.h"

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


