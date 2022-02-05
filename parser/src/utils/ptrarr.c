
#include "utils/ptrarr.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MULT_FACTOR 2

struct ptr_arr {
    void* mem;
    size_t len;
    size_t cap;
};

static void**
pa_get_unsafe(struct ptr_arr* pa, size_t index)
{
    // cast because void* does not support pointer arithmetic
    return (void**)((char*)pa->mem + (index * sizeof(void*)));
}

struct ptr_arr*
pa_create(size_t cap)
{
    if (!cap) {
        assert(false /* Dyn array invalid value */);
        return NULL;
    }

    struct ptr_arr* pa = malloc(sizeof *pa);

    if (!pa) {
        return NULL;
    }

    pa->len = 0;
    pa->cap = cap;

    pa->mem = malloc(pa->cap * (sizeof(void*)));

    if (!pa->mem) {
        pa_free(pa);
        return NULL;
    }

    return pa;
}

void
pa_free(struct ptr_arr* pa)
{
    if (!pa) {
        return;
    }

    if (pa->mem) {
        free(pa->mem);
    }

    free(pa);
}

size_t
pa_len(struct ptr_arr* pa)
{
    return pa->len;
}

size_t
pa_cap(struct ptr_arr* pa)
{
    return pa->cap;
}

void**
pa_reserve(struct ptr_arr* pa)
{
    pa->len++;

    if (pa->len > pa->cap) {
        int mult_factor =
            MULT_FACTOR * ((pa->len / pa->cap) + ((pa->len % pa->cap) != 0));

        assert(mult_factor);
        pa->cap *= mult_factor;
        pa->mem = realloc(pa->mem, pa->cap * (sizeof(void*)));

        if (!pa->mem)
            return NULL;
    }

    return pa_get_unsafe(pa, pa->len - 1);
}

void*
pa_get(struct ptr_arr* pa, size_t index)
{
    if (index >= pa->len)
        return NULL;

    return *pa_get_unsafe(pa, index);
}

void*
pa_front(struct ptr_arr* pa)
{
    return pa_get(pa, 0);
}

void*
pa_back(struct ptr_arr* pa)
{
    if (!pa->len) {
        return NULL;
    }

    return pa_get(pa, pa->len - 1);
}

// since reserve returns unitialized data, it is expected to initialize after
// reserve has been called therefore we dont need to clear the memory, as it
// will either be freed upon destruction or updated when the slot is reused
void*
pa_pop(struct ptr_arr* pa)
{
    if (pa->len <= 0)
        return NULL;

    // pa->len--;

    return *pa_get_unsafe(pa, --pa->len);
}

e_statuscode
pa_push(struct ptr_arr* pa, void* data)
{
    void** mem = pa_reserve(pa);

    if (!mem) {
        assert(false);

        return ST_MALLOC_ERROR;
    }

    *mem = data;

    return ST_OK;
}