
#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAP 100
#define DEFAULT_CAP_MULT 2

typedef struct
{
    char* mem;
    size_t len;
    size_t cap;
} sbuilder;

int sbuilder_init(sbuilder* builder, size_t cap);
void sbuilder_free(sbuilder* builder);

int sbuilder_add(sbuilder* builder, const char* addition);

const char* sbuilder_to_string(const sbuilder* builder);

int sbuilder_init(sbuilder* builder, size_t cap)
{
    if (cap < 1)
        return 1;

    builder->mem = (char*)calloc(cap + 1, sizeof(char));

    if (!builder->mem)
        return 2;

    builder->cap = cap;
    builder->len = 0;

    return 0;
}

void sbuilder_free(sbuilder* builder)
{
    if (!builder->mem)
        return;

    free(builder->mem);
}

int sbuilder_add(sbuilder* builder, const char* addition)
{
    int length = strlen(addition);
    int initial_length = builder->len;
    builder->len += length;

    if (builder->len >= (builder->cap - 1))
    {
        builder->cap *= DEFAULT_CAP_MULT * ((builder->len / builder->cap) + (builder->len % builder->cap != 0));
        builder->mem = (char*)realloc(builder->mem, sizeof(char) * builder->cap);
        
        if (!builder->mem)
            return 1;

        memset(builder->mem + builder->len, 0, builder->cap - builder->len);
    }

    memcpy(builder->mem + initial_length, addition, length);

    return 0;
}

const char* sbuilder_to_string(const sbuilder* builder)
{
    return (const char*)builder->mem;
}


#endif // STRINGBUILDER_H