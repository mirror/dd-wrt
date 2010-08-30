#ifndef RULESET_H
#define RULESET_H 1

#include "rule.h"

struct ruleset_t {
	struct rule_t **rules;
	int rules_c;
};

void ruleset_init(struct ruleset_t *);
void ruleset_add_rule(struct ruleset_t *, struct rule_t *);
void ruleset_clear(struct ruleset_t *);

#endif /* RULESET_H */
