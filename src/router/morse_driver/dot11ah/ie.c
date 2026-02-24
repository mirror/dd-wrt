/*
 * Copyright 2020 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include <linux/crc32.h>
#include <linux/ieee80211.h>

#include "dot11ah.h"
#include "tim.h"
#include "debug.h"
#include "../morse.h"
#include "../mesh.h"

/*
 * The following arrays refer to specific management/extension frame
 * types/subtypes and indicate the order of the information elements for
 * transmit path. The insert by order as specified by the IEEE-2020 standard in
 * section 9.3.3 for management frame, and in section 9.3.4 for extension
 * frame types done in morse_dot11_insert_ordered_ies_from_ies_mask()
 * function.
 */
static const u8 morse_ext_s1g_beacon_ies_order[] = {
		WLAN_EID_S1G_BCN_COMPAT,
		WLAN_EID_TIM,
		WLAN_EID_FMS_DESCRIPTOR,
		WLAN_EID_S1G_RPS,
		WLAN_EID_SST_OPERATION,
		WLAN_EID_SUBCHANNEL_SELECTIVE_TRANSMISSION,
		WLAN_EID_S1G_RELAY,
		WLAN_EID_PAGE_SLICE,
		WLAN_EID_S1G_SECTOR_OPERATION,
		WLAN_EID_S1G_CAC,
		WLAN_EID_TSF_TIMER_ACCURACY,
		WLAN_EID_S1G_RELAY_DISCOVERY,
		WLAN_EID_S1G_CAPABILITIES,
		WLAN_EID_S1G_OPERATION,
		WLAN_EID_S1G_SHORT_BCN_INTERVAL,
		WLAN_EID_MULTIPLE_BSSID,
		WLAN_EID_SSID,
		WLAN_EID_RSN,
		WLAN_EID_EXT_CHANSWITCH_ANN,
		WLAN_EID_MESH_ID,
		WLAN_EID_MESH_CONFIG,
		WLAN_EID_MESH_AWAKE_WINDOW,
		WLAN_EID_BEACON_TIMING,
		WLAN_EID_CHAN_SWITCH_PARAM,
		WLAN_EID_CHANNEL_SWITCH_WRAPPER,
		WLAN_EID_EXTENSION,
		WLAN_EID_VENDOR_SPECIFIC,
};

static const u8 morse_ext_s1g_short_beacon_ies_order[] = {
		WLAN_EID_TIM,
		WLAN_EID_FMS_DESCRIPTOR,
		WLAN_EID_S1G_RPS,
		WLAN_EID_SUBCHANNEL_SELECTIVE_TRANSMISSION,
		WLAN_EID_S1G_RELAY,
		WLAN_EID_SSID,
};

/* For probe request the next are not allowed for S1G:
 * WLAN_EID_DS_PARAMS
 * WLAN_EID_ERP_INFO
 * WLAN_EID_EXT_SUPP_RATES
 * WLAN_EID_HT_CAPABILITY
 * WLAN_EID_HT_OPERATION
 */
static const u8 morse_mgmt_probe_request_ies_order[] = {
		WLAN_EID_SSID,
		WLAN_EID_SSID_LIST,
		WLAN_EID_CHANNEL_USAGE,
		WLAN_EID_INTERWORKING,
		WLAN_EID_MESH_ID,
		WLAN_EID_MULTIPLE_MAC_ADDR,
		WLAN_EID_AP_CSN,
		WLAN_EID_CHANGE_SEQUENCE,
		WLAN_EID_S1G_RELAY_DISCOVERY,
		WLAN_EID_PV1_PROBE_RESPONSE_OPTION,
		WLAN_EID_EXT_CAPABILITY,
		WLAN_EID_S1G_CAPABILITIES,
		WLAN_EID_EL_OPERATION,
		WLAN_EID_S1G_MAX_AWAY_DURATION,
		WLAN_EID_EXTENSION,
		WLAN_EID_VENDOR_SPECIFIC,
};

/* For probe response the next are not allowed for S1G:
 * WLAN_EID_DS_PARAMS
 * WLAN_EID_ERP_INFO
 * WLAN_EID_EXT_SUPP_RATES
 * WLAN_EID_HT_CAPABILITY
 * WLAN_EID_HT_OPERATION
 */
static const u8 morse_mgmt_probe_response_ies_order[] = {
		WLAN_EID_SSID,
		WLAN_EID_COUNTRY,
		WLAN_EID_PWR_CONSTRAINT,
		WLAN_EID_CHANNEL_SWITCH,
		WLAN_EID_QUIET,
		WLAN_EID_IBSS_DFS,
		WLAN_EID_TPC_REPORT,
		WLAN_EID_RSN,
		WLAN_EID_MULTIPLE_BSSID,
		WLAN_EID_QBSS_LOAD,
		WLAN_EID_EDCA_PARAM_SET,
		WLAN_EID_MEASUREMENT_PILOT_TX_INFO,
		WLAN_EID_RRM_ENABLED_CAPABILITIES,
		WLAN_EID_AP_CHAN_REPORT,
		WLAN_EID_BSS_AVG_ACCESS_DELAY,
		WLAN_EID_ANTENNA_INFO,
		WLAN_EID_BSS_AVAILABLE_CAPACITY,
		WLAN_EID_BSS_AC_ACCESS_DELAY,
		WLAN_EID_MOBILITY_DOMAIN,
		WLAN_EID_DSE_REGISTERED_LOCATION,
		WLAN_EID_EXT_CHANSWITCH_ANN,
		WLAN_EID_SUPPORTED_REGULATORY_CLASSES,
		WLAN_EID_OVERLAP_BSS_SCAN_PARAM,
		WLAN_EID_EXT_CAPABILITY,
		WLAN_EID_QOS_TRAFFIC_CAPA,
		WLAN_EID_CHANNEL_USAGE,
		WLAN_EID_TIME_ADVERTISEMENT,
		WLAN_EID_TIME_ZONE,
		WLAN_EID_INTERWORKING,
		WLAN_EID_ADVERTISEMENT_PROTOCOL,
		WLAN_EID_ROAMING_CONSORTIUM,
		WLAN_EID_EMERGENCY_ALERT,
		WLAN_EID_MESH_ID,
		WLAN_EID_MESH_CONFIG,
		WLAN_EID_MESH_AWAKE_WINDOW,
		WLAN_EID_BEACON_TIMING,
		WLAN_EID_MCCAOP_ADV_OVERVIEW,
		WLAN_EID_MCCAOP_ADVERT,
		WLAN_EID_CHAN_SWITCH_PARAM,
		WLAN_EID_QLOAD_REPORT,
		WLAN_EID_MULTI_BAND,
		WLAN_EID_MULTIPLE_MAC_ADDR,
		WLAN_EID_ANTENNA_SECTOR_ID_PATTERN,
		WLAN_EID_EXTENDED_BSS_LOAD,
		WLAN_EID_CHANNEL_SWITCH_WRAPPER,
		WLAN_EID_QUIET_CHANNEL,
		WLAN_EID_OPMODE_NOTIF,
		WLAN_EID_REDUCED_NEIGHBOR_REPORT,
		WLAN_EID_CAG_NUMBER,
		WLAN_EID_FILS_INDICATION,
		WLAN_EID_AP_CSN,
		WLAN_EID_DILS,
		WLAN_EID_S1G_RPS,
		WLAN_EID_PAGE_SLICE,
		WLAN_EID_TSF_TIMER_ACCURACY,
		WLAN_EID_S1G_RELAY_DISCOVERY,
		WLAN_EID_S1G_CAPABILITIES,
		WLAN_EID_S1G_OPERATION,
		WLAN_EID_S1G_MAX_AWAY_DURATION,
		WLAN_EID_S1G_SHORT_BCN_INTERVAL,
		WLAN_EID_S1G_OPEN_LOOP_LINK_MARGIN_IDX,
		WLAN_EID_S1G_RELAY,
		WLAN_EID_RSNX,
		WLAN_EID_VENDOR_SPECIFIC,
		WLAN_EID_EXTENSION,
		WLAN_EID_S1G_CAC,
};

/* For association request the next are not allowed for S1G:
 * WLAN_EID_DS_PARAMS
 * WLAN_EID_ERP_INFO
 * WLAN_EID_EXT_SUPP_RATES
 * WLAN_EID_HT_CAPABILITY
 * WLAN_EID_HT_OPERATION
 */
static const u8 morse_mgmt_assoc_request_ies_order[] = {
		WLAN_EID_SSID,
		WLAN_EID_PWR_CAPABILITY,
		WLAN_EID_SUPPORTED_CHANNELS,
		WLAN_EID_RSN,
		WLAN_EID_QOS_CAPA,
		WLAN_EID_RRM_ENABLED_CAPABILITIES,
		WLAN_EID_MOBILITY_DOMAIN,
		WLAN_EID_SUPPORTED_REGULATORY_CLASSES,
		WLAN_EID_EXT_CAPABILITY,
		WLAN_EID_QOS_TRAFFIC_CAPA,
		WLAN_EID_TIM_BCAST_REQ,
		WLAN_EID_INTERWORKING,
		WLAN_EID_MULTI_BAND,
		WLAN_EID_MULTIPLE_MAC_ADDR,
		WLAN_EID_OPMODE_NOTIF,
		WLAN_EID_S1G_TWT,
		WLAN_EID_AID_REQUEST,
		WLAN_EID_S1G_CAPABILITIES,
		WLAN_EID_S1G_OPERATION,
		WLAN_EID_EL_OPERATION,
		WLAN_EID_S1G_RELAY,
		WLAN_EID_BSS_MAX_IDLE_PERIOD,
		WLAN_EID_HEADER_COMPRESSION,
		WLAN_EID_S1G_MAX_AWAY_DURATION,
		WLAN_EID_REACHABLE_ADDRESS,
		WLAN_EID_S1G_RELAY_ACTIVATION,
		WLAN_EID_FAST_BSS_TRANSITION,
		WLAN_EID_RSNX,
		WLAN_EID_EXTENSION,
		WLAN_EID_VENDOR_SPECIFIC,
};

/* For association response the next are not allowed for S1G:
 * WLAN_EID_DS_PARAMS
 * WLAN_EID_ERP_INFO
 * WLAN_EID_EXT_SUPP_RATES
 * WLAN_EID_HT_CAPABILITY
 * WLAN_EID_HT_OPERATION
 */
static const u8 morse_mgmt_assoc_response_ies_order[] = {
		WLAN_EID_EDCA_PARAM_SET,
		WLAN_EID_RCPI,
		WLAN_EID_RSNI,
		WLAN_EID_RRM_ENABLED_CAPABILITIES,
		WLAN_EID_RSN,
		WLAN_EID_MOBILITY_DOMAIN,
		WLAN_EID_FAST_BSS_TRANSITION,
		WLAN_EID_DSE_REGISTERED_LOCATION,
		WLAN_EID_TIMEOUT_INTERVAL,
		WLAN_EID_OVERLAP_BSS_SCAN_PARAM,
		WLAN_EID_EXT_CAPABILITY,
		WLAN_EID_BSS_MAX_IDLE_PERIOD,
		WLAN_EID_TIM_BCAST_RESP,
		WLAN_EID_QOS_MAP_SET,
		WLAN_EID_MULTI_BAND,
		WLAN_EID_MULTIPLE_MAC_ADDR,
		WLAN_EID_NEIGHBOR_REPORT,
		WLAN_EID_OPMODE_NOTIF,
		WLAN_EID_S1G_SECTOR_OPERATION,
		WLAN_EID_S1G_TWT,
		WLAN_EID_TSF_TIMER_ACCURACY,
		WLAN_EID_S1G_CAPABILITIES,
		WLAN_EID_S1G_OPERATION,
		WLAN_EID_AID_RESPONSE,
		WLAN_EID_SECTORIZED_GROUP_ID_LIST,
		WLAN_EID_S1G_RELAY,
		WLAN_EID_HEADER_COMPRESSION,
		WLAN_EID_SST_OPERATION,
		WLAN_EID_S1G_MAX_AWAY_DURATION,
		WLAN_EID_S1G_RELAY_ACTIVATION,
		WLAN_EID_RSNX,
		WLAN_EID_EXTENSION,
		WLAN_EID_VENDOR_SPECIFIC,
};

static const u8 morse_mgmt_mesh_peering_mgmt_ies_order[] = {
		WLAN_EID_VENDOR_SPECIFIC,
		WLAN_EID_S1G_CAPABILITIES,
		WLAN_EID_S1G_OPERATION,
		WLAN_EID_EXTENSION,
		WLAN_EID_RSN,
		WLAN_EID_MESH_ID,
		WLAN_EID_MESH_CONFIG,
		WLAN_EID_MESH_AWAKE_WINDOW,
		WLAN_EID_CHAN_SWITCH_PARAM,
		WLAN_EID_PEER_MGMT,
		WLAN_EID_MIC,
};

static void free_eid_ies_list(struct ie_element *list_head)
{
	struct ie_element *next, *cur;

	for (cur = list_head; cur; cur = next) {
		next = cur->next;
		if (cur->needs_free)
			kfree(cur->ptr);
		kfree(cur);
	}
}

void morse_dot11_clear_eid_from_ies_mask(struct dot11ah_ies_mask *ies_mask, u8 eid)
{
	free_eid_ies_list(ies_mask->ies[eid].next);
	if (ies_mask->ies[eid].needs_free)
		kfree(ies_mask->ies[eid].ptr);
	ies_mask->ies[eid].ptr = NULL;
	ies_mask->ies[eid].len = 0;
}
EXPORT_SYMBOL(morse_dot11_clear_eid_from_ies_mask);

struct dot11ah_ies_mask *morse_dot11ah_ies_mask_alloc(void)
{
	struct dot11ah_ies_mask *ies_mask = NULL;

	/* Atomic as ies mask can be allocated from the beacon tasklet */
	ies_mask = kzalloc(sizeof(*ies_mask), GFP_ATOMIC);

	return ies_mask;
}
EXPORT_SYMBOL(morse_dot11ah_ies_mask_alloc);

void morse_dot11ah_ies_mask_free(struct dot11ah_ies_mask *ies_mask)
{
	int pos;

	if (!ies_mask)
		return;

	for_each_set_bit(pos, ies_mask->more_than_one_ie, DOT11AH_MAX_EID)
		free_eid_ies_list(ies_mask->ies[pos].next);

	for (pos = 0; pos < ARRAY_SIZE(ies_mask->ies); pos++) {
		if (ies_mask->ies[pos].needs_free)
			kfree(ies_mask->ies[pos].ptr);
	}

	kfree(ies_mask);
}
EXPORT_SYMBOL(morse_dot11ah_ies_mask_free);

void morse_dot11ah_ies_mask_clear(struct dot11ah_ies_mask *ies_mask)
{
	int pos;

	if (!ies_mask)
		return;

	for_each_set_bit(pos, ies_mask->more_than_one_ie, DOT11AH_MAX_EID) {
		free_eid_ies_list(ies_mask->ies[pos].next);
	}

	for (pos = 0; pos < ARRAY_SIZE(ies_mask->ies); pos++) {
		if (ies_mask->ies[pos].needs_free)
			kfree(ies_mask->ies[pos].ptr);
	}

	/* clear the ies_mask */
	memset(ies_mask, 0, sizeof(*ies_mask));
}
EXPORT_SYMBOL(morse_dot11ah_ies_mask_clear);

struct ie_element *morse_dot11_ies_create_ie_element(struct dot11ah_ies_mask *ies_mask,
	u8 eid, int length, bool alloc, bool only_one)
{
	struct ie_element *cur = &ies_mask->ies[eid];
	struct ie_element *new;

	if (cur->ptr) {
		if (only_one) {
			morse_dot11_clear_eid_from_ies_mask(ies_mask, eid);
			WARN_ONCE(1, "EID %u already present, overriding\n", eid);
		} else {
			for (cur = &ies_mask->ies[eid]; cur->next; cur = cur->next)
				continue; /* walk to the end of the list */

			new = kzalloc(sizeof(*new), GFP_ATOMIC);
			if (!new)
				return NULL;

			cur->next = new;
			cur = new;
			set_bit(eid, ies_mask->more_than_one_ie);
		}
	}

	if (alloc) {
		cur->ptr = kzalloc(length, GFP_ATOMIC);
		if (!cur->ptr)
			return NULL;
		cur->needs_free = true;
	} else {
		cur->needs_free = false;
	}

	cur->len = length;

	return cur;
}
EXPORT_SYMBOL(morse_dot11_ies_create_ie_element);

int morse_dot11ah_parse_ies(u8 *start, size_t len, struct dot11ah_ies_mask *ies_mask)
{
	size_t left = len;
	u8 *pos = start;

	if (!start || !ies_mask) {
		dot11ah_warn("Null ref when parsing IEs\n");
		return -EINVAL;
	}

	while (left >= 2) {
		u8 id;
		u8 elen;
		struct ie_element *element;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (id == WLAN_EID_EXTENSION && left > 0) {
			u8 id_extension = *pos;

			/* If present, the FILS Session element is the last unencrypted element in
			 * the frame. The IDs and lengths of the following encrypted elements cannot
			 * be determined, so this element and the remaining data is treated as a
			 * single block of data.
			 */
			if (id_extension == WLAN_EID_EXT_FILS_SESSION) {
				dot11ah_debug("Have FILS session element\n");
				ies_mask->fils_data = (u8 *)(pos - 2);
				ies_mask->fils_data_len = left + 2;
				left = 0;
				break;
			}
		}

		if (elen > left) {
			dot11ah_warn("Element length larger than remaining bytes. have %u expecting %zu\n",
				elen, left);
			return -EINVAL;
		}

		element = morse_dot11_ies_create_ie_element(ies_mask, id, elen, false, false);
		if (!element)
			return -ENOMEM;
		element->ptr = pos;

		left -= elen;
		pos += elen;
	}

	if (left != 0) {
		dot11ah_warn("Leftover bytes after parsing %zu\n", left);
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL(morse_dot11ah_parse_ies);

const u8 *morse_dot11_find_ie(u8 eid, const u8 *ies, int length)
{
	return cfg80211_find_ie(eid, ies, length);
}

u8 *morse_dot11_insert_ie(u8 *dst, const u8 *src, u8 eid, u8 len)
{
	*dst++ = eid;
	*dst++ = len;

	if (src && len > 0) {
		/* Zero-length IE does not need a memcpy, only EID and LEN */
		memcpy(dst, src, len);
		dst += len;
	}
	return dst;
}
EXPORT_SYMBOL(morse_dot11_insert_ie);

u8 *morse_dot11_insert_ie_no_header(u8 *dst, const u8 *src, u8 len)
{
	memcpy(dst, src, len);
	dst += len;
	return dst;
}

u8 *morse_dot11_insert_ie_from_ies_mask(u8 *pos, const struct dot11ah_ies_mask *ies_mask, u8 eid)
{
	struct ie_element *cur;

	if (!ies_mask->ies[eid].ptr)
		return pos;

	pos = morse_dot11_insert_ie(pos, ies_mask->ies[eid].ptr, eid, ies_mask->ies[eid].len);

	/* insert any extras */
	for (cur = ies_mask->ies[eid].next; cur; cur = cur->next)
		pos = morse_dot11_insert_ie(pos, cur->ptr, eid, cur->len);

	return pos;
}

void morse_dot11ah_mask_ies(struct dot11ah_ies_mask *ies_mask, bool mask_ext_cap, bool is_beacon)
{
	/* Masks all the information elements are not needed when sending a packet */
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_DS_PARAMS);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_ERP_INFO);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_EXT_SUPP_RATES);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_HT_CAPABILITY);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_HT_OPERATION);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_SUPP_RATES);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_VHT_CAPABILITY);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_VHT_OPERATION);
#if KERNEL_VERSION(5, 15, 0) <= MAC80211_VERSION_CODE
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_TX_POWER_ENVELOPE);
#else
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_VHT_TX_POWER_ENVELOPE);
#endif
	/* S1G parameters are masked as they will be added explicitly */
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_S1G_SHORT_BCN_INTERVAL);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_S1G_CAPABILITIES);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_S1G_OPERATION);
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_S1G_BCN_COMPAT);

	if (mask_ext_cap)
		morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_EXT_CAPABILITY);

	if (is_beacon) {
		 /* Remove extra elements to minimise DTIM current draw */
		morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_RSN);
		morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_RSNX);
		morse_dot11_clear_eid_from_ies_mask(ies_mask,
						    WLAN_EID_SUPPORTED_REGULATORY_CLASSES);
	}
}

int morse_dot11_insert_ordered_ies_from_ies_mask(struct sk_buff *skb, u8 *pos,
					struct dot11ah_ies_mask *ies_mask,
					__le16 frame_control)
{
	int i, eid;
	struct ie_element *cur;
	const u8 *ies_order_table = NULL;
	u8 ies_order_table_len = 0;
	int ies_len = 0;
	int ampe_len = 0;

	if (!ies_mask)
		return 0;

	if (ieee80211_is_s1g_short_beacon(frame_control) ||
	    (le16_to_cpu(frame_control) & IEEE80211_FC_COMPRESS_SSID)) {
		ies_order_table = morse_ext_s1g_short_beacon_ies_order;
		ies_order_table_len = ARRAY_SIZE(morse_ext_s1g_short_beacon_ies_order);
	} else if (ieee80211_is_s1g_beacon(frame_control)) {
		ies_order_table = morse_ext_s1g_beacon_ies_order;
		ies_order_table_len = ARRAY_SIZE(morse_ext_s1g_beacon_ies_order);
	} else if (ieee80211_is_probe_req(frame_control)) {
		ies_order_table = morse_mgmt_probe_request_ies_order;
		ies_order_table_len = ARRAY_SIZE(morse_mgmt_probe_request_ies_order);
	} else if (ieee80211_is_probe_resp(frame_control)) {
		ies_order_table = morse_mgmt_probe_response_ies_order;
		ies_order_table_len = ARRAY_SIZE(morse_mgmt_probe_response_ies_order);
	} else if (ieee80211_is_assoc_req(frame_control) ||
		ieee80211_is_reassoc_req(frame_control)) {
		ies_order_table = morse_mgmt_assoc_request_ies_order;
		ies_order_table_len = ARRAY_SIZE(morse_mgmt_assoc_request_ies_order);
	} else if (ieee80211_is_assoc_resp(frame_control) ||
		   ieee80211_is_reassoc_resp(frame_control)) {
		ies_order_table = morse_mgmt_assoc_response_ies_order;
		ies_order_table_len = ARRAY_SIZE(morse_mgmt_assoc_response_ies_order);
	} else if (ieee80211_is_action(frame_control)) {
		struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;

		if (morse_dot11_is_mpm_frame(mgmt)) {
			ies_order_table = morse_mgmt_mesh_peering_mgmt_ies_order;
			ies_order_table_len = ARRAY_SIZE(morse_mgmt_mesh_peering_mgmt_ies_order);
			ampe_len = morse_dot11_get_mpm_ampe_len(skb);
			ies_len += ampe_len;
		}
	}

	if (!ies_order_table)
		return 0;

	for (i = 0; i < ies_order_table_len; i++) {
		eid = ies_order_table[i];

		if (!ies_mask->ies[eid].ptr)
			continue;

		/* Allow zero length IEs for SSID and Mesh ID only as a wild-card one
		 * (only EID and LEN=0).
		 */
		if (ies_mask->ies[eid].len == 0 &&
		    !(eid == WLAN_EID_SSID || eid == WLAN_EID_MESH_ID))
			continue;

		if (pos)
			pos = morse_dot11_insert_ie(pos, ies_mask->ies[eid].ptr, eid,
						    ies_mask->ies[eid].len);

		ies_len += ies_mask->ies[eid].len + 2;

		/* insert any extras */
		for (cur = ies_mask->ies[eid].next; cur; cur = cur->next) {
			if (pos)
				pos = morse_dot11_insert_ie(pos, cur->ptr, eid, cur->len);
			ies_len += cur->len + 2;
		}
	}

	/* For mesh the AMPE block needs to be copied after the ordered IEs */
	if (pos && ampe_len) {
		memcpy(pos, (skb->data + (skb->len - ampe_len)), ampe_len);
		pos += ampe_len;
	}

	return ies_len;
}
EXPORT_SYMBOL(morse_dot11_insert_ordered_ies_from_ies_mask);

void morse_dot11ah_insert_element(struct dot11ah_ies_mask *ies_mask, u8 eid,
				const u8 *data, int length)
{
	struct ie_element *element;

	element = morse_dot11_ies_create_ie_element(ies_mask, eid, length, true, true);

	if (!element)
		return;

	memcpy(element->ptr, data, length);
}
EXPORT_SYMBOL(morse_dot11ah_insert_element);

struct dot11ah_ies_mask *morse_dot11ah_ies_to_ies_mask(const u8 *ies, u16 ies_len)
{
	struct dot11ah_ies_mask *ies_mask = NULL;
	int ret;

	ies_mask = morse_dot11ah_ies_mask_alloc();
	if (!ies_mask)
		goto err;

	ret = morse_dot11ah_parse_ies((u8 *)ies, ies_len, ies_mask);
	if (ret)
		goto err;

	return ies_mask;
err:
	morse_dot11ah_ies_mask_free(ies_mask);
	return NULL;
}
EXPORT_SYMBOL(morse_dot11ah_ies_to_ies_mask);
