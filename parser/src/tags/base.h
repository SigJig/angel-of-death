
#ifndef TAGS_BASE_H
#define TAGS_BASE_H

#include "context/context.h"
#include "gedcom.h"
#include "utils/hashmap.h"
#include "utils/ptrarr.h"

struct tag_interface {
    void* const (*create)(struct ged_record*);
    void (*free)(void* data);
};

const struct tag_interface* tag_i_get(const char* key);

void tags_init(void);
void tags_cleanup(void);

#endif // TAGS_BASE_H