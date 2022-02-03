
#include <assert.h>
#include <stdarg.h>

#include "context/context.h"
#include "context/genstate.h"
#include "gedcom.h"
#include "utils/dynarray.h"
#include "utils/hashmap.h"

#define DEFAULT_STACK_CAP 100
#define DEFAULT_STACK_CAP_MULT 2

// cap should be prime, and 1.3 * amount of expected items
// 100 * 1.3, closest prime = 131
#define DEFAULT_XREFS_CAP 131

struct ged_builder {
    uint8_t cur_level;

    struct dyn_array* stack;

    struct hash_table* xrefs;
    struct context* ctx;
};

static e_statuscode
builder_init(struct ged_builder* ged, struct context* ctx)
{
    assert(DEFAULT_STACK_CAP);

    ged->cur_level = 0;
    ged->stack = da_create(DEFAULT_STACK_CAP);
    ged->xrefs = ht_create(DEFAULT_XREFS_CAP);
    ged->ctx = ctx;

    tags_init();

    return ST_OK;
}

static void
builder_destroy(struct ged_builder* ged)
{
    if (ged->stack->len) {
        ctx_debugf(ged->ctx,
                   "ged_builder destroying with non-empty stack (%zu items)",
                   ged->stack->len);

        for (size_t i = 0; i < ged->stack->len; i++) {
            ged_record_free(da_get(ged->stack, i));
        }
    }

    da_free(ged->stack);
    ht_free(ged->xrefs);
    ged->ctx = NULL;

    tags_cleanup();
}

static e_statuscode
builder_stack_add(struct ged_builder* ged, struct ged_record* rec)
{
    ged->cur_level = rec->level;

    e_statuscode result = da_push(ged->stack, rec);

    if (result != ST_OK) {
        return result;
    }

    return ST_OK;
}

static e_statuscode
builder_child_add(struct ged_builder* ged, struct ged_record* rec)
{
    struct ged_record* back = da_back(ged->stack);

    if (!back) {
        return ST_GEN_ERROR;
    }

    struct ged_record* last = back->children;

    if (!last) {
        back->children = rec;
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
    if (!ged->stack->len) {
        return NULL;
    }

    struct ged_record* result = da_pop(ged->stack);

    if (ged->stack->len) {
        struct ged_record* back = da_back(ged->stack);

        ged->cur_level = back->level;
    } else {
        ged->cur_level = 0;
    }

    return result;
}

struct ged_record*
ged_record_construct(struct ged_builder* ged, struct parser_line* line)
{
    struct ged_record* rec = malloc(sizeof *rec);

    rec->level = 0;
    rec->xref = NULL;
    rec->tag = NULL;
    rec->value = NULL;
    rec->next = NULL;
    rec->children = NULL;

    char* level = line->level->lexeme;
    int level_parsed = atoi(level);

    if (strlen(level) > 1 && level[0] == '0') {
        ctx_warnf(ged->ctx,
                  "gedcom standard disallows leading 0 on level "
                  "declarations (%s)",
                  level);

    } else if (level_parsed == 0 && level[0] != '0') {
        ctx_critf(ged->ctx, "unable to parse %s as level", level);
        goto error;
    }

    rec->level = level_parsed;

    if (line->xref) {
        rec->xref = strdup(line->xref->lexeme);

        // add to symbol table
        if (ht_get(ged->xrefs, rec->xref) != NULL) {
            ctx_errf(ged->ctx, "xref %s already defined", rec->xref);
        } else {
            ht_set(ged->xrefs, rec->xref, rec);
        }
    }

    rec->tag = strdup(line->tag->lexeme);

    if (strlen(rec->tag) > 0 && rec->tag[0] == '_') {
        // TODO: Handle custom tags
    }

    if (line->line_value) {
        ctx_push(ged->ctx,
                 tagctx_create(rec->tag, line->tag->line, line->tag->col));

        rec->value =
            tag_create(tag_i_get(rec->tag), ged->ctx, rec, line->line_value);

        ctx_pop(ged->ctx);
    }

    if (rec->level > ged->cur_level) {
        if (rec->level != ged->cur_level + 1) {
            ctx_critf(ged->ctx, "invalid line level (should be %d, %d, is %d)",
                      ged->cur_level, ged->cur_level + 1, rec->level);

            goto error;
        }
    } else {
        while (ged->stack->len && rec->level <= ged->cur_level) {
            builder_stack_pop(ged);
        }
    }

    if (ged->stack->len) {
        builder_child_add(ged, rec);
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

struct ged_record*
ged_from_parser(struct parser_result result, struct context* ctx)
{
    struct ged_builder ged;

    if (builder_init(&ged, ctx) != ST_OK) {
        return NULL;
    }

    ctx_push(ctx, posctx_create("generator"));

    struct parser_line* line = result.front;

    struct ged_record* front = NULL;
    struct ged_record* tmp = NULL;

    while (line) {
        struct ged_record* cur = ged_record_construct(&ged, line);

        if (!cur) {
            ctx_debugf(ctx, "unable to create record for line %d",
                       line->level->line);
        } else if (cur->level) {
            ctx_debugf(ctx, "skipping record level %d", cur->level);
        } else if (tmp) {
            tmp->next = cur;
            tmp = cur;
        } else {
            front = cur;
            tmp = front;
        }

        line = line->next;
    }
    builder_stack_pop(&ged);

    builder_destroy(&ged);
    ctx_pop(ctx);

    return front;
}

void
ged_record_free(struct ged_record* rec)
{
    if (!rec) {
        assert(false);
        return;
    }

    if (rec->xref) {
        free(rec->xref);
    }

    if (rec->tag) {
        free(rec->tag);
    }

    if (rec->value) {
        tag_free(rec->value);
    }

    struct ged_record* child = rec->children;
    struct ged_record* tmp = NULL;

    while (child) {
        tmp = child;
        child = child->next;

        ged_record_free(tmp);
    }

    free(rec);
}

char*
ged_record_to_string(struct ged_record* rec)
{
    struct sbuilder builder = sbuilder_new();

    for (uint8_t i = 0; i < rec->level; i++) {
        sbuilder_write(&builder, "\t");
    }

    sbuilder_writef(&builder, "%d: ", rec->level);

    if (rec->xref) {
        sbuilder_writef(&builder, " <xref: %s>", rec->xref);
    }

    sbuilder_writef(&builder, " <%s> (value)\n", rec->tag);

    struct ged_record* child = rec->children;

    while (child) {
        char* chstr = ged_record_to_string(child);
        sbuilder_writef(&builder, "%s", chstr);
        free(chstr);

        child = child->next;
    }

    return sbuilder_term(&builder);
}