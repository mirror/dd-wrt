#ifndef SRC_MOD_NAT64_COMPUTE_OUTGOING_TUPLE_H_
#define SRC_MOD_NAT64_COMPUTE_OUTGOING_TUPLE_H_

/**
 * @file
 * Third step in the packet processing algorithm defined in the RFC.
 * The 3.6 section of RFC 6146 is encapsulated in this module.
 * Infers a tuple (summary) of the outgoing packet, yet to be created.
 */

#include "mod/common/translation_state.h"

verdict translate_addrs64_siit(struct xlation *state, __be32 *src_out,
		__be32 *dst_out);
verdict translate_addrs46_siit(struct xlation *state, struct in6_addr *src_out,
		struct in6_addr *dst_out);

verdict compute_out_tuple(struct xlation *state);

#endif /* SRC_MOD_NAT64_COMPUTE_OUTGOING_TUPLE_H_ */
