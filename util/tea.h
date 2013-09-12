#ifndef TEA_H_
#define TEA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

int32_t tea_encrypt(const char* src, size_t src_len, int32_t key, char* dst, size_t* dst_len);
int32_t tea_descrypt(const char* src, size_t src_len, int32_t key, char* dst, size_t* dst_len);

#ifdef __cplusplus
}
#endif

#endif // TEA_H_

