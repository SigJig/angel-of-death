
#include "dynarray.h"
#include "errhandler.h"
#include "lexer.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>

size_t
print_errors(struct err_handler* ehandler)
{
    size_t length = ehandler_len(ehandler);

    for (size_t i = 0; i < (length > 10 ? 10 : length); i++) {
        char* msg = emessage_to_string(ehandler_get(ehandler, i));

        printf("%s\n", msg);
        free(msg);
    }

    return length;
}

void
from_example(const char* path)
{
    FILE* fp = fopen(path, "r");
    assert(fp);

    struct err_handler ehandler;
    assert(ehandler_init(&ehandler) == ST_OK);

    struct lex_lexer* lexer = lex_create(&ehandler);
    assert(lexer);

    for (int c = fgetc(fp); c != EOF; c = fgetc(fp)) {
        lex_feed(lexer, c);
    }
    lex_feed(lexer, EOF);

    if (print_errors(&ehandler)) {
        goto cleanup;
    }

    printf("LEXER DONE: \n");
    struct lex_token* tok = lexer->token_first;

#if 1
    int index = 0;
    while (tok && index < 10) {
        if (tok->type != LT_DELIM && tok->type != LT_WHITESPACE)
            printf("\t%d: %s\n", tok->type, tok->lexeme);

        tok = tok->next;
        index++;
    }
#endif

    struct parser_result presult = parser_parse(lexer->token_first, &ehandler);

    print_errors(&ehandler);

    struct parser_line* line = presult.front;

    while (line) {
        char* str = parser_line_to_string(line);
        printf("%s\n", str);
        free(str);

        line = line->next;
    }

    parser_result_destroy(&presult);

cleanup:
    lex_free(lexer);
    ehandler_destroy(&ehandler);

    fclose(fp);
}

int
main(int argc, char** argv)
{

    from_example("/home/sig/Documents/development/projects/angel-of-death/"
                 "parser/example.ged");
    /*struct err_handler ehandler;

    ehandler_init(&ehandler);

    struct lex_lexer* lexer = lex_create(&ehandler);

    const char* teststring =
        "0 @1NAME@ Jahck @anychar escape sequence @ hmm\n\r";

    for (int i = 0; i <= strlen(teststring); i++)
        lex_feed(lexer, teststring[i]);

    struct lex_token* tok = lexer->token_first;

    while (tok) {
        printf("%d: %s\n", tok->type, tok->lexeme);

        tok = tok->next;
    }

    struct parser_result result = parser_parse(lexer->token_first, &ehandler);
    size_t len = ehandler_len(&ehandler);

    if (len) {
        for (size_t i = 0; i < len; i++) {
            char* msg = emessage_to_string(ehandler_get(&ehandler, i));
            printf("%s\n", msg);

            free(msg);
        }
    }

    ehandler_destroy(&ehandler);
    lex_free(lexer);
    parser_result_destroy(&result);

    int nums[10] = {0};
    struct dyn_array* da = da_create(4, sizeof &nums);

    int** n = da_reserve(da);
    *n = &nums[4];
    **n += 1;

    int** x = da_reserve(da);
    *x = &nums[3];
    **x += 5;

    int* l = *(int**)da_pop(da);
    assert(l == &nums[3]);

    int** s = da_pop(da);
    assert(*s == &nums[4]);

    da_free(da);*/

    return 0;
}