/*
 *  QCA Hy-Fi ECM
 *
 * Copyright (c) 2014-2016, The Linux Foundation. All rights reserved.
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
#include <linux/export.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include "hyfi_hash.h"
#include "hyfi_hatbl.h"
#include "hyfi_ecm.h"

/**
 * @brief A H-Active entry is newly accelerated.
 *
 * Set flags and clear stats
 *
 * @param [in] num_bytes  number of bytes sent in the update
 * @param [in] num_packets  number of packets sent in the update
 * @param [inout] ha  H-Active table entry to update
 * @param [inout] flow  flow entry to update
 * @param [out] should_keep_on_fdb_update  set to true if this
 *                                         entry should be kept
 *                                         on FDB update, false
 *                                         if it should be
 *                                         deleted (static
 *                                         entries should be
 *                                         deleted)
 */
static void hyfi_ecm_mark_as_newly_accelerated(u_int64_t num_bytes, u_int64_t num_packets,
	struct net_hatbl_entry *ha, const struct hyfi_ecm_flow_data_t *flow,
	bool *should_keep_on_fdb_update)
{
	DEBUG_TRACE("hyfi: New accelerated connection with serial number (prev): %d (%d), hash: 0x%02x, should_keep %d\n",
		flow->ecm_serial, ha->ecm_serial, ha->hash,
		!hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_STATIC_ENTRY));

	hyfi_hatbl_mark_accelerated(ha, flow->ecm_serial);
	ha->prev_num_bytes = num_bytes;
	ha->prev_num_packets = num_packets;

	/* We want to keep non-static entries.
	 * A static entry is learned from FDB, and in HyFi indicates
	 * a legacy, directly connected device.  If we receive an FDB
	 * update for a static entry, it indicates the place where this
	 * device is connected has changed, and hence connections should
	 * be deleted, and H-Active updated.  A non-static entry would include
	 * connections like those between RE and CAP.  Since HyFi has dual
	 * backhauls, the FDB can change very frequently, and these updates
	 * should be ignored, since no action needs to be taken.
	 */
	*should_keep_on_fdb_update = !hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_STATIC_ENTRY);
}

/*
 * Notify about a new connection
 * returns:
 * -1: error
 * 0: okay
 * 1: not interested
 * 2: hy-fi not attached
 */

static int hyfi_ecm_new_connection(struct hyfi_net_bridge *hyfi_br,
	const struct hyfi_ecm_flow_data_t *flow, u_int32_t hash,
	const u_int8_t *da, const u_int8_t *sa, struct net_hatbl_entry **ha_ret,
	bool *unlock_bh)
{
	u_int32_t traffic_class;
	bool ret;
	struct net_hatbl_entry *ha = NULL;
	*unlock_bh = true;
	*ha_ret = NULL;
	traffic_class = (flow->flag & ECM_HYFI_IS_IPPROTO_UDP) ?
			HYFI_TRAFFIC_CLASS_UDP : HYFI_TRAFFIC_CLASS_OTHER;

	if (unlikely(!hyfi_bridge_is_fwmode_aps(hyfi_br)))
		return 0;

	spin_lock_bh(&hyfi_br->hash_ha_lock);

	/* Find H-Active entry */
	ha = hatbl_find(hyfi_br, hash, da, traffic_class, flow->priority);

	if (ha) {
		*ha_ret = ha;
		return 0;

	} else {
		/* H-Active was not found, look for H-Default entry, and create an
		 * H-Active entry if exists.
		 */
		struct net_hdtbl_entry *hd;

		/* Unlock the ha-lock, and lock the hd-lock */
		spin_unlock_bh(&hyfi_br->hash_ha_lock);

		spin_lock_bh(&hyfi_br->hash_hd_lock);
		hd = hyfi_hdtbl_find(hyfi_br, da);

		if (hd) {
			/* Create a new entry based on H-Default table. The function
			 * will keep the ha-lock if created successfully. */
			ha = hyfi_hatbl_insert_ecm_classifier(hyfi_br, hash,
					traffic_class, hd, flow->priority,
					sa, flow->ecm_serial);

			/* Release the hd-lock, we are done with the hd entry */
			spin_unlock_bh(&hyfi_br->hash_hd_lock);

			if(ha) {
				/* H-Active created. */
				*ha_ret = ha;
				return 0;
			} else {
				DEBUG_TRACE("hyfi: Failed to create new accelerated connection to %02X:%02X:%02X:%02X:%02X:%02X from HD with serial number: %d, hash: 0x%02x",
					da[0], da[1], da[2], da[3],
					da[4], da[5], flow->ecm_serial, hash);
				return -1;
			}
		} else {
			struct net_bridge_fdb_entry *dst;
			struct net_device *br_dev;
			DEBUG_TRACE("hyfi: no hd to %02X:%02X:%02X:%02X:%02X:%02X\n", da[0],
					da[1], da[2], da[3],
					da[4], da[5] );

			/* No such H-Default entry, unlock hd-lock */
			spin_unlock_bh(&hyfi_br->hash_hd_lock);

			rcu_read_lock();
			br_dev = hyfi_bridge_dev_get_rcu(hyfi_br);
			if (!br_dev) {

				rcu_read_unlock();

				/* Hy-Fi bridge must have been detached while this function was
				 running */
				DEBUG_TRACE("hyfi: Bridge detached while processing, failed to create "
					"new accelerated connection to %02X:%02X:%02X:%02X:%02X:%02X "
					"from FDB with serial number: %d, hash: 0x%02x",
					da[0], da[1], da[2], da[3],
					da[4], da[5], flow->ecm_serial, hash);

				return 2;
			}

			dst = os_br_fdb_get(netdev_priv(br_dev), da);
			/* Try and insert from FDB */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0))
			if(dst)
			ret = test_bit(BR_FDB_LOCAL, &dst->flags);
#else
			if(dst)
			ret = dst->is_local;
#endif

			if (dst && !ret) {
				ha = hyfi_hatbl_insert_from_fdb(hyfi_br, hash, dst->dst, sa,
					da, hyfi_br->dev->dev_addr,
					traffic_class, flow->priority, true /* keep_lock */);
				rcu_read_unlock();
				if (ha) {
					*ha_ret = ha;
					*unlock_bh = false;
					return 0;
				} else {

					DEBUG_TRACE("hyfi: Failed to create new accelerated connection to %02X:%02X:%02X:%02X:%02X:%02X from FDB with serial number: %d, hash: 0x%02x",
						da[0], da[1], da[2], da[3],
						da[4], da[5], flow->ecm_serial, hash);

					return -1;
				}
			}
			rcu_read_unlock();
			/* Not found in FDB either - can't handle */
			return 1;
		}
	}

	return 0;
}

/**
 * @brief Calculate the weighted average rate
 *
 * @param [in] new_rate  most recently calculated rate
 * @param [in] new_elapsed_time  time over which the most
 *                               recently calculated rate was
 *                               calculated over
 * @param [in] old_rate  previous rate
 * @param [in] old_elapsed_time  time over which the previous
 *                               rate was calculated over
 *
 * @return weighted average rate
 */
static u_int32_t hyfi_ecm_calculate_weighted_average(u_int32_t new_rate,
	u_int32_t new_elapsed_time, u_int32_t old_rate,
	u_int32_t old_elapsed_time)
{
	u_int32_t new_weighted_rate = new_rate;
	u_int32_t old_weighted_rate = old_rate;

	u_int32_t total_elapsed_time = new_elapsed_time + old_elapsed_time;

	if (!total_elapsed_time) {
		return 0;
	}

	new_weighted_rate /= total_elapsed_time;
	old_weighted_rate /= total_elapsed_time;

	return (new_weighted_rate * new_elapsed_time +
		old_weighted_rate * old_elapsed_time);
}

int hyfi_ecm_update_stats(const struct hyfi_ecm_flow_data_t *flow, u_int32_t hash,
	u_int8_t *da, u_int8_t *sa, u_int64_t num_bytes, u_int64_t num_packets,
	u_int32_t time_now, bool *should_keep_on_fdb_update, u_int32_t *new_elapsed_time,
	const char *br_name)
{
	struct net_hatbl_entry *ha = NULL;
	struct hyfi_net_bridge *hyfi_br;
	bool unlock_bh;
	int ret = 0;

	if(!num_bytes || !num_packets)
		return 0;

	if(!flow)
		return -1;

	hyfi_br = hyfi_ecm_bridge_attached(br_name);
	if (!hyfi_br) {
		/* Hy-Fi bridge not attached */
		return 2;
	}

	spin_lock_bh(&hyfi_br->hash_ha_lock);

	/* Find H-Active entry */
	if (flow->ecm_serial != ~0 &&
		(ha = hatbl_find_ecm(hyfi_br, hash, flow->ecm_serial, da))) {
		if (!hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_ACCL_ENTRY)) {
			/* This flow is now accelerated */
			hyfi_ecm_mark_as_newly_accelerated(num_bytes,
				num_packets, ha, flow, should_keep_on_fdb_update);
			*new_elapsed_time = 0;
		} else {
                        /* Calculate the rate since the last update */
			u_int32_t elapsed_time =
				hyfi_hatbl_calculate_elapsed_time(time_now, flow->last_update);
			u_int32_t elapsed_time_ms = jiffies_to_msecs(elapsed_time);
			/* Note that overflow is not handled here.  It is assumed that
			 * NSS updates will happen more frequently than every 2^32 bytes
			 */
			u_int32_t num_sent_bytes = num_bytes - ha->prev_num_bytes;
			/* Multiply by 8 to convert bytes to bits */
			u_int32_t rate_now = 0;
			if (elapsed_time_ms) {
				rate_now = (num_sent_bytes * 8) / elapsed_time_ms;
			}
			rate_now *= 1000;

			/* If there has been a previous update - calculate weighted
			 * average rate
			 */
			if (ha->num_bytes && ha->rate && flow->last_elapsed_time) {
				ha->rate = hyfi_ecm_calculate_weighted_average(
					rate_now, elapsed_time,
					ha->rate, flow->last_elapsed_time);
				*new_elapsed_time = elapsed_time + flow->last_elapsed_time;
			} else {
				ha->rate = rate_now;
				*new_elapsed_time = elapsed_time;
			}

			ha->num_bytes += num_sent_bytes;
			ha->num_packets += (num_packets - ha->prev_num_packets);
			ha->prev_num_bytes = num_bytes;
			ha->prev_num_packets = num_packets;

			DEBUG_TRACE("hyfi: Hash 0x%02x, serial=%d, num_bytes=%d, num_packets=%d, rate=%u, elapsed time %u ms, new_elapsed_time %u ms\n",
				hash, flow->ecm_serial, ha->num_bytes, ha->num_packets,
				ha->rate, elapsed_time_ms,
				jiffies_to_msecs(*new_elapsed_time));

		}

		spin_unlock_bh(&hyfi_br->hash_ha_lock);

		return 0;
	}

	spin_unlock_bh(&hyfi_br->hash_ha_lock);

	ret = hyfi_ecm_new_connection(hyfi_br, flow, hash, da, sa,
		&ha, &unlock_bh);

	 *new_elapsed_time = 0;

	if (ha) {
		/* Found. Update ecm serial number and return */
		hyfi_ecm_mark_as_newly_accelerated(
			num_bytes, num_packets, ha, flow, should_keep_on_fdb_update);
		if (unlock_bh) {
			spin_unlock_bh(&hyfi_br->hash_ha_lock);
		} else {
			spin_unlock(&hyfi_br->hash_ha_lock);
		}

		return 0;

	} else {
		return ret;
	}
}

EXPORT_SYMBOL(hyfi_ecm_update_stats);

void hyfi_ecm_decelerate(u_int32_t hash, u_int32_t ecm_serial, u_int8_t *da,
	const char *br_name)
{
	struct net_hatbl_entry *ha = NULL;
	struct hyfi_net_bridge *hyfi_br = NULL;

	/* TODO
	 * Initialize the hyfi_br pointer with valid instance of hyfi_br.
	 * For now hyfi_ecm_bridge_attached will return 0, as ecm is not
	 * supported with hyfi multipe bridges.
	 */
	hyfi_br = hyfi_ecm_bridge_attached(br_name);
	if (!hyfi_br) {
		/* Hy-Fi bridge not attached */
		return;
	}

	if (hyfi_br == NULL)
		return;

	spin_lock_bh(&hyfi_br->hash_ha_lock);

	ha = hatbl_find_ecm(hyfi_br, hash, ecm_serial, da);
	/* Find H-Active entry */
	if (ha) {
		/* Clear the accelerated flag and serial number */
		hyfi_hatbl_mark_decelerated(ha);
	}

	spin_unlock_bh(&hyfi_br->hash_ha_lock);
}

EXPORT_SYMBOL(hyfi_ecm_decelerate);

bool hyfi_ecm_should_keep(const struct hyfi_ecm_flow_data_t *flow, uint8_t *mac,
	const char *br_name)
{
	if (!memcmp(&flow->da[0], mac, ETH_ALEN)) {
		/* Need to check the forward */
		return flow->flag & ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_FWD;
	} else {
		/* Need to check the reverse */
		return flow->flag & ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_REV;
	}
}

EXPORT_SYMBOL(hyfi_ecm_should_keep);

bool hyfi_ecm_port_matches(const struct hyfi_ecm_flow_data_t *flow,
	int32_t to_system_index, int32_t from_system_index, const char *br_name)
{
	struct net_hatbl_entry *ha = NULL;
	struct hyfi_net_bridge *hyfi_br;
	u_int32_t traffic_class;
	bool ret = false;
	bool unlock_bh;

	hyfi_br = hyfi_ecm_bridge_attached(br_name);
	if (!hyfi_br) {
		/* Hy-Fi bridge not attached */
		return true;
	}

	traffic_class = (flow->flag & ECM_HYFI_IS_IPPROTO_UDP) ?
		HYFI_TRAFFIC_CLASS_UDP : HYFI_TRAFFIC_CLASS_OTHER;

	DEBUG_TRACE("hyfi_ecm_port_matches: Value of traffic class = %d\n", traffic_class);

	/* Find H-Active entry - forward first */
	/* Note not being able to find the entry or having an invalid
	 *index is also OK - it just means HyFi is not interested in
	 *this direction / flow, and should ignore it
	 */
	if (to_system_index != -1) {
		hyfi_ecm_new_connection(hyfi_br, flow, flow->hash, flow->da, flow->sa,
			&ha, &unlock_bh);
		if (!ha) {
			ret = true;
		} else {
			if (ha->dst->dev->ifindex != to_system_index) {

				DEBUG_TRACE("%x: to connection iface %d != H-Active intf %d (%s)\n",
					flow->hash, to_system_index, ha->dst->dev->ifindex,
					ha->dst->dev->name);

			} else {
				ret = true;
			}

			if (unlock_bh) {
				spin_unlock_bh(&hyfi_br->hash_ha_lock);
			} else {
				spin_unlock(&hyfi_br->hash_ha_lock);
			}
		}
	} else {
		ret = true;
	}

	if (ret && from_system_index != -1) {
		hyfi_ecm_new_connection(hyfi_br, flow, flow->reverse_hash,
			flow->sa, flow->da, &ha, &unlock_bh);
		if (ha) {
			if (ha->dst->dev->ifindex != from_system_index) {

				DEBUG_TRACE("%x: from connection iface %d != H-Active intf %d (%s)\n",
					flow->reverse_hash, from_system_index, ha->dst->dev->ifindex,
					ha->dst->dev->name);

				ret = false;
			}

			if (unlock_bh) {
				spin_unlock_bh(&hyfi_br->hash_ha_lock);
			} else {
				spin_unlock(&hyfi_br->hash_ha_lock);
			}
		}
	}

	return ret;
}

EXPORT_SYMBOL(hyfi_ecm_port_matches);

bool hyfi_ecm_is_port_on_hyfi_bridge(int32_t system_index)
{
	struct net_device *dev;
	bool ret = false;

	rcu_read_lock();
	dev = dev_get_by_index_rcu(&init_net, system_index);
	if (dev) {
		if (hyfi_bridge_get_port_by_dev(dev)) {
			/* Is a HyFi bridge port */
			ret = true;
		}
	}

	rcu_read_unlock();

	return ret;
}

EXPORT_SYMBOL(hyfi_ecm_is_port_on_hyfi_bridge);

struct hyfi_net_bridge * hyfi_ecm_bridge_attached(const char *br_name)
{
	struct net_device *br_dev = NULL;
	struct hyfi_net_bridge *hyfi_br = NULL;

	br_dev = dev_get_by_name(&init_net, br_name);
	if (unlikely(!br_dev))
		return NULL;

	hyfi_br = hyfi_bridge_get_by_dev(br_dev);

	dev_put(br_dev);
	return hyfi_br;
}

EXPORT_SYMBOL(hyfi_ecm_bridge_attached);


