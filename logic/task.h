#ifndef TASK_H_
#define TASK_H_

//
// for asynchronous task abstract
// task is organized as step1, step2 ..
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "base/idtable.h"
#include "base/timer.h"

#define TASK_INVALID_ID 0
#define TASK_NAME_LEN 32

typedef struct task_step_t task_step_t;
typedef struct task_t task_t;

enum E_TASK_RET
{
    TASK_RET_FAIL = -1,
    TASK_RET_SUCCESS = 0,
    TASK_RET_NEXT,
    TASK_RET_RUNNING,
};

// return E_TASK_RET
typedef int32_t (*task_run_func)(task_step_t*);
typedef void (*task_release_data_func)(void*);

task_step_t* task_step_create(task_run_func, void* param);
void task_step_release(task_step_t*);
task_t* task_step_task(task_step_t*);
uint32_t task_step_id(task_step_t*);
void* task_step_param(task_step_t*);

// return E_TASK_RET
// if last step and return TASK_RET_NEXT, then go loop to head step,
// as it's organized as double-link list
int task_step_run(task_step_t*);

// step could be < 0, which means go backward
void task_step_resume(task_step_t*, int ret, int step);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

typedef void (*task_success_func)(task_t*);
// timeout == 0 mean fail because of timeout
typedef void (*task_fail_func)(task_t*, int is_timeout);

task_t* task_create(task_success_func, task_fail_func, void* param);
void task_release(task_t*);
int task_is_finished(task_t*);

// if no timer, set timer and timeout as NULL to ignore
void task_run(task_t*, timerheap_t* timer, struct timeval* timeout);
uint32_t task_id(task_t*);
void task_push_back_step(task_t*, task_step_t*);
void task_erase_step(task_t*, task_step_t*);
task_step_t* task_get_step(task_t*, uint32_t id);
void* task_param(task_t*);

#ifdef __cplusplus
}
#endif

#endif // TASK_H_
