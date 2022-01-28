
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
is_terminator(char c)
{
    return is_delim(c) || c == '\n' || c == '\r';
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

static struct err_message*
lex_errf(struct lex_lexer* lexer, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    struct err_message* result =
        ehandler_verrf(lexer->ehandler, "lexer", format, args);
    va_end(args);

    return result;
}

static struct err_message*
lex_err_unexpected(struct lex_lexer* lexer, char c)
{
    return lex_errf(lexer, "unexpected character (line: %zu, col: %zu): %c",
                    lexer->line, lexer->col, c);
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
    newtok->line = lexer->line;
    newtok->col = lexer->col;
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
            int consec = sbuilder_num_consec(builder, is_at, true);

            if (is_at(c)) {
                if (is_at(lexer->lookahead)) {
                    return LV_CONT;
                }

                return ((consec & 1) == 0) ? LV_DONE : LV_NOT;
            }

            // when reaching this point, an odd amount of @'s means
            // that this is the final non_a
            if (consec & 1) {
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
        int consec_at = sbuilder_num_consec(builder, is_at, true);

        if (is_at(c)) {
            if (!is_at(lexer->lookahead) && (consec_at & 1))

                return LV_CONT;
        } else if (consec_at & 1)
            return LV_NOT;

        return is_anychar_noat(c) ? LV_DONE_WHEN_DELIM : LV_NOT;
    }

    case LT_DELIM:
        return is_space(c) ? LV_DONE : LV_NOT;
    case LT_WHITESPACE:
        return (is_space(c) || c == '\t') ? LV_DONE : LV_NOT;
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
        bool write = false;

        // if eof has been reached, return the first possible
        if (lexer->eof_reached) {
            goto done;
        }

        lex_e_valid status = lex_validate(lexer, type, c);

        switch (status) {
        case LV_NOT: {
            if ((*status_cache == LV_DONE_WHEN_NOT) ||
                (*status_cache == LV_DONE_WHEN_DELIM && is_terminator(c))) {

                goto done;
            }

            goto remove;
        }
        case LV_CONT: {
            goto nothing;
        }
        case LV_DONE: {
            write = true;
            goto done;
        }
        case LV_DONE_WHEN_NOT: {
            *status_cache = LV_DONE_WHEN_NOT;

            goto nothing;
        }
        case LV_DONE_WHEN_DELIM: {
            *status_cache = LV_DONE_WHEN_DELIM;

            goto nothing;
        }
        default: {
            assert(false /*unrecognized status*/);
            return ST_GEN_ERROR;
        }
        }

    nothing:
        continue;

    remove:
        lexer->state.possible_types[i] = LT_INVALID;
        lexer->state.possible_length--;

        continue;
    done:
        if (write) {
            sbuilder_write_char(&lexer->state.builder, c);
        }

        lex_add_token(lexer, type, sbuilder_return(&lexer->state.builder));
        lex_reset_state(lexer);

        if (!write && !lexer->eof_reached) {
            e_statuscode nxt_status = lex_advance(lexer);

            if (!(nxt_status == ST_OK || nxt_status == ST_NOT_OK)) {
                return nxt_status;
            }
        }

        return ST_OK;
    }

    if (!lexer->state.possible_length) {
        lex_err_unexpected(lexer, c);
        return ST_GEN_ERROR;
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
lex_create(struct err_handler* ehandler)
{
    struct lex_lexer* lexer = malloc(sizeof *lexer);

    if (lex_init(lexer, ehandler) != ST_OK) {
        free(lexer);

        return NULL;
    }

    return lexer;
}

e_statuscode
lex_init(struct lex_lexer* lexer, struct err_handler* ehandler)
{
    if (sbuilder_init(&lexer->state.builder, SBUILDER_DEFAULT_CAP) != 0) {
        return ST_INIT_FAIL;
    }

    if (sbuilder_init(&lexer->buf, SBUILDER_DEFAULT_CAP) != 0) {
        return ST_INIT_FAIL;
    }

    if (!ehandler) {
        return ST_INIT_FAIL;
    }

    lexer->ehandler = ehandler;
    lexer->eof_reached = false;
    lexer->current = '\0';
    lexer->lookahead = '\0';
    lexer->line = 1;
    lexer->col = 1;
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
    lexer->ehandler = NULL;

    sbuilder_destroy(&lexer->state.builder);
    sbuilder_destroy(&lexer->buf);
}

e_statuscode
lex_feed(struct lex_lexer* lexer, char c)
{
    if (lexer->eof_reached) {
        return ST_NOT_OK;
    }

    lexer->current = lexer->lookahead;
    bool eof = false;

    if (c == EOF || c == '\0') {
        eof = true;
        lexer->lookahead = '\0';
    } else {
        lexer->lookahead = c;
    }

    if (lexer->current == '\0') {
        return ST_NOT_INIT;
    }

    e_statuscode result = lex_advance(lexer);

    lexer->eof_reached = eof;

    // final call
    if (lexer->eof_reached) {
        lexer->current = lexer->lookahead;
        lexer->lookahead = '\0';

        e_statuscode nxtresult = lex_advance(lexer);

        result = (nxtresult > result) ? nxtresult : result;
    }

    if (result != ST_NOT_INIT) {
        if (lexer->current == '\n') {
            lexer->line++;
            lexer->col = 1;
        } else {
            lexer->col++;
        }
    }

    return result;
}
