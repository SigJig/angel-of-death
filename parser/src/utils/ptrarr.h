
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include "utils/statuscode.h"
#include <stdlib.h>

struct ptr_arr;
typedef struct ptr_arr* ptr_arr;

ptr_arr pa_create(size_t cap);
void pa_free(ptr_arr pa);

size_t pa_len(ptr_arr pa);
size_t pa_cap(ptr_arr pa);

void** pa_reserve(ptr_arr pa);
void* pa_get(ptr_arr pa, size_t index);
void* pa_front(ptr_arr pa);
void* pa_back(ptr_arr pa);
void* pa_pop(ptr_arr pa);
e_statuscode pa_push(ptr_arr pa, void* data);

#endif // DYNARRAY_H