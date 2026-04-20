/*
 *  H-Default table
 *  QCA HyFi Netfilter
 *
 * Copyright (c) 2012-2014, 2016, The Linux Foundation. All rights reserved.
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

#define DEBUG_LEVEL HYFI_NF_DEBUG_LEVEL

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include "hyfi_hdtbl.h"

static struct kmem_cache *hyfi_hdtbl_cache __read_mostly;

static int hdtbl_insert(struct hyfi_net_bridge *br,
		struct net_bridge_port *dst_udp, struct net_bridge_port *dst_other,
		const u_int8_t *addr, const u_int8_t *id, u_int32_t static_entry);

u_int32_t hdtbl_salt __read_mostly;

static void hdtbl_rcu_free(struct rcu_head *head)
{
	struct net_hdtbl_entry *ent = container_of(head, struct net_hdtbl_entry, rcu);
	kmem_cache_free(hyfi_hdtbl_cache, ent);
}

static inline void hdtbl_delete(struct net_hdtbl_entry *hd)
{
	hlist_del_rcu(&hd->hlist);
	call_rcu(&hd->rcu, hdtbl_rcu_free);
}

/* Completely flush all the entries in H default table.*/
void hyfi_hdtbl_flush(struct hyfi_net_bridge *br)
{
	u_int32_t i;

	spin_lock_bh(&br->hash_hd_lock);
	for (i = 0; i < HD_HASH_SIZE; i++) {
		struct net_hdtbl_entry *hd;
		struct hlist_node *h, *n;
		os_hlist_for_each_entry_safe(hd, h, n, &br->hash_hd[i], hlist)	{
			hdtbl_delete(hd);
		}
	}

	spin_unlock_bh(&br->hash_hd_lock);
}

/* No locking or refcounting, assumes caller has no preempt (rcu_read_lock) */
struct net_hdtbl_entry *hyfi_hdtbl_get(struct hyfi_net_bridge *br,
		const u_int8_t *addr)
{
	struct hlist_node *h;
	struct net_hdtbl_entry *hd;

	os_hlist_for_each_entry_rcu(hd, h, &br->hash_hd[hdtbl_mac_hash(addr)], hlist) {
		if (!compare_ether_addr(hd->addr.addr, addr)) {
			return hd;
		}
	}

	return NULL ;
}

/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int hyfi_hdtbl_fillbuf(struct hyfi_net_bridge *br, void *buf, u_int32_t buf_len,
		u_int32_t skip, u_int32_t *bytes_written)
{
	struct __hdtbl_entry *hde = buf;
	u_int32_t i, num = 0, num_entries;
	int ret = 0;
	struct hlist_node *h;
	struct net_hdtbl_entry *hd;

	memset(buf, 0, buf_len);
	num_entries = buf_len / sizeof(struct __hdtbl_entry);

	rcu_read_lock();
	for (i = 0; i < HD_HASH_SIZE; i++) {
		os_hlist_for_each_entry_rcu(hd, h, &br->hash_hd[i], hlist)	{
			if (num >= num_entries) {
				ret = -EAGAIN;
				goto out;
			}
			if (skip) {
				skip--;
				continue;
			}

			/* convert from internal format to API */
			memcpy(hde->mac_addr, hd->addr.addr, ETH_ALEN);
			memcpy(hde->id, hd->id.addr, ETH_ALEN);

			if (hd->dst_udp)
				hde->udp_port = hd->dst_udp->dev->ifindex;

			if (hd->dst_other)
				hde->other_port = hd->dst_other->dev->ifindex;

			if (hyfi_hd_has_flag(hd, HYFI_HDTBL_STATIC_ENTRY))
				hde->static_entry = 1;
			else
				hde->static_entry = 0;

			++hde;
			++num;
		}
	}

	out:
	rcu_read_unlock();
	if (bytes_written)
		*bytes_written = num * sizeof(struct __hdtbl_entry);

	return ret;
}

static inline struct net_hdtbl_entry *hdtbl_find_rcu(struct hlist_head *head,
		const u_int8_t *addr)
{
	struct hlist_node *h;
	struct net_hdtbl_entry *hd;

	os_hlist_for_each_entry_rcu(hd, h, head, hlist)
	{
		if (!compare_ether_addr(hd->addr.addr, addr))
			return hd;
	}
	return NULL ;
}

static inline struct net_hdtbl_entry *__hdtbl_find(struct hlist_head *head,
		const u_int8_t *addr)
{
	struct hlist_node *h;
	struct net_hdtbl_entry *hd;

	os_hlist_for_each_entry(hd, h, head, hlist)
	{
		if (!compare_ether_addr(hd->addr.addr, addr))
			return hd;
	}
	return NULL ;
}

struct net_hdtbl_entry *hyfi_hdtbl_find(struct hyfi_net_bridge *br,
		const u_int8_t *addr)
{
	struct hlist_node *h;
	struct net_hdtbl_entry *hd;

	if (!br || !addr)
		return NULL;

	os_hlist_for_each_entry(hd, h, &br->hash_hd[hdtbl_mac_hash(addr)], hlist) {
		if (!compare_ether_addr(hd->addr.addr, addr)) {
			return hd;
		}
	}

	return NULL;
}

static struct net_hdtbl_entry *hdtbl_create(struct hyfi_net_bridge * hyfi_br,
		struct hlist_head *head,
		struct net_bridge_port *dst_udp, struct net_bridge_port *dst_other,
		const u_int8_t *addr, const u_int8_t *id, u_int32_t static_entry)
{
	struct net_hdtbl_entry *hd;

	hd = kmem_cache_alloc(hyfi_hdtbl_cache, GFP_ATOMIC);
	if (hd) {
		memcpy(hd->addr.addr, addr, ETH_ALEN);
		memcpy(hd->id.addr, id, ETH_ALEN);

		hd->dst_udp = dst_udp;
		hd->dst_other = dst_other;
		hd->flags = 0;
		hd->hyfi_br = hyfi_br;
		if (static_entry) {
			hyfi_hd_set_flag(hd, HYFI_HDTBL_STATIC_ENTRY);
		} else {
			hyfi_hd_clear_flag(hd, HYFI_HDTBL_STATIC_ENTRY);
		}
		hlist_add_head_rcu(&hd->hlist, head);
	}

	return hd;
}

static int hdtbl_insert(struct hyfi_net_bridge *br,
		struct net_bridge_port *dst_udp, struct net_bridge_port *dst_other,
		const u_int8_t *addr, const u_int8_t *id, u_int32_t static_entry)
{
	struct hlist_head *head = &br->hash_hd[hdtbl_mac_hash(addr)];
	struct net_hdtbl_entry *hd;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	hd = __hdtbl_find(head, addr);
	if (hd) {
		hdtbl_delete(hd);
	}

	if (!hdtbl_create(br, head, dst_udp, dst_other, addr, id, static_entry))
		return -ENOMEM;

	return 0;
}

int hyfi_hdtbl_insert(struct hyfi_net_bridge *br, struct net_device *br_dev,
	struct __hdtbl_entry *hde)
{
	int ret = -EINVAL;
	struct net_device *dev_udp, *dev_other;

	do {
		struct net_bridge_port *br_port_u, *br_port_o;

		dev_udp = dev_get_by_index(dev_net(br_dev), hde->udp_port);
		if (dev_udp == NULL )
			break;

		dev_other = dev_get_by_index(dev_net(br_dev), hde->other_port);
		if (dev_other == NULL ) {
			dev_put(dev_udp);
			break;
		}

		br_port_u = hyfi_br_port_get(dev_udp);
		br_port_o = hyfi_br_port_get(dev_other);

		if (!br_port_u || !br_port_o) {
			dev_put(dev_udp);
			dev_put(dev_other);
			break;
		}

		spin_lock_bh(&br->hash_hd_lock);
		ret = hdtbl_insert(br, br_port_u, br_port_o, hde->mac_addr, hde->id,
				hde->static_entry);
		spin_unlock_bh(&br->hash_hd_lock);
		dev_put(dev_udp);
		dev_put(dev_other);
	} while (false);
	return ret;
}

int hyfi_hdtbl_delete_byid(struct hyfi_net_bridge *br, const u_int8_t *id)
{
	u_int32_t i;

	if (!is_valid_ether_addr(id))
		return -EINVAL;

	spin_lock_bh(&br->hash_hd_lock);
	for (i = 0; i < HD_HASH_SIZE; i++) {
		struct net_hdtbl_entry *hd;
		struct hlist_node *h, *n;

		os_hlist_for_each_entry_safe(hd, h, n, &br->hash_hd[i], hlist)	{
			if (!compare_ether_addr(hd->id.addr, id))
				hdtbl_delete(hd);
		}
	}
	spin_unlock_bh(&br->hash_hd_lock);

	return 0;
}

int hyfi_hdtbl_delete(struct hyfi_net_bridge *br, const u_int8_t *addr)
{
	struct hlist_head *head;
	struct net_hdtbl_entry *hd;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	spin_lock_bh(&br->hash_hd_lock);
	head = &br->hash_hd[hdtbl_mac_hash(addr)];
	hd = __hdtbl_find(head, addr);

	if (hd) {
		hdtbl_delete(hd);
	}
	spin_unlock_bh(&br->hash_hd_lock);
	return 0;
}

int hyfi_hdtbl_update(struct hyfi_net_bridge *br, struct net_device *br_dev,
	struct __hdtbl_entry *hde)
{
	struct hlist_head *head = &br->hash_hd[hdtbl_mac_hash(hde->mac_addr)];
	struct net_hdtbl_entry *hd;
	struct net_device *dev_udp, *dev_other;
	struct net_bridge_port *br_port_u;
	struct net_bridge_port *br_port_o;

	dev_udp = dev_get_by_index(dev_net(br_dev), hde->udp_port);
	if(!dev_udp) {
		return -EINVAL;
	}
	dev_other = dev_get_by_index(dev_net(br_dev), hde->other_port);
	if(!dev_other) {
		dev_put(dev_udp);
		return -EINVAL;
	}

	br_port_u = hyfi_br_port_get(dev_udp);
	br_port_o = hyfi_br_port_get(dev_other);

	if(!br_port_u || !br_port_o) {
		dev_put(dev_udp);
		dev_put(dev_other);
		return -EINVAL;
	}

	spin_lock_bh(&br->hash_hd_lock);

	hd = __hdtbl_find(head, hde->mac_addr);
	if (likely(hd)) {
		/* fastpath: update of existing entry */
		hd->dst_udp = br_port_u;
		hd->dst_other = br_port_o;
		memcpy(hd->id.addr, hde->id, ETH_ALEN);
		if (hde->static_entry)
			hyfi_hd_set_flag(hd, HYFI_HDTBL_STATIC_ENTRY);
		else
			hyfi_hd_clear_flag(hd, HYFI_HDTBL_STATIC_ENTRY);
	} else {
		hd = hdtbl_create(br, head, br_port_u, br_port_o, hde->mac_addr, hde->id,
				hde->static_entry);
	}

	spin_unlock_bh(&br->hash_hd_lock);
	dev_put(dev_udp);
	dev_put(dev_other);

	if (hd == NULL )
		return -ENOMEM;
	return 0;
}

/* Flush all entries refering to a specific port.
 *
 */
void hyfi_hdtbl_delete_by_port(struct hyfi_net_bridge *br,
		const struct net_bridge_port *p)
{
	u_int32_t i;

	spin_lock_bh(&br->hash_hd_lock);
	for (i = 0; i < HD_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash_hd[i]) {
			struct net_hdtbl_entry
			*hd = hlist_entry(h, struct net_hdtbl_entry, hlist);

			if (hd->dst_udp == p || hd->dst_other == p)
				hdtbl_delete(hd);
		}
	}
	spin_unlock_bh(&br->hash_hd_lock);
}

int __init hyfi_hdtbl_init(void)
{
	hyfi_hdtbl_cache = kmem_cache_create("hyfi_hd_table",
			sizeof(struct net_hdtbl_entry),
			0,
			SLAB_HWCACHE_ALIGN, NULL);

	if (!hyfi_hdtbl_cache)
		return -ENOMEM;

	get_random_bytes(&hdtbl_salt, sizeof(hdtbl_salt));
	return 0;
}

void hyfi_hdtbl_free(void)
{
	if (hyfi_hdtbl_cache) {
		kmem_cache_destroy(hyfi_hdtbl_cache);
		hyfi_hdtbl_cache = NULL;
	}
}

void hyfi_hdtbl_fini(struct hyfi_net_bridge *br)
{
	u_int32_t i;

	/* Wait for RCU grace period to finish. No other RCU access
	 * should happen now, once the callbacks were already uninstalled,
	 * and the bridge finished the processing of the last packet. */
	synchronize_rcu();

	/* Completely flush all the entries in H default table.*/
	spin_lock_bh(&br->hash_hd_lock);

	for (i = 0; i < HD_HASH_SIZE; i++) {
		struct net_hdtbl_entry *hd;
		struct hlist_node *h, *n;
		os_hlist_for_each_entry_safe(hd, h, n, &br->hash_hd[i], hlist)	{
			hlist_del_rcu(&hd->hlist);
			kmem_cache_free(hyfi_hdtbl_cache, hd);
		}
	}

	spin_unlock_bh(&br->hash_hd_lock);
}
