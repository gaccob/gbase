#ifndef IDTABLE_H_
#define IDTABLE_H_

//
// id-table, it looks likes an array, but with id as key
// fix sized when init
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct idtable_t;
struct idtable_t* idtable_init(int32_t max_count);
int32_t idtable_add(struct idtable_t* table, int32_t id, void* ptr);
void* idtable_get(struct idtable_t* table, int32_t id);
void idtable_remove(struct idtable_t* table, int32_t id);
void idtable_cleanup(struct idtable_t* table);
void idtable_release(struct idtable_t* table);

typedef void (*idtable_free_func)(void*);
void idtable_clean_ex(struct idtable_t* table, idtable_free_func f);
void idtable_release_ex(struct idtable_t* table, idtable_free_func f);

// return 0 means continue loop. otherwise break loop
typedef int32_t (*idtable_callback_func)(void*, void* args);
int32_t idtable_loop(struct idtable_t* table, idtable_callback_func f,
                     void* args, int32_t start_idx);

#ifdef __cplusplus
}
#endif

#endif // IDTABLE_H_


