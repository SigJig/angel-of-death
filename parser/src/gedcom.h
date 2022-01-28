
#ifndef GEDCOM_H
#define GEDCOM_H

#include "dynarray.h"
#include "errhandler.h"
#include "lexer.h"
#include "parser.h"
#include "stringbuilder.h"
#include <stdint.h>

struct ged_record {
    uint8_t level;
    char* xref;
    char* tag;

    struct dyn_array* value;

    struct ged_record* children;
    uint8_t len_children;
};

struct ged_builder;

struct ged_record* ged_record_construct(struct ged_builder* ged,
                                        struct parser_line* line);

void ged_record_free(struct ged_record* rec);

const char* ged_record_value(struct ged_record* rec, size_t index);

#endif // GEDCOM_H