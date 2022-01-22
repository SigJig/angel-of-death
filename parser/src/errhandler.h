
#ifndef ERRHANDLER_H
#define ERRHANDLER_H

#include <stdlib.h>
#include <stdarg.h>
#include "stringbuilder.h"
#include "statuscode.h"

struct err_message
{
    const char* type;
    const char* origin;
    char* message;
};

struct err_handler
{
    struct err_message* messages;
    size_t len;
    size_t cap;
};

e_status ehandler_init(struct err_handler* handler);
void ehandler_destroy(struct err_handler* handler);

void emessage_destroy(struct err_message* msg);
char* emessage_to_string(struct err_message* msg);

struct err_message* ehandler_verrf(struct err_handler* handler, const char* origin, const char* format, va_list args);
struct err_message* ehandler_errf(struct err_handler* handler, const char* origin, const char* format, ...);

#endif // ERRHANDLER_H