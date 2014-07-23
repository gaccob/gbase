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

typedef struct rbuffer_t rbuffer_t;

rbuffer_t* rbuffer_create(uint32_t size);
void rbuffer_release(rbuffer_t* r);

// create from an allocated memory
// if create from memory, usually we don't need to release it
rbuffer_t* rbuffer_attach(void* mem, size_t mem_size);

size_t rbuffer_size(rbuffer_t* r);
size_t rbuffer_head_size();

uint32_t rbuffer_read_bytes(rbuffer_t* r);
uint32_t rbuffer_write_bytes(rbuffer_t* r);

int rbuffer_read(rbuffer_t* r, char* buf, size_t* buf_size);
int rbuffer_peek(rbuffer_t* r, char* buf, size_t* buf_size);
int rbuffer_write(rbuffer_t* r, const char* buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif // RBUFFER_H_



