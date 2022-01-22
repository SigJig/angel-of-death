
#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define SBUILDER_DEFAULT_CAP 100
#define SBUILDER_DEFAULT_CAP_MULT 2

typedef struct
{
    char* mem;
    size_t len;
    size_t cap;
} sbuilder;

int sbuilder_init(sbuilder* builder, size_t cap);
void sbuilder_destroy(sbuilder* builder);
void sbuilder_clear(sbuilder* builder);

int sbuilder_write(sbuilder* builder, const char* addition);
int sbuilder_vwritef(sbuilder* builder, const char* fmt, va_list args);
int sbuilder_writef(sbuilder* builder, const char* fmt, ...);
int sbuilder_write_char(sbuilder* builder, char c);

char sbuilder_back(sbuilder* builder);

// Count how many consecutive charcters c return true for f(c). If param back is false, it counts from front, otherwise from back.
int sbuilder_num_consec(sbuilder* builder, bool (*func)(char), bool back);

const char* sbuilder_to_string(const sbuilder* builder);

// Clears the sbuilder to be reused, and then returns a heap allocated char* (must be freed)
char* sbuilder_return(sbuilder* builder);

// Destroys the builder and returns a heap allocated char* (must be freed)
char* sbuilder_complete(sbuilder* builder);

#endif // STRINGBUILDER_H