
#ifndef TAGS_BASE_H
#define TAGS_BASE_H

#include "context/context.h"
#include "gedcom.h"
#include "utils/hashmap.h"

#define TAG_BASE                                                               \
    struct tag_interface* interface;                                           \
    struct context* ctx

struct ged_record; // defined in gedcom.h

struct tag_interface;
struct tag_base {
    TAG_BASE;
};

struct tag_invalid {
    TAG_BASE;
};

typedef struct tag_base* (*fn_create)(struct tag_interface*, struct ged_record*,
                                      struct lex_token*, struct context* ctx);

typedef void (*fn_free)(struct tag_base*);

struct tag_interface {
    fn_create create;
    fn_free free;
};

void tag_base_free(struct tag_base* tag);

struct tag_interface* tag_i_get(const char* key);

void tags_init(void);
void tags_cleanup(void);

#endif // TAGS_BASE_H