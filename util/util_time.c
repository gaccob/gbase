#include <assert.h>
#include "util_time.h"

#if defined(OS_WIN)
    #include <io.h>
#elif defined(OS_LINUX) || defined(OS_MAC)
    #include <sys/time.h>
#endif

uint32_t util_hour_number(time_t time)
{
    struct tm now_tm;
    util_localtime(&time, &now_tm);
    return (uint32_t)(now_tm.tm_year + 1900) * 1000000
        + (uint32_t)(now_tm.tm_mon + 1) * 10000
        + (uint32_t)now_tm.tm_mday * 100
        + (uint32_t)now_tm.tm_hour;
}

uint32_t util_date_number(time_t time)
{
    struct tm now_tm;
    util_localtime(&time, &now_tm);
    return (uint32_t)(now_tm.tm_year + 1900) * 10000
        + (uint32_t)(now_tm.tm_mon + 1) * 100
        + (uint32_t)now_tm.tm_mday;
}

void util_timestamp(struct timeval* time, char* stamp)
{
    struct tm now_tm;
    util_localtime((time_t*)&time->tv_sec, &now_tm);
    sprintf(stamp, "[%d-%02d-%02d %02d:%02d:%02d:%06d]",
        now_tm.tm_year + 1900,
        now_tm.tm_mon + 1,
        now_tm.tm_mday,
        now_tm.tm_hour,
        now_tm.tm_min,
        now_tm.tm_sec,
        (uint32_t)time->tv_usec);
}

void util_localtime(const time_t *time, struct tm* _tm)
{
#if defined(OS_WIN)
    localtime_s(_tm, time);
#elif defined(OS_LINUX) || defined(OS_MAC)
    localtime_r(time, _tm);
#endif
}

void util_gettimeofday(struct timeval* tv, void* tz)
{
#if defined(OS_WIN)
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tv->tv_sec = (long)clock;
    tv->tv_usec = wtm.wMilliseconds * 1000;
#elif defined(OS_LINUX) || defined(OS_MAC)
    gettimeofday(tv, (struct timezone*)tz);
#endif
}

// tv1 and tv2 must be normalized
int32_t util_time_compare(struct timeval* tv1, struct timeval* tv2)
{
    assert(tv1 && tv2);
    if (tv1->tv_sec == tv2->tv_sec
        && tv1->tv_usec == tv2->tv_usec) {
        return 0;
    }

    if(tv1->tv_sec > tv2->tv_sec
        || (tv1->tv_sec == tv2->tv_sec
        && tv1->tv_usec > tv2->tv_usec)) {
        return 1;
    }

    return -1;
}

void util_time_add(struct timeval* tv1, struct timeval* tv2, struct timeval* sum)
{
    assert(tv1 && tv2 && sum);
    sum->tv_sec = tv1->tv_sec + tv2->tv_sec;
    sum->tv_usec = tv1->tv_usec + tv2->tv_usec;
    while (sum->tv_usec > 1000000) {
        sum->tv_sec ++;
        sum->tv_usec -= 1000000;
    }
}

void util_time_sub(struct timeval* tv1, struct timeval* tv2, struct timeval* sub)
{
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
