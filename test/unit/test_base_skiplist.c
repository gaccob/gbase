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

// static const char*
// _tostring(void* data) {
//     static char buff[32];
//     snprintf(buff, sizeof(buff), "%d", *(int*)data);
//     return buff;
// }

static void
_stamp(const char* content) {
    struct timeval tm;
    char ts[128];
    gettimeofday(&tm, NULL);
    util_timestamp(&tm, ts, sizeof(ts));
    if (content) {
        printf("\t[%s] %s\n", content, ts);
    } else {
        printf("\t%s\n", ts);
    }
}

static void
_release(skiplist_t* sl, Test* test, const char* err) {
    if (err) {
        fprintf(stderr, "\t%s\n", err);
    }
    skiplist_release(sl);
    FREE(test);
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

/////////////////////////////////////////////////////////////

int
test_base_skiplist(char* param) {
    // raw data
    int cap = param ? atoi(param) : 128;
    int limit = cap * 10;
    int coff = 4;
    Test* test = (Test*)MALLOC(sizeof(Test) * cap);
    srand(time(NULL));
    for (int i = 0; i < cap; ++ i) {
        test[i].value = rand() % limit;
    }
    _stamp("prepare");

    // create
    skiplist_t* sl = skiplist_create(_cmp, coff);
    for (int i = 0; i < cap; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        if (ret < 0) {
            _release(sl, test, "insert fail");
            return -1;
        }
    }
    // skiplist_debug(sl, _tostring);
    _stamp("create");

    // erase and add
    for (int i = 0; i < cap; ++ i) {
        int dest = rand() % limit;
        void* data = skiplist_erase(sl, (void*)&test[i]);
        if (!data) {
            _release(sl, test, "erase fail");
            return -1;
        }

        // maybe duplicated, add back
        if (data != &test[i]) {
            int ret = skiplist_insert(sl, (void*)&test[i]);
            if (ret < 0) {
                _release(sl, test, "re-insert fail");
                return -1;
            }
        }

        // replace
        else {
            test[i].value = dest;
            int ret = skiplist_insert(sl, (void*)&test[i]);
            if (ret < 0) {
                _release(sl, test, "replace fail");
                return -1;
            }
        }
    }
    // skiplist_debug(sl, _tostring);
    _stamp("replace");

    // find
    for (int i = 0; i < cap; ++ i) {
        int top = 0;
        void* data = skiplist_find(sl, (void*)&test[i], &top);
        if (!data) {
            _release(sl, test, "find fail");
            return -1;
        }
        assert(data);
        assert(((Test*)data)->value == test[i].value);
    }
    _stamp("find");

    // find by rank
    for (int i = 0; i < cap; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        if (!data) {
            _release(sl, test, "find by rank fail");
            return -1;
        }
    }
    _stamp("find by rank");

    // find by scope
    int scope = 100;
    for (int i = 0; i < cap; ++ i) {
        void* data[scope];
        memset(data, 0, sizeof(data));
        int from = rand() % cap + 1;
        int ret = skiplist_find_list_by_rank(sl, from, &scope, &data[0]);
        if (ret < 0) {
            _release(sl, test, "find list by rank fail");
            return -1;
        }
    }
    _stamp("find list by rank");

    // delete
    _release(sl, test, NULL);
    return 0;
}

/////////////////////////////////////////////////////////////////////////

int
test_base_skiplist_duplicate(char* param) {

    // raw data
    int cap = param ? atoi(param) : 10;
    Test* test = (Test*)MALLOC(sizeof(Test) * cap);
    for (int i = 0; i < cap; ++ i) {
        test[i].value = 99;
    }

    // create
    skiplist_t* sl = skiplist_create(_cmp, 4);
    if (!sl) {
        _release(sl, test, "create fail");
        return -1;
    }

    // insert
    for (int i = 0; i < cap; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        if (ret < 0) {
            _release(sl, test, "insert fail");
            return -1;
        }
    }

    // find
    for (int i = 0; i < cap; ++ i) {
        int top = 0;
        void* data = skiplist_find(sl, (void*)&test[i], &top);
        if (!data || ((Test*)data)->value != test[i].value) {
            _release(sl, test, "find fail");
            return -1;
        }
        printf("\t%d rank[%d] addr[%p]\n", ((Test*)data)->value, top, data);
    }
    printf("\n");

    // find by rank
    for (int i = 0; i < cap; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        if (!data) {
            _release(sl, test, "find by rank fail");
            return -1;
        }
        printf("\t%d rand[%d] addr[%p]\n", ((Test*)data)->value, i + 1, data);
    }

    _release(sl, test, NULL);
    return 0;
}

//////////////////////////////////////////////////////

int
test_base_skiplist_find(char* param) {

    // raw data
    int cap = param ? atoi(param) : 10;
    Test* test = (Test*)MALLOC(sizeof(Test) * cap);
    for (int i = 0; i < cap; ++ i) {
        test[i].value = i + 1;
    }

    // create
    skiplist_t* sl = skiplist_create(_cmp, 4);
    if (!sl) {
        _release(sl, test, "create fail");
        return -1;
    }

    // insert
    for (int i = 0; i < cap; ++ i) {
        int ret = skiplist_insert(sl, (void*)&test[i]);
        if (ret < 0) {
            _release(sl, test, "insert fail");
            return -1;
        }
    }

    // find by rank
    int arg = 0;
    int val, rank;
    for (int i = 0; i < cap; ++ i) {
        void* data = skiplist_find_by_rank(sl, i + 1);
        if (!data) {
            _release(sl, test, "find rank fail");
            return -1;
        }
        val = ((Test*)data)->value;
        rank = i + 1;
        if (val != rank) {
            _release(sl, test, "rank invalid");
            return -1;
        }

        arg = 2;
        data = skiplist_find_from_rank_forward(sl, i + 1, _filter, &arg);
        if (data) {
            val = ((Test*)data)->value;
            rank = i + 1 + 2;
            if (val != rank) {
                _release(sl, test, "rank forward invalid");
                return -1;
            }
        }

        arg = 2;
        data = skiplist_find_from_rank_backward(sl, i + 1, _filter, &arg);
        if (data) {
            val = ((Test*)data)->value;
            rank = i + 1 - 2;
            if (val != rank) {
                _release(sl, test, "rank backward invalid");
                return -1;
            }
        }
    }

    _release(sl, test, NULL);
    return 0;
}

