#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <sys/time.h>

#include "test.h"
#include "util/cmd.h"

#define COLOR_RED   31
#define COLOR_GREEN 32

extern int test_base_conhash(const char*);
extern int test_base_bitset(const char*);
extern int test_base_heap(const char*);
extern int test_base_rbtree(const char*);
extern int test_base_rbuffer(const char*);
extern int test_base_slist(const char*);
extern int test_base_skiplist(const char*);
extern int test_base_skiplist_duplicate(const char*);
extern int test_base_skiplist_find(const char*);
extern int test_base_timer(const char*);

extern int test_core_atomic(const char*);
#ifdef OS_LINUX
extern int test_core_coroutine(const char*);
#endif
extern int test_core_fsm(const char*);
extern int test_core_lock(const char*);
extern int test_core_spin(const char*);
extern int test_core_thread(const char*);

extern int test_logic_dirty(const char*);
extern int test_logic_task(const char*);

extern int test_mm_slab(const char*);
extern int test_mm_shm(const char*);

extern int test_net_curl(const char*);
extern int test_net_echo(const char*);

extern int test_util_base64(const char*);
extern int test_util_cjson_text(const char*);
extern int test_util_cjson_file(const char*);
extern int test_util_cjson_create(const char*);
extern int test_util_dh(const char*);
extern int test_util_dh_perf(const char*);
extern int test_util_random(const char*);
extern int test_util_shuffle(const char*);
extern int test_util_unicode(const char*);
extern int test_util_wscode(const char*);

static bool mode_all = false;
static bool mode_interact = false;

static int
traverse_callback(const char* commands, int result) {
    fflush(stdout);
    if (result == 0) {
        fprintf(stderr, "\033[%dm[SUCCESS]\033[0m   %s\n\n", COLOR_GREEN, commands);
    } else {
        fprintf(stderr, "\033[%dm[FAILURE]\033[0m   %s\n\n", COLOR_RED, commands);
    }
    fflush(stderr);
    return result;
}

static void
usage() {
    printf("usage:\n"
        "\t--all(a)             \"run all test cases\"\n"
        "\t--interact(i)        \"run under interact mode\"\n");
}

static void
run() {
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
    // seems some memory error ...
    // cmd_register(cmd, "core coroutine",             test_core_coroutine);
#endif
    cmd_register(cmd, "core fsm",                   test_core_fsm);
    cmd_register(cmd, "core lock",                  test_core_lock);
    cmd_register(cmd, "core spin",                  test_core_spin);
    cmd_register(cmd, "core thread",                test_core_thread);
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

    // unit test mode
    if (mode_all) {
        int res = cmd_traverse(cmd, NULL, traverse_callback);
        if (res < 0) {
            fprintf(stderr, "\033[%dm--> [RUN ALL TEST CASES FAILURE] <--\033[0m\n\n", COLOR_RED);
            cmd_release(cmd);
            exit(res);
        }
    }

    // interact mode
    if (mode_interact) {
        while (1) {
            char* line = cmd_readline(cmd);
            if (!line) {
                break;
            }
            int ret = cmd_handle(cmd, line);
            traverse_callback(line, ret);
            free(line);
            if (cmd_eof(cmd)) {
                if (cmd_closed(cmd)) {
                    break;
                }
            }
        }
    }

    cmd_release(cmd);
}

int
main(int argc, char** argv) {

    struct option opts[] = {
        {"all",         no_argument,    0,  'a'},
        {"interact",    no_argument,    0,  'i'},
        {0,             0,              0,  0}
    };

    int index, c;
    while ((c = getopt_long_only(argc, argv, "", opts, &index)) != -1) {
        switch (c) {
            case 'a':
                mode_all = true;
                break;
            case 'i':
                mode_interact = true;
                break;
            default:
                usage();
                exit(-1);
        }
    }

    if (!mode_all && !mode_interact) {
        usage();
        exit(-1);
    }

    run();
    return 0;
}

