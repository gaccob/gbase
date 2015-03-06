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

static int
_cmp(void* data1, void* data2) {
    return ((Test*)data1)->value - ((Test*)(data2))->value;
}

static const char*
_tostring(void* data) {
    static char buff[32];
    snprintf(buff, sizeof(buff), "%d", *(int*)data);
    return buff;
}

static void
_print_timestamp() {
    struct timeval tm;
    char ts[128];
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    printf("timestamp: %s\n", ts);
}

int
test_skiplist() {
    int cap = 128;
    int limit = cap * 10;
    int coff = 4;
    Test* test = (Test*)MALLOC(sizeof(Test) * cap);
    srand(time(NULL));
    for (int i = 0; i < cap; ++ i) {
        test[i].value = rand() % limit;
    }
    _print_timestamp();

    // create
    skiplist_t* sl = skiplist_create(_cmp, coff);
    for (int i = 0; i < cap; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        assert(0 == ret);
    }
    skiplist_debug(sl, _tostring);
    _print_timestamp();

    // erase and add
    for (int i = 0; i < cap; ++ i) {
        int dest = rand() % limit;
        void* data = skiplist_erase(sl, (void*)&test[i]);
        assert(data);
        // maybe duplicated, add back
        if (data != &test[i]) {
            int ret = skiplist_insert(sl, (void*)&test[i]);
            assert(0 == ret);
        }
        // replace
        else {
            printf("replace %d->%d\n", test[i].value, dest);
            assert(data == &test[i]);
            test[i].value = dest;
            int ret = skiplist_insert(sl, (void*)&test[i]);
            assert(0 == ret);
        }
    }
    skiplist_debug(sl, _tostring);

    // find
    for (int i = 0; i < cap; ++ i) {
        int top = 0;
        void* data = skiplist_find(sl, (void*)&test[i], &top);
        assert(data);
        assert(((Test*)data)->value == test[i].value);
    }
    _print_timestamp();

    // find by rank
    for (int i = 0; i < cap; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        assert(data);
    }
    _print_timestamp();

    // find by scope 
    int scope = 100;
    for (int i = 0; i < cap; ++ i) {
        void* data[scope];
        memset(data, 0, sizeof(data));
        int from = rand() % cap + 1;
        int ret = skiplist_find_list_by_rank(sl, from, &scope, &data[0]);
        assert(ret == 0);
    }
    _print_timestamp();

    // delete
    skiplist_release(sl);
    _print_timestamp();

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

    skiplist_t* sl = skiplist_create(_cmp, 4);

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
    skiplist_t* sl = skiplist_create(_cmp, 4);

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

