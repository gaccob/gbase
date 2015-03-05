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

#define CAP (1 << 10)
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
        void* data = skiplist_find(sl, (void*)&test[i], &top);
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

int
test_skiplist_dup() {
    int cap = 20;
    Test* test = (Test*)MALLOC(sizeof(Test) * cap);
    for (int i = 0; i < cap; ++ i) {
        test[i].value = 99;
    }

    skiplist_t* sl = skiplist_create(skiplist_cmp, 4);

    // insert
    for (int i = 0; i < cap; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        assert(0 == ret);
    }

    // find 
    for (int i = 0; i < cap; ++ i) {
        int top = 0;
        void* data = skiplist_find(sl, (void*)&test[i], &top);
        assert(data);
        assert(((Test*)data)->value == test[i].value);
        printf("rank %d addr[%lu]\n", top, (intptr_t)data);
    }
    printf("\n");

    // find by rank
    for (int i = 0; i < cap; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        assert(data);
        printf("rand %d addr[%lu]\n", i + 1, (intptr_t)data);
    }

    skiplist_release(sl);
    FREE(test);
    return 0;
}

static int
_filter(void* data, void* args) {
    assert(data && args);
    int count = *(int*)(args);
    if (count > 0) {
        *(int*)args = count - 1;
        return -1;
    }
    return 0;
}

int
test_skiplist_find() {
    int cap = 10;
    Test* test = (Test*)MALLOC(sizeof(Test) * cap);
    for (int i = 0; i < cap; ++ i) {
        test[i].value = i;
    }
    skiplist_t* sl = skiplist_create(skiplist_cmp, 4);

    // insert
    for (int i = 0; i < cap; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        assert(0 == ret);
    }

    // find by rank
    int arg = 0;
    for (int i = 0; i < cap; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        assert(data);
        printf("%d rank = %d\n", ((Test*)data)->value, i + 1);

        arg = 2;
        data = skiplist_find_from_rank_forward(sl, i + 1, _filter, &arg);
        if (data) {
            printf("%d rank = %d + 2\n", ((Test*)data)->value, i + 1);
        }

        arg = 2;
        data = skiplist_find_from_rank_backward(sl, i + 1, _filter, &arg);
        if (data) {
            printf("%d rank = %d - 2\n", ((Test*)data)->value, i + 1);
        }

        printf("\n");
    }

    skiplist_release(sl);
    FREE(test);
    return 0;

}

