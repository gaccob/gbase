#include "atom.h"

// gcc > 4.1.2
#if (GCC_VERSION < 40102)
    #error linux or mac but gcc version less than 4.1.2
#endif

atom_t
atom_inc(atom_t volatile* a) {
    return __sync_add_and_fetch(a, 1);
}

atom_t
atom_dec(atom_t volatile* a) {
    return __sync_sub_and_fetch(a, 1);
}

// return old value
atom_t
atom_set(atom_t volatile* a, uint32_t val) {
    return __sync_lock_test_and_set(a, val, val);
}

atom_t
atom_add(atom_t volatile* a, uint32_t val) {
    return __sync_add_and_fetch(a, val);
}

atom_t
atom_sub(atom_t volatile* a, uint32_t val) {
    return __sync_sub_and_fetch(a, val);
}

atom_ptr_t
atom_ptr_set(atom_ptr_t volatile* ap, void* data) {
    return (atom_ptr_t)__sync_val_compare_and_swap(&ap, (atom_ptr_t)data, (atom_ptr_t)data);
}

atom_ptr_t
atom_ptr_cas(atom_ptr_t volatile* ap, void* cmp, void* val) {
    return (atom_ptr_t)__sync_val_compare_and_swap(&ap, (atom_ptr_t)cmp, (atom_ptr_t)val);
}

