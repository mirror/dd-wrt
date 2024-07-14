/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HYFI_HASH_H_
#define	_HYFI_HASH_H_

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/dsfield.h>
#include "hyfi_api.h"

/* Convenience */
#ifndef	HASHINIT
#define	HASHINIT	5381
#define	HASHSTEP(x,c)	(((x << 5) + x) + (c))
#endif

#define HYBRID_HASH_TCPUDP_PORT 1

#define HASH_POLICY_1 1  /* default, different order -> different hash */
#define HASH_POLICY_2 2  /* different order -> same hash */

/* packet type and priority defines */
#define IS_IPPROTO_UDP       (1 << 0)
#define IS_IPPROTO_TCP       (1 << 1)
#define IS_HYFI_PKT          (1 << 2)
#define IS_HYFI_IP_PKT       (1 << 3)
#define IS_HYFI_AGGR_FLOW    (1 << 4)
#define DSCP_SHIFT           2
#define TCI_PRIORITY_SHIFT  13

#ifndef HYFI_HYFI_HACTIVE_TBL_H_
#define HYFI_HACTIVE_TBL_PRIORITY_DSCP_VALID (1 << 31)
#define HYFI_HACTIVE_TBL_PRIORITY_DSCP_MASK  0x000001F8
#define HYFI_HACTIVE_TBL_PRIORITY_8021_VALID (1 << 30)
#define HYFI_HACTIVE_TBL_PRIORITY_8021_MASK  0x00000007
#endif

#ifndef IPPROTO_ETHERIP
#define IPPROTO_ETHERIP (97)
#endif

/*
 * Return a 32-bit hash of the given buffer.  The init
 * value should be 0, or the previous hash value to extend
 * the previous hash.
 */
static __inline uint32_t hash32_buf(uint32_t v1, uint32_t v2, uint32_t hash)
{
	return (HASHSTEP(hash, (v1 ^ v2)));
}

static inline void hyfi_set_8021_priority(u_int32_t* priority,
		u_int32_t vlan_tag)
{
	*priority |= HYFI_HACTIVE_TBL_PRIORITY_8021_VALID
			| (vlan_tag >> TCI_PRIORITY_SHIFT);
}

static inline void hyfi_set_dscp_priority(u_int32_t* priority, u_int32_t tos)
{
	*priority |= HYFI_HACTIVE_TBL_PRIORITY_DSCP_VALID;
	*priority |= (tos << 1) & HYFI_HACTIVE_TBL_PRIORITY_DSCP_MASK;
}

/**
 * @brief Calculate the hash from a skb 
 *  
 * @note This function and the one below it 
 *       (hyfi_hash_skbuf_reverse) must be kept in sync.  Both
 *       must use the same hash function to calculate the hash,
 *       so if one is changed, the other must be too.  The
 *       reverse function was added for NSS to be able to get
 *       the hash for flows in both the forward and reverse
 *       direction from a single skb.  This function was not
 *       modified to calculate both forward and reverse hashes
 *       because it is used in the data path for unaccelerated
 *       flows, and performance is critical.
 * 
 * @param [in] skb  skb to calculate the hash from
 * @param [out] hash  calculated hash
 * @param [out] flag  flags describing the packet
 * @param [out] priority  priority of the packet
 * @param [out] seq  sequence
 * 
 * @return 0 on success, negative value on error
 */
static inline int hyfi_hash_skbuf(struct sk_buff *skb, u_int32_t *hash,
		u_int32_t *flag, u_int32_t *priority, u_int16_t *seq)
{
	u_int16_t etype = 0, vlan_tag = 0;
	u_int32_t p = 0;
	int retval = 0;
	struct ethhdr *eh;
#ifdef CONFIG_INET
	const struct iphdr *ip = NULL;
	struct udphdr *udp = NULL;
	struct tcphdr *tcp = NULL;
#endif
#ifdef CONFIG_IPV6
	struct ipv6hdr *ip6 = NULL;
	u_int32_t flow;
#endif

	do {
		if (unlikely(skb->len <= 0 || !skb_mac_header_was_set(skb))) {
			retval = -EIO;
			break;
		}

		eh = eth_hdr(skb);
		etype = ntohs(eh->h_proto);

		/* Special handling for encapsulating VLAN frames */
		*priority = 0;
		*flag = 0;
		if (skb->protocol == htons(ETH_P_8021Q)) {
			skb_push(skb, ETH_HLEN);
			if (vlan_get_tag(skb, &vlan_tag) < 0) {
				skb_pull(skb, ETH_HLEN);
				retval = -EFAULT;
				break;
			}
			skb_pull(skb, ETH_HLEN);
			hyfi_set_8021_priority(priority, vlan_tag);

			etype = vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
		}

		switch (etype) {
#ifdef CONFIG_INET
		case ETH_P_IP:
			ip = ip_hdr(skb);
			if (ip == NULL ) {
				retval = -EFAULT;
				break;
			}
			hyfi_set_dscp_priority(priority, ipv4_get_dsfield(ip));

			if (ip->ihl > 5) {
				struct ip_opts {
					u_int8_t type;
					u_int8_t length;
					u_int16_t data;
				}__attribute__((packed));

				struct ip_opts *ip_opts = (struct ip_opts *) ((char *) ip
						+ (ip->ihl - 1) * 4);

				if (ip_opts->type == 0x88 && ip_opts->length == 4) {
					*flag |= IS_HYFI_AGGR_FLOW;
					*seq = ip_opts->data;
				}
			}

#ifdef HYBRID_HASH_TCPUDP_PORT
			if (ip->protocol == IPPROTO_UDP) {
				udp = (struct udphdr *) (skb_network_header(skb)
						+ ip_hdrlen(skb));
				*flag |= IS_IPPROTO_UDP;
			} else if (ip->protocol == IPPROTO_TCP) {
				tcp = (struct tcphdr *) (skb_network_header(skb)
						+ ip_hdrlen(skb));
				*flag |= IS_IPPROTO_TCP;
			}
#else
			if (ip->protocol == IPPROTO_UDP) *flag |= IS_IPPROTO_UDP;
			if (ip->protocol == IPPROTO_TCP) *flag |= IS_IPPROTO_TCP;
#endif
			if (ip->protocol == IPPROTO_ETHERIP) {
				struct ethhdr *encap_eh = (struct ethhdr *) (skb_network_header(
						skb) + ip_hdrlen(skb) + sizeof(u_int16_t));

				if (htons(encap_eh->h_proto) == 0x88b7)
					*flag |= IS_HYFI_IP_PKT;
			}
			break;
#endif
#ifdef CONFIG_IPV6
		case ETH_P_IPV6:
			ip6 = ipv6_hdr(skb);
			if (ip6 == NULL) {
				retval = -EFAULT;
				break;
			}
			hyfi_set_dscp_priority(priority, ipv6_get_dsfield(ip6));
			break;
#endif
		case 0x88b7:
			*flag |= IS_HYFI_PKT;
			break;

		default:
			break;
		}
		if (retval != 0)
			break;

		/* Ethernet address */
		{
			u_int32_t key1 = get_unaligned( (u_int32_t *) ( &eh->h_dest[2] ) );
			u_int32_t key2 = get_unaligned( (u_int32_t *) ( &eh->h_source[2] ) );
			p = hash32_buf(key1, key2, 0);
		}

		/* VLAN tag */
		if (vlan_tag)
			p = hash32_buf(vlan_tag, 0, p);

#ifdef CONFIG_INET
		/* IP Address */
		if (likely(ip)) {
			/* Protocol */
			p = hash32_buf(ip->protocol, 0, p);

			/* To reduce the chance of collisions with correlated port
			 * numbers, mix the destination IP and port together first
			 * and then the source IP and port.
			 */
#ifdef HYBRID_HASH_TCPUDP_PORT
			/* UDP port */
			if (udp && !(ip->frag_off & htons(IP_MF | IP_OFFSET))) {
				p = hash32_buf(ip->daddr, udp->dest, p);
				p = hash32_buf(ip->saddr, udp->source, p);
			}
			/* TCP port */
			else if (tcp && !(ip->frag_off & htons(IP_MF | IP_OFFSET))) {
				p = hash32_buf(ip->daddr, tcp->dest, p);
				p = hash32_buf(ip->saddr, tcp->source, p);
			}
			/* Not TCP nor UDP or is a fragment */
			else {
#endif
				p = hash32_buf(ip->daddr, ip->saddr, p);
#ifdef HYBRID_HASH_TCPUDP_PORT
			}
#endif
		}
#endif /* CONFIG_INET */

#ifdef CONFIG_IPV6
		/* Todo: If there are extension headers in the packet, we would have to process those first.
		 * The last extension header's NH field will indicate the transport layer protocol.
		 */
		if (ip6) {
			u_int32_t key1 = ip6->saddr.s6_addr32[2] ^ ip6->saddr.s6_addr32[3];
			u_int32_t key2 = ip6->daddr.s6_addr32[2] ^ ip6->daddr.s6_addr32[3];
			p = hash32_buf(key1, key2, p);
			/* IPv6 flow label */
			flow = *(u_int32_t*)ip6;
			p = hash32_buf(flow, 0, p);
		}
#endif

		p ^= (p >> 16);
		p ^= (p >> 8);
		*hash = p & 0xff;
	} while (0);

	return (retval);
}

/**
 * @brief Calculate the hash for a skb as if the packet was 
 *        received in the reverse direction (ie. all addresses
 *        and ports are reversed)
 *  
 * @see hyfi_hash_skbuf 
 * 
 * @param [in] skb  skb to calculate the hash from
 * @param [out] hash  calculated hash (from reversed version of 
 *                    skb)
 *  
 * @return 0 on success, negative value on error
 */
static inline int hyfi_hash_skbuf_reverse(struct sk_buff *skb, u_int32_t *hash)
{
	u_int16_t etype = 0, vlan_tag = 0;
	u_int32_t p = 0;
	int retval = 0;
	struct ethhdr *eh;
#ifdef CONFIG_INET
	const struct iphdr *ip = NULL;
	struct udphdr *udp = NULL;
	struct tcphdr *tcp = NULL;
#endif
#ifdef CONFIG_IPV6
	struct ipv6hdr *ip6 = NULL;
	u_int32_t flow;
#endif

	do {
		if (unlikely(skb->len <= 0 || !skb_mac_header_was_set(skb))) {
			retval = -EIO;
			break;
		}

		eh = eth_hdr(skb);
		etype = ntohs(eh->h_proto);

		/* Special handling for encapsulating VLAN frames */
		if (skb->protocol == htons(ETH_P_8021Q)) {
			skb_push(skb, ETH_HLEN);
			if (vlan_get_tag(skb, &vlan_tag) < 0) {
				skb_pull(skb, ETH_HLEN);
				retval = -EFAULT;
				break;
			}
			skb_pull(skb, ETH_HLEN);

			etype = vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
		}

		switch (etype) {
#ifdef CONFIG_INET
		case ETH_P_IP:
			ip = ip_hdr(skb);
			if (ip == NULL ) {
				retval = -EFAULT;
				break;
			}
#ifdef HYBRID_HASH_TCPUDP_PORT
			if (ip->protocol == IPPROTO_UDP) {
				udp = (struct udphdr *) (skb_network_header(skb)
						+ ip_hdrlen(skb));
			} else if (ip->protocol == IPPROTO_TCP) {
				tcp = (struct tcphdr *) (skb_network_header(skb)
						+ ip_hdrlen(skb));
			}
#endif
			break;
#endif
#ifdef CONFIG_IPV6
		case ETH_P_IPV6:
			ip6 = ipv6_hdr(skb);
			if (ip6 == NULL) {
				retval = -EFAULT;
				break;
			}
			break;
#endif
		default:
			break;
		}
		if (retval != 0)
			break;

		/* Ethernet address */
		{
			u_int32_t key1 = get_unaligned( (u_int32_t *) ( &eh->h_source[2] ) );
			u_int32_t key2 = get_unaligned( (u_int32_t *) ( &eh->h_dest[2] ) );
			p = hash32_buf(key1, key2, 0);
		}

		/* VLAN tag */
		if (vlan_tag)
			p = hash32_buf(vlan_tag, 0, p);

#ifdef CONFIG_INET
		/* IP Address */
		if (likely(ip)) {
			/* Protocol */
			p = hash32_buf(ip->protocol, 0, p);

			/* To reduce the chance of collisions with correlated port
			 * numbers, mix the source IP and port together first
			 * and then the destination IP and port.
			 */
#ifdef HYBRID_HASH_TCPUDP_PORT
			/* UDP port */
			if (udp && !(ip->frag_off & htons(IP_MF | IP_OFFSET))) {
				p = hash32_buf(ip->saddr, udp->source, p);
				p = hash32_buf(ip->daddr, udp->dest, p);
			}
			/* TCP port */
			else if (tcp && !(ip->frag_off & htons(IP_MF | IP_OFFSET))) {
				p = hash32_buf(ip->saddr, tcp->source, p);
				p = hash32_buf(ip->daddr, tcp->dest, p);
			}
			/* Not TCP nor UDP or is a fragment */
			else {
#endif
				p = hash32_buf(ip->saddr, ip->daddr, p);
#ifdef HYBRID_HASH_TCPUDP_PORT
			}
#endif
		}
#endif /* CONFIG_INET */

#ifdef CONFIG_IPV6
		/* Todo: If there are extension headers in the packet, we would have to process those first.
		 * The last extension header's NH field will indicate the transport layer protocol.
		 */
		if (ip6) {
			u_int32_t key1 = ip6->daddr.s6_addr32[2] ^ ip6->daddr.s6_addr32[3];
			u_int32_t key2 = ip6->saddr.s6_addr32[2] ^ ip6->saddr.s6_addr32[3];
			p = hash32_buf(key1, key2, p);
			/* IPv6 flow label */
			flow = *(u_int32_t*)ip6;
			p = hash32_buf(flow, 0, p);
		}
#endif

		p ^= (p >> 16);
		p ^= (p >> 8);
		*hash = p & 0xff;
	} while (0);

	return (retval);
}

#endif /* !_HYFI_HASH_H_ */
