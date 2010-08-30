#include "ruleset.h"

void ruleset_init(struct ruleset_t *ruleset) {
	ruleset->rules_c = 0;
	ruleset->rules = NULL;
}

void ruleset_add_rule(struct ruleset_t *ruleset, struct rule_t *rule) {
	ruleset->rules_c++;
	ruleset->rules = realloc(ruleset->rules, sizeof(struct rule_t *) * ruleset->rules_c);
	ruleset->rules[ruleset->rules_c-1] = rule;
}

void ruleset_clear(struct ruleset_t *ruleset) {
	int i;

	for (i = 0; i < ruleset->rules_c; i++) {
		rule_free(ruleset->rules[i]);
	}
	free(ruleset->rules);

	ruleset_init(ruleset);
}
