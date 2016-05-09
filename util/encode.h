#ifndef ENCODE_H_
#define ENCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

//
// utf8: input string
// unicode: output unicode
//
// return -1, fail
// return 0, success
int utf8_unicode(char** utf8, int* unicode);


// signed origin        zigzag encoded
// 0                    0
// -1                   1
// 1                    2
//              ...
// 2^31 - 1             2^32 -2
// -2^31                2^32 -1
#define ZIGZAG32(n) ((n << 1) ^ (n >> 31))
#define ZIGZAG64(n) ((n << 1) ^ (n >> 63))

#ifdef __cplusplus
}
#endif

#endif
