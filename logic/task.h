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

struct task_step_t;
struct task_t;

enum E_TASK_RET
{
    TASK_RET_FAIL = -1,
    TASK_RET_SUCCESS = 0,
    TASK_RET_NEXT,
    TASK_RET_RUNNING,
};

// return E_TASK_RET
typedef int32_t (*task_run_func)(struct task_step_t*);
typedef void (*task_release_data_func)(void*);

struct task_step_t*
task_step_create(task_run_func run, void* param);

void
task_step_release(struct task_step_t* ts);

struct task_t*
task_step_task(struct task_step_t* ts);

uint32_t
task_step_id(struct task_step_t* ts);

void*
task_step_param(struct task_step_t* ts);

// return E_TASK_RET
// if last step and return TASK_RET_NEXT, then go loop to head step,
// as it's organized as double-link list
int32_t
task_step_run(struct task_step_t* ts);

// step could be < 0, which means go backward
void
task_step_resume(struct task_step_t* ts, int ret, int step);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

typedef void (*task_success_func)(struct task_t*);
// timeout == 0 mean fail because of timeout
typedef void (*task_fail_func)(struct task_t*, int is_timeout);

struct task_t*
task_create(task_success_func, task_fail_func, void* param);

void
task_release(struct task_t*);

int32_t
task_is_finished(struct task_t*);

// if no timer, set timer and timeout as NULL to ignore
void
task_run(struct task_t*, struct timerheap_t* timer, struct timeval* timeout);

uint32_t
task_id(struct task_t*);

void
task_push_back_step(struct task_t*, struct task_step_t*);
void
task_erase_step(struct task_t*, struct task_step_t*);
struct task_step_t*
task_get_step(struct task_t*, uint32_t id);

void*
task_param(struct task_t*);

#ifdef __cplusplus
}
#endif

#endif // TASK_H_
