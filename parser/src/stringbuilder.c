
#include "stringbuilder.h"

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
    builder->mem = NULL;
}

void sbuilder_clear(sbuilder *builder)
{
    builder->len = 0;
    memset(builder->mem, 0, builder->cap + 1);
}

int _sbuilder_verify_cap(sbuilder* builder)
{
    if (!builder->cap) return 2;

    if (builder->len >= builder->cap)
    {
        builder->cap *= SBUILDER_DEFAULT_CAP_MULT * ((builder->len / builder->cap) + (builder->len % builder->cap != 0));
        
        if (builder->mem)
        {
            builder->mem = (char*)realloc(builder->mem, sizeof(char) * (builder->cap + 1));
            
            if (!builder->mem)
                return 1;

            memset(builder->mem + builder->len, 0, builder->cap + 1 - builder->len);
        }
    }

    return 0;
}

int sbuilder_write(sbuilder* builder, const char* addition)
{
    if (!builder->cap)
        return 2;

    int length = strlen(addition);
    int initial_length = builder->len;
    builder->len += length;

    int verified = _sbuilder_verify_cap(builder);
    if (verified != 0) return verified;

    strncpy(builder->mem + initial_length, addition, length);

    return 0;
}

int sbuilder_write_char(sbuilder* builder, char c)
{
    if (!builder->cap)
        return 2;

    int initial_length = builder->len;
    builder->len += 1;

    int verified = _sbuilder_verify_cap(builder);
    if (verified != 0) return verified;

    builder->mem[initial_length] = c;

    return 0;
}

char sbuilder_back(sbuilder* builder)
{
    if (!builder->mem || !builder->len) return '\0';

    return builder->mem[builder->len - 1];
}

const char* sbuilder_to_string(const sbuilder* builder)
{
    return (const char*)builder->mem;
}

char* sbuilder_return(sbuilder* builder)
{
    if (!builder->mem)
        return NULL;

    char* ret = (char*)calloc(builder->len + 1, sizeof ret);
    strncpy(ret, builder->mem, builder->len);

    sbuilder_clear(builder);

    return ret;
}
