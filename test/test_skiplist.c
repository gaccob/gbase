#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#include "util/util_time.h"
#include "base/skiplist.h"

typedef struct Test {
    int value;
    char dummy[64];
} Test;

int
skiplist_cmp(void* data1, void* data2) {
    return ((Test*)data1)->value - ((Test*)(data2))->value;
}

const char*
skiplist_tostring(void* data) {
    static char buff[32];
    snprintf(buff, sizeof(buff), "%d", *(int*)data);
    return buff;
}

#define CAP (1 << 20)
#define LIMIT (CAP * 100)

int scope = 100;

int
test_skiplist() {

    Test* test = (Test*)MALLOC(sizeof(Test) * CAP);

    srand(time(NULL));
    for (int i = 0; i < CAP; ++ i) {
        test[i].value = rand() % LIMIT;
    }

    struct timeval tm;
    char ts[128];
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("start %s\n", ts);

    skiplist_t* sl = skiplist_create(skiplist_cmp, 10);

    // insert
    for (int i = 0; i < CAP; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        assert(0 == ret);
    }
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("insert complete %s\n", ts);

    // find 
    for (int i = 0; i < CAP; ++ i) {
        int top = 0;
        void* data = skiplist_find(sl, (void*)&test[i], &top, 1);
        assert(data);
        assert(((Test*)data)->value == test[i].value);
    }
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("find complete %s\n", ts);

    // find by rank
    for (int i = 0; i < CAP; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        assert(data);
    }
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("find by rank complete %s\n", ts);

    // find by scope 
    for (int i = 0; i < CAP; ++ i) {
        void* data[scope];
        memset(data, 0, sizeof(data));
        int from = rand() % CAP + 1;
        int ret = skiplist_find_list_by_rank(sl, from, &scope, &data[0]);
        assert(ret == 0);
    }
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("find by scope complete %s\n", ts);

    // delete
    skiplist_release(sl);
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("release complete %s\n", ts);

    FREE(test);
    return 0;
}
