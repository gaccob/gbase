#ifndef BUS_TERMINAL_H_
#define BUS_TERMINAL_H_

#include "core/os_def.h"

#define BUS_MAX_CHANNLE_COUNT 1024
#define BUS_MAX_TERMINAL_COUNT 64

typedef int bus_addr_t;

#define bus_addr_type(addr) (addr >> 16)
#define bus_addr_id(addr) ((addr << 16) >> 16)

struct bus_terminal_t;

enum {
    bus_err_peer_not_found = -100,
    bus_err_send_fail,
    bus_err_recv_fail,
    bus_err_peek_fail,
    bus_err_channel_full,
    bus_err_channel_fail,
    bus_err_empty,
    bus_err_fail,
    bus_ok = 0,
};

// key: 16 bits, reserved 16 bits for channels
struct bus_terminal_t* bus_terminal_init(int16_t key, bus_addr_t ba);

void bus_terminal_release(struct bus_terminal_t* bt);

// check bus version and do update
// we should do dispatch in every tick
void bus_terminal_dispatch(struct bus_terminal_t* bt);

int32_t bus_terminal_send(struct bus_terminal_t* bt, const char* buf,
                          size_t buf_size, bus_addr_t to);
int32_t bus_terminal_send_by_type(struct bus_terminal_t* bt, const char* buf,
                                  size_t buf_size, int bus_type);
int32_t bus_terminal_send_all(struct bus_terminal_t* bt, const char* buf,
                              size_t buf_size);
int32_t bus_terminal_recv(struct bus_terminal_t* bt, char* buf,
                          size_t* buf_size, bus_addr_t from);
int32_t bus_terminal_recv_all(struct bus_terminal_t* bt, char* buf,
                              size_t* buf_size, bus_addr_t* from);

uint32_t bus_terminal_send_bytes(struct bus_terminal_t* bt, bus_addr_t to);
uint32_t bus_terminal_recv_bytes(struct bus_terminal_t* bt, bus_addr_t from);

void bus_terminal_dump(struct bus_terminal_t* bt, char* debug,
                       size_t debug_size);
#endif
