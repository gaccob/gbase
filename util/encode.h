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
int get_unicode(char** utf8, int* unicode);

#ifdef __cplusplus
}
#endif

#endif
