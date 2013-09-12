#ifndef BASE64_H_
#define BASE64_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

int32_t base64_encode(char* dst, const char* src, size_t sz);
int32_t base64_decode(char* dst, const char* src, size_t sz);

#ifdef __cplusplus
}
#endif

#endif // UTIL_H_

