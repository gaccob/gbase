#include <assert.h>
#include "base/slist.h"

#define SLIST_LOOP 32

int
test_slist() {
    int data[SLIST_LOOP];
    int i, res;
    int* p;
    struct slist_t* sl;
    sl = slist_create();
    assert(sl);

    for (i = 0; i < SLIST_LOOP; ++ i) {
        data[i] = rand() % SLIST_LOOP;
        res = slist_push_front(sl, &data[i]);
        assert(0 == res);
        res = slist_push_back(sl, &data[i]);
        assert(0 == res);
    }
    printf("list size=%d\n", slist_size(sl));

    for (i = SLIST_LOOP-1; i >= 0; -- i) {
        res = slist_find(sl, &data[i]);
        assert(0 == res);
        p = slist_pop_front(sl);
        assert(p == &data[i]);
        p = slist_pop_back(sl);
        assert(p == &data[i]);
        res = slist_find(sl, &data[i]);
        assert(res < 0);
    }
    printf("list size=%d\n", slist_size(sl));

    slist_release(sl);
    sl = 0;
    return 0;
}

