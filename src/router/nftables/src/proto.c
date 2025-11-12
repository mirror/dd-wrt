/*
 * Protocol header and type definitions and related functions.
 *
 * Copyright (c) 2014 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <nft.h>

#include <stddef.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>

#include <expression.h>
#include <headers.h>
#include <proto.h>
#include <gmputil.h>
#include <utils.h>

const char *proto_base_names[] = {
	[PROTO_BASE_INVALID]		= "invalid",
	[PROTO_BASE_LL_HDR]		= "link layer",
	[PROTO_BASE_NETWORK_HDR]	= "network layer",
	[PROTO_BASE_TRANSPORT_HDR]	= "transport layer",
	[PROTO_BASE_INNER_HDR]		= "payload data",
};

const char *proto_base_tokens[] = {
	[PROTO_BASE_INVALID]		= "invalid",
	[PROTO_BASE_LL_HDR]		= "ll",
	[PROTO_BASE_NETWORK_HDR]	= "nh",
	[PROTO_BASE_TRANSPORT_HDR]	= "th",
	[PROTO_BASE_INNER_HDR]		= "ih",
};

const struct proto_hdr_template proto_unknown_template =
	PROTO_HDR_TEMPLATE("unknown", &invalid_type, BYTEORDER_INVALID, 0, 0);

const struct proto_desc proto_unknown = {
	.name		= "unknown",
	.base		= PROTO_BASE_INVALID,
};

/**
 * proto_find_upper - find higher layer protocol description by protocol value
 * 		      linking it to the lower layer protocol
 *
 * @base:	lower layer protocol description
 * @num:	protocol value
 */
const struct proto_desc *
proto_find_upper(const struct proto_desc *base, unsigned int num)
{
	unsigned int i;

	for (i = 0; i < array_size(base->protocols); i++) {
		if (!base->protocols[i].desc)
			break;
		if (base->protocols[i].num == num)
			return base->protocols[i].desc;
	}
	return NULL;
}

/**
 * proto_find_num - return protocol number linking two protocols together
 *
 * @base:	lower layer protocol description
 * @desc:	upper layer protocol description
 */
int proto_find_num(const struct proto_desc *base,
		   const struct proto_desc *desc)
{
	unsigned int i;

	for (i = 0; i < array_size(base->protocols); i++) {
		if (!base->protocols[i].desc)
			break;
		if (base->protocols[i].desc == desc)
			return base->protocols[i].num;
	}
	return -1;
}

static const struct proto_desc *inner_protocols[] = {
	&proto_vxlan,
	&proto_geneve,
	&proto_gre,
	&proto_gretap,
};

const struct proto_desc *proto_find_inner(uint32_t type, uint32_t hdrsize,
					  uint32_t flags)
{
	const struct proto_desc *desc;
	unsigned int i;

	for (i = 0; i < array_size(inner_protocols); i++) {
		desc = inner_protocols[i];
		if (desc->inner.type == type &&
		    desc->inner.hdrsize == hdrsize &&
		    desc->inner.flags == flags)
			return inner_protocols[i];
	}

	return &proto_unknown;
}

static const struct dev_proto_desc dev_proto_desc[] = {
	DEV_PROTO_DESC(ARPHRD_ETHER, &proto_eth),
};

/**
 * proto_dev_type - return arphrd type linking a device and a protocol together
 *
 * @desc:	the protocol description
 * @res:	pointer to result
 */
int proto_dev_type(const struct proto_desc *desc, uint16_t *res)
{
	const struct proto_desc *base;
	unsigned int i, j;

	for (i = 0; i < array_size(dev_proto_desc); i++) {
		base = dev_proto_desc[i].desc;
		if (base == desc) {
			*res = dev_proto_desc[i].type;
			return 0;
		}
		for (j = 0; j < array_size(base->protocols); j++) {
			if (!base->protocols[j].desc)
				break;
			if (base->protocols[j].desc == desc) {
				*res = dev_proto_desc[i].type;
				return 0;
			}
		}
	}
	return -1;
}

/**
 * proto_dev_desc - return protocol description for an arphrd type
 *
 * @type:	the arphrd type
 */
const struct proto_desc *proto_dev_desc(uint16_t type)
{
	unsigned int i;

	for (i = 0; i < array_size(dev_proto_desc); i++) {
		if (dev_proto_desc[i].type == type)
			return dev_proto_desc[i].desc;
	}
	return NULL;
}

const struct hook_proto_desc hook_proto_desc[] = {
	[NFPROTO_BRIDGE]	= HOOK_PROTO_DESC(PROTO_BASE_LL_HDR,	  &proto_eth),
	[NFPROTO_NETDEV]	= HOOK_PROTO_DESC(PROTO_BASE_LL_HDR,	  &proto_netdev),
	[NFPROTO_INET]		= HOOK_PROTO_DESC(PROTO_BASE_LL_HDR,	  &proto_inet),
	[NFPROTO_IPV4]		= HOOK_PROTO_DESC(PROTO_BASE_NETWORK_HDR, &proto_ip),
	[NFPROTO_IPV6]		= HOOK_PROTO_DESC(PROTO_BASE_NETWORK_HDR, &proto_ip6),
	[NFPROTO_ARP]		= HOOK_PROTO_DESC(PROTO_BASE_NETWORK_HDR, &proto_arp),
};

static void proto_ctx_debug(const struct proto_ctx *ctx, enum proto_bases base,
			    unsigned int debug_mask)
{
	unsigned int i;

	if (!(debug_mask & NFT_DEBUG_PROTO_CTX))
		return;

	if (base == PROTO_BASE_LL_HDR && ctx->stacked_ll_count) {
		pr_debug(" saved ll headers:");
		for (i = 0; i < ctx->stacked_ll_count; i++)
			pr_debug(" %s", ctx->stacked_ll[i]->name);
	}

	pr_debug("update %s protocol context%s:\n",
		 proto_base_names[base], ctx->inner ? " (inner)" : "");

	for (i = PROTO_BASE_LL_HDR; i <= PROTO_BASE_MAX; i++) {
		pr_debug(" %-20s: %s",
			 proto_base_names[i],
			 ctx->protocol[i].desc ? ctx->protocol[i].desc->name :
						 "none");
		if (i == base)
			pr_debug(" <-");
		pr_debug("\n");
	}
	pr_debug("\n");
}

/**
 * proto_ctx_init - initialize protocol context for a given hook family
 *
 * @ctx:	protocol context
 * @family:	hook family
 * @debug_mask:	display debugging information
 */
void proto_ctx_init(struct proto_ctx *ctx, unsigned int family,
		    unsigned int debug_mask, bool inner)
{
	const struct hook_proto_desc *h = &hook_proto_desc[family];

	memset(ctx, 0, sizeof(*ctx));
	ctx->family = family;
	ctx->protocol[h->base].desc = h->desc;
	ctx->debug_mask = debug_mask;
	ctx->inner = inner;

	proto_ctx_debug(ctx, h->base, debug_mask);
}

/**
 * proto_ctx_update: update protocol context for given protocol base
 *
 * @ctx:	protocol context
 * @base:	protocol base
 * @loc:	location of the relational expression definiting the context
 * @desc:	protocol description for the given layer
 */
void proto_ctx_update(struct proto_ctx *ctx, enum proto_bases base,
		      const struct location *loc,
		      const struct proto_desc *desc)
{
	bool found = false;
	unsigned int i;

	switch (base) {
	case PROTO_BASE_LL_HDR:
	case PROTO_BASE_NETWORK_HDR:
		break;
	case PROTO_BASE_TRANSPORT_HDR:
		if (ctx->protocol[base].num_protos >= PROTO_CTX_NUM_PROTOS)
			break;

		for (i = 0; i < ctx->protocol[base].num_protos; i++) {
			if (ctx->protocol[base].protos[i].desc == desc) {
				found = true;
				break;
			}
		}
		if (!found) {
			i = ctx->protocol[base].num_protos++;
			ctx->protocol[base].protos[i].desc = desc;
			ctx->protocol[base].protos[i].location = *loc;
		}
		break;
	case PROTO_BASE_INNER_HDR:
		break;
	default:
		BUG("unknown protocol base %d", base);
	}

	ctx->protocol[base].location	= *loc;
	ctx->protocol[base].desc	= desc;

	proto_ctx_debug(ctx, base, ctx->debug_mask);
}

bool proto_ctx_is_ambiguous(struct proto_ctx *ctx, enum proto_bases base)
{
	return ctx->protocol[base].num_protos > 1;
}

const struct proto_desc *proto_ctx_find_conflict(struct proto_ctx *ctx,
						 enum proto_bases base,
						 const struct proto_desc *desc)
{
	unsigned int i;

	switch (base) {
	case PROTO_BASE_LL_HDR:
	case PROTO_BASE_NETWORK_HDR:
		if (desc != ctx->protocol[base].desc)
			return ctx->protocol[base].desc;
		break;
	case PROTO_BASE_TRANSPORT_HDR:
		for (i = 0; i < ctx->protocol[base].num_protos; i++) {
			if (desc != ctx->protocol[base].protos[i].desc)
				return ctx->protocol[base].protos[i].desc;
		}
		break;
	default:
		BUG("unknown protocol base %d", base);
	}

	return NULL;
}

#define HDR_TEMPLATE(__name, __dtype, __type, __member)			\
	PROTO_HDR_TEMPLATE(__name, __dtype,				\
			   BYTEORDER_BIG_ENDIAN,			\
			   offsetof(__type, __member) * 8,		\
			   field_sizeof(__type, __member) * 8)

#define HDR_FIELD(__name, __struct, __member)				\
	HDR_TEMPLATE(__name, &integer_type, __struct, __member)
#define HDR_HEX_FIELD(__name, __struct, __member)				\
	HDR_TEMPLATE(__name, &xinteger_type, __struct, __member)
#define HDR_BITFIELD(__name, __dtype,  __offset, __len)			\
	PROTO_HDR_TEMPLATE(__name, __dtype, BYTEORDER_BIG_ENDIAN,	\
			   __offset, __len)
#define HDR_TYPE(__name, __dtype, __struct, __member)			\
	HDR_TEMPLATE(__name, __dtype, __struct, __member)

#define INET_PROTOCOL(__name, __struct, __member)			\
	HDR_TYPE(__name, &inet_protocol_type, __struct, __member)
#define INET_SERVICE(__name, __struct, __member)			\
	HDR_TYPE(__name, &inet_service_type, __struct, __member)

/*
 * AH
 */

#define AHHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct ip_auth_hdr, __member)

const struct proto_desc proto_ah = {
	.name		= "ah",
	.id		= PROTO_DESC_AH,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.protocol_key	= AHHDR_NEXTHDR,
	.protocols	= {
		PROTO_LINK(IPPROTO_ESP,		&proto_esp),
		PROTO_LINK(IPPROTO_AH,		&proto_ah),
		PROTO_LINK(IPPROTO_COMP,	&proto_comp),
		PROTO_LINK(IPPROTO_UDP,		&proto_udp),
		PROTO_LINK(IPPROTO_UDPLITE,	&proto_udplite),
		PROTO_LINK(IPPROTO_TCP,		&proto_tcp),
		PROTO_LINK(IPPROTO_DCCP,	&proto_dccp),
		PROTO_LINK(IPPROTO_SCTP,	&proto_sctp),
	},
	.templates	= {
		[AHHDR_NEXTHDR]		= INET_PROTOCOL("nexthdr", struct ip_auth_hdr, nexthdr),
		[AHHDR_HDRLENGTH]	= AHHDR_FIELD("hdrlength", hdrlen),
		[AHHDR_RESERVED]	= AHHDR_FIELD("reserved", reserved),
		[AHHDR_SPI]		= AHHDR_FIELD("spi", spi),
		[AHHDR_SEQUENCE]	= AHHDR_FIELD("sequence", seq_no),
	},
	.format		= {
		.order	= {
			AHHDR_SPI, AHHDR_HDRLENGTH, AHHDR_NEXTHDR,
		},
		.filter	= (1 << AHHDR_RESERVED) | (1 << AHHDR_SEQUENCE)
	},
};

/*
 * ESP
 */

#define ESPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct ip_esp_hdr, __member)

const struct proto_desc proto_esp = {
	.name		= "esp",
	.id		= PROTO_DESC_ESP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.templates	= {
		[ESPHDR_SPI]		= ESPHDR_FIELD("spi", spi),
		[ESPHDR_SEQUENCE]	= ESPHDR_FIELD("sequence", seq_no),
	},
};

/*
 * IPCOMP
 */

#define COMPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct ip_comp_hdr, __member)

const struct proto_desc proto_comp = {
	.name		= "comp",
	.id		= PROTO_DESC_COMP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.protocol_key	= COMPHDR_NEXTHDR,
	.protocols	= {
		PROTO_LINK(IPPROTO_ESP,		&proto_esp),
		PROTO_LINK(IPPROTO_AH,		&proto_ah),
		PROTO_LINK(IPPROTO_COMP,	&proto_comp),
		PROTO_LINK(IPPROTO_UDP,		&proto_udp),
		PROTO_LINK(IPPROTO_UDPLITE,	&proto_udplite),
		PROTO_LINK(IPPROTO_TCP,		&proto_tcp),
		PROTO_LINK(IPPROTO_DCCP,	&proto_dccp),
		PROTO_LINK(IPPROTO_SCTP,	&proto_sctp),
	},
	.templates	= {
		[COMPHDR_NEXTHDR]	= INET_PROTOCOL("nexthdr", struct ip_comp_hdr, nexthdr),
		[COMPHDR_FLAGS]		= HDR_TEMPLATE("flags", &bitmask_type, struct ip_comp_hdr, flags),
		[COMPHDR_CPI]		= COMPHDR_FIELD("cpi", cpi),
	},
};

/*
 * ICMP
 */

#include <netinet/ip_icmp.h>

static const struct symbol_table icmp_type_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("echo-reply",			ICMP_ECHOREPLY),
		SYMBOL("destination-unreachable",	ICMP_DEST_UNREACH),
		SYMBOL("source-quench",			ICMP_SOURCE_QUENCH),
		SYMBOL("redirect",			ICMP_REDIRECT),
		SYMBOL("echo-request",			ICMP_ECHO),
		SYMBOL("router-advertisement",		ICMP_ROUTERADVERT),
		SYMBOL("router-solicitation",		ICMP_ROUTERSOLICIT),
		SYMBOL("time-exceeded",			ICMP_TIME_EXCEEDED),
		SYMBOL("parameter-problem",		ICMP_PARAMETERPROB),
		SYMBOL("timestamp-request",		ICMP_TIMESTAMP),
		SYMBOL("timestamp-reply",		ICMP_TIMESTAMPREPLY),
		SYMBOL("info-request",			ICMP_INFO_REQUEST),
		SYMBOL("info-reply",			ICMP_INFO_REPLY),
		SYMBOL("address-mask-request",		ICMP_ADDRESS),
		SYMBOL("address-mask-reply",		ICMP_ADDRESSREPLY),
		SYMBOL_LIST_END
	},
};

const struct datatype icmp_type_type = {
	.type		= TYPE_ICMP_TYPE,
	.name		= "icmp_type",
	.desc		= "ICMP type",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= BITS_PER_BYTE,
	.basetype	= &integer_type,
	.sym_tbl	= &icmp_type_tbl,
};

#define ICMP46HDR_FIELD(__token, __dtype, __struct, __member, __dep)		\
	{									\
		.token		= (__token),					\
		.dtype		= &__dtype,					\
		.byteorder	= BYTEORDER_BIG_ENDIAN,				\
		.offset		= offsetof(__struct, __member) * 8,		\
		.len		= field_sizeof(__struct, __member) * 8,		\
		.icmp_dep	= (__dep),					\
	}

#define ICMPHDR_FIELD(__token, __member, __dep) \
	ICMP46HDR_FIELD(__token, integer_type, struct icmphdr, __member, __dep)

#define ICMPHDR_TYPE(__name, __type, __member) \
	HDR_TYPE(__name,  __type, struct icmphdr, __member)

const struct proto_desc proto_icmp = {
	.name		= "icmp",
	.id		= PROTO_DESC_ICMP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.protocol_key	= ICMPHDR_TYPE,
	.checksum_key	= ICMPHDR_CHECKSUM,
	.checksum_type  = NFT_PAYLOAD_CSUM_INET,
	.templates	= {
		[ICMPHDR_TYPE]		= ICMPHDR_TYPE("type", &icmp_type_type, type),
		[ICMPHDR_CODE]		= ICMPHDR_TYPE("code", &icmp_code_type, code),
		[ICMPHDR_CHECKSUM]	= ICMPHDR_FIELD("checksum", checksum, PROTO_ICMP_ANY),
		[ICMPHDR_ID]		= ICMPHDR_FIELD("id", un.echo.id, PROTO_ICMP_ECHO),
		[ICMPHDR_SEQ]		= ICMPHDR_FIELD("sequence", un.echo.sequence, PROTO_ICMP_ECHO),
		[ICMPHDR_GATEWAY]	= ICMPHDR_FIELD("gateway", un.gateway, PROTO_ICMP_ADDRESS),
		[ICMPHDR_MTU]		= ICMPHDR_FIELD("mtu", un.frag.mtu, PROTO_ICMP_MTU),
	},
};

/*
 * IGMP
 */

#include <netinet/igmp.h>

#ifndef IGMP_V3_MEMBERSHIP_REPORT
#define IGMP_V3_MEMBERSHIP_REPORT	0x22
#endif

static const struct symbol_table igmp_type_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("membership-query",		IGMP_MEMBERSHIP_QUERY),
		SYMBOL("membership-report-v1",		IGMP_V1_MEMBERSHIP_REPORT),
		SYMBOL("membership-report-v2",		IGMP_V2_MEMBERSHIP_REPORT),
		SYMBOL("membership-report-v3",		IGMP_V3_MEMBERSHIP_REPORT),
		SYMBOL("leave-group",			IGMP_V2_LEAVE_GROUP),
		SYMBOL_LIST_END
	},
};

const struct datatype igmp_type_type = {
	.type		= TYPE_IGMP_TYPE,
	.name		= "igmp_type",
	.desc		= "IGMP type",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= BITS_PER_BYTE,
	.basetype	= &integer_type,
	.sym_tbl	= &igmp_type_tbl,
};

#define IGMPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct igmp, __member)
#define IGMPHDR_TYPE(__name, __type, __member) \
	HDR_TYPE(__name, __type, struct igmp, __member)

const struct proto_desc proto_igmp = {
	.name		= "igmp",
	.id		= PROTO_DESC_IGMP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.checksum_key	= IGMPHDR_CHECKSUM,
	.checksum_type  = NFT_PAYLOAD_CSUM_INET,
	.templates	= {
		[IGMPHDR_TYPE]		= IGMPHDR_TYPE("type", &igmp_type_type, igmp_type),
		[IGMPHDR_MRT]		= IGMPHDR_FIELD("mrt", igmp_code),
		[IGMPHDR_CHECKSUM]	= IGMPHDR_FIELD("checksum", igmp_cksum),
		[IGMPHDR_GROUP]		= IGMPHDR_FIELD("group", igmp_group),
	},
};

/*
 * UDP/UDP-Lite
 */

#include <netinet/udp.h>
#define UDPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct udphdr, __member)

const struct proto_desc proto_udp = {
	.name		= "udp",
	.id		= PROTO_DESC_UDP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.templates	= {
		[UDPHDR_SPORT]		= INET_SERVICE("sport", struct udphdr, source),
		[UDPHDR_DPORT]		= INET_SERVICE("dport", struct udphdr, dest),
		[UDPHDR_LENGTH]		= UDPHDR_FIELD("length", len),
		[UDPHDR_CHECKSUM]	= UDPHDR_FIELD("checksum", check),
	},
	.protocols	= {
		PROTO_LINK(0,	&proto_vxlan),
		PROTO_LINK(0,	&proto_geneve),
	},
};

const struct proto_desc proto_udplite = {
	.name		= "udplite",
	.id		= PROTO_DESC_UDPLITE,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.templates	= {
		[UDPHDR_SPORT]		= INET_SERVICE("sport", struct udphdr, source),
		[UDPHDR_DPORT]		= INET_SERVICE("dport", struct udphdr, dest),
		[UDPHDR_CSUMCOV]	= UDPHDR_FIELD("csumcov", len),
		[UDPHDR_CHECKSUM]	= UDPHDR_FIELD("checksum", check),
	},
};

/*
 * TCP
 */

#include <netinet/tcp.h>

static const struct symbol_table tcp_flag_tbl = {
	.base		= BASE_HEXADECIMAL,
	.symbols	= {
		SYMBOL("fin",	TCP_FLAG_FIN),
		SYMBOL("syn",	TCP_FLAG_SYN),
		SYMBOL("rst",	TCP_FLAG_RST),
		SYMBOL("psh",	TCP_FLAG_PSH),
		SYMBOL("ack",	TCP_FLAG_ACK),
		SYMBOL("urg",	TCP_FLAG_URG),
		SYMBOL("ecn",	TCP_FLAG_ECN),
		SYMBOL("cwr",	TCP_FLAG_CWR),
		SYMBOL_LIST_END
	},
};

const struct datatype tcp_flag_type = {
	.type		= TYPE_TCP_FLAG,
	.name		= "tcp_flag",
	.desc		= "TCP flag",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= BITS_PER_BYTE,
	.basetype	= &bitmask_type,
	.sym_tbl	= &tcp_flag_tbl,
};

#define TCPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct tcphdr, __member)

const struct proto_desc proto_tcp = {
	.name		= "tcp",
	.id		= PROTO_DESC_TCP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.checksum_key	= TCPHDR_CHECKSUM,
	.checksum_type  = NFT_PAYLOAD_CSUM_INET,
	.templates	= {
		[TCPHDR_SPORT]		= INET_SERVICE("sport", struct tcphdr, source),
		[TCPHDR_DPORT]		= INET_SERVICE("dport", struct tcphdr, dest),
		[TCPHDR_SEQ]		= TCPHDR_FIELD("sequence", seq),
		[TCPHDR_ACKSEQ]		= TCPHDR_FIELD("ackseq", ack_seq),
		[TCPHDR_DOFF]		= HDR_BITFIELD("doff", &integer_type,
						       (12 * BITS_PER_BYTE), 4),
		[TCPHDR_RESERVED]	= HDR_BITFIELD("reserved", &integer_type,
						       (12 * BITS_PER_BYTE) + 4, 4),
		[TCPHDR_FLAGS]		= HDR_BITFIELD("flags", &tcp_flag_type,
						       13 * BITS_PER_BYTE,
						       BITS_PER_BYTE),
		[TCPHDR_WINDOW]		= TCPHDR_FIELD("window", window),
		[TCPHDR_CHECKSUM]	= TCPHDR_FIELD("checksum", check),
		[TCPHDR_URGPTR]		= TCPHDR_FIELD("urgptr", urg_ptr),
	},
	.format		= {
		.filter	= (1 << TCPHDR_SEQ) | (1 << TCPHDR_ACKSEQ) |
			  (1 << TCPHDR_DOFF) | (1 << TCPHDR_RESERVED) |
			  (1 << TCPHDR_URGPTR),
	},
};

/*
 * DCCP
 */

static const struct symbol_table dccp_pkttype_tbl = {
	.base		= BASE_HEXADECIMAL,
	.symbols	= {
		SYMBOL("request",	DCCP_PKT_REQUEST),
		SYMBOL("response",	DCCP_PKT_RESPONSE),
		SYMBOL("data",		DCCP_PKT_DATA),
		SYMBOL("ack",		DCCP_PKT_ACK),
		SYMBOL("dataack",	DCCP_PKT_DATAACK),
		SYMBOL("closereq",	DCCP_PKT_CLOSEREQ),
		SYMBOL("close",		DCCP_PKT_CLOSE),
		SYMBOL("reset",		DCCP_PKT_RESET),
		SYMBOL("sync",		DCCP_PKT_SYNC),
		SYMBOL("syncack",	DCCP_PKT_SYNCACK),
		SYMBOL_LIST_END
	},
};

const struct datatype dccp_pkttype_type = {
	.type		= TYPE_DCCP_PKTTYPE,
	.name		= "dccp_pkttype",
	.desc		= "DCCP packet type",
	.byteorder	= BYTEORDER_INVALID,
	.size		= 4,
	.basetype	= &integer_type,
	.sym_tbl	= &dccp_pkttype_tbl,
};


#define DCCPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct dccp_hdr, __member)

const struct proto_desc proto_dccp = {
	.name		= "dccp",
	.id		= PROTO_DESC_DCCP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.templates	= {
		[DCCPHDR_SPORT]		= INET_SERVICE("sport", struct dccp_hdr, dccph_sport),
		[DCCPHDR_DPORT]		= INET_SERVICE("dport", struct dccp_hdr, dccph_dport),
		[DCCPHDR_TYPE]		= HDR_BITFIELD("type", &dccp_pkttype_type,
						       (8 * BITS_PER_BYTE) + 3, 4),
	},
};

/*
 * SCTP
 */

#define SCTPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct sctphdr, __member)

const struct proto_desc proto_sctp = {
	.name		= "sctp",
	.id		= PROTO_DESC_SCTP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.checksum_key	= SCTPHDR_CHECKSUM,
	.checksum_type  = NFT_PAYLOAD_CSUM_SCTP,
	.templates	= {
		[SCTPHDR_SPORT]		= INET_SERVICE("sport", struct sctphdr, source),
		[SCTPHDR_DPORT]		= INET_SERVICE("dport", struct sctphdr, dest),
		[SCTPHDR_VTAG]		= SCTPHDR_FIELD("vtag", vtag),
		[SCTPHDR_CHECKSUM]	= SCTPHDR_FIELD("checksum", checksum),
	},
};

/*
 * Dummy Transpor Header (common udp/tcp/dccp/sctp fields)
 */
const struct proto_desc proto_th = {
	.name		= "th",
	.id		= PROTO_DESC_TH,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.templates	= {
		[THDR_SPORT]		= INET_SERVICE("sport", struct udphdr, source),
		[THDR_DPORT]		= INET_SERVICE("dport", struct udphdr, dest),
	},
};

/*
 * IPv4
 */

#include <netinet/ip.h>

static const struct symbol_table dscp_type_tbl = {
	.base		= BASE_HEXADECIMAL,
	.symbols	= {
		SYMBOL("cs0",	0x00),
		SYMBOL("cs1",	0x08),
		SYMBOL("cs2",	0x10),
		SYMBOL("cs3",	0x18),
		SYMBOL("cs4",	0x20),
		SYMBOL("cs5",	0x28),
		SYMBOL("cs6",	0x30),
		SYMBOL("cs7",	0x38),
		SYMBOL("df",	0x00),
		SYMBOL("be",	0x00),
		SYMBOL("lephb",	0x01),
		SYMBOL("af11",	0x0a),
		SYMBOL("af12",	0x0c),
		SYMBOL("af13",	0x0e),
		SYMBOL("af21",	0x12),
		SYMBOL("af22",	0x14),
		SYMBOL("af23",	0x16),
		SYMBOL("af31",	0x1a),
		SYMBOL("af32",	0x1c),
		SYMBOL("af33",	0x1e),
		SYMBOL("af41",	0x22),
		SYMBOL("af42",	0x24),
		SYMBOL("af43",	0x26),
		SYMBOL("va",	0x2c),
		SYMBOL("ef",	0x2e),
		SYMBOL_LIST_END
	},
};

const struct datatype dscp_type = {
	.type		= TYPE_DSCP,
	.name		= "dscp",
	.desc		= "Differentiated Services Code Point",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= 6,
	.basetype	= &integer_type,
	.basefmt	= "0x%.2Zx",
	.sym_tbl	= &dscp_type_tbl,
};

static const struct symbol_table ecn_type_tbl = {
	.base		= BASE_HEXADECIMAL,
	.symbols	= {
		SYMBOL("not-ect",	0x00),
		SYMBOL("ect1",		0x01),
		SYMBOL("ect0",		0x02),
		SYMBOL("ce",		0x03),
		SYMBOL_LIST_END
	},
};

const struct datatype ecn_type = {
	.type		= TYPE_ECN,
	.name		= "ecn",
	.desc		= "Explicit Congestion Notification",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= 2,
	.basetype	= &integer_type,
	.basefmt	= "0x%.1Zx",
	.sym_tbl	= &ecn_type_tbl,
};

#define GREHDR_TEMPLATE(__name, __dtype, __member) \
	HDR_TEMPLATE(__name, __dtype, struct grehdr, __member)
#define GREHDR_TYPE(__name, __member) \
	GREHDR_TEMPLATE(__name, &ethertype_type, __member)

const struct proto_desc proto_gre = {
	.name		= "gre",
	.id		= PROTO_DESC_GRE,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.templates	= {
		[0] = PROTO_META_TEMPLATE("l4proto", &inet_protocol_type, NFT_META_L4PROTO, 8),
		[GREHDR_FLAGS]		= HDR_BITFIELD("flags", &integer_type, 0, 5),
		[GREHDR_VERSION]	= HDR_BITFIELD("version", &integer_type, 13, 3),
		[GREHDR_PROTOCOL]	= HDR_BITFIELD("protocol", &ethertype_type, 16, 16),
	},
	.inner		= {
		.hdrsize	= sizeof(struct grehdr),
		.flags		= NFT_INNER_NH | NFT_INNER_TH,
		.type		= NFT_INNER_GENEVE + 1,
	},
};

const struct proto_desc proto_gretap = {
	.name		= "gretap",
	.id		= PROTO_DESC_GRETAP,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.templates	= {
		[0] = PROTO_META_TEMPLATE("l4proto", &inet_protocol_type, NFT_META_L4PROTO, 8),
	},
	.inner		= {
		.hdrsize	= sizeof(struct grehdr),
		.flags		= NFT_INNER_LL | NFT_INNER_NH | NFT_INNER_TH,
		.type		= NFT_INNER_GENEVE + 2,
	},
};

#define IPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct iphdr, __member)
#define IPHDR_ADDR(__name, __member) \
	HDR_TYPE(__name, &ipaddr_type, struct iphdr, __member)

const struct proto_desc proto_ip = {
	.name		= "ip",
	.id		= PROTO_DESC_IP,
	.base		= PROTO_BASE_NETWORK_HDR,
	.checksum_key	= IPHDR_CHECKSUM,
	.checksum_type  = NFT_PAYLOAD_CSUM_INET,
	.protocols	= {
		PROTO_LINK(IPPROTO_ICMP,	&proto_icmp),
		PROTO_LINK(IPPROTO_IGMP,	&proto_igmp),
		PROTO_LINK(IPPROTO_ICMPV6,	&proto_icmp6),
		PROTO_LINK(IPPROTO_ESP,		&proto_esp),
		PROTO_LINK(IPPROTO_AH,		&proto_ah),
		PROTO_LINK(IPPROTO_COMP,	&proto_comp),
		PROTO_LINK(IPPROTO_UDP,		&proto_udp),
		PROTO_LINK(IPPROTO_UDPLITE,	&proto_udplite),
		PROTO_LINK(IPPROTO_TCP,		&proto_tcp),
		PROTO_LINK(IPPROTO_DCCP,	&proto_dccp),
		PROTO_LINK(IPPROTO_SCTP,	&proto_sctp),
		PROTO_LINK(IPPROTO_GRE,		&proto_gre),
		PROTO_LINK(IPPROTO_GRE,		&proto_gretap),
	},
	.templates	= {
		[0]	= PROTO_META_TEMPLATE("l4proto", &inet_protocol_type, NFT_META_L4PROTO, 8),
		[IPHDR_VERSION]		= HDR_BITFIELD("version", &integer_type, 0, 4),
		[IPHDR_HDRLENGTH]	= HDR_BITFIELD("hdrlength", &integer_type, 4, 4),
		[IPHDR_DSCP]            = HDR_BITFIELD("dscp", &dscp_type, 8, 6),
		[IPHDR_ECN]		= HDR_BITFIELD("ecn", &ecn_type, 14, 2),
		[IPHDR_LENGTH]		= IPHDR_FIELD("length",		tot_len),
		[IPHDR_ID]		= IPHDR_FIELD("id",		id),
		[IPHDR_FRAG_OFF]	= HDR_HEX_FIELD("frag-off", struct iphdr, frag_off),
		[IPHDR_TTL]		= IPHDR_FIELD("ttl",		ttl),
		[IPHDR_PROTOCOL]	= INET_PROTOCOL("protocol", struct iphdr, protocol),
		[IPHDR_CHECKSUM]	= IPHDR_FIELD("checksum",	check),
		[IPHDR_SADDR]		= IPHDR_ADDR("saddr",		saddr),
		[IPHDR_DADDR]		= IPHDR_ADDR("daddr",		daddr),
	},
	.format		= {
		.order	= {
			IPHDR_SADDR, IPHDR_DADDR, IPHDR_DSCP, IPHDR_ECN,
			IPHDR_TTL, IPHDR_ID, IPHDR_PROTOCOL, IPHDR_LENGTH,
		},
		.filter	= (1 << IPHDR_VERSION)  | (1 << IPHDR_HDRLENGTH) |
			  (1 << IPHDR_FRAG_OFF),
	},
	.pseudohdr	= {
		IPHDR_SADDR, IPHDR_DADDR, IPHDR_PROTOCOL, IPHDR_LENGTH,
	},
};

/*
 * ICMPv6
 */

#include <netinet/icmp6.h>

#define IND_NEIGHBOR_SOLICIT	141
#define IND_NEIGHBOR_ADVERT	142
#define ICMPV6_MLD2_REPORT	143

static const struct symbol_table icmp6_type_tbl = {
	.base		= BASE_DECIMAL,
	.symbols	= {
		SYMBOL("destination-unreachable",	ICMP6_DST_UNREACH),
		SYMBOL("packet-too-big",		ICMP6_PACKET_TOO_BIG),
		SYMBOL("time-exceeded",			ICMP6_TIME_EXCEEDED),
		SYMBOL("parameter-problem",		ICMP6_PARAM_PROB),
		SYMBOL("echo-request",			ICMP6_ECHO_REQUEST),
		SYMBOL("echo-reply",			ICMP6_ECHO_REPLY),
		SYMBOL("mld-listener-query",		MLD_LISTENER_QUERY),
		SYMBOL("mld-listener-report",		MLD_LISTENER_REPORT),
		SYMBOL("mld-listener-done",		MLD_LISTENER_REDUCTION),
		SYMBOL("mld-listener-reduction",	MLD_LISTENER_REDUCTION),
		SYMBOL("nd-router-solicit",		ND_ROUTER_SOLICIT),
		SYMBOL("nd-router-advert",		ND_ROUTER_ADVERT),
		SYMBOL("nd-neighbor-solicit",		ND_NEIGHBOR_SOLICIT),
		SYMBOL("nd-neighbor-advert",		ND_NEIGHBOR_ADVERT),
		SYMBOL("nd-redirect",			ND_REDIRECT),
		SYMBOL("router-renumbering",		ICMP6_ROUTER_RENUMBERING),
		SYMBOL("ind-neighbor-solicit",		IND_NEIGHBOR_SOLICIT),
		SYMBOL("ind-neighbor-advert",		IND_NEIGHBOR_ADVERT),
		SYMBOL("mld2-listener-report",		ICMPV6_MLD2_REPORT),
		SYMBOL_LIST_END
	},
};

const struct datatype icmp6_type_type = {
	.type		= TYPE_ICMP6_TYPE,
	.name		= "icmpv6_type",
	.desc		= "ICMPv6 type",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= BITS_PER_BYTE,
	.basetype	= &integer_type,
	.sym_tbl	= &icmp6_type_tbl,
};

#define ICMP6HDR_FIELD(__token, __member, __dep) \
	ICMP46HDR_FIELD(__token, integer_type, struct icmp6_hdr, __member, __dep)
#define ICMP6HDR_TYPE(__name, __type, __member) \
	HDR_TYPE(__name, __type, struct icmp6_hdr, __member)

const struct proto_desc proto_icmp6 = {
	.name		= "icmpv6",
	.id		= PROTO_DESC_ICMPV6,
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.protocol_key	= ICMP6HDR_TYPE,
	.checksum_key	= ICMP6HDR_CHECKSUM,
	.checksum_type  = NFT_PAYLOAD_CSUM_INET,
	.templates	= {
		[ICMP6HDR_TYPE]		= ICMP6HDR_TYPE("type", &icmp6_type_type, icmp6_type),
		[ICMP6HDR_CODE]		= ICMP6HDR_TYPE("code", &icmpv6_code_type, icmp6_code),
		[ICMP6HDR_CHECKSUM]	= ICMP6HDR_FIELD("checksum", icmp6_cksum, PROTO_ICMP_ANY),
		[ICMP6HDR_PPTR]		= ICMP6HDR_FIELD("parameter-problem", icmp6_pptr, PROTO_ICMP6_PPTR),
		[ICMP6HDR_MTU]		= ICMP6HDR_FIELD("mtu", icmp6_mtu, PROTO_ICMP6_MTU),
		[ICMP6HDR_ID]		= ICMP6HDR_FIELD("id", icmp6_id, PROTO_ICMP6_ECHO),
		[ICMP6HDR_SEQ]		= ICMP6HDR_FIELD("sequence", icmp6_seq, PROTO_ICMP6_ECHO),
		[ICMP6HDR_MAXDELAY]	= ICMP6HDR_FIELD("max-delay", icmp6_maxdelay, PROTO_ICMP6_MGMQ),
		[ICMP6HDR_TADDR]	= ICMP46HDR_FIELD("taddr", ip6addr_type,
							  struct nd_neighbor_solicit, nd_ns_target,
							  PROTO_ICMP6_ADDRESS),
		[ICMP6HDR_DADDR]	= ICMP46HDR_FIELD("daddr", ip6addr_type,
							  struct nd_redirect, nd_rd_dst,
							  PROTO_ICMP6_REDIRECT),
	},
};

/*
 * IPv6
 */

#define IP6HDR_FIELD(__name,  __member) \
	HDR_FIELD(__name, struct ipv6hdr, __member)
#define IP6HDR_ADDR(__name, __member) \
	HDR_TYPE(__name, &ip6addr_type, struct ipv6hdr, __member)
#define IP6HDR_PROTOCOL(__name, __member) \
	HDR_TYPE(__name, &inet_service_type, struct ipv6hdr, __member)

const struct proto_desc proto_ip6 = {
	.name		= "ip6",
	.id		= PROTO_DESC_IP6,
	.base		= PROTO_BASE_NETWORK_HDR,
	.protocols	= {
		PROTO_LINK(IPPROTO_ESP,		&proto_esp),
		PROTO_LINK(IPPROTO_AH,		&proto_ah),
		PROTO_LINK(IPPROTO_COMP,	&proto_comp),
		PROTO_LINK(IPPROTO_UDP,		&proto_udp),
		PROTO_LINK(IPPROTO_UDPLITE,	&proto_udplite),
		PROTO_LINK(IPPROTO_TCP,		&proto_tcp),
		PROTO_LINK(IPPROTO_DCCP,	&proto_dccp),
		PROTO_LINK(IPPROTO_SCTP,	&proto_sctp),
		PROTO_LINK(IPPROTO_ICMP,	&proto_icmp),
		PROTO_LINK(IPPROTO_IGMP,	&proto_igmp),
		PROTO_LINK(IPPROTO_ICMPV6,	&proto_icmp6),
		PROTO_LINK(IPPROTO_GRE,		&proto_gre),
		PROTO_LINK(IPPROTO_GRE,		&proto_gretap),
	},
	.templates	= {
		[0]	= PROTO_META_TEMPLATE("l4proto", &inet_protocol_type, NFT_META_L4PROTO, 8),
		[IP6HDR_VERSION]	= HDR_BITFIELD("version", &integer_type, 0, 4),
		[IP6HDR_DSCP]		= HDR_BITFIELD("dscp", &dscp_type, 4, 6),
		[IP6HDR_ECN]		= HDR_BITFIELD("ecn", &ecn_type, 10, 2),
		[IP6HDR_FLOWLABEL]	= HDR_BITFIELD("flowlabel", &integer_type, 12, 20),
		[IP6HDR_LENGTH]		= IP6HDR_FIELD("length",	payload_len),
		[IP6HDR_NEXTHDR]	= INET_PROTOCOL("nexthdr", struct ipv6hdr, nexthdr),
		[IP6HDR_HOPLIMIT]	= IP6HDR_FIELD("hoplimit",	hop_limit),
		[IP6HDR_SADDR]		= IP6HDR_ADDR("saddr",		saddr),
		[IP6HDR_DADDR]		= IP6HDR_ADDR("daddr",		daddr),
	},
	.format		= {
		.order	= {
			IP6HDR_SADDR, IP6HDR_DADDR, IP6HDR_DSCP, IP6HDR_ECN,
			IP6HDR_HOPLIMIT, IP6HDR_FLOWLABEL, IP6HDR_NEXTHDR,
			IP6HDR_LENGTH,
		},
		.filter	= (1 << IP6HDR_VERSION),
	},
	.pseudohdr	= {
		IP6HDR_SADDR, IP6HDR_DADDR, IP6HDR_NEXTHDR, IP6HDR_LENGTH,
	},
};

/*
 * Dummy protocol for mixed IPv4/IPv6 tables. The protocol is set at the link
 * layer header, the upper layer protocols are IPv4 and IPv6.
 */

const struct proto_desc proto_inet = {
	.name		= "inet",
	.base		= PROTO_BASE_LL_HDR,
	.protocols	= {
		PROTO_LINK(NFPROTO_IPV4,	&proto_ip),
		PROTO_LINK(NFPROTO_IPV6,	&proto_ip6),
	},
	.templates	= {
		[0]	= PROTO_META_TEMPLATE("nfproto", &nfproto_type, NFT_META_NFPROTO, 8),
	},
};

/*
 * Dummy protocol for cases where the network layer protocol isn't known
 * (IPv4 or IPv6), The higher layer protocols are the protocols common to
 * both.
 */

const struct proto_desc proto_inet_service = {
	.name		= "inet-service",
	.base		= PROTO_BASE_TRANSPORT_HDR,
	.protocol_key	= 0,
	.protocols	= {
		PROTO_LINK(IPPROTO_ESP,		&proto_esp),
		PROTO_LINK(IPPROTO_AH,		&proto_ah),
		PROTO_LINK(IPPROTO_COMP,	&proto_comp),
		PROTO_LINK(IPPROTO_UDP,		&proto_udp),
		PROTO_LINK(IPPROTO_UDPLITE,	&proto_udplite),
		PROTO_LINK(IPPROTO_TCP,		&proto_tcp),
		PROTO_LINK(IPPROTO_DCCP,	&proto_dccp),
		PROTO_LINK(IPPROTO_SCTP,	&proto_sctp),
		PROTO_LINK(IPPROTO_ICMP,	&proto_icmp),
		PROTO_LINK(IPPROTO_IGMP,	&proto_igmp),
		PROTO_LINK(IPPROTO_ICMPV6,	&proto_icmp6),
		PROTO_LINK(IPPROTO_GRE,		&proto_gre),
		PROTO_LINK(IPPROTO_GRE,		&proto_gretap),
	},
	.templates	= {
		[0]	= PROTO_META_TEMPLATE("l4proto", &inet_protocol_type, NFT_META_L4PROTO, 8),
	},
};

/*
 * ARP
 */

#include <net/if_arp.h>

static const struct symbol_table arpop_tbl = {
	.base		= BASE_HEXADECIMAL,
	.symbols	= {
		SYMBOL("request",	__constant_htons(ARPOP_REQUEST)),
		SYMBOL("reply",		__constant_htons(ARPOP_REPLY)),
		SYMBOL("rrequest",	__constant_htons(ARPOP_RREQUEST)),
		SYMBOL("rreply",	__constant_htons(ARPOP_RREPLY)),
		SYMBOL("inrequest",	__constant_htons(ARPOP_InREQUEST)),
		SYMBOL("inreply",	__constant_htons(ARPOP_InREPLY)),
		SYMBOL("nak",		__constant_htons(ARPOP_NAK)),
		SYMBOL_LIST_END
	},
};

const struct datatype arpop_type = {
	.type		= TYPE_ARPOP,
	.name		= "arp_op",
	.desc		= "ARP operation",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= 2 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.sym_tbl	= &arpop_tbl,
};

#define ARPHDR_TYPE(__name, __type, __member) \
	HDR_TYPE(__name, __type, struct arp_hdr, __member)
#define ARPHDR_FIELD(__name, __member) \
	HDR_FIELD(__name, struct arp_hdr, __member)

const struct proto_desc proto_arp = {
	.name		= "arp",
	.id		= PROTO_DESC_ARP,
	.base		= PROTO_BASE_NETWORK_HDR,
	.templates	= {
		[ARPHDR_HRD]		= ARPHDR_FIELD("htype",	htype),
		[ARPHDR_PRO]		= ARPHDR_TYPE("ptype", &ethertype_type, ptype),
		[ARPHDR_HLN]		= ARPHDR_FIELD("hlen", hlen),
		[ARPHDR_PLN]		= ARPHDR_FIELD("plen", plen),
		[ARPHDR_OP]		= ARPHDR_TYPE("operation", &arpop_type, oper),
		[ARPHDR_SADDR_ETHER]	= ARPHDR_TYPE("saddr ether", &etheraddr_type, sha),
		[ARPHDR_SADDR_IP]	= ARPHDR_TYPE("saddr ip", &ipaddr_type, spa),
		[ARPHDR_DADDR_ETHER]	= ARPHDR_TYPE("daddr ether", &etheraddr_type, tha),
		[ARPHDR_DADDR_IP]	= ARPHDR_TYPE("daddr ip", &ipaddr_type, tpa),
	},
	.format		= {
		.filter	= (1 << ARPHDR_HRD) | (1 << ARPHDR_PRO) |
			  (1 << ARPHDR_HLN) | (1 << ARPHDR_PLN) |
			  (1 << ARPHDR_SADDR_ETHER) | (1 << ARPHDR_DADDR_ETHER) |
			  (1 << ARPHDR_SADDR_IP) | (1 << ARPHDR_DADDR_IP),
	},
};

/*
 * VLAN
 */

#include <net/ethernet.h>

#define VLANHDR_BITFIELD(__name, __offset, __len) \
	HDR_BITFIELD(__name, &integer_type, __offset, __len)
#define VLANHDR_TYPE(__name, __type, __member) \
	HDR_TYPE(__name, __type, struct vlan_hdr, __member)

const struct proto_desc proto_vlan = {
	.name		= "vlan",
	.id		= PROTO_DESC_VLAN,
	.base		= PROTO_BASE_LL_HDR,
	.protocol_key	= VLANHDR_TYPE,
	.length		= sizeof(struct vlan_hdr) * BITS_PER_BYTE,
	.protocols	= {
		PROTO_LINK(__constant_htons(ETH_P_IP),		&proto_ip),
		PROTO_LINK(__constant_htons(ETH_P_ARP),		&proto_arp),
		PROTO_LINK(__constant_htons(ETH_P_IPV6),	&proto_ip6),
		PROTO_LINK(__constant_htons(ETH_P_8021Q),	&proto_vlan),
		PROTO_LINK(__constant_htons(ETH_P_8021AD),	&proto_vlan),

	},
	.templates	= {
		[VLANHDR_PCP]		= VLANHDR_BITFIELD("pcp", 0, 3),
		[VLANHDR_DEI]		= VLANHDR_BITFIELD("dei", 3, 1),
		[VLANHDR_CFI]		= VLANHDR_BITFIELD("cfi", 3, 1),
		[VLANHDR_VID]		= VLANHDR_BITFIELD("id", 4, 12),
		[VLANHDR_TYPE]		= VLANHDR_TYPE("type", &ethertype_type, vlan_type),
	},
};

/*
 * Ethernet
 */

const struct datatype etheraddr_type = {
	.type		= TYPE_ETHERADDR,
	.name		= "ether_addr",
	.desc		= "Ethernet address",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= ETH_ALEN * BITS_PER_BYTE,
	.basetype	= &lladdr_type,
};

static const struct symbol_table ethertype_tbl = {
	.base		= BASE_HEXADECIMAL,
	.symbols	= {
		SYMBOL("ip",		__constant_htons(ETH_P_IP)),
		SYMBOL("arp",		__constant_htons(ETH_P_ARP)),
		SYMBOL("ip6",		__constant_htons(ETH_P_IPV6)),
		SYMBOL("8021q",		__constant_htons(ETH_P_8021Q)),
		SYMBOL("8021ad",	__constant_htons(ETH_P_8021AD)),

		/* for compatibility with older versions */
		SYMBOL("vlan",		__constant_htons(ETH_P_8021Q)),
		SYMBOL_LIST_END
	},
};

static void ethertype_print(const struct expr *expr, struct output_ctx *octx)
{
	return symbolic_constant_print(&ethertype_tbl, expr, false, octx);
}

const struct datatype ethertype_type = {
	.type		= TYPE_ETHERTYPE,
	.name		= "ether_type",
	.desc		= "Ethernet protocol",
	.byteorder	= BYTEORDER_BIG_ENDIAN,
	.size		= 2 * BITS_PER_BYTE,
	.basetype	= &integer_type,
	.basefmt	= "0x%.4Zx",
	.print		= ethertype_print,
	.sym_tbl	= &ethertype_tbl,
};

#define ETHHDR_TEMPLATE(__name, __dtype, __member) \
	HDR_TEMPLATE(__name, __dtype, struct ether_header, __member)
#define ETHHDR_TYPE(__name, __member) \
	ETHHDR_TEMPLATE(__name, &ethertype_type, __member)
#define ETHHDR_ADDR(__name, __member) \
	PROTO_HDR_TEMPLATE(__name, &etheraddr_type,		\
			   BYTEORDER_BIG_ENDIAN,		\
			   offsetof(struct ether_header, __member) * 8,	\
			   field_sizeof(struct ether_header, __member) * 8)

const struct proto_desc proto_eth = {
	.name		= "ether",
	.id		= PROTO_DESC_ETHER,
	.base		= PROTO_BASE_LL_HDR,
	.protocol_key	= ETHHDR_TYPE,
	.length		= sizeof(struct ether_header) * BITS_PER_BYTE,
	.protocols	= {
		PROTO_LINK(__constant_htons(ETH_P_IP),		&proto_ip),
		PROTO_LINK(__constant_htons(ETH_P_ARP),		&proto_arp),
		PROTO_LINK(__constant_htons(ETH_P_IPV6),	&proto_ip6),
		PROTO_LINK(__constant_htons(ETH_P_8021Q),	&proto_vlan),
		PROTO_LINK(__constant_htons(ETH_P_8021AD),	&proto_vlan),
	},
	.templates	= {
		[ETHHDR_DADDR]		= ETHHDR_ADDR("daddr", ether_dhost),
		[ETHHDR_SADDR]		= ETHHDR_ADDR("saddr", ether_shost),
		[ETHHDR_TYPE]		= ETHHDR_TYPE("type", ether_type),
	},
	.format		= {
		.order	= {
			ETHHDR_SADDR, ETHHDR_DADDR, ETHHDR_TYPE,
		},
	},

};

/*
 * VXLAN
 */

const struct proto_desc proto_vxlan = {
	.name		= "vxlan",
	.id		= PROTO_DESC_VXLAN,
	.base		= PROTO_BASE_INNER_HDR,
	.templates	= {
		[VXLANHDR_FLAGS] = HDR_BITFIELD("flags", &bitmask_type, 0, 8),
		[VXLANHDR_VNI]	 = HDR_BITFIELD("vni", &integer_type, (4 * BITS_PER_BYTE), 24),
	},
	.protocols	= {
		PROTO_LINK(__constant_htons(ETH_P_IP),		&proto_ip),
		PROTO_LINK(__constant_htons(ETH_P_ARP),		&proto_arp),
		PROTO_LINK(__constant_htons(ETH_P_IPV6),	&proto_ip6),
		PROTO_LINK(__constant_htons(ETH_P_8021Q),	&proto_vlan),
	},
	.inner		= {
		.hdrsize	= sizeof(struct vxlanhdr),
		.flags		= NFT_INNER_HDRSIZE | NFT_INNER_LL | NFT_INNER_NH | NFT_INNER_TH,
		.type		= NFT_INNER_VXLAN,
	},
};

/*
 * GENEVE
 */

const struct proto_desc proto_geneve = {
	.name		= "geneve",
	.id		= PROTO_DESC_GENEVE,
	.base		= PROTO_BASE_INNER_HDR,
	.templates	= {
		[GNVHDR_TYPE]	= HDR_TYPE("type", &ethertype_type, struct gnvhdr, type),
		[GNVHDR_VNI]	= HDR_BITFIELD("vni", &integer_type, (4 * BITS_PER_BYTE), 24),
	},
	.protocols	= {
		PROTO_LINK(__constant_htons(ETH_P_IP),		&proto_ip),
		PROTO_LINK(__constant_htons(ETH_P_ARP),		&proto_arp),
		PROTO_LINK(__constant_htons(ETH_P_IPV6),	&proto_ip6),
		PROTO_LINK(__constant_htons(ETH_P_8021Q),	&proto_vlan),
	},
	.inner		= {
		.hdrsize	= sizeof(struct gnvhdr),
		.flags		= NFT_INNER_HDRSIZE | NFT_INNER_LL | NFT_INNER_NH | NFT_INNER_TH,
		.type		= NFT_INNER_GENEVE,
	},
};


/*
 * Dummy protocol for netdev tables.
 */
const struct proto_desc proto_netdev = {
	.name		= "netdev",
	.base		= PROTO_BASE_LL_HDR,
	.protocols	= {
		PROTO_LINK(__constant_htons(ETH_P_IP),		&proto_ip),
		PROTO_LINK(__constant_htons(ETH_P_ARP),		&proto_arp),
		PROTO_LINK(__constant_htons(ETH_P_IPV6),	&proto_ip6),
		PROTO_LINK(__constant_htons(ETH_P_8021Q),	&proto_vlan),
		PROTO_LINK(__constant_htons(ETH_P_8021AD),	&proto_vlan),
	},
	.templates	= {
		[0]	= PROTO_META_TEMPLATE("protocol", &ethertype_type, NFT_META_PROTOCOL, 16),
	},
};

static const struct proto_desc *const proto_definitions[PROTO_DESC_MAX + 1] = {
	[PROTO_DESC_AH]		= &proto_ah,
	[PROTO_DESC_ESP]	= &proto_esp,
	[PROTO_DESC_COMP]	= &proto_comp,
	[PROTO_DESC_ICMP]	= &proto_icmp,
	[PROTO_DESC_IGMP]	= &proto_igmp,
	[PROTO_DESC_UDP]	= &proto_udp,
	[PROTO_DESC_UDPLITE]	= &proto_udplite,
	[PROTO_DESC_TCP]	= &proto_tcp,
	[PROTO_DESC_DCCP]	= &proto_dccp,
	[PROTO_DESC_SCTP]	= &proto_sctp,
	[PROTO_DESC_TH]		= &proto_th,
	[PROTO_DESC_IP]		= &proto_ip,
	[PROTO_DESC_IP6]	= &proto_ip6,
	[PROTO_DESC_ICMPV6]	= &proto_icmp6,
	[PROTO_DESC_ARP]	= &proto_arp,
	[PROTO_DESC_VLAN]	= &proto_vlan,
	[PROTO_DESC_ETHER]	= &proto_eth,
	[PROTO_DESC_VXLAN]	= &proto_vxlan,
	[PROTO_DESC_GENEVE]	= &proto_geneve,
	[PROTO_DESC_GRE]	= &proto_gre,
	[PROTO_DESC_GRETAP]	= &proto_gretap,
};

const struct proto_desc *proto_find_desc(enum proto_desc_id desc_id)
{
	if (desc_id >= PROTO_DESC_UNKNOWN &&
	    desc_id <= PROTO_DESC_MAX)
		return proto_definitions[desc_id];

	return NULL;
}
