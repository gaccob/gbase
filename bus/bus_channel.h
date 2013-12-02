#ifndef BUS_CHANNEL_H_
#define BUS_CHANNEL_H_

#ifdef __cpluplus
extern "C" {
#endif

#include "core/os_def.h"
#include "bus/bus_terminal.h"

// single way, saved in share-memory
typedef struct bus_channel_t
{
    int32_t shmkey;
    size_t channel_size;
    bus_addr_t from;
    bus_addr_t to;
} bus_channel_t;

#define BUS_CHANNEL_DEFAULT_SIZE (1 << 20)

// if create == 0: create new share-memory channel
// else: attach exist share-memory channel and verify data
struct bus_terminal_channel_t* bus_terminal_channel_init(int32_t shmkey, bus_addr_t from,
                                                         bus_addr_t to, size_t size,
                                                         int32_t create);

void bus_terminal_channel_release(struct bus_terminal_channel_t*);

// get channel inner rbuffer
struct rbuffer_t* bus_terminal_channel_rbuffer(struct bus_terminal_channel_t*);

// get channel data saved in shm
bus_channel_t* bus_terminal_channel(struct bus_terminal_channel_t*);

#ifdef __cpluplus
}
#endif

#endif
