
#include "context/context.h"
#include "utils/stringbuilder.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static struct ctx_state*
stack_get_state(ptr_arr stack, size_t index)
{
    return pa_get(stack, index);
}

static void
stack_free(ptr_arr stack)
{
    for (size_t i = 0; i < pa_len(stack); i++) {
        struct ctx_state* state = stack_get_state(stack, i);

        assert(state);

        ctx_state_destroy(state);
    }

    pa_free(stack);
}

static ptr_arr
stack_copy(ptr_arr stack)
{
    ptr_arr copy = pa_create(pa_len(stack));

    for (size_t i = 0; i < pa_len(stack); i++) {
        struct ctx_state* state = stack_get_state(stack, i);
        assert(state);

        pa_push(copy, state->interface->copy(state));
    }

    return copy;
}

static char*
stack_trace(ptr_arr stack)
{
    if (!pa_len(stack)) {
        return NULL;
    }

    struct sbuilder builder = sbuilder_new();

    for (size_t i = 0; i < pa_len(stack); i++) {
        struct ctx_state* state = stack_get_state(stack, i);

        if (!state) {
            assert(false);

            continue;
        }

        char* state_string = state->interface->to_string(state);

        sbuilder_writef(&builder, "in <%s>\n", state_string);

        free(state_string);
    }

    return sbuilder_term(&builder);
}

static e_statuscode
log_add(struct context* ctx, ctx_e_loglevel level, const char* message)
{
    if (level == CRITICAL) {
        ctx->can_continue = false;
    }

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

        return ST_MALLOC_ERROR;
    }

    msg->level = level;
    msg->message = strdup(message);
    msg->stack = stack_copy(ctx->stack);

    pa_push(ctx->log, msg);

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
    char* string = sbuilder_term(&builder);

    e_statuscode result = log_add(ctx, level, string);

    free(string);

    return result;
}

static char*
log_to_string(struct ctx_log_message* log)
{
    struct sbuilder builder = sbuilder_new();

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

    return sbuilder_term(&builder);
}

struct ctx_state*
ctx_state_create(const struct ctx_state_interface* interface, void* data)
{
    struct ctx_state* state = malloc(sizeof *state);

    if (!state) {
        assert(false);

        return NULL;
    }

    state->interface = interface;
    *(void**)&state->data = data;

    return state;
}

void
ctx_state_destroy(struct ctx_state* state)
{
    if (!state) {
        return;
    }

    state->interface->free(state);
    free(state);
}

struct context*
ctx_create(ctx_e_loglevel log_level)
{
    struct context* ctx = malloc(sizeof *ctx);

    if (!ctx) {
        assert(false /* context init failed */);
        return NULL;
    }

    ctx->can_continue = true;
    ctx->log_level = log_level;
    ctx->stack = pa_create(100);
    ctx->log = pa_create(10);

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
        for (size_t i = 0; i < pa_len(ctx->log); i++) {
            struct ctx_log_message* log = pa_get(ctx->log, i);

            assert(log);

            log_destroy(log);
        }

        pa_free(ctx->log);
    }

    if (ctx->stack) {
        stack_free(ctx->stack);
    }

    free(ctx);
}

bool
ctx_continue(struct context* ctx)
{
    return ctx->can_continue;
}

e_statuscode
ctx_push(struct context* ctx, struct ctx_state* state)
{
    return pa_push(ctx->stack, state);
}

e_statuscode
ctx_pop(struct context* ctx)
{
    if (!pa_len(ctx->stack)) {
        assert(false /* Stack pop attempted on empty stack */);
        return ST_GEN_ERROR;
    }

    struct ctx_state* state = pa_pop(ctx->stack);

    ctx_state_destroy(state);

    return ST_OK;
}

struct ctx_state*
ctx_state(struct context* ctx)
{
    if (!pa_len(ctx->stack)) {
        assert(false /* No states on stack */);
        return NULL;
    }

    struct ctx_state* state = pa_back(ctx->stack);

    return state;
}

char*
ctx_log_to_string(struct context* ctx)
{
    if (!pa_len(ctx->log)) {
        return NULL;
    }

    struct sbuilder builder;

    if (sbuilder_init(&builder, 1000) != ST_OK) {
        assert(false);

        return NULL;
    }

    for (size_t i = 0; i < pa_len(ctx->log); i++) {
        char* lstr = log_to_string(pa_get(ctx->log, i));

        sbuilder_writef(&builder, "%s\n", lstr);

        free(lstr);
    }

    return sbuilder_term(&builder);
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