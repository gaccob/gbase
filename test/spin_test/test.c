#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "core/os_def.h"
#include "core/thread.h"
#include "core/spin.h"
#include "util/util_time.h"
/*
int val = 0;

struct spin_lock_t* lock;

THREAD_FUNC thread_zero_run(void *arg)
{
    while(1)
    {
        spin_lock(lock);
        if(val > 102400)
        {
            val = 0;
            fprintf(stderr, "therad_zero_run --> val:%d, zero it and unlock\n", val);
        }
        spin_unlock(lock);
    }
    THREAD_RETURN;
}

THREAD_FUNC thread_add_run(void *arg)
{
    while(1)
    {
        spin_lock(lock);
        ++val;
        spin_unlock(lock);
        // fprintf(stderr, "thread_add_run: after add val:%d\n", val);
    }
    THREAD_RETURN;
}

int main()
{
    thread_t t_add, t_zero;

    lock = spin_init();
    assert(lock);

    THREAD_CREATE(t_add, thread_add_run, NULL);
    THREAD_CREATE(t_zero, thread_zero_run, NULL);
    assert(t_add && t_zero);

    THREAD_JOIN(t_add);
    THREAD_JOIN(t_zero);

    spin_release(lock);
    return 0;
}

*/

struct spin_lock_t* spinlock;
void* threadlock;

int thread_count = 4;
int loop_count = 1000000;
int code_len = 1;

int tick = 0;

THREAD_FUNC func1(void* arg)
{
    int i, j, l, k, m;
    k = rand();
    for(i=0; i<loop_count; i++)
    {
        spin_lock(spinlock);
        for(l=0; l<code_len; l++)
        {
            j = k;
            m = j;
            k = m;
            m = j+1;
            k = m+2;
            j = m+k;
        }
        //tick ++;
        spin_unlock(spinlock);
    }
    THREAD_RETURN;
}

THREAD_FUNC func2(void* arg)
{
    int i, j, l, k, m;
    k = rand();
    for(i=0; i<loop_count; i++)
    {
        thread_lock(threadlock);
        for(l=0; l<code_len; l++)
        {
            j = k;
            m = j;
            k = m;
            m = j+1;
            k = m+2;
            j = m+k;
        }
        //tick --;
        thread_unlock(threadlock);
    }
    THREAD_RETURN;
}

int get_process_time(struct timeval* from)
{
    struct timeval tv;
    util_gettimeofday(&tv,NULL);
    return ((tv.tv_sec - from->tv_sec)*1000+(tv.tv_usec - from->tv_usec)/1000);
}

void test_spin()
{
    thread_t* tid;
    int i, ret, lasttime;
    struct timeval from;
    tid = (thread_t*)MALLOC(sizeof(thread_t) * thread_count);
    util_gettimeofday(&from,NULL);
    for(i=0; i<thread_count; i++)
    {
        THREAD_CREATE(tid[i], func1, NULL);
    }

    for(i=0; i<thread_count; i++)
    {
        ret = THREAD_JOIN(tid[i]);
        if(ret !=0)
        {
            printf("cannot join thread1");
        }
    }

    lasttime = get_process_time(&from);
    printf("spin lock\t\tloop_count:%d\tcode_len:%d\ttime:%d\n",loop_count, code_len, lasttime);
    FREE(tid);
}

void test_lock()
{
    thread_t* tid;
    int i, ret, lasttime;
    struct timeval from;
    tid = (thread_t*)MALLOC(sizeof(thread_t) * thread_count);
    util_gettimeofday(&from,NULL);
    for(i=0; i<thread_count; i++)
    {
        THREAD_CREATE(tid[i], func2, NULL);
    }

    for(i=0; i<thread_count; i++)
    {
        ret = THREAD_JOIN(tid[i]);
        if(ret !=0)
        {
            printf("cannot join thread1");
        }
    }

    lasttime = get_process_time(&from);
    printf("thread lock\t\tloop_count:%d\tcode_len:%d\ttime:%d\n",loop_count, code_len, lasttime);
    FREE(tid);
}

int main()
{
    spinlock = spin_init();
    assert(spinlock);
    threadlock = thread_lock_alloc();
    assert(threadlock);

    test_spin();
    test_lock();

    spin_release(spinlock);
    thread_lock_free(threadlock);

    printf("\npress any key to continue...");
    getchar();
    return 0;
}
