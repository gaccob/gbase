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

typedef struct idtable_t idtable_t;
typedef int (*idtable_loop_func)(void* data, void* arg);

idtable_t* idtable_create(int max_count);
int idtable_add(idtable_t* table, int id, void* ptr);
void* idtable_get(idtable_t* table, int id);
void idtable_remove(idtable_t* table, int id);
void idtable_cleanup(idtable_t* table);
void idtable_release(idtable_t* table);
int idtable_loop(idtable_t* table, idtable_loop_func func, void* arg, int start);

#ifdef __cplusplus
}
#endif

#endif // IDTABLE_H_


