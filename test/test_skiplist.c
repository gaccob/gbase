#include <time.h>
#include <assert.h>
#include <stdio.h>

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

#define LIMIT 1024
#define CAP 16

static int test[CAP];

int
test_skiplist() {
    srand(time(NULL));
    for (int i = 0; i < CAP; ++ i) {
        test[i] = rand() % LIMIT;
    }

    skiplist_t* sl = skiplist_create(skiplist_cmp);
    skiplist_debug(sl, skiplist_tostring);

    for (int i = 0; i < CAP; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        assert(0 == ret);
        skiplist_debug(sl, skiplist_tostring);
    }

    for (int i = 0; i < CAP; ++ i) {
        void* data = skiplist_find(sl, (void*)&test[i], 0);
        assert(data);
        assert(*(int*)data == test[i]);
    }
    skiplist_debug(sl, skiplist_tostring);

    skiplist_release(sl);
    skiplist_debug(sl, skiplist_tostring);

    return 0;
}
