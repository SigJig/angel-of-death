
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdlib.h>

struct dyn_array {
	void *mem;
	size_t len;
	size_t cap;
	size_t byte_n;
};

struct dyn_array *da_create(size_t cap, size_t byte_n);
void da_free(struct dyn_array *da);

void *da_reserve(struct dyn_array *da);
void *da_get(struct dyn_array *da, size_t index);
void *da_pop(struct dyn_array *da);

#endif // DYNARRAY_H