
#include "context/genstate.h"
#include "utils/stringbuilder.h"
#include <assert.h>
#include <stdlib.h>

struct ctx_state*
posctx_create(const char* origin)
{
    struct posctx_state* state = malloc(sizeof *state);

    state->origin = origin;
    state->free = (fn_state_free)posctx_fn_free;
    state->to_string = (fn_state_to_string)posctx_fn_to_string;
    state->copy = (fn_state_copy)posctx_fn_copy;
    state->line = 0;
    state->col = 0;

    return (struct ctx_state*)state;
}

struct posctx_state*
posstate_from_ctx(struct context* ctx)
{
    struct posctx_state* state = (struct posctx_state*)ctx_state(ctx);

    if (!state) {
        assert(false);

        return NULL;
    }

    return state;
}

void
posctx_update_line(struct context* ctx, size_t line)
{
    struct posctx_state* state = posstate_from_ctx(ctx);

    if (!state) {
        return;
    }

    state->line = line;
}

void
posctx_update_col(struct context* ctx, size_t col)
{
    struct posctx_state* state = posstate_from_ctx(ctx);

    if (!state) {
        return;
    }

    state->col = col;
}

void
posctx_update(struct context* ctx, size_t line, size_t col)
{
    struct posctx_state* state = posstate_from_ctx(ctx);

    if (!state) {
        return;
    }

    state->line = line;
    state->col = col;
}

void
posctx_fn_free(struct posctx_state* state)
{
    free(state);
}

char*
posctx_fn_to_string(struct posctx_state* state)
{
    struct sbuilder builder;

    if (sbuilder_init(&builder, 50) != ST_OK) {
        assert(false);

        return NULL;
    }

    sbuilder_writef(&builder, "%s (line: %zu, column: %zu)", state->origin,
                    state->line, state->col);

    return sbuilder_complete(&builder);
}

struct ctx_state*
posctx_fn_copy(struct posctx_state* state)
{
    struct posctx_state* copy =
        (struct posctx_state*)posctx_create(state->origin);

    copy->line = state->line;
    copy->col = state->col;

    return (struct ctx_state*)copy;
}