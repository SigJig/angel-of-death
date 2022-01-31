/*
Generic context state
*/

#ifndef GENSTATE_H
#define GENSTATE_H

#include "context/context.h"
#include <stdint.h>

struct posctx_data {
    const char* origin;
    size_t line;
    size_t col;
};

struct ctx_state* posctx_create(const char* origin);

void posctx_fn_free(struct ctx_state* state);
char* posctx_fn_to_string(struct ctx_state* state);
struct ctx_state* posctx_fn_copy(struct ctx_state* state);

struct ctx_state* posstate_from_ctx(struct context* ctx);

void posctx_update_line(struct context* ctx, size_t line);
void posctx_update_col(struct context* ctx, size_t col);
void posctx_update(struct context* ctx, size_t line, size_t col);

struct tagctx_data {
    char* name;
    size_t line;
    size_t col;
};

struct ctx_state* tagctx_create(char* name, size_t line, size_t col);
void tagctx_fn_free(struct ctx_state* state);
char* tagctx_fn_to_string(struct ctx_state* state);
struct ctx_state* tagctx_fn_copy(struct ctx_state* state);

#endif // GENSTATE_H