/*
 * Copyright 2020 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include <linux/crc32.h>
#include <linux/ieee80211.h>
#include "s1g_ieee80211.h"

#include "dot11ah.h"
#include "tim.h"
#include "../morse.h"
#include "../mesh.h"

/* Validity of the cssid entry */
#define MORSE_CSSID_ENTRY_VALIDITY_TIME	(60 * HZ)

/* Serialise operations that manipulate the CSSID list */
spinlock_t cssid_list_lock;
static LIST_HEAD(cssid_list);

/*
 * Static functions used only here
 */
static int __init morse_dot11ah_init(void)
{
	int ret = 0;

	spin_lock_init(&cssid_list_lock);
	pr_info("Morse Micro Dot11ah driver registration. Version %s\n", DOT11AH_VERSION);
	return ret;
}

static void __exit morse_dot11ah_exit(void)
{
	morse_dot11ah_clear_list();
}

/** morse_dot11ah_cssid_has_expired - Checks if the given cssid entry is expired or not.
 * @item: pointer to cssid item
 *
 * Return: True if the cssid entry has expired.
 */
static bool morse_dot11ah_cssid_has_expired(struct morse_dot11ah_cssid_item *item)
{
	u32 age_limit;

	if (item->mesh_beacon)
		age_limit =
			msecs_to_jiffies(MORSE_TU_TO_MS(MESH_CONFIG_NEIGHBOR_ENTRY_VALIDITY_IN_TU));
	else
		age_limit = MORSE_CSSID_ENTRY_VALIDITY_TIME;

	if (time_before(item->last_seen + age_limit, jiffies))
		return true;

	return false;
}

/*
 * Public functions used in  dot11ah module
 */
struct morse_dot11ah_cssid_item *morse_dot11ah_find_cssid_item_for_bssid(const u8 bssid[ETH_ALEN])
{
	struct morse_dot11ah_cssid_item *item, *tmp;
	u64 bssid_64 = mac2uint64(bssid);

	list_for_each_entry_safe(item, tmp, &cssid_list, list) {
		if (mac2uint64(item->bssid) == bssid_64) {
			item->last_seen = jiffies;
			return item;
		}

		if (morse_dot11ah_cssid_has_expired(item)) {
			list_del(&item->list);
			kfree(item->ies);
			kfree(item);
		}
	}

	return NULL;
}

/** Use of this function and any returned items must be protected with cssid_list_lock */
struct morse_dot11ah_cssid_item *morse_dot11ah_find_bssid(const u8 bssid[ETH_ALEN])
{
	struct morse_dot11ah_cssid_item *item, *tmp;

	if (!bssid)
		return NULL;

	list_for_each_entry_safe(item, tmp, &cssid_list, list) {
		if (memcmp(item->bssid, bssid, ETH_ALEN) == 0) {
			item->last_seen = jiffies;
			return item;
		}
	}
	return NULL;
}

void morse_dot11ah_store_cssid(struct dot11ah_ies_mask *ies_mask, u16 capab_info, u8 *s1g_ies,
			       int s1g_ies_len, const u8 *bssid,
			       struct dot11ah_update_rx_beacon_vals *vals)
{
	/**
	 * Kernel allocations in this function must be atomic as it occurs
	 * within a spin lock.
	 */
	struct morse_dot11ah_cssid_item *item, *stored;
	u32 cssid = 0;
	int length;
	const u8 *ssid;
	u8 network_id_eid;

	if (WARN_ON(!bssid))
		return;

	network_id_eid = morse_is_mesh_network(ies_mask) ? WLAN_EID_MESH_ID : WLAN_EID_SSID;
	cssid = morse_generate_cssid(ies_mask->ies[network_id_eid].ptr,
		ies_mask->ies[network_id_eid].len);
	ssid = ies_mask->ies[network_id_eid].ptr;
	length = ies_mask->ies[network_id_eid].len;

	spin_lock_bh(&cssid_list_lock);
	stored = morse_dot11ah_find_cssid_item_for_bssid(bssid);

	if (stored) {
		int s1g_ies_len_updated = s1g_ies_len;
		bool update_beacon = (vals && vals->cssid_ies);
		bool stored_ies_needs_update;
		u8 *s1g_ies_updated;

		if (stored->capab_info != capab_info && capab_info != 0)
			stored->capab_info = capab_info;

		if (update_beacon && s1g_ies) {
			/* Get the RSN/RSNX IE from stored IEs to update incoming beacon IEs */
			const u8 *rsn_ie = morse_dot11_find_ie(WLAN_EID_RSN, vals->cssid_ies,
						vals->cssid_ies_len);
			const u8 *rsnx_ie = morse_dot11_find_ie(WLAN_EID_RSNX, vals->cssid_ies,
						vals->cssid_ies_len);

			/* Update IEs length with RSN/RSNX IE if present */
			if (rsn_ie && !ies_mask->ies[WLAN_EID_RSN].ptr)
				s1g_ies_len_updated += *(rsn_ie + 1) + 2;
			if (rsnx_ie && !ies_mask->ies[WLAN_EID_RSNX].ptr)
				s1g_ies_len_updated += *(rsnx_ie + 1) + 2;
		}
		s1g_ies_updated = kmalloc(s1g_ies_len_updated, GFP_ATOMIC);

		/*
		 * Take a copy of the new beacon's IEs and insert the stored RSN IEs, in order to
		 * check for differences other than in the RSN IEs.
		 */
		if (s1g_ies && s1g_ies_updated) {
			memcpy(s1g_ies_updated, s1g_ies, s1g_ies_len);

			/* Update beacon IEs with stored RSN and RSNX IE (from probe response)
			 * before storing again.
			 */
			if (update_beacon)
				morse_dot11_insert_rsn_and_rsnx_ie(s1g_ies_updated + s1g_ies_len,
								   vals, ies_mask);
		}

		stored_ies_needs_update = (s1g_ies_updated &&
				(stored->ies_len != s1g_ies_len_updated ||
				 memcmp(stored->ies, s1g_ies_updated, stored->ies_len) != 0));

		if (stored_ies_needs_update) {
			kfree(stored->ies);
			stored->ies = s1g_ies_updated;
			stored->ies_len = s1g_ies_len_updated;
		} else {
			kfree(s1g_ies_updated);
		}

		memcpy(stored->bssid, bssid, ETH_ALEN);
		goto exit;
	}

	item = kmalloc(sizeof(*item), GFP_ATOMIC);
	if (!item)
		goto exit;

	item->cssid = cpu_to_le32(cssid);
	item->ssid_len = length;
	item->last_seen = jiffies;
	item->capab_info = capab_info;
	item->fc_bss_bw_subfield = MORSE_FC_BSS_BW_INVALID;
	item->mesh_beacon = (network_id_eid == WLAN_EID_MESH_ID);
	memcpy(item->ssid, ssid, length);

	item->ies = kmalloc(s1g_ies_len, GFP_ATOMIC);
	if (!item->ies) {
		kfree(item);
		goto exit;
	}

	item->ies_len = s1g_ies_len;

	memcpy(item->ies, s1g_ies, s1g_ies_len);
	memcpy(item->bssid, bssid, ETH_ALEN);

	list_add(&item->list, &cssid_list);

exit:
	spin_unlock_bh(&cssid_list_lock);
}

/*
 * Exported functions used elsewhere in morse driver
 */

bool morse_mac_find_channel_info_for_bssid(const u8 bssid[ETH_ALEN],
					   struct morse_channel_info *info)
{
	bool found = false;
	u8 *op = NULL;
	struct morse_dot11ah_cssid_item *item = NULL;

	spin_lock_bh(&cssid_list_lock);

	item = morse_dot11ah_find_bssid(bssid);

	if (item) {
		op = (u8 *)morse_dot11_find_ie(WLAN_EID_S1G_OPERATION, item->ies, item->ies_len);
		if (op) {
			u8 prim_chan_num = op[4];
			u8 op_chan_num = op[5];
			u8 chan_loc = IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_LOC(op[2]);

			info->op_bw_mhz = IEEE80211AH_S1G_OPERATION_GET_OP_CHAN_BW(op[2]);
			info->pri_bw_mhz = IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_BW(op[2]);
			info->pri_1mhz_chan_idx =
				morse_dot11ah_prim_1mhz_chan_loc_to_idx(info->op_bw_mhz,
					info->pri_bw_mhz, prim_chan_num, op_chan_num, chan_loc);
			info->op_chan_freq_hz =
				KHZ_TO_HZ(morse_dot11ah_channel_to_freq_khz(op_chan_num));
			found = true;
		}
	}

	spin_unlock_bh(&cssid_list_lock);

	return found;
}
EXPORT_SYMBOL(morse_mac_find_channel_info_for_bssid);

int morse_dot11_find_bssid_on_channel(u32 op_chan_freq_hz, u8 bssid[ETH_ALEN])
{
	bool found = false;
	struct morse_dot11ah_cssid_item *item, *tmp;

	spin_lock_bh(&cssid_list_lock);

	list_for_each_entry_safe(item, tmp, &cssid_list, list) {
		u8 *op = (u8 *)morse_dot11_find_ie(WLAN_EID_S1G_OPERATION, item->ies,
						   item->ies_len);

		if (op) {
			u8 ap_chan_num = op[5];
			u32 ap_freq =
				morse_dot11ah_channel_to_freq_khz(ap_chan_num);

			if (op_chan_freq_hz == KHZ_TO_HZ(ap_freq)) {
				memcpy(bssid, item->bssid, ETH_ALEN);
				found = true;
				break;
			}
		}
	}

	spin_unlock_bh(&cssid_list_lock);

	return found ? 0 : -ENOENT;
}
EXPORT_SYMBOL(morse_dot11_find_bssid_on_channel);

void morse_dot11ah_clear_list(void)
{
	struct morse_dot11ah_cssid_item *item, *tmp;

	spin_lock_bh(&cssid_list_lock);
	/* Free allocated list */
	list_for_each_entry_safe(item, tmp, &cssid_list, list) {
		list_del(&item->list);
		kfree(item->ies);
		kfree(item);
	}
	spin_unlock_bh(&cssid_list_lock);
}
EXPORT_SYMBOL(morse_dot11ah_clear_list);

bool morse_dot11ah_find_s1g_caps_for_bssid(u8 *bssid, struct ieee80211_s1g_cap *s1g_caps)
{
	struct morse_dot11ah_cssid_item *item;
	bool found = false;
	u8 *ie = NULL;

	spin_lock_bh(&cssid_list_lock);

	item = morse_dot11ah_find_bssid(bssid);
	if (item) {
		ie = (u8 *)morse_dot11_find_ie(WLAN_EID_S1G_CAPABILITIES, item->ies, item->ies_len);
		if (ie) {
			found = true;
			memcpy((u8 *)s1g_caps, (ie + 2), *(ie + 1));
		}
	}

	spin_unlock_bh(&cssid_list_lock);
	return found;
}
EXPORT_SYMBOL(morse_dot11ah_find_s1g_caps_for_bssid);

bool morse_dot11ah_find_bss_bw(u8 *bssid, u8 *fc_bss_bw_subfield)
{
	struct morse_dot11ah_cssid_item *bssid_item = NULL;
	bool found = false;

	spin_lock_bh(&cssid_list_lock);
	bssid_item = morse_dot11ah_find_bssid(bssid);

	if (bssid_item) {
		*fc_bss_bw_subfield = bssid_item->fc_bss_bw_subfield;
		found = true;
	}
	spin_unlock_bh(&cssid_list_lock);

	return found;
}
EXPORT_SYMBOL(morse_dot11ah_find_bss_bw);

bool morse_dot11ah_is_mesh_peer_known(const u8 *peer_mac_addr)
{
	bool ret = false;

	if (!peer_mac_addr)
		return false;

	spin_lock_bh(&cssid_list_lock);
	ret = morse_dot11ah_find_cssid_item_for_bssid(peer_mac_addr);
	spin_unlock_bh(&cssid_list_lock);

	return ret;
}
EXPORT_SYMBOL(morse_dot11ah_is_mesh_peer_known);

bool morse_dot11ah_add_mesh_peer(struct dot11ah_ies_mask *ies_mask, u16 capab_info,
				 const u8 *peer_mac_addr)
{
	if (!peer_mac_addr)
		return false;

	/* Create entry for this new mesh peer */
	morse_dot11ah_store_cssid(ies_mask, capab_info, NULL, 0, peer_mac_addr, NULL);

	return true;
}
EXPORT_SYMBOL(morse_dot11ah_add_mesh_peer);

bool morse_dot11ah_del_mesh_peer(const u8 *peer_mac_addr)
{
	struct morse_dot11ah_cssid_item *item, *tmp;
	u64 bssid_64;
	bool ret = false;

	if (!peer_mac_addr)
		return false;
	bssid_64 = mac2uint64(peer_mac_addr);

	spin_lock_bh(&cssid_list_lock);
	list_for_each_entry_safe(item, tmp, &cssid_list, list) {
		if (mac2uint64(item->bssid) == bssid_64) {
			list_del(&item->list);
			kfree(item->ies);
			kfree(item);
			ret = true;
			break;
		}
	}
	spin_unlock_bh(&cssid_list_lock);

	return ret;
}
EXPORT_SYMBOL(morse_dot11ah_del_mesh_peer);

/* Calculate primary channel location within operating bandwidth */
int morse_dot11_calc_prim_s1g_chan_loc(int prim_cent_freq, int op_chan_centre_freq, int op_bw_mhz)
{
	int pri_1mhz_location = 0;

	if (prim_cent_freq < op_chan_centre_freq)
		pri_1mhz_location =
			((op_bw_mhz - 1) - (op_chan_centre_freq - prim_cent_freq) / 500) / 2;
	else
		pri_1mhz_location =
			((op_bw_mhz - 1) + (prim_cent_freq - op_chan_centre_freq) / 500) / 2;

	return pri_1mhz_location;
}
EXPORT_SYMBOL(morse_dot11_calc_prim_s1g_chan_loc);

int morse_dot11ah_find_no_of_mesh_neighbors(u16 beacon_int)
{
	struct morse_dot11ah_cssid_item *item, *tmp;
	int mesh_neighbor_count = 0;

	spin_lock_bh(&cssid_list_lock);
	list_for_each_entry_safe(item, tmp, &cssid_list, list) {
		if (morse_dot11ah_cssid_has_expired(item)) {
			list_del(&item->list);
			kfree(item->ies);
			kfree(item);
		} else if (item->mesh_beacon && (item->beacon_int == beacon_int)) {
			mesh_neighbor_count++;
		}
	}
	spin_unlock_bh(&cssid_list_lock);

	return mesh_neighbor_count;
}
EXPORT_SYMBOL(morse_dot11ah_find_no_of_mesh_neighbors);

bool morse_dot11ah_is_page_slicing_enabled_on_bss(u8 *bssid)
{
	struct morse_dot11ah_cssid_item *item;
	bool enabled = false;
	u8 *ie = NULL;
	struct ieee80211_s1g_cap *s1g_caps;

	spin_lock_bh(&cssid_list_lock);

	item = morse_dot11ah_find_bssid(bssid);
	if (item) {
		ie = (u8 *)morse_dot11_find_ie(WLAN_EID_S1G_CAPABILITIES, item->ies, item->ies_len);
		if (ie) {
			s1g_caps = (struct ieee80211_s1g_cap *)(ie + 2);
			enabled = s1g_caps->capab_info[6] & S1G_CAP6_PAGE_SLICING;
		}
	}

	spin_unlock_bh(&cssid_list_lock);
	return enabled;
}
EXPORT_SYMBOL(morse_dot11ah_is_page_slicing_enabled_on_bss);

module_init(morse_dot11ah_init);
module_exit(morse_dot11ah_exit);

MODULE_AUTHOR("Morse Micro");
MODULE_DESCRIPTION("S1G support for Morse Micro drivers");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(DOT11AH_VERSION);
