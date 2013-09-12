#ifndef CRC_H_
#define CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

int32_t crc32(const char* buffer, size_t length);

#ifdef __cplusplus
}
#endif

#endif // CRC_H_

