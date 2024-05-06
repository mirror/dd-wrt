#ifndef SRC_USR_UTIL_RESULT_H_
#define SRC_USR_UTIL_RESULT_H_

#include <stdbool.h>

/* -- Jool Result Flags -- */
/* Does msg need to be freed? */
#define JRF_MSG_IN_HEAP (1 << 0)
/*
 * Was this jool_result initialized?
 * This flag doesn't work on its own, obviously. Zeroize the result beforehand
 * if you plan on reading it.
 */
#define JRF_INITIALIZED (1 << 1)

struct jool_result {
	int error;
	char *msg;
	unsigned int flags;
};

struct jool_result result_success(void);
struct jool_result result_from_error(int errcode, char const *msg, ...);
struct jool_result result_from_enomem(void);

void result_cleanup(struct jool_result *);

#endif /* SRC_USR_UTIL_RESULT_H_ */
