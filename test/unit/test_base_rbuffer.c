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
    int ret, loop;
    loop = 0;
    do {
        ret = rbuffer_write(_rbuffer, _bytes, sizeof(_bytes));
        if (ret < 0) {
            usleep(100);
        } else {
            loop ++;
            if (loop % 1000 == 0) {
                printf("\tthread write: %d\n", loop);
            }
        }
    } while(loop < _loop);
    return NULL;
}

static void*
_read(void* arg) {
    int i, ret, loop;
    char recv[BYTES_SIZE];
    size_t nrecv;
    loop = 0;
    do {
        nrecv = BYTES_SIZE;
        ret = rbuffer_read(_rbuffer, recv, &nrecv);
        if (ret < 0) {
            usleep(100);
        } else {
            loop ++;
            if (loop % 1000 == 0) {
                printf("\tthread read: %d\n", loop);
            }
            assert(nrecv == BYTES_SIZE);
            for (i = 0; i < (int)nrecv; i++) {
                assert(recv[i] == _bytes[i]);
            }
        }
    } while(loop < _loop);
    return NULL;
}

int
test_base_rbuffer(char* param) {
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

    pthread_t p1, p2;
    pthread_create(&p1, NULL, _write, NULL);
    pthread_create(&p2, NULL, _read, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    rbuffer_release(_rbuffer);
    return 0;
}

