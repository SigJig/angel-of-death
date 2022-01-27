
#include "tags/base.h"
#include <string.h>

// TODO: add value!
static struct tag_base *month_create(struct tag_interface interface,
				     struct ged_record *rec)
{
	if (rec->value->len != 1) {
		// TODO: err

		return NULL;
	}

	const char *entry = ged_record_value(rec, 0);

	const char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
				"JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

	const size_t size = sizeof months / sizeof *months;

	for (size_t i = 0; i < size; i++) {
		if (strcmp(months[i], entry) == 0) {
			struct tag_base *tag = malloc(sizeof *tag);

			tag->name = strdup(entry);

			return tag;
		}
	}

	// TODO: throw err
	return NULL;
}

static struct tag_interface tag_i_month = {.create = month_create,
					   .free = tag_base_free};

void init_months(struct hash_table *ht)
{
	ht_set(ht, "MONTH", &tag_i_month);

	return;
}