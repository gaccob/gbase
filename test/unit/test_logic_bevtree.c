#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "util/random.h"
#include "logic/bevtree.h"

typedef struct input_t {
    int ratio;
} input_t;

int
condition_300_cb(void* input) {
    input_t* i = (input_t*)(input);
    if (i->ratio < 50)
        return BVT_SUCCESS;
    return BVT_ERROR;
}
int
condition_301_cb(void* input) {
    input_t* i = (input_t*)(input);
    if (i->ratio >= 50)
        return BVT_SUCCESS;
    return BVT_ERROR;
}

int
action_400_cb(void* input) {
    return BVT_SUCCESS;
}
int
action_401_cb(void* input) {
    return BVT_SUCCESS;
}
int
action_402_cb(void* input) {
    return BVT_SUCCESS;
}
int
action_403_cb(void* input) {
    return BVT_SUCCESS;
}
int
action_404_cb(void* input) {
    return BVT_SUCCESS;
}

int
test_logic_bevtree(const char* param) {

    rand_seed(time(NULL));

    const char* file = param ? param : "bevtree.json";
    bvt_t* bvt = bvt_load_gliffy(file);
    if (!bvt) {
        fprintf(stderr, "bvt init fail\n");
        return -1;
    }
    bvt_debug(bvt);

    int ret = -1;
    ret = bvt_register_callback(bvt, action_400_cb, 400);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_401_cb, 401);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_402_cb, 402);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_403_cb, 403);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_404_cb, 404);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, condition_300_cb, 300);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, condition_301_cb, 301);
    assert(ret == BVT_SUCCESS);

    int loop = 10;
    while (-- loop > 0) {
        input_t i;
        i.ratio = rand_gen() % 100;
        ret = bvt_run(bvt, (void*)&i);
        if (ret != BVT_SUCCESS) {
            fprintf(stderr, "bvt run fail\n");
            bvt_release(bvt);
            return -1;
        }
        printf("============\n\n");
        usleep(100);
    }

    bvt_release(bvt);
    return 0;
}

