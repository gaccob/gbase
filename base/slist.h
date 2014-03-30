#ifndef SLIST_H_
#define SLIST_H_

// easy single list

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct slist_t;
struct slist_t* slist_create();
void slist_release(struct slist_t* sl);

// more effective than push_back
int slist_push_front(struct slist_t* sl, void* data);
int slist_push_back(struct slist_t* sl, void* data);
// more effective than pop_back
void* slist_pop_front(struct slist_t* sl);
void* slist_pop_back(struct slist_t* sl);

int slist_remove(struct slist_t* sl, void* data);
int slist_find(struct slist_t* sl, void* data);
int slist_clean(struct slist_t* sl);
int slist_size(struct slist_t* sl);

#ifdef __cplusplus
}
#endif

#endif // SLIST_H_



