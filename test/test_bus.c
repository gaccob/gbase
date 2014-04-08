#include <assert.h>
#include <time.h>
#include "core/os_def.h"
#include "core/thread.h"
#include "logic/bus.h"

#include "test.h"

THREAD_FUNC
thread_input(void* arg) {
    char input[1024];
    int ret;
    size_t sz = sizeof(input); 
    struct bus_t* bt = (struct bus_t*)arg;
    while (1) {
        printf("\n> ");
        fgets(input, sz, stdin);
        ret = bus_send_all(bt, input, strnlen(input, sz)); 
        if (ret != BUS_OK) {
            printf("==>bus send fail:%d\n", ret);
        } else {
            printf("==>bus send:\n%s\n", input);
        }
        SLEEP(1);
    }
	THREAD_RETURN;
}

THREAD_FUNC
thread_bus(void* arg) {
    struct bus_t* bt = (struct bus_t*)arg;
    static time_t now;
    static char debug[1024];
    static char recv[1024];
    bus_addr_t from;
    size_t rsize;
    while (1) {
        bus_poll(bt);
        if (time(NULL) != now) {
            now = time(NULL);
            if (now % 15 == 0) {
                bus_dump(bt, debug, sizeof(debug));
                printf("==>bus dump:\n%s\n", debug);
            }
        }
        rsize = sizeof(recv);
        while (bus_recv_all(bt, recv, &rsize, &from) == 0) {
            recv[rsize] = 0;
            printf("==>bus recv from [%d]:\n%s\n", from, recv);
        }
        SLEEP(1);
    }
	THREAD_RETURN;
}

int
test_bus(bus_addr_t addr) {
    struct bus_t* bt;
    thread_t ti, tb;
    bt = bus_create(BUS_KEY, addr);
    assert(bt);
    THREAD_CREATE(ti, thread_input, bt);
    THREAD_CREATE(tb, thread_bus, bt);
    assert(ti && tb);
    THREAD_JOIN(ti);
    THREAD_JOIN(tb);
    bus_release(bt);
    return 0;
}

