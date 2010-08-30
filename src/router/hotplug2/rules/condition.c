#include "condition.h"

struct condition_t *condition_create(const char *lvalue, const char *rvalue, int cmp) {
	struct condition_t *condition;

	condition = malloc(sizeof(struct condition_t));

	condition->cmp = cmp;
	condition->lvalue = strdup(lvalue);

	/*rvalue may be regex or string*/
	switch (cmp) {
		case CONDITION_CMP_EQ:
		case CONDITION_CMP_NE:
			condition->rvalue.string = strdup(rvalue);
			break;

		case CONDITION_CMP_REEQ:
		case CONDITION_CMP_RENE:
			if (regcomp(&condition->rvalue.preg, rvalue, REG_EXTENDED|REG_NOSUB)) {
				condition_free(condition);
				return NULL;
			}
			
			break;
	}

	return condition;
}

void condition_free(struct condition_t *condition) {
	free(condition->lvalue);

	switch (condition->cmp) {
		case CONDITION_CMP_EQ:
		case CONDITION_CMP_NE:
			free(condition->rvalue.string);
			break;

		case CONDITION_CMP_REEQ:
		case CONDITION_CMP_RENE:
			regfree(&condition->rvalue.preg);
			break;
	}

	free(condition);
}
