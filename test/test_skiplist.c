#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#include "util/util_time.h"
#include "base/skiplist.h"

int
skiplist_cmp(void* data1, void* data2) {
    return *(int*)data1 - *(int*)(data2);
}

const char*
skiplist_tostring(void* data) {
    static char buff[32];
    snprintf(buff, sizeof(buff), "%d", *(int*)data);
    return buff;
}

#define LIMIT 10240
#define CAP 16

static int test[CAP];

int
test_skiplist() {
    srand(time(NULL));
    for (int i = 0; i < CAP; ++ i) {
        test[i] = rand() % LIMIT;
    }

    struct timeval tm;
    char ts[128];
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("%s\n", ts);

    skiplist_t* sl = skiplist_create(skiplist_cmp);
    // skiplist_debug(sl, skiplist_tostring);

    for (int i = 0; i < CAP; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        assert(0 == ret);
        // skiplist_debug(sl, skiplist_tostring);
    }
    skiplist_debug(sl, skiplist_tostring);

    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("%s\n", ts);

    // int val = 1;
    // void* data = skiplist_find(sl, (void*)&val, 1);
    // assert(data);

    for (int i = 0; i < CAP; ++ i) {
        int top = 0;
        void* data = skiplist_find(sl, (void*)&test[i], &top, 1);
        assert(data);
        assert(*(int*)data == test[i]);
        // printf("data=%d rank=%d\n", test[i], top);
        // skiplist_debug(sl, skiplist_tostring);
    }
    skiplist_debug(sl, skiplist_tostring);

    for (int i = 0; i < CAP; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        assert(data);
        printf("data=%d rank=%d\n", *(int*)data, i + 1);
    }
    skiplist_debug(sl, skiplist_tostring);

    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("%s\n", ts);

    for (int i = 0; i < CAP; ++ i) {
        void* data[CAP];
        memset(data, 0, sizeof(data));
        int scope = rand() % CAP + 1;
        int from = rand() % CAP + 1;
        printf("find [%d, %d+%d=%d)\n", from, from, scope, from + scope);
        int ret = skiplist_find_list_by_rank(sl, from, &scope, &data[0]);
        assert(ret == 0);
        printf("find %d: ", scope);
        for (int j = 0; j < scope; ++ j) {
            printf("%d[%d] ", *(int*)data[j], from + j);
        }
        printf("\n\n");
    }

    for (int i = 0; i < CAP; ++ i) {
        int top = 0;
        void* data = skiplist_find(sl, (void*)&test[i], &top, 0);
        assert(data);
        assert(*(int*)data == test[i]);
        // skiplist_debug(sl, skiplist_tostring);
    }

    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("%s\n", ts);

    // skiplist_debug(sl, skiplist_tostring);
    skiplist_release(sl);

    return 0;
}
