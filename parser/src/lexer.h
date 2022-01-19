
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stringbuilder.h"

typedef enum
{
    DIGIT,
    ALPHA,
    OTHERCHAR,
    ANYCHAR,
    NO_AT, // ANYCHAR without @@
    HASHTAG,
    SPACE,
    AMPERSAND,
    CARRIAGERETURN,
    LINEFEED
} lex_e_char;

typedef enum
{
    NUMBER,
    POINTER,
    TERMINATOR,
    S_ALNUM,
    ESCAPE,
    DELIM
} lex_e_token;

typedef enum {OK = 0, GEN_ERROR, FILE_ERROR, NON_INIT} lex_e_status;

typedef struct lex_token
{
    lex_e_token kind;
    char* lexeme;
    size_t line;
    size_t col;

    struct lex_token* next;
} lex_token;

typedef struct
{
    bool eof_reached;
    char current;
    char lookahead;
    size_t line;
    size_t col;
    lex_token* token_first;
    lex_token* token_last;
} lex_lexer;

lex_token* lex_token_create(lex_lexer* lexer, lex_e_token kind, const char* lexeme);

// Beware this does not set to null where another token might have ->next as this token
void lex_token_destroy(lex_lexer* lexer, lex_token* token);

lex_lexer* lex_create();
void lex_destroy(lex_lexer* lexer);

lex_e_status lex_init(lex_lexer* lexer);
void lex_free(lex_lexer* lexer);

lex_e_status lex_advance(lex_lexer* lexer);
lex_e_status lex_feed(lex_lexer* lexer, char c);
