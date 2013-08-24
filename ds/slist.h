#ifndef SLIST_H_
#define SLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct slist_t;
struct slist_t* slist_init();
void slist_release(struct slist_t* sl);
int slist_insert(struct slist_t* sl, void* data);
int slist_remove(struct slist_t* sl, void* data);
int slist_find(struct slist_t* sl, void* data);
int slist_clean(struct slist_t* sl);
int slist_count(struct slist_t* sl);

#ifdef __cplusplus
}
#endif

#endif // SLIST_H_



