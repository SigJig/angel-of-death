
#include "parser.h"


parser_result parser_parse(lex_token* tokens)
{
    parser_result result = {.lines = NULL};

    if (!tokens) return result;

    _parser_t parser;

    if (!_parser_init(&parser))
    {
        _parser_destroy(&parser);

        return result;
    }

    _parser_destroy(&parser);
}


// Internal functions

e_status _parser_init(_parser_t* parser)
{
    if (!parser) return ST_INIT_FAIL;

    parser->tokens = NULL;
    parser->stack_idx = 0;

    parser->state.cur_line = NULL;
    parser->state.index = 0;
}

void _parser_destroy(_parser_t* parser)
{
    if (!parser) return;

    parser->tokens = NULL;
}