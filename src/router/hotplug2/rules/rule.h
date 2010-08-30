#ifndef RULE_H
#define RULE_H 1

#include "condition.h"
#include "expression.h"

struct rule_t {
	struct condition_t **conditions;
	int conditions_c;

	struct expression_t **expressions;
	int expressions_c;
	
	int flags;
};

struct rule_t *rule_create(void);
void rule_add_condition(struct rule_t *, struct condition_t *);
void rule_add_expression(struct rule_t *, struct expression_t *);
void rule_free(struct rule_t *);

#endif /* RULE_H */
