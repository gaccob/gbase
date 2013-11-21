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

struct idtable_iterator_t;
// return null when idtable empty
struct idtable_iterator_t* idtable_iterator_init(struct idtable_t*, int start_idx);
// return < 0 when end-loop
int32_t idtable_iterator_next(struct idtable_iterator_t*);
void* idtable_iterator_value(struct idtable_iterator_t*);
int32_t idtable_iterator_id(struct idtable_iterator_t*);
void idtable_iterator_release(struct idtable_iterator_t*);

#ifdef __cplusplus
}
#endif

#endif // IDTABLE_H_


