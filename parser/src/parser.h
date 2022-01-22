
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "lexer.h"
#include "statuscode.h"

#define HAS_XREF    (1 << 0)
#define HAS_LINEVAL (1 << 1)



struct parser_line
{
    int optional;
    int line;
    char* xref;
    char* tag;
    char* line_value;

    struct parser_line* next;
};

struct parser_result
{
    struct parser_line* lines;
};

//e_statuscode parser_parse_token(struct lex_token* token);
struct parser_result parser_parse(struct lex_token* tokens);

#endif // PARSER_H