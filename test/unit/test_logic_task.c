#include <assert.h>
#include <sys/time.h>

#include "logic/task.h"
#include "base/timer.h"
#include "net/curlc.h"
#include "net/curlp.h"
#include "util/util_time.h"

static curlp_t* _curl_pool;
static uint32_t _task_step_id;

////////////////////////////////
// t1 is a synchronous task step
////////////////////////////////
typedef struct t1_param_t {
    int loop;
} t1_param_t;

static int
t1_run(task_step_t* ts) {
    printf("task step[%x] run complete\n", task_step_id(ts));
    return TASK_RET_NEXT;
}

////////////////////////////////
// t2 is an asynchrounous curl task
////////////////////////////////

static void
t2_notify(curlc_t* cc, void* args) {
    task_step_t* ts = (task_step_t*)(args);
    if (curlc_ret(cc) != CURLE_OK) {
        printf("task step[%x] asynchronous notify, fail\n", task_step_id(ts));
        task_step_resume(ts, TASK_RET_FAIL, 0);
    } else {
        printf("task step[%x] asynchronous notify, complete\n", task_step_id(ts));
        task_step_resume(ts, TASK_RET_NEXT, 1);
    }
}

static int
t2_run(struct task_step_t* ts) {
    int ret = curlp_add_get(_curl_pool, "www.baidu.com", t2_notify, ts, NULL);
    if (ret < 0) {
        fprintf(stderr, "curl add step fail\n");
        return TASK_RET_FAIL;
    }
    printf("task step[%x] asynchronous running\n", task_step_id(ts));
    return TASK_RET_RUNNING;
}

//////////////////////////////
// t3 is an asynchronous curl task
//////////////////////////////

static void
t3_notify(curlc_t* cc, void* args) {
    task_step_t* ts = (struct task_step_t*)(args);
    if (curlc_ret(cc) != CURLE_OK) {
        printf("task step[%x] asynchronous notify, fail\n", task_step_id(ts));
        task_step_resume(ts, TASK_RET_FAIL, 0);
    }
    printf("task step[%x] asynchronous notify, complete\n", task_step_id(ts));

    task_step_t* t1 = task_get_step(task_step_task(ts), _task_step_id);
    if (!t1) {
        fprintf(stderr, "task get step fail");
        return;
    }

    t1_param_t* tsp = (t1_param_t*)task_step_param(t1);
    if (tsp->loop -- > 0) {
        printf("need to loop %d times\n", tsp->loop);
        task_step_resume(ts, TASK_RET_NEXT, -1);
    } else {
        task_step_resume(ts, TASK_RET_SUCCESS, 0);
    }
}

static int
t3_run(struct task_step_t* ts) {
    int ret = curlp_add_get(_curl_pool, "www.soso.com", t3_notify, ts, NULL);
    if (ret < 0) {
        fprintf(stderr, "curl add step fail\n");
        return TASK_RET_FAIL;
    }
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
test_logic_task(char* param) {
    timerheap_t* timer = timer_create_heap();
    if (!timer) {
        fprintf(stderr, "timer create fail\n");
        return -1;
    }

    _curl_pool = curlp_create();
    if (!_curl_pool) {
        fprintf(stderr, "curl create fail\n");
        timer_release(timer);
        return -1;
    }

    task_t* t = task_create(on_success, on_fail, NULL);
    if (!t) {
        fprintf(stderr, "task create fail\n");
        timer_release(timer);
        curlp_release(_curl_pool);
        return -1;
    }

    struct timeval timeout;
    struct timeval tv;

    t1_param_t p;
    p.loop = (param ? atoi(param) : 3);
    task_step_t* t1 = task_step_create(t1_run, (void*)&p);
    task_step_t* t2 = task_step_create(t2_run, NULL);
    task_step_t* t3 = task_step_create(t3_run, NULL);
    if (!t1 || !t2 || !t3) {
        fprintf(stderr, "task step create fail\n");
        if (t1) task_step_release(t1);
        if (t2) task_step_release(t2);
        if (t3) task_step_release(t3);
        task_release(t);
        timer_release(timer);
        curlp_release(_curl_pool);
        return -1;
    }

    task_push_back_step(t, t1);
    _task_step_id = task_step_id(t1);
    task_push_back_step(t, t2);
    task_push_back_step(t, t3);

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    task_run(t, timer, &timeout);

    while (1) {
        if (curlp_running_count(_curl_pool) > 0) {
            curlp_poll(_curl_pool);
        }
        gettimeofday(&tv, NULL);
        timer_poll(timer, &tv);
        if (task_is_finished(t) == 0) {
            break;
        }
    }

    task_release(t);
    timer_release(timer);
    curlp_release(_curl_pool);
    return 0;
}

