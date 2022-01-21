
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "lexer.h"
#include "errcodes.h"

#define LEVEL_MAX 99
#define LEVEL_INVALID -1

typedef struct parser_line
{
    enum
    {
        NONE = 0,
        XREF =      1 << 0,
        LINEVAL =   1 << 1
    } optional; // Determines whether the line contains optional xref and option line value
    int line;
    char* xref;
    char* tag;
    char* line_value;

    struct parser_line* next;
} parser_line;

typedef struct
{
    parser_line* lines;
} parser_result;

typedef struct
{
    int stack[LEVEL_MAX + 1];
    size_t stack_idx;
    
    lex_token* tokens;

    struct
    {
        int index;
        parser_line* cur_line;
    } state;
} _parser_t;

parser_result parser_parse(lex_token* tokens);


// Internal functions

e_status _parser_init(_parser_t* parser);

// will not free tokens!
void _parser_destroy(_parser_t* parser);

#endif // PARSER_H