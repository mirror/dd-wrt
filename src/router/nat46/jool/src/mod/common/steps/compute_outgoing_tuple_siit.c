#include "mod/common/steps/compute_outgoing_tuple.h"

#include "mod/common/address_xlat.h"
#include "mod/common/log.h"
#include "mod/common/db/eam.h"
#include "mod/common/db/rfc6791v4.h"
#include "mod/common/db/rfc6791v6.h"

verdict translate_addrs64_siit(struct xlation *state, __be32 *src_out,
		__be32 *dst_out)
{
	struct ipv6hdr *hdr6 = pkt_ip6_hdr(&state->in);
	struct result_addrxlat64 src, dst;
	struct addrxlat_result addr_result;

	/* Dst address. (SRC DEPENDS CON DST, SO WE NEED TO XLAT DST FIRST!) */
	addr_result = addrxlat_siit64(&state->jool, &hdr6->daddr, &dst, true);
	if (addr_result.reason) {
		log_debug(state, "Unable to translate %pI6c: %s", &hdr6->daddr,
				addr_result.reason);
	}

	switch (addr_result.verdict) {
	case ADDRXLAT_CONTINUE:
		*dst_out = dst.addr.s_addr;
		break;
	case ADDRXLAT_TRY_SOMETHING_ELSE:
		return untranslatable(state, JSTAT64_SRC);
	case ADDRXLAT_ACCEPT:
		return untranslatable(state, JSTAT64_SRC);
	case ADDRXLAT_DROP:
		return drop(state, JSTAT_UNKNOWN);
	}

	/* Src address. */
	addr_result = addrxlat_siit64(&state->jool, &hdr6->saddr, &src,
			!pkt_is_icmp6_error(&state->in));
	if (addr_result.reason) {
		log_debug(state, "Unable to translate %pI6c: %s", &hdr6->saddr,
				addr_result.reason);
	}

	switch (addr_result.verdict) {
	case ADDRXLAT_CONTINUE:
		break;
	case ADDRXLAT_TRY_SOMETHING_ELSE:
		if (pkt_is_icmp6_error(&state->in)
				&& !rfc6791v4_find(state, &src.addr)) {
			src.entry.method = AXM_RFC6791;
			break; /* Ok, success. */
		}
		return untranslatable(state, JSTAT64_DST);
	case ADDRXLAT_ACCEPT:
		return untranslatable(state, JSTAT64_DST);
	case ADDRXLAT_DROP:
		return drop(state, JSTAT_UNKNOWN);
	}

	*src_out = src.addr.s_addr;

	/*
	 * Mark intrinsic hairpinning if it's going to be needed.
	 * Why here? It's the only place where we know whether RFC 6052 was
	 * involved.
	 */
	if (state->jool.globals.siit.eam_hairpin_mode == EHM_INTRINSIC) {
		struct eam_table *eamt = state->jool.siit.eamt;
		/* Condition set A */
		if (pkt_is_outer(&state->in) && !pkt_is_icmp6_error(&state->in)
				&& (dst.entry.method == AXM_RFC6052)
				&& eamt_contains4(eamt, dst.addr.s_addr)) {
			state->is_hairpin = true;

		/* Condition set B */
		} else if (pkt_is_inner(&state->in)
				&& (src.entry.method == AXM_RFC6052)
				&& eamt_contains4(eamt, src.addr.s_addr)) {
			state->is_hairpin = true;
		}
	}

	log_debug(state, "Result: %pI4->%pI4", &src.addr, &dst.addr);
	return VERDICT_CONTINUE;
}

static bool disable_src_eam(struct packet *in, bool hairpin)
{
	struct iphdr *inner_hdr;

	if (!hairpin || pkt_is_inner(in))
		return false;
	if (!pkt_is_icmp4_error(in))
		return true;

	inner_hdr = pkt_payload(in);
	return pkt_ip4_hdr(in)->saddr == inner_hdr->daddr;
}

static bool disable_dst_eam(struct packet *in, bool hairpin)
{
	return hairpin && pkt_is_inner(in);
}

verdict translate_addrs46_siit(struct xlation *state, struct in6_addr *src_out,
		struct in6_addr *dst_out)
{
	struct packet *in = &state->in;
	struct iphdr *hdr4 = pkt_ip4_hdr(in);
	bool is_hairpin;
	struct result_addrxlat46 addr6;
	struct addrxlat_result addr_result;

	is_hairpin = (state->jool.globals.siit.eam_hairpin_mode == EHM_SIMPLE)
			|| state->is_hairpin;

	/* Dst address. (SRC DEPENDS CON DST, SO WE NEED TO XLAT DST FIRST!) */

	addr_result = addrxlat_siit46(&state->jool, hdr4->daddr, &addr6,
			!disable_dst_eam(in, is_hairpin), true);
	if (addr_result.reason) {
		log_debug(state, "Unable to translate %pI4: %s", &hdr4->daddr,
				addr_result.reason);
	}

	switch (addr_result.verdict) {
	case ADDRXLAT_CONTINUE:
		*dst_out = addr6.addr;
		break;
	case ADDRXLAT_TRY_SOMETHING_ELSE:
	case ADDRXLAT_ACCEPT:
		return untranslatable(state, JSTAT46_DST);
	case ADDRXLAT_DROP:
		return drop(state, JSTAT_UNKNOWN);
	}

	/* Src address. */
	addr_result = addrxlat_siit46(&state->jool, hdr4->saddr, &addr6,
			!disable_src_eam(in, is_hairpin),
			!pkt_is_icmp4_error(in));
	if (addr_result.reason) {
		log_debug(state, "Unable to translate %pI4: %s", &hdr4->saddr,
				addr_result.reason);
	}

	switch (addr_result.verdict) {
	case ADDRXLAT_CONTINUE:
		break;
	case ADDRXLAT_TRY_SOMETHING_ELSE:
		if (pkt_is_icmp4_error(in)
				&& !rfc6791v6_find(state, &addr6.addr)) {
			addr6.entry.method = AXM_RFC6791;
			break; /* Ok, success. */
		}
		return untranslatable(state, JSTAT46_SRC);
	case ADDRXLAT_ACCEPT:
		return untranslatable(state, JSTAT46_SRC);
	case ADDRXLAT_DROP:
		return drop(state, JSTAT_UNKNOWN);
	}

	*src_out = addr6.addr;

	log_debug(state, "Result: %pI6c->%pI6c", src_out, dst_out);
	return VERDICT_CONTINUE;
}
