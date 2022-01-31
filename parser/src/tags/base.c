
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "tags/base.h"
#include "tags/month.h"

static void* const
tag_invalid_create(struct tag* tag, struct ged_record* rec,
                   struct lex_token* token)
{
    ctx_critf(tag->ctx, "invalid tag name");

    return NULL;
}

static void
tag_invalid_free(struct tag* tag)
{
}

static const struct tag_interface invalid_interface = {
    .create = tag_invalid_create, .free = tag_invalid_free};

// Hashtable of tag interfaces
static struct hash_table* tags_ht = NULL;

static struct hash_table*
tags_ht_instance(void)
{
    if (!tags_ht) {
        assert(false);
    }

    return tags_ht;
}

struct tag*
tag_create(const struct tag_interface* iface, struct context* ctx,
           struct ged_record* rec, struct lex_token* tok)
{
    struct tag* tag = malloc(sizeof *tag);

    if (!tag) {
        assert(false);

        return NULL;
    }

    tag->interface = iface;
    tag->ctx = ctx;
    *(void**)&tag->data = tag->interface->create(tag, rec, tok);

    return tag;
}

void
tag_free(struct tag* tag)
{
    if (!tag) {
        return;
    }

    tag->interface->free(tag);
    free(tag);
}

void
tags_init(void)
{
    assert(!tags_ht /* Tag hashtable has already been initialized */);

    tags_ht = ht_create(31);

    /*
    Array of modules, only used for initialization as the contents of
    this are added to the hash_table through the init function added
    here.

    Each module should contain an init function following the signature:
    void init_module(struct hash_table*);

    This array is kept inside this function, and not inside the global
    scope, as this function is called only once. Keeping this array in
    global space would then be unnecessary memory usage throughout the
    runtime of the program.
    */
    void (*init_modules[])(struct hash_table*) = {
        init_months,
    };

    assert(sizeof init_modules /* No tag modules to initialize */);

    size_t mlength = sizeof init_modules / sizeof *init_modules;

    for (size_t i = 0; i < mlength; i++) {
        (*init_modules[i])(tags_ht);
    }
}

void
tags_cleanup(void)
{
    if (!tags_ht) {
        return;
    }

    ht_free(tags_ht);
    tags_ht = NULL;
}

const struct tag_interface*
tag_i_get(const char* key)
{
    const struct tag_interface* tag = ht_get(tags_ht_instance(), key);

    if (!tag) {
        return &invalid_interface;
    }

    return tag;
}