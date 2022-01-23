
#include "dynarray.h"
#include "gedcom.h"
#include <assert.h>

#define DEFAULT_STACK_CAP 100
#define DEFAULT_STACK_CAP_MULT 2

struct ged_builder
{
    struct err_handler* ehandler;

    // The stack contains pointers to record pointers
    // Thus, the records are not freed when calling free on the stack's memory block
    struct dyn_array* stack;
};

static e_statuscode builder_init(struct ged_builder* ged, struct err_handler* ehandler)
{
    assert(DEFAULT_STACK_CAP);

    ged->ehandler = ehandler;
    ged->stack = da_create(DEFAULT_STACK_CAP, sizeof(struct ged_record*));
}

static void builder_destroy(struct ged_builder* ged)
{
    if (ged->stack->len)
    {
        for (size_t i = 0; i < ged->stack->len; i++)
        {
            ged_record_free(*(struct ged_record**)da_get(ged->stack, i));
        }
    }

    da_free(ged->stack);
}

static e_statuscode builder_stack_add(struct ged_builder* ged, struct ged_record* rec)
{
    struct ged_record** mem = da_reserve(ged->stack);
    if (!mem) return ST_GEN_ERROR;

    *mem = rec;

    return ST_OK;
}

static struct ged_record* builder_stack_pop(struct ged_builder* ged)
{
    return *(struct ged_record**)da_pop(ged->stack);
}

void ged_line_value_free(struct ged_line_value* val)
{
    free(val->mem);
    free(val);
}

void ged_record_free(struct ged_record* rec)
{
    if (rec->xref)  free(rec->xref);
    if (rec->tag)   free(rec->tag);
    if (rec->value) ged_line_value_free(rec->value);

    for (uint8_t i = 0; i < rec->len_children; i++)
    {
        ged_record_free(&rec->children[i]);
    }

    free(rec);
}