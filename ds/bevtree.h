#ifndef BEVTREE_H_
#define BEVTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#ifndef BVT_DEBUG
#define BVT_DEBUG
#endif

enum
{
    BVT_SUCCESS = 0,
    BVT_ERROR = -10000,
    BVT_SELECTOR_ERROR,
    BVT_CONDITION_ERROR,
    BVT_SEQUENCE_ERROR,
    BVT_PARALLEL_ERROR,
};

// return BVT error code
typedef int32_t (*bvt_callback)(void* input);

struct bvt_t;
struct bvt_t* bvt_init();
int32_t bvt_register_callback(struct bvt_t* b, bvt_callback cb, int32_t id);
int32_t bvt_run(struct bvt_t* b, void* input);
void bvt_release(struct bvt_t* b);

#ifdef __cplusplus
}
#endif

#endif
