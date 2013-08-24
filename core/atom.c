#include "atom.h"

#if defined(OS_WIN)
    #if !defined(_MSC_VER)
    #error  windows but not MSC
    #endif
#elif defined(OS_LINUX) || defined(OS_MAC)
    /* from 4.1.2 */
    #if (GCC_VERSION < 40102)
    #error linux or mac but gcc version less than 4.1.2
    #endif
#endif

atom_t atom_inc(atom_t volatile* a)
{
#if defined(OS_WIN)
    return InterlockedIncrement(a);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return __sync_add_and_fetch(a, 1);
#endif
}

atom_t atom_dec(atom_t volatile* a)
{
#if defined(OS_WIN)
    return InterlockedDecrement(a);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return __sync_sub_and_fetch(a, 1);
#endif
}

/* return old value */
atom_t atom_set(atom_t volatile* a, uint32_t val)
{
#if defined(OS_WIN)
    return InterlockedCompareExchange(a, val, val);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return __sync_val_compare_and_swap(a, val, val);
#endif
}

atom_t atom_add(atom_t volatile* a, uint32_t val)
{
#if defined(OS_WIN)
    return InterlockedExchangeAdd(a, val);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return __sync_add_and_fetch(a, val);
#endif
}

atom_t atom_sub(atom_t volatile* a, uint32_t val)
{
#if defined(OS_WIN)
    LONG delta = - ((LONG) val);
    return InterlockedExchangeAdd(a, delta);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return __sync_sub_and_fetch(a, val);
#endif
}

atom_ptr_t atom_ptr_set(atom_ptr_t volatile* ap, void* data)
{
#if defined(OS_WIN)
    return (atom_ptr_t)InterlockedCompareExchangePointer((volatile PVOID*)&ap, (atom_ptr_t)data, (atom_ptr_t)data);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return (atom_ptr_t)__sync_val_compare_and_swap(&ap, (atom_ptr_t)data, (atom_ptr_t)data);
#endif
}

atom_ptr_t atom_ptr_cas(atom_ptr_t volatile* ap, void* cmp, void* val)
{
#if defined(OS_WIN)
    return (atom_ptr_t)InterlockedCompareExchangePointer((volatile PVOID*)&ap, (atom_ptr_t)val, (atom_ptr_t)cmp);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return (atom_ptr_t)__sync_val_compare_and_swap(&ap, (atom_ptr_t)cmp, (atom_ptr_t)val);
#endif
}


