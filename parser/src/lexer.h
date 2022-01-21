
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stringbuilder.h"
#include "errcodes.h"

typedef enum
{
    LT_NUMBER = 0,
    LT_POINTER,
    LT_TERMINATOR,
    LT_S_ALNUM, // sequence alnum
    LT_ESCAPE_TEXT_NOAT,
    LT_AT,
    LT_DOUBLE_AT,
    LT_HASH_TAG,
    LT_DELIM,
    LT_INVALID // should always be last
} lex_token_t;

typedef struct lex_token
{
    lex_token_t type;

    size_t line;
    size_t col;

    char* lexeme;
    struct lex_token* next;
} lex_token;

// used by the validator
typedef enum
{
    LV_NOT = 0, // invalid (cast from false)
    LV_CONT, // valid and can continue until v_done (cast from true)
    LV_DONE, // valid and done
    LV_DONE_WHEN_NOT // done when the validator returns not
} lex_e_valid;

typedef struct
{
    struct
    {
        sbuilder builder;

        lex_token_t possible_types[(int)LT_INVALID]; // contains each type
        lex_e_valid status[(int)LT_INVALID]; // contains status for each type

        int possible_length;
    } state;

    bool eof_reached;
    char current;
    char lookahead;

    size_t line;
    size_t col;
    lex_token* token_first;
    lex_token* token_last;
    
    sbuilder buf;
} lex_lexer;


// API functions

// used when lexer is heap allocated
lex_lexer* lex_create();
void lex_free(lex_lexer* lexer);

// used when lexer is not heap allocated
e_status lex_init(lex_lexer* lexer);
void lex_destroy(lex_lexer* lexer);

e_status lex_feed(lex_lexer* lexer, char c);

#endif // LEXER_H