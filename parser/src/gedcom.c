
#include <assert.h>
#include <stdarg.h>

#include "dynarray.h"
#include "gedcom.h"
#include "hashmap.h"

#define DEFAULT_STACK_CAP 100
#define DEFAULT_STACK_CAP_MULT 2

// cap should be prime, and 1.3 * amount of expected items
// 100 * 1.3, closest prime = 131
#define DEFAULT_XREFS_CAP 131

struct ged_builder {
	struct err_handler *ehandler;

	// The stack contains pointers to record pointers
	// Thus, the records are not freed when calling free on the stack's
	// memory block
	struct dyn_array *stack;
	struct hash_table *xrefs;
};

static struct err_message *builder_verrf(struct ged_builder *ged,
					 const char *format, va_list args)
{
	return ehandler_verrf(ged->ehandler, "builder", format, args);
}

static struct err_message *builder_errf(struct ged_builder *ged,
					const char *format, ...)
{
	va_list args;
	va_start(args, format);

	struct err_message *result = builder_verrf(ged, format, args);

	va_end(args);

	return result;
}

static e_statuscode builder_init(struct ged_builder *ged,
				 struct err_handler *ehandler)
{
	assert(DEFAULT_STACK_CAP);

	ged->ehandler = ehandler;
	ged->stack = da_create(DEFAULT_STACK_CAP, sizeof(struct ged_record *));
	ged->xrefs = ht_create(DEFAULT_XREFS_CAP);
}

static void builder_destroy(struct ged_builder *ged)
{
	if (ged->stack->len) {
		for (size_t i = 0; i < ged->stack->len; i++) {

			ged_record_free(
			    *(struct ged_record **)da_get(ged->stack, i));
		}
	}

	da_free(ged->stack);
	ht_free(ged->xrefs);
}

static e_statuscode builder_stack_add(struct ged_builder *ged,
				      struct ged_record *rec)
{
	struct ged_record **mem = da_reserve(ged->stack);
	if (!mem)
		return ST_GEN_ERROR;

	*mem = rec;

	return ST_OK;
}

static struct ged_record *builder_stack_pop(struct ged_builder *ged)
{
	return *(struct ged_record **)da_pop(ged->stack);
}

struct ged_record *ged_record_construct(struct ged_builder *ged,
					struct parser_line *line)
{
	struct ged_record *rec = malloc(sizeof *rec);

	char *level = line->level->lexeme;
	int level_parsed = atoi(level);

	if (strlen(level) > 1 && level[0] == '0') {
		// the gedcom standard does not allow leading 0s on levels
		// TODO: this should be warning
		builder_errf(ged,
			     "gedcom standard disallows leading 0 on level "
			     "declarations (%s)",
			     level);

	} else if (level_parsed == 0) {
		builder_errf(ged, "unable to parse %s as level", level);
		goto error;
	}

	rec->level = level_parsed;

	if (line->xref) {
		rec->xref = strdup(line->xref->lexeme);

		// add to symbol table
		if (ht_get(ged->xrefs, rec->xref) != NULL) {
			builder_errf(ged, "xref %s already defined", rec->xref);
		} else {
			ht_set(ged->xrefs, rec->xref, rec);
		}
	} else {
		rec->xref = NULL;
	}

	rec->tag = strdup(line->tag->lexeme);

	if (strlen(rec->tag) > 0 && rec->tag[0] == '_') {
		// TODO: Handle custom tags
	}

	if (line->line_value) {
		// rec->value = malloc(sizeof *rec->value);
		// TODO: Build value
		assert(8 == sizeof(line->line_value->lexeme));
		rec->value = da_create(10, sizeof(line->line_value->lexeme));

		struct lex_token *tok = line->line_value;

		while (tok) {
			char **mem = da_reserve(rec->value);

			if (!mem) {
				builder_errf(ged, "malloc errror");

				goto error;
			}

			*mem = strdup(tok->lexeme);

			tok = tok->next;
		}
	} else {
		rec->value = NULL;
	}

	if (builder_stack_add(ged, rec) != ST_OK) {
		assert(false);
		// TODO: error
	}

	return rec;

error:
	ged_record_free(rec);

	return NULL;
}

void ged_record_free(struct ged_record *rec)
{
	if (rec->xref) {
		free(rec->xref);
	}
	if (rec->tag) {
		free(rec->tag);
	}

	if (rec->value) {
		for (size_t i = 0; i < rec->value->len; i++) {
			char **mem = da_get(rec->value, i);

			if (mem && *mem) {
				free(*mem);
			}
		}

		da_free(rec->value);
	}

	for (uint8_t i = 0; i < rec->len_children; i++) {
		ged_record_free(&rec->children[i]);
	}

	free(rec);
}