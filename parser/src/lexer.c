
#include "lexer.h"

const lex_token_t lex_tok_cvtable[] = {
    LT_NUMBER,
    LT_POINTER,
    LT_TERMINATOR,
    LT_S_ALNUM,
    LT_ESCAPE_TEXT_NOAT,
    LT_AT,
    LT_DOUBLE_AT,
    LT_HASH_TAG,
    LT_DELIM,
    LT_INVALID
};

lex_token_t lex_token_t_from_int(int i)
{
    return (lex_token_t)i;
}

// Begin tokenizing functions
static bool _is_digit(char c)
{
    return '0' <= c && c <= '9';
}

static bool _is_alpha(char c)
{
    return (c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static bool _is_alnum(char c)
{
    return _is_digit(c) || _is_alpha(c);
}

static bool _is_otherchar(char c)
{
    return (
        '!' <= c && c <= '"' ||
        '$' <= c && c <= '/' ||
        ':' <= c && c <= '?' ||
        '[' <= c && c <= '^' ||
        '{' <= c && c <= '~' ||
        '\x80' <= c && c <= '\xFE' ||
        c == '`'
    );
}

static bool _is_space(char c) { return c == '\x20'; }
static bool _is_hash_tag(char c) { return c == '#'; }
static bool _is_at(char c) { return c == '@'; }

static bool _is_anychar_noat(char c)
{
    return (
        _is_alnum(c) ||
        _is_otherchar(c) ||
        _is_space(c) ||
        _is_hash_tag(c)
    );
}

static bool _is_car_ret(char c)
{
    return c == '\r';
}

static bool _is_line_feed(char c)
{
    return c == '\n';
}

static lex_token* lex_add_token(lex_lexer* lexer, lex_token_t type, char* lexeme)
{
    lex_token* newtok = (lex_token*)malloc(sizeof *newtok);

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

static void lex_del_token(lex_lexer* lexer, lex_token* token)
{
    if (!token) return;

    // lexeme has been made by stringbuilder and must be free after use
    if (token->lexeme)
    {
        free(token->lexeme);
        token->lexeme = NULL;
    }

    free(token);
}

static void lex_reset_state(lex_lexer* lexer)
{
    for (int i = 0; i < (int)LT_INVALID; i++)
    {
        lexer->state.possible_types[i] = (lex_token_t)i;
        lexer->state.status[i] = LV_NOT;
    }

    lexer->state.possible_length = (int)LT_INVALID;
}

static lex_e_valid lex_validate(lex_lexer* lexer, lex_token_t type, char c)
{
    sbuilder* builder = &lexer->state.builder;

    switch (type)
    {
        case LT_NUMBER:
            return _is_digit(c) ? LV_DONE_WHEN_NOT : LV_NOT;
        case LT_POINTER:
        {
            // pointer: @ alnum non_at* @
            switch (builder->len)
            {
                case 0: return (lex_e_valid)_is_at(c);
                case 1: return (lex_e_valid)_is_alnum(c);
                default:
                {
                    if (_is_anychar_noat(c)) return LV_CONT;
                    else if (!_is_at(c)) return LV_NOT;

                    return LV_DONE;
                }
            }
        }
        case LT_TERMINATOR:
        {
            char back = sbuilder_back(builder);

            if (_is_car_ret(c))
            {
                if (_is_line_feed(back)) return LV_DONE; // if it can become \n\r
                else if (builder->len) return LV_NOT;

                return LV_CONT;
            }
            else if (_is_line_feed(c))
            {
                if (_is_car_ret(back)) return LV_DONE;
                else if (builder->len) return LV_NOT;

                return LV_CONT;
            }

            return LV_NOT;
        }
        case LT_S_ALNUM:
            return _is_alnum(c) ? LV_DONE_WHEN_NOT : LV_NOT;
        case LT_ESCAPE_TEXT_NOAT: return _is_anychar_noat(c) ? LV_DONE_WHEN_NOT : LV_NOT;
        case LT_AT:
        {
            // if lookahead is alnum then this must be treated as a pointer
            if (!_is_at(c) || builder->len || _is_alnum(lexer->lookahead)) return LV_NOT;

            return _is_at(lexer->lookahead) ? LV_NOT : LV_DONE;
        }
        case LT_DOUBLE_AT:
        {
            if (!_is_at(c)) return LV_NOT;

            if (lexer->state.builder.len)
                return LV_DONE;

            return LV_CONT;
        }
        case LT_HASH_TAG: return _is_hash_tag(c) ? LV_DONE : LV_NOT;
        case LT_DELIM: return _is_space(c) ? LV_DONE : LV_NOT;
        default:
            // TODO: should warn
            return LV_NOT;
    }
}

// does not currently process current character if done by not
static e_status lex_advance(lex_lexer* lexer)
{
    // if eof has not been reached and lookahead is empty, we need to wait for more characters
    // before continuing
    if (lexer->eof_reached)
        return ST_NOT_OK;
    else if (lexer->current == '\0')
        return ST_NOT_INIT;

    char c = lexer->current;

    for (int i = 0; i < (int)LT_INVALID; i++)
    {
        lex_token_t type = (lex_token_t)i;
        if (lexer->state.possible_types[i] == LT_INVALID) continue;

        lex_e_valid status = lex_validate(lexer, type, c);
        lex_e_valid* status_cache = &lexer->state.status[i];
        bool write = false;

        switch (status)
        {
            case LV_NOT:
            {
                if (*status_cache == LV_DONE_WHEN_NOT) goto done;

                goto remove;
            }
            case LV_CONT:
            {
                goto nothing;
            }
            case LV_DONE:
            {
                write = true;
                goto done;
            }
            case LV_DONE_WHEN_NOT:
            {
                *status_cache = LV_DONE_WHEN_NOT;

                goto nothing;
            }
            default:
            {
                // TODO: Should warn / error
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
            if (write)
            {
                sbuilder_write_char(&lexer->state.builder, c);
            }

            lex_add_token(lexer, type, sbuilder_return(&lexer->state.builder));
            lex_reset_state(lexer);

            if (!write)
            {
                e_status nxt_status = lex_advance(lexer);

                if (!(nxt_status == ST_OK || nxt_status == ST_NOT_OK))
                {
                    return nxt_status;
                }
            }

            return ST_OK;
    }

    if (!lexer->state.possible_length)
    {
        // TODO err
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

lex_lexer* lex_create()
{
    lex_lexer* lexer = (lex_lexer*)malloc(sizeof *lexer);

    if (lex_init(lexer) != ST_OK)
    {
        sbuilder_destroy(&lexer->state.builder);
        sbuilder_destroy(&lexer->buf);
        free(lexer);

        return NULL;
    }

    return lexer;
}

e_status lex_init(lex_lexer* lexer)
{
    if (sbuilder_init(&lexer->state.builder, SBUILDER_DEFAULT_CAP) != 0)
    {
        return ST_INIT_FAIL;
    }

    if (sbuilder_init(&lexer->buf, SBUILDER_DEFAULT_CAP) != 0)
    {
        return ST_INIT_FAIL;
    }

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

void lex_free(lex_lexer* lexer)
{
    if (!lexer) return;

    lex_destroy(lexer);
    free(lexer);
}

void lex_destroy(lex_lexer* lexer)
{
    lex_token* tok = lexer->token_first;
    lex_token* tmp = NULL;

    while (tok != NULL)
    {
        tmp = tok;
        tok = tok->next;
        lex_del_token(lexer, tmp);
    }

    lexer->token_first = NULL;
    lexer->token_last = NULL;

    sbuilder_destroy(&lexer->state.builder);
    sbuilder_destroy(&lexer->buf);
}

e_status lex_feed(lex_lexer* lexer, char c)
{
    lexer->current = lexer->lookahead;
    
    if (c == EOF)
    {
        lexer->eof_reached = true;
        lexer->lookahead = '\0';
    }
    else
    {
        lexer->lookahead = c;
    }

    return lex_advance(lexer);
}
