#ifndef SLIST_H_
#define SLIST_H_

// easy single list

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct slist_t slist_t;

slist_t* slist_create();
void slist_release(slist_t* sl);

// more effective than push_back
int slist_push_front(slist_t* sl, void* data);
int slist_push_back(slist_t* sl, void* data);
// more effective than pop_back
void* slist_pop_front(slist_t* sl);
void* slist_pop_back(slist_t* sl);

int slist_remove(slist_t* sl, void* data);
int slist_find(slist_t* sl, void* data);
int slist_clean(slist_t* sl);
int slist_size(slist_t* sl);

#ifdef __cplusplus
}
#endif

#endif // SLIST_H_



