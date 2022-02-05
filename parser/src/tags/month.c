
#include "tags/month.h"
#include "utils/ptrarr.h"
#include <assert.h>
#include <string.h>

struct month {
    char* name;
    char* value;
};

static struct month*
month_base_create(struct ged_record* rec, const char** possible_months,
                  const char* name)
{
    size_t len = pa_len(rec->value);

    if (len != 1) {
#if 0
        ctx_critf(tag->ctx, "%s expects exactly one argument (got %d)", name,
                  count);
#endif

        return NULL;
    }

    struct lex_token* tok = pa_front(rec->value);

    if (!tok->lexeme) {
#if 0
        ctx_critf(tag->ctx, "%s empty argument", name);
#endif
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

#if 0
    ctx_errf(tag->ctx, "%s got invalid month %s", name, entry);
#endif

    return NULL;
}

static void
month_base_free(void* data)
{
    struct month* m = data;

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
month_eng_create(struct ged_record* rec)
{
    const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    return month_base_create(rec, months, MONTH_ENG);
}

static struct month*
month_fren_create(struct ged_record* rec)
{
    const char* months[] = {"VEND", "BRUM", "FRIM", "NIVO", "PLUV",
                            "VENT", "GERM", "FLOR", "PRAI", "MESS",
                            "THER", "FRUC", "COMP"};

    return month_base_create(rec, months, MONTH_FREN);
}

static struct month*
month_hebr_create(struct ged_record* rec)
{
    const char* months[] = {"TSH", "CSH", "KSL", "TVT", "SHV", "ADR", "ADS",
                            "NSN", "IYR", "SVN", "TMZ", "AAV", "ELL"};

    return month_base_create(rec, months, MONTH_HEBR);
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