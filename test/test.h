#ifndef TEST_H_
#define TEST_H_

#include "util/util_time.h"
#include "util/cmd.h"
#include "logic/bus.h"

int get_process_time(struct timeval* from);

#ifdef OS_LINUX
int test_coroutine();
#endif
int test_spin();
int test_shm_send();
int test_shm_recv();
int test_thread();

int test_dh_perf();
int test_dh();

#define ECHO_IP "0.0.0.0"
#define ECHO_PORT 8000
#define ECHO_CMD_STOP "stop\n"
#define ECHO_CMD_WORD '\n'
int test_echo_svr();
int test_echo_cli();

#define WS_IP "127.0.0.1"
#define WS_PORT 8000
int test_ws_server();

#define BUS_KEY 0x1234
int test_bus(bus_addr_t);

#define BEV_LOOP 1000
#define BEV_FILE "json_test_file"
int test_bevtree();

#endif

