#include "util/random.h"
#include "base/slist.h"

#define CHECK(sl, condition, prompt) \
    if (!(condition)) {\
        fprintf(stderr, "%s\n", prompt); \
        slist_release(sl); \
        return -1; \
    }

int
test_base_slist(const char* param) {
    struct slist_t* sl = slist_create();
    if (!sl) {
        fprintf(stderr, "slist create fail\n");
        return -1;
    }

    rand_seed(time(NULL));
    int loop = param ? atoi(param) : 32;
    int data[loop];
    for (int i = 0; i < loop; ++ i) {
        data[i] = (int)(rand_gen()) % loop;
        int res = slist_push_front(sl, &data[i]);
        CHECK(sl, res == 0, "slist push front fail");

        res = slist_push_back(sl, &data[i]);
        CHECK(sl, res == 0, "slist push back fail");
    }

    CHECK(sl, slist_size(sl) == 2 * loop, "slist size fail");

    for (int i = loop - 1; i >= 0; -- i) {
        int res = slist_find(sl, &data[i]);
        CHECK(sl, res == 0, "slist find fail");

        void* p = slist_pop_front(sl);
        CHECK(sl, p == &data[i], "slist pop front fail");

        p = slist_pop_back(sl);
        CHECK(sl, p == &data[i], "slist pop back fail");

        res = slist_find(sl, &data[i]);
        CHECK(sl, res < 0, "slist find fail");
    }

    CHECK(sl, slist_size(sl) == 0, "slist size fail");

    slist_release(sl);
    return 0;
}

