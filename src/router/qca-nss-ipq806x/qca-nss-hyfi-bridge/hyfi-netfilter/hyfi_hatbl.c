/*
 *  H-Active table
 *  QCA HyFi Netfilter
 *
 * Copyright (c) 2012-2016, The Linux Foundation. All rights reserved.
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
#include "hyfi_hatbl.h"
#include "hyfi_hdtbl.h"
#include "hyfi_api.h"
#include "hyfi_netlink.h"
#include "hyfi_aggr.h"
#include "hyfi_hash.h"
#include "hyfi_bridge.h"

static struct kmem_cache *hyfi_hatbl_cache __read_mostly;

static inline int has_expired(const struct hyfi_net_bridge *br,
		const struct net_hatbl_entry *ha)
{
	return time_before_eq((unsigned long)(ha->last_access + br->hatbl_aging_time), jiffies);
}

static void hatbl_rcu_free(struct rcu_head *head)
{
	struct net_hatbl_entry *ha = container_of(head, struct net_hatbl_entry, rcu);

	if (!ha)
		return;

	hyfi_psw_flush_track_q(&ha->psw_stm_entry);
	hyfi_psw_flush_buf_q(ha);
	hyfi_aggr_flush(ha);

	kmem_cache_free(hyfi_hatbl_cache, ha);
}

static inline void hatbl_delete(struct hyfi_net_bridge *br,
		struct net_hatbl_entry *ha)
{
	br->ha_entry_cnt--;
	hlist_del_rcu(&ha->hlist);

	call_rcu(&ha->rcu, hatbl_rcu_free);
}

/* Clean up the expired entries in ha table.*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
void hyfi_hatbl_cleanup(struct timer_list *t)
#else
void hyfi_hatbl_cleanup(unsigned long _data)
#endif
{
	u_int32_t i, aging = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct hyfi_net_bridge *br = from_timer(br, t, hatbl_timer);
#else
	struct hyfi_net_bridge *br = (struct hyfi_net_bridge *) _data;
#endif
	unsigned long this_timer, next_timer = jiffies
		+ msecs_to_jiffies(HYFI_HACTIVE_TBL_AGING_TIME);

	spin_lock_bh(&br->hash_ha_lock);
	for (i = 0; i < HA_HASH_SIZE; i++) {
		struct net_hatbl_entry *ha;
		struct hlist_node *h, *n;

		os_hlist_for_each_entry_safe(ha, h, n, &br->hash_ha[i], hlist)	{
			this_timer = ha->last_access + br->hatbl_aging_time;
			if (time_before_eq(this_timer, jiffies)) {
				hatbl_delete(br, ha);
				aging++;
			} else if (time_before(this_timer, next_timer))
				next_timer = this_timer;
		}
	}

	spin_unlock_bh(&br->hash_ha_lock);

	if (aging) {
		hyfi_netlink_event_send(br, HYFI_EVENT_AGEOUT_HA_ENTRIES, 0, NULL);
	}

	/* Add HZ/4 to ensure we round the jiffies upwards to be after the next
	 * timer, otherwise we might round down and will have no-op run. */
	mod_timer(&br->hatbl_timer, round_jiffies(next_timer + HZ / 4));
}

/* Completely flush all entries in ha table.*/
void hyfi_hatbl_flush(struct hyfi_net_bridge *br)
{
	u_int32_t i;

	spin_lock_bh(&br->hash_ha_lock);
	for (i = 0; i < HA_HASH_SIZE; i++) {
		struct net_hatbl_entry *ha;
		struct hlist_node *h, *n;
		os_hlist_for_each_entry_safe (ha, h, n, &br->hash_ha[i], hlist) {
			hatbl_delete(br, ha);
		}
	}

	spin_unlock_bh(&br->hash_ha_lock);
}

/* Flush all entries refering to a specific port.
 *
 */
void hyfi_hatbl_delete_by_port(struct hyfi_net_bridge *br,
		const struct net_bridge_port *p)
{
	u_int32_t i;

	spin_lock_bh(&br->hash_ha_lock);
	for (i = 0; i < HA_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe (h, g, &br->hash_ha[i])	{
			struct net_hatbl_entry *ha = hlist_entry(h, struct net_hatbl_entry, hlist);
			if (ha->dst != p)
				continue;

			hatbl_delete(br, ha);
		}
	}
	spin_unlock_bh(&br->hash_ha_lock);
}

/*
 * Fill buffer with ha table records in
 * the API format.
 */

static void hatbl_fillbuf(struct net_hatbl_entry *ha, struct __hatbl_entry *hae)
{
        if(!ha || !ha->dst || !ha->dst->dev) {
             DEBUG_ERROR("hyfi: ADD_HA-entry is NULL\n");
             return;
        }
	memcpy(hae->da, ha->da.addr, ETH_ALEN);
	memcpy(hae->sa, ha->sa.addr, ETH_ALEN);
	memcpy(hae->id, ha->id.addr, ETH_ALEN);

	if (hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY)) {
		u_int32_t i;

		for (i = 0; i < HYFI_AGGR_MAX_IFACES; i++) {
			hae->port_list[i].port =
					ha->iface_info[i].dst ?
							ha->iface_info[i].dst->dev->ifindex : ~0;
			hae->port_list[i].quota = ha->iface_info[i].packet_quota;
		}

		hae->aggr_entry = 1;
	} else {
		hae->port_list[0].port = ha->dst ? ha->dst->dev->ifindex : ~0;
		hae->aggr_entry = 0;
	}

	hae->age = (u_int32_t)hyfi_hatbl_calculate_elapsed_time(jiffies, ha->last_access);
	hae->age = jiffies_to_msecs(hae->age) / 1000; /* sec */
	hae->num_packets = ha->num_packets;
	hae->num_bytes = ha->num_bytes;
	hae->action = ha->action;
	hae->hash = ha->hash;
	hae->sub_class = ha->sub_class;
	hae->local = ha->local;
	hae->priority = ha->priority;
	hae->create_time = jiffies_to_msecs(ha->create_time) / 1000; /* sec */
	if (hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_STATIC_ENTRY))
		hae->static_entry = 1;
	else
		hae->static_entry = 0;

	if (hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_ACCL_ENTRY)) {
		hae->accl_entry = 1;
		hae->serial = ha->ecm_serial;
		hae->reserved = ha->rate;
	} else {
		hae->accl_entry = 0;
		hae->serial = 0;
		hae->reserved = 0;
	}

}

int hyfi_hatbl_fillbuf(struct hyfi_net_bridge *br, void *buf, u_int32_t buf_len,
		u_int32_t skip, u_int32_t *bytes_written, u_int32_t *bytes_needed)
{
	struct __hatbl_entry *hae = buf;
	u_int32_t i, total = 0, num = 0, num_entries;
	int ret = 0;
	struct hlist_node *h;
	struct net_hatbl_entry *ha;

	memset(buf, 0, buf_len);

	num_entries = buf_len / sizeof(struct __hatbl_entry);
	rcu_read_lock();
	for (i = 0; i < HA_HASH_SIZE; i++) {
		os_hlist_for_each_entry_rcu(ha, h, &br->hash_ha[i], hlist)	{
			if (hyfi_ha_has_flag(ha,
					HYFI_HACTIVE_TBL_TRACKED_ENTRY
							| HYFI_HACTIVE_TBL_AGGR_RX_ENTRY))
				continue;
			total++;
			if (num >= num_entries) {
				ret = -EAGAIN;
				continue;
			}

			if (skip) {
				--skip;
				continue;
			}

			hatbl_fillbuf(ha, hae);
			if (!ha->num_bytes) {
				ha->rate = 0;
			}
			ha->num_packets = 0;
			ha->num_bytes = 0;

			++hae;
			++num;
		}
	}

	rcu_read_unlock();
	if (bytes_written)
		*bytes_written = num * sizeof(struct __hatbl_entry);

	if (bytes_needed) {
		if (ret == -EAGAIN)
			*bytes_needed = total * sizeof(struct __hatbl_entry);
		else
			*bytes_needed = 0;
	}

	return ret;
}

static inline struct net_hatbl_entry *hatbl_find_rcu(struct hlist_head *head,
		const u_int8_t *da, u_int32_t sub_class, u_int32_t priority)
{
	struct hlist_node *h;
	struct net_hatbl_entry *ha;

	os_hlist_for_each_entry_rcu(ha, h, head, hlist) {
		if ((ha->sub_class == sub_class) && (ha->priority == priority)
				&& !compare_ether_addr(ha->da.addr, da)) {
			ha->last_access = jiffies;
			return ha;
		}
	}

	return NULL ;
}

static inline struct net_hatbl_entry *__hatbl_find(struct hlist_head *head,
		const u_int8_t *da, u_int32_t sub_class, u_int32_t priority)
{
	struct hlist_node *h;
	struct net_hatbl_entry *ha;

	os_hlist_for_each_entry(ha, h, head, hlist) {
		if ((ha->sub_class == sub_class) && (ha->priority == priority)
				&& !compare_ether_addr(ha->da.addr, da)) {
			ha->last_access = jiffies;
			return ha;
		}
	}

	return NULL ;
}

struct net_hatbl_entry *hatbl_find(struct hyfi_net_bridge *br, u_int32_t hash,
		const unsigned char *da, u_int32_t sub_class, u_int32_t priority)
{
	struct hlist_node *h;
	struct net_hatbl_entry *ha;

	if(!br)
		return NULL;

	os_hlist_for_each_entry(ha, h, &br->hash_ha[hash], hlist) {
		if ((ha->sub_class == sub_class) && (ha->priority == priority)
				&& !compare_ether_addr(ha->da.addr, da)) {
			ha->last_access = jiffies;
			return ha;
		}
	}

	return NULL ;
}

struct net_hatbl_entry *hatbl_find_ecm(struct hyfi_net_bridge *br, u_int32_t hash,
		u_int32_t ecm_serial, const unsigned char *da)
{
	struct hlist_node *h;
	struct net_hatbl_entry *ha;

	if(!br)
		return NULL;

	os_hlist_for_each_entry(ha, h, &br->hash_ha[hash], hlist) {
		if (ha->ecm_serial == ecm_serial &&
			hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_SERIAL_VALID) &&
			!compare_ether_addr(ha->da.addr, da)) {
			ha->last_access = jiffies;
			return ha;
		}
	}

	return NULL ;
}

static struct net_hatbl_entry *hatbl_create(struct hyfi_net_bridge *br,
		u_int32_t hash, struct net_bridge_port *dst, const u_int8_t *sa,
		const u_int8_t *da, const u_int8_t *id, u_int32_t sub_class,
		u_int32_t priority, u_int32_t static_entry)
{
	struct hlist_head *head = &br->hash_ha[hash];
	struct net_hatbl_entry *ha = NULL;

	if (likely(br->ha_entry_cnt >= HYFI_HACTIVE_TBL_SIZE))
		return NULL ;

	if (!(ha = kmem_cache_alloc(hyfi_hatbl_cache, GFP_ATOMIC)))
		return NULL ;

	memset(ha, 0, sizeof(struct net_hatbl_entry));
	memcpy(ha->da.addr, da, ETH_ALEN);
	memcpy(ha->sa.addr, sa, ETH_ALEN);
	memcpy(ha->id.addr, id, ETH_ALEN);

	ha->dst = dst;
	ha->sub_class = sub_class;
	ha->last_access = jiffies;
	ha->create_time = jiffies;
	ha->priority = priority;
	ha->hash = hash;
	ha->ecm_serial = UINT_MAX;
	ha->hyfi_br = br;

	if (static_entry) {
		hyfi_ha_set_flag(ha, HYFI_HACTIVE_TBL_STATIC_ENTRY);
	}

	hyfi_psw_stm_init(br, ha);

	ha->iface_info[0].dst = dst;
	ha->aggr_seq_data.aggr_cur_iface = HYFI_AGGR_MAX_IFACE;
	spin_lock_init(&ha->aggr_lock);

	hlist_add_head_rcu(&ha->hlist, head);
	br->ha_entry_cnt++;

	return ha;
}

/**
 * @brief Create a new H-Active entry, deleting a matching entry
 *        if it already exists (ensuring the new entry is
 *        unique)
 *
 * Note that this function should only be called if we believe a
 * new entry should be created.  However, since no lock is held
 * between when the H-Active entry is first checked for, and
 * when this function is called, it's possible that due to a
 * race condition, an entry may have been created in the
 * intervening time.
 *
 * @param [in] br  Hy-Fi bridge
 * @param [in] hash  hash for the entry
 * @param [in] dst  destination port
 * @param [in] sa  source MAC address
 * @param [in] da  destination MAC address
 * @param [in] id  next-hop MAC address
 * @param [in] sub_class  traffic type (UDP or other)
 * @param [in] priority  packet priority
 * @param [in] static_entry  if new H-Active entry is static,
 *                           set to 1
 *
 * @return struct net_hatbl_entry*
 */
static struct net_hatbl_entry *hatbl_find_before_create(struct hyfi_net_bridge *br,
		u_int32_t hash, struct net_bridge_port *dst, const u_int8_t *sa,
		const u_int8_t *da, const u_int8_t *id, u_int32_t sub_class,
		u_int32_t priority, u_int32_t static_entry)
{
	struct hlist_head *head = &br->hash_ha[hash];
	struct net_hatbl_entry *ha = NULL;

	if (unlikely(br->ha_entry_cnt >= HYFI_HACTIVE_TBL_SIZE)) {
		DEBUG_WARN("hyfi: Unable to add entry 0x%02x - max table size exceeded\n",
			hash);
		return NULL;
	}

	ha = hatbl_find_rcu(head, da, sub_class, priority);

	if (unlikely(ha)) {
		DEBUG_WARN("hyfi: H-Active entry 0x%02x exists\n", hash);
		hatbl_delete(br, ha);
	}

	ha = hatbl_create(br, hash, dst, sa, da, id,
		sub_class, priority, static_entry);

	if (!ha) {
		DEBUG_WARN("hyfi: Unable to create H-Active entry 0x%02x\n", hash);
	}

	return ha;
}

/* for test purpose, only called from netlink socket */
int hyfi_hatbl_addentry(struct hyfi_net_bridge *br, struct __hatbl_entry *hae)
{
	int ret = 0;
	struct hlist_head *head = &br->hash_ha[hae->hash];
	struct net_hatbl_entry *ha = NULL;
	struct net_device *dev = dev_get_by_index(&init_net,
			hae->port_list[0].port);

	do {
		struct net_bridge_port *br_port = hyfi_br_port_get(dev);

		if (!dev || !br_port) {
			ret = -EINVAL;
			break;
		}

		spin_lock_bh(&br->hash_ha_lock);
		ha = __hatbl_find(head, hae->da, hae->sub_class, hae->priority);
		if (unlikely(ha)) {
			hatbl_delete(br, ha);
		}

		if (!hatbl_create(br, hae->hash, br_port, hae->sa, hae->da, hae->id,
				hae->sub_class, hae->priority, hae->static_entry))
			ret = -ENOMEM;

		spin_unlock_bh(&br->hash_ha_lock);
	} while (false);

	if (dev)
		dev_put(dev);

	return ret;
}

struct net_hatbl_entry* hyfi_hatbl_insert(struct hyfi_net_bridge *br,
		u_int32_t hash, u_int32_t sub_class, struct net_hdtbl_entry *hd,
		u_int32_t priority, const u_int8_t* sa)
{
	struct net_hatbl_entry *ha = NULL;

	spin_lock(&br->hash_ha_lock);
	ha = hatbl_find_before_create(br, hash,
		sub_class == HYFI_TRAFFIC_CLASS_UDP ?
			hd->dst_udp : hd->dst_other, sa, hd->addr.addr,
		hd->id.addr, sub_class, priority,
		hyfi_hd_has_flag(hd, HYFI_HDTBL_STATIC_ENTRY));

	spin_unlock(&br->hash_ha_lock);

	if (ha) {
		hyfi_netlink_event_send(ha->hyfi_br, HYFI_EVENT_ADD_HA_ENTRY,
				sizeof(struct __hatbl_entry), ha);
	}
	return ha;
}

struct net_hatbl_entry* hyfi_hatbl_insert_ecm_classifier(struct hyfi_net_bridge *br,
		u_int32_t hash, u_int32_t sub_class, struct net_hdtbl_entry *hd,
		u_int32_t priority, const u_int8_t* sa, u_int32_t ecm_serial)
{
	struct net_hatbl_entry *ha = NULL;

	spin_lock_bh(&br->hash_ha_lock);
	ha = hatbl_find_before_create(br, hash,
		sub_class == HYFI_TRAFFIC_CLASS_UDP ?
			hd->dst_udp : hd->dst_other, sa, hd->addr.addr,
		hd->id.addr, sub_class, priority,
		hyfi_hd_has_flag(hd, HYFI_HDTBL_STATIC_ENTRY));

	if (ha) {
		ha->ecm_serial = ecm_serial;

		hyfi_netlink_event_send(ha->hyfi_br, HYFI_EVENT_ADD_HA_ENTRY,
				sizeof(struct __hatbl_entry), ha);
	} else {
		spin_unlock_bh(&br->hash_ha_lock);
	}
	return ha;
}


struct net_hatbl_entry * hyfi_hatbl_find_tracked_entry(
		struct hyfi_net_bridge *br, u_int32_t hash, const u_int8_t *mac_addr,
		u_int32_t sub_class, u_int32_t priority)
{
	struct hlist_head *head = &br->hash_ha[hash];
	struct net_hatbl_entry *ha = hatbl_find_rcu(head, mac_addr, sub_class,
			priority);

	if (ha && hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_TRACKED_ENTRY))
		return ha;

	return NULL ;
}

/* Locking must be done by the caller */
struct net_hatbl_entry * hyfi_hatbl_create_tracked_entry(
		struct hyfi_net_bridge *br, u_int32_t hash, const u_int8_t *sa,
		const u_int8_t *da, u_int32_t sub_class, u_int32_t priority)
{
	struct net_hatbl_entry *ha;
	struct net_bridge_fdb_entry *dst = os_br_fdb_get(netdev_priv(br->dev), da);
	bool ret;
	if (!dst ) {
		return NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
	ret = test_bit(BR_FDB_LOCAL, &dst->flags);
#else
	ret = dst->is_local;
#endif

	if (ret) {
		return NULL;
	}

	ha = hatbl_create(br, hash, dst->dst, sa, da, da, sub_class, priority, 0);

	if (!ha) {
		DEBUG_ERROR("hyfi: Failed to allocate memory for entry\n");
		return NULL;
	}

	hyfi_ha_set_flag(ha, HYFI_HACTIVE_TBL_TRACKED_ENTRY);
	hyfi_psw_init_entry(ha);

	return ha;
}

struct net_hatbl_entry * hyfi_hatbl_create_aggr_entry(
		struct hyfi_net_bridge *br, u_int32_t hash, const u_int8_t *sa,
		const u_int8_t *da, u_int32_t sub_class, u_int32_t priority,
		u_int16_t seq)
{
	struct net_hatbl_entry *ha;
	struct net_bridge_fdb_entry *dst = os_br_fdb_get(netdev_priv(br->dev), da);
	bool ret;
	if (!dst ) {
		return NULL;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
	ret = test_bit(BR_FDB_LOCAL, &dst->flags);
#else
	ret = dst->is_local;
#endif
	if ((((seq >> 14) & 3) == 0) || ret) {
		return NULL ;
	}

	spin_lock(&br->hash_ha_lock);
	ha = hatbl_create(br, hash, dst->dst, sa, da, da, sub_class, priority, 0);

	if (!ha) {
		DEBUG_ERROR("hyfi: Failed to allocate memory for entry\n");

		spin_unlock(&br->hash_ha_lock);
		return NULL ;
	}

	if (hyfi_aggr_init_entry(ha, seq) < 0) {
		DEBUG_ERROR("hyfi: Failed to allocate memory for entry\n");
		hatbl_delete(br, ha);

		spin_unlock(&br->hash_ha_lock);
		return NULL ;
	}

	spin_unlock(&br->hash_ha_lock);

	DEBUG_INFO("hyfi: Created an aggregated entry, ha = %p, hash = 0x%02x, seq = %d, num_ifs = %d\n",
			ha, hash, seq & 0x3fff, (seq >> 14) & 3);

	return ha;
}

int hyfi_hatbl_update_local(struct hyfi_net_bridge *br, u_int32_t hash,
		const u_int8_t *sa, const u_int8_t *da, struct net_hdtbl_entry *hd,
		struct net_bridge_port *dst, u_int32_t sub_class, u_int32_t priority,
		u_int32_t flag)
{
	struct hlist_head *head = &br->hash_ha[hash];
	struct net_hatbl_entry *ha;

	ha = hatbl_find_rcu(head, sa, sub_class, priority);
	if (likely(ha)) {
		if ((flag & IS_HYFI_AGGR_FLOW)&& ha->local) {
			/* Clear the local flag for aggregated flows */
			ha->local = 0;
		} else {
			/* fastpath: update of existing entry */
			ha->dst = dst;
			ha->sub_class = sub_class;
			ha->priority = priority;
			ha->local = 1;
		}

		return 0;
	}

	/* Let the TCP-ACK flow be created by real traffic when TCP data flow is aggregated */
	if (flag & IS_HYFI_AGGR_FLOW)
		return 0;

	spin_lock(&br->hash_ha_lock);
	ha = hatbl_create(br, hash, dst, da, sa, hd->id.addr, sub_class,
			priority, 0);
	if (likely(ha)) {
		ha->local = 1;
	}
	spin_unlock(&br->hash_ha_lock);

	if (ha == NULL)
		return -ENOENT;

	return 0;
}

int hyfi_hatbl_update(struct hyfi_net_bridge *br, struct net_device *brdev,
	struct __hatbl_entry *hae, int update_local)
{
	int status = 0;
	struct hlist_head *head = &br->hash_ha[hae->hash];
	struct net_hatbl_entry *ha;
	struct net_device *dev = NULL;
	struct ha_psw_stm_entry *pha_psw_stm_entry;
	struct net_bridge_port *br_port;

	spin_lock_bh(&br->hash_ha_lock);
	ha = __hatbl_find(head, hae->da, hae->sub_class, hae->priority);

	if (unlikely(!ha)) {
		status = -ENOENT;
		goto out;
	}

	/* fastpath: update of existing entry */
	if (hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY)) {
		status = hyfi_aggr_update_flow(br, hae, ha);
		goto out;
	}

	if (hae->aggr_entry) {
		status = hyfi_aggr_new_flow(br, hae, ha);
		goto out;
	}

	dev = dev_get_by_index(dev_net(brdev), hae->port_list[0].port);
	br_port = hyfi_br_port_get(dev);

	if (likely(dev && br_port && br_port->br->dev == brdev)) {
		int if_change = ha->dst->dev->ifindex != dev->ifindex;

		pha_psw_stm_entry = &ha->psw_stm_entry;

		if (if_change && br->path_switch_param.enable_switch_markers) {
			u_int32_t i = HYFI_PSW_MSE_CNT;

			DEBUG_TRACE("Sending switch end of flow x%d\n", i);
			while (i--) {
				rcu_read_lock();
				hyfi_psw_send_pkt(br, ha, HYFI_PSW_PKT_3, 0);
				rcu_read_unlock();
			}
		}

		ha->dst = br_port;
		ha->sub_class = hae->sub_class;
		ha->action = hae->action;

		if (update_local) {
			ha->local = hae->local;
		}

		if (if_change && br->path_switch_param.enable_switch_markers
				&& !hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY)) {
			DEBUG_TRACE("Sending switch start of flow\n");
			rcu_read_lock();
			hyfi_psw_send_pkt(br, ha, HYFI_PSW_PKT_4, 0);
			rcu_read_unlock();
			pha_psw_stm_entry->mrk_id++;
		}

	} else {
		hatbl_delete(br, ha);
	}

	if (likely(dev))
		dev_put(dev);

	out:
	spin_unlock_bh(&br->hash_ha_lock);

	return status;
}

int hyfi_hatbl_get_entry(struct hyfi_net_bridge *br, u_int8_t hash,
		const u_int8_t *addr, u_int8_t sub_class, u_int32_t priority, void *buf)
{
	struct hlist_head *head = &br->hash_ha[hash];
	struct net_hatbl_entry *ha;
	struct __hatbl_entry *hae = buf;
	int ret = 0;

	rcu_read_lock();
	ha = hatbl_find_rcu(head, addr, sub_class, priority);
	if (ha) {
		hatbl_fillbuf(ha, hae);
	} else {
		ret = -ENXIO;
	}
	rcu_read_unlock();
	return ret;
}

struct net_hatbl_entry* hyfi_hatbl_insert_from_fdb(struct hyfi_net_bridge *br,
		u_int32_t hash, struct net_bridge_port *dst, const u_int8_t *sa, const u_int8_t *da,
		const u_int8_t *id, u_int32_t sub_class, u_int32_t priority, bool keep_lock)
{
	struct net_hatbl_entry *ha = NULL;

	spin_lock(&br->hash_ha_lock);

	ha = hatbl_find_before_create(br, hash, dst, sa, da, id, sub_class, priority, 1);

	if (keep_lock && ha) {
		hyfi_netlink_event_send(ha->hyfi_br, HYFI_EVENT_ADD_HA_ENTRY,
			sizeof(struct __hatbl_entry), ha);
	} else {
		spin_unlock(&br->hash_ha_lock);
	}

	return ha;
}

void hyfi_hatbl_update_mcast_stats(struct net_bridge *br, struct sk_buff *skb,
		struct net_bridge_port *dst)
{
	u_int32_t flag, priority;
	u_int32_t hash, traffic_class;
	u_int16_t unused;
	struct net_hatbl_entry *ha = NULL;
	struct hyfi_net_bridge *hyfi_br = hyfi_bridge_get(br);

	if(!hyfi_br)
		return;

	if (unlikely(!hyfi_bridge_is_fwmode_aps(hyfi_br)))
		return;

	if (hyfi_hash_skbuf(skb, &hash, &flag, &priority, &unused)) {
		return;
	}

	traffic_class = (flag & IS_IPPROTO_UDP) ?
			HYFI_TRAFFIC_CLASS_UDP : HYFI_TRAFFIC_CLASS_OTHER;

	if ((ha = __hyfi_hatbl_get(hyfi_br, hash, eth_hdr(skb)->h_dest,
			traffic_class, priority))) {
		ha->num_packets++;
		ha->num_bytes += (skb)->len;
	} else {
		hyfi_hatbl_insert_from_fdb(hyfi_br, hash, dst, eth_hdr(skb)->h_source,
				eth_hdr(skb)->h_dest, br->dev->dev_addr,
				traffic_class, priority, false /* keep_lock */);
	}
}

static void hyfi_hatbl_timer_init(struct hyfi_net_bridge *br)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	timer_setup(&br->hatbl_timer, hyfi_hatbl_cleanup, 0);
#else
	setup_timer(&br->hatbl_timer, hyfi_hatbl_cleanup, (unsigned long) br);
#endif
	mod_timer(&br->hatbl_timer, jiffies + msecs_to_jiffies(HYFI_HACTIVE_TBL_AGING_TIME));
}

int __init hyfi_hatbl_init(void)
{
	hyfi_hatbl_cache = kmem_cache_create("hyfi_hatbl_cache",
			sizeof(struct net_hatbl_entry),
			0,
			SLAB_HWCACHE_ALIGN, NULL);

	if (!hyfi_hatbl_cache)
		return -ENOMEM;

	return 0;
}

int __init hyfi_hatbl_setup(struct hyfi_net_bridge *br)
{
	br->hatbl_aging_time =  msecs_to_jiffies(HYFI_HACTIVE_TBL_EXPIRE_TIME);
	hyfi_hatbl_timer_init(br);

	return 0;
}

void hyfi_hatbl_mark_accelerated(struct net_hatbl_entry *ha, u_int32_t ecm_serial)
{
	hyfi_ha_set_flag(ha, HYFI_HACTIVE_TBL_ACCL_ENTRY);
	hyfi_ha_set_flag(ha, HYFI_HACTIVE_TBL_SERIAL_VALID);
	ha->ecm_serial = ecm_serial;
}

void hyfi_hatbl_mark_decelerated(struct net_hatbl_entry *ha)
{
	hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_ACCL_ENTRY);
	hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_SERIAL_VALID);
	ha->ecm_serial = UINT_MAX;
}

void hyfi_hatbl_free(void)
{
	if (hyfi_hatbl_cache) {
		kmem_cache_destroy(hyfi_hatbl_cache);
		hyfi_hatbl_cache = NULL;
	}

}

void hyfi_hatbl_fini(struct hyfi_net_bridge *br)
{
	u_int32_t i;

	synchronize_rcu();
	del_timer_sync(&br->hatbl_timer);

	spin_lock_bh(&br->hash_ha_lock);

	for (i = 0; i < HA_HASH_SIZE; i++) {
		struct net_hatbl_entry *ha;
		struct hlist_node *h, *n;
		os_hlist_for_each_entry_safe (ha, h, n, &br->hash_ha[i], hlist) {
			br->ha_entry_cnt--;
			hlist_del_rcu(&ha->hlist);

			hyfi_psw_flush_buf_q(ha);
			hyfi_aggr_flush(ha);

			kmem_cache_free(hyfi_hatbl_cache, ha);
		}
	}

	spin_unlock_bh(&br->hash_ha_lock);
}

unsigned long hyfi_hatbl_calculate_elapsed_time(unsigned long time_now,
	unsigned long time_previous)
{
	if (time_now >= time_previous) {
		/* Non-rollover case */
		return time_now - time_previous;
	} else {
		/* Rollover case */
		return ULONG_MAX - (time_previous - time_now) + 1;
	}
}
