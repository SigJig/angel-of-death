
#ifndef ERRHANDLER_H
#define ERRHANDLER_H

#include <stdlib.h>
#include "stringbuilder.h"
#include "statuscode.h"

struct err_message
{
    const char* type;
    const char* origin;
    char* message;
    size_t line;
    size_t col;
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

/*
origin: string literal
message: can be constructed dynamically, the handler will copy it and free its copy upon destruction
*/
struct err_message* ehandler_err(struct err_handler* handler, const char* origin, const char* message, size_t line, size_t col);

#endif // ERRHANDLER_H