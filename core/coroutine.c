#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>

#include "base/slist.h"
#include "coroutine.h"

#if defined(OS_LINUX)
#include <ucontext.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <malloc.h>

// reserved stack size for pretecting stack overflow
// it must be round up by pagesize
#define CRT_UNIT_RESERVED_SIZE getpagesize()

// default coroutine units size
#define CRT_DEFAULT_SIZE 16

struct crt_unit_t;

typedef struct crt_t {
    ucontext_t main;
    int stack_size;
    int current;
    int count;
    int capacity;
    struct crt_unit_t** units;
    struct slist_t* freelst;
} crt_t;

typedef struct crt_unit_t {
    crt_func_t func;
    void* arg;
    ucontext_t ctx;
    crt_t* crt;
    int status;
    char* stack;
} crt_unit_t;

static crt_unit_t*
_crt_unit_create(crt_t* c, crt_func_t func, void* arg) {
    int ret;
    crt_unit_t* cu = (crt_unit_t*)MALLOC(sizeof(*cu));
    assert(cu);
    cu->func = func;
    cu->arg = arg;
    cu->crt = c;
    cu->status = CRT_INIT;
    cu->stack = (char*)VALLOC(c->stack_size + CRT_UNIT_RESERVED_SIZE);
    // printf("alloc %p\n", cu->stack);
    assert(cu->stack);
    // protect reserved space, top done, so resverd at front
    ret = mprotect(cu->stack, CRT_UNIT_RESERVED_SIZE, PROT_NONE);
    cu->stack = cu->stack + CRT_UNIT_RESERVED_SIZE;
    assert(0 == ret);
    return cu;
}

static void
_crt_unit_release(crt_unit_t* cu) {
    if (cu) {
        if (cu->stack) {
            cu->stack -= CRT_UNIT_RESERVED_SIZE;
            // printf("free %p\n", cu->stack);
            FREE(cu->stack);
            cu->stack = 0;
        }
        FREE(cu);
    }
}

crt_t*
crt_create(int crt_stack_size) {
    assert(crt_stack_size > 0);
    crt_t* c = (crt_t*)MALLOC(sizeof(*c));
    assert(c);
    c->count = 0;
    c->capacity = CRT_DEFAULT_SIZE;
    c->current = -1;
    // stack, round up 4k
    c->stack_size = ((crt_stack_size - 1) / getpagesize() + 1) * getpagesize();
    // printf("coroutine stack size %d\n", c->stack_size);
    // coroutine units
    c->units = (crt_unit_t**)MALLOC(sizeof(crt_unit_t*) * c->capacity);
    assert(c->units);
    memset(c->units, 0, sizeof(crt_unit_t*) * c->capacity);
    c->freelst = slist_create();
    assert(c->freelst);
    return c;
}

void
crt_release(crt_t* c) {
    int i;
    crt_unit_t* cu;
    void* vcu;
    if (c) {
        // release running coroutines
        for (i = 0; i < c->capacity; ++ i) {
            cu = c->units[i];
            if (cu) _crt_unit_release(cu);
        }
        FREE(c->units);
        c->units = 0;

        // release free list
        while ((vcu = slist_pop_front(c->freelst))) {
            _crt_unit_release((crt_unit_t*)vcu);
        }
        slist_release(c->freelst);
        c->freelst = NULL;

        FREE(c);
    }
}

int
crt_new(crt_t* c, crt_func_t func, void* arg) {
    int i, id;
    struct crt_unit_t* cu = slist_pop_back(c->freelst);
    if (!cu) {
        cu = _crt_unit_create(c, func, arg);
    } else {
        cu->status = CRT_INIT;
    }
    assert(cu);

    // expand crt
    if (c->count >= c->capacity) {
        id = c->capacity;

        // coroutine units
        c->units = (crt_unit_t**)REALLOC(c->units,
            c->capacity * 2 * sizeof(crt_unit_t*));
        assert(c->units);
        memset(c->units + c->capacity, 0, c->capacity * sizeof(crt_unit_t*));
        c->units[c->capacity] = cu;

        // capacity
        c->capacity *= 2;
        ++ c->count;
        return id;
    }

    // pre-allocted coroutine unit
    for (i = 0; i < c->capacity; ++ i) {
        id = (i + c->count) % c->capacity;
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
static void
_crt_main(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    crt_t* c = (crt_t*)ptr;
    int id = c->current;
    assert(id >= 0 && id < c->capacity);
    crt_unit_t* cu = c->units[id];
    cu->func(c, cu->arg);

    // save to freelst
    slist_push_back(c->freelst, cu);
    c->units[id] = NULL;
    -- c->count;
    c->current = CRT_INVALID_ID;
}

void
crt_resume(crt_t* c, int id) {
    assert(c && c->current < 0 && id >= 0 && id < c->capacity);
    crt_unit_t* cu = c->units[id];
    if (!cu) return;
    switch (cu->status) {
        case CRT_INIT:
            getcontext(&cu->ctx);
            cu->ctx.uc_stack.ss_sp = cu->stack;
            cu->ctx.uc_stack.ss_size = c->stack_size;
            cu->ctx.uc_link = &c->main;
            cu->status = CRT_RUNNING;
            c->current = id;
            uintptr_t ptr = (uintptr_t)c;
            makecontext(&cu->ctx, (void (*)(void))_crt_main, 2, (uint32_t)ptr,
                 (uint32_t)(ptr >> 32));
            swapcontext(&c->main, &cu->ctx);
            break;

        case CRT_SUSPEND:
            c->current = id;
            cu->status = CRT_RUNNING;
            swapcontext(&c->main, &cu->ctx);
            break;

        default:
            assert(0);
    }
}

void
crt_yield(crt_t* c) {
    assert(c);
    int id = c->current;
    assert(id >= 0);
    crt_unit_t* cu = c->units[id];
    cu->status = CRT_SUSPEND;
    c->current = -1;
    swapcontext(&cu->ctx, &c->main);
}

int
crt_status(crt_t* c, int id) {
    assert(id >= 0 && id < c->capacity);
    if (c->units[id] == NULL)
        return CRT_DEAD;
    return c->units[id]->status;
}

int
crt_current(crt_t* c) {
    assert(c);
    return c->current;
}

char*
crt_current_stack_top(crt_t* c) {
    assert(c);
    if (c->current >= 0) {
        return c->units[c->current]->stack + c->stack_size;
    }
    return NULL;
}

#endif

