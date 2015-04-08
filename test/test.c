#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include "test.h"
#include "util/cmd.h"

#define COLOR_RED   31
#define COLOR_GREEN 32

extern int test_base_conhash(char*);
extern int test_base_bitset(char*);
extern int test_base_heap(char*);
extern int test_base_rbtree(char*);
extern int test_base_rbuffer(char*);
extern int test_base_slist(char*);
extern int test_base_skiplist(char*);
extern int test_base_skiplist_duplicate(char*);
extern int test_base_skiplist_find(char*);
extern int test_base_timer(char*);

extern int test_core_atomic(char*);
#ifdef OS_LINUX
extern int test_core_coroutine(char*);
#endif
extern int test_core_fsm(char*);
extern int test_core_lock(char*);
extern int test_core_spin(char*);
extern int test_core_thread(char*);

extern int test_logic_bevtree(char*);
extern int test_logic_dirty(char*);
extern int test_logic_task(char*);

extern int test_mm_slab(char*);
extern int test_mm_shm(char*);

extern int test_net_curl(char*);
extern int test_net_echo(char*);

extern int test_util_base64(char*);
extern int test_util_cjson_text(char*);
extern int test_util_cjson_file(char*);
extern int test_util_cjson_create(char*);
extern int test_util_dh(char*);
extern int test_util_dh_perf(char*);
extern int test_util_random(char*);
extern int test_util_shuffle(char*);
extern int test_util_unicode(char*);
extern int test_util_wscode(char*);

int
main(int argc, char** argv) {
    cmd_t* cmd = cmd_create(".history", "~>");
    cmd_register(cmd, "base conhash",               test_base_conhash);
    cmd_register(cmd, "base bitset",                test_base_bitset);
    cmd_register(cmd, "base heap",                  test_base_heap);
    cmd_register(cmd, "base rbtree",                test_base_rbtree);
    cmd_register(cmd, "base rbuffer",               test_base_rbuffer);
    cmd_register(cmd, "base slist",                 test_base_slist);
    cmd_register(cmd, "base skiplist",              test_base_skiplist);
    cmd_register(cmd, "base skiplist find",         test_base_skiplist_find);
    cmd_register(cmd, "base skiplist duplicate",    test_base_skiplist_duplicate);
    cmd_register(cmd, "base timer",                 test_base_timer);
    cmd_register(cmd, "core atomic",                test_core_atomic);
#ifdef OS_LINUX
    cmd_register(cmd, "core coroutine",             test_core_coroutine);
#endif
    cmd_register(cmd, "core fsm",                   test_core_fsm);
    cmd_register(cmd, "core lock",                  test_core_lock);
    cmd_register(cmd, "core spin",                  test_core_spin);
    cmd_register(cmd, "core thread",                test_core_thread);
    cmd_register(cmd, "logic bevtree",              test_logic_bevtree);
    cmd_register(cmd, "logic dirty",                test_logic_dirty);
    cmd_register(cmd, "logic task",                 test_logic_task);
    cmd_register(cmd, "mm slab",                    test_mm_slab);
    cmd_register(cmd, "mm shm",                     test_mm_shm);
    cmd_register(cmd, "net curl",                   test_net_curl);
    cmd_register(cmd, "net echo",                   test_net_echo);
    cmd_register(cmd, "util base64",                test_util_base64);
    cmd_register(cmd, "util cjson text",            test_util_cjson_text);
    cmd_register(cmd, "util cjson file",            test_util_cjson_file);
    cmd_register(cmd, "util cjson create",          test_util_cjson_create);
    cmd_register(cmd, "util dh",                    test_util_dh);
    cmd_register(cmd, "util dh perf",               test_util_dh_perf);
    cmd_register(cmd, "util random",                test_util_random);
    cmd_register(cmd, "util shuffle",               test_util_shuffle);
    cmd_register(cmd, "util unicode",               test_util_unicode);
    cmd_register(cmd, "util wscode",                test_util_wscode);

    while (1) {
        char* line = cmd_readline(cmd);
        if (!line) {
            break;
        }

        int ret = cmd_handle(cmd, line);
        if (ret == 0) {
            printf("\033[%dm[SUCCESS]\033[0m\n", COLOR_GREEN);
        } else {
            printf("\033[%dm[FAIL]\033[0m\n", COLOR_RED);
        }
        fflush(stdout);

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
            "\t<wssvr>\n"
            "\t<bus> <addr>\n");
        return 0;
    }
    if (0 == strcmp(argv[1], "wssvr")) {
        test_ws_server();
    } else if (0 == strcmp(argv[1], "bus")) {
        if (argc >= 3) {
            test_bus(atoi(argv[2]));
        }
    }
    return 0;
}

