#ifndef UTIL_TIME_H_
#define UTIL_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include <time.h>

uint32_t util_hour_number(time_t time);
uint32_t util_date_number(time_t time);
void util_timestamp(struct timeval* time, char* stamp, size_t sz);
void util_localtime(const time_t *time, struct tm* _tm);
void util_gettimeofday(struct timeval* tv, void* tz);

// tv1 and tv2 must be normalized
int32_t util_time_compare(struct timeval* tv1, struct timeval* tv2);
void util_time_add(struct timeval* tv1, struct timeval* tv2, struct timeval* sum);
void util_time_sub(struct timeval* tv1, struct timeval* tv2, struct timeval* sub);

#ifdef __cplusplus
}
#endif

#endif // UTIL_TIME_H_

