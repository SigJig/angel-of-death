
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stringbuilder.h"

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

// convert to
extern const lex_token_t _lex_tok_cvtable[];

lex_token_t lex_token_t_from_int(int i);

typedef enum {
    LS_OK = 0,
    LS_NOT_OK, // general not ok, does not necessarily mean error
    LS_FILE_ERROR,
    LS_INIT_FAIL,
    LS_NOT_INIT,
    LS_GEN_ERROR
} lex_e_status;

// used by the validator
typedef enum
{
    LV_NOT = 0, // invalid (cast from false)
    LV_CONT, // valid and can continue until v_done (cast from true)
    LV_DONE, // valid and done
    LV_DONE_WHEN_NOT // done when the validator returns not
} _lex_e_valid;

typedef struct lex_token
{
    lex_token_t type;
    char* lexeme;
    size_t line;
    size_t col;

    struct lex_token* next;
} lex_token;


typedef struct
{
    struct
    {
        sbuilder builder;
        lex_token_t possible_types[(int)LT_INVALID]; // contains each type
        _lex_e_valid status[(int)LT_INVALID]; // contains status for each type
    } state;

    bool eof_reached;
    char current;
    char lookahead;
    size_t line;
    size_t col;
    lex_token* token_first;
    lex_token* token_last;
} lex_lexer;


// API functions

// used when lexer is heap allocated
lex_lexer* lex_create();
void lex_free(lex_lexer* lexer);

// used when lexer is not heap allocated
lex_e_status lex_init(lex_lexer* lexer);
void lex_destroy(lex_lexer* lexer);

lex_e_status lex_feed(lex_lexer* lexer, char c);


// Internal functions

// should not clear sbuilder, as that should be cleared either by using sbuilder_return or sbuilder_clear manually
void _lex_reset_state(lex_lexer* lexer);

lex_token* _lex_add_token(lex_lexer* lexer, lex_token_t type, char* lexeme);

// Beware this does not set to null where another token might have ->next as this token
void _lex_del_token(lex_lexer* lexer, lex_token* token);

lex_e_status _lex_advance(lex_lexer* lexer);

// returns 0 if invalid, 1 if valid and can continue, and 2 if valid and can not continue
_lex_e_valid _lex_validate(lex_lexer* lexer, lex_token_t type, char c);

#endif // LEXER_H