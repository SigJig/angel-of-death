
#ifndef GEDCOM_H
#define GEDCOM_H

#include "lexer.h"
#include "parser.h"
#include "tags/base.h"
#include "utils/dynarray.h"
#include "utils/stringbuilder.h"
#include <stdint.h>

struct ged_record {
    uint8_t level;
    char* xref;
    char* tag;

    struct tag_base* value;

    uint8_t len_children;
    struct ged_record* children;

    struct ged_record* next;
};

struct ged_builder;

struct ged_record* ged_record_construct(struct ged_builder* ged,
                                        struct parser_line* line);

void ged_record_free(struct ged_record* rec);

#endif // GEDCOM_H