#include "mod/common/rfc7915/6to4.h"

#include <linux/inetdevice.h>
#include <net/ip6_checksum.h>
#include <net/udp.h>
#include <net/tcp.h>

#include "mod/common/ipv6_hdr_iterator.h"
#include "mod/common/linux_version.h"
#include "mod/common/log.h"
#include "mod/common/route.h"
#include "mod/common/steps/compute_outgoing_tuple.h"

static __u8 xlat_tos(struct jool_globals const *config, struct ipv6hdr const *hdr)
{
	return config->reset_tos ? config->new_tos : get_traffic_class(hdr);
}

/**
 * One-liner for creating the IPv4 header's Protocol field.
 */
static __u8 xlat_proto(struct ipv6hdr const *hdr6)
{
	struct hdr_iterator iterator = HDR_ITERATOR_INIT(hdr6);
	hdr_iterator_last(&iterator);
	return (iterator.hdr_type == NEXTHDR_ICMP)
			? IPPROTO_ICMP
			: iterator.hdr_type;
}

static verdict xlat64_external_addresses(struct xlation *state)
{
	struct flowi4 *flow = &state->flowx.v4.flowi;

	switch (xlator_get_type(&state->jool)) {
	case XT_NAT64:
		flow->saddr = state->out.tuple.src.addr4.l3.s_addr;
		flow->daddr = state->out.tuple.dst.addr4.l3.s_addr;
		return VERDICT_CONTINUE;

	case XT_SIIT:
		return translate_addrs64_siit(state, &flow->saddr, &flow->daddr);
	}

	WARN(1, "xlator type is not SIIT nor NAT64: %u",
			xlator_get_type(&state->jool));
	return drop(state, JSTAT_UNKNOWN);
}

static verdict xlat64_internal_addresses(struct xlation *state)
{
	struct bkp_skb_tuple bkp;
	verdict result;

	if (pkt_is_inner(&state->in))
		return VERDICT_CONTINUE; /* Called from xlat64_icmp_type() */

	switch (xlator_get_type(&state->jool)) {
	case XT_NAT64:
		state->flowx.v4.inner_src = state->out.tuple.dst.addr4.l3;
		state->flowx.v4.inner_dst = state->out.tuple.src.addr4.l3;
		return VERDICT_CONTINUE;

	case XT_SIIT:
		result = become_inner_packet(state, &bkp, false);
		if (result != VERDICT_CONTINUE)
			return result;
		log_debug(state, "Translating internal addresses...");
		result = translate_addrs64_siit(state,
				&state->flowx.v4.inner_src.s_addr,
				&state->flowx.v4.inner_dst.s_addr);
		restore_outer_packet(state, &bkp, false);
		return result;
	}

	WARN(1, "xlator type is not SIIT nor NAT64: %u",
			xlator_get_type(&state->jool));
	return drop(state, JSTAT_UNKNOWN);
}

static verdict xlat64_tcp_ports(struct xlation *state)
{
	struct flowi4 *flow4;
	struct packet const *in;
	struct tcphdr const *hdr;

	flow4 = &state->flowx.v4.flowi;
	switch (xlator_get_type(&state->jool)) {
	case XT_NAT64:
		flow4->fl4_sport = cpu_to_be16(state->out.tuple.src.addr4.l4);
		flow4->fl4_dport = cpu_to_be16(state->out.tuple.dst.addr4.l4);
		break;
	case XT_SIIT:
		in = &state->in;
		if (is_first_frag6(pkt_frag_hdr(in))) {
			hdr = pkt_tcp_hdr(in);
			flow4->fl4_sport = hdr->source;
			flow4->fl4_dport = hdr->dest;
		}
	}

	return VERDICT_CONTINUE;
}

static verdict xlat64_udp_ports(struct xlation *state)
{
	struct flowi4 *flow4;
	struct packet const *in;
	struct udphdr const *udp;

	flow4 = &state->flowx.v4.flowi;
	switch (xlator_get_type(&state->jool)) {
	case XT_NAT64:
		flow4->fl4_sport = cpu_to_be16(state->out.tuple.src.addr4.l4);
		flow4->fl4_dport = cpu_to_be16(state->out.tuple.dst.addr4.l4);
		break;
	case XT_SIIT:
		in = &state->in;
		if (is_first_frag6(pkt_frag_hdr(in))) {
			udp = pkt_udp_hdr(in);
			flow4->fl4_sport = udp->source;
			flow4->fl4_dport = udp->dest;
		}
	}

	return VERDICT_CONTINUE;
}

static verdict xlat64_icmp_type(__u8 *out_type, __u8 *out_code,
		struct xlation *state)
{
	struct icmp6hdr const *in_hdr = pkt_icmp6_hdr(&state->in);

	switch (in_hdr->icmp6_type) {
	case ICMPV6_ECHO_REQUEST:
		*out_type = ICMP_ECHO;
		*out_code = 0;
		return VERDICT_CONTINUE;

	case ICMPV6_ECHO_REPLY:
		*out_type = ICMP_ECHOREPLY;
		*out_code = 0;
		return VERDICT_CONTINUE;

	case ICMPV6_DEST_UNREACH:
		*out_type = ICMP_DEST_UNREACH;
		switch (in_hdr->icmp6_code) {
		case ICMPV6_NOROUTE:
		case ICMPV6_NOT_NEIGHBOUR:
		case ICMPV6_ADDR_UNREACH:
			*out_code = ICMP_HOST_UNREACH;
			return xlat64_internal_addresses(state);
		case ICMPV6_ADM_PROHIBITED:
			*out_code = ICMP_HOST_ANO;
			return xlat64_internal_addresses(state);
		case ICMPV6_PORT_UNREACH:
			*out_code = ICMP_PORT_UNREACH;
			return xlat64_internal_addresses(state);
		}
		break;

	case ICMPV6_PKT_TOOBIG:
		*out_type = ICMP_DEST_UNREACH;
		*out_code = ICMP_FRAG_NEEDED;
		return xlat64_internal_addresses(state);

	case ICMPV6_TIME_EXCEED:
		*out_type = ICMP_TIME_EXCEEDED;
		*out_code = in_hdr->icmp6_code;
		return xlat64_internal_addresses(state);

	case ICMPV6_PARAMPROB:
		switch (in_hdr->icmp6_code) {
		case ICMPV6_HDR_FIELD:
			*out_type = ICMP_PARAMETERPROB;
			*out_code = 0;
			return xlat64_internal_addresses(state);
		case ICMPV6_UNK_NEXTHDR:
			*out_type = ICMP_DEST_UNREACH;
			*out_code = ICMP_PROT_UNREACH;
			return xlat64_internal_addresses(state);
		}
	}

	/*
	 * The following codes are known to fall through here:
	 * ICMPV6_MGM_QUERY, ICMPV6_MGM_REPORT, ICMPV6_MGM_REDUCTION, Neighbor
	 * Discover messages (133 - 137).
	 */
	log_debug(state, "ICMPv6 messages type %u code %u lack an ICMPv4 counterpart.",
			in_hdr->icmp6_type, in_hdr->icmp6_code);
	return drop(state, JSTAT_UNKNOWN_ICMP6_TYPE);
}

/** Initializes state->flowx. */
static verdict compute_flowix64(struct xlation *state)
{
	struct flowi4 *flow4;
	struct ipv6hdr const *hdr6;
	verdict result;

	flow4 = &state->flowx.v4.flowi;
	hdr6 = pkt_ip6_hdr(&state->in);

	flow4->flowi4_mark = state->in.skb->mark;
	flow4->flowi4_tos = xlat_tos(&state->jool.globals, hdr6);
	flow4->flowi4_scope = RT_SCOPE_UNIVERSE;
	flow4->flowi4_proto = xlat_proto(hdr6);
	/*
	 * ANYSRC disables the source address reachable validation.
	 * It's best to include it because none of the xlat addresses are
	 * required to be present in the routing table.
	 */
	flow4->flowi4_flags = FLOWI_FLAG_ANYSRC;

	result = xlat64_external_addresses(state);
	if (result != VERDICT_CONTINUE)
		return result;

	switch (flow4->flowi4_proto) {
	case IPPROTO_TCP:
		return xlat64_tcp_ports(state);
	case IPPROTO_UDP:
		return xlat64_udp_ports(state);
	case IPPROTO_ICMP:
		return xlat64_icmp_type(&state->flowx.v4.flowi.fl4_icmp_type,
				&state->flowx.v4.flowi.fl4_icmp_code, state);
	}

	return VERDICT_CONTINUE;
}

static verdict select_good_saddr(struct xlation *state)
{
	struct flowi4 *flow4 = &state->flowx.v4.flowi;

	flow4->saddr = inet_select_addr(state->dst->dev, flow4->daddr,
			RT_SCOPE_UNIVERSE);
	if (flow4->saddr == 0) {
		log_debug(state, "ICMPv6 error has untranslatable source, but the kernel could not find a suitable source for destination %pI4.",
				&flow4->daddr);
		return drop(state, JSTAT64_6791_ENOENT);
	}

	return VERDICT_CONTINUE;
}

static verdict select_any_saddr(struct xlation *state)
{
	struct net_device *dev;
	struct in_device *in_dev;
#if LINUX_VERSION_AT_LEAST(5, 3, 0, 8, 0)
	struct in_ifaddr *ifa;
#endif

	rcu_read_lock();
	for_each_netdev_rcu(state->jool.ns, dev) {
		in_dev = __in_dev_get_rcu(dev);
		if (!in_dev)
			continue;

#if LINUX_VERSION_AT_LEAST(5, 3, 0, 8, 0)
		in_dev_for_each_ifa_rcu(ifa, in_dev) {
			if (ifa->ifa_flags & IFA_F_SECONDARY)
				continue;
			if (ifa->ifa_scope == RT_SCOPE_UNIVERSE) {
				state->flowx.v4.flowi.saddr = ifa->ifa_local;
				goto success;
			}
		}
#else
		for_primary_ifa(in_dev) {
			if (ifa->ifa_scope == RT_SCOPE_UNIVERSE) {
				state->flowx.v4.flowi.saddr = ifa->ifa_local;
				goto success;
			}
		} endfor_ifa(in_dev);
#endif
	}

	rcu_read_unlock();
	log_debug(state, "ICMPv6 error has untranslatable source, and there aren't any universe-scoped addresses to mask it with.");
	return drop(state, JSTAT64_6791_ENOENT);

success:
	rcu_read_unlock();
	return VERDICT_CONTINUE;
}

/**
 * Initializes state->dst.
 * Please note: The resulting dst might be NULL even on VERDICT_CONTINUE.
 * Handle properly.
 */
static verdict __predict_route64(struct xlation *state)
{
	struct flowi4 *flow4;
	verdict result;

#ifdef UNIT_TESTING
	return VERDICT_CONTINUE;
#endif

	flow4 = &state->flowx.v4.flowi;

	if (state->is_hairpin) {
		log_debug(state, "Packet is hairpinning; skipping routing.");
	} else {
		log_debug(state, "Routing: %pI4->%pI4", &flow4->saddr, &flow4->daddr);
		state->dst = route4(&state->jool, flow4);
		if (!state->dst)
			return untranslatable(state, JSTAT_FAILED_ROUTES);
	}

	if (flow4->saddr == 0) { /* Empty pool4 or empty pool6791v4 */
		if (state->dst) {
			result = select_good_saddr(state);
			if (result != VERDICT_CONTINUE) {
				dst_release(state->dst);
				state->dst = NULL;
				return result;
			}
		} else {
			result = select_any_saddr(state);
			if (result != VERDICT_CONTINUE)
				return result;
		}
	}

	return VERDICT_CONTINUE;
}

verdict predict_route64(struct xlation *state)
{
	verdict result;

	if (!state->flowx_set) {
		result = compute_flowix64(state);
		if (result != VERDICT_CONTINUE)
			return result;
		state->flowx_set = true;
	}

	if (!state->dst) {
		result = __predict_route64(state);
		if (result != VERDICT_CONTINUE)
			return result;
	}

	return VERDICT_CONTINUE;
}

/*
 * Returns:
 * 0: Packet does not exceed MTU.
 * 1: First fragment exceeds MTU. (ie. PTB needed)
 * 2: Subsequent fragment exceeds MTU. (ie. PTB not needed)
 */
static int fragment_exceeds_mtu64(struct packet const *in, unsigned int mtu)
{
	struct sk_buff *iter;
	unsigned short gso_size;
	int delta;

	/*
	 * shinfo->gso_size is the value the kernel uses (during resegmentation)
	 * to remember the length of the original segments after GRO.
	 *
	 * Interestingly, if packet A has frag_list fragments B, and B have
	 * frags fragments C, then A's gso_size also applies to B, as well as C.
	 *
	 * (Note: Ugh. This comment is old. I don't remember if I checked
	 * whether B's gso_size was nonzero.)
	 *
	 * I don't know if gso_size can be populated if there are frag_list
	 * fragments but not frags fragments. Luckily, this code should work
	 * either way.
	 *
	 * See ip_exceeds_mtu() and ip6_pkt_too_big().
	 */

	gso_size = skb_shinfo(in->skb)->gso_size;
	if (gso_size) {
		if (sizeof(struct iphdr) + pkt_l4hdr_len(in) + gso_size > mtu)
			goto generic_too_big;
		return 0;
	}

	delta = sizeof(struct iphdr) - pkt_l3hdr_len(in);
	if (skb_headlen(in->skb) + delta > mtu)
		goto generic_too_big;

	/*
	 * TODO (performance) This loop could probably be optimized away by
	 * querying IP6CB(skb)->frag_max_size. You'll have to test it.
	 */
	mtu -= sizeof(struct iphdr);
	skb_walk_frags(in->skb, iter)
		if (iter->len > mtu)
			return 2;

	return 0;

generic_too_big:
	return is_first_frag6(pkt_frag_hdr(in)) ? 1 : 2;
}

static verdict validate_size(struct xlation *state)
{
	unsigned int nexthop_mtu;

	if (!state->dst || pkt_is_icmp6_error(&state->in))
		return VERDICT_CONTINUE;

	nexthop_mtu = dst_mtu(state->dst);
	switch (fragment_exceeds_mtu64(&state->in, nexthop_mtu)) {
	case 0:
		return VERDICT_CONTINUE;
	case 1:
		return drop_icmp(state, JSTAT_PKT_TOO_BIG, ICMPERR_FRAG_NEEDED,
				max(1280u, nexthop_mtu + 20u));
	case 2:
		return drop(state, JSTAT_PKT_TOO_BIG);
	}

	WARN(1, "fragment_exceeds_mtu64() returned garbage.");
	return drop(state, JSTAT_UNKNOWN);
}

static verdict ttp64_alloc_skb(struct xlation *state)
{
	struct packet const *in = &state->in;
	struct sk_buff *out;
	struct skb_shared_info *shinfo;
	verdict result;

	result = predict_route64(state);
	if (result != VERDICT_CONTINUE)
		return result;
	result = validate_size(state);
	if (result != VERDICT_CONTINUE)
		goto revert;

	/*
	 * pskb_copy() is more efficient than allocating a new packet, because
	 * it shares (not copies) the original's paged data with the copy. This
	 * is great, because we don't need to modify the payload in either
	 * packet.
	 *
	 * Since the IPv4 version of the packet is going to be invariably
	 * smaller than its IPv6 counterpart, you'd think we should reserve less
	 * memory for it. But there's a problem: __pskb_copy() only allows us to
	 * shrink the headroom; not the head. If we try to shrink the head
	 * through the headroom and the v6 packet happens to have one too many
	 * extension headers, the `headroom` we'll send to __pskb_copy() will be
	 * negative, and then skb_copy_from_linear_data() will write onto the
	 * tail area without knowing it. (I'm reading the Linux 4.4 code.)
	 *
	 * We will therefore *not* attempt to allocate less.
	 */

	out = pskb_copy(in->skb, GFP_ATOMIC);
	if (!out) {
		log_debug(state, "pskb_copy() returned NULL.");
		result = drop(state, JSTAT64_PSKB_COPY);
		goto revert;
	}

	skb_cleanup_copy(out);

	/* Remove outer l3 and l4 headers from the copy. */
	skb_pull(out, pkt_hdrs_len(in));

	if (is_first_frag6(pkt_frag_hdr(in)) && pkt_is_icmp6_error(in)) {
		struct ipv6hdr *hdr = pkt_payload(in);
		struct hdr_iterator iterator = HDR_ITERATOR_INIT(hdr);
		hdr_iterator_last(&iterator);

		/* Remove inner l3 headers from the copy. */
		skb_pull(out, iterator.data - (void *)hdr);

		/* Add inner l3 headers to the copy. */
		skb_push(out, sizeof(struct iphdr));
	}

	/* Add outer l4 headers to the copy. */
	skb_push(out, pkt_l4hdr_len(in));
	/* Add outer l3 headers to the copy. */
	skb_push(out, sizeof(struct iphdr));

	skb_reset_mac_header(out);
	skb_reset_network_header(out);
	skb_set_transport_header(out, sizeof(struct iphdr));

	/* Wrap up. */
	pkt_fill(&state->out, out, L3PROTO_IPV4, pkt_l4_proto(in),
			NULL, skb_transport_header(out) + pkt_l4hdr_len(in),
			pkt_original_pkt(in));

	memset(out->cb, 0, sizeof(out->cb));
	out->mark = state->flowx.v4.flowi.flowi4_mark;
	out->protocol = htons(ETH_P_IP);

	shinfo = skb_shinfo(out);
	if (shinfo->gso_type & SKB_GSO_TCPV6) {
		shinfo->gso_type &= ~SKB_GSO_TCPV6;
		shinfo->gso_type |= SKB_GSO_TCPV4;
	}

	if (state->dst) {
		skb_dst_set(out, state->dst);
		state->dst = NULL;
	}
	return VERDICT_CONTINUE;

revert:
	if (state->dst) {
		dst_release(state->dst);
		state->dst = NULL;
	}
	return result;
}

/**
 * One-liner for creating the IPv4 header's Identification field.
 */
static void generate_ipv4_id(struct xlation const *state, struct iphdr *hdr4,
    struct frag_hdr const *hdr_frag)
{
	if (hdr_frag) {
		hdr4->id = cpu_to_be16(be32_to_cpu(hdr_frag->identification));
	} else {
		__ip_select_ident(state->jool.ns, hdr4, 1);
	}
}

static bool generate_df_flag(struct xlation const *state)
{
	struct packet const *in;
	struct packet const *out;

	/*
	 * This is the RFC logic, but it's complicated by frag_list, GRO and
	 * internal packets.
	 */

	in = &state->in;
	out = &state->out;

	if (pkt_is_inner(out)) {
		/* Unimportant. Guess: RFC logic. Meh. */
		return ntohs(pkt_ip4_hdr(out)->tot_len) > 1260;
	}
	if (skb_has_frag_list(in->skb)) {
		/* Clearly fragmented */
		return false;
	}
	if (skb_is_gso(in->skb)) {
		if (pkt_l4_proto(in) != L4PROTO_TCP) {
			/* UDP fragmented, ICMP & OTHER undefined */
			return false;
		}
		/* TCP not fragmented */
		return pkt_hdrs_len(out) + skb_shinfo(in->skb)->gso_size > 1260;
	}

	/* Not fragmented */
	return out->skb->len > 1260;
}

static __be16 xlat_frag_off(struct frag_hdr const *hdr_frag,
		struct xlation const *state)
{
	bool df;
	__u16 mf;
	__u16 frag_offset;

	if (hdr_frag) {
		df = 0;
		mf = is_mf_set_ipv6(hdr_frag);
		frag_offset = get_fragment_offset_ipv6(hdr_frag);
	} else {
		df = generate_df_flag(state);
		mf = 0;
		frag_offset = 0;
	}

	return build_ipv4_frag_off_field(df, mf, frag_offset);
}

/**
 * has_nonzero_segments_left - Returns true if @hdr6's packet has a routing
 * header, and its Segments Left field is not zero.
 *
 * @location: if the packet has nonzero segments left, the offset
 *		of the segments left field (from the start of @hdr6) will be
 *		stored here.
 */
static bool has_nonzero_segments_left(struct ipv6hdr const *hdr6,
		__u32 *location)
{
	struct ipv6_rt_hdr const *rt_hdr;
	unsigned int offset;

	rt_hdr = hdr_iterator_find(hdr6, NEXTHDR_ROUTING);
	if (!rt_hdr)
		return false;

	if (rt_hdr->segments_left == 0)
		return false;

	offset = ((void *)rt_hdr) - (void *)hdr6;
	*location = offset + offsetof(struct ipv6_rt_hdr, segments_left);
	return true;
}

/**
 * Translates @state->in's IPv6 header into @state->out's IPv4 header.
 * Only used for external IPv6 headers. (ie. not enclosed in ICMP errors.)
 * RFC 7915 sections 5.1 and 5.1.1.
 */
static verdict ttp64_ipv4_external(struct xlation *state)
{
	struct ipv6hdr const *hdr6;
	struct iphdr *hdr4;
	struct frag_hdr const *hdr_frag;
	struct flowi4 *flow4;
	__u32 nonzero_location;

	hdr6 = pkt_ip6_hdr(&state->in);

	if (hdr6->hop_limit <= 1) {
		log_debug(state, "Packet's hop limit <= 1.");
		return drop_icmp(state, JSTAT64_TTL, ICMPERR_TTL, 0);
	}
	if (has_nonzero_segments_left(hdr6, &nonzero_location)) {
		log_debug(state, "Packet's segments left field is nonzero.");
		return drop_icmp(state, JSTAT64_SEGMENTS_LEFT,
				ICMPERR_HDR_FIELD, nonzero_location);
	}

	hdr4 = pkt_ip4_hdr(&state->out);
	hdr_frag = pkt_frag_hdr(&state->in);
	flow4 = &state->flowx.v4.flowi;

	hdr4->version = 4;
	hdr4->ihl = 5;
	hdr4->tos = flow4->flowi4_tos;
	hdr4->tot_len = cpu_to_be16(state->out.skb->len);
	generate_ipv4_id(state, hdr4, hdr_frag);
	hdr4->frag_off = xlat_frag_off(hdr_frag, state);
	hdr4->ttl = hdr6->hop_limit - 1;
	hdr4->protocol = flow4->flowi4_proto;
	/* ip4_hdr->check is set later; please scroll down. */
	hdr4->saddr = flow4->saddr;
	hdr4->daddr = flow4->daddr;
	hdr4->check = 0;
	hdr4->check = ip_fast_csum(hdr4, hdr4->ihl);

	return VERDICT_CONTINUE;
}

/**
 * Same as ttp64_ipv4_external(), except only used on internal headers.
 */
static verdict ttp64_ipv4_internal(struct xlation *state)
{
	struct packet const *in = &state->in;
	struct packet *out = &state->out;
	struct ipv6hdr const *hdr6 = pkt_ip6_hdr(in);
	struct iphdr *hdr4 = pkt_ip4_hdr(out);
	struct frag_hdr const *hdr_frag = pkt_frag_hdr(in);

	hdr4->version = 4;
	hdr4->ihl = 5;
	hdr4->tos = xlat_tos(&state->jool.globals, hdr6);
	hdr4->tot_len = cpu_to_be16(get_tot_len_ipv6(in->skb) - pkt_hdrs_len(in)
			+ pkt_hdrs_len(out));
	generate_ipv4_id(state, hdr4, hdr_frag);
	hdr4->frag_off = xlat_frag_off(hdr_frag, state);
	hdr4->ttl = hdr6->hop_limit;
	hdr4->protocol = xlat_proto(hdr6);
	hdr4->saddr = state->flowx.v4.inner_src.s_addr;
	hdr4->daddr = state->flowx.v4.inner_dst.s_addr;
	hdr4->check = 0;
	hdr4->check = ip_fast_csum(hdr4, hdr4->ihl);

	return VERDICT_CONTINUE;
}

/**
 * One liner for creating the ICMPv4 header's MTU field.
 * Returns the smallest out of the three parameters.
 */
static __be16 minimum(unsigned int mtu1, unsigned int mtu2, unsigned int mtu3)
{
	return cpu_to_be16(min(mtu1, min(mtu2, mtu3)));
}

static verdict compute_mtu4(struct xlation const *state)
{
	/* Meant for unit tests. */
	static const unsigned int INFINITE = 0xffffffff;
	struct icmphdr *out_icmp;
	struct icmp6hdr const *in_icmp;
	struct net_device const *in_dev;
	struct dst_entry const *out_dst;
	unsigned int in_mtu;
	unsigned int out_mtu;

	out_icmp = pkt_icmp4_hdr(&state->out);
	in_icmp = pkt_icmp6_hdr(&state->in);
	in_dev = state->in.skb->dev;
	in_mtu = in_dev ? in_dev->mtu : INFINITE;
	out_dst = skb_dst(state->out.skb);
	out_mtu = out_dst ? dst_mtu(out_dst) : INFINITE;

	log_debug(state, "Packet MTU: %u", be32_to_cpu(in_icmp->icmp6_mtu));
	log_debug(state, "In dev MTU: %u", in_mtu);
	log_debug(state, "Out dev MTU: %u", out_mtu);

	out_icmp->un.frag.mtu = minimum(be32_to_cpu(in_icmp->icmp6_mtu) - 20,
			out_mtu,
			in_mtu - 20);
	log_debug(state, "Resulting MTU: %u", be16_to_cpu(out_icmp->un.frag.mtu));

	return VERDICT_CONTINUE;
}

/**
 * One liner for translating the ICMPv6's pointer field to ICMPv4.
 * "Pointer" is a field from "Parameter Problem" ICMP messages.
 */
static verdict icmp6_to_icmp4_param_prob_ptr(struct xlation *state)
{
	struct icmp6hdr const *icmpv6_hdr = pkt_icmp6_hdr(&state->in);
	struct icmphdr *icmpv4_hdr = pkt_icmp4_hdr(&state->out);
	__u32 icmp6_ptr = be32_to_cpu(icmpv6_hdr->icmp6_dataun.un_data32[0]);
	__u32 icmp4_ptr;

	if (icmp6_ptr < 0 || 39 < icmp6_ptr)
		goto failure;

	switch (icmp6_ptr) {
	case 0:
		icmp4_ptr = 0;
		goto success;
	case 1:
		icmp4_ptr = 1;
		goto success;
	case 2:
	case 3:
		goto failure;
	case 4:
	case 5:
		icmp4_ptr = 2;
		goto success;
	case 6:
		icmp4_ptr = 9;
		goto success;
	case 7:
		icmp4_ptr = 8;
		goto success;
	}

	if (icmp6_ptr >= 24) {
		icmp4_ptr = 16;
		goto success;
	}
	if (icmp6_ptr >= 8) {
		icmp4_ptr = 12;
		goto success;
	}

	/* The above ifs are supposed to cover all the possible values. */
	WARN(true, "Parameter problem pointer '%u' is unknown.", icmp6_ptr);
	goto failure;

success:
	icmpv4_hdr->icmp4_unused = cpu_to_be32(icmp4_ptr << 24);
	return VERDICT_CONTINUE;
failure:
	log_debug(state, "Parameter problem pointer '%u' lacks an ICMPv4 counterpart.",
			icmp6_ptr);
	return drop(state, JSTAT64_UNTRANSLATABLE_PARAM_PROB_PTR);
}

/**
 * One-liner for translating "Parameter Problem" messages from ICMPv6 to ICMPv4.
 */
static verdict icmp6_to_icmp4_param_prob(struct xlation *state)
{
	struct icmp6hdr const *icmpv6_hdr = pkt_icmp6_hdr(&state->in);
	struct icmphdr *icmpv4_hdr = pkt_icmp4_hdr(&state->out);

	switch (icmpv6_hdr->icmp6_code) {
	case ICMPV6_HDR_FIELD:
		return icmp6_to_icmp4_param_prob_ptr(state);

	case ICMPV6_UNK_NEXTHDR:
		icmpv4_hdr->icmp4_unused = 0;
		return VERDICT_CONTINUE;
	}

	/* Dead code */
	WARN(1, "ICMPv6 Parameter Problem code %u was unhandled by the switch above.",
			icmpv6_hdr->icmp6_type);
	return drop(state, JSTAT_UNKNOWN);
}

/*
 * Use this when only the ICMP header changed, so all there is to do is subtract
 * the old data from the checksum and add the new one.
 */
static void update_icmp4_csum(struct xlation const *state)
{
	struct ipv6hdr const *in_ip6 = pkt_ip6_hdr(&state->in);
	struct icmp6hdr const *in_icmp = pkt_icmp6_hdr(&state->in);
	struct icmphdr *out_icmp = pkt_icmp4_hdr(&state->out);
	struct icmp6hdr copy_hdr;
	__wsum csum, tmp;

	csum = ~csum_unfold(in_icmp->icmp6_cksum);

	/* Remove the ICMPv6 pseudo-header. */
	tmp = ~csum_unfold(csum_ipv6_magic(&in_ip6->saddr, &in_ip6->daddr,
			pkt_datagram_len(&state->in), NEXTHDR_ICMP, 0));
	csum = csum_sub(csum, tmp);

	/*
	 * Remove the ICMPv6 header.
	 * I'm working on a copy because I need to zero out its checksum.
	 * If I did that directly on the skb, I'd need to make it writable
	 * first.
	 */
	memcpy(&copy_hdr, in_icmp, sizeof(*in_icmp));
	copy_hdr.icmp6_cksum = 0;
	tmp = csum_partial(&copy_hdr, sizeof(copy_hdr), 0);
	csum = csum_sub(csum, tmp);

	/* Add the ICMPv4 header. There's no ICMPv4 pseudo-header. */
	out_icmp->checksum = 0;
	tmp = csum_partial(out_icmp, sizeof(*out_icmp), 0);
	csum = csum_add(csum, tmp);

	out_icmp->checksum = csum_fold(csum);
}

/**
 * Use this when header and payload both changed completely, so we gotta just
 * trash the old checksum and start anew.
 */
static void compute_icmp4_csum(struct packet const *out)
{
	struct icmphdr *hdr = pkt_icmp4_hdr(out);

	/*
	 * This function only gets called for ICMP error checksums, so
	 * pkt_datagram_len() is fine.
	 */
	hdr->checksum = 0;
	hdr->checksum = csum_fold(skb_checksum(out->skb,
			skb_transport_offset(out->skb),
			pkt_datagram_len(out), 0));
	out->skb->ip_summed = CHECKSUM_NONE;
}

static verdict validate_icmp6_csum(struct xlation *state)
{
	struct packet const *in = &state->in;
	struct ipv6hdr const *hdr6;
	unsigned int len;
	__sum16 csum;

	if (in->skb->ip_summed != CHECKSUM_NONE)
		return VERDICT_CONTINUE;

	hdr6 = pkt_ip6_hdr(in);
	len = pkt_datagram_len(in);
	csum = csum_ipv6_magic(&hdr6->saddr, &hdr6->daddr, len, NEXTHDR_ICMP,
			skb_checksum(in->skb, skb_transport_offset(in->skb),
					len, 0));
	if (csum != 0) {
		log_debug(state, "Checksum doesn't match.");
		return drop(state, JSTAT64_ICMP_CSUM);
	}

	return VERDICT_CONTINUE;
}

static void update_total_length(struct packet const *out)
{
	struct iphdr *hdr;
	unsigned int new_len;

	hdr = pkt_ip4_hdr(out);
	new_len = out->skb->len;

	if (be16_to_cpu(hdr->tot_len) == new_len)
		return;

	hdr->tot_len = cpu_to_be16(new_len);
	hdr->frag_off &= cpu_to_be16(~IP_DF); /* Assumes new_len <= 1260 */
	hdr->check = 0;
	hdr->check = ip_fast_csum(hdr, hdr->ihl);
}

static verdict handle_icmp4_extension(struct xlation *state)
{
	struct icmpext_args args;
	verdict result;
	struct packet *out;

	args.max_pkt_len = 576;
	args.ipl = pkt_icmp6_hdr(&state->in)->icmp6_length << 3;
	args.out_bits = 2;
	args.force_remove_ie = false;

	result = handle_icmp_extension(state, &args);
	if (result != VERDICT_CONTINUE)
		return result;

	out = &state->out;
	icmp4_length(pkt_icmp4_hdr(out)) = args.ipl;
	update_total_length(out);
	return VERDICT_CONTINUE;
}

/*
 * According to my tests, if we send an ICMP error that exceeds the MTU, Linux
 * will either drop it (if skb->local_df is false) or fragment it (if
 * skb->local_df is true).
 * Neither of these possibilities is even remotely acceptable.
 * We'll maximize delivery probability by truncating to mandatory minimum size.
 */
static verdict trim_576(struct xlation *state)
{
	struct packet *out;
	int error;

	out = &state->out;
	if (out->skb->len <= 576)
		return VERDICT_CONTINUE;

	error = pskb_trim(out->skb, 576);
	if (error) {
		log_debug(state, "pskb_trim() error: %d", error);
		return drop(state, JSTAT_ENOMEM);
	}

	update_total_length(out);
	return VERDICT_CONTINUE;
}

static verdict post_icmp4error(struct xlation *state, bool handle_extensions)
{
	verdict result;

	log_debug(state, "Translating the inner packet (6->4)...");

	result = validate_icmp6_csum(state);
	if (result != VERDICT_CONTINUE)
		return result;

	result = ttpcomm_translate_inner_packet(state, &ttp64_steps);
	if (result != VERDICT_CONTINUE)
		return result;

	if (handle_extensions) {
		result = handle_icmp4_extension(state);
		if (result != VERDICT_CONTINUE)
			return result;
	}

	result = trim_576(state);
	if (result != VERDICT_CONTINUE)
		return result;

	compute_icmp4_csum(&state->out);
	return VERDICT_CONTINUE;
}

/**
 * Translates in's icmp6 header and payload into out's icmp4 header and payload.
 * This is the core of RFC 7915 sections 5.2 and 5.3, except checksum (See
 * post_icmp4*()).
 */
static verdict ttp64_icmp(struct xlation *state)
{
	struct icmp6hdr const *icmpv6_hdr = pkt_icmp6_hdr(&state->in);
	struct icmphdr *icmpv4_hdr = pkt_icmp4_hdr(&state->out);
	verdict result;

	if (pkt_is_outer(&state->in)) {
		icmpv4_hdr->type = state->flowx.v4.flowi.fl4_icmp_type;
		icmpv4_hdr->code = state->flowx.v4.flowi.fl4_icmp_code;
	} else {
		result = xlat64_icmp_type(&icmpv4_hdr->type, &icmpv4_hdr->code,
				state);
		if (result != VERDICT_CONTINUE)
			return result;
	}
	icmpv4_hdr->checksum = icmpv6_hdr->icmp6_cksum; /* default. */

	switch (icmpv6_hdr->icmp6_type) {
	case ICMPV6_ECHO_REQUEST:
	case ICMPV6_ECHO_REPLY:
		icmpv4_hdr->un.echo.id = xlation_is_nat64(state)
				? cpu_to_be16(state->out.tuple.icmp4_id)
				: icmpv6_hdr->icmp6_identifier;
		icmpv4_hdr->un.echo.sequence = icmpv6_hdr->icmp6_sequence;
		update_icmp4_csum(state);
		return VERDICT_CONTINUE;

	case ICMPV6_DEST_UNREACH:
	case ICMPV6_TIME_EXCEED:
		icmpv4_hdr->icmp4_unused = 0;
		return post_icmp4error(state, true);

	case ICMPV6_PKT_TOOBIG:
		/*
		 * BTW, I have no idea what the RFC means by "taking into
		 * account whether or not the packet in error includes a
		 * Fragment Header"... What does the fragment header have to do
		 * with anything here?
		 */
		icmpv4_hdr->un.frag.__unused = htons(0);
		result = compute_mtu4(state);
		if (result != VERDICT_CONTINUE)
			return result;
		return post_icmp4error(state, false);

	case ICMPV6_PARAMPROB:
		result = icmp6_to_icmp4_param_prob(state);
		if (result != VERDICT_CONTINUE)
			return result;
		return post_icmp4error(state, false);
	}

	/* Dead code */
	WARN(1, "ICMPv6 type %u was unhandled by the switch above.",
			icmpv6_hdr->icmp6_type);
	return drop(state, JSTAT_UNKNOWN);
}

static __be16 get_src_port64(struct xlation *state)
{
	return pkt_is_inner(&state->out)
			? cpu_to_be16(state->out.tuple.dst.addr4.l4)
			: cpu_to_be16(state->out.tuple.src.addr4.l4);
}

static __be16 get_dst_port64(struct xlation *state)
{
	return pkt_is_inner(&state->out)
			? cpu_to_be16(state->out.tuple.src.addr4.l4)
			: cpu_to_be16(state->out.tuple.dst.addr4.l4);
}

static __wsum pseudohdr6_csum(struct ipv6hdr const *hdr)
{
	return ~csum_unfold(csum_ipv6_magic(&hdr->saddr, &hdr->daddr, 0, 0, 0));
}

static __wsum pseudohdr4_csum(struct iphdr const *hdr)
{
	return csum_tcpudp_nofold(hdr->saddr, hdr->daddr, 0, 0, 0);
}

static __sum16 update_csum_6to4(__sum16 csum16,
		struct ipv6hdr const *in_ip6, void const *in_l4_hdr, size_t in_l4_hdr_len,
		struct iphdr const *out_ip4, void const *out_l4_hdr, size_t out_l4_hdr_len)
{
	__wsum csum;

	csum = ~csum_unfold(csum16);

	/*
	 * Regarding the pseudoheaders:
	 * The length is pretty hard to obtain if there's TCP and fragmentation,
	 * and whatever it is, it's not going to change. Therefore, instead of
	 * computing it only to cancel it out with itself later, simply sum
	 * (and substract) zero.
	 * Do the same with proto since we're feeling ballsy.
	 */

	/* Remove the IPv6 crap. */
	csum = csum_sub(csum, pseudohdr6_csum(in_ip6));
	csum = csum_sub(csum, csum_partial(in_l4_hdr, in_l4_hdr_len, 0));

	/* Add the IPv4 crap. */
	csum = csum_add(csum, pseudohdr4_csum(out_ip4));
	csum = csum_add(csum, csum_partial(out_l4_hdr, out_l4_hdr_len, 0));

	return csum_fold(csum);
}

static verdict ttp64_tcp(struct xlation *state)
{
	struct packet const *in = &state->in;
	struct packet *out = &state->out;
	struct tcphdr const *tcp_in = pkt_tcp_hdr(in);
	struct tcphdr *tcp_out = pkt_tcp_hdr(out);
	struct tcphdr tcp_copy;

	/* Header */
	memcpy(tcp_out, tcp_in, pkt_l4hdr_len(in));
	if (xlation_is_nat64(state)) {
		tcp_out->source = get_src_port64(state);
		tcp_out->dest = get_dst_port64(state);
	}

	/* Header.checksum */
	if (in->skb->ip_summed != CHECKSUM_PARTIAL) {
		memcpy(&tcp_copy, tcp_in, sizeof(*tcp_in));
		tcp_copy.check = 0;

		tcp_out->check = 0;
		tcp_out->check = update_csum_6to4(tcp_in->check,
				pkt_ip6_hdr(in), &tcp_copy, sizeof(tcp_copy),
				pkt_ip4_hdr(out), tcp_out, sizeof(*tcp_out));
		out->skb->ip_summed = CHECKSUM_NONE;

	} else {
		tcp_out->check = ~tcp_v4_check(pkt_datagram_len(out),
				pkt_ip4_hdr(out)->saddr,
				pkt_ip4_hdr(out)->daddr, 0);
		partialize_skb(out->skb, offsetof(struct tcphdr, check));
	}

	return VERDICT_CONTINUE;
}

static verdict ttp64_udp(struct xlation *state)
{
	struct packet const *in = &state->in;
	struct packet *out = &state->out;
	struct udphdr const *udp_in = pkt_udp_hdr(in);
	struct udphdr *udp_out = pkt_udp_hdr(out);
	struct udphdr udp_copy;

	/* Header */
	memcpy(udp_out, udp_in, pkt_l4hdr_len(in));
	if (xlation_is_nat64(state)) {
		udp_out->source = get_src_port64(state);
		udp_out->dest = get_dst_port64(state);
	}

	/* Header.checksum */
	if (in->skb->ip_summed != CHECKSUM_PARTIAL) {
		memcpy(&udp_copy, udp_in, sizeof(*udp_in));
		udp_copy.check = 0;

		udp_out->check = 0;
		udp_out->check = update_csum_6to4(udp_in->check,
				pkt_ip6_hdr(in), &udp_copy, sizeof(udp_copy),
				pkt_ip4_hdr(out), udp_out, sizeof(*udp_out));
		if (udp_out->check == 0)
			udp_out->check = CSUM_MANGLED_0;
		out->skb->ip_summed = CHECKSUM_NONE;

	} else {
		udp_out->check = ~udp_v4_check(pkt_datagram_len(out),
				pkt_ip4_hdr(out)->saddr,
				pkt_ip4_hdr(out)->daddr, 0);
		partialize_skb(out->skb, offsetof(struct udphdr, check));
	}

	return VERDICT_CONTINUE;
}

const struct translation_steps ttp64_steps = {
	.skb_alloc = ttp64_alloc_skb,
	.xlat_outer_l3 = ttp64_ipv4_external,
	.xlat_inner_l3 = ttp64_ipv4_internal,
	.xlat_tcp = ttp64_tcp,
	.xlat_udp = ttp64_udp,
	.xlat_icmp = ttp64_icmp,
};
