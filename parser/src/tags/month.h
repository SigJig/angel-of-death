
#include "tags/base.h"
#include <assert.h>
#include <string.h>

const char* MONTH_ENG = "MONTH";
const char* MONTH_FREN = "MONTH_FREN";
const char* MONTH_HEBR = "MONTH_HEBR";

struct month {
    char* name;
    char* value;
};

static struct month*
month_base_create(const char** possible_months, const char* name,
                  struct tag* tag, struct ged_record* rec,
                  struct lex_token* tok)
{
    if (tok->next) {
        int count = 0;

        while (tok) {
            count++;
            tok = tok->next;
        }

        ctx_critf(tag->ctx, "%s expects exactly one argument (got %d)", name,
                  count);

        return NULL;
    }

    if (!tok->lexeme) {
        ctx_critf(tag->ctx, "%s empty argument", name);

        return NULL;
    }

    const char* entry = tok->lexeme;

    size_t size = sizeof possible_months / sizeof *possible_months;

    for (size_t i = 0; i < size; i++) {
        if (strcmp(possible_months[i], entry) == 0) {
            struct month* m = malloc(sizeof *m);

            if (!m) {
                assert(false);

                return NULL;
            }

            m->name = strdup(name);
            m->value = strdup(entry);

            return m;
        }
    }

    ctx_errf(tag->ctx, "%s got invalid month %s", name, entry);

    return NULL;
}

static void
month_base_free(struct tag* tag)
{
    struct month* m = tag->data;

    if (!m) {
        return;
    }

    if (m->name) {
        free(m->name);
    }

    if (m->value) {
        free(m->value);
    }

    free(m);
}

static struct month*
month_eng_create(struct tag* tag, struct ged_record* rec,
                 struct lex_token* toks)
{
    const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    return month_base_create(months, MONTH_ENG, tag, rec, toks);
}

static struct month*
month_fren_create(struct tag* tag, struct ged_record* rec,
                  struct lex_token* toks)
{
    const char* months[] = {"VEND", "BRUM", "FRIM", "NIVO", "PLUV",
                            "VENT", "GERM", "FLOR", "PRAI", "MESS",
                            "THER", "FRUC", "COMP"};

    return month_base_create(months, MONTH_FREN, tag, rec, toks);
}

static struct month*
month_hebr_create(struct tag* tag, struct ged_record* rec,
                  struct lex_token* toks)
{
    const char* months[] = {"TSH", "CSH", "KSL", "TVT", "SHV", "ADR", "ADS",
                            "NSN", "IYR", "SVN", "TMZ", "AAV", "ELL"};

    return month_base_create(months, MONTH_HEBR, tag, rec, toks);
}

static const struct tag_interface tag_i_month_eng = {.create = month_eng_create,
                                                     .free = month_base_free};

static const struct tag_interface tag_i_month_fren = {
    .create = month_fren_create, .free = month_base_free};

static const struct tag_interface tag_i_month_hebr = {
    .create = month_hebr_create, .free = month_base_free};

void
init_months(struct hash_table* ht)
{
    ht_set(ht, MONTH_ENG, &tag_i_month_eng);
    ht_set(ht, MONTH_FREN, &tag_i_month_fren);
    ht_set(ht, MONTH_HEBR, &tag_i_month_hebr);

    return;
}