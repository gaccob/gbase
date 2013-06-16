#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"

struct serial_t;

struct serial_t* serial_init(char* buffer, size_t size);
void serial_release(struct serial_t* s);

int serial_write8(struct serial_t* s, uint8_t src);
int serial_read8(struct serial_t* s, uint8_t* dest);

int serial_write16(struct serial_t* s, uint16_t src);
int serial_read16(struct serial_t* s, uint16_t* dest);

int serial_write32(struct serial_t* s, uint32_t src);
int serial_read32(struct serial_t* s, uint32_t* dest);

int serial_write64(struct serial_t* s, uint64_t src);
int serial_read64(struct serial_t* s, uint64_t* dest);

int serial_writef(struct serial_t* s, float src);
int serial_readf(struct serial_t* s, float* dest);

int serial_writed(struct serial_t* s, double src);
int serial_readd(struct serial_t* s, double* dest);

int serial_writen(struct serial_t* s, const char* data, uint32_t len);
int serial_readn(struct serial_t* s, char* data, uint32_t* len);

#ifdef __cplusplus
}
#endif

#endif // SERIALIZE_H_

