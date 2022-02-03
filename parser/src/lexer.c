
#include "lexer.h"
#include <assert.h>
#include <stdarg.h>

// Begin tokenizing functions
static bool
is_digit(char c)
{
    return '0' <= c && c <= '9';
}

static bool
is_alpha(char c)
{
    return (c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static bool
is_alnum(char c)
{
    return is_digit(c) || is_alpha(c);
}

static bool
is_otherchar(char c)
{
    return ('!' <= c && c <= '"' || '$' <= c && c <= '/' ||
            ':' <= c && c <= '?' || '[' <= c && c <= '^' ||
            '{' <= c && c <= '~' || '\x80' <= c && c <= '\xFE' || c == '`');
}

static bool
is_space(char c)
{
    return c == '\x20';
}

static bool
is_delim(char c)
{
    return is_space(c);
}

static bool
is_eof(char c)
{
    return c == EOF;
}

static bool
is_terminator(char c)
{
    return is_delim(c) || c == '\n' || c == '\r' || is_eof(c);
}

static bool
is_hash_tag(char c)
{
    return c == '#';
}

static bool
is_at(char c)
{
    return c == '@';
}

static bool
is_anychar_noat(char c)
{
    return (is_alnum(c) || is_otherchar(c) || is_space(c) || is_hash_tag(c));
}

static bool
is_car_ret(char c)
{
    return c == '\r';
}

static bool
is_line_feed(char c)
{
    return c == '\n';
}

// Number of @s at the back of an sbuilder
static int
num_at_back(struct sbuilder* builder)
{
    if (!builder->len)
        return 0;

    int n = 0;

    for (int i = builder->len - 1; i >= 0 && is_at(builder->mem[i]); i--) {
        n++;
    }

    return n;
}

static struct lex_token*
lex_add_token(struct lex_lexer* lexer, lex_token_type type, char* lexeme)
{
    struct lex_token* newtok = malloc(sizeof *newtok);

    if (!newtok)
        return NULL;

    if (lexer->token_last)
        lexer->token_last->next = newtok;

    if (!lexer->token_first)
        lexer->token_first = newtok;

    lexer->token_last = newtok;

    newtok->type = type;
    newtok->lexeme = lexeme;
    newtok->line = lexer->tokline;
    newtok->col = lexer->tokcol;
    newtok->next = NULL;

    return newtok;
}

static void
lex_reset_state(struct lex_lexer* lexer)
{
    for (int i = 0; i < (int)LT_INVALID; i++) {
        lexer->state.possible_types[i] = (lex_token_type)i;
        lexer->state.status[i] = LV_NOT;
    }

    lexer->state.possible_length = (int)LT_INVALID;
}

static void
lex_tok_complete(struct lex_lexer* lexer, lex_token_type type)
{
    lex_add_token(lexer, type, sbuilder_clear(&lexer->state.builder));
    lex_reset_state(lexer);
}

static lex_e_valid
lex_validate(struct lex_lexer* lexer, lex_token_type type, char c)
{
    struct sbuilder* builder = &lexer->state.builder;

    switch (type) {
    case LT_NUMBER:
        return is_digit(c) ? LV_DONE_WHEN_DELIM : LV_NOT;
    case LT_POINTER: {
        // pointer: @ alnum non_at* @
        switch (builder->len) {
        case 0:
            return is_at(c) ? LV_CONT : LV_NOT;
        case 1:
            return is_alnum(c) ? LV_CONT : LV_NOT;
        default: {
            if (is_anychar_noat(c))
                return LV_CONT;
            else if (!is_at(c))
                return LV_NOT;

            return LV_DONE;
        }
        }
    }
    case LT_ESCAPE: {
        // escape: @ # anychar+ @ non_at
        switch (builder->len) {
        case 0:
            return is_at(c) ? LV_CONT : LV_NOT;
        case 1:
            return is_hash_tag(c) ? LV_CONT : LV_NOT;
        case 2:
            return is_anychar_noat(c) ? LV_CONT : LV_NOT;
        default: {
            int consec_ats = num_at_back(builder);

            if (is_at(c)) {
                if (is_at(lexer->lookahead)) {
                    return LV_CONT;
                }

                return ((consec_ats & 1) == 0) ? LV_DONE : LV_NOT;
            }

            // when reaching this point, an odd amount of @'s means
            // that this is the final non_a
            if (consec_ats & 1) {
                return is_anychar_noat(c) ? LV_DONE : LV_NOT;
            }

            return is_anychar_noat(c) ? LV_CONT : LV_NOT;
        }
        }
    }
    case LT_TERMINATOR: {
        char back = sbuilder_back(builder);

        if (is_car_ret(c)) {
            if (is_line_feed(back))
                return LV_DONE; // if it can become \n\r
            else if (builder->len)
                return LV_NOT;

            return LV_DONE_WHEN_NOT;
        } else if (is_line_feed(c)) {
            if (is_car_ret(back))
                return LV_DONE;
            else if (builder->len)
                return LV_NOT;

            return LV_DONE_WHEN_NOT;
        }

        return LV_NOT;
    }
    case LT_S_ALNUM:
        return is_alnum(c) ? LV_DONE_WHEN_DELIM : LV_NOT;
    case LT_S_ANYCHAR: {
        int consec_ats = num_at_back(builder);

        if (is_at(c)) {
            if (!is_at(lexer->lookahead) && (consec_ats & 1)) {
                return LV_CONT;
            }
        } else if (consec_ats & 1)
            return LV_NOT;

        return is_anychar_noat(c) ? LV_DONE_WHEN_DELIM : LV_NOT;
    }

    case LT_DELIM:
        return is_space(c) ? LV_DONE : LV_NOT;
    case LT_WHITESPACE:
        return (is_space(c) || c == '\t') ? LV_DONE : LV_NOT;
    case LT_EOF:
        return is_eof(c) ? LV_DONE : LV_NOT;
    default: {
        assert(false /*unrecognized type*/);
        return LV_NOT;
    }
    }
}

// does not currently process current character if done by not
static e_statuscode
lex_advance(struct lex_lexer* lexer)
{
    char c = lexer->current;

    for (int i = 0; i < (int)LT_INVALID; i++) {
        lex_token_type type = (lex_token_type)i;

        if (lexer->state.possible_types[i] == LT_INVALID)
            continue;

        lex_e_valid* status_cache = &lexer->state.status[i];
        lex_e_valid status = lex_validate(lexer, type, c);

        switch (status) {
        case LV_NOT: {
            if ((*status_cache == LV_DONE_WHEN_NOT) ||
                (*status_cache == LV_DONE_WHEN_DELIM && is_terminator(c))) {

                lex_tok_complete(lexer, type);
                e_statuscode nxt_status = lex_advance(lexer);

                if (!(nxt_status == ST_OK || nxt_status == ST_NOT_OK)) {
                    return nxt_status;
                }

                return ST_OK;
            }

            lexer->state.possible_types[i] = LT_INVALID;
            lexer->state.possible_length--;

            continue;
        }
        case LV_CONT: {
            continue;
        }
        case LV_DONE: {
            sbuilder_write_char(&lexer->state.builder, c);
            lex_tok_complete(lexer, type);

            return ST_OK;
        }
        case LV_DONE_WHEN_NOT: {
            *status_cache = LV_DONE_WHEN_NOT;

            continue;
        }
        case LV_DONE_WHEN_DELIM: {
            *status_cache = LV_DONE_WHEN_DELIM;

            continue;
        }
        default: {
            assert(false /*unrecognized status*/);
            return ST_GEN_ERROR;
        }
        }
    }

    if (!lexer->state.possible_length) {
        ctx_critf(
            lexer->ctx,
            "unexpected character '%c' encountered (at line %zu, column %zu)",
            c, lexer->curline, lexer->curcol);

        sbuilder_write_char(&lexer->state.builder, c);

        lex_add_token(lexer, LT_INVALID, sbuilder_clear(&lexer->state.builder));
        lex_reset_state(lexer);

        return ST_OK;
    }

    sbuilder_write_char(&lexer->state.builder, c);

    return ST_NOT_OK;
}

/*
=================================================
END INTERNAL
=================================================
*/

struct lex_token*
lex_token_copy(struct lex_token* token)
{
    if (!token) {
        return NULL;
    }

    struct lex_token* copy = malloc(sizeof *copy);
    copy->type = token->type;
    copy->lexeme = strdup(token->lexeme);
    copy->line = token->line;
    copy->col = token->col;

    copy->next = NULL;

    return copy;
}

void
lex_token_free(struct lex_token* token)
{
    if (!token)
        return;

    // lexeme has been made by stringbuilder and must be free after use
    if (token->lexeme) {
        free(token->lexeme);
        token->lexeme = NULL;
    }

    free(token);
}

struct lex_lexer*
lex_create(struct context* ctx)
{
    struct lex_lexer* lexer = malloc(sizeof *lexer);

    if (lex_init(lexer, ctx) != ST_OK) {
        free(lexer);

        return NULL;
    }

    return lexer;
}

e_statuscode
lex_init(struct lex_lexer* lexer, struct context* ctx)
{
    lexer->state.builder = sbuilder_new();
    lexer->buf = sbuilder_new();

    if (!ctx) {
        return ST_INIT_FAIL;
    }

    lexer->ctx = ctx;
    lexer->eof_reached = false;
    lexer->current = '\0';
    lexer->lookahead = '\0';
    lexer->tokline = 1;
    lexer->tokcol = 1;
    lexer->curline = 1;
    lexer->curcol = 1;
    lexer->token_first = NULL;
    lexer->token_last = NULL;

    lex_reset_state(lexer);

    return ST_OK;
}

void
lex_free(struct lex_lexer* lexer)
{
    if (!lexer)
        return;

    lex_destroy(lexer);
    free(lexer);
}

void
lex_destroy(struct lex_lexer* lexer)
{
    struct lex_token* tok = lexer->token_first;
    struct lex_token* tmp = NULL;

    while (tok != NULL) {
        tmp = tok;
        tok = tok->next;
        lex_token_free(tmp);
    }

    lexer->token_first = NULL;
    lexer->token_last = NULL;

    sbuilder_destroy(&lexer->state.builder);
    sbuilder_destroy(&lexer->buf);

    lexer->ctx = NULL;
}

e_statuscode
lex_feed(struct lex_lexer* lexer, char c)
{
    if (lexer->eof_reached) {
        return ST_NOT_OK;
    }

    lexer->current = lexer->lookahead;

    if (c == EOF || c == '\0') {
        lexer->eof_reached = true;
        lexer->lookahead = EOF;
    } else {
        lexer->lookahead = c;
    }

    if (lexer->current == '\0') {
        return ST_NOT_INIT;
    }

    e_statuscode result = lex_advance(lexer);

    // final call
    if (lexer->eof_reached) {
        lexer->current = lexer->lookahead;
        lexer->lookahead = EOF;

        e_statuscode nxtresult = lex_advance(lexer);

        result = (nxtresult > result) ? nxtresult : result;
    }

    if (result != ST_NOT_INIT) {
        if (lexer->current == '\n') {
            lexer->curline++;
            lexer->curcol = 1;
        } else {
            lexer->curcol++;
        }

        if (result == ST_OK) {
            lexer->tokline = lexer->curline;
            lexer->tokcol = lexer->curcol;

            posctx_update(lexer->ctx, lexer->tokline, lexer->tokcol);
        }
    }

    return result;
}
