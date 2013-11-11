#include <assert.h>
#include "core/os_def.h"
#include "core/task.h"
#include "core/timer.h"
#include "net/curl_client.h"
#include "net/curl_pool.h"
#include "util/util_time.h"

struct curl_pool_t* cp;
uint32_t id;

////////////////////////////////
// t1 is a synchronous task step
////////////////////////////////
typedef struct t1_data_t
{
    int32_t result;
} t1_data_t;

void t1_data_release(void* data)
{
    FREE((t1_data_t*)data);
}

int32_t t1_run(struct task_step_t* ts)
{
	struct t1_data_t* td;
	void* arg;

    td = (t1_data_t*)MALLOC(sizeof(*td));
    task_step_set_data(ts, td, t1_data_release);
	arg = task_data(task_step_task(ts));
    assert(arg);
    td->result = *(int32_t*)arg;
    printf("task step[%x] run complete\n", task_step_id(ts));
    return TASK_RET_NEXT;
}

////////////////////////////////
// t2 is an asynchrounous curl task
////////////////////////////////

void t2_notify(struct curl_client_t* cc, void* args)
{
    struct task_step_t* ts = (struct task_step_t*)(args);
	struct task_step_result_t* rt = NULL;
    if (curl_client_err_code(cc) != CURLE_OK) {
        printf("task step[%x] asynchronous notify, fail\n", task_step_id(ts));
        rt = task_step_result_init(TASK_RET_FAIL);
        task_step_finish(ts, rt);
        task_step_result_release(rt);
    } else {
        printf("task step[%x] asynchronous notify, complete\n", task_step_id(ts));
        rt = task_step_result_init(TASK_RET_NEXT);
        task_step_finish(ts, rt);
        task_step_result_release(rt);
    }
}

int32_t t2_run(struct task_step_t* ts)
{
    int32_t ret = curl_pool_add_get_req(cp, "www.baidu.com", t2_notify, ts, NULL);
    assert(ret == 0);
    printf("task step[%x] asynchronous running\n", task_step_id(ts));
    return TASK_RET_RUNNING;
}

//////////////////////////////
// t3 is an asynchronous curl task
//////////////////////////////

void t3_notify(struct curl_client_t* cc, void* args)
{
    struct task_step_t* ts = (struct task_step_t*)(args);
	struct task_step_result_t* rt = NULL;
	struct task_step_t* t1 = NULL;
	t1_data_t* td = NULL;

    if (curl_client_err_code(cc) != CURLE_OK) {
        printf("task step[%x] asynchronous notify, fail\n", task_step_id(ts));
        rt = task_step_result_init(TASK_RET_FAIL);
        task_step_finish(ts, rt);
        task_step_result_release(rt);
    }
    printf("task step[%x] asynchronous notify, complete\n", task_step_id(ts));

    t1 = task_get_step(task_step_task(ts), id);
    assert(t1);
    td = (t1_data_t*)task_step_data(t1);
    td->result --;
    if (td->result > 0) {
        printf("need to loop %d times\n", td->result);
        rt = task_step_result_init(TASK_RET_NEXT);
        task_step_result_set_backward(rt);
        task_step_finish(ts, rt);
        task_step_result_release(rt);
    } else {
        rt = task_step_result_init(TASK_RET_SUCCESS);
        task_step_finish(ts, rt);
        task_step_result_release(rt);
    }
}

int32_t t3_run(struct task_step_t* ts)
{
    int32_t ret = curl_pool_add_get_req(cp, "www.soso.com", t3_notify, ts, NULL);
    assert(ret == 0);
    printf("task step[%x] asynchronous running\n", task_step_id(ts));
    return TASK_RET_RUNNING;
}

/////////////////////////////////
// task callback
//////////////////////////////////

void on_success(struct task_t* t)
{
    printf("task[%x] success\n", task_id(t));
}

void on_fail(struct task_t* t, int timeout)
{
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

int main(int argc, char** argv)
{
	struct heaptimer_t* timer;
	struct task_t* t;
	struct task_step_t* t1, *t2, *t3;
	struct timeval timeout;
	struct timeval tv;

    int32_t loop = argc > 1 ? atoi(argv[1]) : 3;

    cp = curl_pool_init();
    assert(cp);

    timer = timer_init();
    assert(timer);

    t = task_init(on_success, on_fail, (void*)&loop);
    assert(t);

    t1 = task_step_init(t1_run);
    assert(t1);
    task_push_back_step(t, t1);
    id = task_step_id(t1);

    t2 = task_step_init(t2_run);
    assert(t2);
    task_push_back_step(t, t2);

    t3 = task_step_init(t3_run);
    assert(t3);
    task_push_back_step(t, t3);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    task_run(t, timer, &timeout);

    while (1) {
        if (curl_pool_running_count(cp) > 0) {
            curl_pool_run(cp);
        }

        util_gettimeofday(&tv, NULL);
        timer_poll(timer, &tv);

        if (task_finished(t) == 0) {
            break;
        }
    }

    task_release(t);
    timer_release(timer);
    curl_pool_release(cp);
    return 0;
}

