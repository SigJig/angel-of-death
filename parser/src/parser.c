
#include "parser.h"
#include <stdarg.h>

static struct parser_line *parser_curline_reset(struct parser *parser)
{
	struct parser_line *old = parser->state.cur_line;
	struct parser_line *cur_line = malloc(sizeof *cur_line);

	cur_line->level = NULL;
	cur_line->xref = NULL;
	cur_line->tag = NULL;
	cur_line->line_value = NULL;
	cur_line->next = NULL;

	parser->state.cur_line = cur_line;

	return old;
}

static e_statuscode parser_init(struct parser *parser,
				struct err_handler *ehandler)
{
	if (!parser || !ehandler)
		return ST_INIT_FAIL;

	parser->stack_idx = 0;
	parser->result.front = NULL;
	parser->result.back = NULL;
	parser->ehandler = ehandler;

	if (!ehandler) {
		return ST_INIT_FAIL;
	}

	parser_curline_reset(parser);

	parser->state.index = 0;
}

static void parser_destroy(struct parser *parser)
{
	if (!parser)
		return;

	if (parser->state.cur_line) {
		parser_line_free(parser->state.cur_line);
	}

	parser->ehandler = NULL;
}

static struct err_message *parser_errf_from(struct parser *parser,
					    const char *format, ...)
{
	va_list args;
	va_start(args, format);

	struct err_message *result =
	    ehandler_verrf(parser->ehandler, "parser", format, args);

	va_end(args);

	return result;
}

static struct err_message *parser_err_unexpected(struct parser *parser,
						 struct lex_token *token)
{
	return parser_errf_from(
	    parser, "unexpected token type (line: %zu, col: %zu) :%d",
	    token->line, token->col, token->type);
}

static bool parser_valid_at_idx(struct parser *parser, struct lex_token *token,
				int index)
{
	// gedcom_line: level delim [xref delim] tag [delim line_value]
	// terminator
	switch (index) {
	case 0:
		return (token->type == LT_NUMBER);
	case 2:
		return (token->type == LT_POINTER);
	case 4:
		return (token->type == LT_S_ALNUM || token->type == LT_NUMBER);
	default:
		return (token->type != LT_TERMINATOR);
	}

	return false;
}

static struct parser_line *parser_curline_terminate(struct parser *parser)
{
	struct parser_line *line = parser_curline_reset(parser);

	if (parser->result.back) {
		(parser->result.back)->next = line;
		parser->result.back = line;
	} else {
		parser->result.front = line;
		parser->result.back = line;
	}

	return line;
}

static void parser_update_at(struct parser *parser, struct lex_token *token,
			     int index)
{
	struct parser_line *cur_line = parser->state.cur_line;
	struct lex_token *token_copy = lex_token_copy(token);

	switch (index) {
	case 0: {
		cur_line->level = token_copy;
		break;
	}
	case 2: {
		cur_line->xref = token_copy;
		break;
	}
	case 4: {
		cur_line->tag = token_copy;
		break;
	}
	default: {
		// The optional line_value can contain multiple values
		struct lex_token *back = cur_line->line_value;

		if (back) {
			for (; back->next != NULL; back = back->next)
				;
			back->next = token_copy;
		} else {
			cur_line->line_value = token_copy;
		}
	}
	}
}

/*
=================================================
END INTERNAL
=================================================
*/

void parser_line_free(struct parser_line *line)
{
	lex_token_free(line->level);
	lex_token_free(line->xref);
	lex_token_free(line->tag);

	struct lex_token *tok = line->line_value;
	struct lex_token *tmp = NULL;

	while (tok) {
		tmp = tok;
		tok = tok->next;

		lex_token_free(tmp);
	}

	free(line);
}

void parser_result_destroy(struct parser_result *result)
{
	struct parser_line *line = result->front;
	struct parser_line *tmp = NULL;

	while (line) {
		tmp = line;
		line = line->next;

		parser_line_free(tmp);
	}
}

e_statuscode parser_parse_token(struct parser *parser, struct lex_token *token)
{
	int index = parser->state.index;

	parser->state.index++;

	// leading whitespace
	if (index && token->type == LT_WHITESPACE || token->type == LT_DELIM ||
	    token->type == LT_TERMINATOR) {
		return ST_NOT_OK;
	}

	if (index < 6 && index & 1) {
		if (token->type == LT_DELIM)
			return ST_NOT_OK;

		parser_err_unexpected(parser, token);
		return ST_GEN_ERROR;
	} else if (index > 6 && token->type == LT_TERMINATOR) {
		parser_curline_terminate(parser);
		return ST_OK;
	}

	if (!parser_valid_at_idx(parser, token, index)) {
		parser_err_unexpected(parser, token);
		return ST_GEN_ERROR;
	}

	parser_update_at(parser, token, index);

	return ST_NOT_OK;
}

struct parser_result parser_parse(struct lex_token *tokens,
				  struct err_handler *ehandler)
{
	struct parser_result empty = {.front = NULL, .back = NULL};

	if (!tokens)
		return empty;

	struct parser parser;

	if (!parser_init(&parser, ehandler)) {
		parser_destroy(&parser);

		return empty;
	}

	for (struct lex_token *tok = tokens; tok; tok = tok->next) {
		parser_parse_token(&parser, tok);
	}

	parser_destroy(&parser);

	return parser.result;
}
