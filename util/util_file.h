#ifndef UTIL_FILE_H_
#define UTIL_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

int32_t util_access(const char* filepath);
const char* util_dirname(char* filepath);

int32_t util_path_exist(const char* filepath);
int32_t util_is_dir(const char* path);
int32_t util_is_file(const char* path);

#ifdef __cplusplus
}
#endif

#endif // UTIL_FILE_H_

