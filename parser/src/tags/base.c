
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "tags/base.h"
#include "tags/month.h"

// Hashtable of tag interfaces
static struct hash_table *tags_ht;

static struct hash_table *tags_ht_instance()
{
	if (!tags_ht) {
		assert(false);
	}

	return tags_ht;
}

void tag_base_free(struct tag_base *tag)
{
	free(tag->name);
	free(tag);
}

void tags_init()
{
	assert(!tags_ht /* Tag hashtable has already been initialized */);

	tags_ht = ht_create(31);

	/*
	Array of modules, only used for initialization as the contents of
    this are added to the hash_table through the init function added
    here.

    Each module should contain an init function following the signature:
    void init_module(struct hash_table*);

    This array is kept inside this function, and not inside the global scope,
    as this function is called only once. Keeping this array in global space
    would then be unnecessary memory usage throughout the runtime of the
    program.
	*/
	const void (*init_modules[])(struct hash_table *) = {
	    init_months,
	};

	assert(sizeof init_modules /* No tag modules to initialize */);

	size_t mlength = sizeof init_modules / sizeof *init_modules;

	for (size_t i = 0; i < mlength; i++) {
		(*init_modules[i])(tags_ht);
	}
}

void tags_cleanup()
{
	if (!tags_ht) {
		return;
	}

	ht_free(tags_ht);
	tags_ht = NULL;
}

const struct tag_interface *tag_i_get(const char *key)
{
	return ht_get(tags_ht_instance(), key);
}