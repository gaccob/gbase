#ifndef IDTABLE_H_
#define IDTABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"

struct idtable_t;
struct idtable_t* idtable_init(int32_t max_count);
int32_t idtable_add(struct idtable_t* table, int32_t id, void* ptr);
void* idtable_get(struct idtable_t* table, int32_t id);
void idtable_remove(struct idtable_t* table, int32_t id);
void idtable_cleanup(struct idtable_t* table);
void idtable_release(struct idtable_t* table);

#ifdef __cplusplus
}
#endif

#endif // IDTABLE_H_


