
#ifndef TAGS_BASE_H
#define TAGS_BASE_H

#include "context/context.h"
#include "gedcom.h"
#include "utils/hashmap.h"

struct ged_record; // defined in gedcom.h

struct tag_interface;
struct tag {
    const struct tag_interface* interface;
    struct context* ctx;
    void* const data;
};

struct tag_interface {
    void* const (*create)(struct tag*, struct ged_record*, struct lex_token*);
    void (*free)(struct tag*);
};

struct tag* tag_create(const struct tag_interface* iface, struct context* ctx,
                       struct ged_record* rec, struct lex_token* tok);

void tag_free(struct tag* tag);

const struct tag_interface* tag_i_get(const char* key);

void tags_init(void);
void tags_cleanup(void);

#endif // TAGS_BASE_H