#ifndef SRC_MOD_NAT64_DETERMINE_INCOMING_TUPLE_H_
#define SRC_MOD_NAT64_DETERMINE_INCOMING_TUPLE_H_

/**
 * @file
 * The first step in the packet processing algorithm defined in the RFC.
 * The 3.4 section of RFC 6146 is encapsulated in this module.
 * Creates a tuple (summary) of the incoming packet.
 */

#include "mod/common/translation_state.h"

verdict determine_in_tuple(struct xlation *state);


#endif /* SRC_MOD_NAT64_DETERMINE_INCOMING_TUPLE_H_ */
