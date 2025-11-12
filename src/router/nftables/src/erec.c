/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stdio.h>
#include <stdarg.h>

#include <netlink.h>
#include <gmputil.h>
#include <erec.h>

static const struct input_descriptor internal_indesc = {
	.type	= INDESC_INTERNAL,
	.name	= "internal",
};

const struct location internal_location = {
	.indesc	= &internal_indesc,
};

static const char * const error_record_names[] = {
	[EREC_INFORMATIONAL]	= NULL,
	[EREC_WARNING]		= "Warning",
	[EREC_ERROR]		= "Error"
};

void erec_add_location(struct error_record *erec, const struct location *loc)
{
	assert(erec->num_locations < EREC_LOCATIONS_MAX);
	erec->locations[erec->num_locations] = *loc;
	erec->locations[erec->num_locations].indesc = loc->indesc ?
						    : &internal_indesc;
	erec->num_locations++;
}

void erec_destroy(struct error_record *erec)
{
	free(erec->msg);
	free(erec);
}

__attribute__((format(printf, 3, 0)))
struct error_record *erec_vcreate(enum error_record_types type,
				  const struct location *loc,
				  const char *fmt, va_list ap)
{
	struct error_record *erec;

	erec = xmalloc(sizeof(*erec));
	erec->type		= type;
	erec->num_locations	= 0;
	erec_add_location(erec, loc);

	if (vasprintf(&erec->msg, fmt, ap) < 0)
		erec->msg = NULL;

	return erec;
}

__attribute__((format(printf, 3, 4)))
struct error_record *erec_create(enum error_record_types type,
				 const struct location *loc,
				 const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	va_start(ap, fmt);
	erec = erec_vcreate(type, loc, fmt, ap);
	va_end(ap);
	return erec;
}

void print_location(FILE *f, const struct input_descriptor *indesc,
		    const struct location *loc)
{
	const struct input_descriptor *tmp;
	const struct location *iloc;

	if (indesc->location.indesc != NULL) {
		const char *prefix = "In file included from";
		iloc = &indesc->location;
		for (tmp = iloc->indesc;
		     tmp != NULL && tmp->type != INDESC_INTERNAL;
		     tmp = iloc->indesc) {
			fprintf(f, "%s %s:%u:%u-%u:\n", prefix,
				tmp->name,
				iloc->first_line, iloc->first_column,
				iloc->last_column);
			prefix = "                 from";
			iloc = &tmp->location;
		}
	}
	if (indesc->type != INDESC_BUFFER && indesc->name) {
		fprintf(f, "%s:%u:%u-%u: ", indesc->name,
			loc->first_line, loc->first_column,
			loc->last_column);
	}
}

const char *line_location(const struct input_descriptor *indesc,
			  const struct location *loc, char *buf, size_t bufsiz)
{
	const char *line = NULL;
	FILE *f;

	f = fopen(indesc->name, "r");
	if (!f)
		return NULL;

	if (!fseek(f, loc->line_offset, SEEK_SET) &&
	    fread(buf, 1, bufsiz - 1, f) > 0) {
		*strchrnul(buf, '\n') = '\0';
		line = buf;
	}
	fclose(f);

	return line;
}

void erec_print(struct output_ctx *octx, const struct error_record *erec,
		unsigned int debug_mask)
{
	const struct location *loc = erec->locations;
	const struct input_descriptor *indesc = loc->indesc;
	const char *line = NULL;
	char buf[1024] = {};
	char *pbuf = NULL;
	unsigned int i, end;
	FILE *f;
	int l;

	switch (indesc->type) {
	case INDESC_BUFFER:
	case INDESC_CLI:
		line = indesc->data;
		*strchrnul(line, '\n') = '\0';
		break;
	case INDESC_STDIN:
		line = indesc->data;
		line += loc->line_offset;
		*strchrnul(line, '\n') = '\0';
		break;
	case INDESC_FILE:
		line = line_location(indesc, loc, buf, sizeof(buf));
		break;
	case INDESC_INTERNAL:
	case INDESC_NETLINK:
		break;
	default:
		BUG("invalid input descriptor type %u\n", indesc->type);
	}

	f = octx->error_fp;

	if (indesc->type == INDESC_NETLINK) {
		fprintf(f, "%s: ", indesc->name);
		if (error_record_names[erec->type])
			fprintf(f, "%s: ", error_record_names[erec->type]);
		fprintf(f, "%s\n", erec->msg);
		for (l = 0; l < (int)erec->num_locations; l++) {
			loc = &erec->locations[l];
			if (!loc->nle)
				continue;
			netlink_dump_expr(loc->nle, f, debug_mask);
		}
		return;
	}

	print_location(f, indesc, loc);

	if (error_record_names[erec->type])
		fprintf(f, "%s: ", error_record_names[erec->type]);
	fprintf(f, "%s\n", erec->msg);

	if (line) {
		fprintf(f, "%s\n", line);

		end = 0;
		for (l = erec->num_locations - 1; l >= 0; l--) {
			loc = &erec->locations[l];
			end = max(end, loc->last_column);
		}
		pbuf = xmalloc(end + 1);
		memset(pbuf, ' ', end + 1);
		for (i = 0; i < end && line[i]; i++) {
			if (line[i] == '\t')
				pbuf[i] = '\t';
		}
		for (l = erec->num_locations - 1; l >= 0; l--) {
			loc = &erec->locations[l];
			for (i = loc->first_column ? loc->first_column - 1 : 0;
			     i < loc->last_column; i++)
				pbuf[i] = l ? '~' : '^';
		}
		pbuf[end] = '\0';
		fprintf(f, "%s", pbuf);
		free(pbuf);
	}
	fprintf(f, "\n");
}

void erec_print_list(struct output_ctx *octx, struct list_head *list,
		     unsigned int debug_mask)
{
	struct error_record *erec, *next;

	list_for_each_entry_safe(erec, next, list, list) {
		list_del(&erec->list);
		erec_print(octx, erec, debug_mask);
		erec_destroy(erec);
	}
}

int __fmtstring(4, 5) __stmt_binary_error(struct eval_ctx *ctx,
					  const struct location *l1,
					  const struct location *l2,
					  const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	va_start(ap, fmt);
	erec = erec_vcreate(EREC_ERROR, l1, fmt, ap);
	if (l2 != NULL)
		erec_add_location(erec, l2);
	va_end(ap);
	erec_queue(erec, ctx->msgs);
	return -1;
}
