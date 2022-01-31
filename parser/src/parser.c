
#include "parser.h"
#include "utils/stringbuilder.h"
#include <assert.h>
#include <stdarg.h>

static bool
is_terminator(struct lex_token* token)
{
    return token->type == LT_TERMINATOR || token->type == LT_EOF;
}

static struct parser_line*
parser_curline_reset(struct parser* parser)
{
    struct parser_line* old = parser->state.cur_line;
    struct parser_line* cur_line = malloc(sizeof *cur_line);

    cur_line->level = NULL;
    cur_line->xref = NULL;
    cur_line->tag = NULL;
    cur_line->line_value = NULL;
    cur_line->next = NULL;

    parser->state.cur_line = cur_line;
    parser->state.index = 0;

    return old;
}

static e_statuscode
parser_init(struct parser* parser, struct context* ctx)
{
    if (!parser || !ctx)
        return ST_INIT_FAIL;

    parser->stack_idx = 0;
    parser->result.front = NULL;
    parser->result.back = NULL;
    parser->ctx = ctx;

    parser_curline_reset(parser);

    parser->state.index = 0;

    return ST_OK;
}

static void
parser_destroy(struct parser* parser)
{
    if (!parser)
        return;

    if (parser->state.cur_line) {
        parser_line_free(parser->state.cur_line);
    }

    parser->ctx = NULL;
}

static bool
parser_valid_at_idx(struct parser* parser, struct lex_token* token, int index)
{
    // gedcom_line: level delim [xref delim] tag [delim line_value]
    // terminator
    switch (index) {
    case 0:
        return (token->type == LT_NUMBER);
    case 2:
        return (token->type == LT_POINTER);
    case 4:
        return (token->type == LT_S_ALNUM || token->type == LT_NUMBER);
    default:
        return !is_terminator(token);
    }

    return false;
}

static struct parser_line*
parser_curline_terminate(struct parser* parser)
{
    struct parser_line* line = parser_curline_reset(parser);

    if (parser->result.back) {
        (parser->result.back)->next = line;
        parser->result.back = line;
    } else {
        parser->result.front = line;
        parser->result.back = line;
    }

    return line;
}

static void
parser_update_at(struct parser* parser, struct lex_token* token, int index)
{
    struct parser_line* cur_line = parser->state.cur_line;
    struct lex_token* token_copy = lex_token_copy(token);

    switch (index) {
    case 0: {
        cur_line->level = token_copy;
        break;
    }
    case 2: {
        cur_line->xref = token_copy;
        break;
    }
    case 4: {
        cur_line->tag = token_copy;
        break;
    }
    default: {
        // The optional line_value can contain multiple values
        struct lex_token* back = cur_line->line_value;

        if (back) {
            for (; back->next != NULL; back = back->next) {
            }

            back->next = token_copy;
        } else {
            cur_line->line_value = token_copy;
        }
    }
    }
}

/*
=================================================
END INTERNAL
=================================================
*/

void
parser_line_free(struct parser_line* line)
{
    lex_token_free(line->level);
    lex_token_free(line->xref);
    lex_token_free(line->tag);

    struct lex_token* tok = line->line_value;
    struct lex_token* tmp = NULL;

    while (tok) {
        tmp = tok;
        tok = tok->next;

        lex_token_free(tmp);
    }

    free(line);
}

char*
parser_line_to_string(struct parser_line* line)
{
    struct sbuilder builder = sbuilder_new();

    sbuilder_writef(&builder, "<LINE %s:", line->level->lexeme);

    if (line->xref != NULL) {
        sbuilder_writef(&builder, " (%s)", line->xref->lexeme);
    }

    sbuilder_writef(&builder, " %s", line->tag->lexeme);

    if (line->line_value != NULL) {
        sbuilder_write(&builder, " = ");

        struct lex_token* tok = line->line_value;

        while (tok) {
            sbuilder_writef(&builder, "%s;", tok->lexeme);

            tok = tok->next;
        }
    }

    sbuilder_write(&builder, ">");

    return sbuilder_term(&builder);
}

void
parser_result_destroy(struct parser_result* result)
{
    struct parser_line* line = result->front;
    struct parser_line* tmp = NULL;

    while (line) {
        tmp = line;
        line = line->next;

        parser_line_free(tmp);
    }
}

/*
gedcom_line:
        0: level
        1: delim
        2: xref?
        3: delim?
        4: tag
        5: delim?
        6+: line_value?
        n: terminator
*/
e_statuscode
parser_parse_token(struct parser* parser, struct lex_token* token)
{
    int index = parser->state.index;

    // leading whitespace
    if (!index && (token->type == LT_WHITESPACE || token->type == LT_DELIM ||
                   is_terminator(token))) {
        return ST_NOT_OK;
    }

    parser->state.index++;

    // yes i know i can use !index u cunt
    if (index == 0) {
        posctx_update_line(parser->ctx, token->line);
    }

    posctx_update_col(parser->ctx, token->col);

    if (index < 6 && (index & 1)) {
        if (token->type == LT_DELIM) {
            return ST_NOT_OK;
        } else if (!(index == 5 && is_terminator(token))) {
            // If index 5 is not a delimeter, it must be a terminator or eof.

            ctx_critf(
                parser->ctx,
                "optional value must start with a delimeter, or be omitted "
                "completely with a terminator or end-of-file (expected type "
                "(%d, %d, %d), got %d)",
                (int)LT_DELIM, (int)LT_TERMINATOR, (int)LT_EOF,
                (int)token->type);

            return ST_GEN_ERROR;
        }
    }

    if (index > 4 && is_terminator(token)) {
        parser_curline_terminate(parser);
        return ST_OK;
    }

    if (!parser_valid_at_idx(parser, token, index)) {
        if (index != 2) {
            ctx_critf(parser->ctx, "unexpected type %d", (int)token->type);
            return ST_GEN_ERROR;
        }

        // xref, optional. if it does not match consider current token a tag
        // increment the index by 2 (has already been incremented by one at
        // the start of the function)
        parser->state.index++;

        return parser_parse_token(parser, token);
    }

    parser_update_at(parser, token, index);

    return ST_NOT_OK;
}

struct parser_result
parser_parse(struct lex_token* tokens, struct context* ctx)
{
    ctx_push(ctx, posctx_create("parser"));
    struct parser_result empty = {.front = NULL, .back = NULL};

    if (!tokens)
        return empty;

    struct parser parser;

    if (parser_init(&parser, ctx) != ST_OK) {
        parser_destroy(&parser);

        return empty;
    }

    for (struct lex_token* tok = tokens; tok; tok = tok->next) {
        parser_parse_token(&parser, tok);
    }

    parser_destroy(&parser);
    ctx_pop(ctx);

    return parser.result;
}
