#include <assert.h>
#include "core/list.h"
#include "mm/slab.h"

LIST_HEAD(g_slab_small);
LIST_HEAD(g_slab_medium);
LIST_HEAD(g_slab_large);

#define META_COLOR_DEFAULT 0x0
#define META_COLOR_ALLOC 0x01

typedef struct meta_t
{
    int16_t size;
    int16_t color;

    // saved shift position according to page
    int16_t free_next;
    int16_t free_prev;
} meta_t;

#define META_SIZE SLAB_ALIGN(sizeof(meta_t))
#define META_MEM(meta) ((char*)meta + META_SIZE)

// getpagesize() must be 2^n
#define META_SHIFT(meta) (int16_t)((uint64_t)meta & (getpagesize() - 1))
#define META_PAGE(meta) (page_t*)((uint64_t)meta & ~(getpagesize() - 1))
#define META(meta, shift) (meta_t*)((char*)META_PAGE(meta) + shift)

typedef struct page_t
{
    list_head_t link;
    int16_t free; // first free memory shift
} page_t;

#define PAGE_HEAD_SIZE SLAB_ALIGN(sizeof(page_t))
#define PAGE_META(page, shift) (meta_t*)((char*)page + shift)

list_head_t* _slab_free_page(size_t sz)
{
    list_head_t* head = NULL;
    if (sz <= SLAB_SIZE_SMALL) {
        head = &g_slab_small;
    } else if (sz <= SLAB_SIZE_MEDIUM) {
        head = &g_slab_medium;
    } else if (sz <= getpagesize() - META_SIZE - PAGE_HEAD_SIZE) {
        head = &g_slab_large;
    }
    return head;
}

void _slab_free_insert(meta_t* meta, meta_t* insert)
{
    meta_t* next;
    if (!meta || !insert) return;
    if (meta->free_next > 0) {
        next = META(meta, meta->free_next);
        insert->free_next = META_SHIFT(next);
        next->free_prev = META_SHIFT(insert);
    }
    insert->free_prev = META_SHIFT(meta);
    meta->free_next = META_SHIFT(insert);
}

// return next meta, if any
meta_t* _slab_free_erase(meta_t* meta)
{
    meta_t *next, *prev;
    if (!meta) {
        return NULL;
    }

    // prev and next free link
    if (meta->free_prev > 0) {
        prev = META(meta, meta->free_prev);
    } else {
        prev = NULL;
    }
    if (meta->free_next > 0) {
        next = META(meta, meta->free_next);
    } else {
        next = NULL;
    }

    // relink
    if (next) {
        next->free_prev = prev ? META_SHIFT(prev) : -1;
    }
    if (prev) {
        prev->free_next = next ? META_SHIFT(next) : -1;
    }

    // erase self link
    meta->free_prev = -1;
    meta->free_next = -1;
    return next;
}

void _slab_free_replace(meta_t* meta, meta_t* replace)
{
    meta_t *next, *prev;
    if (!meta || !replace) { return; }

    if (meta->free_prev > 0) {
        prev = META(meta, meta->free_prev);
        prev->free_next = META_SHIFT(replace);
        replace->free_prev = META_SHIFT(prev);
    } else {
        replace->free_prev = -1;
    }

    if (meta->free_next > 0) {
        next = META(meta, meta->free_next);
        next->free_prev = META_SHIFT(replace);
        replace->free_next = META_SHIFT(next);
    } else {
        replace->free_next = -1;
    }
}

void _slab_meta_init(meta_t* meta, size_t sz)
{
    if (meta) {
        meta->size = sz;
        meta->color = META_COLOR_DEFAULT;
        meta->free_next = meta->free_prev = -1;
    }
}

// return split meta
meta_t* _slab_free_split(meta_t* meta, int16_t used)
{
    meta_t* next = NULL;
    assert(meta->size > used + META_SIZE);
    next = (meta_t*)((char*)meta + META_SIZE + used);
    _slab_meta_init(next, meta->size - used - META_SIZE);
    meta->size = used;
    return meta;
}

void* _slab_alloc(page_t* page, size_t sz)
{
    int16_t shift;
    meta_t* meta, *next, *split;

    // no free memory
    if (page->free <= 0) {
        return NULL;
    }

    // loop free memory
    shift = page->free;
    while (shift > 0) {
        meta = PAGE_META(page, shift);
        shift = meta->free_next;

        // not enough, ignore
        if (meta->size < sz) {
            continue;
        }

        // just fit, erase from free list
        else if (meta->size <= sz + META_SIZE) {
            next = _slab_free_erase(meta);
            // erase free-link head
            if (META_SHIFT(meta) == page->free) {
                page->free = next ? -1 : META_SHIFT(next);
            }
            meta->color |= META_COLOR_ALLOC;
            return META_MEM(meta);
        }

        // do split
        else {
            split = _slab_free_split(meta, sz);
            _slab_free_replace(meta, split);

            if (META_SHIFT(meta) == page->free) {
                page->free = META_SHIFT(split);
            }
            meta->color |= META_COLOR_ALLOC;
            return META_MEM(meta);
        }
    }
    return NULL;
}

void* slab_alloc(size_t sz)
{
    meta_t* meta;
    page_t* page;
    list_head_t* head;
    void* memory;

    if (sz == 0) return NULL;
    sz = SLAB_ALIGN(sz);

    // try different list, if exceeds max return null
    head = _slab_free_page(sz);
    if (!head) return NULL;

    // alloc from free page
    list_for_each_entry(page, page_t, head, link) {
        memory = _slab_alloc(page, sz);
        if (memory) return memory;
    }

    // now, we need alloc a new page
    page = (page_t*)malloc(getpagesize());
    if (!page) return NULL;
    page->free = PAGE_HEAD_SIZE;
    meta = PAGE_META(page, PAGE_HEAD_SIZE);
    _slab_meta_init(meta, getpagesize() - PAGE_HEAD_SIZE - META_SIZE);

    // add add to link list
    list_add(&page->link, head);

    // recursive
    return _slab_alloc(page, sz);
}

void slab_free(void* memory)
{
    // 1. check merge with last one


    // 2. loop page link
    //
    // 2.1 merge with prev
    //
    // 2.2 seperate one, add to link

    // TODO:
}

