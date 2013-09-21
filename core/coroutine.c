#ifdef OS_LINUX

#include <ucontext.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include "coroutine.h"

#define CRT_STACK_SIZE (1 << 20)
#define CRT_DEFAULT_SIZE 16

struct crt_unit_t;
struct crt_t {
    char stack[CRT_STACK_SIZE];
    ucontext_t main;
    int current;
    int capacity;
    int count;
    struct crt_unit_t** units;
};

struct crt_unit_t {
    crt_func_t func;
    void* arg;
    ucontext_t ctx;
    struct crt_t* crt;
    ptrdiff_t capacity;
    ptrdiff_t size;
    int status;
    char* stack;
};

struct crt_unit_t* _crt_unit_init(struct crt_t* c, crt_func_t func, void* arg)
{
    struct crt_unit_t* cu = (struct crt_unit_t*)MALLOC(sizeof(*cu));
    assert(cu);
    cu->func = func;
    cu->arg = arg;
    cu->crt = c;
    cu->capacity = 0;
    cu->size = 0;
    cu->status = CRT_INIT;
    cu->stack = 0;
    return cu;
}

void _crt_unit_release(struct crt_unit_t* cu)
{
    if (cu) {
        if (cu->stack)
            FREE(cu->stack);
        FREE(cu);
    }
}

struct crt_t* crt_init()
{
    struct crt_t* c = (struct crt_t*)MALLOC(sizeof(*c));
    assert(c);
    c->count = 0;
    c->capacity = CRT_DEFAULT_SIZE;
    c->current = -1;
    c->units = (struct crt_unit_t**)MALLOC(sizeof(struct crt_unit_t*) * c->capacity);
    assert(c->units);
    memset(c->units, 0, sizeof(struct crt_unit_t*) * c->capacity);
    return c;
}

void crt_release(struct crt_t* c)
{
    if (c) {
        int i;
        for (i = 0; i < c->capacity; ++ i) {
            struct crt_unit_t* cu = c->units[i];
            if (cu) _crt_unit_release(cu);
        }
        FREE(c->units);
        c->units = 0;
        FREE(c);
    }
}

int crt_new(struct crt_t* c, crt_func_t func, void* arg)
{
    struct crt_unit_t* cu = _crt_unit_init(c, func, arg);
    if (c->count >= c->capacity) {
        int id = c->capacity;
        c->units = (struct crt_unit_t**)realloc(c->units,
            c->capacity * 2 * sizeof(struct crt_unit_t*));
        assert(c->units);
        memset(c->units + c->capacity, 0, c->capacity * sizeof(struct crt_unit_t*));
        c->units[c->capacity] = cu;
        c->capacity *= 2;
        ++ c->count;
        return id;
    }

    int i;
    for (i = 0; i < c->capacity; ++ i) {
        int id = (i + c->count) % c->capacity;
        if (c->units[id] == NULL) {
            c->units[id] = cu;
            ++ c->count;
            return id;
        }
    }
    assert(0);
    return CRT_INVALID_ID;
}

// this is coroutine running context: call register function
static void _crt_main(uint32_t low32, uint32_t hi32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    struct crt_t* c = (struct crt_t*)ptr;
    int id = c->current;
    assert(id >= 0 && id < c->capacity);
    struct crt_unit_t* cu = c->units[id];
    cu->func(c, cu->arg);

    _crt_unit_release(cu);
    c->units[id] = NULL;
    -- c->count;
    c->current = CRT_INVALID_ID;
}

void crt_resume(struct crt_t* c, int id)
{
    assert(c && c->current < 0 && id >= 0 && id < c->capacity);
    struct crt_unit_t* cu = c->units[id];
    if (!cu) return;

    switch (cu->status) {
        case CRT_INIT:
            getcontext(&cu->ctx);
            cu->ctx.uc_stack.ss_sp = c->stack;
            cu->ctx.uc_stack.ss_size = CRT_STACK_SIZE;
            cu->ctx.uc_link = &c->main;
            cu->status = CRT_RUNNING;
            c->current = id;
            uintptr_t ptr = (uintptr_t)c;
            makecontext(&cu->ctx, (void (*)(void))_crt_main, 2, (uint32_t)ptr,
                 (uint32_t)(ptr >> 32));
            swapcontext(&c->main, &cu->ctx);
            break;

        case CRT_SUSPEND:
            // copy back main stack (now allocated by heap)
            // as stack grows top-down, copy stack from top(+ CRT_STACK_SIZE)
            memcpy(c->stack + CRT_STACK_SIZE - cu->size, cu->stack, cu->size);
            c->current = id;
            cu->status = CRT_SUSPEND;
            swapcontext(&c->main, &cu->ctx);
            break;

        default:
            assert(0);
    }
}

//      stack stored in heap(ucontext_t.uc_stack.ss_sp)
//
//      -----------------------
//      |                     |
//      |                     |
//      -----------------------  <-- stack top, c->stack + CRT_STACK_SIZE
//      |                     |
//      |  used(need to save) |
//      |                     |
//      -----------------------  <-- &dummy, current stack position
//      |                     |
//      |                     |
//      -----------------------  <-- stack bottom, c->stack
//
void _crt_save_stack(struct crt_unit_t* cu, char* top)
{
    char dummy = 0;
    if (cu->capacity < top - &dummy) {
        FREE(cu->stack);
        cu->capacity = top - &dummy;
        cu->stack = MALLOC(cu->capacity);
    }
    cu->size = top - &dummy;
    memcpy(cu->stack, &dummy, cu->size);
}

void crt_yield(struct crt_t* c)
{
    assert(c);
    int id = c->current;
    assert(id >= 0);
    struct crt_unit_t* cu = c->units[id];

    // &cu: it's an address on current stack
    // make sure current stack still above c->stack(in CRT_STACK_SIZE range)
    assert((char*)&cu > c->stack);

    // save main's stack to cu
    // save from stack top
    _crt_save_stack(cu, c->stack + CRT_STACK_SIZE);
    cu->status = CRT_SUSPEND;
    c->current = -1;
    swapcontext(&cu->ctx, &c->main);
}

int crt_status(struct crt_t* c, int id)
{
    assert(id >= 0 && id < c->capacity);
    if (c->units[id] == NULL)
        return CRT_DEAD;
    return c->units[id]->status;
}

int crt_current(struct crt_t* c)
{
    assert(c);
    return c->current;
}

#endif
