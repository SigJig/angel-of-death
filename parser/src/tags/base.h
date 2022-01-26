
#ifndef TAGS_BASE_H
#define TAGS_BASE_H

struct ged_record; // defined in gedcom.h

struct tag_base {
	char *name;
};

struct tag_interface {
	struct tag_base *(*create)(struct tag_interface, struct ged_record *);
	void (*free)(struct ged_record *);
};

struct tag_interface tag_get_interface(const char *key);

#endif // TAGS_BASE_H