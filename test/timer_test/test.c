#include "ds/timer.h"
#include "core/util.h"

#define LOOP 3

static int flag = 0;

int timer_cb(void* args)
{
    printf("timer[%d] callback\n", *(int*)args);
    return 0;
}

int timer_cb2(void* args)
{
    printf("timer close\n");
    flag = 1;
    return 0;
}

int main()
{
    int* ids;
    int i, ret;
    struct timeval delay;
    struct timeval now;
    struct timeval interval;
    struct heaptimer_t* timer = timer_init();
    ids = (int*)MALLOC(sizeof(int) * LOOP);
    interval.tv_sec = 3;
    interval.tv_usec = 0;
    for(i=0; i<LOOP; i++)
    {
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

    while(0 == flag)
    {
        util_gettimeofday(&now, NULL);
        timer_poll(timer, &now);
        SLEEP(10);
    }

    timer_release(timer);
    FREE(ids);
    return 0;
}

