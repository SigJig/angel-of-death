
#include "context/context.h"
#include "gedcom.h"
#include "lexer.h"
#include "parser.h"
#include "utils/hashmap.h"
#include "utils/ptrarr.h"
#include <assert.h>
#include <stdio.h>

struct ehm {
    int x;
    int y;
};

void
test_dynarray(void)
{
    ptr_arr pa = pa_create(5);
    struct ehm* e = malloc(sizeof *e);
    e->x = 3;
    e->y = 2;

    pa_push(pa, (void*)5);
    pa_push(pa, (void*)4);
    pa_push(pa, (void*)3);
    pa_push(pa, (void*)2);
    pa_push(pa, (void*)1);

    for (int i = 0; i < 5; i++) {
        printf("dig %d\n", (int)pa_pop(pa));
    }

    pa_push(pa, e);
    struct ehm* t = pa_pop(pa);
    printf("%d, %d\n", t->x, t->y);

    free(e);
    pa_free(pa);
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

    ptr_arr arr = ged_from_parser(presult, ctx);

    for (size_t i = 0; i < pa_len(arr); i++) {

#if 0
        char* tostring = ged_record_to_string(tmp);
        printf("%s\n", tostring);
        free(tostring);
#endif

        ged_record_free(pa_get(arr, i));
    }

    pa_free(arr);

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

    return 0;
}