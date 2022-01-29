
#include "tags/base.h"
#include <assert.h>
#include <string.h>

const char* MONTH_ENG = "MONTH";
const char* MONTH_FREN = "MONTH_FREN";
const char* MONTH_HEBR = "MONTH_HEBR";

struct month {
    TAG_BASE;
    char* name;
    char* value;
};

static struct month*
month_base_create(const char** possible_months, const char* name,
                  struct tag_interface* interface, struct ged_record* rec,
                  struct lex_token* tok, struct context* ctx)
{
    if (tok->next) {
        int count = 0;

        while (tok) {
            count++;
            tok = tok->next;
        }

        ctx_critf(ctx, "%s expects exactly one argument (got %d)", name, count);

        return NULL;
    }

    if (!tok->lexeme) {
        ctx_critf(ctx, "%s empty argument", name);

        return NULL;
    }

    const char* entry = tok->lexeme;

    size_t size = sizeof possible_months / sizeof *possible_months;

    for (size_t i = 0; i < size; i++) {
        if (strcmp(possible_months[i], entry) == 0) {
            struct month* m = malloc(sizeof *m);

            if (!m) {
                return NULL;
            }

            m->interface = interface;
            m->name = strdup(name);
            m->value = strdup(entry);
            m->ctx = ctx;
        }
    }

    ctx_errf(ctx, "%s got invalid month %s", name, entry);

    return NULL;
}

static void
month_base_free(struct month* m)
{
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
month_eng_create(struct tag_interface* interface, struct ged_record* rec,
                 struct lex_token* toks, struct context* ctx)
{
    const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    return month_base_create(months, MONTH_ENG, interface, rec, toks, ctx);
}

static struct month*
month_fren_create(struct tag_interface* interface, struct ged_record* rec,
                  struct lex_token* toks, struct context* ctx)
{
    const char* months[] = {"VEND", "BRUM", "FRIM", "NIVO", "PLUV",
                            "VENT", "GERM", "FLOR", "PRAI", "MESS",
                            "THER", "FRUC", "COMP"};

    return month_base_create(months, MONTH_FREN, interface, rec, toks, ctx);
}

static struct month*
month_hebr_create(struct tag_interface* interface, struct ged_record* rec,
                  struct lex_token* toks, struct context* ctx)
{
    const char* months[] = {"TSH", "CSH", "KSL", "TVT", "SHV", "ADR", "ADS",
                            "NSN", "IYR", "SVN", "TMZ", "AAV", "ELL"};

    return month_base_create(months, MONTH_HEBR, interface, rec, toks, ctx);
}

static struct tag_interface tag_i_month_eng = {
    .create = (fn_create)month_eng_create, .free = (fn_free)month_base_free};

static struct tag_interface tag_i_month_fren = {
    .create = (fn_create)month_fren_create, .free = (fn_free)month_base_free};

static struct tag_interface tag_i_month_hebr = {
    .create = (fn_create)month_hebr_create, .free = (fn_free)month_base_free};

void
init_months(struct hash_table* ht)
{
    ht_set(ht, MONTH_ENG, &tag_i_month_eng);
    ht_set(ht, MONTH_FREN, &tag_i_month_fren);
    ht_set(ht, MONTH_HEBR, &tag_i_month_hebr);

    return;
}