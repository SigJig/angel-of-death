
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
void sbuilder_reset_mem(sbuilder* builder);

int sbuilder_add(sbuilder* builder, const char* addition);

const char* sbuilder_to_string(const sbuilder* builder);

// Frees the sbuilder to be reused, and then returns a heap allocated char* (must be freed)
char* sbuilder_return(sbuilder* builder);

int sbuilder_init(sbuilder* builder, size_t cap)
{
    builder->cap = cap;
    builder->len = 0;

    if (cap < 1)
    {
        builder->mem = NULL;
    }
    else
    {
        builder->mem = (char*)calloc(cap + 1, sizeof(char));

        if (!builder->mem)
            return 2;
    }

    return 0;
}

void sbuilder_free(sbuilder* builder)
{
    if (!builder->mem)
        return;

    free(builder->mem);
}

void sbuilder_reset_mem(sbuilder *builder)
{
    memset(builder->mem, 0, builder->cap);
}

int sbuilder_add(sbuilder* builder, const char* addition)
{
    int length = strlen(addition);
    int initial_length = builder->len;
    builder->len += length;

    if (builder->len >= (builder->cap - 1))
    {
        builder->cap *= DEFAULT_CAP_MULT * ((builder->len / builder->cap) + (builder->len % builder->cap != 0));
        
        if (builder->mem)
        {
            builder->mem = (char*)realloc(builder->mem, sizeof(char) * builder->cap);
            
            if (!builder->mem)
                return 1;

            memset(builder->mem + builder->len, 0, builder->cap - builder->len);
        }
        else
        {
            // If memory is NULL (cap was 0 at initialization)
            builder->mem = (char*)malloc(sizeof(char) * builder->cap);
            sbuilder_reset_mem(builder);
        }
    }

    memcpy(builder->mem + initial_length, addition, length);

    return 0;
}

const char* sbuilder_to_string(const sbuilder* builder)
{
    return (const char*)builder->mem;
}

char* sbuilder_return(sbuilder* builder)
{
    if (!builder->mem)
        return NULL;

    char* ret = strdup(builder->mem);

    sbuilder_reset_mem(builder);

    return ret;
}


#endif // STRINGBUILDER_H