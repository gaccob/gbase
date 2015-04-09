#include <assert.h>
#include <unistd.h>
#include "base/rbuffer.h"
#include "core/thread.h"

#define BYTES_SIZE 10240

static rbuffer_t* _rbuffer;
static int _loop = 10000;
static char _bytes[BYTES_SIZE];

static void*
_write(void* arg) {
    int loop = 0;
    do {
        int ret = rbuffer_write(_rbuffer, _bytes, sizeof(_bytes));
        if (ret < 0) {
            usleep(100);
        } else {
            loop ++;
            if (loop % 1000 == 0) {
                printf("thread write: %d\n", loop);
            }
        }
    } while(loop < _loop);
    return NULL;
}

static void*
_read(void* arg) {
    char recv[BYTES_SIZE];
    int loop = 0;
    do {
        size_t nrecv = BYTES_SIZE;
        int ret = rbuffer_read(_rbuffer, recv, &nrecv);
        if (ret < 0) {
            usleep(100);
        } else {
            loop ++;
            if (loop % 1000 == 0) {
                printf("thread read: %d\n", loop);
            }
            assert(nrecv == BYTES_SIZE);
            for (int i = 0; i < (int)nrecv; i++) {
                assert(recv[i] == _bytes[i]);
            }
        }
    } while(loop < _loop);
    return NULL;
}

int
test_base_rbuffer(const char* param) {
    if (param) {
        _loop = atoi(param);
    }

    for (int i = 0; i < BYTES_SIZE; i++) {
        _bytes[i] = i % 26 + 'a';
    }

    _rbuffer = rbuffer_create(1024 * 1024);
    if (!_rbuffer) {
        fprintf(stderr, "rbuffer create fail\n");
        return -1;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (1 << 20));

    pthread_t p1, p2;
    pthread_create(&p1, &attr, _write, NULL);
    pthread_create(&p2, &attr, _read, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    rbuffer_release(_rbuffer);
    return 0;
}

