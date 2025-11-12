#ifndef NFTABLES_PROTO_H
#define NFTABLES_PROTO_H

#include <nftables.h>
#include <datatype.h>
#include <linux/netfilter/nf_tables.h>

/**
 * enum proto_bases - protocol bases
 *
 * @PROTO_BASE_INVALID:		uninitialised, does not happen
 * @PROTO_BASE_LL_HDR:		link layer header
 * @PROTO_BASE_NETWORK_HDR:	network layer header
 * @PROTO_BASE_TRANSPORT_HDR:	transport layer header
 */
enum proto_bases {
	PROTO_BASE_INVALID,
	PROTO_BASE_LL_HDR,
	PROTO_BASE_NETWORK_HDR,
	PROTO_BASE_TRANSPORT_HDR,
	PROTO_BASE_INNER_HDR,
	__PROTO_BASE_MAX
};
#define PROTO_BASE_MAX		(__PROTO_BASE_MAX - 1)

extern const char *proto_base_names[];
extern const char *proto_base_tokens[];

enum icmp_hdr_field_type {
	PROTO_ICMP_ANY = 0,
	PROTO_ICMP_ECHO,	/* echo and reply */
	PROTO_ICMP_MTU,		/* destination unreachable */
	PROTO_ICMP_ADDRESS,	/* redirect */
	PROTO_ICMP6_MTU,
	PROTO_ICMP6_PPTR,
	PROTO_ICMP6_ECHO,
	PROTO_ICMP6_MGMQ,
	PROTO_ICMP6_ADDRESS,	/* neighbor solicit/advert, redirect and MLD */
	PROTO_ICMP6_REDIRECT,
};

/**
 * struct proto_hdr_template - protocol header field description
 *
 * @token:	parser token describing the header field
 * @dtype:	data type of the header field
 * @offset:	offset of the header field from base
 * @len:	length of header field
 * @meta_key:	special case: meta expression key
 * @icmp_dep:  special case: icmp header dependency
 */
struct proto_hdr_template {
	const char			*token;
	const struct datatype		*dtype;
	uint16_t			offset;
	uint16_t			len;
	enum byteorder			byteorder:8;
	enum nft_meta_keys		meta_key:8;
	enum icmp_hdr_field_type	icmp_dep:8;
};

#define PROTO_HDR_TEMPLATE(__token, __dtype,  __byteorder, __offset, __len)\
	{								\
		.token		= (__token),				\
		.dtype		= (__dtype),				\
		.byteorder	= (__byteorder),			\
		.offset		= (__offset),				\
		.len		= (__len),				\
	}

#define PROTO_META_TEMPLATE(__token, __dtype, __key, __len)		\
	{								\
		.token		= (__token),				\
		.dtype		= (__dtype),				\
		.meta_key	= (__key),				\
		.len		= (__len),				\
	}

#define PROTO_UPPER_MAX		16
#define PROTO_HDRS_MAX		20

enum proto_desc_id {
	PROTO_DESC_UNKNOWN	= 0,
	PROTO_DESC_AH,
	PROTO_DESC_ESP,
	PROTO_DESC_COMP,
	PROTO_DESC_ICMP,
	PROTO_DESC_IGMP,
	PROTO_DESC_UDP,
	PROTO_DESC_UDPLITE,
	PROTO_DESC_TCP,
	PROTO_DESC_DCCP,
	PROTO_DESC_SCTP,
	PROTO_DESC_TH,
	PROTO_DESC_IP,
	PROTO_DESC_IP6,
	PROTO_DESC_ICMPV6,
	PROTO_DESC_ARP,
	PROTO_DESC_VLAN,
	PROTO_DESC_ETHER,
	PROTO_DESC_VXLAN,
	PROTO_DESC_GENEVE,
	PROTO_DESC_GRE,
	PROTO_DESC_GRETAP,
	__PROTO_DESC_MAX
};
#define PROTO_DESC_MAX	(__PROTO_DESC_MAX - 1)

/**
 * struct proto_desc - protocol header description
 *
 * @name:	protocol name
 * @id:		protocol identifier
 * @base:	header base
 * @checksum_key: key of template containing checksum
 * @protocol_key: key of template containing upper layer protocol description
 * @length:	total size of the header, in bits
 * @protocols:	link to upper layer protocol descriptions indexed by protocol value
 * @templates:	header templates
 * @pseudohdr:  header fields that are part of upper layer checksum pseudoheader
 */
struct proto_desc {
	const char			*name;
	enum proto_desc_id		id:8;
	enum proto_bases		base:8;
	enum nft_payload_csum_types	checksum_type:8;
	uint16_t			checksum_key;
	uint16_t			protocol_key;
	unsigned int			length;
	struct {
		unsigned int			num;
		const struct proto_desc		*desc;
	}				protocols[PROTO_UPPER_MAX];
	struct proto_hdr_template	templates[PROTO_HDRS_MAX];
	struct {
		uint8_t				order[PROTO_HDRS_MAX];
		uint32_t			filter;
	}				format;
	unsigned int			pseudohdr[PROTO_HDRS_MAX];
	struct {
		uint32_t		hdrsize;
		uint32_t		flags;
		enum nft_inner_type	type;
	} inner;
};

#define PROTO_LINK(__num, __desc)	{ .num = (__num), .desc = (__desc), }

/**
 * struct hook_proto_desc - description of protocol constraints imposed by hook family
 *
 * @base:	protocol base of packets
 * @desc:	protocol description of packets
 */
struct hook_proto_desc {
	enum proto_bases		base;
	const struct proto_desc		*desc;
};

#define HOOK_PROTO_DESC(__base, __desc)	{ .base = (__base), .desc = (__desc), }

extern const struct hook_proto_desc hook_proto_desc[];

/**
 * struct dev_proto_desc - description of device LL protocol
 *
 * @desc:	protocol description
 * @type:	arphrd value
 */
struct dev_proto_desc {
	const struct proto_desc		*desc;
	uint16_t			type;
};

#define DEV_PROTO_DESC(__type, __desc)	{ .type = (__type), .desc = (__desc), }

extern int proto_dev_type(const struct proto_desc *desc, uint16_t *res);
extern const struct proto_desc *proto_dev_desc(uint16_t type);

#define PROTO_CTX_NUM_PROTOS	16

/**
 * struct proto_ctx - protocol context
 *
 * debug_mask:	display debugging information
 * @family:	hook family
 * @location:	location of the relational expression defining the context
 * @desc:	protocol description for this layer
 * @offset:	offset from the base, for stacked headers (eg 8*14 for vlan on top of ether)
 *
 * The location of the context is the location of the relational expression
 * defining it, either directly through a protocol match or indirectly
 * through a dependency.
 */
struct proto_ctx {
	unsigned int			debug_mask;
	uint8_t				family;
	bool				inner;
	union {
		struct {
			uint8_t			type;
		} icmp;
	} th_dep;
	struct {
		struct location			location;
		const struct proto_desc		*desc;
		struct {
			struct location		location;
			const struct proto_desc	*desc;
		} protos[PROTO_CTX_NUM_PROTOS];
		unsigned int			num_protos;
	} protocol[PROTO_BASE_MAX + 1];
	const struct proto_desc *stacked_ll[PROTO_CTX_NUM_PROTOS];
	uint8_t stacked_ll_count;
};

extern void proto_ctx_init(struct proto_ctx *ctx, unsigned int family,
			   unsigned int debug_mask, bool inner);
extern void proto_ctx_update(struct proto_ctx *ctx, enum proto_bases base,
			     const struct location *loc,
			     const struct proto_desc *desc);
bool proto_ctx_is_ambiguous(struct proto_ctx *ctx, enum proto_bases bases);
const struct proto_desc *proto_ctx_find_conflict(struct proto_ctx *ctx,
						 enum proto_bases base,
						 const struct proto_desc *desc);
extern const struct proto_desc *proto_find_upper(const struct proto_desc *base,
						 unsigned int num);
extern int proto_find_num(const struct proto_desc *base,
			  const struct proto_desc *desc);
const struct proto_desc *proto_find_inner(uint32_t type, uint32_t hdrsize,
					  uint32_t flags);

extern const struct proto_desc *proto_find_desc(enum proto_desc_id desc_id);

enum eth_hdr_fields {
	ETHHDR_INVALID,
	ETHHDR_DADDR,
	ETHHDR_SADDR,
	ETHHDR_TYPE,
};

enum vlan_hdr_fields {
	VLANHDR_INVALID,
	VLANHDR_PCP,
	VLANHDR_DEI,
	VLANHDR_CFI,
	VLANHDR_VID,
	VLANHDR_TYPE,
};

enum arp_hdr_fields {
	ARPHDR_INVALID,
	ARPHDR_HRD,
	ARPHDR_PRO,
	ARPHDR_HLN,
	ARPHDR_PLN,
	ARPHDR_OP,
	ARPHDR_SADDR_ETHER,
	ARPHDR_SADDR_IP,
	ARPHDR_DADDR_ETHER,
	ARPHDR_DADDR_IP,
};

enum ip_hdr_fields {
	IPHDR_INVALID,
	IPHDR_VERSION,
	IPHDR_HDRLENGTH,
	IPHDR_DSCP,
	IPHDR_ECN,
	IPHDR_LENGTH,
	IPHDR_ID,
	IPHDR_FRAG_OFF,
	IPHDR_TTL,
	IPHDR_PROTOCOL,
	IPHDR_CHECKSUM,
	IPHDR_SADDR,
	IPHDR_DADDR,
};
#define IPHDR_MAX	IPHDR_DADDR

enum icmp_hdr_fields {
	ICMPHDR_INVALID,
	ICMPHDR_TYPE,
	ICMPHDR_CODE,
	ICMPHDR_CHECKSUM,
	ICMPHDR_ID,
	ICMPHDR_SEQ,
	ICMPHDR_GATEWAY,
	ICMPHDR_MTU,
};

enum igmp_hdr_fields {
	IGMPHDR_INVALID,
	IGMPHDR_TYPE,
	IGMPHDR_CHECKSUM,
	IGMPHDR_MRT,
	IGMPHDR_GROUP,
};

enum icmp6_hdr_fields {
	ICMP6HDR_INVALID,
	ICMP6HDR_TYPE,
	ICMP6HDR_CODE,
	ICMP6HDR_CHECKSUM,
	ICMP6HDR_PPTR,
	ICMP6HDR_MTU,
	ICMP6HDR_ID,
	ICMP6HDR_SEQ,
	ICMP6HDR_MAXDELAY,
	ICMP6HDR_TADDR,
	ICMP6HDR_DADDR,
};

enum ip6_hdr_fields {
	IP6HDR_INVALID,
	IP6HDR_VERSION,
	IP6HDR_DSCP,
	IP6HDR_ECN,
	IP6HDR_FLOWLABEL,
	IP6HDR_LENGTH,
	IP6HDR_NEXTHDR,
	IP6HDR_HOPLIMIT,
	IP6HDR_SADDR,
	IP6HDR_DADDR,
	IP6HDR_PROTOCOL,
};

enum ah_hdr_fields {
	AHHDR_INVALID,
	AHHDR_NEXTHDR,
	AHHDR_HDRLENGTH,
	AHHDR_RESERVED,
	AHHDR_SPI,
	AHHDR_SEQUENCE,
};

enum esp_hdr_fields {
	ESPHDR_INVALID,
	ESPHDR_SPI,
	ESPHDR_SEQUENCE,
};

enum comp_hdr_fields {
	COMPHDR_INVALID,
	COMPHDR_NEXTHDR,
	COMPHDR_FLAGS,
	COMPHDR_CPI,
};

enum udp_hdr_fields {
	UDPHDR_INVALID,
	UDPHDR_SPORT,
	UDPHDR_DPORT,
	UDPHDR_LENGTH,
	UDPHDR_CSUMCOV = UDPHDR_LENGTH,
	UDPHDR_CHECKSUM,
};

enum tcp_hdr_fields {
	TCPHDR_INVALID,
	TCPHDR_UNSPEC = TCPHDR_INVALID,
	TCPHDR_SPORT,
	TCPHDR_DPORT,
	TCPHDR_SEQ,
	TCPHDR_ACKSEQ,
	TCPHDR_DOFF,
	TCPHDR_RESERVED,
	TCPHDR_FLAGS,
	TCPHDR_WINDOW,
	TCPHDR_CHECKSUM,
	TCPHDR_URGPTR,
};

enum dccp_hdr_fields {
	DCCPHDR_INVALID,
	DCCPHDR_SPORT,
	DCCPHDR_DPORT,
	DCCPHDR_TYPE,
};

enum sctp_hdr_fields {
	SCTPHDR_INVALID,
	SCTPHDR_SPORT,
	SCTPHDR_DPORT,
	SCTPHDR_VTAG,
	SCTPHDR_CHECKSUM,
};

enum th_hdr_fields {
	THDR_INVALID,
	THDR_SPORT,
	THDR_DPORT,
};

struct vxlanhdr {
	uint32_t vx_flags;
	uint32_t vx_vni;
};

enum vxlan_hdr_fields {
	VXLANHDR_INVALID,
	VXLANHDR_VNI,
	VXLANHDR_FLAGS,
};

struct gnvhdr {
	uint16_t flags;
	uint16_t type;
	uint32_t vni;
};
enum geneve_hdr_fields {
	GNVHDR_INVALID,
	GNVHDR_VNI,
	GNVHDR_TYPE,
};

struct grehdr {
	uint16_t flags;
	uint16_t protocol;
};

enum gre_hdr_fields {
	GREHDR_INVALID,
	GREHDR_VERSION,
	GREHDR_FLAGS,
	GREHDR_PROTOCOL,
};

extern const struct proto_desc proto_vxlan;
extern const struct proto_desc proto_geneve;
extern const struct proto_desc proto_gre;
extern const struct proto_desc proto_gretap;

extern const struct proto_desc proto_icmp;
extern const struct proto_desc proto_igmp;
extern const struct proto_desc proto_ah;
extern const struct proto_desc proto_esp;
extern const struct proto_desc proto_comp;
extern const struct proto_desc proto_udp;
extern const struct proto_desc proto_udplite;
extern const struct proto_desc proto_tcp;
extern const struct proto_desc proto_dccp;
extern const struct proto_desc proto_sctp;
extern const struct proto_desc proto_th;
extern const struct proto_desc proto_icmp6;

extern const struct proto_desc proto_ip;
extern const struct proto_desc proto_ip6;

extern const struct proto_desc proto_inet;
extern const struct proto_desc proto_inet_service;

extern const struct proto_desc proto_arp;

extern const struct proto_desc proto_vlan;
extern const struct proto_desc proto_eth;

extern const struct proto_desc proto_netdev;

extern const struct proto_desc proto_unknown;
extern const struct proto_hdr_template proto_unknown_template;

extern const struct datatype icmp_type_type;
extern const struct datatype tcp_flag_type;
extern const struct datatype dccp_pkttype_type;
extern const struct datatype arpop_type;
extern const struct datatype icmp6_type_type;
extern const struct datatype dscp_type;
extern const struct datatype ecn_type;

struct eval_ctx;
struct proto_ctx *eval_proto_ctx(struct eval_ctx *ctx);

#endif /* NFTABLES_PROTO_H */
