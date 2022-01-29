
#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "context/context.h"
#include "context/genstate.h"
#include "utils/statuscode.h"
#include "utils/stringbuilder.h"

typedef enum {
    LT_NUMBER = 0,
    LT_POINTER,
    LT_TERMINATOR,
    LT_S_ALNUM, // sequence alnum
    LT_S_ANYCHAR,
    LT_ESCAPE,
    LT_DELIM,
    LT_WHITESPACE,
    LT_INVALID // should always be last
} lex_token_type;

// used by the validator
typedef enum lex_e_valid {
    LV_NOT = 0,        // invalid (cast from false)
    LV_CONT,           // valid and can continue until v_done (cast from true)
    LV_DONE,           // valid and done
    LV_DONE_WHEN_NOT,  // done when the validator returns not
    LV_DONE_WHEN_DELIM // done when a delim is encountered (space)
} lex_e_valid;

struct lex_token {
    lex_token_type type;

    size_t line;
    size_t col;

    char* lexeme;
    struct lex_token* next;
};

struct lex_lexer {
    struct {
        struct sbuilder builder;

        lex_token_type possible_types[(int)LT_INVALID]; // contains each type
        lex_e_valid status[(int)LT_INVALID]; // contains status for each type

        int possible_length;
    } state;

    bool eof_reached;
    char current;
    char lookahead;

    size_t tokline;
    size_t tokcol;
    size_t curline;
    size_t curcol;
    struct lex_token* token_first;
    struct lex_token* token_last;

    struct context* ctx;
    struct sbuilder buf;
};

// API functions

// copies token. sets next to null, meaning the new token will be isolated
struct lex_token* lex_token_copy(struct lex_token* token);
void lex_token_free(struct lex_token* token);

// used when lexer is heap allocated
struct lex_lexer* lex_create(struct context* ctx);
void lex_free(struct lex_lexer* lexer);

// used when lexer is not heap allocated
e_statuscode lex_init(struct lex_lexer* lexer, struct context* ctx);
void lex_destroy(struct lex_lexer* lexer);

e_statuscode lex_feed(struct lex_lexer* lexer, char c);

#endif // LEXER_H