#ifndef TEST_H_
#define TEST_H_

#include "util/util_time.h"

int
get_process_time(struct timeval* from) {
    struct timeval tv;
    util_gettimeofday(&tv,NULL);
    return ((tv.tv_sec - from->tv_sec)*1000+(tv.tv_usec - from->tv_usec)/1000);
}

#endif
