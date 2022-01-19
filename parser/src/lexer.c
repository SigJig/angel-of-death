
#include "lexer.h"

lex_token* lex_add_token(lex_lexer* lexer, lex_e_token kind, const char* lexeme)
{
    lex_token* newtok = (lex_token*)malloc(sizeof newtok);

    if (!newtok)
        return NULL;

    if (lexer->token_last)
        lexer->token_last->next = newtok;

    if (!lexer->token_first)
        lexer->token_first = newtok;

    lexer->token_last = newtok;

    newtok->kind = kind;
    newtok->lexeme = lexeme;
    newtok->line = lexer->line;
    newtok->col = lexer->col;
    newtok->next = NULL;

    return newtok;
}

void lex_del_token(lex_lexer* lexer, lex_token* token)
{
    free(token);
}

lex_lexer* lex_create()
{
    lex_lexer* lexer = (lex_lexer*)malloc(sizeof lexer);

    if (lex_init(lexer) != OK)
    {
        free(lexer);

        return NULL;
    }

    return lexer;
}

void lex_destroy(lex_lexer* lexer)
{
    lex_token* tok = lexer->token_first;
    lex_token* next;

    if (tok)
    {
        do
        {
            next = tok->next;
            lex_token_destroy(lexer, tok);
        } while (next != NULL);
    }

    lex_free(lexer);
}

lex_e_status lex_init(lex_lexer* lexer)
{
    lexer->fp = NULL;
    lexer->eof_reached = false;
    lexer->current = '\0';
    lexer->lookahead = '\0';
    lexer->line = 1;
    lexer->col = 1;
    lexer->token_first = NULL;
    lexer->token_last = NULL;

    return OK;
}

void lex_free(lex_lexer* lexer)
{
    free(lexer);
}

lex_e_status lex_next_char(lex_lexer* lexer, char* dest)
{
    if (lexer->fp == NULL)
        return FILE_ERROR;

    *dest = (char)fgetc(lexer->fp);

    if (*dest == EOF)
    {
        lexer->eof_reached = true;
        *dest = '\0';

        return LEOF;
    }

    return OK;
}

lex_e_status lex_advance(lex_lexer* lexer)
{
    lex_e_status eok;

    if (lexer->eof_reached)
    {
        if (lexer->current != '\0')
        {
            lexer->current = lexer->lookahead;
            lexer->lookahead = '\0';
        }

        return OK;
    }

    if (lexer->lookahead != '\0')
    {
        lexer->current = lexer->lookahead;
    }
    else
    {
        eok = lex_next_char(lexer, &lexer->current);

        if (eok != OK)
        {
            return eok == LEOF ? OK : eok;
        }
    }

    eok = lex_next_char(lexer, &lexer->lookahead);

    if (eok != OK)
    {
        if (eok == LEOF) return OK;
    }

    return eok;
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
    return _isdigit(c) || _is_alpha(c);
}

static bool _is_otherchar(char c)
{
    return (
        '!' <= c && c <= '"' ||
        '$' <= c && c <= '/' ||
        ':' <= c && c <= '?' ||
        '[' <= c && c <= '^' ||
        '{' <= c && c <= '~' ||
        0x80 <= c && c <= 0xFE ||
        c == '`'
    );
}

static bool _is_space(char c) { return c == 0x20; }
static bool _is_hashtag(char c) { return c == '#'; }
static bool _is_at(char c) { return c == '@'; }

static bool _is_no_at(char c)
{
    return (
        _is_alnum(c) ||
        _is_otherchar(c) ||
        _is_space(c) ||
        _is_hashtag(c)
    );
}