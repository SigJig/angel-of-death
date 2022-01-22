
#include "errhandler.h"

#include <stdio.h>
#include <string.h>

#define CAP_INIT_SZ 10
#define CAP_MULT_FAC 2

static struct err_message* ehandler_reserve(struct err_handler* handler)
{
    handler->len += 1;

    if (handler->len > handler->cap)
    {
        int mult_fac = (handler->len / handler->cap + (handler->len % handler->cap != 0)) * CAP_MULT_FAC;

        handler->messages = (struct err_message*)realloc(handler->messages, ((sizeof *handler->messages) * handler->cap) * mult_fac);
    }

    return &handler->messages[handler->len - 1];
}

static struct err_message* ehandler_add(struct err_handler* handler, const char* err_type, const char* origin, const char* message, size_t line, size_t col)
{
    struct err_message* msg = ehandler_reserve(handler);

    msg->type = err_type;
    msg->origin = origin;
    msg->message = strdup(message);
    msg->line = line;
    msg->col = col;

    return msg;
}

void emessage_destroy(struct err_message* msg)
{
    free(msg->message);
}

char* emessage_to_string(struct err_message* msg)
{
    sbuilder builder;
    
    if (!sbuilder_init(&builder, 100)) return NULL;

    sbuilder_write(&builder, msg->type);
    sbuilder_write(&builder, " from ");
    sbuilder_write(&builder, msg->origin);

    // TODO: This needs a better solution to handle larger numbers, prob best to make a printf function for the stringbuilder
    char line_info[50];
    sprintf(line_info, " (line: %zu, column: %zu): ", msg->line, msg->col);

    sbuilder_write(&builder, msg->message);

    return sbuilder_complete(&builder);
}

e_status ehandler_init(struct err_handler* handler)
{
    if (!handler) return ST_INIT_FAIL;

    handler->len = 0;
    handler->cap = CAP_INIT_SZ;
    handler->messages = (struct err_message*)malloc((sizeof *handler->messages) * CAP_INIT_SZ);

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

struct err_message* ehandler_err(struct err_handler* handler, const char* origin, const char* message, size_t line, size_t column)
{
    return ehandler_add(handler, "error", origin, message, line, column);
}