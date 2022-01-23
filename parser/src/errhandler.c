
#include "errhandler.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define CAP_INIT_SZ 10
#define CAP_MULT_FAC 2

static struct err_message* ehandler_reserve(struct err_handler* handler)
{
    handler->len += 1;

    if (handler->len > handler->cap)
    {
        int mult_fac = (handler->len / handler->cap + (handler->len % handler->cap != 0)) * CAP_MULT_FAC;
        handler->cap *= mult_fac;

        handler->messages = realloc(handler->messages, handler->cap * (sizeof *handler->messages));
    }

    return &handler->messages[handler->len - 1];
}

static struct err_message* ehandler_add(struct err_handler* handler, const char* err_type, const char* origin, const char* format, va_list args)
{
    struct err_message* msg = ehandler_reserve(handler);
    struct sbuilder builder;

    if (sbuilder_init(&builder, 100) != ST_OK) return NULL;

    sbuilder_vwritef(&builder, format, args);

    msg->type = err_type;
    msg->origin = origin;
    msg->message = sbuilder_complete(&builder);

    return msg;
}

void emessage_destroy(struct err_message* msg)
{
    free(msg->message);
}

char* emessage_to_string(struct err_message* msg)
{
    struct sbuilder builder;
    
    if (sbuilder_init(&builder, 100) != ST_OK) return NULL;

    sbuilder_writef(&builder, "%s from %s: %s", msg->type, msg->origin, msg->message);

    return sbuilder_complete(&builder);
}

e_statuscode ehandler_init(struct err_handler* handler)
{
    if (!handler) return ST_INIT_FAIL;

    handler->len = 0;

    assert(CAP_INIT_SZ);

    handler->cap = CAP_INIT_SZ;
    handler->messages = malloc((sizeof *handler->messages) * CAP_INIT_SZ);

    if (!handler->messages) return ST_INIT_FAIL;

    return ST_OK;
}

void ehandler_destroy(struct err_handler* handler)
{
    for (size_t i = 0; i < handler->len; i++)
    {
        emessage_destroy(&handler->messages[i]);
    }

    free(handler->messages);
}

struct err_message* ehandler_verrf(struct err_handler* handler, const char* origin, const char* format, va_list args)
{
    return ehandler_add(handler, "error", origin, format, args);
}

struct err_message* ehandler_errf(struct err_handler* handler, const char* origin, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    struct err_message* result = ehandler_verrf(handler, origin, format, args);
    
    va_end(args);

    return result;
}