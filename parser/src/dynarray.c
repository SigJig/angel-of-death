
#include "dynarray.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MULT_FACTOR 2

static void*
da_get_unsafe(struct dyn_array* da, size_t index)
{
    // cast because void* does not support pointer arithmetic
    return ((char*)da->mem) + (index * da->byte_n);
}

struct dyn_array*
da_create(size_t cap, size_t byte_n)
{
    if (!cap || !byte_n) {
        assert(false /* Dyn array invalid values */);
        return NULL;
    }

    struct dyn_array* da = malloc(sizeof *da);

    if (!da) {
        return NULL;
    }

    da->len = 0;
    da->cap = cap;
    da->byte_n = byte_n;

    da->mem = malloc(da->cap * da->byte_n);

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
        da->mem = realloc(da->mem, da->cap * da->byte_n);

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

    return da_get_unsafe(da, index);
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

    return da_get_unsafe(da, --da->len);
}
