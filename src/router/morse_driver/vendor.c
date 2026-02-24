/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <net/mac80211.h>
#include <net/netlink.h>

#include "command.h"
#include "mac.h"
#include "bus.h"
#include "debug.h"
#include "wiphy.h"
#include "vendor.h"
#include "mesh.h"
#ifdef CONFIG_ANDROID
#include "apf.h"
#endif

/** Extra overhead to account for any additional netlink framing */
#define VENDOR_EVENT_OVERHEAD			(30)

/* Allow/disallow insertion of vendor IEs */
static bool enable_mm_vendor_ie __read_mostly = true;
module_param(enable_mm_vendor_ie, bool, 0644);
MODULE_PARM_DESC(enable_mm_vendor_ie, "Allow insertion of Morse vendor IEs");

static int
morse_vendor_cmd_to_morse(struct wiphy *wiphy, struct wireless_dev *wdev,
			  const void *data, int data_len)
{
	struct sk_buff *skb;
	struct morse *mors = morse_wiphy_to_morse(wiphy);
	int skb_len;
	int dataout_len;
	struct morse_cmd_req_vendor *datain;
	struct morse_cmd_resp_vendor *dataout;
	struct morse_vif *mors_vif = NULL;

	if (!data || data_len < sizeof(struct morse_cmd_req))
		return -EINVAL;

	datain = kzalloc(sizeof(*datain), GFP_KERNEL);
	if (!datain)
		return -ENOMEM;
	memcpy(datain, data, data_len);

	if (wdev) {
		mors_vif = morse_wdev_to_morse_vif(wdev);
		/* Add the VIF ID to the command header */
		if (mors_vif)
			datain->hdr.vif_id = cpu_to_le16(mors_vif->id);
	}

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(*dataout));
	if (!skb) {
		kfree(datain);
		return -ENOMEM;
	}

	skb_len = skb->len;
	dataout = (struct morse_cmd_resp_vendor *)skb_put(skb, sizeof(*dataout));

	mutex_lock(&mors->lock);
	if (wdev)
		morse_cmd_vendor(mors, mors_vif, datain, data_len, dataout, &dataout_len);
	else
		morse_hw_cmd_vendor(mors, datain, data_len, dataout, &dataout_len);
	mutex_unlock(&mors->lock);
	skb_len += dataout_len;
	kfree(datain);
	skb_trim(skb, skb_len);
	return cfg80211_vendor_cmd_reply(skb);
}

static const struct wiphy_vendor_command morse_vendor_commands[] = {
	{
	 .info = {
		  .vendor_id = MORSE_OUI,
		  .subcmd = MORSE_VENDOR_CMD_TO_MORSE,
		  },
	 .flags = WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
	 .policy = VENDOR_CMD_RAW_DATA,
#endif
	 .doit = morse_vendor_cmd_to_morse,
	  },
	{
	 .info = {
		  .vendor_id = MORSE_OUI,
		  .subcmd = MORSE_VENDOR_HW_CMD_TO_MORSE,
		  },
	 .flags = 0,
#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
	 .policy = VENDOR_CMD_RAW_DATA,
#endif
	 .doit = morse_vendor_cmd_to_morse,
	},
#ifdef CONFIG_ANDROID
	{
		.info = {
			 .vendor_id = MORSE_OUI,
			 .subcmd = MORSE_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES,
			 },
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
		.policy = VENDOR_CMD_RAW_DATA,
#endif
		.doit = morse_vendor_cmd_get_supported_feature_set,
	},
	{
		.info = {
			 .vendor_id = MORSE_OUI,
			 .subcmd = MORSE_VENDOR_SUBCMD_GET_PACKET_FILTER_CAPABILITIES,
			 },
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
		.policy = morse_apf_nla_policy,
		.maxattr = VENDOR_ATTR_PACKET_FILTER_MAX,
#endif
		.doit = morse_vendor_cmd_apf_get_capabilities,
	},
	{
		.info = {
			 .vendor_id = MORSE_OUI,
			 .subcmd = MORSE_VENDOR_SUBCMD_SET_PACKET_FILTER,
			 },
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
		.policy = morse_apf_nla_policy,
		.maxattr = VENDOR_ATTR_PACKET_FILTER_MAX,
#endif
		.doit = morse_vendor_cmd_apf_set_packet_filter,
	},
	{
		.info = {
			 .vendor_id = MORSE_OUI,
			 .subcmd = MORSE_VENDOR_SUBCMD_READ_PACKET_FILTER_DATA,
			 },
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
		.policy = morse_apf_nla_policy,
		.maxattr = VENDOR_ATTR_PACKET_FILTER_MAX,
#endif
		.doit = morse_vendor_cmd_apf_read_packet_filter_data,
	},
#endif
};

static const struct nl80211_vendor_cmd_info morse_vendor_events[] = {
	[MORSE_VENDOR_EVENT_BCN_VENDOR_IE_FOUND] = {
						    .vendor_id = MORSE_OUI,
						    .subcmd = MORSE_VENDOR_EVENT_BCN_VENDOR_IE_FOUND
						    },
	[MORSE_VENDOR_EVENT_OCS_DONE] = {
					 .vendor_id = MORSE_OUI,
					 .subcmd = MORSE_VENDOR_EVENT_OCS_DONE },
	[MORSE_VENDOR_EVENT_MGMT_VENDOR_IE_FOUND] = {
						     .vendor_id = MORSE_OUI,
						     .subcmd =
						     MORSE_VENDOR_EVENT_MGMT_VENDOR_IE_FOUND },
	[MORSE_VENDOR_EVENT_MESH_PEER_ADDR] = {
					       .vendor_id = MORSE_OUI,
					       .subcmd = MORSE_VENDOR_EVENT_MESH_PEER_ADDR },
	[MORSE_VENDOR_EVENT_BSS_STATS] = {
							.vendor_id = MORSE_OUI,
							.subcmd = MORSE_VENDOR_EVENT_BSS_STATS },
};

void morse_set_vendor_commands_and_events(struct wiphy *wiphy)
{
	wiphy->vendor_commands = morse_vendor_commands;
	wiphy->n_vendor_commands = ARRAY_SIZE(morse_vendor_commands);
	wiphy->vendor_events = morse_vendor_events;
	wiphy->n_vendor_events = ARRAY_SIZE(morse_vendor_events);
}

static void put_vendor_ie(struct dot11_morse_vendor_caps_ops_ie *data,
			  struct dot11ah_ies_mask *ies_mask)
{
	struct ie_element *element;
	struct ieee80211_vendor_ie ie = {
		.element_id = WLAN_EID_VENDOR_SPECIFIC,
		.len = sizeof(*data),
	};

	/* Fill oui */
	memcpy(&data->oui, morse_oui, sizeof(ie.oui));
	data->oui_type = MORSE_VENDOR_IE_CAPS_OPS_OUI_TYPE;

	element = morse_dot11_ies_create_ie_element(ies_mask, ie.element_id, ie.len, true, false);

	if (!element)
		return;

	memcpy(element->ptr, (u8 *)data, ie.len);
}

static bool is_morse_vendor_caps_ops_ie_allowed(struct ieee80211_vif *vif,
						const struct ieee80211_mgmt *mgmt)
{
	bool ret = false;
	bool is_assoc_reassoc_req = (ieee80211_is_assoc_req(mgmt->frame_control) ||
				     ieee80211_is_reassoc_req(mgmt->frame_control));
	bool is_assoc_reassoc_resp = (ieee80211_is_assoc_resp(mgmt->frame_control) ||
				      ieee80211_is_reassoc_resp(mgmt->frame_control));

	switch (vif->type) {
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_STATION:
		if (is_assoc_reassoc_req || is_assoc_reassoc_resp)
			ret = true;
		break;
	case NL80211_IFTYPE_MESH_POINT:
		if (morse_dot11_is_mpm_open_frame(mgmt))
			ret = true;
		break;
	default:
		break;
	}

	return ret;
}

void morse_vendor_insert_caps_ops_ie(struct morse *mors,
				     struct ieee80211_vif *vif,
				     struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct dot11_morse_vendor_caps_ops_ie ie_data = { 0 };
	struct morse_sta *mors_sta = NULL;
	struct ieee80211_sta *sta = NULL;
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
	bool is_assoc_reassoc_req = (ieee80211_is_assoc_req(mgmt->frame_control) ||
				     ieee80211_is_reassoc_req(mgmt->frame_control));
	bool is_assoc_reassoc_resp = (ieee80211_is_assoc_resp(mgmt->frame_control) ||
				      ieee80211_is_reassoc_resp(mgmt->frame_control));

	if (!ies_mask || !enable_mm_vendor_ie)
		return;

	if (!is_morse_vendor_caps_ops_ie_allowed(vif, mgmt))
		return;

	/* Fill common version information */
	ie_data.hw_ver = mors->chip_id;
	ie_data.sw_ver.reserved = 0;
	ie_data.sw_ver.major = mors->sw_ver.major;
	ie_data.sw_ver.minor = mors->sw_ver.minor;
	ie_data.sw_ver.patch = mors->sw_ver.patch;

	/* If the chip has signalled a non-zero MMSS requirement, then
	 * communicate the morse mmss offset (if set), else set to 0.
	 */
	ie_data.cap0 |= MORSE_VENDOR_IE_CAP0_SET_MMSS_OFFSET((mors_vif->capabilities.ampdu_mss >
					      0) ? mors_vif->capabilities.morse_mmss_offset : 0);

	ie_data.cap0 |= MORSE_VENDOR_IE_CAP0_SHORT_ACK_TIMEOUT;

	if (mors_vif->enable_pv1)
		ie_data.cap0 |= MORSE_VENDOR_IE_CAP0_PV1_DATA_FRAME_SUPPORT;

	if (mors_vif->page_slicing_info.enabled)
		ie_data.cap0 |= MORSE_VENDOR_IE_CAP0_PAGE_SLICING_EXCLUSIVE_SUPPORT;

	ie_data.ops0 |= BMSET(mors->rc_method, MORSE_VENDOR_IE_OPS0_RATE_CONTROL);

	if (vif->type == NL80211_IFTYPE_AP) {
		/* Always indicate usage of DTIM CTS-to-self */
		if (MORSE_OPS_IN_USE(&mors_vif->operations, DTIM_CTS_TO_SELF))
			ie_data.ops0 |= MORSE_VENDOR_IE_OPS0_DTIM_CTS_TO_SELF;

		/* See if we need to negotiate LEGACY AMSDU for sta */
		if (is_assoc_reassoc_resp) {
			/* Must be held while finding and dereferencing sta */
			rcu_read_lock();
			sta = ieee80211_find_sta(vif, mgmt->da);
			if (sta)
				mors_sta = (struct morse_sta *)sta->drv_priv;

			if (mors_sta && mors_sta->vendor_info.valid) {
				/* STA has previously indicated that it would like AMSDU */
				if (MORSE_OPS_IN_USE(&mors_sta->vendor_info.operations,
						     LEGACY_AMSDU)) {
					if (mors->custom_configs.enable_legacy_amsdu)
						ie_data.ops0 |= MORSE_VENDOR_IE_OPS0_LEGACY_AMSDU;
					else
						MORSE_OPS_CLEAR(&mors_sta->vendor_info.operations,
								LEGACY_AMSDU);
				}
			}
			rcu_read_unlock();
		}
	} else if (vif->type == NL80211_IFTYPE_STATION) {
		if (is_assoc_reassoc_req) {
			/* Attempt to negotiate Legacy AMSDU */
			if (mors->custom_configs.enable_legacy_amsdu)
				ie_data.ops0 |= MORSE_VENDOR_IE_OPS0_LEGACY_AMSDU;
		}
	}

	put_vendor_ie(&ie_data, ies_mask);
}

struct dot11_morse_vendor_caps_ops_ie *
morse_vendor_find_vendor_ie(struct dot11ah_ies_mask *ies_mask)
{
	struct dot11_morse_vendor_caps_ops_ie *ie = NULL;
	struct ie_element *cur = &ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC];
	bool found = false;

	while (cur && cur->ptr) {
		ie = (struct dot11_morse_vendor_caps_ops_ie *)cur->ptr;
		if (memcmp(ie->oui, morse_oui, sizeof(ie->oui)) == 0 &&
		    ie->oui_type == MORSE_VENDOR_IE_CAPS_OPS_OUI_TYPE) {
			found = true;
			break;
		}
		cur = cur->next;
	}

	if (found)
		return ie;

	return NULL;
}

void morse_vendor_fill_sta_vendor_info(struct morse_vif *mors_vif,
				       struct dot11_morse_vendor_caps_ops_ie *ie)
{
	struct morse *mors = morse_vif_to_morse(mors_vif);

	memset(&mors_vif->bss_vendor_info, 0, sizeof(mors_vif->bss_vendor_info));
	mors_vif->bss_vendor_info.valid = true;
	mors_vif->bss_vendor_info.chip_id = ie->hw_ver;
	mors_vif->bss_vendor_info.sw_ver.major = ie->sw_ver.major;
	mors_vif->bss_vendor_info.sw_ver.minor = ie->sw_ver.minor;
	mors_vif->bss_vendor_info.sw_ver.patch = ie->sw_ver.patch;
	mors_vif->bss_vendor_info.morse_mmss_offset =
		MORSE_VENDOR_IE_CAP0_GET_MMSS_OFFSET(ie->cap0);
	mors_vif->bss_vendor_info.rc_method = BMGET(ie->ops0,
						    MORSE_VENDOR_IE_OPS0_RATE_CONTROL);

	if (ie->ops0 & MORSE_VENDOR_IE_OPS0_DTIM_CTS_TO_SELF)
		MORSE_OPS_SET(&mors_vif->bss_vendor_info.operations, DTIM_CTS_TO_SELF);

	mors_vif->bss_vendor_info.supports_short_ack_timeout =
	    (ie->cap0 & MORSE_VENDOR_IE_CAP0_SHORT_ACK_TIMEOUT);

	mors_vif->bss_vendor_info.pv1_data_frame_only_support =
		(ie->cap0 & MORSE_VENDOR_IE_CAP0_PV1_DATA_FRAME_SUPPORT);

	/* Enable Page slicing capability if chip supports it */
	if (mors_vif->page_slicing_info.enabled)
		mors_vif->bss_vendor_info.page_slicing_exclusive_support =
			(ie->cap0 & MORSE_VENDOR_IE_CAP0_PAGE_SLICING_EXCLUSIVE_SUPPORT);

	if ((ie->ops0 & MORSE_VENDOR_IE_OPS0_LEGACY_AMSDU) &&
	    mors->custom_configs.enable_legacy_amsdu) {
		/* AP agreed to our request for LEGACY AMSDU */
		MORSE_OPS_SET(&mors_vif->operations, LEGACY_AMSDU);
		MORSE_OPS_SET(&mors_vif->bss_vendor_info.operations, LEGACY_AMSDU);
	} else {
		MORSE_OPS_CLEAR(&mors_vif->operations, LEGACY_AMSDU);
	}
}

void morse_vendor_rx_caps_ops_ie(struct morse_vif *mors_vif,
				 const struct ieee80211_mgmt *mgmt,
				 struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct dot11_morse_vendor_caps_ops_ie *ie = NULL;
	struct morse_sta *mors_sta = NULL;
	struct ieee80211_sta *sta = NULL;
	bool is_assoc_reassoc_req = (ieee80211_is_assoc_req(mgmt->frame_control) ||
				     ieee80211_is_reassoc_req(mgmt->frame_control));
	bool is_assoc_reassoc_resp = (ieee80211_is_assoc_resp(mgmt->frame_control) ||
				      ieee80211_is_reassoc_resp(mgmt->frame_control));
	bool is_mesh_open_frame = morse_dot11_is_mpm_open_frame(mgmt);

	ie = morse_vendor_find_vendor_ie(ies_mask);

	if (!ie)
		return;

	if (!is_morse_vendor_caps_ops_ie_allowed(vif, mgmt))
		return;

	if ((vif->type == NL80211_IFTYPE_AP && is_assoc_reassoc_req) ||
	    (ieee80211_vif_is_mesh(vif) && is_mesh_open_frame)) {
		/* Must be held while finding and dereferencing sta */
		rcu_read_lock();
		sta = ieee80211_find_sta(vif, mgmt->sa);
		if (sta) {
			mors_sta = (struct morse_sta *)sta->drv_priv;
			memset(&mors_sta->vendor_info, 0, sizeof(mors_sta->vendor_info));

			/* Unconditionally fill version information */
			mors_sta->vendor_info.valid = true;
			mors_sta->vendor_info.chip_id = ie->hw_ver;
			mors_sta->vendor_info.sw_ver.major = ie->sw_ver.major;
			mors_sta->vendor_info.sw_ver.minor = ie->sw_ver.minor;
			mors_sta->vendor_info.sw_ver.patch = ie->sw_ver.patch;
			mors_sta->vendor_info.morse_mmss_offset =
				MORSE_VENDOR_IE_CAP0_GET_MMSS_OFFSET(ie->cap0);
			mors_sta->vendor_info.rc_method = BMGET(ie->ops0,
								MORSE_VENDOR_IE_OPS0_RATE_CONTROL);

			if ((ie->ops0 & MORSE_VENDOR_IE_OPS0_LEGACY_AMSDU) &&
			    mors->custom_configs.enable_legacy_amsdu)
				MORSE_OPS_SET(&mors_sta->vendor_info.operations, LEGACY_AMSDU);
			else
				MORSE_OPS_CLEAR(&mors_sta->vendor_info.operations, LEGACY_AMSDU);

			mors_sta->vendor_info.supports_short_ack_timeout =
			    (ie->cap0 & MORSE_VENDOR_IE_CAP0_SHORT_ACK_TIMEOUT);

			mors_sta->vendor_info.pv1_data_frame_only_support =
				(ie->cap0 & MORSE_VENDOR_IE_CAP0_PV1_DATA_FRAME_SUPPORT);
		}
		rcu_read_unlock();
	} else if (vif->type == NL80211_IFTYPE_STATION && is_assoc_reassoc_resp) {
		morse_vendor_fill_sta_vendor_info(mors_vif, ie);
	}
}

void morse_vendor_reset_sta_transient_info(struct ieee80211_vif *vif, struct morse_sta *mors_sta)
{
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;

	memset(&mors_sta->vendor_info, 0, sizeof(mors_sta->vendor_info));
	if (vif->type == NL80211_IFTYPE_STATION) {
		memset(&mors_vif->operations, 0, sizeof(mors_vif->operations));
		memset(&mors_vif->bss_vendor_info, 0, sizeof(mors_vif->bss_vendor_info));
	}
}

int morse_vendor_get_ie_len_for_pkt(struct sk_buff *pkt, int oui_type)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)pkt->data;

	if (!enable_mm_vendor_ie)
		return 0;

	if (!(ieee80211_is_assoc_req(hdr->frame_control) ||
	      ieee80211_is_reassoc_req(hdr->frame_control) ||
	      ieee80211_is_assoc_resp(hdr->frame_control) ||
	      ieee80211_is_reassoc_resp(hdr->frame_control)))
		return 0;	/* No insertion of IE on any other frames */

	if (oui_type != MORSE_VENDOR_IE_CAPS_OPS_OUI_TYPE)
		return 0;

	return sizeof(struct dot11_morse_vendor_caps_ops_ie) + 2;
}

static int morse_vendor_send_bcn_vendor_ie_found_event(struct wireless_dev *wdev,
						const struct ieee80211_vendor_ie *vie)
{
	struct sk_buff *skb;
	int ret;

	skb = cfg80211_vendor_event_alloc(wdev->wiphy, NULL, vie->len + VENDOR_EVENT_OVERHEAD,
					  MORSE_VENDOR_EVENT_BCN_VENDOR_IE_FOUND, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	ret = nla_put(skb, MORSE_VENDOR_ATTR_DATA, vie->len, vie->oui);
	if (ret) {
		kfree_skb(skb);
		return ret;
	}

	cfg80211_vendor_event(skb, GFP_ATOMIC);
	return 0;
}

int morse_vendor_send_mgmt_vendor_ie_found_event(struct wireless_dev *wdev, u16 frame_type,
						 const struct ieee80211_vendor_ie *vie)
{
	struct sk_buff *skb;
	int ret;

	skb = cfg80211_vendor_event_alloc(wdev->wiphy, NULL,
					  vie->len + VENDOR_EVENT_OVERHEAD + sizeof(frame_type),
					  MORSE_VENDOR_EVENT_MGMT_VENDOR_IE_FOUND, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	ret = nla_put_u16(skb, MORSE_VENDOR_ATTR_MGMT_FRAME_TYPE, frame_type);
	if (ret)
		goto err;

	ret = nla_put(skb, MORSE_VENDOR_ATTR_DATA, vie->len, vie->oui);
	if (ret)
		goto err;

	cfg80211_vendor_event(skb, GFP_ATOMIC);

	/* Also send legacy vendor IE event if a beacon */
	if (frame_type == MORSE_VENDOR_IE_TYPE_BEACON)
		ret = morse_vendor_send_bcn_vendor_ie_found_event(wdev, vie);

	return ret;

err:
	kfree_skb(skb);
	return ret;
}

int morse_vendor_send_ocs_done_event(struct ieee80211_vif *vif, struct morse_cmd_evt_ocs_done *evt)
{
	struct wireless_dev *wdev;
	struct sk_buff *skb;
	int ret;
	size_t ocs_data_size = sizeof(*evt) - sizeof(evt->hdr);

	if (!vif)
		return -EIO;

	wdev = ieee80211_vif_to_wdev(vif);

	/* cast to prevent warnings from Sparse */
	evt->time_listen = (__force __le64)le64_to_cpu(evt->time_listen);
	evt->time_rx = (__force __le64)le64_to_cpu(evt->time_rx);

	skb = cfg80211_vendor_event_alloc(wdev->wiphy, NULL, ocs_data_size,
					  MORSE_VENDOR_EVENT_OCS_DONE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	/* Offset ocs evt pointer so hdr data is not passed to cfg80211 */
	ret = nla_put(skb, MORSE_VENDOR_ATTR_DATA, ocs_data_size,
		(void *)((char *)evt + sizeof(evt->hdr)));
	if (ret < 0) {
		kfree_skb(skb);
		return ret;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return ret;
}

int morse_vendor_send_peer_addr_event(struct ieee80211_vif *vif,
				      struct morse_mesh_peer_addr_vendor_evt *peer_addr_evt)
{
	struct wireless_dev *wdev;
	struct sk_buff *skb;
	int ret;

	if (!vif)
		return -EIO;

	wdev = ieee80211_vif_to_wdev(vif);

	skb = cfg80211_vendor_event_alloc(wdev->wiphy, NULL, sizeof(*peer_addr_evt),
					  MORSE_VENDOR_EVENT_MESH_PEER_ADDR, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	ret = nla_put(skb, MORSE_VENDOR_ATTR_DATA, sizeof(*peer_addr_evt),
		      peer_addr_evt);
	if (ret < 0) {
		kfree_skb(skb);
		return ret;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return ret;
}

void morse_vendor_update_ack_timeout_on_assoc(struct morse *mors,
					      struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	const int minimum_req_ack_timeout_us = 1000;
	struct morse_vendor_info *info = NULL;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);

	if (vif->type == NL80211_IFTYPE_STATION)
		info = &(ieee80211_vif_to_morse_vif(vif)->bss_vendor_info);
	else if (vif->type == NL80211_IFTYPE_AP)
		info = &((struct morse_sta *)sta->drv_priv)->vendor_info;

	if (!info || !info->valid)
		goto exit;

	if (info->supports_short_ack_timeout)
		return;

	if (mors->extra_ack_timeout_us >= minimum_req_ack_timeout_us)
		return;

exit:
	MORSE_DBG(mors, "%s: Increasing ctrl resp wait time to: %dus",
		  __func__, minimum_req_ack_timeout_us);

	morse_cmd_ack_timeout_adjust(mors, mors_vif->id, minimum_req_ack_timeout_us);
}

int morse_vendor_send_bss_stats_event(struct ieee80211_vif *vif,
			struct morse_evt_bss_stats *evt, size_t evt_data_len)
{
	struct wireless_dev *wdev;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct sk_buff *skb;
	int ret = 0;

	if (!vif)
		return -EIO;

	wdev = ieee80211_vif_to_wdev(vif);

	if (!wdev)
		return -EIO;

	skb = cfg80211_vendor_event_alloc(wdev->wiphy, NULL, evt_data_len,
					MORSE_VENDOR_EVENT_BSS_STATS, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	ret = nla_put(skb, MORSE_VENDOR_ATTR_DATA, evt_data_len, evt);
	if (ret < 0) {
		MORSE_ERR(mors, "%s: Failed to prepare BSS stats event buffer. num_stas: %d err:%d",
		  __func__, evt->num_stas, ret);
		kfree_skb(skb);
		return ret;
	}
	MORSE_DBG(mors, "%s: Success in sending BSS stats event. num_stas: %d active_stas:%d",
		  __func__, evt->num_stas, evt->num_active_stas);
	cfg80211_vendor_event(skb, GFP_KERNEL);
	return ret;
}
