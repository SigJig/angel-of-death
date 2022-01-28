
#include "tags/base.h"
#include <assert.h>
#include <string.h>

const char *MONTH_ENG = "MONTH";
const char *MONTH_FREN = "MONTH_FREN";
const char *MONTH_HEBR = "MONTH_HEBR";

struct month {
    char *name;
    char *value;
};

static struct month *
month_base_create(const char **possible_months, const char *name,
                  struct ged_record *rec)
{
    if (rec->value->len != 1) {
        // TODO: err

        return NULL;
    }

    const char *entry = ged_record_value(rec, 0);

    if (!entry) {
        // TODO: ERr

        return NULL;
    }

    size_t size = sizeof possible_months / sizeof *possible_months;
    assert(size == 12);

    for (size_t i = 0; i < size; i++) {
        if (strcmp(possible_months[i], entry) == 0) {
            struct month *m = malloc(sizeof *m);

            if (!m) {
                return NULL;
            }

            m->name = strdup(name);
            m->value = strdup(entry);
        }
    }

    // TODO: Warn

    return NULL;
}

static void
month_base_free(struct month *m)
{
    free(m->name);
    free(m->value);
    free(m);
}

static struct month *
month_create(struct tag_interface interface, struct ged_record *rec)
{
    const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                            "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    return month_base_create(months, MONTH_ENG, rec);
}

static struct month *
month_fren_create(struct tag_interface interface, struct ged_record *rec)
{
    const char *months[] = {"VEND", "BRUM", "FRIM", "NIVO", "PLUV",
                            "VENT", "GERM", "FLOR", "PRAI", "MESS",
                            "THER", "FRUC", "COMP"};

    return month_base_create(months, MONTH_FREN, rec);
}

static struct month *
month_hebr_create(struct tag_interface interface, struct ged_record *rec)
{
    const char *months[] = {"TSH", "CSH", "KSL", "TVT", "SHV", "ADR", "ADS",
                            "NSN", "IYR", "SVN", "TMZ", "AAV", "ELL"};

    return month_base_create(months, MONTH_HEBR, rec);
}

static struct tag_interface tag_i_month_eng = {
    .create = (fn_create)month_create, .free = (fn_free)month_base_free};

static struct tag_interface tag_i_month_fren = {
    .create = (fn_create)month_fren_create, .free = (fn_free)month_base_free};

static struct tag_interface tag_i_month_hebr = {
    .create = (fn_create)month_hebr_create, .free = (fn_free)month_base_free};

void
init_months(struct hash_table *ht)
{
    ht_set(ht, MONTH_ENG, &tag_i_month_eng);
    ht_set(ht, MONTH_FREN, &tag_i_month_fren);
    ht_set(ht, MONTH_HEBR, &tag_i_month_hebr);

    return;
}