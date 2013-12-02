#ifndef ATOM_H_
#define ATOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#if defined(OS_WIN)
    typedef LONG atom_t;
#elif defined(OS_LINUX) || defined(OS_MAC)
    typedef uint32_t atom_t;
#endif

atom_t atom_inc(atom_t volatile* a);
atom_t atom_dec(atom_t volatile* a);
atom_t atom_set(atom_t volatile* a, uint32_t val);
atom_t atom_add(atom_t volatile* a, uint32_t val);
atom_t atom_sub(atom_t volatile* a, uint32_t val);

typedef void* atom_ptr_t;
atom_ptr_t atom_ptr_set(atom_ptr_t volatile* ap, void* data);
atom_ptr_t atom_ptr_cas(atom_ptr_t volatile* ap, void* cmp, void* val);

#ifdef __cplusplus
}
#endif

#endif // ATOM_H_

