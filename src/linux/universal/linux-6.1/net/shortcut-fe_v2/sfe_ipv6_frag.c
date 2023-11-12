/*
 * sfe_ipv6_frag.c
 *	Shortcut forwarding engine - IPv6 Fragment forwarding support.
 *
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <net/tcp.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <net/udp.h>
#include <net/vxlan.h>
#include <linux/refcount.h>
#include <linux/netfilter.h>
#include <linux/inetdevice.h>
#include <linux/netfilter_ipv6.h>
#include <linux/seqlock.h>
#include <net/protocol.h>
#include <net/addrconf.h>
#include <net/gre.h>

#if defined(SFE_RFS_SUPPORTED)
#include <ppe_rfs.h>
#endif
#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv6.h"
#include "sfe_ipv6_udp.h"
#include "sfe_ipv6_tcp.h"
#include "sfe_ipv6_icmp.h"
#include "sfe_pppoe.h"
#include "sfe_pppoe_mgr.h"
#include "sfe_ipv6_pppoe_br.h"
#include "sfe_ipv6_tunipip6.h"
#include "sfe_ipv6_gre.h"
#include "sfe_ipv6_esp.h"
#include "sfe_ipv6_frag.h"
#include "sfe_vlan.h"
#include "sfe_trustsec.h"
#include "sfe_ipv6_multicast.h"
#include "sfe_ipv6_frag.h"

/*
 * sfe_ipv6_frag_process_exception_queue()
 *	Timer function to exception the skbs.
 */
static void sfe_ipv6_frag_process_exception_queue(struct timer_list *t)
{
	struct sfe_ipv6_frag_exception *pcpu = from_timer(pcpu, t, timer);
	struct sk_buff_head *skbh = &pcpu->exception_queue;
	struct sk_buff *skb;

	while ((skb = skb_dequeue(skbh))) {
		netif_receive_skb_core(skb);
	}
}

/*
 * sfe_ipv6_frag_hash_init()
 *	Initialize conn match hash lists
 */
static void sfe_ipv6_frag_hash_init(struct sfe_ipv6_frag *sif, int len)
{
	struct hlist_head *hash_list = sif->frag_id_hashtable;
	int i;

	for (i = 0; i < len; i++) {
		INIT_HLIST_HEAD(&hash_list[i]);
	}
}

/*
 * sfe_ipv6_frag_id_put()
 *	Reference count decrement.
 */
static inline void sfe_ipv6_frag_id_put(struct sfe_ipv6_frag *sif, struct sfe_ipv6_frag_id_entity *fie)
{
	struct sfe_ipv6_frag_id_element *fiel, *fiel_tmp;

	if (!refcount_dec_and_test(&fie->refcnt)) {
		return;
	}

	/*
	 * If ref count is zero, free the resources held by the fragment id entity
	 * Followed by freeing the fragment id entity itself.
	 */
	list_for_each_entry_safe(fiel, fiel_tmp, &fie->q_head, q_node) {
		list_del(&fiel->q_node);
		kfree_skb(fiel->skb);
		this_cpu_inc(sif->si->stats_pcpu->fragment_dropped64);
		kfree(fiel);
	}

	kfree(fie);
}

/*
 * sfe_ipv6_frag_remove_id_node()
 *	Remove a frag id entity from the hash.
 */
static inline void sfe_ipv6_frag_remove_id_node(struct sfe_ipv6_frag *sif, struct sfe_ipv6_frag_id_entity *fie)
{
	lockdep_assert_held(&sif->lock);
	hlist_del_init_rcu(&fie->hnode);
}

/*
 * sfe_ipv6_frag_free_id_all()
 *	Free all the fragment entries in the global lru queue.
 */
static void sfe_ipv6_frag_free_id_all(struct sfe_ipv6_frag *sif)
{
	struct sfe_ipv6_frag_id_entity *fie, *fie_tmp;
	spin_lock_bh(&sif->lock);
	list_for_each_entry_safe(fie, fie_tmp, &sif->lru_head, lru_node) {
		list_del(&fie->lru_node);
		sif->frag_id_entity_counter--;

		/*
		 * Remove the fragment from the hash table.
		 */
		sfe_ipv6_frag_remove_id_node(sif, fie);
		sfe_ipv6_frag_id_put(sif, fie);
	}
	spin_unlock_bh(&sif->lock);
}

/*
 * sfe_ipv6_frag_timer_cb()
 *	Check if the fragment id entity timer is expired.
 */
static void sfe_ipv6_frag_timer_cb(struct timer_list *t)
{
	struct sfe_ipv6_frag *sif = from_timer(sif, t, timer);
	unsigned long curr_time = jiffies;
	struct sfe_ipv6_frag_id_entity *fie, *fie_tmp;
	spin_lock(&sif->lock);
	list_for_each_entry_safe(fie, fie_tmp, &sif->lru_head, lru_node) {
		if (fie->timeout > curr_time) {
			break;
		}

		/*
		 * Timer has expired. Remove fragment id entity from LRU list.
		 */
		list_del(&fie->lru_node);

		/*
		 * Remove the fragment from the hash table.
		 */
		sfe_ipv6_frag_remove_id_node(sif, fie);

		sif->frag_id_entity_counter--;
		sfe_ipv6_frag_id_put(sif, fie);
		this_cpu_inc(sif->si->stats_pcpu->fragment_id_timeout64);
	}

	spin_unlock(&sif->lock);
	mod_timer(t, jiffies + SFE_IPV6_FRAG_ID_TIMEOUT_TIMER_FREQ);
}

/*
 * sfe_ipv6_frag_id_evict_one()
 *	Evict the older fragment id entity.
 *
 * This API removes the fragment id entity from the LRU list and the hash table.
 */
static inline void sfe_ipv6_frag_id_evict_one(struct sfe_ipv6_frag *sif) {
	struct sfe_ipv6_frag_id_entity *fie;

	/*
	 * Remove the id entity from the global LRU list.
	 */
	if (list_empty(&sif->lru_head)) {
		spin_unlock_bh(&sif->lock);
		return;
	}

	/*
	 * New entries are added to the tail.
	 * Hence, the oldest entry will be the first entry.
	 */
	fie = list_first_entry(&sif->lru_head, struct sfe_ipv6_frag_id_entity, lru_node);
	list_del(sif->lru_head.next);

	/*
	 * Remove the id entity from the hash table.
	 */
	sfe_ipv6_frag_remove_id_node(sif, fie);

	sif->frag_id_entity_counter--;
	sfe_ipv6_frag_id_put(sif, fie);
}

/*
 * sfe_ipv6_get_frag_hash()
 *	Generate the hash used in fragment id entity lookups.
 */
static inline unsigned int sfe_ipv6_get_frag_hash(struct sfe_ipv6_frag *sif, struct sfe_ipv6_addr *src_ip,
						struct sfe_ipv6_addr *dest_ip,
						 __be32 frag_id)
{
	u32 hash = 0;
	struct sfe_ipv6_frag_id_map map;

	/*
	 * Update the 3 tuple required to calculate the hash.
	 */
	map.src_ip = *src_ip;
	map.dest_ip = *dest_ip;
	map.frag_id = frag_id;

	hash = jhash2((const u32 *)&map, sizeof(struct sfe_ipv6_frag_id_map)/sizeof(u32), sif->hash_rand);
	return ((hash >> SFE_IPV6_FRAG_HASH_SHIFT) ^ hash) & SFE_IPV6_FRAG_HASH_MASK;
}

/*
 * sfe_ipv6_frag_id_entity_lookup()
 *	Get the fragment id entity that corresponds to a 3-tuple.
 *
 * Takes the reference on the fragment id entity on successful lookup.
 */
struct sfe_ipv6_frag_id_entity
*sfe_ipv6_frag_id_entity_lookup(struct sfe_ipv6_frag *sif,
					struct sfe_ipv6_addr *src_ip,
					struct sfe_ipv6_addr *dest_ip, __be32 frag_id)
{
	struct sfe_ipv6_frag_id_entity *fie = NULL;
	unsigned int frag_id_idx;
	struct hlist_head *lhead;

	frag_id_idx = sfe_ipv6_get_frag_hash(sif, src_ip, dest_ip, frag_id);

	lhead = &sif->frag_id_hashtable[frag_id_idx];

	/*
	 * Hopefully the first id entity is the one we want.
	 */
	hlist_for_each_entry_rcu(fie, lhead, hnode) {
		if ((!sfe_ipv6_addr_equal(fie->src_ip, src_ip)) ||
		    (!sfe_ipv6_addr_equal(fie->dest_ip, dest_ip)) ||
		    (fie->frag_id != frag_id)) {
			continue;
		}

		this_cpu_inc(sif->si->stats_pcpu->fragment_id_hash_hits64);
		refcount_inc(&fie->refcnt);
		break;
	}

	return fie;
}

/*
 * sfe_ipv6_insert_frag_id entity()
 *	Insert a fragmet id entity into the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static inline void sfe_ipv6_frag_id_entity_insert(struct sfe_ipv6_frag *sif,
						    struct sfe_ipv6_frag_id_entity *fie)
{
	unsigned int frag_id_idx
		= sfe_ipv6_get_frag_hash(sif, fie->src_ip, fie->dest_ip, fie->frag_id);
	lockdep_assert_held(&sif->lock);
	hlist_add_head_rcu(&fie->hnode, &sif->frag_id_hashtable[frag_id_idx]);
}

/*
 * sfe_ipv6_frag_id_entity_alloc()
 *	Allocate memory for fragment id entity
 */
static struct sfe_ipv6_frag_id_entity *sfe_ipv6_frag_id_entity_alloc(struct sfe_ipv6_frag *sif)
{
	struct sfe_ipv6_frag_id_entity *fie;
	if (sif->frag_id_entity_counter > sif->high_tresh) {
		sfe_ipv6_frag_id_evict_one(sif);
		DEBUG_INFO("%px: Out of fragment entries, evicting the oldest one\n", sif);
		this_cpu_inc(sif->si->stats_pcpu->fragment_id_evict64);
	}

	fie = (struct sfe_ipv6_frag_id_entity *)kzalloc(sizeof(struct sfe_ipv6_frag_id_entity), GFP_ATOMIC);
	if (!fie) {
		return NULL;
	}

	fie->sif = sif;
	fie->state = SFE_IPV6_FRAG_FORWARD_NEW;
	fie->timeout = jiffies + sif->timeout;
	spin_lock_init(&fie->lock);
	INIT_LIST_HEAD(&fie->q_head);
	INIT_LIST_HEAD(&fie->lru_node);

	return fie;
}

/*
 * sfe_ipv6_frag_id_entity_create()
 *	Create a fragment id entity.
 *
 * Create a fragment id entity corresponding to the 3-tuple and add it to the hash table.
 */
static inline struct sfe_ipv6_frag_id_entity *sfe_ipv6_frag_id_entity_create(struct sfe_ipv6_frag *sif, struct sfe_ipv6_addr *src_ip, struct sfe_ipv6_addr *dest_ip, __be32 frag_id)
{
	struct sfe_ipv6_frag_id_entity *fie = sfe_ipv6_frag_id_entity_alloc(sif);
	if (!fie) {
		return NULL;
	}

	/*
	 * Update the 3-tuple information.
	 */
	memcpy(fie->src_ip, src_ip, sizeof(struct sfe_ipv6_addr));
	memcpy(fie->dest_ip, dest_ip, sizeof(struct sfe_ipv6_addr));
	fie->frag_id = frag_id;

	/*
	 * Add the id entity to the global LRU list.
	 */
	list_add_tail(&fie->lru_node, &sif->lru_head);

	/*
	 * Add the id entity to the hash table.
	 */
	sfe_ipv6_frag_id_entity_insert(sif, fie);

	sif->frag_id_entity_counter++;

	/*
	 * One reference is for the fragment id entity being added to the hash table.
	 */
	refcount_set(&fie->refcnt, 1);

	return fie;
}

/*
 * sfe_ipv6_frag_id_entity_get()
 *	Find the fragment queue based on 3-tuple.
 *
 * We will create a new frag id entity if not found. A reference is taken on the returned frag id entity.
 */
static inline struct sfe_ipv6_frag_id_entity *sfe_ipv6_frag_id_entity_get(struct sfe_ipv6_frag *sif, struct sfe_ipv6_addr *src_ip, struct sfe_ipv6_addr *dest_ip, __be32 frag_id)
{
	struct sfe_ipv6_frag_id_entity *fie;
	spin_lock_bh(&sif->lock);
	fie = sfe_ipv6_frag_id_entity_lookup(sif , src_ip, dest_ip, frag_id);
	if (!fie) {
		DEBUG_INFO("Fragment id entity lookup failed for 3 Tuple\n"
				"SIP: %pI6, DIP: %pI6, Frag_id = %u\n", src_ip, dest_ip, ntohs(frag_id));
		this_cpu_inc(sif->si->stats_pcpu->fragment_id_lookup_fail64);

		fie = sfe_ipv6_frag_id_entity_create(sif, src_ip, dest_ip, frag_id);
		if (!fie) {
			DEBUG_WARN("Fragment id entity create failed for 3 Tuple\n"
				"SIP: %pI6, DIP: %pI6, Frag_id = %u\n", src_ip, dest_ip, ntohs(frag_id));
			this_cpu_inc(sif->si->stats_pcpu->fragment_id_entity_alloc_fail64);
		}

		/*
		 * Increment the reference count on the frag id entity before
		 * returning.
		 */
		refcount_inc(&fie->refcnt);
	}

	spin_unlock_bh(&sif->lock);
	return fie;
}

/*
 * sfe_ipv6_frag_udp_forward()
 *	Handle UDP fragment forwarding.
 */
static int sfe_ipv6_frag_udp_forward(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
			     unsigned int len, struct ipv6hdr *iph, unsigned int ihl, uint16_t sync_on_find, struct sfe_l2_info *l2_info, bool tun_outer, struct sfe_ipv6_connection_match *cm, struct udphdr *udph, bool first_frag)
{
	struct net_device *xmit_dev;
	u32 service_class_id;
	int ret;
	bool hw_csum;
	bool bridge_flow;
	bool fast_xmit;
	netdev_features_t features;
	u8 ingress_flags = 0;
	sfe_fls_conn_stats_update_t update_cb;

	DEBUG_TRACE("%px: sfe: sfe_ipv6_frag_udp_forward called\n", skb);

	/*
	 * Do we expect an ingress VLAN tag for this flow?
	 */
#ifdef SFE_BRIDGE_VLAN_FILTERING_ENABLE
	ingress_flags = cm->vlan_filter_rule.ingress_flags;
#endif
	if (unlikely(!sfe_vlan_validate_ingress_tag(skb, cm->ingress_vlan_hdr_cnt, cm->ingress_vlan_hdr, l2_info, ingress_flags))) {
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INGRESS_VLAN_TAG_MISMATCH);
		DEBUG_TRACE("VLAN tag mismatch. skb=%px\n", skb);
		return 0;
	}

	/*
	 * Do we expect a trustsec header for this flow ?
	 */
	if (unlikely(!sfe_trustsec_validate_ingress_sgt(skb, cm->ingress_trustsec_valid, &cm->ingress_trustsec_hdr, l2_info))) {
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INGRESS_TRUSTSEC_SGT_MISMATCH);
		DEBUG_TRACE("Trustsec SGT mismatch. skb=%px\n", skb);
		return 0;
	}

	/*
	 * Source interface validate.
	 */
	if (unlikely((cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK) && (cm->match_dev != dev))) {
		if (!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK_NO_FLUSH)) {
			struct sfe_ipv6_connection *c = cm->connection;
			DEBUG_TRACE("flush on source interface check failure\n");
			spin_lock_bh(&si->lock);
			ret = sfe_ipv6_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			if (ret) {
				sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
		}
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INVALID_SRC_IFACE);
		DEBUG_TRACE("exception the packet on source interface check failure\n");
		return 0;
	}

	update_cb = rcu_dereference(sfe_fls_info.stats_update_cb);
	if (cm->fls_conn && update_cb) {
		update_cb(cm->fls_conn, skb);
	}

	/*
	 * If our packet has been marked as "sync on find" we can't actually
	 * forward it in the fast path, but now that we've found an associated
	 * connection we need sync its status before exception it to slow path.
	 */
	if (unlikely(sync_on_find)) {
		sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT);
		DEBUG_TRACE("Sync on find\n");
		return 0;
	}

#ifdef CONFIG_XFRM
	/*
	 * We can't accelerate the flow on this direction, just let it go
	 * through the slow path.
	 */
	if (unlikely(!cm->flow_accel)) {
		this_cpu_inc(si->stats_pcpu->packets_not_forwarded64);
		return 0;
	}
#endif

	bridge_flow = !!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_BRIDGE_FLOW);

	/*
	 * Does our hop_limit allow forwarding?
	 */
	if (likely(!bridge_flow)) {
		if (unlikely(iph->hop_limit < 2)) {
			sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);

			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_UDP_SMALL_TTL);
			DEBUG_TRACE("hop_limit too low\n");
			return 0;
		}
	}

	/*
	 * If our packet is larger than the MTU of the transmit interface then
	 * we can't forward it easily.
	 */
	if (unlikely(len > cm->xmit_dev_mtu)) {
		sfe_ipv6_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION);
		DEBUG_TRACE("Larger than MTU\n");
		return 0;
	}

	/*
	 * Check if skb was cloned. If it was, unclone it. Because
	 * the data area is going to be written in this path and we don't want to
	 * change the cloned skb's data section.
	 */
	if (unlikely(skb_cloned(skb))) {
		DEBUG_TRACE("%px: skb is a cloned skb\n", skb);

		if (unlikely(skb_shared(skb)) || unlikely(skb_unclone(skb, GFP_ATOMIC))) {
			DEBUG_WARN("Failed to unclone the cloned skb\n");
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_UNCLONE_FAILED);
			return 0;
		}

		/*
		 * Update the iph and udph pointers with the unclone skb's data area.
		 */
		iph = (struct ipv6hdr *)skb->data;
		if (first_frag) {
			udph = (struct udphdr *)(skb->data + ihl);
		}
	}

	/*
	 * Check if skb has enough headroom to write L2 headers
	 */
	if (unlikely(skb_headroom(skb) < cm->l2_hdr_size)) {
		DEBUG_WARN("%px: Not enough headroom: %u\n", skb, skb_headroom(skb));
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_NO_HEADROOM);
		return 0;
	}

	/*
	 * For PPPoE packets, match server MAC and session id
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_DECAP)) {
		struct ethhdr *eth;
		bool pppoe_match;

		if (unlikely(!sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS))) {
			DEBUG_TRACE("%px: PPPoE header not present in packet for PPPoE rule\n", skb);
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INCORRECT_PPPOE_PARSING);
			return 0;
		}

		eth = eth_hdr(skb);

		pppoe_match = (cm->pppoe_session_id == sfe_l2_pppoe_session_id_get(l2_info)) &&
				ether_addr_equal((u8*)cm->pppoe_remote_mac, (u8 *)eth->h_source);

		if (unlikely(!pppoe_match)) {
			DEBUG_TRACE("%px: PPPoE sessions ID %d and %d or MAC %pM and %pM did not match\n",
					skb, cm->pppoe_session_id, sfe_l2_pppoe_session_id_get(l2_info),
					cm->pppoe_remote_mac, eth->h_source);
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_INVALID_PPPOE_SESSION);
			return 0;
		}

		skb->protocol = htons(l2_info->protocol);
		this_cpu_inc(si->stats_pcpu->pppoe_decap_packets_forwarded64);
	} else if (unlikely(sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS))) {

		/*
		 * If packet contains PPPoE header but CME doesn't contain PPPoE flag yet we are exceptioning the packet to linux
		 */
		if (unlikely(!bridge_flow)) {
			DEBUG_TRACE("%px: CME doesn't contain PPPoE flag but packet has PPPoE header\n", skb);
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_PPPOE_NOT_SET_IN_CME);
			return 0;
		}

		/*
		 * For bridged flows when packet contains PPPoE header, restore the header back and forward to xmit interface
		 */
		__skb_push(skb, PPPOE_SES_HLEN);
		this_cpu_inc(si->stats_pcpu->pppoe_bridge_packets_forwarded64);
	}

	/*
	 * From this point on we're good to modify the packet.
	 */

	/*
	 * Multicast share the same check with unicast, from this point, they are going to
	 * divert.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_MULTICAST)) {
		sfe_ipv6_recv_multicast(si, skb, ihl, len, cm, l2_info, tun_outer, first_frag);
		return 1;
	}

	/*
	 * For PPPoE flows, add PPPoE header before L2 header is added.
	 * SFE + PPPOE flow is not supported with GSO, hence destroy the connection.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_ENCAP)) {
		if ((unlikely(skb_shinfo(skb)->gso_segs))) {
			struct sfe_ipv6_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv6_remove_connection(si, c);
			spin_unlock_bh(&si->lock);
			if (ret) {
				sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}

			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_GSO_NOT_SUPPORTED);
			return 0;
		}

		sfe_pppoe_add_header(skb, cm->pppoe_session_id, PPP_IPV6);
		this_cpu_inc(si->stats_pcpu->pppoe_encap_packets_forwarded64);
	}

	/*
	 * Set SKB packet type to PACKET_HOST
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PACKET_HOST)) {
		skb->pkt_type = PACKET_HOST;
	}

	/*
	 * Update DSCP
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
		sfe_ipv6_change_dsfield(iph, cm->dscp);
	}

	/*
	 * Decrement our hop_limit.
	 */
	if (likely(!bridge_flow)) {
		iph->hop_limit -= (u8)!tun_outer;
	}

	/*
	 * Enable HW csum if rx checksum is verified and xmit interface is CSUM offload capable.
	 * Note: If L4 csum at Rx was found to be incorrect, we (router) should use incremental L4 checksum here
	 * so that HW does not re-calculate/replace the L4 csum
	 */
	hw_csum = !!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_CSUM_OFFLOAD) && (skb->ip_summed == CHECKSUM_UNNECESSARY);

	/*
	 * Do we have to perform translations of the source address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
		u16 udp_csum;

		iph->saddr.s6_addr32[0] = cm->xlate_src_ip[0].addr[0];
		iph->saddr.s6_addr32[1] = cm->xlate_src_ip[0].addr[1];
		iph->saddr.s6_addr32[2] = cm->xlate_src_ip[0].addr[2];
		iph->saddr.s6_addr32[3] = cm->xlate_src_ip[0].addr[3];
		if (first_frag) {
			udph->source = cm->xlate_src_port;

			/*
			 * Do we have a non-zero UDP checksum?  If we do then we need
			 * to update it.
			 */
			if (unlikely(!hw_csum)) {
				udp_csum = udph->check;
				if (likely(udp_csum)) {
					u32 sum = udp_csum + cm->xlate_src_csum_adjustment;
					sum = (sum & 0xffff) + (sum >> 16);
					udph->check = (u16)sum;
				}
			}
		}
	}

	/*
	 * Do we have to perform translations of the destination address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
		u16 udp_csum;

		iph->daddr.s6_addr32[0] = cm->xlate_dest_ip[0].addr[0];
		iph->daddr.s6_addr32[1] = cm->xlate_dest_ip[0].addr[1];
		iph->daddr.s6_addr32[2] = cm->xlate_dest_ip[0].addr[2];
		iph->daddr.s6_addr32[3] = cm->xlate_dest_ip[0].addr[3];

		if (first_frag) {
			udph->dest = cm->xlate_dest_port;

			/*
			 * Do we have a non-zero UDP checksum?  If we do then we need
			 * to update it.
			 */
			if (unlikely(!hw_csum)) {
				udp_csum = udph->check;
				if (likely(udp_csum)) {
					u32 sum = udp_csum + cm->xlate_dest_csum_adjustment;
					sum = (sum & 0xffff) + (sum >> 16);
					udph->check = (u16)sum;
				}
			}
		}
	}

	/*
	 * If HW checksum offload is not possible, incremental L4 checksum is used to update the packet.
	 * Setting ip_summed to CHECKSUM_UNNECESSARY ensures checksum is not recalculated further in packet
	 * path.
	 */
	if (likely(hw_csum)) {
		skb->ip_summed = CHECKSUM_PARTIAL;
	}

	/*
	 * Update traffic stats.
	 */
	atomic_inc(&cm->rx_packet_count);
	atomic_add(len, &cm->rx_byte_count);

	xmit_dev = cm->xmit_dev;
	skb->dev = xmit_dev;

	/*
	 * For trustsec flows, add trustsec header before L2 header is added.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_INSERT_EGRESS_TRUSTSEC_SGT)) {
		sfe_trustsec_add_sgt(skb, &cm->egress_trustsec_hdr);
	}

	/*
	 * Check to see if we need to add VLAN tags
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_INSERT_EGRESS_VLAN_TAG)) {
		sfe_vlan_add_tag(skb, cm->egress_vlan_hdr_cnt, cm->egress_vlan_hdr);
	}

	/*
	 * Check to see if we need to write an Ethernet header.
	 */
	if (likely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
		if (unlikely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
			dev_hard_header(skb, xmit_dev, ntohs(skb->protocol),
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
		} else {
			/*
			 * For the simple case we write this really fast.
			 */
			struct ethhdr *eth = (struct ethhdr *)__skb_push(skb, ETH_HLEN);
			eth->h_proto = skb->protocol;
			ether_addr_copy((u8 *)eth->h_dest, (u8 *)cm->xmit_dest_mac);
			ether_addr_copy((u8 *)eth->h_source, (u8 *)cm->xmit_src_mac);
		}
	}

	/*
	 * Update priority and int_pri of skb.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
		skb->priority = cm->priority;
#if defined(SFE_PPE_QOS_SUPPORTED)
		skb_set_int_pri(skb, cm->int_pri);
#endif
	}

	/*
	 * Mark outgoing packet.
	 */
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_MARK)) {
		skb->mark = cm->mark;
		/*
		 * Update service class stats if SAWF is valid.
		 */
		if (likely(cm->sawf_valid)) {
			service_class_id = SFE_GET_SAWF_SERVICE_CLASS(cm->mark);
			sfe_ipv6_service_class_stats_inc(si, service_class_id, len);
		}
	}

	/*
	 * For the first packets, check if it could got fast xmit.
	 */
	if (unlikely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_FAST_XMIT_FLOW_CHECKED)
				&& (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_FAST_XMIT_DEV_ADMISSION))){
		cm->features = netif_skb_features(skb);
		if (likely(sfe_fast_xmit_check(skb, cm->features))) {
			cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_FAST_XMIT;
		}
		cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_FAST_XMIT_FLOW_CHECKED;
	}
	features = cm->features;

	fast_xmit = !!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_FAST_XMIT);

	this_cpu_inc(si->stats_pcpu->packets_forwarded64);
	/*
	 * We do per packet condition check before we could fast xmit the
	 * packet.
	 */
	if (likely(fast_xmit && dev_fast_xmit(skb, xmit_dev, features))) {
		this_cpu_inc(si->stats_pcpu->packets_fast_xmited64);
		return 1;
	}

	/*
	 * We're going to check for GSO flags when we transmit the packet so
	 * start fetching the necessary cache line now.
	 */
	prefetch(skb_shinfo(skb));

	/*
	 * Mark that this packet has been fast forwarded.
	 */
	skb->fast_forwarded = 1;

	/*
	 * TODO: Remove this check when fast qdisc support is added for 6.1 kernel
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))

	/*
	 * check if fast qdisc xmit is enabled and send the packet on its way.
	 */
	if (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_FAST_QDISC_XMIT) {
		if (likely(dev_fast_xmit_qdisc(skb, xmit_dev, cm->qdisc_xmit_dev))) {
			this_cpu_inc(si->stats_pcpu->packets_fast_qdisc_xmited64);
			return 1;
		}
	}
#endif

	dev_queue_xmit(skb);
	return 1;
}

/*
 * sfe_ipv6_frag_recv_udp()
 *	Handle UDP packet receives and forwarding.
 */
int sfe_ipv6_recv_udp_frag(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
			     unsigned int len, struct ipv6hdr *iph, unsigned int ihl, bool sync_on_find, struct sfe_l2_info *l2_info, bool tun_outer, struct frag_hdr *fhdr)
{
	struct udphdr *udph = NULL;
	struct sfe_ipv6_addr *src_ip;
	struct sfe_ipv6_addr *dest_ip;
	__be16 src_port;
	__be16 dest_port;
	__be32 frag_id;
	struct sfe_ipv6_connection_match *cm;
	struct sfe_ipv6_frag_id_entity *fie = NULL;
	struct sfe_ipv6_frag_id_element *fiel = NULL;
	bool first_frag = false;
	int ret = 0;
	struct sfe_ipv6_frag *sif = si->sif;

	DEBUG_TRACE("%px: sfe: sfe_ipv6_recv_udp_frag called. Soucrce IP %pI6 Destination IP %pI6, frag_id %d\n", skb, iph->saddr.s6_addr32, iph->daddr.s6_addr32, ntohl(fhdr->identification));

	/*
	 * Fragment header with MF and fragment offset field as 0, is a
	 * dummy fragment header.
	 */
	if (!(fhdr->frag_off & htons(IP6_OFFSET | IP6_MF))) {
		sfe_ipv6_exception_stats_inc(si,SFE_IPV6_EXCEPTION_EVENT_DUMMY_FRAGMENT);
		DEBUG_TRACE("%px: Dummy fragment\n", skb);
		return 0;
	}
	/*
	 * Read the IP address and port information. Read the IP header data first
	 * because we've almost certainly got that in the cache.  We may not yet have
	 * the UDP header cached though so allow more time for any prefetching.
	 */
	src_ip = (struct sfe_ipv6_addr *)iph->saddr.s6_addr32;
	dest_ip = (struct sfe_ipv6_addr *)iph->daddr.s6_addr32;
	frag_id = fhdr->identification;

	/*
	 * A reference will be taken on the frag_id_entity on successfull return.
	 */
	fie = sfe_ipv6_frag_id_entity_get(sif, src_ip, dest_ip, frag_id);
	if (!fie) {
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_FAILED_TO_GET_FRAG_ID_ENTITY);
		DEBUG_TRACE("%px: Failed to get fragment id entity\n", skb);
		return 0;
	}
	spin_lock_bh(&fie->lock);

	/*
	 * Head fragment (Fragment with offset 0).
	 */
	if (!(fhdr->frag_off & htons(IP6_OFFSET))) {
		/*
		 * Is our packet too short to contain a valid UDP header?
		 */
		if (!pskb_may_pull(skb, (sizeof(struct udphdr) + ihl))) {
			fie->state = SFE_IPV6_FRAG_FORWARD_DROP;
			spin_unlock_bh(&fie->lock);
			sfe_ipv6_frag_id_put(sif, fie);

			sfe_ipv6_exception_stats_inc(si,SFE_IPV6_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE);
			DEBUG_TRACE("%px: packet too short for UDP header\n", skb);
			return 0;
		}

		/*
		 * Read the port infomration from the head fragment.
		 */
		first_frag = true;
		udph = (struct udphdr *)(skb->data + ihl);
		src_port = udph->source;
		dest_port = udph->dest;
	} else {

		/*
		 * Intermediate Fragments.
		 */
		switch (fie->state) {
		case SFE_IPV6_FRAG_FORWARD_NEW:
		case SFE_IPV6_FRAG_FORWARD_BUF:

			/*
			 * Fragment id entity queue limit exceeded.
			 * Drop all the queued fragments and change the state of the
			 * fragment id entity to drop incoming fragments with the same
			 * fragment ID.
			 */
			if (fie->qlen == sif->qlen_max) {
				struct sfe_ipv6_frag_id_element *fiel_tmp;
				DEBUG_WARN("%px: Queue lenght exceeded max limit\n", skb);
				fie->state = SFE_IPV6_FRAG_FORWARD_DROP;
				list_for_each_entry_safe(fiel ,fiel_tmp, &fie->q_head, q_node) {
					kfree_skb(fiel->skb);
					list_del(&fiel->q_node);
					kfree(fiel);
					this_cpu_inc(si->stats_pcpu->fragment_dropped64);
				}

				spin_unlock_bh(&fie->lock);
				sfe_ipv6_frag_id_put(sif, fie);
				kfree_skb(skb);
				this_cpu_inc(si->stats_pcpu->fragment_dropped64);
				return 1;
			}

			/*
			 * Received intermediate fragment before the head fragment, store it in the queue.
			 */
			fie->state = SFE_IPV6_FRAG_FORWARD_BUF;
			fiel = (struct sfe_ipv6_frag_id_element *)kzalloc(sizeof(struct sfe_ipv6_frag_id_element), GFP_ATOMIC);
			if (!fiel) {
				fie->state = SFE_IPV6_FRAG_FORWARD_DROP;
				spin_unlock_bh(&fie->lock);
				sfe_ipv6_frag_id_put(sif, fie);
				kfree_skb(skb);
				this_cpu_inc(si->stats_pcpu->fragment_dropped64);
				return 1;
			}

			fiel->skb = skb;
			memcpy(&fiel->l2_info, l2_info, sizeof(struct sfe_l2_info));
			fiel->iph = iph;
			fiel->ihl = ihl;

			list_add_tail(&fiel->q_node, &fie->q_head);
			fie->qlen++;
			spin_unlock_bh(&fie->lock);
			sfe_ipv6_frag_id_put(sif, fie);
			return 1;

		case SFE_IPV6_FRAG_FORWARD_EXCEPTION:
			/*
			 * Exception the fragment to Linux.
			 */
			spin_unlock_bh(&fie->lock);
			sfe_ipv6_frag_id_put(sif, fie);
			this_cpu_inc(si->stats_pcpu->fragment_exception64);
			return 0;

		case SFE_IPV6_FRAG_FORWARD_DROP:
			/*
			 * Drop the fragment.
			 */
			spin_unlock_bh(&fie->lock);
			sfe_ipv6_frag_id_put(sif, fie);
			kfree_skb(skb);
			this_cpu_inc(si->stats_pcpu->fragment_dropped64);
			return 1;

		case SFE_IPV6_FRAG_FORWARD:
			/*
			 * Forward the fragment.
			 */
			src_port = fie->src_port;
			dest_port = fie->dest_port;
			break;

		default:
			sfe_ipv6_frag_id_put(sif, fie);
			spin_unlock_bh(&fie->lock);
			DEBUG_WARN("%px: Invalid state: %d\n", skb, fie->state);
			return 0;
		}
	}

	rcu_read_lock();

	/*
	 * Look for a connection match.
	 */
	cm = sfe_ipv6_find_connection_match_rcu(si, dev, IPPROTO_UDP, src_ip, src_port, dest_ip, dest_port);
	if (unlikely(!cm)) {
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_UDP_NO_CONNECTION);
		DEBUG_TRACE("%px:no connection found\n", skb);
		if (first_frag && (fie->state == SFE_IPV6_FRAG_FORWARD_BUF)) {

			/*
			 * If we have skbs queued for this 3 tuple, exception them.
			 */
			struct sfe_ipv6_frag_id_element *fiel_tmp;
			int cpu = smp_processor_id();
			struct sfe_ipv6_frag_exception *pcpu = per_cpu_ptr(sif->pcpu, cpu);

			/*
			 * To exception the packets,
			 * 1. We add all the fragments to a percpu exception queue.
			 * 2. Immediately Schedule a timer interrupt.
			 * 3. Timer call back function calls netif_receive_skb.
			 *
			 * When the packets come back to sfe, we will have the head
			 * fragmet, and all the fragments will be exceptioned to Linux.
			 */
			sfe_recv_undo_parse_l2(dev, skb, l2_info);

			/*
			 * Add the head skb to the per CPU exception queue
			 * followed by the intermediate fragments.
			 */
			skb_queue_tail(&pcpu->exception_queue, skb);
			list_for_each_entry_safe(fiel, fiel_tmp, &fie->q_head, q_node) {
				sfe_recv_undo_parse_l2(dev, fiel->skb, &fiel->l2_info);
				skb_queue_tail(&pcpu->exception_queue, fiel->skb);
				list_del(&fiel->q_node);
				kfree(fiel);
				fie->qlen--;
			}

			fie->state = SFE_IPV6_FRAG_FORWARD_EXCEPTION;
			spin_unlock_bh(&fie->lock);
			sfe_ipv6_frag_id_put(sif, fie);

			/*
			 * Schedule a timer.
			 */
			pcpu->timer.expires = jiffies;
			if (!timer_pending(&pcpu->timer)) {
				add_timer_on(&pcpu->timer, cpu);
			}

			return 1;
		}

		fie->state = SFE_IPV6_FRAG_FORWARD_EXCEPTION;
		spin_unlock_bh(&fie->lock);
		sfe_ipv6_frag_id_put(sif, fie);
		this_cpu_inc(si->stats_pcpu->fragment_exception64);
		return 0;
	}

	if (first_frag) {
		/*
		 * Update the information required for 5 tuple lookup.
		 */
		fie->src_port = src_port;
		fie->dest_port = dest_port;
		fie->protocol = IPPROTO_UDP;

		if (fie->state == SFE_IPV6_FRAG_FORWARD_BUF) {
			/*
			 * This is an out of order case where the head fragment arrived after
			 * the intermediate fragments. As a result of this, the intermediate fragments
			 * would be queued in SFE.
			 *
			 * Once we receive the head fragment, we try to forward all the fgragments in the queue.
			 * If a fragemnt in the queue cannot be forwarded, the remaining fragments
			 * in the queue will be dropped and we change the state of the fragment id
			 * entity to drop the new fragments that might come in later.
		 	 */
			struct sfe_ipv6_frag_id_element *fiel_tmp;
			bool can_forward_frag = true;
			list_for_each_entry_safe(fiel, fiel_tmp, &fie->q_head, q_node) {
				if (likely(can_forward_frag)) {
					ret = sfe_ipv6_frag_udp_forward(si, fiel->skb, dev, fiel->skb->len, fiel->iph, fiel->ihl, sync_on_find, &fiel->l2_info, tun_outer, cm, udph, false);
				}

				if (!ret) {
					kfree_skb(fiel->skb);
					this_cpu_inc(si->stats_pcpu->fragment_dropped64);
					can_forward_frag = false;
				} else {
					this_cpu_inc(si->stats_pcpu->fragment_forwarded64);
				}

				list_del(&fiel->q_node);
				kfree(fiel);
				fie->qlen--;
			}

			/*
			 * Intermediate fragment frowarding failed.
			 * Drop the first fragment as well.
			 */
			if (!ret) {
				fie->state = SFE_IPV6_FRAG_FORWARD_DROP;
				spin_unlock_bh(&fie->lock);
				rcu_read_unlock();
				kfree_skb(skb);
				sfe_ipv6_frag_id_put(sif, fie);
				this_cpu_inc(si->stats_pcpu->fragment_dropped64);
				return 1;
			}
		} else if (fie->state == SFE_IPV6_FRAG_FORWARD_DROP) {
				/*
				 * Out of oder case where the lenght of queued skbs
				 * exceeded the maximum limit and hence all the intermediadte fragments
				 * are already dropped.
				 *
				 * We need to drop the associated head fragment as well.
				 */
				spin_unlock_bh(&fie->lock);
				rcu_read_unlock();
				kfree_skb(skb);
				sfe_ipv6_frag_id_put(sif, fie);
				this_cpu_inc(si->stats_pcpu->fragment_dropped64);
				return 1;
		}
	}

	/*
	 * Forward the fragment.
	 */
	ret = sfe_ipv6_frag_udp_forward(si, skb, dev, len, iph, ihl, sync_on_find, l2_info, tun_outer, cm , udph, first_frag);
	if (!ret) {
		fie->state = SFE_IPV6_FRAG_FORWARD_DROP;
		spin_unlock_bh(&fie->lock);
		rcu_read_unlock();
		kfree_skb(skb);
		sfe_ipv6_frag_id_put(sif, fie);
		this_cpu_inc(si->stats_pcpu->fragment_dropped64);
		return 1;
	}

	/*
	 * Change the state of the fragment id entity to forward.
	 */
	fie->state = SFE_IPV6_FRAG_FORWARD;
	spin_unlock_bh(&fie->lock);
	rcu_read_unlock();
	sfe_ipv6_frag_id_put(sif, fie);
	this_cpu_inc(si->stats_pcpu->fragment_forwarded64);
	return 1;
}

/*
 * sfe_ipv6_frag_free()
 *	Free fragment handler.
 */
void sfe_ipv6_frag_free(struct sfe_ipv6_frag *sif) {

	free_percpu(sif->pcpu);
	kfree(sif);
}

/*
 * sfe_ipv6_frag_init()
 *	Initialize fragment handling related objects.
 */
void sfe_ipv6_frag_init(struct sfe_ipv6_frag *sif, struct sfe_ipv6 *si)
{
	int cpu;

	for_each_online_cpu(cpu) {
		struct sfe_ipv6_frag_exception *pcpu = per_cpu_ptr(sif->pcpu, cpu);
		timer_setup(&pcpu->timer, sfe_ipv6_frag_process_exception_queue, 0);
		skb_queue_head_init(&pcpu->exception_queue);
	}

	/*
	 * Initialize the hash table.
	 */
	sif->si = si;
	sfe_ipv6_frag_hash_init(sif , ARRAY_SIZE(sif->frag_id_hashtable));
	spin_lock_init(&sif->lock);

	/*
	 * Random bytes to be used for calculating 3 tuple hash.
	 */
	get_random_bytes(&sif->hash_rand, sizeof(sif->hash_rand));

	/*
	 * Initialize the global lru list.
	 */
	INIT_LIST_HEAD(&sif->lru_head);

	/*
	 * Setup periodic timer to free timedout fragment entries.
	 */
	sif->timeout = SFE_IPV6_FRAG_ID_TIMEOUT_DEFAULT;
	timer_setup(&sif->timer, sfe_ipv6_frag_timer_cb, 0);
	mod_timer(&sif->timer, SFE_IPV6_FRAG_ID_TIMEOUT_TIMER_FREQ);

	sif->qlen_max = SFE_IPV6_FRAG_ID_QUEUE_LIMIT;
	sif->high_tresh = SFE_IPV6_FRAG_ID_TOTAL_HIGH_TRESH;
	sif->frag_id_entity_counter = 0;

	/*
	 * Enable fragment forwarding by default.
	 */
	si->fragment_forwarding_enable = 1;
}

/*
 * sfe_ipv6_frag_alloc()
 *	Allocate memory for fragment handler.
 */
struct sfe_ipv6_frag *sfe_ipv6_frag_alloc(struct sfe_ipv6 *si) {
	struct sfe_ipv6_frag *sif = (struct sfe_ipv6_frag *)kzalloc(sizeof(struct sfe_ipv6_frag), GFP_KERNEL);
	if (!sif) {
		DEBUG_ERROR("%px:Failed to allocate memory for V6 Fragment handler\n", si);
		return NULL;
	}

	sif->pcpu = alloc_percpu(struct sfe_ipv6_frag_exception);
	if (!sif->pcpu) {
		DEBUG_WARN("%p: Failed to allocate per cpu fragment exception objects\n", si);
		kfree(sif);
		return NULL;
	}

	return sif;
}

/*
 * sfe_ipv6_frag_exit()
 *	Free up resources help by fragment handler.
 */
void sfe_ipv6_frag_exit(struct sfe_ipv6_frag *sif)
{
	int cpu;

	/*
	 * Free percpu exception timer and queue.
	 */
	for_each_online_cpu(cpu) {
		struct sfe_ipv6_frag_exception *pcpu = per_cpu_ptr(sif->pcpu, cpu);
		del_timer(&pcpu->timer);
		skb_queue_purge(&pcpu->exception_queue);
	}

	/*
	 * Delete the timer.
	 */
	del_timer(&sif->timer);

	/*
	 * Free the fragment entries.
	 */
	sfe_ipv6_frag_free_id_all(sif);
}
