
#ifndef GEDCOM_H
#define GEDCOM_H

#include <stdint.h>
#include "dynarray.h"
#include "stringbuilder.h"
#include "errhandler.h"
#include "lexer.h"
#include "parser.h"

struct ged_line_value
{
    char** mem;
    uint8_t len;
    uint8_t cap;
};

struct ged_record
{
    uint8_t level;
    char* xref;
    char* tag;

    struct ged_line_value* value;

    struct ged_record* children;
    uint8_t len_children;
};

struct ged_builder;

struct ged_record* ged_record_construct(struct ged_builder* ged, struct parser_line* line);

void ged_line_value_free(struct ged_line_value* val);
void ged_record_free(struct ged_record* rec);

#endif // GEDCOM_H