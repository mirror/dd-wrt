/*
 **************************************************************************
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_connnmgr_map_t.c
 *
 * This file implements NSS map-t client module
 */
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/of.h>
#include <net/ipv6.h>
#include <linux/rwlock_types.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/version.h>
#include <linux/list_sort.h>
#include <linux/ipv6.h>
#include <linux/debugfs.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include "nss_connmgr_map_t.h"
#include <nat46-core.h>
#include <nat46-netdev.h>

/*
 * NSS map_t debug macros
 */
#if (NSS_MAP_T_DEBUG_LEVEL < 1)
#define nss_connmgr_map_t_assert(fmt, args...)
#else
#define nss_connmgr_map_t_assert(c) BUG_ON(!(c));
#endif

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_connmgr_map_t_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_map_t_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_connmgr_map_t_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_MAP_T_DEBUG_LEVEL < 2)
#define nss_connmgr_map_t_warning(s, ...)
#else
#define nss_connmgr_map_t_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_MAP_T_DEBUG_LEVEL < 3)
#define nss_connmgr_map_t_info(s, ...)
#else
#define nss_connmgr_map_t_info(s, ...) pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_MAP_T_DEBUG_LEVEL < 4)
#define nss_connmgr_map_t_trace(s, ...)
#else
#define nss_connmgr_map_t_trace(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * Format for header: "netdevice = eth0\n"
 */
#define MAP_T_NETDEVICE_SZ (strlen("netdevice = ") + IFNAMSIZ + 1)

/*
 * Format for a rule: <11 digits decmal> + ": 0x" + <16 hex digits for long long> + newline
 */
#define MAP_T_INT_TO_DEC 11
#define MAP_T_LONG_LONG_TO_HEX 16
#define MAP_T_RULE_SZ ((MAP_T_INT_TO_DEC) + (MAP_T_LONG_LONG_TO_HEX) + 5)

#define MAP_T_IPV6_CLASS_SHIFT 20
#define MAP_T_IPV6_CLASS_MASK  0x0FF00000

/*
 * Max map-t interfaces supported = NSS_MAX_MAP_T_DYNAMIC_INTERFACES
 */
static int mapt_interfaces_count;

/*
 * MAP-T flags.
 */
static uint8_t map_t_flags;

/*
 * client code check for correctness of rule. This debug stats helps in
 * to see those stats
 */
static struct dentry *map_t_debugfs;

/*
 *      net_device1--------->net_device_2--------->net_device_3
 *		|		|			|
 *		|		|			|
 *		|		|			|
 *	     rule_set	     rule_set		      rule_set
 *
 */
static LIST_HEAD(list_dev_to_map_t_rules_head);

/*
 * nss_connmgr_map_t_dump_rules()
 *	dumps the rule list. Should only call this func from nss_connmgr_map_t_dev_up()
 *
 * This function dumps map-t rules read from nat-46 module. This function is
 * only for debug purpose.
 */
static void __maybe_unused nss_connmgr_map_t_dump_rules(struct net_device *dev __maybe_unused, struct nat46_xlate_rulepair *apair __maybe_unused, int count)
{
	const char *style_to_string[] __maybe_unused = {"NONE", "MAP", "MAP-0", "RFC6052"};
	int i = 0;

	if (!apair) {
		return;
	}

	for (i = 0; i < count; i++) {
		nss_connmgr_map_t_info("%p: local.v4 %pI4/%d local.v6 %pI6c/%d local.style %s local.ea-len %d local.psid-offset %d remote.v4 %pI4/%d remote.v6 %pI6c/%d remote.style %s remote.ea-len %d remote.psid-offset %d\n", dev,
			 &apair->local.v4_pref,
			 apair->local.v4_pref_len,
			 &apair->local.v6_pref,
			 apair->local.v6_pref_len,
			 style_to_string[apair->local.style],
			 apair->local.ea_len,
			 apair->local.psid_offset,

			 &apair->remote.v4_pref,
			 apair->remote.v4_pref_len,
			 &apair->remote.v6_pref,
			 apair->remote.v6_pref_len,
			 style_to_string[apair->remote.style],
			 apair->remote.ea_len,
			 apair->remote.psid_offset);

		apair++;
	}
}

/*
 * nss_connmgr_map_t_dump_list()
 *	dump sorted list. Should only call this func from nss_connmgr_map_t_dev_up()
 *
 * client module will sort map-t rules based on local ipv4 prefix len, remote
 * ipv4 prefix len, local ipv6 prefix len, remote ipv6 prefix len. This function
 * can dump those sorted list. This function is only for debug purpose.
 * We may call this function once sort is done.
 */
static void __maybe_unused nss_connmgr_map_t_dump_list(struct net_device *dev __maybe_unused, struct list_head *head)
{
	const char *style_to_string[] __maybe_unused = {"NONE", "MAP", "MAP-0", "RFC6052"};
	struct list_lookup_entry_t *entry;
	list_for_each_entry(entry, head, list) {
		struct nat46_xlate_rulepair *apair __maybe_unused = entry->ptr_rule_set;
		nss_connmgr_map_t_info("%p: local.v4 %pI4/%d local.v6 %pI6c/%d local.style %s local.ea-len %d local.psid-offset %d remote.v4 %pI4/%d remote.v6 %pI6c/%d remote.style %s remote.ea-len %d remote.psid-offset %d\n", dev,
			&apair->local.v4_pref,
			apair->local.v4_pref_len,
			&apair->local.v6_pref,
			apair->local.v6_pref_len,
			style_to_string[apair->local.style],
			apair->local.ea_len,
			apair->local.psid_offset,

			&apair->remote.v4_pref,
			apair->remote.v4_pref_len,
			&apair->remote.v6_pref,
			apair->remote.v6_pref_len,
			style_to_string[apair->remote.style],
			apair->remote.ea_len,
			apair->remote.psid_offset);
	}
}

/*
 * nss_connmgr_map_t_debugfs_set_rule_status()
 */
static void nss_connmgr_map_t_debugfs_set_rule_status(struct net_device *dev, int rule_num, uint64_t status)
{
	struct list_dev_to_map_t_rules_entry_t *entry;

	if (rule_num < 1) {
		return;
	}

	rule_num--;

	list_for_each_entry(entry, &list_dev_to_map_t_rules_head, list) {
		if (entry->dev != dev) {
			continue;
		}
		entry->rule_status[rule_num] = status;
		return;
	}
}

/*
 * nss_connmgr_map_t_free_all()
 *	Free up all memory allocated for linked list
 */
static void nss_connmgr_map_t_free_all(struct net_device *dev)
{
	struct list_dev_to_map_t_rules_entry_t *entry, *tmp;

	list_for_each_entry_safe(entry, tmp, &list_dev_to_map_t_rules_head, list) {

		if (entry->dev != dev) {
			continue;
		}

		kfree(entry->rule_status);

		list_del(&entry->list);
		kfree(entry);
	}
}

/*
 * nss_connmgr_map_t_allocate_all()
 *	allocate all data structures
 */
static void nss_connmgr_map_t_allocate_all(struct net_device *dev, struct nat46_xlate_rulepair *rule_pairs, int rule_pair_count)
{
	struct list_dev_to_map_t_rules_entry_t *entry;

	entry = (struct list_dev_to_map_t_rules_entry_t *)
					kmalloc(sizeof(struct list_dev_to_map_t_rules_entry_t), GFP_KERNEL);
	if (!entry) {
		nss_connmgr_map_t_warning("%p: Allocation for dev_to_map_t_rules_list_entry failed for netdev = %s\n", dev, dev->name);
		nss_connmgr_map_t_free_all(dev);
		return;
	}

	/*
	 * Add to map-t netdevice list
	 */
	INIT_LIST_HEAD(&entry->list);

	entry->dev = dev;
	entry->rule_set = rule_pairs;

	list_add(&entry->list, &list_dev_to_map_t_rules_head);

	entry->rule_pair_count = rule_pair_count;

	/*
	 * allocate memory for rule validation status
	 */
	entry->rule_status = kzalloc(sizeof(uint64_t) * rule_pair_count, GFP_KERNEL);

	if (!entry->rule_status) {
		nss_connmgr_map_t_free_all(dev);
	}

}

/*
 * nss_connmgr_mapt_validate_rule_style_mapt()
 *	Validate map-t style rule
 */
static bool nss_connmgr_mapt_validate_rule_style_mapt(struct net_device *dev, struct nat46_xlate_rule *rule, int rule_num,
						       bool is_local_rule, uint64_t *stats)
{
	int psid_len;

	/*
	 * Validate rule parameters
	 */
	if (rule->ea_len < 0 || rule->ea_len > 48) {
		nss_connmgr_map_t_warning("%p: mapt rule %d is invalid as ea_len < 0 or ea_len > 48\n", dev, rule_num);
		*stats |= 1 << (is_local_rule ? MAP_T_LOCAL_EA_BITS_LEN_IS_INVALID : MAP_T_REMOTE_EA_BITS_LEN_IS_INVALID);
		return false;
	}

	if (rule->v4_pref_len + rule->ea_len > 32) {
		psid_len = rule->ea_len - (32 - rule->v4_pref_len);
	} else {
		psid_len = 0;
	}

	if (psid_len + rule->psid_offset > 16) {
		nss_connmgr_map_t_warning("%p: mapt rule %d is invalid as psid offset + psid len > 16\n", dev, rule_num);
		*stats |= 1 << (is_local_rule ? MAP_T_LOCAL_PSID_LEN_PLUS_PSID_OFFSET_IS_GREATER_THAN_16 : MAP_T_REMOTE_PSID_LEN_PLUS_REMOTE_PSID_OFFSET_IS_GREATER_THAN_16);
		return false;
	}

	return true;
}

/*
 * nss_connmgr_mapt_validate_rule_style_rfc6052()
 *	Validate map-t style rule
 */
static bool nss_connmgr_mapt_validate_rule_style_rfc6052(struct net_device *dev, struct nat46_xlate_rule *rule, int rule_num, bool is_local_rule, uint64_t *stats)
{
	if (!(rule->v6_pref_len == 32 || rule->v6_pref_len == 40 ||
	      rule->v6_pref_len == 48 || rule->v6_pref_len == 56 ||
	      rule->v6_pref_len == 64 || rule->v6_pref_len == 96)) {
		nss_connmgr_map_t_warning("%p: mapt rule %d is invalid as rfc6052 end user prefix is invalid\n", dev, rule_num);
		*stats |= 1 << (is_local_rule ? MAP_T_LOCAL_IPV6_PREFIX_LEN_IS_NOT_32_40_48_56_64_OR_96 : MAP_T_REMOTE_IPV6_PREFIX_LEN_IS_NOT_32_40_48_56_64_OR_96);
		return false;
	}

	return true;
}

/*
 * nss_connmgr_mapt_check_correctness_of_mapt_rule()
 *	Check each mapt rule params and validate each field.
 *
 * Returns true if all parameters are correct.
 * As per RFC7599 (map-t rfc), local style should be map-t. Remote style
 * of FMR must be map-t, but remote syle of DMR is RFC6052.
 * map-t user space process doesnot really mandates this restriction and
 * allows style of rule can be anything irrespective of FMR or DMR. So
 * this check also won't fails on style mismatch.
 */
static bool nss_connmgr_mapt_check_correctness_of_mapt_rule(struct net_device *dev, struct nat46_xlate_rulepair *rule_pair, int rule_num, uint64_t *stats)
{
	/*
	 * Validate local rule parameters
	 */
	switch (rule_pair->local.style) {
	case NAT46_XLATE_NONE:
		break;

	case NAT46_XLATE_MAP:
		if (!nss_connmgr_mapt_validate_rule_style_mapt(dev, &rule_pair->local, rule_num, true, stats)) {
			return false;
		}
		break;

	case NAT46_XLATE_RFC6052:
		if (!nss_connmgr_mapt_validate_rule_style_rfc6052(dev, &rule_pair->local, rule_num, true, stats)) {
			return false;
		}
		break;

	default:
		*stats |= 1 << MAP_T_LOCAL_STYLE_IS_NOT_MAP_T_OR_RFC6052;
		return false;
	}

	/*
	 * Validate remote rule parameters
	 */
	switch (rule_pair->remote.style) {
	case NAT46_XLATE_MAP:
		if (!nss_connmgr_mapt_validate_rule_style_mapt(dev, &rule_pair->remote, rule_num, false, stats)) {
			return false;
		}
		break;

	case NAT46_XLATE_RFC6052:
		if (!nss_connmgr_mapt_validate_rule_style_rfc6052(dev, &rule_pair->remote, rule_num, false, stats)) {
			return false;
		}
		break;

	default:
		*stats |= 1 << MAP_T_REMOTE_STYLE_IS_NOT_MAP_T_OR_RFC6052;
		return false;
	}

	return true;
}

 /*
 * nss_connmgr_map_t_ipv6_get_tclass()
 * 	Get traffic class from IPv6 header.
 */
static inline uint8_t nss_connmgr_map_t_ipv6_get_tclass(struct ipv6hdr *ip6hdr)
{
	uint32_t verclassflow = ntohl(*(uint32_t *)ip6hdr);
	return  (verclassflow & MAP_T_IPV6_CLASS_MASK) >>
		 MAP_T_IPV6_CLASS_SHIFT;
}

/*
 * nss_connmgr_map_t_ipv6_set_tclass()
 * 	Set traffic class in IPv6 header.
 */
static inline void nss_connmgr_map_t_ipv6_set_tclass(struct ipv6hdr *ip6hdr, uint8_t tclass)
{
	uint32_t *ptr = (uint32_t *)ip6hdr;
	uint32_t verclassflow = ntohl(*ptr);

	verclassflow &= (uint32_t)~MAP_T_IPV6_CLASS_MASK;
	verclassflow |= (tclass << MAP_T_IPV6_CLASS_SHIFT) & MAP_T_IPV6_CLASS_MASK;
	*ptr = htonl(verclassflow);
}

/*
 * nss_connmgr_map_t_decap_exception()
 *	Exception handler registered to NSS for handling map_t ipv6 pkts
 */
static void nss_connmgr_map_t_decap_exception(struct net_device *dev,
			struct sk_buff *skb,
			__attribute__((unused)) struct napi_struct *napi)

{
	struct iphdr *ip4_hdr;
	struct ipv6hdr *ip6_hdr;
	uint32_t v4saddr = 0, v4daddr = 0;
	struct ipv6hdr ip6_hdr_r;
	uint8_t next_hdr, hop_limit, tclass, l4_proto;
	int total_len;
	uint32_t identifier;
	bool df_bit = false;
	uint16_t skip_sz = 0;

	/* discard L2 header */
	skb_pull(skb, sizeof(struct ethhdr));
	skb_reset_mac_header(skb);

	skb_reset_network_header(skb);

	ip6_hdr = ipv6_hdr(skb);
	skb_set_transport_header(skb, sizeof(struct ipv6hdr));

	/*
	 * IPv4 packet is xlated to ipv6 packet by acceleration engine. But there is no ipv6 rule.
	 * Call xlate_6_to_4() [ which is exported by nat46.ko ] to find original ipv4 src and ipv4 dest address.
	 * These function is designed for packets from wan to lan. Since this packet is from lan, need to call
	 * this function with parameters reversed. ipv6_hdr_r is used for reversing ip addresses.
	 */
	memcpy(&ip6_hdr_r.saddr, &ip6_hdr->daddr, sizeof(struct in6_addr));
	memcpy(&ip6_hdr_r.daddr, &ip6_hdr->saddr, sizeof(struct in6_addr));

	if (unlikely(!xlate_6_to_4(dev, &ip6_hdr_r, ip6_hdr->nexthdr, &v4saddr, &v4daddr))) {  /* packet needs to be xlated v6 to v4 */
		nss_connmgr_map_t_warning("%p: Martian ipv6 packet !!..free it. (saddr=%pI6c daddr=%pI6c)\n", dev,\
					  &ip6_hdr->saddr, &ip6_hdr->daddr);
		dev_kfree_skb_any(skb);
		return;
	}

	next_hdr = ip6_hdr->nexthdr;
	total_len = sizeof(struct iphdr) + ntohs(ip6_hdr->payload_len);
	hop_limit = ip6_hdr->hop_limit;
	tclass = nss_connmgr_map_t_ipv6_get_tclass(ip6_hdr);

	if (likely(next_hdr != NEXTHDR_FRAGMENT)) {
		df_bit = true;
		l4_proto = next_hdr;
	} else {
		struct frag_hdr tmp_fh, *fh;
		const __be32 *fh_addr = skb_header_pointer(skb, sizeof(struct ipv6hdr), sizeof(struct frag_hdr), &tmp_fh);
		skip_sz = sizeof(struct frag_hdr);
		if (!fh_addr) {
			nss_connmgr_map_t_warning("%p: Not able to offset to frag header while v6 -->v4 xlate\n", dev);
			dev_kfree_skb_any(skb);
			return;
		}

		fh = (struct frag_hdr *)fh_addr;
		identifier = ntohl(fh->identification);
		l4_proto = fh->nexthdr;
	}

	skb_pull(skb, sizeof(struct ipv6hdr) + skip_sz - sizeof(struct iphdr));
	skb_reset_network_header(skb);
	skb_reset_mac_header(skb);

	ip4_hdr = ip_hdr(skb);
	memset(ip4_hdr, 0, sizeof(struct iphdr));

	skb_set_transport_header(skb, sizeof(struct iphdr));
	skb->protocol = htons(ETH_P_IP);

	ip4_hdr->ihl = 5;
	ip4_hdr->version = 4;
	ip4_hdr->tot_len = htons(total_len - skip_sz);
	ip4_hdr->ttl = hop_limit;
	ip4_hdr->protocol = l4_proto;
	ip4_hdr->saddr = v4daddr;
	ip4_hdr->daddr = v4saddr;
	ip4_hdr->tos = tclass;
	if (unlikely(df_bit)) {
		ip4_hdr->frag_off = htons(IP_DF);
	} else {
		ip4_hdr->id = htons(identifier & 0xffff);
	}

	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->ip_summed = CHECKSUM_NONE;
	skb->dev = dev;

	nss_connmgr_map_t_trace("%p: ipv6 packet exceptioned after v4 ---> v6 xlate, created original ipv4 packet\n", dev);
	nss_connmgr_map_t_trace("%p: Calculated ipv4 params: src_addr=0x%x dest_addr=0x%x totallen=%d\n", dev, ip4_hdr->saddr, ip4_hdr->daddr, total_len);

	dev_queue_xmit(skb);
	return;
}

/*
 * nss_connmgr_map_t_encap_exception()
 *	Exception handler registered to NSS for handling map_t ipv4 pkts
 */
static void nss_connmgr_map_t_encap_exception(struct net_device *dev,
			struct sk_buff *skb,
			__attribute__((unused)) struct napi_struct *napi)

{
	struct iphdr *ip4_hdr;
	struct ipv6hdr *ip6_hdr;
	uint8_t v6saddr[16], v6daddr[16];
	struct tcphdr *v4_tcp_hdr = NULL;
	struct udphdr *v4_udp_hdr = NULL;
	struct iphdr ip4_hdr_r;
	__be16 sport, dport;
	uint8_t nexthdr, hop_limit, tos;
	int payload_len;
	bool df_bit = false;
	uint16_t append_hdr_sz = 0;
	uint16_t identifier;


	/* discard L2 header */
	skb_pull(skb, sizeof(struct ethhdr));
	skb_reset_mac_header(skb);

	skb_reset_network_header(skb);

	ip4_hdr = ip_hdr(skb);
	skb_set_transport_header(skb, ip4_hdr->ihl*4);

	if (ip4_hdr->protocol == IPPROTO_TCP) {
		v4_tcp_hdr = tcp_hdr(skb);
		sport = v4_tcp_hdr->source;
		dport = v4_tcp_hdr->dest;
	} else if (ip4_hdr->protocol == IPPROTO_UDP) {
		v4_udp_hdr = udp_hdr(skb);
		sport = v4_udp_hdr->source;
		dport = v4_udp_hdr->dest;
	} else {
		nss_connmgr_map_t_warning("%p: Unsupported protocol, free it up\n", dev);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * IPv6 packet is xlated to ipv4 packet by acceleration engine. But there is no ipv4 rule.
	 * Call xlate_4_to_6() [ which is exported by nat46.ko ] to find original ipv6 src and ipv6 dest address.
	 * These functions is designed for packets from lan to wan. Since this packet is from wan, need to call
	 * this function with parameters reversed. ipv4_hdr_r is used for reversing ip addresses.
	 */
	ip4_hdr_r.daddr = ip4_hdr->saddr;
	ip4_hdr_r.saddr = ip4_hdr->daddr;

	if (unlikely(!xlate_4_to_6(dev, &ip4_hdr_r, dport, sport, v6saddr, v6daddr))) { /* exception happened after packet got xlated */
		nss_connmgr_map_t_warning("%p: Martian ipv4 packet !!..free it. (saddr = 0x%x daddr = 0x%x sport = %d dport = %d)\n", dev,\
					  ip4_hdr->saddr, ip4_hdr->daddr, sport, dport);
		dev_kfree_skb_any(skb);
		return;
	}

	nexthdr = ip4_hdr->protocol;
	payload_len = ntohs(ip4_hdr->tot_len) - sizeof(struct iphdr);
	hop_limit = ip4_hdr->ttl;
	tos = ip4_hdr->tos;
	identifier = ntohs(ip4_hdr->id);

	if (ip4_hdr->frag_off & htons(IP_DF)) {
		df_bit = true;
	}  else if (map_t_flags & MAPT_FLAG_ADD_DUMMY_HDR) {
		append_hdr_sz = sizeof(struct frag_hdr);
	}

	if (!pskb_may_pull(skb, sizeof(struct ipv6hdr) + append_hdr_sz - sizeof(struct iphdr))) {
		nss_connmgr_map_t_warning("%p: Not enough headroom for ipv6 packet...Freeing the packet\n", dev);
		dev_kfree_skb_any(skb);
		return;
	}

	skb_push(skb, sizeof(struct ipv6hdr) + append_hdr_sz - sizeof(struct iphdr));
	skb_reset_network_header(skb);
	skb_reset_mac_header(skb);

	skb->protocol = htons(ETH_P_IPV6);

	ip6_hdr = ipv6_hdr(skb);
	memset(ip6_hdr, 0, sizeof(struct ipv6hdr));

	ip6_hdr->version = 6;
	ip6_hdr->payload_len = htons(payload_len + append_hdr_sz);
	ip6_hdr->hop_limit = hop_limit;

	nss_connmgr_map_t_ipv6_set_tclass(ip6_hdr, tos);
	memcpy(&ip6_hdr->daddr, v6saddr, sizeof(struct in6_addr));
	memcpy(&ip6_hdr->saddr, v6daddr, sizeof(struct in6_addr));

	if (unlikely(df_bit) || !(map_t_flags & MAPT_FLAG_ADD_DUMMY_HDR))  {
		ip6_hdr->nexthdr = nexthdr;
	} else {
		struct frag_hdr tmp_fh, *fh;
		const __be32 *fh_addr = skb_header_pointer(skb, sizeof(struct ipv6hdr), sizeof(struct frag_hdr), &tmp_fh);
		if (!fh_addr) {
			nss_connmgr_map_t_warning("%p: Not able to offset to frag header\n", dev);
			dev_kfree_skb_any(skb);
			return;
		}
		fh = (struct frag_hdr *)fh_addr;
		memset(fh, 0, sizeof(struct frag_hdr));
		fh->identification = htonl(identifier);
		fh->nexthdr = nexthdr;
		ip6_hdr->nexthdr = NEXTHDR_FRAGMENT;
	}

	skb_set_transport_header(skb, sizeof(struct ipv6hdr) + append_hdr_sz);


	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->ip_summed = CHECKSUM_NONE;
	skb->dev = dev;

	nss_connmgr_map_t_trace("%p: ipv4 packet exceptioned after v6 ---> v4 xlate, created original ipv6 packet\n", dev);
	nss_connmgr_map_t_trace("%p: Calculted ipv6 params: src_addr=%pI6, dest_addr=%pI6, payload_len=%d\n", dev, v6saddr, v6daddr, payload_len);

	dev_queue_xmit(skb);
	return;
}

/*
 * nss_map_t_update_dev_stats()
 */
void nss_map_t_update_dev_stats(struct net_device *dev, struct nss_map_t_sync_stats_msg *sync_stats)
{
	if (!dev) {
		nss_connmgr_map_t_warning("dev is null\n");
		return;
	}

	dev_hold(dev);

	nat46_update_stats(dev,
		sync_stats->node_stats.rx_packets,
		sync_stats->node_stats.rx_bytes,
		sync_stats->node_stats.tx_packets,
		sync_stats->node_stats.tx_bytes,
		nss_cmn_rx_dropped_sum(&sync_stats->node_stats),
		sync_stats->tx_dropped);

	dev_put(dev);
}

/*
 * nss_connmgr_map_t_event_receive()
 *	Event Callback to receive events from NSS
 */
static void nss_connmgr_map_t_event_receive(void *if_ctx, struct nss_map_t_msg *tnlmsg)
{
	struct net_device *netdev = if_ctx;

	switch (tnlmsg->cm.type) {
	case NSS_MAP_T_MSG_SYNC_STATS:
		nss_map_t_update_dev_stats(netdev, (struct nss_map_t_sync_stats_msg *)&tnlmsg->msg.stats);
		break;

	default:
		nss_connmgr_map_t_info("%p: Unknown Event from NSS\n", netdev);
		break;
	}
}

/*
 * nss_connmgr_map_t_dev_up()
 */
static int nss_connmgr_map_t_dev_up(struct net_device *dev)
{
	struct nat46_xlate_rulepair *rule_pairs;
	struct nss_ctx_instance *nss_ctx;
	struct nss_map_t_msg maptmsg;
	struct nss_map_t_instance_rule_config_msg *maptcfg;
	int rule_pair_count = 0;
	int if_inner, if_outer;
	nss_tx_status_t status;
	uint32_t features = 0;
	int i, j;
	uint64_t map_t_rule_validation_stats;

	/*
	 * Get MAP-T interface's information.
	 */
	if (!nat46_get_info(dev, &rule_pairs, &rule_pair_count, &map_t_flags)) {
		nss_connmgr_map_t_warning("%p: Failed to get ruleset on map-t netdevice (%s)\n", dev, dev->name);
		return NOTIFY_DONE;
	}

	/*
	 * Return, if number of  rules configured for the map-t
	 * interface is < 1 or > 64
	 */

	if (rule_pair_count < MAP_T_MIN_NUM_RULES_PER_MAP_T_INSTANCE || rule_pair_count > MAP_T_MAX_NUM_RULES_PER_MAP_T_INSTANCE) {
		nss_connmgr_map_t_warning("%p: No accleration supported if number of rules configured is %d\n", dev, rule_pair_count);
		return NOTIFY_DONE;
	}

	if (mapt_interfaces_count == NSS_MAX_MAP_T_DYNAMIC_INTERFACES) {
		nss_connmgr_map_t_warning("%p: Max number of mapt interfaces supported is %d\n", dev, NSS_MAX_MAP_T_DYNAMIC_INTERFACES);
		return NOTIFY_DONE;
	}

	/*
	 * Create MAP-T inner dynamic interface
	 */
	if_inner = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_INNER);
	if (if_inner < 0) {
		nss_connmgr_map_t_warning("%p: Request interface number failed\n", dev);
		return NOTIFY_DONE;
	}
	nss_connmgr_map_t_info("%p: encap nss_dynamic_interface_alloc_node() successful. if_number = %d\n", dev, if_inner);

	/*
	 * Create MAP-T outer dynamic interface
	 */
	if_outer = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_OUTER);
	if (if_outer < 0) {
		nss_connmgr_map_t_warning("%p: Request interface number failed\n", dev);
		goto outer_alloc_fail;
	}
	nss_connmgr_map_t_info("%p: decap nss_dynamic_interface_alloc_node() successful. if_number = %d\n", dev, if_outer);

	/*
	 * Register MAP-T encap interface with NSS
	 */
	nss_ctx = nss_map_t_register_if(if_inner,
			NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_INNER,
			nss_connmgr_map_t_encap_exception,
			nss_connmgr_map_t_event_receive,
			dev,
			features);

	if (!nss_ctx) {
		nss_connmgr_map_t_warning("%p: encap nss_register_map_t_if failed\n", dev);
		goto inner_register_fail;
	}
	nss_connmgr_map_t_info("%p: encap nss_register_map_t_if() successful. nss_ctx = %p\n", dev, nss_ctx);

	/*
	 * Register MAP-T decap interface with NSS
	 */
	nss_ctx = nss_map_t_register_if(if_outer,
			NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_OUTER,
			nss_connmgr_map_t_decap_exception,
			nss_connmgr_map_t_event_receive,
			dev,
			features);

	if (!nss_ctx) {
		nss_connmgr_map_t_warning("%p: decap nss_register_map_t_if failed\n", dev);
		goto outer_register_fail;
	}

	/*
	 * allocate needed data structures
	 */
	nss_connmgr_map_t_allocate_all(dev, rule_pairs, rule_pair_count);

	/*
	 * Send Rule configuration to acceleration engine.
	 */
	for (i = 0; i < rule_pair_count; i++) {
		map_t_rule_validation_stats = 0;

		memset(&maptmsg, 0, sizeof(struct nss_map_t_msg));
		maptcfg = &maptmsg.msg.create_msg;

		/*
		 * These info is used by nss to decide if all rules are
		 * sent to accleration engine. ie rule_num == rule_pair_count
		 */
		maptcfg->rule_num = i + 1;
		maptcfg->total_rules = rule_pair_count;

		/*
		 * Set local rule params
		 */
		maptcfg->local_map_style = (rule_pairs + i)->local.style;

		for (j = 0; j < 4; j++) {
			*((uint32_t *)(&maptcfg->local_ipv6_prefix) + j) = ntohl(*((uint32_t *)((rule_pairs + i)->local.v6_pref.s6_addr) + j));
		}
		maptcfg->local_ipv6_prefix_len = (rule_pairs + i)->local.v6_pref_len;

		maptcfg->local_ipv4_prefix = ntohl((rule_pairs + i)->local.v4_pref);
		maptcfg->local_ipv4_prefix_len = (rule_pairs + i)->local.v4_pref_len;
		maptcfg->local_ea_len = (rule_pairs + i)->local.ea_len;
		maptcfg->local_psid_offset = (rule_pairs + i)->local.psid_offset;

		maptcfg->valid_rule = 1;

		/*
		 * Set remote rule params
		 */
		maptcfg->remote_map_style = (rule_pairs + i)->remote.style;
		for (j = 0; j < 4; j++) {
			*((uint32_t *)(&maptcfg->remote_ipv6_prefix) + j) = ntohl(*((uint32_t *)((rule_pairs + i)->remote.v6_pref.s6_addr) + j));
		}
		maptcfg->remote_ipv6_prefix_len = (rule_pairs + i)->remote.v6_pref_len;

		maptcfg->remote_ipv4_prefix = ntohl((rule_pairs + i)->remote.v4_pref);
		maptcfg->remote_ipv4_prefix_len = (rule_pairs + i)->remote.v4_pref_len;
		maptcfg->remote_ea_len = (rule_pairs + i)->remote.ea_len;
		maptcfg->remote_psid_offset = (rule_pairs + i)->remote.psid_offset;

		/*
		 * check correctness of a map-t rule
		 */
		maptcfg->valid_rule = nss_connmgr_mapt_check_correctness_of_mapt_rule(dev, rule_pairs + i, i, &map_t_rule_validation_stats);

		/*
		 * Debugfs stats file <debug-fs-dir>/map-t helps to figure out
		 * goodness of a map-t rule. All bits should be 0, if rule is good
		 * and valid.
		 */
		if (unlikely(!maptcfg->valid_rule)) {
			map_t_rule_validation_stats |= 1 << MAP_T_INVALID_RULE;
		}

		/*
		 * set the sibling interface number
		 */
		maptcfg->sibling_if = if_outer;

		/*
		 * set MAP-T flags
		 */
		maptcfg->flags = map_t_flags;

		/*
		 * Send configure message to MAP-T encap interface.
		 */
		nss_map_t_msg_init(&maptmsg, if_inner, NSS_MAP_T_MSG_INSTANCE_RULE_CONFIGURE, sizeof(struct nss_map_t_instance_rule_config_msg), NULL, NULL);
		status = nss_map_t_tx_sync(nss_ctx, &maptmsg);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_map_t_warning("%p: nss encap MAP-T instance configure command error %d\n", dev, status);

			map_t_rule_validation_stats |= ((uint64_t)(1)) << (64 - MAPT_AE_ERR_CONFIGURE);
			nss_connmgr_map_t_debugfs_set_rule_status(dev, i + 1, map_t_rule_validation_stats);

			goto config_fail;
		}

		nss_connmgr_map_t_debugfs_set_rule_status(dev, i + 1, map_t_rule_validation_stats);
		nss_connmgr_map_t_info("%p: encap nss_map_t_tx() rule #%d configuration successful\n", dev, i + 1);

		/*
		 * set the sibling interface number
		 */
		maptcfg->sibling_if = if_inner;

		/*
		 * Send configure message to MAP-T decap interface.
		 */
		nss_map_t_msg_init(&maptmsg, if_outer, NSS_MAP_T_MSG_INSTANCE_RULE_CONFIGURE, sizeof(struct nss_map_t_instance_rule_config_msg), NULL, NULL);
		status = nss_map_t_tx_sync(nss_ctx, &maptmsg);
		if (status != NSS_TX_SUCCESS) {
			nss_connmgr_map_t_warning("%p: nss decap MAP-T instance configure command error %d\n", dev, status);

			map_t_rule_validation_stats |= ((uint64_t)(1)) << (64 - MAPT_AE_ERR_CONFIGURE);
			nss_connmgr_map_t_debugfs_set_rule_status(dev, i + 1, map_t_rule_validation_stats);

			goto config_fail;
		}

		nss_connmgr_map_t_debugfs_set_rule_status(dev, i + 1, map_t_rule_validation_stats);
		nss_connmgr_map_t_info("%p: decap nss_map_t_tx() rule #%d configuration successful\n", dev, i + 1);
	}

	/*
	 * Increment map-t interface count
	 */
	mapt_interfaces_count++;
	nss_connmgr_map_t_info("%p: MAP-T interface count is #%d\n", dev, mapt_interfaces_count);

	return NOTIFY_DONE;

config_fail:
	nss_connmgr_map_t_free_all(dev);
	nss_map_t_unregister_if(if_outer);
outer_register_fail:
	nss_map_t_unregister_if(if_inner);
inner_register_fail:
	status = nss_dynamic_interface_dealloc_node(if_outer, NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_map_t_warning("%p: Unable to dealloc the decap node[%d] in the NSS FW!\n", dev, if_outer);
	}
outer_alloc_fail:
	status = nss_dynamic_interface_dealloc_node(if_inner, NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_map_t_warning("%p: Unable to dealloc the encap node[%d] in the NSS FW!\n", dev, if_inner);
	}
	return NOTIFY_DONE;
}

/*
 * nss_connmgr_map_t_dev_down()
 */
static int nss_connmgr_map_t_dev_down(struct net_device *dev)
{
	int if_inner, if_outer;
	nss_tx_status_t status;
	struct nss_map_t_msg maptmsg;
	struct nss_map_t_instance_rule_deconfig_msg *maptcfg;

	/*
	 * Check if MAP-T encap interface is registered with NSS
	 */
	if_inner = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_INNER);
	if (if_inner < 0) {
		nss_connmgr_map_t_warning("%p: MAP-T encap net device is not registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Check if MAP-T decap interface is registered with NSS
	 */
	if_outer = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_OUTER);
	if (if_outer < 0) {
		nss_connmgr_map_t_warning("%p: MAP-T decap net device is not registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Free all allocated data structures.
	 */
	nss_connmgr_map_t_free_all(dev);

	memset(&maptmsg, 0, sizeof(struct nss_map_t_msg));
	maptcfg = &maptmsg.msg.destroy_msg;
	maptcfg->if_number = if_inner;

	/*
	 * Send deconfigure message to MAP-T encap interface.
	 */
	nss_map_t_msg_init(&maptmsg, if_inner, NSS_MAP_T_MSG_INSTANCE_RULE_DECONFIGURE, sizeof(struct nss_map_t_instance_rule_deconfig_msg), NULL, NULL);
	status = nss_map_t_tx_sync(nss_map_t_get_context(), &maptmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_map_t_warning("%p: map_t encap instance deconfigure command failed, if_number = %d\n", dev, if_inner);
		return NOTIFY_DONE;
	}

	memset(&maptmsg, 0, sizeof(struct nss_map_t_msg));
	maptcfg = &maptmsg.msg.destroy_msg;
	maptcfg->if_number = if_outer;

	/*
	 * Send deconfigure message to MAP-T decap interface.
	 */
	nss_map_t_msg_init(&maptmsg, if_outer, NSS_MAP_T_MSG_INSTANCE_RULE_DECONFIGURE, sizeof(struct nss_map_t_instance_rule_deconfig_msg), NULL, NULL);
	status = nss_map_t_tx_sync(nss_map_t_get_context(), &maptmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_map_t_warning("%p: map_t decap instance deconfigure command failed, if_number = %d\n", dev, if_outer);
		return NOTIFY_DONE;
	}

	nss_map_t_unregister_if(if_inner);
	nss_map_t_unregister_if(if_outer);

	status = nss_dynamic_interface_dealloc_node(if_inner, NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_INNER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_map_t_warning("%p: map_t encap dealloc node failure for if_number = %d\n", dev, if_inner);
		return NOTIFY_DONE;
	}
	nss_connmgr_map_t_info("%p: deleted map_t encap instance, if_number = %d\n", dev, if_inner);

	status = nss_dynamic_interface_dealloc_node(if_outer, NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_OUTER);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_map_t_warning("%p: map_t decap dealloc node failure for if_number = %d\n", dev, if_outer);
		return NOTIFY_DONE;
	}
	nss_connmgr_map_t_info("%p: deleted map_t decap instance, if_number = %d\n", dev, if_outer);

	/*
	 * Decrement interface count
	 */
	mapt_interfaces_count--;
	nss_connmgr_map_t_info("%p: MAP-T interface count is #%d\n", dev, mapt_interfaces_count);

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_map_t_dev_event()
 *	Net device notifier for nss map_t module
 */
static int nss_connmgr_map_t_dev_event(struct notifier_block *nb,
		unsigned long event, void *dev)
{
	struct net_device *netdev;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 10, 0))
	netdev = (struct net_device *)dev;
#else
	netdev = netdev_notifier_info_to_dev(dev);
#endif

	/*
	 * Check for map-t device
	 */
	if (!is_map_t_dev(netdev)) {
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_UP:
		return nss_connmgr_map_t_dev_up(netdev);

	case NETDEV_DOWN:
		return nss_connmgr_map_t_dev_down(netdev);

	default:
		break;
	}

	return NOTIFY_DONE;
}

/*
 * map_t_debugfs_read()
 *	Read function of <DEBUGFS>/map-t entry
 *
 * map-t client code does a map-t rule validation before pushing ae-engine
 * rules. This helps to easily figure out problems in rules and hence quick
 * debuggging. <DEBUG-FS-DIR>/map-t has a 64bit value for each rule. All
 * bits '0' indicate that there is no validation error.
 */
static ssize_t map_t_debugfs_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	struct list_head *list_head_ptr = file->private_data;
	struct list_dev_to_map_t_rules_entry_t *entry;
	char *buf;
	int total_rules = 0;
	int num_net_devices = 0;
	int user_copy_len = 0;
	int err = -EFAULT;

	/*
	 * Process context should sleep if the netdevice is going down and
	 * list_head_ptr is being emptied so that 'cat' of this debugfs
	 * entry won't race in smp
	 */
	rtnl_lock();

	/*
	 * Allocate memory
	 */
	list_for_each_entry(entry, list_head_ptr, list) {
		total_rules += entry->rule_pair_count;
		num_net_devices++;
	}

	/*
	 * Allocate buffer
	 */
	buf = kzalloc((MAP_T_NETDEVICE_SZ * num_net_devices) + (MAP_T_RULE_SZ * total_rules) + 1, GFP_KERNEL);
	if (!buf) {
		rtnl_unlock();
		return err;
	}

	list_for_each_entry(entry, list_head_ptr, list) {
		int i;
		int copied_len;
		copied_len = scnprintf(buf + user_copy_len, MAP_T_NETDEVICE_SZ, "netdevice = %s\n", entry->dev->name);
		if (unlikely(copied_len == -1)) {
			kfree(buf);
			rtnl_unlock();
			return err;
		}
		user_copy_len += copied_len;

		for (i = 0; i < entry->rule_pair_count; i++) {

			copied_len = scnprintf(buf + user_copy_len, MAP_T_RULE_SZ, "%d: 0x%016llx\n",
					      i + 1, entry->rule_status[i]);

			if (unlikely(copied_len == -1)) {
				kfree(buf);
				rtnl_unlock();
				return err;
			}
			user_copy_len += copied_len;
		}
	}

	err = simple_read_from_buffer(user_buf, count, ppos, buf, user_copy_len);
	kfree(buf);
	rtnl_unlock();
	return err;
}

/*
 * debugfs file ops structure
 */
static const struct file_operations map_t_debugfs_fops = {
	.open = simple_open,
	.read = map_t_debugfs_read,
};

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_connmgr_map_t_notifier = {
	.notifier_call = nss_connmgr_map_t_dev_event,
};

/*
 * nss_connmgr_map_t_init_module()
 *	map_t module init function
 */
int __init nss_connmgr_map_t_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif

	map_t_debugfs = debugfs_create_file("map-t", 0444, NULL, &list_dev_to_map_t_rules_head, &map_t_debugfs_fops);

	register_netdevice_notifier(&nss_connmgr_map_t_notifier);
	return 0;
}

/*
 * nss_connmgr_map_t_exit_module
 *	map_t module exit function
 */
void __exit nss_connmgr_map_t_exit_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

	if (map_t_debugfs) {
		debugfs_remove(map_t_debugfs);
	}

	unregister_netdevice_notifier(&nss_connmgr_map_t_notifier);
}

module_init(nss_connmgr_map_t_init_module);
module_exit(nss_connmgr_map_t_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS map_t offload manager");
