#include <assert.h>
#include "core/os_def.h"
#include "core/task.h"
#include "base/timer.h"
#include "net/curl_client.h"
#include "net/curl_pool.h"
#include "util/util_time.h"

struct curl_pool_t* CURL_POOL;
uint32_t TASK_STEP_ID_1;

#define TASK_LOOP 3

////////////////////////////////
// t1 is a synchronous task step
////////////////////////////////
typedef struct t1_param_t {
    int32_t loop;
} t1_param_t;
t1_param_t param;

static int32_t
t1_run(struct task_step_t* ts) {
    t1_param_t* tsp = task_step_param(ts);
    assert(tsp);
    tsp->loop = TASK_LOOP;
    printf("task step[%x] run complete\n", task_step_id(ts));
    return TASK_RET_NEXT;
}

////////////////////////////////
// t2 is an asynchrounous curl task
////////////////////////////////

static void
t2_notify(struct curl_client_t* cc, void* args) {
    struct task_step_t* ts = (struct task_step_t*)(args);
    if (curl_client_err_code(cc) != CURLE_OK) {
        printf("task step[%x] asynchronous notify, fail\n", task_step_id(ts));
        task_step_resume(ts, TASK_RET_FAIL, 0);
    } else {
        printf("task step[%x] asynchronous notify, complete\n", task_step_id(ts));
        task_step_resume(ts, TASK_RET_NEXT, 1);
    }
}

static int32_t
t2_run(struct task_step_t* ts) {
    int32_t ret = curl_pool_add_get_req(CURL_POOL, "www.baidu.com", t2_notify, ts, NULL);
    assert(ret == 0);
    printf("task step[%x] asynchronous running\n", task_step_id(ts));
    return TASK_RET_RUNNING;
}

//////////////////////////////
// t3 is an asynchronous curl task
//////////////////////////////

static void
t3_notify(struct curl_client_t* cc, void* args) {
    struct task_step_t* ts = (struct task_step_t*)(args);
    struct task_step_t* t1 = NULL;
    t1_param_t* tsp = NULL;

    if (curl_client_err_code(cc) != CURLE_OK) {
        printf("task step[%x] asynchronous notify, fail\n", task_step_id(ts));
        task_step_resume(ts, TASK_RET_FAIL, 0);
    }
    printf("task step[%x] asynchronous notify, complete\n", task_step_id(ts));

    t1 = task_get_step(task_step_task(ts), TASK_STEP_ID_1);
    assert(t1);
    tsp = (t1_param_t*)task_step_param(t1);
    if (tsp->loop -- > 0) {
        printf("need to loop %d times\n", tsp->loop);
        task_step_resume(ts, TASK_RET_NEXT, -1);
    } else {
        task_step_resume(ts, TASK_RET_SUCCESS, 0);
    }
}

static int32_t
t3_run(struct task_step_t* ts) {
    int32_t ret = curl_pool_add_get_req(CURL_POOL, "www.soso.com", t3_notify, ts, NULL);
    assert(ret == 0);
    printf("task step[%x] asynchronous running\n", task_step_id(ts));
    return TASK_RET_RUNNING;
}

/////////////////////////////////
// task callback
//////////////////////////////////

static void
on_success(struct task_t* t) {
    printf("task[%x] success\n", task_id(t));
}

static void
on_fail(struct task_t* t, int timeout) {
    if (timeout == 0) {
        printf("task[%x] timeout fail\n", task_id(t));
    } else {
        printf("task[%x] fail\n", task_id(t));
    }
}

// task: t1-t3 repeat twice
// t1 -> t2 -> t3
//       ^      |
//       |------v

int
test_task() {
    struct timer_t* timer;
    struct task_t* t;
    struct task_step_t* t1, *t2, *t3;
    struct timeval timeout;
    struct timeval tv;

    CURL_POOL = curl_pool_init();
    assert(CURL_POOL);

    timer = timer_create();
    assert(timer);

    t = task_create(on_success, on_fail, NULL);
    assert(t);

    t1 = task_step_create(t1_run, (void*)&param);
    assert(t1);
    task_push_back_step(t, t1);
    TASK_STEP_ID_1 = task_step_id(t1);

    t2 = task_step_create(t2_run, NULL);
    assert(t2);
    task_push_back_step(t, t2);

    t3 = task_step_create(t3_run, NULL);
    assert(t3);
    task_push_back_step(t, t3);

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    task_run(t, timer, &timeout);

    while (1) {
        if (curl_pool_running_count(CURL_POOL) > 0) {
            curl_pool_run(CURL_POOL);
        }
        util_gettimeofday(&tv, NULL);
        timer_poll(timer, &tv);
        if (task_is_finished(t) == 0) {
            break;
        }
    }

    task_release(t);
    timer_release(timer);
    curl_pool_release(CURL_POOL);
    return 0;
}

