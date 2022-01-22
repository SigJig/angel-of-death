
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "lexer.h"
#include "statuscode.h"

#define HAS_XREF    (1 << 0)
#define HAS_LINEVAL (1 << 1)



typedef struct parser_line
{
    int optional;
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

//e_status parser_parse_token(lex_token* token);
parser_result parser_parse(lex_token* tokens);

#endif // PARSER_H