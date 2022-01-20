
#include <lexer.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    lex_lexer* lexer = lex_create();

    const char* teststring = "1921 19 @a #@ @ alnum @@";
    printf("sizeof lexer: %zu\nsizeof token: %zu\nsizeof sbuilder: %zu\n", sizeof(lex_lexer), sizeof(lex_token), sizeof(sbuilder));
    printf("sizeof char*: %zu\nsizeof size_t: %zu\nsizeof enum: %zu\n", sizeof(char*), sizeof(size_t), sizeof(lex_token_t));

    for (int i = 0; i < 23; i++) lex_feed(lexer, teststring[i]);

    lex_token* tok = lexer->token_first;

    while (tok)
    {
        printf("%d: %s\n", tok->type, tok->lexeme);

        tok = tok->next;
    }    

    lex_free(lexer);

    return 0;
}