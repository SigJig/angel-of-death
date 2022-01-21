
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "lexer.h"
#include "errcodes.h"

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

parser_result parser_parse(lex_token* tokens);


#endif // PARSER_H