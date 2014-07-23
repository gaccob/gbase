#include <sys/time.h>
#include <assert.h>
#include "util_time.h"

uint32_t
util_hour_number(time_t time) {
    struct tm now_tm;
    localtime_r(&time, &now_tm);
    return (uint32_t)(now_tm.tm_year + 1900) * 1000000
        + (uint32_t)(now_tm.tm_mon + 1) * 10000
        + (uint32_t)now_tm.tm_mday * 100
        + (uint32_t)now_tm.tm_hour;
}

uint32_t
util_date_number(time_t time) {
    struct tm now_tm;
    localtime_r(&time, &now_tm);
    return (uint32_t)(now_tm.tm_year + 1900) * 10000
        + (uint32_t)(now_tm.tm_mon + 1) * 100
        + (uint32_t)now_tm.tm_mday;
}

void
util_timestamp(struct timeval* time, char* stamp, size_t sz) {
    struct tm now_tm;
    localtime_r((time_t*)&time->tv_sec, &now_tm);
    snprintf(stamp, sz, "[%d-%02d-%02d %02d:%02d:%02d:%06d]",
        now_tm.tm_year + 1900,
        now_tm.tm_mon + 1,
        now_tm.tm_mday,
        now_tm.tm_hour,
        now_tm.tm_min,
        now_tm.tm_sec,
        (uint32_t)time->tv_usec);
}

// tv1 and tv2 must be normalized
int32_t
util_time_compare(struct timeval* tv1, struct timeval* tv2) {
    assert(tv1 && tv2);
    if (tv1->tv_sec > tv2->tv_sec) return 1;
    if (tv1->tv_sec < tv2->tv_sec) return -1;
    if (tv1->tv_usec > tv2->tv_usec) return 1;
    if (tv1->tv_usec < tv2->tv_usec) return -1;
    return 0;
}

void
util_time_add(struct timeval* tv1, struct timeval* tv2, struct timeval* sum) {
    assert(tv1 && tv2 && sum);
    sum->tv_sec = tv1->tv_sec + tv2->tv_sec;
    sum->tv_usec = tv1->tv_usec + tv2->tv_usec;
    while (sum->tv_usec > 1000000) {
        sum->tv_sec ++;
        sum->tv_usec -= 1000000;
    }
}

void
util_time_sub(struct timeval* tv1, struct timeval* tv2, struct timeval* sub) {
    assert(tv1 && tv2 && sub);
    assert(util_time_compare(tv1, tv2) > 0);
    sub->tv_sec = tv1->tv_sec - tv2->tv_sec;
    if (tv1->tv_usec < tv2->tv_usec) {
        sub->tv_usec = tv1->tv_usec + 1000000 - tv2->tv_usec;
        sub->tv_sec --;
    } else {
        sub->tv_usec = tv1->tv_usec - tv2->tv_usec;
    }
}

