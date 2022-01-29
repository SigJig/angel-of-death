
#include <assert.h>
#include <stdarg.h>

#include "dynarray.h"
#include "gedcom.h"
#include "hashmap.h"

#define DEFAULT_STACK_CAP 100
#define DEFAULT_STACK_CAP_MULT 2

// cap should be prime, and 1.3 * amount of expected items
// 100 * 1.3, closest prime = 131
#define DEFAULT_XREFS_CAP 131

struct ged_builder {
    struct err_handler* ehandler;

    uint8_t cur_level;

    // The stack contains pointers to record pointers
    // Thus, the records are not freed when calling free on the stack's
    // memory block
    struct dyn_array* stack;
    struct hash_table* xrefs;
    struct context* ctx;
};

static e_statuscode
builder_init(struct ged_builder* ged, struct context* ctx)
{
    assert(DEFAULT_STACK_CAP);

    ged->cur_level = 0;
    ged->stack = da_create(DEFAULT_STACK_CAP, sizeof(struct ged_record*));
    ged->xrefs = ht_create(DEFAULT_XREFS_CAP);
    ged->ctx = ctx;
}

static void
builder_destroy(struct ged_builder* ged)
{
    if (ged->stack->len) {
        for (size_t i = 0; i < ged->stack->len; i++) {

            ged_record_free(*(struct ged_record**)da_get(ged->stack, i));
        }
    }

    da_free(ged->stack);
    ht_free(ged->xrefs);
    ged->ctx = NULL;
}

static e_statuscode
builder_stack_add(struct ged_builder* ged, struct ged_record* rec)
{
    ged->cur_level = rec->level;
    struct ged_record** mem = da_reserve(ged->stack);
    if (!mem)
        return ST_GEN_ERROR;

    *mem = rec;

    return ST_OK;
}

static e_statuscode
builder_child_add(struct ged_builder* ged, struct ged_record* rec)
{
    struct ged_record** recp = da_back(ged->stack);

    if (!(recp && *recp)) {
        return ST_GEN_ERROR;
    }

    struct ged_record* last = (*recp)->children;

    if (!last) {
        (*recp)->children = rec;
    } else {
        while (last->next) {
            last = last->next;
        }

        last->next = rec;
    }

    ged->cur_level++;

    return ST_OK;
}

static struct ged_record*
builder_stack_pop(struct ged_builder* ged)
{
    if (ged->cur_level) {
        ged->cur_level--;
    }

    return *(struct ged_record**)da_pop(ged->stack);
}

struct ged_record*
ged_record_construct(struct ged_builder* ged, struct parser_line* line)
{
    struct ged_record* rec = malloc(sizeof *rec);

    char* level = line->level->lexeme;
    int level_parsed = atoi(level);

    if (strlen(level) > 1 && level[0] == '0') {
        // the gedcom standard does not allow leading 0s on levels
        // TODO: this should be warning
        ctx_warnf(ged->ctx,
                  "gedcom standard disallows leading 0 on level "
                  "declarations (%s)",
                  level);

    } else if (level_parsed == 0) {
        ctx_critf(ged->ctx, "unable to parse %s as level", level);
        goto error;
    }

    rec->level = level_parsed;

    if (line->xref) {
        rec->xref = strdup(line->xref->lexeme);

        // add to symbol table
        if (ht_get(ged->xrefs, rec->xref) != NULL) {
            ctx_critf(ged->ctx, "xref %s already defined", rec->xref);
        } else {
            ht_set(ged->xrefs, rec->xref, rec);
        }
    } else {
        rec->xref = NULL;
    }

    rec->tag = strdup(line->tag->lexeme);

    if (strlen(rec->tag) > 0 && rec->tag[0] == '_') {
        // TODO: Handle custom tags
    }

    if (line->line_value) {
        struct tag_interface* interface = tag_i_get(rec->tag);

        rec->value =
            (*interface->create)(interface, rec, line->line_value, ged->ctx);
    } else {
        rec->value = NULL;
    }

    if (rec->level) {
        if (rec->level > ged->cur_level) {
            if (rec->level != ged->cur_level + 1) {
                ctx_critf(ged->ctx,
                          "invalid line level (should be %d, %d, is %d)",
                          ged->cur_level, ged->cur_level + 1, rec->level);

                return NULL;
            }

            builder_child_add(ged, rec);
        } else {
            builder_stack_pop(ged);
        }
    }

    if (builder_stack_add(ged, rec) != ST_OK) {
        assert(false);
        // TODO: error
    }

    return rec;

error:
    ged_record_free(rec);

    return NULL;
}

void
ged_record_free(struct ged_record* rec)
{
    if (rec->xref) {
        free(rec->xref);
    }
    if (rec->tag) {
        free(rec->tag);
    }

    if (rec->value) {
        (*rec->value->interface->free)(rec->value);
    }

    for (uint8_t i = 0; i < rec->len_children; i++) {
        ged_record_free(&rec->children[i]);
    }

    free(rec);
}
