
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "errcodes.h"

#define LEVEL_MAX 99
#define LEVEL_INVALID -1

typedef struct parser_line
{
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
    int stack[MAX_LEVEL + 1];
    size_t stack_idx;
    
    lex_token* tokens;
} _parser_t;

parser_result parser_parse(lex_token* tokens);


// Internal functions

e_status _parser_init(_parser_t* parser);

// will not free tokens!
void _parser_destroy(_parser_t* parser);

#endif // PARSER_H