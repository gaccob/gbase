/*
* for push-pull
*/
#ifndef RBUFFER_H_
#define RBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"

struct rbuffer_t;
struct rbuffer_t* rbuffer_init(uint32_t size);
void rbuffer_release(struct rbuffer_t* r);
uint32_t rbuffer_read_bytes(struct rbuffer_t* r);
uint32_t rbuffer_write_bytes(struct rbuffer_t* r);
int rbuffer_read(struct rbuffer_t* r, char* buf, size_t* buf_size);
int rbuffer_peek(struct rbuffer_t* r, char* buf, size_t* buf_size);
int rbuffer_write(struct rbuffer_t* r, const char* buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif // RBUFFER_H_



