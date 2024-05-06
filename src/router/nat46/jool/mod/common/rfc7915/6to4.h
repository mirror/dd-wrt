#ifndef SRC_MOD_COMMON_RFC7915_6TO4_H_
#define SRC_MOD_COMMON_RFC7915_6TO4_H_

/**
 * @file
 * Actual translation of packet contents from from IPv6 to IPv4.
 *
 * This is RFC 7915 sections 5.1, 5.1.1, 5.2 and 5.3. Not to be confused with
 * the technology called "6to4", which is RFC 3056.
 */

#include "mod/common/rfc7915/common.h"

extern const struct translation_steps ttp64_steps;

verdict predict_route64(struct xlation *state);

#endif /* SRC_MOD_COMMON_RFC7915_6TO4_H_ */
