#ifndef SRC_USR_NL_JSON_H_
#define SRC_USR_NL_JSON_H_

#include <linux/types.h>
#include "usr/util/cJSON.h"
#include "usr/util/result.h"

struct jool_result type_mismatch(char const *field, cJSON *json,
		char const *expected);
struct jool_result validate_uint(char const *field_name, cJSON *node,
		__u64 min, __u64 max);

#endif /* SRC_USR_NL_JSON_H_ */
