
#include "context/context.h"
#include "gedcom.h"
#include "lexer.h"
#include "parser.h"
#include "utils/dynarray.h"
#include "utils/hashmap.h"
#include <assert.h>
#include <stdio.h>

struct ehm {
    int x;
    int y;
};

void
test_dynarray(void)
{
    struct dyn_array* da = da_create(5);
    struct ehm* e = malloc(sizeof *e);
    e->x = 3;
    e->y = 2;

    da_push(da, (void*)5);
    da_push(da, (void*)4);
    da_push(da, (void*)3);
    da_push(da, (void*)2);
    da_push(da, (void*)1);

    for (int i = 0; i < 5; i++) {
        printf("dig %d\n", (int)da_pop(da));
    }

    da_push(da, e);
    struct ehm* t = da_pop(da);
    printf("%d, %d\n", t->x, t->y);

    free(e);
    da_free(da);
}

void
test_hashtable(void)
{
    struct hash_table* ht = ht_create(31);

    ht_set(ht, "testcase", (void*)(uintptr_t)4);
    printf("Hashtable test: %d\n", (int)(uintptr_t)ht_get(ht, "testcase"));

    ht_free(ht);
}

size_t
print_errors(struct context* ctx)
{
    char* str = ctx_log_to_string(ctx);

    if (!str) {
        return 0;
    }

    printf("%s", str);

    free(str);

    return 1;
}

void
from_example(const char* path)
{
    FILE* fp = fopen(path, "r");
    assert(fp);

    struct context* ctx = ctx_create(DEBUG);
    ctx_push(ctx, posctx_create("lexer"));

    struct lex_lexer* lexer = lex_create(ctx);
    assert(lexer);

    for (int c = fgetc(fp); c != EOF; c = fgetc(fp)) {
        lex_feed(lexer, c);
    }
    lex_feed(lexer, EOF);

    ctx_pop(ctx);

    if (print_errors(ctx) && !ctx_continue(ctx)) {
        goto cleanup;
    }

    printf("LEXER DONE: \n");
    struct lex_token* tok = lexer->token_first;

#if 0
    int index = 0;
    while (tok /* && index < 0*/) {
        if (tok->type != LT_DELIM && tok->type != LT_WHITESPACE)
            printf("\t%d: %s\n", tok->type, tok->lexeme);

        tok = tok->next;
        index++;
    }
#endif

#if 1
    struct parser_result presult = parser_parse(lexer->token_first, ctx);

    print_errors(ctx);

    struct parser_line* line = presult.front;

    while (line) {
#if 0
        char* str = parser_line_to_string(line);
        printf("%s\n", str);
        free(str);
#endif

        line = line->next;
    }

    struct ged_record* recfront = ged_from_parser(presult, ctx);
    struct ged_record* tmp = NULL;

    while (recfront) {
        tmp = recfront;
        recfront = recfront->next;

#if 0
        char* tostring = ged_record_to_string(tmp);
        printf("%s\n", tostring);
        free(tostring);
#endif

        ged_record_free(tmp);
    }

#if 0
    print_errors(ctx);
#endif

    parser_result_destroy(&presult);
#endif
cleanup:
    lex_free(lexer);
    // ehandler_destroy(&ehandler);
    ctx_free(ctx);

    fclose(fp);
}

int
main(int argc, char** argv)
{
    test_dynarray();
    test_hashtable();

#if 1
    from_example("/home/sig/Documents/development/projects/angel-of-death/"
                 "examples/example.ged");
#endif
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