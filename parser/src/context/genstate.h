/*
Generic context state
*/

#ifndef GENSTATE_H
#define GENSTATE_H

#include "context/context.h"
#include <stdint.h>

struct posctx_state {
    CTX_STATE_INTERFACE;

    const char* origin;
    size_t line;
    size_t col;
};

void posctx_fn_free(struct posctx_state* state);
char* posctx_fn_to_string(struct posctx_state* state);
struct ctx_state* posctx_fn_copy(struct posctx_state* state);

struct ctx_state* posctx_create(const char* origin);

struct posctx_state* posstate_from_ctx(struct context* ctx);

void posctx_update_line(struct context* ctx, size_t line);
void posctx_update_col(struct context* ctx, size_t col);
void posctx_update(struct context* ctx, size_t line, size_t col);

#endif // GENSTATE_H