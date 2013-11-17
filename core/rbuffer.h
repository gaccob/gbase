#ifndef RBUFFER_H_
#define RBUFFER_H_

//
// it's a ring buffer, so don't need to do memmove
// it's flag is atomic, so lock-free
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct rbuffer_t;

struct rbuffer_t* rbuffer_init(uint32_t size);
void rbuffer_release(struct rbuffer_t* r);

// init from an allocated memory
// if inited from memory, usually we don't need to release it
struct rbuffer_t* rbuffer_init_mem(void* mem, size_t mem_size);

size_t rbuffer_size(struct rbuffer_t* r);
size_t rbuffer_head_size();

uint32_t rbuffer_read_bytes(struct rbuffer_t* r);
uint32_t rbuffer_write_bytes(struct rbuffer_t* r);

int rbuffer_read(struct rbuffer_t* r, char* buf, size_t* buf_size);
int rbuffer_peek(struct rbuffer_t* r, char* buf, size_t* buf_size);
int rbuffer_write(struct rbuffer_t* r, const char* buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif // RBUFFER_H_



