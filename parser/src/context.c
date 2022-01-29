
#include "context.h"
#include "stringbuilder.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static void
state_free(struct ctx_state* state)
{
    (*state->free)(state);
}

static struct ctx_state*
stack_get_state(struct dyn_array* stack, size_t index)
{
    return *(struct ctx_state**)da_get(stack, index);
}

static void
stack_free(struct dyn_array* stack)
{
    for (size_t i = 0; i < stack->len; i++) {
        struct ctx_state* state = stack_get_state(stack, i);

        assert(state);

        state_free(state);
    }

    da_free(stack);
}

static struct dyn_array*
stack_copy(struct dyn_array* stack)
{
    struct dyn_array* copy = da_create(stack->len, stack->byte_n);

    for (size_t i = 0; i < stack->len; i++) {
        struct ctx_state** mem = (struct ctx_state**)da_reserve(copy);
        assert(mem);

        struct ctx_state* state = stack_get_state(stack, i);
        assert(state);

        *mem = (*state->copy)(state);
    }

    return copy;
}

static char*
stack_trace(struct dyn_array* stack)
{
    if (!stack->len) {
        return NULL;
    }

    struct sbuilder builder;

    if (sbuilder_init(&builder, 100) != ST_OK) {
        assert(false);

        return NULL;
    }

    for (size_t i = 0; i < stack->len; i++) {
        struct ctx_state* state = stack_get_state(stack, i);

        if (!stack) {
            assert(false);

            continue;
        }

        char* state_string = (*state->to_string)(state);

        sbuilder_writef(&builder, "in <%s>\n", state_string);

        free(state_string);
    }

    return sbuilder_complete(&builder);
}

static e_statuscode
log_add(struct context* ctx, ctx_e_loglevel level, const char* message)
{
    if (level < ctx->log_level) {
        return ST_NOT_OK;
    }

    struct ctx_state* state = ctx_state(ctx);

    if (!state) {
        assert(false /* attempted to add log before state initialized */);

        return ST_GEN_ERROR;
    }

    struct ctx_log_message* msg = malloc(sizeof *msg);

    if (!msg) {
        assert(false);

        return ST_GEN_ERROR;
    }

    msg->level = level;
    msg->message = strdup(message);
    msg->stack = stack_copy(ctx->stack);

    struct ctx_log_message** mem =
        (struct ctx_log_message**)da_reserve(ctx->log);

    if (!mem) {
        assert(false);

        return ST_GEN_ERROR;
    }

    *mem = msg;

    return ST_OK;
}

static void
log_destroy(struct ctx_log_message* log)
{
    free(log->message);
    stack_free(log->stack);

    free(log);
}

static e_statuscode
log_vaddf(struct context* ctx, ctx_e_loglevel level, const char* format,
          va_list args)
{
    struct sbuilder builder;

    if (sbuilder_init(&builder, 500) != ST_OK) {
        assert(false /* sbuilder init failed in ctx_verrf */);

        return ST_GEN_ERROR;
    }

    sbuilder_vwritef(&builder, format, args);
    char* string = sbuilder_complete(&builder);

    e_statuscode result = log_add(ctx, level, string);

    free(string);

    return result;
}

static char*
log_to_string(struct ctx_log_message* log)
{
    struct sbuilder builder;

    if (sbuilder_init(&builder, 100) != ST_OK) {
        assert(false /* log_to_string sbuilder init fail */);

        return NULL;
    }

    char* trace = stack_trace(log->stack);
    sbuilder_write(&builder, trace);
    free(trace);

    sbuilder_write(&builder, "\t<");

    switch (log->level) {
    case DEBUG:
        sbuilder_write(&builder, "DEBUG");
        break;
    case INFO:
        sbuilder_write(&builder, "INFO");
        break;
    case WARNING:
        sbuilder_write(&builder, "WARNING");
        break;
    case ERROR:
        sbuilder_write(&builder, "ERROR");
        break;
    case CRITICAL:
        sbuilder_write(&builder, "CRITICAL");
        break;
    default: {
        assert(false);

        sbuilder_write(&builder, "UNKNOWN");
    }
    }

    sbuilder_writef(&builder, ">: %s", log->message);

    return sbuilder_complete(&builder);
}

struct context*
ctx_create(ctx_e_loglevel log_level)
{
    struct context* ctx = malloc(sizeof *ctx);

    if (!ctx) {
        assert(false /* context init failed */);
        return NULL;
    }

    ctx->log_level = log_level;
    ctx->stack = da_create(100, sizeof(struct ctx_state*));
    ctx->log = da_create(10, sizeof(struct ctx_log_message*));

    if (!(ctx->log && ctx->stack)) {
        assert(false /* ctx members initialization failed */);
        ctx_free(ctx);

        return NULL;
    }

    return ctx;
}

void
ctx_free(struct context* ctx)
{
    if (!ctx) {
        return;
    }

    if (ctx->log) {
        for (size_t i = 0; i < ctx->log->len; i++) {
            struct ctx_log_message* log =
                *(struct ctx_log_message**)da_get(ctx->log, i);

            assert(log);

            log_destroy(log);
        }

        da_free(ctx->log);
    }

    if (ctx->stack) {
        stack_free(ctx->stack);
    }

    free(ctx);
}

e_statuscode
ctx_push(struct context* ctx, struct ctx_state* state)
{
    struct ctx_state** mem = (struct ctx_state**)da_reserve(ctx->stack);

    if (!mem) {
        assert(false /* Memory allocation for stack push failed */);

        return ST_GEN_ERROR;
    }

    *mem = state;

    return ST_OK;
}

e_statuscode
ctx_pop(struct context* ctx)
{
    if (!ctx->stack->len) {
        assert(false /* Stack pop attempted on empty stack */);
        return ST_GEN_ERROR;
    }

    struct ctx_state* state = *(struct ctx_state**)da_pop(ctx->stack);

    state_free(state);

    return ST_OK;
}

struct ctx_state*
ctx_state(struct context* ctx)
{
    if (!ctx->stack->len) {
        assert(false /* No states on stack */);
        return NULL;
    }

    struct ctx_state* state = *(struct ctx_state**)da_back(ctx->stack);

    return state;
}

char*
ctx_log_to_string(struct context* ctx)
{
    if (!ctx->log->len) {
        return NULL;
    }

    struct sbuilder builder;

    if (sbuilder_init(&builder, 1000) != ST_OK) {
        assert(false);

        return NULL;
    }

    for (size_t i = 0; i < ctx->log->len; i++) {
        char* lstr =
            log_to_string(*(struct ctx_log_message**)da_get(ctx->log, i));

        sbuilder_writef(&builder, "%s\n", lstr);

        free(lstr);
    }

    return sbuilder_complete(&builder);
}

e_statuscode
ctx_debug(struct context* ctx, const char* message)
{
    return log_add(ctx, DEBUG, message);
}

e_statuscode
ctx_debugf(struct context* ctx, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    e_statuscode result = ctx_vdebugf(ctx, format, args);

    va_end(args);

    return result;
}

e_statuscode
ctx_vdebugf(struct context* ctx, const char* format, va_list args)
{
    return log_vaddf(ctx, DEBUG, format, args);
}

e_statuscode
ctx_info(struct context* ctx, const char* message)
{
    return log_add(ctx, INFO, message);
}

e_statuscode
ctx_infof(struct context* ctx, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    e_statuscode result = ctx_vinfof(ctx, format, args);

    va_end(args);

    return result;
}

e_statuscode
ctx_vinfof(struct context* ctx, const char* format, va_list args)
{
    return log_vaddf(ctx, INFO, format, args);
}

e_statuscode
ctx_warn(struct context* ctx, const char* message)
{
    return log_add(ctx, WARNING, message);
}

e_statuscode
ctx_warnf(struct context* ctx, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    e_statuscode result = ctx_vwarnf(ctx, format, args);

    va_end(args);

    return result;
}

e_statuscode
ctx_vwarnf(struct context* ctx, const char* format, va_list args)
{
    return log_vaddf(ctx, WARNING, format, args);
}

e_statuscode
ctx_err(struct context* ctx, const char* message)
{
    return log_add(ctx, ERROR, message);
}

e_statuscode
ctx_errf(struct context* ctx, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    e_statuscode result = ctx_verrf(ctx, format, args);

    va_end(args);

    return result;
}

e_statuscode
ctx_verrf(struct context* ctx, const char* format, va_list args)
{
    return log_vaddf(ctx, ERROR, format, args);
}

e_statuscode
ctx_crit(struct context* ctx, const char* message)
{
    return log_add(ctx, CRITICAL, message);
}

e_statuscode
ctx_critf(struct context* ctx, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    e_statuscode result = ctx_vcritf(ctx, format, args);

    va_end(args);

    return result;
}

e_statuscode
ctx_vcritf(struct context* ctx, const char* format, va_list args)
{
    return log_vaddf(ctx, CRITICAL, format, args);
}