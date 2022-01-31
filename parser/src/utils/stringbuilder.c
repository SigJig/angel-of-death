
#include "utils/stringbuilder.h"
#include "utils/statuscode.h"
#include <assert.h>
#include <stdio.h>

static int
sbuilder_reserve(struct sbuilder* builder, size_t length)
{
    assert(builder->cap);
    assert(SBUILDER_DEFAULT_CAP_MULT /* sbuilder default cap mult not set */);

    builder->len += length;

    if (builder->len >= builder->cap) {
        builder->cap *=
            SBUILDER_DEFAULT_CAP_MULT * ((builder->len / builder->cap) +
                                         (builder->len % builder->cap != 0));

        if (builder->mem) {
            size_t new_size = (sizeof *builder->mem) * (builder->cap + 1);

            builder->mem = realloc(builder->mem, new_size);

            if (!builder->mem) {
                return ST_MALLOC_ERROR;
            }

            memset(builder->mem + builder->len, 0,
                   builder->cap + 1 - builder->len);
        }
    }

    return ST_OK;
}

struct sbuilder
sbuilder_new()
{
    assert(SBUILDER_DEFAULT_CAP /* SBUILDER_DEFAULT_CAP not set */);
    struct sbuilder builder;

    if (sbuilder_init(&builder, SBUILDER_DEFAULT_CAP) != ST_OK) {
        assert(false);
    }

    return builder;
}

int
sbuilder_init(struct sbuilder* builder, size_t cap)
{
    builder->cap = cap;
    builder->len = 0;

    if (cap < 1) {
        builder->mem = NULL;
        return 1;
    } else {
        builder->mem = calloc(cap + 1, sizeof *builder->mem);

        if (!builder->mem)
            return 2;
    }

    return 0;
}

void
sbuilder_destroy(struct sbuilder* builder)
{
    if (!builder->mem)
        return;

    free(builder->mem);
    builder->mem = NULL;
}

char*
sbuilder_clear(struct sbuilder* builder)
{
    if (!builder->mem)
        return NULL;

    char* ret = strdup(builder->mem);

    builder->len = 0;
    memset(builder->mem, 0, builder->cap + 1);

    return ret;
}

char*
sbuilder_term(struct sbuilder* builder)
{
    if (!builder->mem)
        return NULL;

    char* ret = strdup(builder->mem);

    sbuilder_destroy(builder);

    return ret;
}

int
sbuilder_write(struct sbuilder* builder, const char* addition)
{
    if (!builder->cap)
        return 2;

    int length = strlen(addition);
    int initial_length = builder->len;

    int verified = sbuilder_reserve(builder, length);
    if (verified != 0)
        return verified;

    strncpy(builder->mem + initial_length, addition, length);

    return 0;
}

int
sbuilder_vwritef(struct sbuilder* builder, const char* fmt, va_list args)
{
    if (!builder->cap)
        return 2;

    va_list copy;
    va_copy(copy, args);

    int required = vsnprintf(NULL, 0, fmt, args) + 1;

    if (required <= 0)
        return 2;

    char* mem = calloc(required, sizeof *mem);

    if (!mem)
        return 1;

    vsprintf(mem, fmt, copy);

    va_end(copy);

    int result = sbuilder_write(builder, mem);

    free(mem);

    return result;
}

int
sbuilder_writef(struct sbuilder* builder, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int result = sbuilder_vwritef(builder, fmt, args);

    va_end(args);

    return result;
}

int
sbuilder_write_char(struct sbuilder* builder, char c)
{
    if (!builder->cap)
        return 2;

    int initial_length = builder->len;

    int verified = sbuilder_reserve(builder, 1);
    if (verified != 0)
        return verified;

    builder->mem[initial_length] = c;

    return 0;
}

char
sbuilder_back(struct sbuilder* builder)
{
    if (!builder->mem || !builder->len)
        return '\0';

    return builder->mem[builder->len - 1];
}

const char*
sbuilder_to_string(const struct sbuilder* builder)
{
    return (const char*)builder->mem;
}