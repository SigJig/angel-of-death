
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include "utils/statuscode.h"
#include <stdlib.h>

struct dyn_array {
    void* mem;
    size_t len;
    size_t cap;
};

struct dyn_array* da_create(size_t cap);
void da_free(struct dyn_array* da);

void** da_reserve(struct dyn_array* da);
void* da_get(struct dyn_array* da, size_t index);
void* da_front(struct dyn_array* da);
void* da_back(struct dyn_array* da);
void* da_pop(struct dyn_array* da);
e_statuscode da_push(struct dyn_array* da, void* data);

#endif // DYNARRAY_H