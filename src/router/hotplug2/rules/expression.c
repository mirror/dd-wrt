#include "expression.h"

struct expression_t *expression_create(const char *name, int argc, char **argv) {
	int i;
	struct command_def_t *command;
	struct expression_t *expression;

	/* Resolve command name to function pointer */
	command = NULL;
	for (i = 0; commands[i].name != NULL; i++) {
		if (!strcmp(commands[i].name, name)) {
			command = &commands[i];
			break;
		}
	}
	if (command == NULL)
		return NULL;
	
	/* Arity check */
	if (argc < command->minarity)
		return NULL;
	if (argc > command->maxarity && command->maxarity >= 0)
		return NULL;

	expression = malloc(sizeof(struct expression_t));

	expression->argc = argc;
	expression->argv = argv;	
	expression->command = command;

	return expression;
}

void expression_free(struct expression_t *expression) {
	int i;

	for (i = 0; i < expression->argc; i++) {
		free(expression->argv[i]);
	}
	free(expression->argv);
	free(expression);
}
