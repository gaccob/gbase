#ifndef TCP_TEST_H_
#define TCP_TEST_H_

#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "net/acceptor.h"
#include "net/connector.h"

const char* server_addr = "127.0.0.1";
int server_port = 8000;

const char* const stop_cmd = "stop\n";
const char word_cmd = '\n';


#endif // TCP_TEST_H_
