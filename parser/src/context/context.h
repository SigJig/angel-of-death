
#ifndef CONTEXT_H
#define CONTEXT_H

#include "utils/dynarray.h"
#include "utils/statuscode.h"
#include <stdarg.h>
#include <stdbool.h>

#define CTX_STATE_INTERFACE                                                    \
    fn_state_to_string to_string;                                              \
    fn_state_free free;                                                        \
    fn_state_copy copy

struct ctx_state;

typedef char* (*fn_state_to_string)(struct ctx_state*);
typedef void (*fn_state_free)(struct ctx_state*);
typedef struct ctx_state* (*fn_state_copy)(struct ctx_state*);

typedef enum { DEBUG = 0, INFO, WARNING, ERROR, CRITICAL } ctx_e_loglevel;

struct ctx_state {
    CTX_STATE_INTERFACE;
};

struct ctx_log_message {
    char* message;
    ctx_e_loglevel level;
    struct dyn_array* stack; // copy of the current context stack
};

struct context {
    bool can_continue;
    ctx_e_loglevel log_level;
    struct dyn_array* stack; // array of ctx_state
    struct dyn_array* log;   // array of ctx_log_message
};

struct context* ctx_create(ctx_e_loglevel log_level);
void ctx_free(struct context* ctx);

// Whether critical errors that should stop parsing have been encountered.
// For example, if critical error is encountered in lexing, it will not continue
// to parsing
bool ctx_continue(struct context* ctx);

e_statuscode ctx_push(struct context* ctx, struct ctx_state* state);
e_statuscode ctx_pop(struct context* ctx);

struct ctx_state* ctx_state(struct context* ctx);

char* ctx_log_to_string(struct context* ctx);

e_statuscode ctx_debug(struct context* ctx, const char* message);
e_statuscode ctx_debugf(struct context* ctx, const char* format, ...);
e_statuscode ctx_vdebugf(struct context* ctx, const char* format, va_list args);

e_statuscode ctx_info(struct context* ctx, const char* message);
e_statuscode ctx_infof(struct context* ctx, const char* format, ...);
e_statuscode ctx_vinfof(struct context* ctx, const char* format, va_list args);

e_statuscode ctx_warn(struct context* ctx, const char* message);
e_statuscode ctx_warnf(struct context* ctx, const char* format, ...);
e_statuscode ctx_vwarnf(struct context* ctx, const char* format, va_list args);

e_statuscode ctx_err(struct context* ctx, const char* message);
e_statuscode ctx_errf(struct context* ctx, const char* format, ...);
e_statuscode ctx_verrf(struct context* ctx, const char* format, va_list args);

e_statuscode ctx_crit(struct context* ctx, const char* message);
e_statuscode ctx_critf(struct context* ctx, const char* format, ...);
e_statuscode ctx_vcritf(struct context* ctx, const char* format, va_list args);

#endif // CONTEXT_H