
#include "utils/dynarray.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MULT_FACTOR 2

static void*
da_get_unsafe(struct dyn_array* da, size_t index)
{
    // cast because void* does not support pointer arithmetic
    return (char*)da->mem + (index * sizeof(void*));
}

struct dyn_array*
da_create(size_t cap)
{
    if (!cap) {
        assert(false /* Dyn array invalid value */);
        return NULL;
    }

    struct dyn_array* da = malloc(sizeof *da);

    if (!da) {
        return NULL;
    }

    da->len = 0;
    da->cap = cap;

    da->mem = malloc(da->cap * (sizeof(void*)));

    if (!da->mem) {
        da_free(da);
        return NULL;
    }

    return da;
}

void
da_free(struct dyn_array* da)
{
    if (da->mem)
        free(da->mem);
    if (da)
        free(da);
}

void*
da_reserve(struct dyn_array* da)
{
    da->len++;

    if (da->len > da->cap) {
        int mult_factor =
            MULT_FACTOR * ((da->len / da->cap) + ((da->len % da->cap) != 0));

        assert(mult_factor);
        da->cap *= mult_factor;
        da->mem = realloc(da->mem, da->cap * (sizeof(void*)));

        if (!da->mem)
            return NULL;
    }

    return da_get_unsafe(da, da->len - 1);
}

void*
da_get(struct dyn_array* da, size_t index)
{
    if (index >= da->len)
        return NULL;

    return *(void**)da_get_unsafe(da, index);
}

void*
da_front(struct dyn_array* da)
{
    return da_get(da, 0);
}

void*
da_back(struct dyn_array* da)
{
    if (!da->len) {
        return NULL;
    }

    return da_get(da, da->len - 1);
}

// since reserve returns unitialized data, it is expected to initialize after
// reserve has been called therefore we dont need to clear the memory, as it
// will either be freed upon destruction or updated when the slot is reused
void*
da_pop(struct dyn_array* da)
{
    if (da->len <= 0)
        return NULL;

    // da->len--;

    return *(void**)da_get_unsafe(da, --da->len);
}

e_statuscode
da_push(struct dyn_array* da, void* data)
{
    void** mem = (void**)da_reserve(da);

    if (!mem) {
        assert(false);

        return ST_MALLOC_ERROR;
    }

    *mem = data;

    return ST_OK;
}