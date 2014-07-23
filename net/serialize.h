#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct serial_t serial_t;

serial_t* serial_create(char* buffer, size_t size);
void serial_release(serial_t*);

int serial_write8(serial_t*, uint8_t src);
int serial_read8(serial_t*, uint8_t* dest);

int serial_write16(serial_t*, uint16_t src);
int serial_read16(serial_t*, uint16_t* dest);

int serial_write32(serial_t*, uint32_t src);
int serial_read32(serial_t*, uint32_t* dest);

int serial_write64(serial_t*, uint64_t src);
int serial_read64(serial_t*, uint64_t* dest);

int serial_writef(serial_t*, float src);
int serial_readf(serial_t*, float* dest);

int serial_writed(serial_t*, double src);
int serial_readd(serial_t*, double* dest);

int serial_writen(serial_t*, const char* data, uint32_t len);
int serial_readn(serial_t*, char* data, uint32_t* len);

#ifdef __cplusplus
}
#endif

#endif // SERIALIZE_H_

