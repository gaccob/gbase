#ifndef BUS_H_
#define BUS_H_

#ifdef __cpluplus
extern "C" {
#endif

#include "core/os_def.h"

#define BUS_MAX_PIPE_COUNT 1024
#define BUS_MAX_TERMINAL_COUNT 64

#define BUS_PIPE_DEFAULT_SIZE 102400

typedef int bus_addr_t;

#define bus_addr_type(addr) (addr >> 16)
#define bus_addr_id(addr) ((addr << 16) >> 16)

typedef struct bus_t bus_t;

enum {
    BUS_ERR_PEER_NOT_FOUND = -100,
    BUS_ERR_SEND_FAIL,
    BUS_ERR_RECV_FAIL,
    BUS_ERR_PEER_EXIST,
    BUS_ERR_PEER_FAIL,
    BUS_ERR_PIPE_FULL,
    BUS_ERR_PIPE_FAIL,
    BUS_ERR_EMPTY,
    BUS_ERR_FAIL,
    BUS_OK = 0,
};

// key: 16 bits, reserved 16 bits for channels
// distinct address is ensured by user
bus_t* bus_create(int16_t key, bus_addr_t addr);
void bus_release(bus_t*);

// check bus version and do update
// we should check in every tick
void bus_poll(bus_t*);

int bus_send(bus_t*, const char* buf, size_t bufsz, bus_addr_t to);
int bus_send_by_type(bus_t*, const char* buf, size_t bufsz, int type);
int bus_send_all(bus_t*, const char* buf, size_t bufsz);
int bus_recv(bus_t*, char* buf, size_t* bufsz, bus_addr_t from);
int bus_recv_all(bus_t*, char* buf, size_t* bufsz, bus_addr_t* from);

uint32_t bus_send_bytes(bus_t*, bus_addr_t to);
uint32_t bus_recv_bytes(bus_t*, bus_addr_t from);

void bus_dump(bus_t*, char* debug, size_t debugsz);

#ifdef __cpluplus
}
#endif

#endif

