#ifndef TEST_H_
#define TEST_H_

#include "util/util_time.h"

int get_process_time(struct timeval* from);

int test_base64();
int test_wscode();
int test_random();
int test_shuffle();
int test_unicode();
int test_conhash();
#ifdef OS_LINUX
int test_coroutine();
#endif
int test_fsm();
int test_bitset();
int test_heap();
int test_rbtree();
int test_rbuffer();
int test_slist();
int test_timer();
int test_atom();
int test_spin();
int test_lock();
int test_task();
int test_shm_send();
int test_shm_recv();
int test_slab();
int test_dirty();
int test_thread();

int test_json_text();
int test_json_file();
int test_json_create();

int test_dh_perf();
int test_dh();

#define ECHO_IP "127.0.0.1"
#define ECHO_PORT 8000
#define ECHO_CMD_STOP "stop\n"
#define ECHO_CMD_WORD '\n'
int test_echo_svr();
int test_echo_cli();

int test_curl();

#define WS_IP "127.0.0.1"
#define WS_PORT 8000
int test_ws_server();

#endif
