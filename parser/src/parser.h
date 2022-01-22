
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "lexer.h"
#include "statuscode.h"
#include "errhandler.h"

#define HAS_XREF    (1 << 0)
#define HAS_LINEVAL (1 << 1)

#define LEVEL_MAX 99
#define LEVEL_INVALID -1

struct parser_line
{
    struct lex_token* level;
    struct lex_token* xref;
    struct lex_token* tag;
    struct lex_token* line_value;

    struct parser_line* next;
};

struct parser_result
{
    struct parser_line* front;
    struct parser_line* back;

    struct err_handler ehandler;
};

struct parser
{
    int stack[LEVEL_MAX + 1];
    size_t stack_idx;

    struct
    {
        int index;
        struct parser_line* cur_line;
    } state;

    struct err_handler* ehandler;
    struct parser_result result;
};

void parser_line_free(struct parser_line*);
void parser_result_destroy(struct parser_result* result);

e_statuscode parser_parse_token(struct parser* parser, struct lex_token* token);
struct parser_result parser_parse(struct lex_token* tokens, struct err_handler* ehandler);

#endif // PARSER_H