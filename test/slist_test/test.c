#include "ds/slist.h"
#include <assert.h>

#define LOOP 32

int main()
{
	int data[LOOP];
	int i, res;
	struct slist_t* sl;
	sl = slist_init();
	assert(sl);
	
	for(i=0; i<LOOP; i++)
	{	
		data[i] = rand() % LOOP;
		res = slist_insert(sl, &data[i]);
		assert(0 == res);
	}
	printf("list count=%d\n", slist_count(sl));

	for(i=0; i<LOOP; i++)
	{
		res = slist_find(sl, &data[i]);
		assert(0 == res);
		slist_remove(sl, &data[i]);
	}
	printf("list count=%d\n", slist_count(sl));

	slist_release(sl);
	sl = 0;
	return 0;
}

