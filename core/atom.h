#ifndef ATOM_H_
#define ATOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef uint32_t atom_t;

atom_t atom_inc(atom_t volatile*);
atom_t atom_dec(atom_t volatile*);
atom_t atom_set(atom_t volatile*, uint32_t val);
atom_t atom_add(atom_t volatile*, uint32_t val);
atom_t atom_sub(atom_t volatile*, uint32_t val);

typedef void* atom_ptr_t;
atom_ptr_t atom_ptr_set(atom_ptr_t volatile*, void* data);
atom_ptr_t atom_ptr_cas(atom_ptr_t volatile*, void* cmp, void* val);

#ifdef __cplusplus
}
#endif

#endif // ATOM_H_

