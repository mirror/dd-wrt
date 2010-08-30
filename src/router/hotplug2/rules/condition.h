#ifndef RULES_CONDITION_H
#define RULES_CONDITION_H 1

#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define CONDITION_CMP_EQ	1
#define CONDITION_CMP_NE	2
#define CONDITION_CMP_REEQ	3
#define CONDITION_CMP_RENE	4
#define CONDITION_CMP_SET	5	/* "is set" */
#define CONDITION_CMP_UNSET	6	/* "is unset" */

union rvalue_u {
	char *string;
	regex_t preg;
};

struct condition_t {
	char *lvalue;
	union rvalue_u rvalue;
	int cmp;
};

struct condition_t *condition_create(const char *, const char *, int);
void condition_free(struct condition_t *);

#endif /* RULES_CONDITION_H */
