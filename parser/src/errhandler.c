
#include "errhandler.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define CAP_INIT_SZ 10
#define CAP_MULT_FAC 2

static struct err_message* ehandler_add(struct err_handler* handler, const char* err_type, const char* origin, const char* format, va_list args)
{
    struct err_message* msg = da_reserve(handler->messages);
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

    assert(CAP_INIT_SZ);

    handler->messages = da_create(CAP_INIT_SZ, sizeof *handler->messages);

    if (!handler->messages) return ST_INIT_FAIL;

    return ST_OK;
}

void ehandler_destroy(struct err_handler* handler)
{
    for (size_t i = 0; i < handler->messages->len; i++)
    {
        emessage_destroy(da_get(handler->messages, i));
    }

    da_free(handler->messages);
}

size_t ehandler_len(struct err_handler* handler)
{
    return handler->messages->len;
}

struct err_message* ehandler_get(struct err_handler* handler, size_t index)
{
    return da_get(handler->messages, index);
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