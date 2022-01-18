
#ifndef LEXER_HEADER_ONLY
#include "lexer.h"
#endif

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

    return 0;
}

void lex_free(lex_lexer* lexer)
{
    free(lexer);
}

lex_e_status lext_next_char(lex_lexer* lexer, char* dest)
{
    if (lexer->fp == NULL)
        return FILE_ERROR;

    *dest = (char)fgetc(lexer->fp);

    if (dest == EOF)
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