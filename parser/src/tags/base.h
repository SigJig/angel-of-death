
#ifndef TAGS_BASE_H
#define TAGS_BASE_H

#include "gedcom.h"
#include "hashmap.h"

struct ged_record; // defined in gedcom.h

struct tag_base {
	char *name;
};

struct tag_interface {
	struct tag_base *(*create)(struct tag_interface, struct ged_record *);
	void (*free)(struct tag_base *);
};

void tag_base_free(struct tag_base *tag);

const struct tag_interface *tag_i_get(const char *key);

void tags_init();
void tags_cleanup();

#endif // TAGS_BASE_H