#include <assert.h>

#include "core/heap.h"
#include "util/util_time.h"
#include "timer.h"

typedef struct timer_node_t
{
    int timer_id;
    struct timeval expire_time;
    struct timeval interval_time;
    void* args;
    timer_callback cb_func;
} timer_node_t;

typedef struct heaptimer_t
{
    struct heap_t* heap;
} heaptimer_t;

int _timer_cmp(void* data1, void* data2)
{
    timer_node_t* node1, *node2;
    assert(data1 && data2);
    node1 = (timer_node_t*)data1;
    node2 = (timer_node_t*)data2;
    return util_time_compare(&node1->expire_time, &node2->expire_time);
}

heaptimer_t* timer_init()
{
    heaptimer_t* timer = (heaptimer_t*)MALLOC(sizeof(heaptimer_t));
    if (!timer) goto TIMER_FAIL;
    timer->heap = heap_init(_timer_cmp);
    if (!timer->heap) goto TIMER_FAIL1;
    return timer;
TIMER_FAIL1:
    FREE(timer);
TIMER_FAIL:
    return NULL;
}

void timer_release(heaptimer_t* timer)
{
    timer_node_t* node;
    if (!timer) return;
    while (heap_count(timer->heap) > 0) {
        node = (timer_node_t*)heap_pop(timer->heap);
        assert(node);
        FREE(node);
    }
    heap_release(timer->heap);
    FREE(timer);
}

//  interval==NULL means once
//  return registered timer id
//  if fail, return TIMER_INVALID_ID
int timer_register(heaptimer_t* timer, struct timeval* interval,
                   struct timeval* delay, timer_callback cb, void* args)
{
    timer_node_t* node;
    struct timeval now;
    if (!timer || !delay || !cb) return TIMER_INVALID_ID;

    node = (timer_node_t*)MALLOC(sizeof(*node));
    node->args = args;
    node->cb_func = cb;
    if (interval) {
        memcpy(&node->interval_time, interval, sizeof(struct timeval));
    } else {
        memset(&node->interval_time, 0, sizeof(struct timeval));
    }
    util_gettimeofday(&now, NULL);
    util_time_add(&now, delay, &node->expire_time);

    node->timer_id = heap_insert(timer->heap, node);
    if (node->timer_id < 0) {
        FREE(node);
        return TIMER_INVALID_ID;
    }
    return node->timer_id;
}

void timer_unregister(heaptimer_t* timer, int timer_id)
{
    timer_node_t* node;
    if (!timer || timer_id < 0) return;

    node = (timer_node_t*)heap_erase(timer->heap, timer_id);
    if (node) FREE(node);
}

void timer_poll(heaptimer_t* timer, struct timeval* now)
{
    timer_node_t* top;
    struct timeval* next_due_time;
    int ret;
    if (!timer || !now) return;
    if (heap_count(timer->heap) <= 0) return;
    while (1) {
        top = (timer_node_t*)heap_top(timer->heap);
        if (!top) break;
        next_due_time = &top->expire_time;
        // out-of-date
        if (util_time_compare(next_due_time, now) < 0) {
            ret = top->cb_func(top->args);
            // update timer
            if (ret >= 0 && (top->interval_time.tv_sec > 0
                || top->interval_time.tv_usec > 0)) {
                util_time_add(now, &top->interval_time, &top->expire_time);
                heap_update(timer->heap, top->timer_id, top);
            } else {
                // erase timer
                top = (timer_node_t*)heap_pop(timer->heap);
                FREE(top);
            }
            continue;
        }
        break;
    }
}

