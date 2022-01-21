
#include "parser.h"

#define LEVEL_MAX 99
#define LEVEL_INVALID -1

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
} parser_t;


static e_status parser_init(parser_t* parser)
{
    if (!parser) return ST_INIT_FAIL;

    parser->tokens = NULL;
    parser->stack_idx = 0;

    parser->state.cur_line = NULL;
    parser->state.index = 0;
}

static void parser_destroy(parser_t* parser)
{
    if (!parser) return;

    parser->tokens = NULL;
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
