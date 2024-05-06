#ifndef SRC_USR_ARGP_REQUIREMENTS_H_
#define SRC_USR_ARGP_REQUIREMENTS_H_

#include <stdbool.h>

struct requirement {
	bool set;
	char *what;
};

int requirement_print(struct requirement *reqs);

#endif /* SRC_USR_ARGP_REQUIREMENTS_H_ */
