#include "execution.h"

static int condition_eval(struct condition_t *condition) {
	char *value;
	int rv;

	switch (condition->cmp) {
		case CONDITION_CMP_EQ:
		case CONDITION_CMP_NE:
			value = getenv(condition->lvalue);
			if (value == NULL)
				return -1;

			rv = strcmp(value, condition->rvalue.string);
			if (condition->cmp == CONDITION_CMP_NE)
				rv = !rv;

			return rv;

		case CONDITION_CMP_REEQ:
		case CONDITION_CMP_RENE:
			value = getenv(condition->lvalue);
			if (value == NULL)
				return -1;

			rv = regexec(&condition->rvalue.preg, value, 0, NULL, 0);
			if (condition->cmp == CONDITION_CMP_RENE)
				rv = !rv;

			return rv;

		case CONDITION_CMP_SET:
			value = getenv(condition->lvalue);
			return (value == NULL) ? -1 : 0;
			
		case CONDITION_CMP_UNSET:
			value = getenv(condition->lvalue);
			return (value == NULL) ? 0 : -1;
	}

	return -1;
}

static int rule_conditions_eval(struct rule_t *rule) {
	int i;
	int rv;

	for (i = 0; i < rule->conditions_c; i++) {
		rv = condition_eval(rule->conditions[i]);
		if (rv != 0)
			return rv;
	}
	return 0;
}

/**
 * This is basically a fairly ugly function that takes a string and
 * replaces %KEY% by the environment value KEY.
 *
 * @1 Original string
 *
 * Returns: Newly allocated string with all keys replaced.
 */
static char *replace_variables(char *original) {
	char *string;
	char *key;
	char *value;
	char *ptr, *sptr;
	int in_variable;
	size_t len, varlen;
	size_t offset;
	
	len = strlen(original);
	string = malloc(len+1);
	sptr = ptr = original;
	offset = 0;
	
	in_variable = 0;
	while(*ptr != '\0') {
		if (in_variable) {
			if (*ptr == '%') {
				key = malloc(ptr - sptr + 1);
				memcpy(key, sptr+1, ptr - sptr - 1);
				key[ptr - sptr - 1] = '\0';
				value = getenv(key);
				free(key);

				if (value) {
					/*
					 * We have a value, let's push it in
					 * instead of the %KEY% 
					 */
					varlen = strlen(value);
					len = len - (ptr-sptr) + varlen - 1;

					string = realloc(string, len + 1); 
					memcpy(string+offset, value, varlen);
					offset += varlen;
					sptr = ptr + 1;
				} else {
					/*
					 * No value. It's probably a literal then,
					 * let's push in "%KEY%". */
					memcpy(string+offset, sptr, ptr - sptr);
					offset += ptr - sptr;
					sptr = ptr;
				}   
				in_variable = 0;
			}   
		} else {
			if (*ptr == '%') {
				/* Add string */
				memcpy(string+offset, sptr, ptr - sptr);
				offset += ptr - sptr;
				sptr = ptr;
				in_variable = 1;
			}   
		}		 
				  
		ptr++;
	}   

	/* Add the rest. */
	memcpy(string+offset, sptr, ptr - sptr);
	string[len] = '\0';

	return string;
}


static char **rule_expression_variables(struct expression_t *expression, struct uevent_t *uevent) {
	char **argv;
	int i;

	argv = malloc(sizeof(char*) * (expression->argc + 1));
	argv[expression->argc] = NULL;
	for (i = 0; i < expression->argc; i++) {
		argv[i] = replace_variables(expression->argv[i]);
	}

	return argv;
}

static int rule_execute(struct rule_t *rule, struct execution_ctx_t *ctx) {
	int i, j;
	int rv;
	char **argv;

	rv = rule_conditions_eval(rule);
	if (rv != 0)
		return rv;

	ctx->branching = BR_NONE;
	ctx->last_rv = 0;
	for (i = 0; i < rule->expressions_c; i++) {
		/* If it's some compatibility or flag directive, skip it */
		if (rule->expressions[i]->command->command == NULL)
			continue;

		argv = rule_expression_variables(rule->expressions[i], ctx->uevent);

		ctx->last_rv = rule->expressions[i]->command->command(ctx, rule->expressions[i]->argc, argv);

		for (j = 0; j < rule->expressions[i]->argc; j++) {
			free(argv[j]);
		}
		free(argv);

		/* Deal with branching. */
		if (ctx->branching == BR_UEVENT) {
			break;
		} else if (ctx->branching == BR_RULE) {
			ctx->branching = BR_NONE;
			break;
		}
	}

	return ctx->last_rv;
}

int ruleset_flags(struct ruleset_t *ruleset, struct uevent_t *uevent) {
	int i;
	int flags;

	flags = FLAG_NONE;
	for (i = 0; i < ruleset->rules_c; i++) {
		if (rule_conditions_eval(ruleset->rules[i]))
			continue;
		flags |= ruleset->rules[i]->flags;
	}

	return flags;
}

int ruleset_execute(struct ruleset_t *ruleset, struct uevent_t *uevent, struct settings_t *settings) {
	int i;
	struct execution_ctx_t ctx;

	ctx.uevent = uevent;
	ctx.settings = settings;
	ctx.branching = BR_NONE;

	for (i = 0; i < ruleset->rules_c; i++) {
		rule_execute(ruleset->rules[i], &ctx);

		if (ctx.branching == BR_UEVENT)
			break;
	}

	return 0;
}
