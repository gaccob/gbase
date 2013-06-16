#include "ds/rbuffer.h"
#include "os/thread.h"
#include <assert.h>

#define BYTES_SIZE 10240
#define LOOP 1024000

struct rbuffer_t* buf;
static char bytes[BYTES_SIZE];

THREAD_FUNC f1(void* arg)
{
	int ret, loop;
	loop = 0;
	do{
		ret = rbuffer_write(buf, bytes, sizeof(bytes));
		if(ret < 0)
		{
			SLEEP(1);
		}
		else
		{
			loop ++;
			printf("write: %d\n", loop);
		}
	}while(loop < LOOP);
	THREAD_RETURN;
}

THREAD_FUNC f2(void* arg)
{
	int i, ret, loop;
	char recv[BYTES_SIZE];
	size_t nrecv;
	loop = 0;
	do{
		nrecv = BYTES_SIZE;
		ret = rbuffer_read(buf, recv, &nrecv);
		if(ret < 0)
		{
			SLEEP(1);
		}
		else
		{
			loop ++;
			printf("read: %d\n", loop);
			assert(nrecv == BYTES_SIZE);
			for(i = 0; i < nrecv; i++)
				assert(recv[i] == bytes[i]);
		}
	}while(loop < LOOP);
	THREAD_RETURN;
}

int main()
{
	thread_t p1, p2;
	int i;
	for(i = 0; i < BYTES_SIZE; i++)
		bytes[i] = i % 26 + 'a';
		
	buf = rbuffer_init(1024 * 1024);
	assert(buf);
	
	THREAD_CREATE(p1, f1, NULL);
	THREAD_CREATE(p2, f2, NULL);

	THREAD_JOIN(p1);
	THREAD_JOIN(p2);

	rbuffer_release(buf);
	getchar();
	return 0;
}

