#ifndef RULES_EXPRESSION_H
#define RULES_EXPRESSION_H 1

#include "command.h"
#include "command-list.h"

#include <stdlib.h>
#include <string.h>

struct expression_t {
	struct command_def_t *command;
	char		**argv;
	int			argc;
};

struct expression_t *expression_create(const char *, int, char **);
void expression_free(struct expression_t *);

#endif /* RULES_EXPRESSION_H */
