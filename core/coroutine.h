#ifndef COROUTINE_H_
#define COROUTINE_H_

// a simple coroutine monitor
// copy stack while switch conext, so it's not fit for heavy context
// now only works under linux (not for unix and mac)

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#define CRT_INVALID_ID -1

#define CRT_DEAD 0
#define CRT_INIT 1
#define CRT_RUNNING 2
#define CRT_SUSPEND 3

struct crt_t;
typedef struct crt_t crt_t;

typedef void (*crt_func_t)(crt_t*, void* arg);

crt_t* crt_create(int crt_stack_size);
void crt_release(crt_t*);
int crt_new(crt_t*, crt_func_t, void* arg);
void crt_resume(crt_t*, int id);
int crt_status(crt_t*, int id);
int crt_current(crt_t*);
char* crt_current_stack_top(crt_t*);
void crt_yield(crt_t*);

#ifdef __cplusplus
}
#endif

#endif // COROUTINE_H_
