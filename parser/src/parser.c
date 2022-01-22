
#include "parser.h"

#define LEVEL_MAX 99
#define LEVEL_INVALID -1

struct parser
{
    int stack[LEVEL_MAX + 1];
    size_t stack_idx;

    struct
    {
        int index;
        struct parser_line* cur_line;
    } state;
};

static bool is_tag(lex_token_type type)
{
    return type == LT_S_ALNUM || type == LT_NUMBER;
}

static e_statuscode parser_init(struct parser* parser)
{
    if (!parser) return ST_INIT_FAIL;

    parser->stack_idx = 0;

    parser->state.cur_line = NULL;
    parser->state.index = 0;
}

static void parser_destroy(struct parser* parser)
{
    if (!parser) return;
}

static bool parser_valid_at_idx(struct parser* parser, struct lex_token* token)
{
    int index = parser->state.index;

    // gedcom_line: level delim [xref delim] tag [delim line_value] terminator
    switch (index)
    {
        case 0: return (token->type == LT_NUMBER);
        case 2: return (token->type == LT_POINTER);
        case 4: return (token->type == LT_S_ALNUM || token->type == LT_NUMBER);
        case 6: return (token->type == LT_POINTER || token->type == LT_ESCAPE || token->type == LT_S_ANYCHAR || token->type == LT_S_ALNUM || token->type == LT_NUMBER);
    }
}

/*
=================================================
END INTERNAL
=================================================
*/

e_statuscode parser_parse_token(struct parser* parser, struct lex_token* token)
{
    int index = parser->state.index;

    if (index < 6)
    {
        if (index & 1) return ST_NOT_OK;
    }
    else if (index > 6 && token->type == LT_TERMINATOR)
    {
        return ST_OK;
    }

    return parser_valid_at_idx(parser, token) ? ST_NOT_OK : ST_GEN_ERROR;
}

struct parser_result parser_parse(struct lex_token* tokens)
{
    struct parser_result result = {.lines = NULL};

    if (!tokens) return result;

    struct parser parser;

    if (!parser_init(&parser))
    {
        parser_destroy(&parser);

        return result;
    }

    parser_destroy(&parser);
}
