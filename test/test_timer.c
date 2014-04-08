#include "base/timer.h"
#include "util/util_time.h"

#define TIMER_LOOP 3
static int flag = 0;

int
timer_cb(void* args) {
    printf("timer[%d] callback\n", *(int*)args);
    return 0;
}

int
timer_cb2(void* args) {
    printf("timer close\n");
    flag = 1;
    return 0;
}

int
test_timer() {
    int* ids;
    int i, ret;
    struct timeval delay;
    struct timeval now;
    struct timeval interval;
    struct timerheap_t* timer = timer_create_heap();
    ids = (int*)MALLOC(sizeof(int) * TIMER_LOOP);
    interval.tv_sec = 3;
    interval.tv_usec = 0;
    for (i=0; i<TIMER_LOOP; i++) {
        ids[i] = i;
        delay.tv_sec = rand() % 16;
        delay.tv_usec = 0;
        ret = timer_register(timer, &interval, &delay, timer_cb, &ids[i]);
        printf("register timer[%d] delay=%d\n", ret, (int)delay.tv_sec);
    }

    delay.tv_sec = 180;
    delay.tv_usec = 0;
    ret = timer_register(timer, NULL, &delay, timer_cb2, NULL);
    printf("register timer[%d] delay=%d\n", ret, (int)delay.tv_sec);

    while (0 == flag) {
        util_gettimeofday(&now, NULL);
        timer_poll(timer, &now);
        SLEEP(10);
    }

    timer_release(timer);
    FREE(ids);
    return 0;
}

