
#include <lexer.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    lex_lexer* lexer = lex_create();

    const char* teststring = "1921 19 @a #@ alnum @@";

    for (int i = 0; i < 23; i++)
    {
        printf("%d\n", lex_feed(lexer, teststring[i]));
    }

    lex_token* tok = lexer->token_first;

    while (tok)
    {
        printf("%d: %s\n", tok->type, tok->lexeme);

        tok = tok->next;
    }    

    lex_free(lexer);

    return 0;
}