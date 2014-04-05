#include <assert.h>
#include <stddef.h>
#include "mm/slab.h"

LIST_HEAD(g_slab_minor);
LIST_HEAD(g_slab_common);

#define META_COLOR_DEFAULT 0x0
#define META_COLOR_ALLOC 0x01

typedef struct meta_t {
    int16_t size;
    int16_t color;

    // saved shift position according to page
    int16_t free_next;
    int16_t free_prev;
} meta_t;

#define META_SIZE SLAB_ALIGN(sizeof(meta_t))
#define META_MEM(meta) ((char*)meta + META_SIZE)
#define META_FROM_MEM(memory) (meta_t*)((char*)memory - META_SIZE)

// getpagesize() must be 2^n
#define META_SHIFT(meta) (int16_t)((uint64_t)meta & (getpagesize() - 1))
#define META_PAGE(meta) (page_t*)((uint64_t)meta & ~(getpagesize() - 1))
#define META(meta, shift) (meta_t*)((char*)META_PAGE(meta) + shift)

typedef struct page_t {
    list_head_t link;
    int16_t free; // first free memory shift
    int16_t remain; // remain free size
} page_t;

#define PAGE_HEAD_SIZE SLAB_ALIGN(sizeof(page_t))
#define PAGE_META(page, shift) (meta_t*)((char*)page + shift)

static list_head_t*
_slab_free_page(size_t sz) {
    list_head_t* head = NULL;
    if (sz <= SLAB_SIZE_MINOR) {
        head = &g_slab_minor;
    } else if (sz <= SLAB_SIZE_MAX) {
        head = &g_slab_common;
    }
    return head;
}

static void
_slab_free_insert(meta_t* meta, meta_t* insert) {
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
static meta_t*
_slab_free_erase(meta_t* meta) {
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

static void
_slab_free_replace(meta_t* meta, meta_t* replace) {
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

// check next meta whether free, if free, then do quick merge
// replace_next == 0 means meta will replace next in free link
// return 0 means do quick merge success
static int
_slab_free_quick_merge(meta_t* meta, int replace_next) {
    meta_t* next, *prev;
    page_t* page = META_PAGE(meta);
    int shift = META_SHIFT(meta) + META_SIZE + meta->size;
    if (shift < getpagesize()) {
        next = META(meta, shift);
        if (!(next->color & META_COLOR_ALLOC)) {
            prev = next->free_prev > 0 ? PAGE_META(page, next->free_prev) : NULL;
            // quick merge with next
            meta->size += next->size + META_SIZE;
            if (replace_next == 0) {
                _slab_free_replace(next, meta);
                if (META_SHIFT(next) == page->free) {
                    page->free = META_SHIFT(meta);
                }
            }
            // quick merge with prev
            if (prev && META_SHIFT(prev) + META_SIZE + prev->size == META_SHIFT(meta)) {
                prev->size += META_SIZE + meta->size;
                prev->free_next = meta->free_next;
                next = meta->free_next > 0 ? PAGE_META(page, meta->free_next) : NULL;
                if (next) {
                    next->free_prev = META_SHIFT(prev);
                }
            }
            return 0;
        }
    }
    return -1;
}

static void
_slab_meta_init(meta_t* meta, size_t sz) {
    if (meta) {
        meta->size = sz;
        meta->color = META_COLOR_DEFAULT;
        meta->free_next = meta->free_prev = -1;
    }
}

// return split meta
static meta_t*
_slab_free_split(meta_t* meta, int16_t used) {
    meta_t* next = NULL;
    assert(meta->size > used + (int)META_SIZE);
    next = (meta_t*)((char*)meta + META_SIZE + used);
    _slab_meta_init(next, meta->size - used - META_SIZE);
    meta->size = used;
    return next;
}

static void
_slab_page_erase(page_t* page, list_head_t* head) {
    // make full threshold as size limit
    // erase from slab link
    if (head == &g_slab_minor) {
        if (page->remain < SLAB_SIZE_MINOR) {
            list_del(&page->link);
        }
    } else if (head == &g_slab_common) {
        if (page->remain < SLAB_SIZE_MAX) {
            list_del(&page->link);
        }
    }
}

static void
_slab_page_insert(page_t* page, meta_t* meta) {
    list_head_t* head;
    head = _slab_free_page(meta->size);
    // page not full, then add list
    if (head == &g_slab_minor && list_not_in_link(&page->link)) {
        list_add(&page->link, head);
    } else if (head == &g_slab_common && list_not_in_link(&page->link)) {
        list_add(&page->link, head);
    }
}

static void*
_slab_alloc(page_t* page, list_head_t* head, size_t sz) {
    int16_t shift;
    meta_t* meta, *next, *split;
    // no free memory
    if (page->free <= 0 || page->remain < (int)sz + (int)META_SIZE) {
        return NULL;
    }
    // loop free memory
    shift = page->free;
    while (shift > 0) {
        meta = PAGE_META(page, shift);
        shift = meta->free_next;
        // not enough, ignore
        if (meta->size < (int)sz) {
            meta = NULL;
            continue;
        }
        // just fit, erase from free list
        else if (meta->size <= (int)(sz + META_SIZE)) {
            next = _slab_free_erase(meta);
            // erase free-link head
            if (META_SHIFT(meta) == page->free) {
                page->free = next ? META_SHIFT(next) : -1;
            }
            meta->color |= META_COLOR_ALLOC;
            break;
        }
        // do split
        else {
            split = _slab_free_split(meta, sz);
            _slab_free_replace(meta, split);
            if (META_SHIFT(meta) == page->free) {
                page->free = META_SHIFT(split);
            }
            meta->color |= META_COLOR_ALLOC;
            break;
        }
    }
    if (meta) {
        page->remain -= (META_SIZE + meta->size);
        _slab_page_erase(page, head);
        return META_MEM(meta);
    }
    return NULL;
}

void*
slab_alloc(size_t sz) {
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
        memory = _slab_alloc(page, head, sz);
        if (memory) return memory;
    }
    // now, we need alloc a new page
    page = NULL;
#if defined (OS_LINUX) || defined (OS_MAC)
    posix_memalign((void**)&page, getpagesize(), getpagesize());
#elif defined (OS_WIN)
    page = _aligned_malloc(getpagesize(), getpagesize());
#endif
    if (!page) return NULL;
    page->free = PAGE_HEAD_SIZE;
    page->remain = (int16_t)getpagesize() - page->free;
    meta = PAGE_META(page, PAGE_HEAD_SIZE);
    _slab_meta_init(meta, getpagesize() - PAGE_HEAD_SIZE - META_SIZE);
    // add add to link list
    list_add(&page->link, head);
    // recursive
    return _slab_alloc(page, head, sz);
}

void
slab_free(void* memory) {
    meta_t* meta, *next, *prev;
    page_t* page;
    size_t shift, next_shift;
    if (!memory) return;
    meta = META_FROM_MEM(memory);
    if (!(meta->color & META_COLOR_ALLOC)) {
        assert(0);
    }
    meta->color &= (~META_COLOR_ALLOC);
    page = META_PAGE(meta);
    _slab_page_insert(page, meta);
    page->remain += meta->size + META_SIZE;
    // try quick merge with next one
    if (_slab_free_quick_merge(meta, 0) == 0) {
        goto FREE_SUCCESS;
    }
    shift = META_SHIFT(meta);
    // if no free head
    if (page->free < 0) {
        meta->free_prev = -1;
        meta->free_next = -1;
        page->free = shift;
        goto FREE_SUCCESS;
    }
    // insert from head
    next_shift = page->free;
    next = PAGE_META(page, next_shift);
    if (shift < next_shift) {
        meta->free_prev = -1;
        meta->free_next = next_shift;
        next->free_prev = shift;
        page->free = shift;
        goto FREE_SUCCESS;
    }
    // loop page free list
    while (1) {
        prev = next;
        if (prev->free_next < 0) {
            break;
        }
        next = PAGE_META(page, prev->free_next);
        if ((int)shift < prev->free_next) {
            break;
        }
    }
    // if cant merge, then do link after after
    if (_slab_free_quick_merge(prev, -1)) {
        _slab_free_insert(prev, meta);
    }
FREE_SUCCESS:
    // free page
    if (page->remain + PAGE_HEAD_SIZE == getpagesize()) {
        list_del(&page->link);
#if defined(OS_WIN)
		_aligned_free((void*)page);
#else
        free((void*)page);
#endif
    }
}

static void
_page_debug(page_t* page) {
    meta_t* meta;
    int shift;
    if (page) {
        printf("--> page %tX, remain %d: ", (ptrdiff_t)page, page->remain);
        shift = page->free;
        if (shift != PAGE_HEAD_SIZE) {
            printf("[%d:%d] ", (int)PAGE_HEAD_SIZE, shift);
        }
        while (shift > 0) {
            meta = PAGE_META(page, shift);
            printf("(%d:%d) ", shift, shift + (int)META_SIZE + (int)(meta->size));
            if (meta->free_next > 0) {
                printf("[%d:%d] ", shift + (int)(META_SIZE) + (int)(meta->size),
                    (int)(meta->free_next));
            }
            shift = meta->free_next;
        }
        printf("\n");
    }
}

void
slab_debug() {
    page_t* page;
    printf("minor slab: \n");
    list_for_each_entry(page, page_t, &g_slab_minor, link) {
        _page_debug(page);
    }
    printf("common slab: \n");
    list_for_each_entry(page, page_t, &g_slab_common, link) {
        _page_debug(page);
    }
    printf("slab debug complete!\n\n");
}

