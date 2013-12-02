#include <assert.h>
#include <time.h>

#include "core/os_def.h"
#include "core/thread.h"
#include "bus/bus_terminal.h"

THREAD_FUNC thread_input(void* arg)
{
    char input[1024];
    int ret;
    size_t sz = sizeof(input); 
    struct bus_terminal_t* bt = (struct bus_terminal_t*)arg;
    while (1) {
        printf("\n> ");
        fgets(input, sz, stdin);
        ret = bus_terminal_send_all(bt, input, strnlen(input, sz)); 
        if (ret != bus_ok) {
            printf("==>bus send fail:%d\n", ret);
        } else {
            printf("==>bus send:\n%s\n", input);
        }
        SLEEP(1);
    }
	THREAD_RETURN;
}

THREAD_FUNC thread_bus(void* arg)
{
    struct bus_terminal_t* bt = (struct bus_terminal_t*)arg;
    static time_t now;
    static char debug[1024];
    static char recv[1024];
    bus_addr_t from;
    size_t rsize;
    while (1) {
        // bus tickh
        bus_terminal_tick(bt);

        // dump bus
        if (time(NULL) != now) {
            now = time(NULL);
            if (now % 15 == 0) {
                bus_terminal_dump(bt, debug, sizeof(debug));
                printf("==>bus dump:\n%s\n", debug);
            }
        }

        // bus recv
        rsize = sizeof(recv);
        while (bus_terminal_recv_all(bt, recv, &rsize, &from) == 0) {
            recv[rsize] = 0;
            printf("==>bus recv from [%d]:\n%s\n", from, recv);
        }

        SLEEP(1);
    }
	THREAD_RETURN;
}

int main(int argc, char** argv)
{
    int16_t key = 0x1234;
    int addr;
    struct bus_terminal_t* bt;
    thread_t ti, tb;

    if (argc != 2) {
        printf("usage: ./bus_echo bus_addr\n");
        return -1;
    }
    addr = atoi(argv[1]);

    bt = bus_terminal_init(key, addr);
    assert(bt);

    THREAD_CREATE(ti, thread_input, bt);
    THREAD_CREATE(tb, thread_bus, bt);
    assert(ti && tb);

    THREAD_JOIN(ti);
    THREAD_JOIN(tb);

    bus_terminal_release(bt);
    return 0;
}

