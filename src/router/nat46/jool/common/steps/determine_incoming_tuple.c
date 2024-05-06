#include "mod/common/steps/determine_incoming_tuple.h"

#include "mod/common/icmp_wrapper.h"
#include "mod/common/ipv6_hdr_iterator.h"
#include "mod/common/log.h"
#include "mod/common/stats.h"

/*
 * There are several points in this module where the RFC says "drop the packet",
 * but Jool "accepts" it instead.
 * This is because of Netfilter idiosyncrasies. The RFC probably assumes a NAT64
 * wouldn't be positioned exactly where Netfilter hooks run.
 * Every one of these RFC mismatches should be commented.
 */

/**
 * ipv4_extract_l4_hdr - Assumes that @hdr_ipv4 is part of a packet, and returns
 * a pointer to the chunk of data after it.
 *
 * Skips IPv4 options if any.
 */
static void const *ipv4_extract_l4_hdr(struct iphdr const *hdr_ipv4)
{
	return ((void const *) hdr_ipv4) + (hdr_ipv4->ihl << 2);
}

/**
 * unknown_inner_proto - whenever this function is called, the RFC says the
 * packet should be dropped, but Jool is accepting it instead.
 *
 * I made it into a function so I wouldn't have to replicate the rationale:
 *
 * If the packet is an ICMP error that contains a packet of unknown transport
 * protocol, we couldn't have possibly translated the packet that caused the
 * error.
 * Therefore, the original packet came from this host... or it's just crafted
 * garbage.
 * Either way, Linux should be the one who decides the fate of the ICMP error.
 */
static verdict unknown_inner_proto(struct xlation *state, __u8 proto)
{
	log_debug(state, "Packet's inner packet is not UDP, TCP or ICMP (%u).",
			proto);
	return untranslatable(state, JSTAT_UNKNOWN_PROTO_INNER);
}

static void ipv4_udp(struct xlation *state)
{
	struct packet *pkt = &state->in;
	struct tuple *tuple4 = &pkt->tuple;

	tuple4->src.addr4.l3.s_addr = pkt_ip4_hdr(pkt)->saddr;
	tuple4->src.addr4.l4 = be16_to_cpu(pkt_udp_hdr(pkt)->source);
	tuple4->dst.addr4.l3.s_addr = pkt_ip4_hdr(pkt)->daddr;
	tuple4->dst.addr4.l4 = be16_to_cpu(pkt_udp_hdr(pkt)->dest);
	tuple4->l3_proto = L3PROTO_IPV4;
	tuple4->l4_proto = L4PROTO_UDP;
}

static void ipv4_tcp(struct xlation *state)
{
	struct packet *pkt = &state->in;
	struct tuple *tuple4 = &pkt->tuple;

	tuple4->src.addr4.l3.s_addr = pkt_ip4_hdr(pkt)->saddr;
	tuple4->src.addr4.l4 = be16_to_cpu(pkt_tcp_hdr(pkt)->source);
	tuple4->dst.addr4.l3.s_addr = pkt_ip4_hdr(pkt)->daddr;
	tuple4->dst.addr4.l4 = be16_to_cpu(pkt_tcp_hdr(pkt)->dest);
	tuple4->l3_proto = L3PROTO_IPV4;
	tuple4->l4_proto = L4PROTO_TCP;
}

static verdict ipv4_icmp_info(struct xlation *state)
{
	struct packet *pkt = &state->in;
	struct tuple *tuple4 = &pkt->tuple;

	tuple4->src.addr4.l3.s_addr = pkt_ip4_hdr(pkt)->saddr;
	tuple4->src.addr4.l4 = be16_to_cpu(pkt_icmp4_hdr(pkt)->un.echo.id);
	tuple4->dst.addr4.l3.s_addr = pkt_ip4_hdr(pkt)->daddr;
	tuple4->dst.addr4.l4 = tuple4->src.addr4.l4;
	tuple4->l3_proto = L3PROTO_IPV4;
	tuple4->l4_proto = L4PROTO_ICMP;

	return VERDICT_CONTINUE;
}

static verdict ipv4_icmp_err(struct xlation *state)
{
	struct tuple *tuple4 = &state->in.tuple;
	union {
		struct iphdr const *ip4;
		struct udphdr const *udp;
		struct tcphdr const *tcp;
		struct icmphdr const *icmp;
	} inner;

	inner.ip4 = (struct iphdr *)(pkt_icmp4_hdr(&state->in) + 1);
	tuple4->src.addr4.l3.s_addr = inner.ip4->daddr;
	tuple4->dst.addr4.l3.s_addr = inner.ip4->saddr;

	switch (inner.ip4->protocol) {
	case IPPROTO_UDP:
		inner.udp = ipv4_extract_l4_hdr(inner.ip4);
		tuple4->src.addr4.l4 = be16_to_cpu(inner.udp->dest);
		tuple4->dst.addr4.l4 = be16_to_cpu(inner.udp->source);
		tuple4->l4_proto = L4PROTO_UDP;
		break;

	case IPPROTO_TCP:
		inner.tcp = ipv4_extract_l4_hdr(inner.ip4);
		tuple4->src.addr4.l4 = be16_to_cpu(inner.tcp->dest);
		tuple4->dst.addr4.l4 = be16_to_cpu(inner.tcp->source);
		tuple4->l4_proto = L4PROTO_TCP;
		break;

	case IPPROTO_ICMP:
		inner.icmp = ipv4_extract_l4_hdr(inner.ip4);

		if (is_icmp4_error(inner.icmp->type)) {
			log_debug(state, "Bogus pkt: ICMP error inside ICMP error.");
			return drop(state, JSTAT_DOUBLE_ICMP4_ERROR);
		}

		tuple4->src.addr4.l4 = be16_to_cpu(inner.icmp->un.echo.id);
		tuple4->dst.addr4.l4 = tuple4->src.addr4.l4;
		tuple4->l4_proto = L4PROTO_ICMP;
		break;

	default:
		return unknown_inner_proto(state, inner.ip4->protocol);
	}

	tuple4->l3_proto = L3PROTO_IPV4;

	return VERDICT_CONTINUE;
}

static verdict ipv4_icmp(struct xlation *state)
{
	__u8 type = pkt_icmp4_hdr(&state->in)->type;

	if (is_icmp4_info(type))
		return ipv4_icmp_info(state);
	if (is_icmp4_error(type))
		return ipv4_icmp_err(state);

	log_debug(state, "Unknown ICMPv4 type: %u", type);
	/*
	 * Hope the kernel has something to do with the packet.
	 * Neighbor discovery not likely an issue, but see ipv6_icmp() anyway.
	 */
	return untranslatable(state, JSTAT_UNKNOWN_ICMP4_TYPE);
}

static void ipv6_udp(struct xlation *state)
{
	struct packet *pkt = &state->in;
	struct tuple *tuple6 = &pkt->tuple;

	tuple6->src.addr6.l3 = pkt_ip6_hdr(pkt)->saddr;
	tuple6->src.addr6.l4 = be16_to_cpu(pkt_udp_hdr(pkt)->source);
	tuple6->dst.addr6.l3 = pkt_ip6_hdr(pkt)->daddr;
	tuple6->dst.addr6.l4 = be16_to_cpu(pkt_udp_hdr(pkt)->dest);
	tuple6->l3_proto = L3PROTO_IPV6;
	tuple6->l4_proto = L4PROTO_UDP;
}

static void ipv6_tcp(struct xlation *state)
{
	struct packet *pkt = &state->in;
	struct tuple *tuple6 = &pkt->tuple;

	tuple6->src.addr6.l3 = pkt_ip6_hdr(pkt)->saddr;
	tuple6->src.addr6.l4 = be16_to_cpu(pkt_tcp_hdr(pkt)->source);
	tuple6->dst.addr6.l3 = pkt_ip6_hdr(pkt)->daddr;
	tuple6->dst.addr6.l4 = be16_to_cpu(pkt_tcp_hdr(pkt)->dest);
	tuple6->l3_proto = L3PROTO_IPV6;
	tuple6->l4_proto = L4PROTO_TCP;
}

static verdict ipv6_icmp_info(struct xlation *state)
{
	struct packet *pkt = &state->in;
	struct tuple *tuple6 = &pkt->tuple;
	__u16 id = be16_to_cpu(pkt_icmp6_hdr(pkt)->icmp6_identifier);

	tuple6->src.addr6.l3 = pkt_ip6_hdr(pkt)->saddr;
	tuple6->src.addr6.l4 = id;
	tuple6->dst.addr6.l3 = pkt_ip6_hdr(pkt)->daddr;
	tuple6->dst.addr6.l4 = id;
	tuple6->l3_proto = L3PROTO_IPV6;
	tuple6->l4_proto = L4PROTO_ICMP;

	return VERDICT_CONTINUE;
}

static verdict ipv6_icmp_err(struct xlation *state)
{
	struct packet *pkt = &state->in;
	struct tuple *tuple6 = &pkt->tuple;
	struct hdr_iterator iterator;
	union {
		struct ipv6hdr const *ip6;
		struct udphdr const *udp;
		struct tcphdr const *tcp;
		struct icmp6hdr const *icmp;
	} inner;
	__u16 id;

	inner.ip6 = (struct ipv6hdr *)(pkt_icmp6_hdr(pkt) + 1);
	tuple6->src.addr6.l3 = inner.ip6->daddr;
	tuple6->dst.addr6.l3 = inner.ip6->saddr;

	hdr_iterator_init(&iterator, inner.ip6);
	hdr_iterator_last(&iterator);

	switch (iterator.hdr_type) {
	case NEXTHDR_UDP:
		inner.udp = iterator.data;
		tuple6->src.addr6.l4 = be16_to_cpu(inner.udp->dest);
		tuple6->dst.addr6.l4 = be16_to_cpu(inner.udp->source);
		tuple6->l4_proto = L4PROTO_UDP;
		break;

	case NEXTHDR_TCP:
		inner.tcp = iterator.data;
		tuple6->src.addr6.l4 = be16_to_cpu(inner.tcp->dest);
		tuple6->dst.addr6.l4 = be16_to_cpu(inner.tcp->source);
		tuple6->l4_proto = L4PROTO_TCP;
		break;

	case NEXTHDR_ICMP:
		inner.icmp = iterator.data;

		if (is_icmp6_error(inner.icmp->icmp6_type)) {
			log_debug(state, "Bogus pkt: ICMP error inside ICMP error.");
			return drop(state, JSTAT_DOUBLE_ICMP6_ERROR);
		}

		id = be16_to_cpu(inner.icmp->icmp6_identifier);
		tuple6->src.addr6.l4 = id;
		tuple6->dst.addr6.l4 = id;
		tuple6->l4_proto = L4PROTO_ICMP;
		break;

	default:
		return unknown_inner_proto(state, iterator.hdr_type);
	}

	tuple6->l3_proto = L3PROTO_IPV6;

	return VERDICT_CONTINUE;
}

static verdict ipv6_icmp(struct xlation *state)
{
	__u8 type = pkt_icmp6_hdr(&state->in)->icmp6_type;

	if (is_icmp6_info(type))
		return ipv6_icmp_info(state);
	if (is_icmp6_error(type))
		return ipv6_icmp_err(state);

	log_debug(state, "Unknown ICMPv6 type: %u.", type);
	/*
	 * Netfilter Jool returns ACCEPT instead of DROP because the neighbor
	 * discovery code happens after Jool, apparently (even though it's
	 * layer 2 man, wtf).
	 * This message, which is likely single-hop, might actually be intended
	 * for the kernel.
	 */
	return untranslatable(state, JSTAT_UNKNOWN_ICMP6_TYPE);
}

/**
 * Extracts relevant data from "skb" and stores it in the "tuple" tuple.
 *
 * @param skb packet the data will be extracted from.
 * @param tuple this function will populate this value using "skb"'s contents.
 * @return whether packet processing should continue.
 */
verdict determine_in_tuple(struct xlation *state)
{
	verdict result = VERDICT_CONTINUE;

	log_debug(state, "Step 1: Determining the Incoming Tuple");

	switch (pkt_l3_proto(&state->in)) {
	case L3PROTO_IPV4:
		switch (pkt_l4_proto(&state->in)) {
		case L4PROTO_UDP:
			ipv4_udp(state);
			break;
		case L4PROTO_TCP:
			ipv4_tcp(state);
			break;
		case L4PROTO_ICMP:
			result = ipv4_icmp(state);
			break;
		case L4PROTO_OTHER:
			goto unknown_proto;
		}
		break;

	case L3PROTO_IPV6:
		switch (pkt_l4_proto(&state->in)) {
		case L4PROTO_UDP:
			ipv6_udp(state);
			break;
		case L4PROTO_TCP:
			ipv6_tcp(state);
			break;
		case L4PROTO_ICMP:
			result = ipv6_icmp(state);
			break;
		case L4PROTO_OTHER:
			goto unknown_proto;
		}
		break;
	}

	if (result == VERDICT_CONTINUE)
		log_tuple(state, &state->in.tuple);
	log_debug(state, "Done step 1.");
	return result;

unknown_proto:
	log_debug(state, "NAT64 doesn't support unknown transport protocols.");
	return untranslatable_icmp(state, JSTAT_UNKNOWN_L4_PROTO,
			ICMPERR_PROTO_UNREACHABLE, 0);
}
