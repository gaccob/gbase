#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"
#include <time.h>

int32_t util_crc32(const char* buffer, size_t length);

int32_t util_encrypt(const char* src, size_t src_len, int32_t key, char* dst, size_t* dst_len);
int32_t util_descrypt(const char* src, size_t src_len, int32_t key, char* dst, size_t* dst_len);

uint32_t util_hour_number(time_t time);
uint32_t util_date_number(time_t time);
void util_timestamp(struct timeval* time, char* stamp);
void util_localtime(const time_t *time, struct tm* _tm);
void util_gettimeofday(struct timeval* tv, void* tz);

/* tv1 and tv2 must be normalized */
int32_t util_time_compare(struct timeval* tv1, struct timeval* tv2);
void util_time_add(struct timeval* tv1, struct timeval* tv2, struct timeval* sum);
void util_time_sub(struct timeval* tv1, struct timeval* tv2, struct timeval* sub);

int32_t util_access(const char* filepath);
const char* util_dirname(char* filepath);

int32_t util_path_exist(const char* filepath);
int32_t util_is_dir(const char* path);
int32_t util_is_file(const char* path);

int32_t util_base64_encode(char* dst, const char* src, size_t sz);
int32_t util_base64_decode(char* dst, const char* src, size_t sz);

#ifdef __cplusplus
}
#endif

#endif // UTIL_H_

