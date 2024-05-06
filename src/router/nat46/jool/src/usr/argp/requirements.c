#include "usr/argp/requirements.h"

#include <errno.h>
#include <stdio.h>

static struct requirement *get_next(struct requirement *current)
{
	for (current++; current->what; current++)
		if (!current->set)
			return current;
	return NULL;
}

int requirement_print(struct requirement *reqs)
{
	struct requirement *current;
	struct requirement *next;

	fprintf(stderr, "The command is missing ");
	for (current = reqs; current->what; current++) {
		if (current->set)
			continue;

		fprintf(stderr, "%s", current->what);

		next = get_next(current);
		if (next) {
			if (get_next(next))
				fprintf(stderr, ", ");
			else
				fprintf(stderr, " and ");
		}
	}
	fprintf(stderr, ".\n");

	return -EINVAL;
}
