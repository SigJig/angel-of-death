
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "context/context.h"
#include "context/genstate.h"
#include "lexer.h"
#include "utils/statuscode.h"

#define LEVEL_MAX 99
#define LEVEL_INVALID -1

struct parser_line {
    struct lex_token* level;
    struct lex_token* xref;
    struct lex_token* tag;
    struct lex_token* line_value;

    struct parser_line* next;
};

struct parser_result {
    struct parser_line* front;
    struct parser_line* back;
};

struct parser {
    int stack[LEVEL_MAX + 1];
    size_t stack_idx;

    struct {
        int index;
        struct parser_line* cur_line;
    } state;

    struct context* ctx;
    struct parser_result result;
};

void parser_line_free(struct parser_line* line);
char* parser_line_to_string(struct parser_line* line);

void parser_result_destroy(struct parser_result* result);

e_statuscode parser_parse_token(struct parser* parser, struct lex_token* token);
struct parser_result parser_parse(struct lex_token* tokens,
                                  struct context* ctx);

#endif // PARSER_H