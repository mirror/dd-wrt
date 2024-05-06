#ifndef TEST_GRAYBOX_MOD_HEADER_H_
#define TEST_GRAYBOX_MOD_HEADER_H_

#include "expecter.h"

unsigned int collect_errors(struct expected_packet const *expected,
		struct sk_buff const *actual);

#endif /* TEST_GRAYBOX_MOD_HEADER_H_ */
