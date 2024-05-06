#include "empty.h"

#include "common/constants.h"
#include "mod/common/dev.h"
#include "mod/common/ipv6_hdr_iterator.h"
#include "mod/common/log.h"
#include "mod/common/rfc6052.h"
#include "mod/common/translation_state.h"
#include "mod/common/xlator.h"
#include "mod/common/rfc7915/6to4.h"

static int check_ifa(struct in_ifaddr *ifa, void const *arg)
{
	struct in_addr const *addr = arg;

	if (ifa->ifa_scope != RT_SCOPE_UNIVERSE)
		return 0;
	if (ifa->ifa_local == addr->s_addr)
		return 1;

	return 0;
}

static bool contains_addr(struct net *ns, const struct in_addr *addr)
{
	return foreach_ifa(ns, check_ifa, addr);
}

bool pool4empty_contains(struct net *ns, const struct ipv4_transport_addr *addr)
{
	if (addr->l4 < DEFAULT_POOL4_MIN_PORT)
		return false;

	return contains_addr(ns, &addr->l3);
}

/**
 * Initializes @range with the address candidates that could source @state's
 * outgoing packet.
 */
verdict pool4empty_find(struct xlation *state, struct ipv4_range *range)
{
	verdict result;

	if (__rfc6052_6to4(&state->jool.globals.pool6.prefix,
			&state->in.tuple.dst.addr6.l3,
			&state->out.tuple.dst.addr4.l3))
		return untranslatable(state, JSTAT_UNTRANSLATABLE_DST6);
	state->out.tuple.dst.addr4.l4 = state->in.tuple.dst.addr6.l4;

	result = predict_route64(state);
	if (result != VERDICT_CONTINUE)
		return result;

	range->prefix.addr.s_addr = state->flowx.v4.flowi.saddr;
	range->prefix.len = 0;
	range->ports.min = DEFAULT_POOL4_MIN_PORT;
	range->ports.max = DEFAULT_POOL4_MAX_PORT;
	return VERDICT_CONTINUE;
}
