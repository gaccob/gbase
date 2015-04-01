#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include "test.h"
#include "util/cmd.h"

int
get_process_time(struct timeval* from) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return ((tv.tv_sec - from->tv_sec)*1000+(tv.tv_usec - from->tv_usec)/1000);
}

extern int test_base_conhash(char*);
extern int test_base_bitset(char*);
extern int test_base_heap(char*);
extern int test_base_rbtree(char*);
extern int test_base_rbuffer(char*);
extern int test_base_skiplist(char*);
extern int test_base_skiplist_duplicate(char*);
extern int test_base_skiplist_find(char*);

extern int test_core_fsm(char*);
extern int test_core_atomic(char*);
extern int test_core_lock(char*);

extern int test_logic_dirty(char*);

extern int test_mm_slab(char*);

extern int test_net_curl(char*);

extern int test_util_base64(char*);
extern int test_util_wscode(char*);
extern int test_util_random(char*);
extern int test_util_shuffle(char*);
extern int test_util_unicode(char*);
extern int test_util_cjson_text(char*);
extern int test_util_cjson_file(char*);
extern int test_util_cjson_create(char*);

int
main(int argc, char** argv) {
    cmd_t* cmd = cmd_create(".history", "~>");
    cmd_register(cmd, "base conhash",               test_base_conhash);
    cmd_register(cmd, "base bitset",                test_base_bitset);
    cmd_register(cmd, "base heap",                  test_base_heap);
    cmd_register(cmd, "base rbtree",                test_base_rbtree);
    cmd_register(cmd, "base rbuffer",               test_base_rbuffer);
    cmd_register(cmd, "base skiplist",              test_base_skiplist);
    cmd_register(cmd, "base skiplist find",         test_base_skiplist_find);
    cmd_register(cmd, "base skiplist duplicate",    test_base_skiplist_duplicate);
    cmd_register(cmd, "core fsm",                   test_core_fsm);
    cmd_register(cmd, "core atomic",                test_core_atomic);
    cmd_register(cmd, "core lock",                  test_core_lock);
    cmd_register(cmd, "logic dirty",                test_logic_dirty);
    cmd_register(cmd, "mm slab",                    test_mm_slab);
    cmd_register(cmd, "net curl",                   test_net_curl);
    cmd_register(cmd, "util base64",                test_util_base64);
    cmd_register(cmd, "util cjson text",            test_util_cjson_text);
    cmd_register(cmd, "util cjson file",            test_util_cjson_file);
    cmd_register(cmd, "util cjson create",          test_util_cjson_create);
    cmd_register(cmd, "util wscode",                test_util_wscode);
    cmd_register(cmd, "util random",                test_util_random);
    cmd_register(cmd, "util shuffle",               test_util_shuffle);
    cmd_register(cmd, "util unicode",               test_util_unicode);

    while (1) {
        char* line = cmd_readline(cmd);
        if (!line) {
            break;
        }

        int ret = cmd_handle(cmd, line);
        if (ret == 0) {
            printf("[SUCCESS]   %s\n", line);
        } else {
            fprintf(stderr, "[FAIL] %s\n", line);
        }

        free(line);
        if (cmd_eof(cmd)) {
            if (cmd_closed(cmd)) {
                break;
            }
        }
    }
    cmd_release(cmd);
    return 0;

    if (argc < 2) {
        printf("usage: ./test\n"
#ifdef OS_LINUX
            "\t<coroutine>\n"
#endif
            "\t<slist>\n"
            "\t<timer>\n"
            "\t<spin>\n"
            "\t<task>\n"
            "\t<shm> <send | recv>\n"
            "\t<thread>\n"
            "\t<dh> [perf]\n"
            "\t<echo> <client | server>\n"
            "\t<wssvr>\n"
            "\t<bus> <addr>\n"
            "\t<bevtree>\n");
        return 0;
    }
    if (0 == strcmp(argv[1], "echo")) {
        if (argc >= 3) {
            if (0 == strcmp(argv[2], "client")) {
                test_echo_cli();
            } else if (0 == strcmp(argv[2], "server")) {
                test_echo_svr();
            }
        }
    }
#ifdef OS_LINUX
    else if (0 == strcmp(argv[1], "coroutine")) {
        test_coroutine();
    }
#endif
    else if (0 == strcmp(argv[1], "slist")) {
        test_slist();
    } else if (0 == strcmp(argv[1], "timer")) {
        test_timer();
    } else if (0 == strcmp(argv[1], "spin")) {
        test_spin();
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
    } else if (0 == strcmp(argv[1], "thread")) {
        test_thread();
    } else if (0 == strcmp(argv[1], "dh")) {
        if (argc >= 3 && 0 == strcmp(argv[2], "perf")) {
            test_dh_perf();
        } else {
            test_dh();
        }
    } else if (0 == strcmp(argv[1], "wssvr")) {
        test_ws_server();
    } else if (0 == strcmp(argv[1], "bus")) {
        if (argc >= 3) {
            test_bus(atoi(argv[2]));
        }
    } else if (0 == strcmp(argv[1], "bevtree")) {
        test_bevtree();
    }
    return 0;
}

