#include "rule.h"

struct rule_t *rule_create(void) {
	struct rule_t *rule;

	rule = malloc(sizeof(struct rule_t));

	rule->conditions = NULL;
	rule->conditions_c = 0;
	rule->expressions = NULL;
	rule->expressions_c = 0;

	rule->flags = 0;

	return rule;
}

void rule_add_condition(struct rule_t *rule, struct condition_t *condition) {
	rule->conditions_c++;
	rule->conditions = realloc(rule->conditions, sizeof(struct condition_t *) * rule->conditions_c);
	rule->conditions[rule->conditions_c-1] = condition;
}

void rule_add_expression(struct rule_t *rule, struct expression_t *expression) {
	rule->expressions_c++;
	rule->expressions = realloc(rule->expressions, sizeof(struct expression_t *) * rule->expressions_c);
	rule->expressions[rule->expressions_c-1] = expression;

	rule->flags |= expression->command->flags;
}

void rule_free(struct rule_t *rule) {
	int i;
	for (i = 0; i < rule->conditions_c; i++) {
		condition_free(rule->conditions[i]);
	}
	free(rule->conditions);

	for (i = 0; i < rule->expressions_c; i++) {
		expression_free(rule->expressions[i]);
	}
	free(rule->expressions);

	free(rule);
}

