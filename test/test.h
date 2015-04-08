#ifndef TEST_H_
#define TEST_H_

#include "logic/bus.h"

#define WS_IP "127.0.0.1"
#define WS_PORT 8000
int test_ws_server();

#define BUS_KEY 0x1234
int test_bus(bus_addr_t);

#endif

