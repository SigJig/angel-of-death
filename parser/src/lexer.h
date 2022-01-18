
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

typedef enum {OK = 0, GEN_ERROR, FILE_ERROR, LEOF} lex_e_status;

typedef struct
{
    FILE* fp; // Prob better for the file to feed the lexer, and construct a list of tokens
    bool eof_reached;
    char current;
    char lookahead;
    size_t line;
    size_t col;
} lex_lexer;

lex_lexer* lex_create();
void lex_destroy(lex_lexer* lexer);

lex_e_status lex_init(lex_lexer* lexer);
void lex_free(lex_lexer* lexer);

lex_e_status lex_advance(lex_lexer* lexer);
lex_e_status lex_next_char(lex_lexer* lexer, char* dest);
