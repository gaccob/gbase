#include <assert.h>
#include <unistd.h>
#include <time.h>
#include "core/os_def.h"
#include "core/thread.h"

#include "test.h"

void*
thread_input(void* arg) {
    while (1) {
        printf("\n> ");
        char input[1024];
        size_t sz = sizeof(input);
        fgets(input, sz, stdin);

        bus_t* bt = (bus_t*)arg;
        int ret = bus_send_all(bt, input, strnlen(input, sz));
        if (ret != BUS_OK) {
            printf("==>bus send fail:%d\n", ret);
        } else {
            printf("==>bus send:\n%s\n", input);
        }
        usleep(100);
    }
    return NULL;
}

void*
thread_bus(void* arg) {
    bus_t* bt = (bus_t*)arg;
    static time_t now;
    static char debug[1024];
    static char recv[1024];
    while (1) {
        bus_poll(bt);
        if (time(NULL) != now) {
            now = time(NULL);
            if (now % 15 == 0) {
                bus_dump(bt, debug, sizeof(debug));
                printf("==>bus dump:\n%s\n", debug);
            }
        }
        size_t rsize = sizeof(recv);
        bus_addr_t from;
        while (bus_recv_all(bt, recv, &rsize, &from) == 0) {
            recv[rsize] = 0;
            printf("==>bus recv from [%d]:\n%s\n", from, recv);
        }
        usleep(100);
    }
    return NULL;
}

int
test_bus(bus_addr_t addr) {
    bus_t* bt;
    pthread_t ti, tb;
    bt = bus_create(BUS_KEY, addr);
    assert(bt);
    pthread_create(&ti, NULL, thread_input, bt);
    pthread_create(&tb, NULL, thread_bus, bt);
    assert(ti && tb);
    pthread_join(ti, NULL);
    pthread_join(tb, NULL);
    bus_release(bt);
    return 0;
}

