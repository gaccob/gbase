#include <assert.h>
#include "base/slist.h"

#define LOOP 32

int main()
{
    int data[LOOP];
    int i, res;
    int* p;
    struct slist_t* sl;
    sl = slist_init();
    assert(sl);

    for (i = 0; i < LOOP; ++ i) {
        data[i] = rand() % LOOP;
        res = slist_push_front(sl, &data[i]);
        assert(0 == res);
        res = slist_push_back(sl, &data[i]);
        assert(0 == res);
    }
    printf("list count=%d\n", slist_count(sl));

    for (i = LOOP-1; i >= 0; -- i) {
        res = slist_find(sl, &data[i]);
        assert(0 == res);
        p = slist_pop_front(sl);
        assert(p == &data[i]);
        p = slist_pop_back(sl);
        assert(p == &data[i]);
        res = slist_find(sl, &data[i]);
        assert(res < 0);
    }
    printf("list count=%d\n", slist_count(sl));

    slist_release(sl);
    sl = 0;
    return 0;
}

