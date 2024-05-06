#include "mod/common/icmp_wrapper.h"

#include <linux/icmpv6.h>
#include <net/icmp.h>
#include "common/types.h"
#include "mod/common/log.h"

static int route4_input(struct xlator *jool, struct sk_buff *skb)
{
	struct iphdr *hdr;
	int error;

	if (!skb->dev) {
		log_err("skb lacks an incoming device.");
		return -EINVAL;
	}

	hdr = ip_hdr(skb);
	error = ip_route_input(skb, hdr->daddr, hdr->saddr, hdr->tos, skb->dev);
	if (error)
		__log_debug(jool, "ip_route_input failed: %d", error);

	return error;
}

static char *icmp_error_to_string(icmp_error_code error)
{
	switch (error) {
	case ICMPERR_NONE:
		return "ICMPERR_NONE";
	case ICMPERR_ADDR_UNREACHABLE:
		return "ICMPERR_ADDR_UNREACHABLE";
	case ICMPERR_PORT_UNREACHABLE:
		return "ICMPERR_PORT_UNREACHABLE";
	case ICMPERR_PROTO_UNREACHABLE:
		return "ICMPERR_PROTO_UNREACHABLE";
	case ICMPERR_TTL:
		return "ICMPERR_TTL";
	case ICMPERR_FRAG_NEEDED:
		return "ICMPERR_FRAG_NEEDED";
	case ICMPERR_HDR_FIELD:
		return "ICMPERR_HDR_FIELD";
	case ICMPERR_SRC_ROUTE:
		return "ICMPERR_SRC_ROUTE";
	case ICMPERR_FILTER:
		return "ICMPERR_FILTER";
	}

	return "Unknown";
}

bool icmp64_send4(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info)
{
	int type, code;

	if (unlikely(!skb) || !skb->dev)
		return false;

	/*
	 * I don't know why the kernel needs this nonsense,
	 * but it's not my fault.
	 */
	if (route4_input(jool, skb))
		return false;

	switch (error) {
	case ICMPERR_ADDR_UNREACHABLE:
		type = ICMP_DEST_UNREACH;
		code = ICMP_HOST_UNREACH;
		break;
	case ICMPERR_PORT_UNREACHABLE:
		type = ICMP_DEST_UNREACH;
		code = ICMP_PORT_UNREACH;
		break;
	case ICMPERR_PROTO_UNREACHABLE:
		type = ICMP_DEST_UNREACH;
		code = ICMP_PROT_UNREACH;
		break;
	case ICMPERR_TTL:
		type = ICMP_TIME_EXCEEDED;
		code = ICMP_EXC_TTL;
		break;
	case ICMPERR_FRAG_NEEDED:
		type = ICMP_DEST_UNREACH;
		code = ICMP_FRAG_NEEDED;
		break;
	case ICMPERR_FILTER:
		type = ICMP_DEST_UNREACH;
		code = ICMP_PKT_FILTERED;
		break;
	case ICMPERR_SRC_ROUTE:
		type = ICMP_DEST_UNREACH;
		code = ICMP_SR_FAILED;
		break;
	default:
		return false; /* Not supported or needed. */
	}

	__log_debug(jool, "Sending ICMPv4 error: %s, type: %d, code: %d, rest: %u.",
			icmp_error_to_string(error), type, code, info);
	icmp_send(skb, type, code, cpu_to_be32(info));
	return true;
}

bool icmp64_send6(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info)
{
	int type, code;

	if (unlikely(!skb) || !skb->dev)
		return false;

	switch (error) {
	case ICMPERR_ADDR_UNREACHABLE:
		type = ICMPV6_DEST_UNREACH;
		code = ICMPV6_ADDR_UNREACH;
		break;
	case ICMPERR_PORT_UNREACHABLE:
	case ICMPERR_PROTO_UNREACHABLE:
		/* See RFC6146, determine incoming tuple step. */
		type = ICMPV6_DEST_UNREACH;
		code = ICMPV6_PORT_UNREACH;
		break;
	case ICMPERR_TTL:
		type = ICMPV6_TIME_EXCEED;
		code = ICMPV6_EXC_HOPLIMIT;
		break;
	case ICMPERR_FILTER:
		type = ICMPV6_DEST_UNREACH;
		code = ICMPV6_ADM_PROHIBITED;
		break;
	case ICMPERR_HDR_FIELD:
		type = ICMPV6_PARAMPROB;
		code = ICMPV6_HDR_FIELD;
		break;
	case ICMPERR_FRAG_NEEDED:
		type = ICMPV6_PKT_TOOBIG;
		code = 0; /* No code. */
		break;
	default:
		return false; /* Not supported or needed. */
	}

	__log_debug(jool, "Sending ICMPv6 error: %s, type: %d, code: %d, rest: %u",
			icmp_error_to_string(error), type, code, info);
	icmpv6_send(skb, type, code, info);
	return true;
}

bool icmp64_send(struct xlator *jool, struct sk_buff *skb,
		icmp_error_code error, __u32 info)
{
	if (unlikely(!skb))
		return false;

	switch (ntohs(skb->protocol)) {
	case ETH_P_IP:
		return icmp64_send4(jool, skb, error, info);
	case ETH_P_IPV6:
		return icmp64_send6(jool, skb, error, info);
	}

	return false;
}
