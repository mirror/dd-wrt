#ifndef NFTABLES_HEADERS_H
#define NFTABLES_HEADERS_H

#include <netinet/in.h>

#ifndef IPPROTO_UDPLITE
# define IPPROTO_UDPLITE	136
#endif

enum tcp_hdr_flags {
	TCP_FLAG_FIN	= 0x01,
	TCP_FLAG_SYN	= 0x02,
	TCP_FLAG_RST	= 0x04,
	TCP_FLAG_PSH	= 0x08,
	TCP_FLAG_ACK	= 0x10,
	TCP_FLAG_URG	= 0x20,
	TCP_FLAG_ECN	= 0x40,
	TCP_FLAG_CWR	= 0x80,
};

struct ip_auth_hdr {
	uint8_t 	nexthdr;
	uint8_t 	hdrlen;
	uint16_t	reserved;
	uint32_t	spi;
	uint32_t	seq_no;
};

struct ip_esp_hdr {
	uint32_t	spi;
	uint32_t	seq_no;
};

struct ip_comp_hdr {
	uint8_t		nexthdr;
	uint8_t		flags;
	uint16_t	cpi;
};

#ifndef IPPROTO_DCCP
# define IPPROTO_DCCP 33
#endif

enum dccp_pkt_type {
	DCCP_PKT_REQUEST = 0,
	DCCP_PKT_RESPONSE,
	DCCP_PKT_DATA,
	DCCP_PKT_ACK,
	DCCP_PKT_DATAACK,
	DCCP_PKT_CLOSEREQ,
	DCCP_PKT_CLOSE,
	DCCP_PKT_RESET,
	DCCP_PKT_SYNC,
	DCCP_PKT_SYNCACK,
	DCCP_PKT_INVALID,
};

struct dccp_hdr {
	uint16_t	dccph_sport,
			dccph_dport;
	uint8_t		dccph_doff;
	uint8_t		dccph_ccval:4,
			dccph_cscov:4;
	uint16_t	dccph_checksum;
	uint8_t		dccph_reserved:3,
			dccph_type:4,
			dccph_x:1;
	uint8_t		dccph_seq2;
	uint16_t	dccph_seq;
};

#ifndef IPPROTO_SCTP
# define IPPROTO_SCTP 132
#endif

struct sctphdr {
	uint16_t	source;
	uint16_t	dest;
	uint32_t	vtag;
	uint32_t	checksum;
};

struct arp_hdr {
	uint16_t	htype;
	uint16_t	ptype;
	uint8_t		hlen;
	uint8_t		plen;
	uint16_t	oper;
	uint8_t		sha[6];
	uint32_t	spa;
	uint8_t		tha[6];
	uint32_t	tpa;
} __attribute__((__packed__));

struct ipv6hdr {
	uint8_t		version:4,
			priority:4;
	uint8_t		flow_lbl[3];

	uint16_t	payload_len;
	uint8_t		nexthdr;
	uint8_t		hop_limit;

	struct in6_addr	saddr;
	struct in6_addr	daddr;
};

struct vlan_hdr {
	uint16_t	vlan_id:12,
			vlan_cfi:1,
			vlan_pcp:3;
	uint16_t	vlan_type;
};

#ifndef IPPROTO_MH
# define IPPROTO_MH 135
#endif

struct ip6_mh {
	uint8_t		ip6mh_proto;
	uint8_t		ip6mh_hdrlen;
	uint8_t		ip6mh_type;
	uint8_t		ip6mh_reserved;
	uint16_t	ip6mh_cksum;
	/* Followed by type specific messages */
	uint8_t		data[0];
};

/* Type 4 Routing header - well known as srh */
struct ip6_rt4 {
	uint8_t		ip6r4_nxt;		/* next header			*/
	uint8_t		ip6r4_len;		/* length in units of 8 octets	*/
	uint8_t		ip6r4_type;		/* always zero			*/
	uint8_t		ip6r4_segleft;		/* segments left		*/
	uint8_t		ip6r4_last_entry;	/* last entry			*/
	uint8_t		ip6r4_flags;		/* flags			*/
	uint16_t	ip6r4_tag;		/* tag				*/
	struct in6_addr	ip6r4_segments[0];	/* SID list			*/
};

/* RFC 3775 */
#define IP6_MH_TYPE_BRR		0	/* Binding Refresh Request	*/
#define IP6_MH_TYPE_HOTI	1	/* HOTI Message			*/
#define IP6_MH_TYPE_COTI	2	/* COTI Message			*/
#define IP6_MH_TYPE_HOT		3	/* HOT Message			*/
#define IP6_MH_TYPE_COT		4	/* COT Message			*/
#define IP6_MH_TYPE_BU		5	/* Binding Update		*/
#define IP6_MH_TYPE_BACK	6	/* Binding ACK			*/
#define IP6_MH_TYPE_BERROR	7	/* Binding Error		*/
/* RFC 4068 */
#define IP6_MH_TYPE_FBU		8	/* Fast Binding Update		*/
#define IP6_MH_TYPE_FBACK	9	/* Fast Binding ACK		*/
#define IP6_MH_TYPE_FNA		10	/* Fast Binding Advertisement	*/
/* RFC 5096 */
#define IP6_MH_TYPE_EMH		11	/* Experimental Mobility Header	*/
/* RFC 5142 */
#define IP6_MH_TYPE_HASM	12	/* Home Agent Switch Message	*/

#endif /* NFTABLES_HEADERS_H */
