#ifndef SRC_MOD_COMMON_PACKET_H_
#define SRC_MOD_COMMON_PACKET_H_

/**
 * @file
 * Random packet-related functions.
 *
 * Relevant topics:
 *
 * # Packet Buffering
 *
 * GRO, nf_defrag_ipv6 and nf_defrag_ipv4 can merge a bunch of related packets
 * on input, by buffering them in `skb_shinfo(skb)->frags` or queuing them in
 * `skb_shinfo(skb)->frag_list`. Lots of kernel functions will try to fool you
 * into thinking they're a single packet.
 *
 * For the most part, this is fine. Unfortunately, individual fragment surgery
 * is sometimes necessary evil for PMTU reasons. Therefore, you need to
 * understand frags and frag_list if you're going to manipulate lengths (and
 * sometimes checksums).
 *
 * # Internal Packets
 *
 * Packets contained inside ICMP errors. A good chunk of the RFC7915 code is
 * reused by external and internal packets.
 *
 * They can be truncated. When this happens, their header lengths will
 * contradict their actual lengths. For this reason, in general, Jool should
 * rarely rely on header lengths.
 *
 * # Local Glossary
 *
 * - data payload area: Bytes that lie in an skb between skb->head and
 *   skb->tail, excluding headers.
 * - paged area: Bytes the skb stores in skb_shinfo(skb)->frags.
 * - frag_list area: Bytes the skb stores in skb_shinfo(skb)->frag_list,
 *   and *also* the bytes these fragments store in their own paged areas.
 *
 * These are all L4 payload only. The kernel deletes frags and frag_list headers
 * on input, then recreates them on output.
 *
 * - Subsequent fragment: Packet with fragment offset nonzero.
 *   (These only show up when nf_defrag_ipv* is disabled; ie. stateless
 *   translators only.)
 */

#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/tcp.h>
#include <linux/icmp.h>

#include "mod/common/types.h"


/** Returns a hack-free version of the 'Traffic class' field from @hdr. */
static inline __u8 get_traffic_class(const struct ipv6hdr *hdr)
{
	__u8 upper_bits = hdr->priority;
	__u8 lower_bits = hdr->flow_lbl[0] >> 4;
	return (upper_bits << 4) | lower_bits;
}

/**
 * Returns a big endian (but otherwise hack-free) version of the 'Flow label'
 * field from @hdr.
 */
static inline __be32 get_flow_label(const struct ipv6hdr *hdr)
{
	return (*(__be32 *) hdr) & IPV6_FLOWLABEL_MASK;
}

/** Returns IP_DF if the DF flag from @hdr is set, 0 otherwise. */
static inline __u16 is_df_set(const struct iphdr *hdr)
{
	return be16_to_cpu(hdr->frag_off) & IP_DF;
}

/** Returns IP6_MF if the MF flag from @hdr is set, 0 otherwise. */
static inline __u16 is_mf_set_ipv6(const struct frag_hdr *hdr)
{
	return be16_to_cpu(hdr->frag_off) & IP6_MF;
}

/** Returns IP_MF if the MF flag from @hdr is set, 0 otherwise. */
static inline __u16 is_mf_set_ipv4(const struct iphdr *hdr)
{
	return be16_to_cpu(hdr->frag_off) & IP_MF;
}

/** Returns a hack-free version of the 'Fragment offset' field from @hdr. */
static inline __u16 get_fragment_offset_ipv6(const struct frag_hdr *hdr)
{
	return be16_to_cpu(hdr->frag_off) & 0xFFF8U;
}

/** Returns a hack-free version of the 'Fragment offset' field from @hdr. */
static inline __u16 get_fragment_offset_ipv4(const struct iphdr *hdr)
{
	__u16 frag_off = be16_to_cpu(hdr->frag_off);
	/* 3 bit shifts to the left == multiplication by 8. */
	return (frag_off & IP_OFFSET) << 3;
}

/**
 * Pretends @skb's IPv6 header has a "total length" field and returns its value.
 */
static inline unsigned int get_tot_len_ipv6(const struct sk_buff *skb)
{
	return sizeof(struct ipv6hdr) + be16_to_cpu(ipv6_hdr(skb)->payload_len);
}

/**
 * @{
 * Does @hdr belong to a "first fragment"?
 * A non-fragmented packet is also considered a first fragment.
 */
static inline bool is_first_frag4(const struct iphdr *hdr)
{
	return get_fragment_offset_ipv4(hdr) == 0;
}

static inline bool is_first_frag6(const struct frag_hdr *hdr)
{
	return hdr ? (get_fragment_offset_ipv6(hdr) == 0) : true;
}
/**
 * @}
 */

/**
 * @{
 * Is @hdr's packet a fragment?
 */
static inline bool is_fragmented_ipv4(const struct iphdr *hdr)
{
	return ip_is_fragment(hdr);
}

static inline bool is_fragmented_ipv6(const struct frag_hdr *hdr)
{
	if (!hdr)
		return false;
	return (get_fragment_offset_ipv6(hdr) != 0) || is_mf_set_ipv6(hdr);
}
/**
 * @}
 */

/**
 * frag_hdr.frag_off is actually a combination of the 'More fragments' flag and
 * the 'Fragment offset' field. This function is a one-liner for creating a
 * settable frag_off.
 * Note that fragment offset is measured in units of eight-byte blocks. That
 * means that you want @frag_offset to be a multiple of 8 if you want your
 * fragmentation to work properly.
 */
static inline __be16 build_ipv6_frag_off_field(__u16 frag_offset, __u16 mf)
{
	__u16 result = (frag_offset & 0xFFF8U) | (mf ? 1U : 0U);
	return cpu_to_be16(result);
}

/**
 * iphdr.frag_off is actually a combination of the DF flag, the MF flag and the
 * 'Fragment offset' field. This function is a one-liner for creating a settable
 * frag_off.
 * Note that fragment offset is measured in units of eight-byte blocks. That
 * means that you want @frag_offset to be a multiple of 8 if you want your
 * fragmentation to work properly.
 */
static inline __be16 build_ipv4_frag_off_field(const bool df, const __u16 mf,
		const __u16 frag_offset)
{
	__u16 result = (df ? (1U << 14) : 0)
			| (mf ? (1U << 13) : 0)
			/* 3 bit shifts to the right == division by 8. */
			| (frag_offset >> 3);
	return cpu_to_be16(result);
}

/**
 * Returns the size in bytes of @hdr, including options.
 * skbless variant of tcp_hdrlen().
 */
static inline unsigned int tcp_hdr_len(const struct tcphdr *hdr)
{
	return hdr->doff << 2;
}

/**
 * We need to store packet metadata, so we encapsulate sk_buffs into this.
 *
 * Do **not** use control buffers (skb->cb) for this purpose. The kernel is
 * known to misbehave and store information there which we should not override.
 */
struct packet {
	struct sk_buff *skb;
	struct tuple tuple;

	/**
	 * Protocol of the layer-3 header of the packet.
	 * Yes, skb->proto has the same superpowers, but it's a little
	 * unreliable (it's not set in the Local Out chain).
	 * Also this spares me a switch in pkt_l3_proto() :p.
	 */
	enum l3_protocol l3_proto;
	/**
	 * Protocol of the layer-4 header of the packet. To the best of my
	 * knowledge, the kernel also uses skb->proto for this, but only on
	 * layer-4 code.
	 */
	enum l4_protocol l4_proto;
	/** Is this a subpacket, contained in an ICMP error? */
	bool is_inner;

	/** Offset of the skb's fragment header (from skb->data), if any. */
	unsigned int frag_offset;
	/**
	 * Offset of the packet's payload. (From skb->data.)
	 * Because skbs only store pointers to headers.
	 *
	 * Sometimes the kernel seems to use skb->data for this. It would be
	 * troublesome if we did the same, however, since functions such as
	 * icmp_send() fail early when skb->data is after the layer-3 header.
	 *
	 * Note, even after the packet is validated, the payload can be paged
	 * (unlike headers). Do not access the data pointed by this field
	 * carelessly.
	 */
	unsigned int payload_offset;
	/**
	 * If this is an incoming packet (as in, incoming to Jool), this points
	 * to the same packet (pkt->original_pkt = pkt). Otherwise (which
	 * includes hairpin packets), this points to the original (incoming)
	 * packet.
	 */
	struct packet *original_pkt;
};

/**
 * Initializes @pkt using the rest of the arguments.
 */
static inline void pkt_fill(struct packet *pkt, struct sk_buff *skb,
		l3_protocol l3_proto, l4_protocol l4_proto,
		struct frag_hdr *frag, void *payload,
		struct packet *original_pkt)
{
	pkt->skb = skb;
	pkt->l3_proto = l3_proto;
	pkt->l4_proto = l4_proto;
	pkt->is_inner = 0;
	/* pkt->is_hairpin = false; */
	pkt->frag_offset = frag ? ((unsigned char *)frag - skb->data) : 0;
	pkt->payload_offset = (unsigned char *)payload - skb->data;
	pkt->original_pkt = original_pkt;
}

static inline l3_protocol pkt_l3_proto(const struct packet *pkt)
{
	return pkt->l3_proto;
}

/* l3_proto must be IPv4. */
static inline struct iphdr *pkt_ip4_hdr(const struct packet *pkt)
{
	return ip_hdr(pkt->skb);
}

/* l3_proto must be IPv6. */
static inline struct ipv6hdr *pkt_ip6_hdr(const struct packet *pkt)
{
	return ipv6_hdr(pkt->skb);
}

static inline l4_protocol pkt_l4_proto(const struct packet *pkt)
{
	return pkt->l4_proto;
}

/* Incompatible with subsequent fragments, l4_proto must be TCP. */
static inline struct udphdr *pkt_udp_hdr(const struct packet *pkt)
{
	return udp_hdr(pkt->skb);
}

/* Incompatible with subsequent fragments, l4_proto must be UDP. */
static inline struct tcphdr *pkt_tcp_hdr(const struct packet *pkt)
{
	return tcp_hdr(pkt->skb);
}

/* l4_proto must be ICMP. */
static inline struct icmphdr *pkt_icmp4_hdr(const struct packet *pkt)
{
	return icmp_hdr(pkt->skb);
}

/* l4_proto must be ICMP. */
static inline struct icmp6hdr *pkt_icmp6_hdr(const struct packet *pkt)
{
	return icmp6_hdr(pkt->skb);
}

/* l3_proto must be IPv6. */
static inline struct frag_hdr *pkt_frag_hdr(const struct packet *pkt)
{
	if (!pkt->frag_offset)
		return NULL;
	return (struct frag_hdr *)(pkt->skb->data + pkt->frag_offset);
}

static inline void *pkt_payload(const struct packet *pkt)
{
	return pkt->skb->data + pkt->payload_offset;
}

static inline bool pkt_is_inner(const struct packet *pkt)
{
	return pkt->is_inner;
}

static inline bool pkt_is_outer(const struct packet *pkt)
{
	return !pkt_is_inner(pkt);
}

static inline struct packet *pkt_original_pkt(const struct packet *pkt)
{
	return pkt->original_pkt;
}

/**
 * Returns the length of @pkt's first set of layer-3 headers (including options
 * and extension headers).
 * Counts neither frag_list headers, frag headers nor ICMP error inner headers.
 *
 * Compatible with fragments.
 *
 * Includes l3 header.
 * Does not include l4 header, data payload area, paged area nor frag_list area.
 */
static inline unsigned int pkt_l3hdr_len(const struct packet *pkt)
{
	return skb_transport_header(pkt->skb) - skb_network_header(pkt->skb);
}

/**
 * Returns the length of @pkt's first set of layer-4 headers (including
 * options).
 * Counts neither frag headers nor ICMP error inner headers.
 *
 * Compatible with fragments. (Returns 0 on subsequent fragments.)
 *
 * Includes l4 header.
 * Does not include l3 header, data payload area, paged area nor frag_list area.
 */
static inline unsigned int pkt_l4hdr_len(const struct packet *pkt)
{
	return pkt_payload(pkt) - (void *)skb_transport_header(pkt->skb);
}

/**
 * Returns the length of @pkt's first set of layer-3 and layer-4 headers.
 * Counts neither frag_list headers, frag headers nor ICMP error inner headers.
 *
 * Compatible with fragments.
 *
 * Includes l3 header and l4 header.
 * Does not include data payload area, paged area nor frag_list area.
 */
static inline unsigned int pkt_hdrs_len(const struct packet *pkt)
{
	return pkt->payload_offset;
}

/**
 * Returns the length of @pkt's layer-3 payload.
 * Includes headroom payload, frag_list payload and frags payload.
 *
 * Technically (but not semantically) compatible with fragments. (There is no
 * "datagram" in a fragment.) If you want to use this function outside of the
 * context of checksum computation, please update this comment.
 *
 * Includes l4 header, data payload area, paged area and frag_list area.
 * Does not include l3 header.
 */
static inline unsigned int pkt_datagram_len(const struct packet *pkt)
{
	return pkt->skb->len - pkt_l3hdr_len(pkt);
}

static inline bool pkt_is_icmp6_error(const struct packet *pkt)
{
	return pkt_l4_proto(pkt) == L4PROTO_ICMP /* Implies "not subsequent" */
			&& is_icmp6_error(pkt_icmp6_hdr(pkt)->icmp6_type);
}

static inline bool pkt_is_icmp4_error(const struct packet *pkt)
{
	return pkt_l4_proto(pkt) == L4PROTO_ICMP /* Implies "not subsequent" */
			&& is_icmp4_error(pkt_icmp4_hdr(pkt)->type);
}

struct xlation;

/**
 * Ensures @skb isn't corrupted and initializes @state->in out of it.
 *
 * After this function, code can assume:
 * - @skb contains full l3 and l4 headers (including inner ones), their order
 *   seems to make sense, and they are all within the data area of @skb. (ie.
 *   they are not paged.)
 * - @skb's payload isn't truncated (though inner packet payload might).
 * - The pkt_* functions above can now be used on @state->in.
 * - The length fields in the l3 headers can be relied upon. (But not the ones
 *   contained in inner packets.)
 *
 * Healthy layer 4 checksums and lengths are not guaranteed, but that's not an
 * issue since this kind of corruption should be translated along (see
 * validate_icmp6_csum()).
 *
 * Also, this function does not ensure @skb is either TCP, UDP or ICMP. This is
 * because SIIT Jool must translate other protocols in a best-effort basis.
 *
 * This function can change the packet's pointers. If you eg. stored a pointer
 * to skb_network_header(skb), you will need to assign it again (by calling
 * skb_network_header() again).
 */
verdict pkt_init_ipv6(struct xlation *state, struct sk_buff *skb);
verdict pkt_init_ipv4(struct xlation *state, struct sk_buff *skb);
/**
 * @}
 */

unsigned char *jskb_pull(struct sk_buff *skb, unsigned int len);
unsigned char *jskb_push(struct sk_buff *skb, unsigned int len);

#endif /* SRC_MOD_COMMON_PACKET_H_ */
