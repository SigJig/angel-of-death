
#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

#include <stdlib.h>
#include <string.h>

#define SBUILDER_DEFAULT_CAP 100
#define SBUILDER_DEFAULT_CAP_MULT 2

// 24b
typedef struct
{
    // 8b * 3
    char* mem;
    size_t len;
    size_t cap;
} sbuilder;

int sbuilder_init(sbuilder* builder, size_t cap);
void sbuilder_destroy(sbuilder* builder);
void sbuilder_clear(sbuilder* builder);

int _sbuilder_verify_cap(sbuilder* builder);

int sbuilder_write(sbuilder* builder, const char* addition);
int sbuilder_write_char(sbuilder* builder, char c);

char sbuilder_back(sbuilder* builder);
const char* sbuilder_to_string(const sbuilder* builder);

// Clears the sbuilder to be reused, and then returns a heap allocated char* (must be freed)
char* sbuilder_return(sbuilder* builder);

#endif // STRINGBUILDER_H