
#ifndef ERRHANDLER_H
#define ERRHANDLER_H

#include "dynarray.h"
#include "statuscode.h"
#include "stringbuilder.h"
#include <stdarg.h>
#include <stdlib.h>

struct err_message {
	const char *type;
	const char *origin;
	char *message;
};

struct err_handler {
	struct dyn_array *messages;
};

e_statuscode ehandler_init(struct err_handler *handler);
void ehandler_destroy(struct err_handler *handler);
size_t ehandler_len(struct err_handler *handler);

struct err_message *ehandler_get(struct err_handler *handler, size_t index);

void emessage_destroy(struct err_message *msg);
char *emessage_to_string(struct err_message *msg);

struct err_message *ehandler_verrf(struct err_handler *handler,
				   const char *origin, const char *format,
				   va_list args);
struct err_message *ehandler_errf(struct err_handler *handler,
				  const char *origin, const char *format, ...);

#endif // ERRHANDLER_H