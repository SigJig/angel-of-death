
#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define SBUILDER_DEFAULT_CAP 100
#define SBUILDER_DEFAULT_CAP_MULT 2

struct sbuilder
{
    char* mem;
    size_t len;
    size_t cap;
};

int sbuilder_init(struct sbuilder* builder, size_t cap);
void sbuilder_destroy(struct sbuilder* builder);
void sbuilder_clear(struct sbuilder* builder);

int sbuilder_write(struct sbuilder* builder, const char* addition);
int sbuilder_vwritef(struct sbuilder* builder, const char* fmt, va_list args);
int sbuilder_writef(struct sbuilder* builder, const char* fmt, ...);
int sbuilder_write_char(struct sbuilder* builder, char c);

char sbuilder_back(struct sbuilder* builder);

// Count how many consecutive charcters c return true for f(c). If param back is false, it counts from front, otherwise from back.
int sbuilder_num_consec(struct sbuilder* builder, bool (*func)(char), bool back);

const char* sbuilder_to_string(const struct sbuilder* builder);

// Clears the struct sbuilder to be reused, and then returns a heap allocated char* (must be freed)
char* sbuilder_return(struct sbuilder* builder);

// Destroys the builder and returns a heap allocated char* (must be freed)
char* sbuilder_complete(struct sbuilder* builder);

#endif // STRINGBUILDER_H