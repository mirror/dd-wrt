#ifndef SRC_MOD_COMMON_RFC7915_CORE_H_
#define SRC_MOD_COMMON_RFC7915_CORE_H_

/**
 * @file
 * This is the face of the "Translating the Packet" code. Files outside of this
 * folder should only see the API exposed by this file.
 *
 * "Translating the Packet" is the core translation of SIIT and the fourth step
 * of NAT64 (RFC6146 section 3.7).
 */

#include <linux/ip.h>
#include <linux/skbuff.h>
#include "mod/common/translation_state.h"

verdict translating_the_packet(struct xlation *state);

#endif /* SRC_MOD_COMMON_RFC7915_CORE_H_ */
