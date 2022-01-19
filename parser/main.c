
#include "lexer.h"

int main(int argc, char** argv)
{
    sbuilder builder;

    sbuilder_init(&builder, SBUILDER_DEFAULT_CAP);

    char* hmm[10] = {
        "the ", "quick ", "brown",  " fox ", "jumps", " over ", "the ", "lazy ", "dog", "."
    };

    for (size_t i = 0; i < 10; i++)
    {
        sbuilder_write(&builder, hmm[i]);
        sbuilder_write(&builder, "\n");

        if (i & 1)
        {
            sbuilder_write(&builder, "\n");
            printf("%s", sbuilder_return(&builder));
        }
    }

    sbuilder_free(&builder);

    return 0;
}