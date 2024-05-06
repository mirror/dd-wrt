#ifndef SRC_MOD_COMMON_ICMP_WRAPPER_H_
#define SRC_MOD_COMMON_ICMP_WRAPPER_H_

/**
 * @file
 * Direct use of the icmp_send() and icmpv6_send() functions after the determine
 * incoming tuple step is bound to become a bug nest. That's because steps
 * filtering through translate are reused in hairpinning, so when an error
 * occurs while translating a IPv4 packet, one cannot assume that the resulting
 * ICMP error will be a IPv4 one.
 *
 * In those situations, you can use this code instead. It transparently sends
 * the correct ICMP error no matter where you are.
 *
 * For the sake of consistency, and so the unit tests don't send bogus ICMP
 * errors left and right (because the unit tests use an impersonator no-op ICMP
 * wrapper), use this module even if your code isn't reused in hairpinning,
 * please.
 */

#include "mod/common/packet.h"
#include "mod/common/xlator.h"

typedef enum icmp_errcode {
	ICMPERR_NONE,
	ICMPERR_ADDR_UNREACHABLE,
	ICMPERR_PORT_UNREACHABLE,
	ICMPERR_PROTO_UNREACHABLE,
	ICMPERR_TTL,
	ICMPERR_FRAG_NEEDED,
	ICMPERR_HDR_FIELD,
	ICMPERR_SRC_ROUTE,
	ICMPERR_FILTER,
} icmp_error_code;

/**
 * Wrappers for icmp_send() and icmpv6_send().
 */
bool icmp64_send6(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info);
bool icmp64_send4(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info);
bool icmp64_send(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info);

/**
 * Return the numbers of icmp error that was sent, also reset the static counter
 * This is only used in Unit Testing.
 */
int icmp64_pop(void);


#endif /* SRC_MOD_COMMON_ICMP_WRAPPER_H_ */
