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
struct idtable_t* idtable_create(int32_t max_count);
int32_t idtable_add(struct idtable_t* table, int32_t id, void* ptr);
void* idtable_get(struct idtable_t* table, int32_t id);
void idtable_remove(struct idtable_t* table, int32_t id);
void idtable_cleanup(struct idtable_t* table);
void idtable_release(struct idtable_t* table);

typedef int (*idtable_loop_func)(void* data, void* arg);
int idtable_loop(struct idtable_t* table, idtable_loop_func func, void* arg, int start);

#ifdef __cplusplus
}
#endif

#endif // IDTABLE_H_


