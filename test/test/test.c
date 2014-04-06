#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "test.h"

int
get_process_time(struct timeval* from) {
    struct timeval tv;
    util_gettimeofday(&tv,NULL);
    return ((tv.tv_sec - from->tv_sec)*1000+(tv.tv_usec - from->tv_usec)/1000);
}

int
main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: ./test <base64>\n"
            "\t<wscode>\n"
            "\t<random>\n"
            "\t<shuffle>\n"
            "\t<unicode>\n"
            "\t<conhash>\n"
#ifdef OS_LINUX
            "\t<coroutine>\n"
#endif
            "\t<fsm>\n"
            "\t<bitset>\n"
            "\t<heap>\n"
            "\t<rbtree>\n"
            "\t<rbuffer>\n"
            "\t<slist>\n"
            "\t<timer>\n"
            "\t<atom>\n"
            "\t<spin>\n"
            "\t<lock>\n"
            "\t<task>\n"
            "\t<shm> <send | recv>\n"
            "\t<slab>\n"
            "\t<dirty>\n"
            "\t<thread>\n"
            "\t<json> <text | file | create>\n"
            "\t<dh> [perf]\n"
            "\t<echo> <client | server>\n");
        return 0;
    }

    if (0 == strcmp(argv[1], "base64")) {
        test_base64();
    } else if (0 == strcmp(argv[1], "wscode")) {
        test_wscode();
    } else if (0 == strcmp(argv[1], "random")) {
        test_random();
    } else if (0 == strcmp(argv[1], "shuffle")) {
        test_shuffle();
    } else if (0 == strcmp(argv[1], "unicode")) {
        test_unicode();
    } else if (0 == strcmp(argv[1], "conhash")) {
        test_conhash();
    }
#ifdef OS_LINUX
    else if (0 == strcmp(argv[1], "coroutine")) {
        test_coroutine();
    }
#endif
    else if (0 == strcmp(argv[1], "fsm")) {
        test_fsm();
    } else if (0 == strcmp(argv[1], "bitset")) {
        test_bitset();
    } else if (0 == strcmp(argv[1], "heap")) {
        test_heap();
    } else if (0 == strcmp(argv[1], "rbtree")) {
        test_rbtree();
    } else if (0 == strcmp(argv[1], "rbuffer")) {
        test_rbuffer();
    } else if (0 == strcmp(argv[1], "slist")) {
        test_slist();
    } else if (0 == strcmp(argv[1], "timer")) {
        test_timer();
    } else if (0 == strcmp(argv[1], "atom")) {
        test_atom();
    } else if (0 == strcmp(argv[1], "spin")) {
        test_spin();
    } else if (0 == strcmp(argv[1], "lock")) {
        test_lock();
    } else if (0 == strcmp(argv[1], "task")) {
        test_task();
    } else if (0 == strcmp(argv[1], "shm")) {
        if (argc >= 3) {
            if (0 == strcmp(argv[2], "send")) {
                test_shm_send();
            } else if (0 == strcmp(argv[2], "recv")) {
                test_shm_recv();
            }
        }
    } else if (0 == strcmp(argv[1], "slab")) {
        test_slab();
    } else if (0 == strcmp(argv[1], "dirty")) {
        test_dirty();
    } else if (0 == strcmp(argv[1], "thread")) {
        test_thread();
    } else if (0 == strcmp(argv[1], "json")) {
        if (argc >= 3) {
            if (0 == strcmp(argv[2], "text")) {
                test_json_text();
            } else if (0 == strcmp(argv[2], "file")) {
                test_json_file();
            } else if (0 == strcmp(argv[2], "create")) {
                test_json_create();
            }
        }
    } else if (0 == strcmp(argv[1], "dh")) {
        if (argc >= 3 && 0 == strcmp(argv[2], "perf")) {
            test_dh_perf();
        } else {
            test_dh();
        }
    } else if (0 == strcmp(argv[1], "echo")) {
        if (argc >= 3) {
            if (0 == strcmp(argv[2], "client")) {
                test_echo_cli();
            } else if (0 == strcmp(argv[2], "server")) {
                test_echo_svr();
            }
        }
    }
    return 0;
}

