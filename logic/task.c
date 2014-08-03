#include <assert.h>
#include "task.h"

#define TASK_STEP_ID_MASK (1UL << 30)

struct task_step_t {
    uint32_t id;
    struct task_t* t;
    struct task_step_t* next;
    struct task_step_t* prev;
    task_run_func run;
    void* param;
};

struct task_t {
    uint32_t id;
    task_step_t* current;
    task_step_t* head;
    task_success_func on_success;
    task_fail_func on_fail;
    void* param;
    timerheap_t* timer;
    int timer_id;
};

static int _task_timeout(void* args);
static void _task_run(task_t* t);

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

task_step_t*
task_step_create(task_run_func run, void* param) {
    static uint32_t id = 0;
    task_step_t* ts = (task_step_t*)MALLOC(sizeof(*ts));
    if (++ id >= TASK_STEP_ID_MASK) id = 1;
    ts->id = id;
    ts->next = ts->prev = NULL;
    ts->run = run;
    ts->param = param;
    return ts;
}

inline void
task_step_release(task_step_t* ts) {
    if (ts) {
        FREE(ts);
    }
}

inline uint32_t
task_step_id(task_step_t* ts) {
    return ts ? ts-> id : TASK_INVALID_ID;
}

inline task_t*
task_step_task(task_step_t* ts) {
    return ts ? ts->t : NULL;
}

inline void*
task_step_param(task_step_t* ts) {
    return ts ? ts->param : NULL;
}

int32_t
task_step_run(task_step_t* ts) {
    if (ts && ts->run) {
        return ts->run(ts);
    }
    return TASK_RET_FAIL;
}

void
task_step_resume(task_step_t* ts, int ret, int step) {
    if (!ts) return;
    switch (ret) {
        // waiting for async result
        case TASK_RET_RUNNING:
            return;

        // task complete
        case TASK_RET_SUCCESS:
            ts->t->on_success(ts->t);
            break;

        // go next by step, no safety check
        case TASK_RET_NEXT:
            if (step > 0) {
                while (step --) {
                    ts->t->current = ts->t->current->next;
                }
            } else if (step < 0) {
                while (step ++) {
                    ts->t->current = ts->t->current->prev;
                }
            }
            _task_run(ts->t);
			return;

        // task complet fail
        default:
            ts->t->on_fail(ts->t, TASK_RET_FAIL);
            break;
    }
    ts->t->current = NULL;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

task_t*
task_create(task_success_func success, task_fail_func fail, void* param) {
    if (!success || !fail)
        return NULL;
    task_t* t = (task_t*)MALLOC(sizeof(*t));
    static uint32_t id = TASK_STEP_ID_MASK;
    if (++ id < TASK_STEP_ID_MASK) id = TASK_STEP_ID_MASK + 1;
    t->id = id;
    t->current = t->head = NULL;
    t->on_success = success;
    t->on_fail = fail;
    t->param = param;
    t->timer = NULL;
    t->timer_id = TIMER_INVALID_ID;
    return t;
}

inline int
task_is_finished(task_t* t) {
    if (t && t->current == NULL)
        return 0;
    return -1;
}

inline uint32_t
task_id(task_t* t) {
    return t ? t->id : TASK_INVALID_ID;
}

void
task_push_back_step(task_t* t, task_step_t* ts) {
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

void
task_erase_step(task_t* t, task_step_t* ts) {
    if (t && ts) {
        ts->prev->next = ts->next;
        ts->next->prev = ts->prev;
        ts->prev = NULL;
        ts->next = NULL;
        ts->t = NULL;
    }
}

task_step_t*
task_get_step(task_t* t, uint32_t id) {
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

inline void*
task_param(task_t* t) {
    return t ? t->param : NULL;
}

void
task_run(task_t* t, struct timerheap_t* timer, struct timeval* timeout) {
    if (!t || !t->head) return;
    t->current = t->head;
    if (timer && timeout) {
        t->timer = timer;
        t->timer_id = timer_register(timer, NULL, timeout, _task_timeout, t);
        assert(t->timer_id != TIMER_INVALID_ID);
    }
    _task_run(t);
}

static int
_task_timeout(void* args) {
    task_t* t = (task_t*)args;
    assert(t);
    t->on_fail(t, 0);
    t->current = NULL;
    t->timer_id = TIMER_INVALID_ID;
    return 0;
}

static void
_task_run(task_t* t) {
    if (!t || !t->head || !t->current)
        return;
    while (1) {
        int ret = task_step_run(t->current);
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

void
task_release(task_t* t) {
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

