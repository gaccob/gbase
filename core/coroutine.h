#ifndef COROUTINE_H_
#define COROUTINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#ifdef OS_LINUX

#define CRT_INVALID_ID -1

#define CRT_DEAD 0
#define CRT_INIT 1
#define CRT_RUNNING 2
#define CRT_SUSPEND 3

struct crt_t;
typedef void (*crt_func_t)(struct crt_t*, void* arg);

struct crt_t* crt_init();
void crt_release(struct crt_t*);
int crt_new(struct crt_t*, crt_func_t, void* arg);
void crt_resume(struct crt_t*, int id);
int crt_status(struct crt_t*, int id);
int crt_current(struct crt_t*);
void crt_yield(struct crt_t*);

#endif

#ifdef __cplusplus
}
#endif

#endif // COROUTINE_H_
