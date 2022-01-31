
#include "context/genstate.h"
#include "utils/stringbuilder.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const struct ctx_state_interface posctx_i = {.free = posctx_fn_free,
                                                    .to_string =
                                                        posctx_fn_to_string,
                                                    .copy = posctx_fn_copy};

struct ctx_state*
posctx_create(const char* origin)
{
    struct posctx_data* data = malloc(sizeof *data);

    if (!data) {
        assert(data);

        return NULL;
    }

    data->origin = origin;
    data->line = 0;
    data->col = 0;

    return ctx_state_create(&posctx_i, data);
}

static struct posctx_data*
posdata_from_ctx(struct context* ctx)
{
    struct ctx_state* state = ctx_state(ctx);

    if (!state) {
        assert(false);

        return NULL;
    }

    return state->data;
}

void
posctx_update_line(struct context* ctx, size_t line)
{
    struct posctx_data* data = posdata_from_ctx(ctx);

    if (!data) {
        return;
    }

    data->line = line;
}

void
posctx_update_col(struct context* ctx, size_t col)
{
    struct posctx_data* data = posdata_from_ctx(ctx);

    if (!data) {
        return;
    }

    data->col = col;
}

void
posctx_update(struct context* ctx, size_t line, size_t col)
{
    struct posctx_data* data = posdata_from_ctx(ctx);

    if (!data) {
        return;
    }

    data->line = line;
    data->col = col;
}

void
posctx_fn_free(struct ctx_state* state)
{
    free(state->data);
}

char*
posctx_fn_to_string(struct ctx_state* state)
{
    struct sbuilder builder = sbuilder_new();
    struct posctx_data* data = state->data;

    sbuilder_writef(&builder, "%s (line: %zu, column: %zu)", data->origin,
                    data->line, data->col);

    return sbuilder_term(&builder);
}

struct ctx_state*
posctx_fn_copy(struct ctx_state* state)
{
    struct posctx_data* orig_data = state->data;
    struct ctx_state* copy = posctx_create(orig_data->origin);
    struct posctx_data* copy_data = copy->data;

    if (!copy) {
        assert(false);

        return NULL;
    }

    copy_data->line = orig_data->line;
    copy_data->col = orig_data->col;

    return copy;
}

static struct ctx_state_interface tagctx_i = {.free = tagctx_fn_free,
                                              .to_string = tagctx_fn_to_string,
                                              .copy = tagctx_fn_copy};

struct ctx_state*
tagctx_create(char* name, size_t line, size_t col)
{
    struct tagctx_data* data = malloc(sizeof *data);

    if (!data) {
        assert(false);

        return NULL;
    }

    data->name = strdup(name);
    data->line = line;
    data->col = col;

    return ctx_state_create(&tagctx_i, data);
}

void
tagctx_fn_free(struct ctx_state* state)
{
    if (!state) {
        return;
    }

    struct tagctx_data* data = state->data;
    free(data->name);
    free(data);
}

char*
tagctx_fn_to_string(struct ctx_state* state)
{
    struct sbuilder builder = sbuilder_new();
    struct tagctx_data* data = state->data;

    sbuilder_writef(&builder, "tag %s (line %zu, column %zu)", data->name,
                    data->line, data->col);

    return sbuilder_term(&builder);
}

struct ctx_state*
tagctx_fn_copy(struct ctx_state* state)
{
    struct tagctx_data* data = state->data;

    return tagctx_create(data->name, data->line, data->col);
}