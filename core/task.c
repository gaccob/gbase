#include <assert.h>
#include "task.h"

#define TASK_STEP_ID_MASK (1UL << 30)

typedef struct task_step_t
{
    uint32_t id;

    // host
    struct task_t* t;

    // double list
    struct task_step_t* next;
    struct task_step_t* prev;

    // step callback
    task_step_run_func run;

    // step data
    void* data;
    task_step_release_data_func data_release;
} task_step_t;

typedef struct task_step_result_t
{
    int8_t ret;
    int8_t flag_backward : 1; // 0 means backward, 1 means forward
    int8_t steps : 7;
} task_step_result_t;

typedef struct task_t
{
    uint32_t id;
    task_step_t* current;
    task_step_t* head;

    // callback data
    task_on_success_func on_success;
    task_on_fail_func on_fail;

    // task data
    void* data;

    // timer
    struct heaptimer_t* timer;
    int32_t timer_id;
} task_t;

int _task_timeout(void* args);
void _task_run(task_t* t);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

task_step_t* task_step_init(task_step_run_func run)
{
    static uint32_t id = 0;
    task_step_t* ts = (task_step_t*)MALLOC(sizeof(*ts));
    if (++ id >= TASK_STEP_ID_MASK) id = 1;
    ts->id = id;
    ts->next = ts->prev = NULL;
    ts->run = run;
    ts->data = NULL;
    ts->data_release = NULL;
    return ts;
}

uint32_t task_step_id(task_step_t* ts)
{
    return ts ? ts-> id : TASK_INVALID_ID;
}

task_t* task_step_task(task_step_t* ts)
{
    return ts ? ts->t : NULL;
}

void task_step_set_data(task_step_t* ts, void* data,
                        task_step_release_data_func data_release)
{
    if (ts) {
        ts->data = data;
        ts->data_release = data_release;
    }
}

void* task_step_data(task_step_t* ts)
{
    return ts ? ts->data : NULL;
}

int32_t task_step_run(task_step_t* ts)
{
    if (ts && ts->run) {
        return ts->run(ts);
    }
    return TASK_RET_FAIL;
}

void task_step_finish(task_step_t* ts, task_step_result_t* rt)
{
    if (!ts || !ts->t || !rt) return;
    switch (rt->ret) {
        // waiting for async result
        case TASK_RET_RUNNING:
            return;

        // task complete
        case TASK_RET_SUCCESS:
            ts->t->on_success(ts->t);
            break;

        // go next by step, no safety check
        case TASK_RET_NEXT:
            if (rt->flag_backward) {
                while (rt->steps --) {
                    ts->t->current = ts->t->current->next;
                }
            } else {
                while (rt->steps --) {
                    ts->t->current = ts->t->current->prev;
                }
            }
            return _task_run(ts->t);

        // task complet fail
        default:
            ts->t->on_fail(ts->t, TASK_RET_FAIL);
            break;
    }
    ts->t->current = NULL;
}

void task_step_release(task_step_t* ts)
{
    if (ts) {
        if (ts->data && ts->data_release) {
            ts->data_release(ts->data);
            ts->data = NULL;
        }
        FREE(ts);
    }
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

task_step_result_t* task_step_result_init(int32_t ret)
{
    task_step_result_t* tr = (task_step_result_t*)MALLOC(sizeof(*tr));
    tr->ret = ret;
    tr->flag_backward = 1;
    tr->steps = 1;
    return tr;
}

void task_step_result_set_backward(struct task_step_result_t* tr)
{
    if (tr) tr->flag_backward = 0;
}

void task_step_result_set_steps(struct task_step_result_t* tr, int steps)
{
    assert(steps < (1 << 7) && steps > 0);
    if (tr) tr->steps = steps;
}

void task_step_result_release(struct task_step_result_t* rt)
{
    FREE(rt);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

task_t* task_init(task_on_success_func on_success,
                  task_on_fail_func on_fail,
                  void* data)
{
    task_t* t = NULL;
    static uint32_t id = TASK_STEP_ID_MASK;
    if (!on_success || !on_fail) return NULL;
    t = (task_t*)MALLOC(sizeof(*t));
    if (++ id < TASK_STEP_ID_MASK) id = TASK_STEP_ID_MASK + 1;
    t->id = id;
    t->current = t->head = NULL;
    t->on_success = on_success;
    t->on_fail = on_fail;
    t->data = data;
    t->timer = NULL;
    t->timer_id = TIMER_INVALID_ID;
    return t;
}

int32_t task_finished(task_t* t)
{
    if (t && t->current == NULL) return 0;
    return -1;
}

uint32_t task_id(task_t* t)
{
    return t ? t->id : TASK_INVALID_ID;
}

void task_push_back_step(task_t* t, task_step_t* ts)
{
    if (t && ts) {
        if (!t->head) {
            t->head = ts;
            t->head->prev = t->head->next = t->head;
        } else {
            t->head->prev->next = ts;
            ts->prev = t->head->prev;
            t->head->prev = ts;
            ts->next = t->head;
        }
        ts->t = t;
    }
}

void task_erase_step(task_t* t, task_step_t* ts)
{
    if (t && ts) {
        ts->prev->next = ts->next;
        ts->next->prev = ts->prev;
        ts->prev = NULL;
        ts->next = NULL;
        ts->t = NULL;
    }
}

task_step_t* task_get_step(task_t* t, uint32_t id)
{
    struct task_step_t* ts = NULL;
    if (t && t->head) {
        ts = t->head;
        do {
            if (task_step_id(ts) == id) return ts;
            ts = ts->next;
        } while (ts != t->head);
    }
    return ts;
}

void* task_data(task_t* t)
{
    return t ? t->data : NULL;
}

void task_run(task_t* t, struct heaptimer_t* timer, struct timeval* timeout)
{
    if (!t || !t->head) return;
    t->current = t->head;
    if (timer && timeout) {
        t->timer = timer;
        t->timer_id = timer_register(timer, NULL, timeout, _task_timeout, t);
        assert(t->timer_id != TIMER_INVALID_ID);
    }
    return _task_run(t);
}

int _task_timeout(void* args)
{
    task_t* t = (task_t*)args;
    assert(t);
    t->on_fail(t, 0);
    t->current = NULL;
    t->timer_id = TIMER_INVALID_ID;
    return 0;
}

void _task_run(task_t* t)
{
    int32_t ret = TASK_RET_FAIL;
    if (!t || !t->head) return;
    if (!t->current) return;
    while (1) {
        ret = task_step_run(t->current);
        if (ret == TASK_RET_NEXT) {
            t->current = t->current->next;
            if (t->current == t->head) {
                t->on_success(t);
                break;
            }
        } else if (ret == TASK_RET_RUNNING) {
            // waiting for async notify
            return;
        } else if (ret == TASK_RET_SUCCESS) {
            t->on_success(t);
            break;
        } else {
            t->on_fail(t, TASK_RET_FAIL);
            break;
        }
    }
    t->current = NULL;
}

void task_release(task_t* t)
{
    task_step_t* ts = t->head;
    task_step_t* tmp = NULL;
    if (ts) {
        do {
            tmp = ts;
            ts = ts->next;
            task_step_release(tmp);
        } while (ts != t->head);
        t->head = NULL;
    }
    if (t->timer && t->timer_id != TIMER_INVALID_ID) {
        timer_unregister(t->timer, t->timer_id);
        t->timer_id = TIMER_INVALID_ID;
    }
    FREE(t);
}


