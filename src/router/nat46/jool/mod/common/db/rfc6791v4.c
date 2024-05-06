#include "mod/common/db/rfc6791v4.h"

/**
 * Returns in @result the IPv4 address the ICMP error should be sourced with.
 */
static int get_pool_address(struct xlation *state, struct in_addr *result)
{
	struct ipv4_prefix *pool;
	__u32 n; /* We are going to return the "n"th address. */

	if (state->jool.globals.siit.randomize_error_addresses)
		get_random_bytes(&n, sizeof(n));
	else
		n = pkt_ip6_hdr(&state->in)->hop_limit;

	pool = &state->jool.globals.siit.rfc6791_prefix4.prefix;
	n &= ~get_prefix4_mask(pool);
	result->s_addr = cpu_to_be32(be32_to_cpu(pool->addr.s_addr) | n);

	return 0;
}

/**
 * Flags the source address for automatic completion later.
 *
 * (We cannot compute the actual address at this point, because figuring out its
 * ideal value requires routing, which we can't do until certain packet fields
 * are translated.)
 */
static int get_host_address(struct in_addr *result)
{
	result->s_addr = 0;
	return 0;
}

int rfc6791v4_find(struct xlation *state, struct in_addr *result)
{
	return state->jool.globals.siit.rfc6791_prefix4.set
			? get_pool_address(state, result)
			: get_host_address(result);
}
