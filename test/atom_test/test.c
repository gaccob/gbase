#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "core/os_def.h"
#include "core/thread.h"
#include "core/atom.h"
#include "core/util.h"

atom_t flag;
uint32_t cmp;
#define LOOP 1000000
THREAD_FUNC func(void* arg)
{
	int index;
	for(index = 0; index < LOOP; index ++)
	{
		//atom_inc(&flag);
		atom_add(&flag, 2);
		cmp += 2;
	}
	THREAD_RETURN;
}

int main()
{
	flag = 0; 
	cmp = 0;
	thread_t t1, t2;
	THREAD_CREATE(t1, func, NULL);
	THREAD_CREATE(t2, func, NULL);

	THREAD_JOIN(t1);
	THREAD_JOIN(t2);
	
	printf("\nflag=%u cmp=%u...", flag, cmp);
	getchar();
	return 0;
}

