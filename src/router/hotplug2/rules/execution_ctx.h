#ifndef EXECUTION_CTX_H
#define EXECUTION_CTX_H 1

#include "../uevent.h"

#define BR_NONE   0
#define BR_UEVENT 1
#define BR_RULE   2

struct execution_ctx_t {
	struct uevent_t *uevent;
	void *settings;

	int last_rv;
	int branching;
};

#endif /* EXECUTION_CTX_H */
