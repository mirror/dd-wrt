#ifndef SRC_MOD_COMMON_CORE_H_
#define SRC_MOD_COMMON_CORE_H_

/**
 * @file
 * The core is the packet handling's entry point.
 */

#include <linux/skbuff.h>
#include "mod/common/translation_state.h"

/**
 * Assumes @skb is an IPv6 packet, and attempts to translate and send its IPv4
 * counterpart.
 */
verdict core_6to4(struct sk_buff *skb, struct xlation *state);
/**
 * Assumes @skb is an IPv4 packet, and attempts to translate and send its IPv4
 * counterpart.
 */
verdict core_4to6(struct sk_buff *skb, struct xlation *state);

#endif /* SRC_MOD_COMMON_CORE_H_ */
