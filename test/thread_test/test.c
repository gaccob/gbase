#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "core/os_def.h"
#include "core/thread.h"

int val = 0;

void* lock;
void* cond;

THREAD_FUNC thread_zero_run(void *arg)
{
	while(1)
	{
		thread_lock(lock);
		while(val <= 2)
		{ 
			fprintf(stderr, "thread_zero_run --> val:%d, wait for wake up\n", val);
			thread_cond_wait(cond, lock, NULL);
		}
		fprintf(stderr, "therad_zero_run --> val:%d, zero it and unlock\n", val);
		val = 0;
		thread_unlock(lock);
	}
	THREAD_RETURN;
 }

 THREAD_FUNC thread_add_run(void *arg)
 {
	while(1)
	{
		thread_lock(lock);
		++val;
		thread_unlock(lock);
		thread_cond_signal(cond, 0);
		
		fprintf(stderr, "after add val:%d and wake up one zero thread for check\n", val);
		SLEEP(1000);
	}
	THREAD_RETURN;
}

int main()
{
	thread_t t_add, t_zero;
	cond = thread_cond_alloc();
	lock = thread_lock_alloc();
	
	THREAD_CREATE(t_add, thread_add_run, NULL);
	THREAD_CREATE(t_zero, thread_zero_run, NULL);
	assert(t_add && t_zero);
	
	THREAD_JOIN(t_add);
	THREAD_JOIN(t_zero);

	thread_cond_free(cond);
	thread_lock_free(lock);
	return 0;
}


