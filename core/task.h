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
#include "core/idtable.h"
#include "core/timer.h"

#define TASK_INVALID_ID 0
#define TASK_NAME_LEN 32

struct task_step_t;
struct task_t;
struct task_step_result_t;

enum E_TASK_RET
{
    TASK_RET_FAIL = -1,
    TASK_RET_SUCCESS = 0,
    TASK_RET_NEXT,
    TASK_RET_RUNNING,
};

// return E_TASK_RET
typedef int32_t (*task_step_run_func)(struct task_step_t*);
typedef void (*task_step_release_data_func)(void*);

struct task_step_t* task_step_init(task_step_run_func run);

struct task_t* task_step_task(struct task_step_t* ts);
uint32_t task_step_id(struct task_step_t* ts);

void task_step_set_data(struct task_step_t* ts, void* data,
                        task_step_release_data_func data_release);
void* task_step_data(struct task_step_t* ts);

//
// return E_TASK_RET
//
// if last step and return TASK_RET_NEXT, then go loop to head step,
// as it's organized as double-link list
int32_t task_step_run(struct task_step_t* ts);

void task_step_finish(struct task_step_t* ts,
                      struct task_step_result_t* rt);

void task_step_release(struct task_step_t* ts);

////////////////////////////////////////////////////////////////////////

struct task_step_result_t* task_step_result_init(int32_t ret);

void task_step_result_set_backward(struct task_step_result_t*);
void task_step_result_set_steps(struct task_step_result_t*, int steps);

void task_step_result_release(struct task_step_result_t*);

////////////////////////////////////////////////////////////////////////

typedef void (*task_on_success_func)(struct task_t*);
// timeout == 0 mean fail because of timeout
typedef void (*task_on_fail_func)(struct task_t*, int is_timeout);

struct task_t* task_init(task_on_success_func,
                         task_on_fail_func,
                         void* data);

// return 0 means finished
int32_t task_finished(struct task_t*);

uint32_t task_id(struct task_t*);

void task_push_back_step(struct task_t*, struct task_step_t*);
void task_erase_step(struct task_t*, struct task_step_t*);
struct task_step_t* task_get_step(struct task_t*, uint32_t id);

void* task_data(struct task_t*);

// if no timer, set timer=NULL, timeout=NULL to ignore
void task_run(struct task_t*, struct heaptimer_t* timer,
              struct timeval* timeout);

void task_release(struct task_t*);

#ifdef __cplusplus
}
#endif

#endif // TASK_H_
