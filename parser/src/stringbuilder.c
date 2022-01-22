
#include <stdio.h>
#include "stringbuilder.h"

static int sbuilder_verify_cap(sbuilder* builder)
{
    if (!builder->cap) return 2;

    if (builder->len >= builder->cap)
    {
        builder->cap *= SBUILDER_DEFAULT_CAP_MULT * ((builder->len / builder->cap) + (builder->len % builder->cap != 0));
        
        if (builder->mem)
        {
            builder->mem = (char*)realloc(builder->mem, (sizeof *builder->mem) * (builder->cap + 1));
            
            if (!builder->mem)
                return 1;

            memset(builder->mem + builder->len, 0, builder->cap + 1 - builder->len);
        }
    }

    return 0;
}

int sbuilder_init(sbuilder* builder, size_t cap)
{
    builder->cap = cap;
    builder->len = 0;

    if (cap < 1)
    {
        builder->mem = NULL;
        return 1;
    }
    else
    {
        builder->mem = (char*)calloc(cap + 1, sizeof *builder->mem);

        if (!builder->mem)
            return 2;
    }

    return 0;
}

void sbuilder_destroy(sbuilder* builder)
{
    if (!builder->mem)
        return;

    free(builder->mem);
    builder->mem = NULL;
}

void sbuilder_clear(sbuilder *builder)
{
    builder->len = 0;
    memset(builder->mem, 0, builder->cap + 1);
}

int sbuilder_write(sbuilder* builder, const char* addition)
{
    if (!builder->cap)
        return 2;

    int length = strlen(addition);
    int initial_length = builder->len;
    builder->len += length;

    int verified = sbuilder_verify_cap(builder);
    if (verified != 0) return verified;

    strncpy(builder->mem + initial_length, addition, length);

    return 0;
}

int sbuilder_vwritef(sbuilder* builder, const char* fmt, va_list args)
{
    if (!builder->cap) return 2;

    va_list copy;
    va_copy(copy, args);

    int required = vsnprintf(NULL, 0, fmt, args) + 1;

    if (required <= 0) return 2;

    char* mem = (char*)calloc(required, sizeof *mem);

    if (!mem)
        return 1;

    vsprintf(mem, fmt, copy);

    va_end(copy);

    int result = sbuilder_write(builder, mem);

    free(mem);

    return result;
}

int sbuilder_writef(sbuilder* builder, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int result = sbuilder_vwritef(builder, fmt, args);

    va_end(args);

    return result;
}

int sbuilder_write_char(sbuilder* builder, char c)
{
    if (!builder->cap)
        return 2;

    int initial_length = builder->len;
    builder->len += 1;

    int verified = sbuilder_verify_cap(builder);
    if (verified != 0) return verified;

    builder->mem[initial_length] = c;

    return 0;
}

char sbuilder_back(sbuilder* builder)
{
    if (!builder->mem || !builder->len) return '\0';

    return builder->mem[builder->len - 1];
}

int sbuilder_num_consec(sbuilder* builder, bool (*func)(char), bool back)
{
    if (!builder->len) return 0;
    
    int n = 0;

    if (back)
        for (int i = builder->len - 1; i >= 0 && func(builder->mem[i]); i--) n++;
    else
        for (int i = 0; i < builder->len && func(builder->mem[i]); i++) n++;
    
    return n;
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

    sbuilder_clear(builder);

    return ret;
}

char* sbuilder_complete(sbuilder* builder)
{
    if (!builder->mem) return NULL;

    char* ret = strdup(builder->mem);

    sbuilder_destroy(builder);

    return ret;
}