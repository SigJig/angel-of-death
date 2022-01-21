
#include "parser.h"

#define LEVEL_MAX 99
#define LEVEL_INVALID -1

typedef struct
{
    int stack[LEVEL_MAX + 1];
    size_t stack_idx;

    struct
    {
        int index;
        parser_line* cur_line;
    } state;
} parser_t;

static bool is_tag(lex_token_t type)
{
    return type == LT_S_ALNUM || type == LT_NUMBER;
}

static e_status parser_init(parser_t* parser)
{
    if (!parser) return ST_INIT_FAIL;

    parser->stack_idx = 0;

    parser->state.cur_line = NULL;
    parser->state.index = 0;
}

static void parser_destroy(parser_t* parser)
{
    if (!parser) return;
}

static bool parser_valid_at_idx(parser_t* parser, lex_token* token)
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

e_status parser_parse_token(parser_t* parser, lex_token* token)
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

parser_result parser_parse(lex_token* tokens)
{
    parser_result result = {.lines = NULL};

    if (!tokens) return result;

    parser_t parser;

    if (!parser_init(&parser))
    {
        parser_destroy(&parser);

        return result;
    }

    parser_destroy(&parser);
}
