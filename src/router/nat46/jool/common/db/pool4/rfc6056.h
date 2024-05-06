#ifndef SRC_MOD_NAT64_POOL4_RFC6056_H_
#define SRC_MOD_NAT64_POOL4_RFC6056_H_

#include "common/types.h"
#include "mod/common/translation_state.h"

int rfc6056_setup(void);
void rfc6056_teardown(void);

int rfc6056_f(struct xlation *state, unsigned int *result);

#endif /* SRC_MOD_NAT64_POOL4_RFC6056_H_ */
