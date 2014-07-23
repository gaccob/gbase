#include <assert.h>
#include <sys/time.h>

#include "base/heap.h"
#include "util/util_time.h"
#include "timer.h"

typedef struct node_t {
    int timer_id;
    tv_t expire_time;
    tv_t interval_time;
    void* args;
    timer_callback cb_func;
} node_t;

typedef struct timerheap_t {
    struct heap_t* heap;
} timerheap_t;

static int
_timer_cmp(void* data1, void* data2) {
    assert(data1 && data2);
    node_t* node1 = (node_t*)data1;
    node_t* node2 = (node_t*)data2;
    return util_time_compare(&node1->expire_time, &node2->expire_time);
}

timerheap_t*
timer_create_heap() {
    timerheap_t* timer = (timerheap_t*)MALLOC(sizeof(timerheap_t));
    if (!timer)
        return NULL;
    timer->heap = heap_create(_timer_cmp);
    if (!timer->heap) {
        FREE(timer);
        return NULL;
    }
    return timer;
}

void
timer_release(timerheap_t* timer) {
    if (!timer)
        return;
    while (heap_size(timer->heap) > 0) {
        node_t* node = (node_t*)heap_pop(timer->heap);
        assert(node);
        FREE(node);
    }
    heap_release(timer->heap);
    FREE(timer);
}

//  interval==NULL means once
//  return registered timer id
//  if fail, return TIMER_INVALID_ID
int
timer_register(timerheap_t* timer, tv_t* interval, tv_t* delay,
               timer_callback cb, void* args) {
    if (!timer || !delay || !cb)
        return TIMER_INVALID_ID;

    node_t* node = (node_t*)MALLOC(sizeof(*node));
    assert(node);
    node->args = args;
    node->cb_func = cb;
    if (interval) {
        memcpy(&node->interval_time, interval, sizeof(tv_t));
    } else {
        memset(&node->interval_time, 0, sizeof(tv_t));
    }

    tv_t now;
    gettimeofday(&now, NULL);
    util_time_add(&now, delay, &node->expire_time);

    node->timer_id = heap_insert(timer->heap, node);
    if (node->timer_id < 0) {
        FREE(node);
        return TIMER_INVALID_ID;
    }
    return node->timer_id;
}

void
timer_unregister(timerheap_t* timer, int timer_id) {
    if (!timer || timer_id < 0)
        return;
    node_t* node = (node_t*)heap_erase(timer->heap, timer_id);
    if (node) FREE(node);
}

void
timer_poll(timerheap_t* timer, tv_t* now) {
    if (!timer || !now)
        return;
    if (heap_size(timer->heap) <= 0)
        return;
    while (1) {
        node_t* top = (node_t*)heap_top(timer->heap);
        if (!top)
            break;
        tv_t* next_due_time = &top->expire_time;
        // out-of-date
        if (util_time_compare(next_due_time, now) < 0) {
            int ret = top->cb_func(top->args);
            // update timer
            if (ret >= 0 && (top->interval_time.tv_sec > 0
                || top->interval_time.tv_usec > 0)) {
                util_time_add(now, &top->interval_time, &top->expire_time);
                heap_update(timer->heap, top->timer_id, top);
            } else {
                // erase timer
                top = (node_t*)heap_pop(timer->heap);
                FREE(top);
            }
            continue;
        }
        break;
    }
}

