#ifndef DIRTY_CHECK_H_
#define DIRTY_CHECK_H_

#include "core/os_def.h"

//
// dirty config or check source must be gbk encoding
// it's not a good way, utf-8 is better
// however, maybe we only need to do dirty check for gbk words
//
// note: now only gbk support!
//

// max dirty word len
#define MAX_DIRTY_WORDS_LEN 32

// max dirty word count
#define MAX_DIRTY_WORDS_COUNT 1024 * 16

// max check source
#define MAX_SOURCE_WORDS_LEN 1024 * 8

// replace charater
#define DIRTY_REPLACE_CHAR '*'

typedef struct dirty_ctx_t dirty_ctx_t;

dirty_ctx_t* dirty_create(const char* dirty_cfg);

int dirty_reload(dirty_ctx_t**, const char* dirty_cfg);

int dirty_check(dirty_ctx_t*, const char* src, int len);

int dirty_replace(dirty_ctx_t*, char* src, int len);

int dirty_release(dirty_ctx_t*);

#endif

