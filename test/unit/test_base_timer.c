#include <unistd.h>
#include <sys/time.h>

#include "base/timer.h"
#include "util/util_time.h"

static int _live = 0;
static int _duration = 30;

int
timer_cb(void* args) {
    printf("-->timer[%d] callback\n", *(int*)args);
    return 0;
}

int
timer_cb2(void* args) {
    printf("-->timer close\n");
    _live = 1;
    return 0;
}

int
test_base_timer(const char* param) {
    timerheap_t* timer = timer_create_heap();
    if (!timer) {
        fprintf(stderr, "timer create fail\n");
        return -1;
    }

    int loop = param ? atoi(param) : 3;
    int ids[loop];
    for (int i = 0; i < loop; ++ i) {
        ids[i] = i;
    }

    struct timeval interval;
    interval.tv_sec = 3;
    interval.tv_usec = 0;
    for (int i = 0; i < loop; ++ i) {
        struct timeval delay;
        delay.tv_sec = rand() % 10;
        delay.tv_usec = 0;
        int ret = timer_register(timer, &interval, &delay, timer_cb, &ids[i]);
        if (ret < 0) {
            fprintf(stderr, "timer register fail: %d\n", ret);
            timer_release(timer);
            return -1;
        }
        printf("register timer[%d] delay=%d\n", ret, (int)delay.tv_sec);
    }

    struct timeval delay;
    delay.tv_sec = _duration;
    delay.tv_usec = 0;
    int ret = timer_register(timer, NULL, &delay, timer_cb2, NULL);
    if (ret < 0) {
        fprintf(stderr, "timer register fail: %d\n", ret);
        timer_release(timer);
        return -1;
    }
    printf("register timer[%d] delay=%d\n", ret, (int)delay.tv_sec);

    while (0 == _live) {
        struct timeval now;
        gettimeofday(&now, NULL);
        timer_poll(timer, &now);
        usleep(100);
    }

    timer_release(timer);
    return 0;
}

