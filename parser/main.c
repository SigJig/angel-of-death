
#include <lexer.h>
#include <parser.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    struct err_handler ehandler;
    ehandler_init(&ehandler);

    struct lex_lexer* lexer = lex_create(&ehandler);

    const char* teststring = "0 @1NAME@ Jahck @anychar escape sequence @ hmm\n\r";
    printf("sizeof lexer: %zu\nsizeof token: %zu\nsizeof struct sbuilder: %zu\n", sizeof(struct lex_lexer), sizeof(struct lex_token), sizeof(struct sbuilder));
    printf("sizeof char*: %zu\nsizeof size_t: %zu\nsizeof enum: %zu\n", sizeof(char*), sizeof(size_t), sizeof(lex_token_type));

    for (int i = 0; i <= strlen(teststring); i++) lex_feed(lexer, teststring[i]);

    struct lex_token* tok = lexer->token_first;

    while (tok)
    {
        printf("%d: %s\n", tok->type, tok->lexeme);

        tok = tok->next;
    }

    struct parser_result result = parser_parse(lexer->token_first, &ehandler);

    if (ehandler.len)
    {
        for (size_t i = 0; i < ehandler.len; i++)
        {
            char* msg = emessage_to_string(&ehandler.messages[i]);
            printf("%s\n", msg);

            free(msg);
        }
    }

    ehandler_destroy(&ehandler);
    lex_free(lexer);
    parser_result_destroy(&result);

    return 0;
}