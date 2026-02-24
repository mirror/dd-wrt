/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/etherdevice.h>
#include <linux/slab.h>
#include <linux/ieee80211.h>
#include <linux/inetdevice.h>
#include <linux/jiffies.h>
#include <linux/crc32.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>
#include <net/sock.h>
#include <linux/notifier.h>
#include <linux/rtnetlink.h>
#include <linux/workqueue.h>
#include <asm/div64.h>

#include "bus.h"
#include "command.h"
#include "debug.h"
#include "mac.h"
#include "morse.h"
#include "ps.h"
#include "utils.h"
#include "vendor.h"
#include "vendor_ie.h"
#include "wiphy.h"

/**
 * The maximum number of SSIDs we support scanning for in a single scan request.
 */
#define SCAN_MAX_SSIDS 1

/**
 * Number of bytes of extra padding to be inserted at the start of each tx packet.
 *
 * Fullmac firmware needs at most 26 bytes so that it can do 802.3 to 802.11 header
 * translation in place.
 *
 * Arithmetic for this offset:
 * 1. 12 bytes added to the offset from removing 2 MAC addresses
 * 2. 6 bytes removed from the offset from adding LLC/SNAP header
 * 3. 2 bytes removed form the offset from QoS header
 * 4. 24 bytes removed from the offset from 80211 MAC header, 30 bytes in 4-address mode
 */
#define EXTRA_TX_OFFSET 26

struct connect_tlv_hdr {
	__le16 tag;
	__le16 len;
} __packed;

struct connect_tlv_auth_type {
	struct connect_tlv_hdr hdr;
	u8 auth_type;
} __packed;

struct connect_tlv_ssid {
	struct connect_tlv_hdr hdr;
	u8 ssid[];
} __packed;

struct connect_tlv_sae_pwd {
	struct connect_tlv_hdr hdr;
	u8 sae_pwd[];
} __packed;

struct connect_tlv_extra_assoc_ies {
	struct connect_tlv_hdr hdr;
	u8 ies[];
} __packed;

struct connect_tlv_bssid {
	struct connect_tlv_hdr hdr;
	u8 bssid[ETH_ALEN];
} __packed;

struct connect_tlv_bg_scan_period {
	struct connect_tlv_hdr hdr;
	__le16 bg_scan_period;
} __packed;

struct connect_tlv_4addr_mode {
	struct connect_tlv_hdr hdr;
	u8 enabled;
} __packed;

/**
 * morse_wiphy_privid -  privid value for our wiphy
 *
 * This lets us distinguish it from ieee80211_hw owned by mac80211.
 */
static const void *const morse_wiphy_privid = &morse_wiphy_privid;

/*
 * sta_vif - singleton STA vif in fullmac mode
 *
 * Fullmac only supports one station virtual interface. This is a pointer to its private structure,
 * or NULL if it was removed.
 *
 * TODO: refactor private struct layout to allow multiple vifs, remove this global pointer.
 */
static struct morse_vif *sta_vif;

static const struct ieee80211_txrx_stypes
morse_wiphy_txrx_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_REQ >> 4),
	},
};

struct morse_vif *morse_wiphy_get_sta_vif(struct morse *mors)
{
	return sta_vif;
}

struct morse *morse_wiphy_to_morse(struct wiphy *wiphy)
{
	struct ieee80211_hw *hw;

	/* If we were loaded in fullmac mode, our struct morse is the priv structure in wiphy,
	 * there is no ieee80211_hw.
	 */
	if (wiphy->privid == morse_wiphy_privid)
		return wiphy_priv(wiphy);

	/* In softmac mode, mac80211 has installed struct ieee80211_hw as the priv structure
	 * in wiphy, ours is inside that.
	 */
	hw = wiphy_to_ieee80211_hw(wiphy);
	return hw->priv;
}

struct morse_vif *morse_wdev_to_morse_vif(struct wireless_dev *wdev)
{
	struct ieee80211_vif *vif;
	/* If we were loaded in fullmac mode, our struct morse_vif is the container of wdev.
	 */
	if (wdev->wiphy->privid == morse_wiphy_privid)
		return container_of(wdev, struct morse_vif, wdev);

	/* In softmac mode, mac80211 has installed morse_vif in ieee80211_vif
	 * as the private structure.
	 */
	vif = wdev_to_ieee80211_vif(wdev);
	if (vif)
		return ieee80211_vif_to_morse_vif(vif);

	/* We return NULL when we're in sniffer mode for now. */
	return NULL;
}

/**
 * morse_wiphy_dot11ah_channel_to_5g - Map 802.11ah channel to 5GHz channel.
 */
static struct ieee80211_channel *morse_wiphy_dot11ah_channel_to_5g(struct wiphy *wiphy,
						const struct morse_dot11ah_channel *chan_s1g)
{
	struct ieee80211_supported_band *sband = wiphy->bands[NL80211_BAND_5GHZ];
	int i;

	if (WARN_ON(!chan_s1g))
		return NULL;

	for (i = 0; i < sband->n_channels; i++) {
		if (sband->channels[i].hw_value == chan_s1g->hw_value_map)
			return &sband->channels[i];
	}

	WARN(1, "5GHz channel mapping not defined");
	return NULL;
}

/**
 * morse_wiphy_get_5g_channel - Get 5GHz channel by its channel number.
 */
static struct ieee80211_channel *morse_wiphy_get_5g_channel(struct wiphy *wiphy, int chan_5g)
{
	struct ieee80211_supported_band *sband = wiphy->bands[NL80211_BAND_5GHZ];
	int i;

	for (i = 0; i < sband->n_channels; i++) {
		if (sband->channels[i].hw_value == chan_5g)
			return &sband->channels[i];
	}

	return NULL;
}

/**
 * morse_wiphy_fixed_rate - Send command for fixed transmission rate modparam.
 */
static int morse_wiphy_fixed_rate(struct morse *mors)
{
	s32 bw, mcs;
	s8 sgi, ss, nss;
	u8 enable = morse_rc_get_enable_fixed_rate();

	if (!enable)
		return MORSE_RET_SUCCESS;

	switch (morse_rc_get_fixed_bandwidth()) {
	case 0:
		bw = 1;
		break;
	case 1:
		bw = 2;
		break;
	case 2:
		bw = 4;
		break;
	case 3:
		bw = 8;
		break;
	default:
		bw = -1;
		break;
	}

	mcs = morse_rc_get_fixed_mcs();
	sgi = morse_rc_get_fixed_guard();

	ss = morse_rc_get_fixed_ss();
	nss = (ss == -1) ? -1 : (ss - 1);

	return morse_cmd_set_fixed_transmission_rate(mors, bw, mcs, sgi, nss);
}

static void morse_wiphy_update_arp_filter(struct morse_vif *mors_vif)
{
	struct morse *mors = wiphy_priv(mors_vif->wdev.wiphy);
	int ret;

	lockdep_assert_held(&mors->lock);

	if (!mors_vif->custom_configs->enable_arp_offload)
		return;

	if (!test_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state))
		return;

	ret = morse_cmd_arp_offload_update_ip_table(mors, mors_vif->id,
						    mors_vif->arp_filter.addr_cnt,
						    mors_vif->arp_filter.addr_list);
	if (ret)
		MORSE_ERR(mors, "failed to configure ARP offload address table: %d\n", ret);
}

static int morse_ndev_open(struct net_device *dev)
{
	struct morse_vif *mors_vif = netdev_priv(dev);
	struct wireless_dev *wdev = &mors_vif->wdev;
	struct morse *mors = wiphy_priv(wdev->wiphy);
	int ret;

	/* Carrier state is initially off. It will be set on when a connection is established.
	 */
	netif_carrier_off(dev);

	mutex_lock(&mors->lock);

	ret = morse_cmd_set_country(mors, mors->country);
	if (ret)
		goto out;

	ret = morse_cmd_set_rate_control(mors);
	if (ret)
		goto out;

	ret = morse_wiphy_fixed_rate(mors);
	if (ret)
		goto out;

	ret = morse_cmd_add_if(mors, &mors_vif->id, dev->dev_addr, wdev->iftype);
	if (ret)
		goto out;
	if (wdev->iftype == NL80211_IFTYPE_MONITOR)
		mors->mon_if.id = mors_vif->id;

	morse_vendor_ie_init_interface(mors_vif);

	if (mors->cfg->set_slow_clock_mode)
		mors->cfg->set_slow_clock_mode(mors, morse_mac_slow_clock_mode());

	mors->started = true;

out:
	mutex_unlock(&mors->lock);

	return ret;
}

static int morse_ndev_close(struct net_device *dev)
{
	struct morse_vif *mors_vif = netdev_priv(dev);
	struct wireless_dev *wdev = &mors_vif->wdev;
	struct morse *mors = wiphy_priv(wdev->wiphy);
	int ret = 0;

	mutex_lock(&mors->lock);

	if (!mors->started)
		goto out;

	morse_wiphy_cleanup(mors);

	morse_vendor_ie_deinit_interface(mors_vif);

	ret = morse_cmd_rm_if(mors, mors_vif->id);

	if (wdev->iftype == NL80211_IFTYPE_MONITOR)
		mors->mon_if.id = INVALID_VIF_ID;
	mors->started = false;

out:
	mutex_unlock(&mors->lock);

	return ret;
}

static netdev_tx_t morse_ndev_data_tx(struct sk_buff *skb, struct net_device *dev)
{
	int ret;
	int aci;
	struct morse_skbq *mq;

	struct morse_vif *mors_vif = netdev_priv(dev);
	struct morse *mors = wiphy_priv(mors_vif->wdev.wiphy);
	struct morse_skb_tx_info tx_info = { 0 };

	sk_pacing_shift_update(skb->sk, SK_PACING_SHIFT);

	skb->priority = cfg80211_classify8021d(skb, NULL);
	aci = dot11_tid_to_ac(skb->priority);
	mq = mors->cfg->ops->skbq_tc_q_from_aci(mors, aci);
	tx_info.tid = skb->priority;

	ret = morse_skbq_skb_tx(mq, &skb, &tx_info, MORSE_SKB_CHAN_WIPHY);
	if (ret < 0)
		goto tx_err;

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	return NETDEV_TX_OK;

tx_err:
	MORSE_ERR_RATELIMITED(mors, "%s failed with error [%d]\n", __func__, ret);
	dev->stats.tx_dropped++;
	dev->stats.tx_aborted_errors++;

	return NETDEV_TX_OK;
}

/** Network device operations vector table */
static const struct net_device_ops mors_netdev_ops = {
	.ndo_open = morse_ndev_open,
	.ndo_stop = morse_ndev_close,
	.ndo_start_xmit = morse_ndev_data_tx,
	.ndo_set_mac_address = eth_mac_addr,
	/*
	 * TBD
	 * Place holder of what we need to do. Do not remove
	 */
	/*
	 * .ndo_set_features       = morse_netdev_set_features,
	 * .ndo_set_rx_mode     = morse_netdev_set_multicast_list,
	 */
};

static void morse_netdev_init(struct net_device *dev, struct morse *mors)
{
	dev->netdev_ops = &mors_netdev_ops;
	dev->watchdog_timeo = 10;
	dev->needed_headroom = ETH_HLEN + sizeof(struct morse_buff_skb_header) +
			       mors->bus_ops->bulk_alignment +
			       mors->extra_tx_offset;
#if KERNEL_VERSION(4, 12, 0) <= LINUX_VERSION_CODE
	dev->needs_free_netdev = true;
#else
	dev->destructor = free_netdev;
#endif
}

/** Ethernet Tool operations */
static const struct ethtool_ops mors_ethtool_ops = {
	/*
	 * TBD
	 * Place holder of what we need to do. Do not remove
	 */
	/*
	 * .get_drvinfo = morse_ethtool__get_drvinfo,
	 * .get_link = morse_ethtool_op_get_link,
	 * .get_strings = morse_ethtool_et_strings,
	 * .get_ethtool_stats = morse_ethtool_et_stats,
	 * .get_sset_count = morse_ethtool_et_sset_count,
	 */
};

/**
 * morse_wiphy_fill_vendor_ies() - Copy custom vendor IEs into destination buffer.
 *
 * @mors_vif: morse virtual interface
 * @dest: destination buffer to put the IEs in
 * @mgmt_type_mask: type of the management frame that these IEs will go into
 */
static void
morse_wiphy_fill_vendor_ies(struct morse_vif *mors_vif, u8 *dest,
			   enum morse_vendor_ie_mgmt_type_flags mgmt_type_mask)
{
	struct vendor_ie_list_item *vendor_ie, *tmp;
	u16 dest_idx = 0;

	spin_lock_bh(&mors_vif->vendor_ie.lock);

	list_for_each_entry_safe(vendor_ie, tmp, &mors_vif->vendor_ie.ie_list, list) {
		if (vendor_ie->mgmt_type_mask & mgmt_type_mask) {
			dest[dest_idx++] = vendor_ie->ie.element_id;
			dest[dest_idx++] = vendor_ie->ie.len;
			memcpy(&dest[dest_idx], vendor_ie->ie.oui, vendor_ie->ie.len);
			dest_idx += vendor_ie->ie.len;
		}
	}

	spin_unlock_bh(&mors_vif->vendor_ie.lock);
}

static int morse_wiphy_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
{
	struct morse *mors = wiphy_priv(wiphy);
	struct morse_vif *mors_vif = sta_vif;
	u32 dwell_time_ms = 0;
	const u8 *ssid = NULL;
	size_t ssid_len = 0;
	u16 vendor_ie_len = 0;
	u8 *extra_ies = NULL;
	u16 extra_ies_len = 0;
	int ret;

	/* We configured these limits in struct wiphy. */
	if (request->n_ssids > SCAN_MAX_SSIDS ||
	    request->ie_len > MORSE_CMD_SCAN_EXTRA_IES_MAX_LEN) {
		MORSE_WARN_ON_ONCE(FEATURE_ID_DEFAULT, 1);
		return -EFAULT;
	}

	mutex_lock(&mors->lock);

	if (test_bit(MORSE_SCAN_STATE_SCANNING, &mors->scan_state)) {
		ret = -EBUSY;
		goto out;
	}

	/* TODO: obey channels, mac_addr, mac_addr_mask, bssid, scan_width */
	/* TODO: apply a timeout to the scan operation on the driver side */

	if (request->n_ssids) {
		ssid = request->ssids[0].ssid;
		ssid_len = request->ssids[0].ssid_len;
	}
	if (request->duration)
		dwell_time_ms = MORSE_TU_TO_MS(request->duration);
	vendor_ie_len = morse_vendor_ie_get_ies_length(mors_vif, MORSE_VENDOR_IE_TYPE_PROBE_REQ);
	if (request->ie_len + vendor_ie_len > MORSE_CMD_EXTRA_ASSOC_IES_MAX_LEN) {
		MORSE_INFO(mors, "Probe request IEs too long: %u > %u\n",
			   (u32)(request->ie_len + vendor_ie_len),
			   MORSE_CMD_EXTRA_ASSOC_IES_MAX_LEN);
		ret = -E2BIG;
		goto out;
	}
	if (request->ie_len || vendor_ie_len)
		extra_ies = kmalloc(MORSE_CMD_SCAN_EXTRA_IES_MAX_LEN, GFP_KERNEL);
	if (request->ie_len) {
		memcpy(extra_ies, request->ie, request->ie_len);
		extra_ies_len = request->ie_len;
	}
	if (vendor_ie_len) {
		morse_wiphy_fill_vendor_ies(mors_vif, &extra_ies[extra_ies_len],
					    MORSE_VENDOR_IE_TYPE_PROBE_REQ);
		extra_ies_len += vendor_ie_len;
	}

	ret = morse_cmd_start_scan(mors, request->n_ssids, ssid, ssid_len, extra_ies,
				   extra_ies_len, dwell_time_ms);
	if (ret)
		goto out;

	set_bit(MORSE_SCAN_STATE_SCANNING, &mors->scan_state);
	mors->scan_req = request;

out:
	kfree(extra_ies);
	mutex_unlock(&mors->lock);

	return ret;
}

static void morse_wiphy_abort_scan(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct morse *mors = wiphy_priv(wiphy);
	int ret;

	mutex_lock(&mors->lock);

	if (!test_bit(MORSE_SCAN_STATE_SCANNING, &mors->scan_state))
		goto out;

	ret = morse_cmd_abort_scan(mors);
	if (ret)
		MORSE_ERR(mors, "failed to abort scan: %d\n", ret);

out:
	mutex_unlock(&mors->lock);
}

static void morse_wiphy_scan_done_work(struct work_struct *work)
{
	struct morse *mors = container_of(work, struct morse, scan_done_work);
	struct cfg80211_scan_info info = { 0 };

	mutex_lock(&mors->lock);

	if (WARN_ON(!test_and_clear_bit(MORSE_SCAN_STATE_SCANNING, &mors->scan_state)))
		goto exit;
	if (WARN_ON(!mors->scan_req))
		goto exit;

	info.aborted = test_and_clear_bit(MORSE_SCAN_STATE_ABORTED, &mors->scan_state);

	cfg80211_scan_done(mors->scan_req, &info);

	mors->scan_req = NULL;

exit:
	mutex_unlock(&mors->lock);
}

/**
 * morse_wiphy_filter_assoc_ie() - Filter assoc request IEs before sending to the chip.
 *
 * @assoc_ie: buffer containing original IEs
 * @assoc_ie_len: length of @ref assoc_ie
 * @dest: buffer to copy filtered IEs into, should have the length of @ref assoc_ie_len
 *
 * Return: length of filtered IEs in dest
 */
static int morse_wiphy_filter_assoc_ie(const u8 *assoc_ie, size_t assoc_ie_len, u8 *dest)
{
	size_t dest_idx = 0;
	const u8 *ie_iter;

	for (ie_iter = assoc_ie;
	     assoc_ie + assoc_ie_len - ie_iter >= 3 &&
	     assoc_ie + assoc_ie_len - ie_iter >= ie_iter[1];
	     ie_iter = &ie_iter[2] + ie_iter[1]) {
		u8 ie_id = ie_iter[0];
		u8 ie_data_len = ie_iter[1];

		switch (ie_id) {
		/* Firmware is responsible for generating these IEs, we should not send them down
		 * in the connect command.
		 */
		case WLAN_EID_RSN:
		case WLAN_EID_RSNX:
		case WLAN_EID_EXT_CAPABILITY:
		case WLAN_EID_SUPPORTED_REGULATORY_CLASSES:
			break;
		default:
			memcpy(&dest[dest_idx], ie_iter, ie_data_len + 2);
			dest_idx += ie_data_len + 2;
			break;
		}
	}

	return dest_idx;
}

static int connect_build_extra_assoc_ies(struct morse_wiphy_connect_params *params,
					 struct morse_vif *mors_vif,
					 struct cfg80211_connect_params *sme)
{
	u16 vendor_ies_len =
		morse_vendor_ie_get_ies_length(mors_vif, MORSE_VENDOR_IE_TYPE_ASSOC_REQ);
	int filtered_ies_len;
	u8 *ies;

	ies = kzalloc(sme->ie_len + vendor_ies_len, GFP_KERNEL);
	if (!ies)
		return -ENOMEM;

	params->extra_assoc_ies = ies;
	params->extra_assoc_ies_len = 0;

	/* Fill in IEs from host supplicant, filtered down to the ones we will obey. */
	filtered_ies_len = morse_wiphy_filter_assoc_ie(sme->ie, sme->ie_len, ies);
	params->extra_assoc_ies_len += filtered_ies_len;
	ies += filtered_ies_len;

	/* Fill in additional vendor IEs. */
	morse_wiphy_fill_vendor_ies(mors_vif, ies, MORSE_VENDOR_IE_TYPE_ASSOC_REQ);
	params->extra_assoc_ies_len += vendor_ies_len;
	ies += vendor_ies_len;

	return 0;
}

static inline struct connect_tlv_hdr connect_pack_tlv_hdr(u16 tag, u16 len)
{
	struct connect_tlv_hdr hdr = {
		.tag = cpu_to_le16(tag),
		.len = cpu_to_le16(len)
	};
	return hdr;
}

static u8 *connect_insert_auth_type_tlv(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_auth_type *auth_type = (struct connect_tlv_auth_type *)buf;

	auth_type->hdr = connect_pack_tlv_hdr(MORSE_CMD_CONNECT_TLV_TAG_AUTH_TYPE,
					      sizeof(*auth_type) - sizeof(auth_type->hdr));
	auth_type->auth_type = params->auth_type;

	return buf += sizeof(*auth_type);
}

static u8 *connect_insert_ssid_tlv(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_ssid *ssid = (struct connect_tlv_ssid *)buf;

	ssid->hdr = connect_pack_tlv_hdr(MORSE_CMD_CONNECT_TLV_TAG_SSID, params->ssid_len);
	memcpy(ssid->ssid, params->ssid, params->ssid_len);

	return buf += struct_size(ssid, ssid, params->ssid_len);
}

static u8 *connect_insert_sae_pwd_tlv(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_sae_pwd *sae_pwd = (struct connect_tlv_sae_pwd *)buf;

	sae_pwd->hdr = connect_pack_tlv_hdr(MORSE_CMD_CONNECT_TLV_TAG_SAE_PWD, params->sae_pwd_len);
	memcpy(sae_pwd->sae_pwd, params->sae_pwd, params->sae_pwd_len);

	return buf += struct_size(sae_pwd, sae_pwd, params->sae_pwd_len);
}

static u8 *
connect_insert_extra_assoc_ies_tlv(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_extra_assoc_ies *extra_assoc_ies =
		(struct connect_tlv_extra_assoc_ies *)buf;

	extra_assoc_ies->hdr = connect_pack_tlv_hdr(MORSE_CMD_CONNECT_TLV_TAG_EXTRA_ASSOC_IES,
						    params->extra_assoc_ies_len);
	memcpy(extra_assoc_ies->ies, params->extra_assoc_ies, params->extra_assoc_ies_len);

	return buf += struct_size(extra_assoc_ies, ies, params->extra_assoc_ies_len);
}

static u8 *connect_insert_bssid_tlv(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_bssid *bssid = (struct connect_tlv_bssid *)buf;

	bssid->hdr = connect_pack_tlv_hdr(MORSE_CMD_CONNECT_TLV_TAG_BSSID,
					  sizeof(*bssid) - sizeof(bssid->hdr));
	memcpy(bssid->bssid, params->bssid, sizeof(bssid->bssid));

	return buf += sizeof(*bssid);
}

static u8 *
connect_insert_bg_scan_period_tlv(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_bg_scan_period *bg_scan_period =
		(struct connect_tlv_bg_scan_period *)buf;
	u16 len = sizeof(*bg_scan_period) - sizeof(bg_scan_period->hdr);

	bg_scan_period->hdr = connect_pack_tlv_hdr(MORSE_CMD_CONNECT_TLV_TAG_BG_SCAN_PERIOD, len);
	bg_scan_period->bg_scan_period = cpu_to_le16((u16)params->bg_scan_period);

	return buf += sizeof(*bg_scan_period);
}

static u8 *connect_insert_4addr_mode_tlv(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_4addr_mode *tlv = (struct connect_tlv_4addr_mode *)buf;

	tlv->hdr = connect_pack_tlv_hdr(MORSE_CMD_CONNECT_TLV_TAG_4ADDR_MODE,
					sizeof(*tlv) - sizeof(tlv->hdr));
	tlv->enabled = params->use_4addr ? 1 : 0;

	return buf += sizeof(*tlv);
}

void morse_wiphy_connect_insert_tlvs(u8 *buf, const struct morse_wiphy_connect_params *params)
{
	buf = connect_insert_auth_type_tlv(buf, params);
	buf = connect_insert_ssid_tlv(buf, params);
	if (params->auth_type == MORSE_CMD_CONNECT_AUTH_TYPE_SAE)
		buf = connect_insert_sae_pwd_tlv(buf, params);
	if (params->extra_assoc_ies_len)
		buf = connect_insert_extra_assoc_ies_tlv(buf, params);
	if (params->bssid)
		buf = connect_insert_bssid_tlv(buf, params);
	if (params->bg_scan_period >= 0)
		buf = connect_insert_bg_scan_period_tlv(buf, params);
	if (params->use_4addr)
		buf = connect_insert_4addr_mode_tlv(buf, params);
}

size_t morse_wiphy_connect_get_command_size(const struct morse_wiphy_connect_params *params)
{
	struct connect_tlv_auth_type *auth_type;
	struct connect_tlv_ssid *ssid;
	struct connect_tlv_sae_pwd *sae_pwd;
	struct connect_tlv_extra_assoc_ies *extra_assoc_ies;
	struct connect_tlv_bssid *bssid;
	struct connect_tlv_bg_scan_period *bg_scan_period;
	struct connect_tlv_4addr_mode *use_4addr;
	struct morse_cmd_req_connect *req;
	size_t req_len = sizeof(*req);

	req_len += sizeof(*auth_type);
	req_len += struct_size(ssid, ssid, params->ssid_len);
	if (params->auth_type == MORSE_CMD_CONNECT_AUTH_TYPE_SAE)
		req_len += struct_size(sae_pwd, sae_pwd, params->sae_pwd_len);
	if (params->extra_assoc_ies_len)
		req_len += struct_size(extra_assoc_ies, ies, params->extra_assoc_ies_len);
	if (params->bssid)
		req_len += sizeof(*bssid);
	if (params->bg_scan_period >= 0)
		req_len += sizeof(*bg_scan_period);
	if (params->use_4addr)
		req_len += sizeof(*use_4addr);

	return req_len;
}

static int morse_wiphy_connect(struct wiphy *wiphy, struct net_device *ndev,
			       struct cfg80211_connect_params *sme)
{
	struct wireless_dev *wdev = ndev->ieee80211_ptr;
	struct morse_vif *mors_vif = netdev_priv(ndev);
	struct morse *mors = wiphy_priv(wiphy);
	struct morse_wiphy_connect_params params = {0};
	int ret;

	mutex_lock(&mors->lock);

	if (test_bit(MORSE_SME_STATE_CONNECTING, &mors_vif->sme_state)) {
		ret = -EBUSY;
		goto out;
	}

	if (sme->prev_bssid) {
		/* This is a roaming request, not an initial connection. */
		params.roam = true;
		if (WARN_ON(!test_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state))) {
			ret = -ENOENT;
			goto out;
		}
	} else {
		if (test_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state)) {
			ret = -EBUSY;
			goto out;
		}
	}

#if KERNEL_VERSION(5, 11, 0) <= MAC80211_VERSION_CODE
	switch (sme->crypto.sae_pwe) {
	case NL80211_SAE_PWE_UNSPECIFIED:
	case NL80211_SAE_PWE_HASH_TO_ELEMENT:
		break;
	case NL80211_SAE_PWE_HUNT_AND_PECK:
	case NL80211_SAE_PWE_BOTH:
	default:
		/* Only H2E (hash-to-element) is permitted in 802.11ah, hunt and peck
		 * is not supported.
		 */
		ret = -EOPNOTSUPP;
		goto out;
	}
#endif

	params.ssid = sme->ssid;
	params.ssid_len = sme->ssid_len;

	if (sme->auth_type == NL80211_AUTHTYPE_SAE) {
		/* SAE offload is mandatory for this driver: if SAE is selected then
		 * the SAE passphrase must also be given.
		 */
#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
		if (!sme->crypto.sae_pwd || !sme->crypto.sae_pwd_len) {
			ret = -EINVAL;
			goto out;
		}
		params.sae_pwd = sme->crypto.sae_pwd;
		params.sae_pwd_len = sme->crypto.sae_pwd_len;
#else
		ret = -EOPNOTSUPP;
		goto out;
#endif
	}

	switch (sme->auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
		if (sme->crypto.wpa_versions)
			params.auth_type = MORSE_CMD_CONNECT_AUTH_TYPE_OWE;
		else
			params.auth_type = MORSE_CMD_CONNECT_AUTH_TYPE_OPEN;
		break;
	case NL80211_AUTHTYPE_SAE:
		params.auth_type = MORSE_CMD_CONNECT_AUTH_TYPE_SAE;
		break;
	case NL80211_AUTHTYPE_AUTOMATIC:
		params.auth_type = MORSE_CMD_CONNECT_AUTH_TYPE_AUTOMATIC;
		break;
	default:
		ret = -EOPNOTSUPP;
		goto out;
	}

	ret = connect_build_extra_assoc_ies(&params, mors_vif, sme);
	if (ret)
		goto out;

	params.bssid = sme->bssid;
	if (params.roam)
		/* User-initiated roaming will come with the requested BSSID in bssid_hint.
		 * Strictly enforce the "hint" in this case, so that roaming does what the
		 * user expects.
		 */
		params.bssid = sme->bssid_hint;

	if (sme->bg_scan_period > U16_MAX) {
		ret = -EINVAL;
		goto out;
	}
	params.bg_scan_period = sme->bg_scan_period;

	params.use_4addr = wdev->use_4addr;

	/* TODO: obey channel, bss_select */
	/* TODO: obey cipher suite selection */
	/* TODO: obey controlled port config */
	/* TODO: apply a timeout to the connect operation on the driver side */

	ret = morse_cmd_connect(mors, &params);
	if (ret)
		goto out;

	set_bit(MORSE_SME_STATE_CONNECTING, &mors_vif->sme_state);

out:
	mutex_unlock(&mors->lock);
	kfree(params.extra_assoc_ies);

	return ret;
}

static int morse_wiphy_disconnect(struct wiphy *wiphy, struct net_device *ndev, u16 reason_code)
{
	struct morse_vif *mors_vif = netdev_priv(ndev);
	struct morse *mors = wiphy_priv(wiphy);
	int ret;

	mutex_lock(&mors->lock);

	if (!test_bit(MORSE_SME_STATE_CONNECTING, &mors_vif->sme_state) &&
	    !test_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state)) {
		ret = -EINVAL;
		goto out;
	}

	ret = morse_cmd_disconnect(mors);

	if (test_and_clear_bit(MORSE_SME_STATE_CONNECTING, &mors_vif->sme_state)) {
		cfg80211_connect_timeout(ndev, /* bssid */ NULL,
					 /* req_ie */ NULL, /* req_ie_len */ 0,
					 GFP_KERNEL
#if KERNEL_VERSION(4, 11, 0) <= MAC80211_VERSION_CODE
					 , NL80211_TIMEOUT_UNSPECIFIED
#endif
		    );
		clear_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state);
	}

out:
	mutex_unlock(&mors->lock);

	return ret;
}

static int morse_wiphy_get_channel(struct wiphy *wiphy, struct wireless_dev *wdev,
#if KERNEL_VERSION(5, 19, 2) <= MAC80211_VERSION_CODE
			     unsigned int link_id,
#endif
			     struct cfg80211_chan_def *chandef)
{
	struct morse *mors = wiphy_priv(wiphy);
	int ret;
	u32 op_chan_freq_hz;
	u8 op_bw_mhz;
	u8 pri_bw_mhz;
	u8 pri_1mhz_chan_idx;
	int op_chan_s1g, pri_chan_s1g;
	int op_chan_5g, pri_chan_5g;
	u32 op_freq_5g;
	enum nl80211_chan_width width_5g;

	mutex_lock(&mors->lock);

	if (!mors->started || mors->ps.suspended) {
		ret = -EBUSY;
		goto out;
	}

	ret = morse_cmd_get_current_channel(mors, &op_chan_freq_hz, &pri_1mhz_chan_idx, &op_bw_mhz,
					    &pri_bw_mhz);
	if (ret)
		goto out;

	/* Look up S1G channel numbers based on the channel info we received from the chip.
	 */
	op_chan_s1g = morse_dot11ah_freq_khz_bw_mhz_to_chan(HZ_TO_KHZ(op_chan_freq_hz), op_bw_mhz);
	pri_chan_s1g = morse_dot11ah_calc_prim_s1g_chan(op_bw_mhz, 1, op_chan_s1g,
							pri_1mhz_chan_idx);

	/* Map to 5GHz channel info.
	 */
	op_chan_5g = morse_dot11ah_s1g_chan_to_5g_chan(op_chan_s1g);
	pri_chan_5g = morse_dot11ah_s1g_op_chan_pri_chan_to_5g(op_chan_s1g, pri_chan_s1g);
	op_freq_5g = ieee80211_channel_to_frequency(op_chan_5g, NL80211_BAND_5GHZ);
	switch (op_bw_mhz) {
	case 1:
		width_5g = NL80211_CHAN_WIDTH_20_NOHT;
		break;
	case 2:
		width_5g = NL80211_CHAN_WIDTH_40;
		break;
	case 4:
		width_5g = NL80211_CHAN_WIDTH_80;
		break;
	case 8:
		width_5g = NL80211_CHAN_WIDTH_160;
		break;
	default:
		width_5g = WARN_ON(NL80211_CHAN_WIDTH_20_NOHT);
	}

	if (op_chan_5g == -ENOENT || pri_chan_5g == -ENOENT) {
		/* This can happen if we observe the chip with its initial channel configuration,
		 * which is not necessarily valid for the current regulatory domain.
		 */
		ret = -EINVAL;
		goto out;
	}

	chandef->chan = morse_wiphy_get_5g_channel(wiphy, pri_chan_5g);
	chandef->center_freq1 = op_freq_5g;
	chandef->width = width_5g;
	chandef->center_freq2 = 0;

out:
	mutex_unlock(&mors->lock);

	return ret;
}

static int morse_wiphy_get_station(struct wiphy *wiphy, struct net_device *ndev, const u8 *mac,
				   struct station_info *sinfo)
{
	struct morse_vif *mors_vif = netdev_priv(ndev);
	struct morse *mors = wiphy_priv(wiphy);
	int ret;

	sinfo->filled = 0;

	mutex_lock(&mors->lock);

	if (!test_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state)) {
		ret = -ENODEV;
		goto out;
	}

	ret = morse_cmd_get_connection_state(mors,
					     &sinfo->signal,
					     &sinfo->connected_time,
					     &sinfo->bss_param.dtim_period,
					     &sinfo->bss_param.beacon_interval);
	if (ret)
		goto out;

	/* Short slot time is not relevant for 802.11ah, but mac80211 reports this flag
	 * for 5GHz bands, which we are pretending to be. So report it here too for consistency.
	 */
	sinfo->bss_param.flags = BSS_PARAM_FLAGS_SHORT_SLOT_TIME;

	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_SIGNAL) |
			 BIT_ULL(NL80211_STA_INFO_CONNECTED_TIME) |
			 BIT_ULL(NL80211_STA_INFO_BSS_PARAM);

out:
	mutex_unlock(&mors->lock);

	return ret;
}

static int morse_wiphy_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	struct morse *mors = wiphy_priv(wiphy);
	int ret = 0;

	mutex_lock(&mors->lock);

	if (changed & WIPHY_PARAM_RTS_THRESHOLD) {
		/* cfg80211 uses (u32)-1 to indicate RTS/CTS disabled, whereas the chip uses 0 to
		 * indicate RTS/CTS disabled.
		 */
		if (wiphy->rts_threshold != U32_MAX) {
			if (!mors->rts_allowed) {
				ret = -EPERM;
				goto out;
			}
			MORSE_DBG(mors, "setting RTS threshold %u\n", wiphy->rts_threshold);
			ret = morse_cmd_set_rts_threshold(mors, wiphy->rts_threshold);
		} else {
			MORSE_DBG(mors, "disabling RTS\n");
			ret = morse_cmd_set_rts_threshold(mors, 0);
		}
		if (ret)
			goto out;
	}
	if (changed & WIPHY_PARAM_FRAG_THRESHOLD) {
		MORSE_DBG(mors, "setting fragmentation threshold %u\n", wiphy->frag_threshold);
		ret = morse_cmd_set_frag_threshold(mors, wiphy->frag_threshold);
		if (ret)
			goto out;
	}

out:
	mutex_unlock(&mors->lock);

	return ret;
}

static int
morse_wiphy_set_power_mgmt(struct wiphy *wiphy, struct net_device *dev, bool enabled, int timeout)
{
	/* It doesn't make sense to disable powersave offload with fullmac firmware.
	 */
	const bool enable_dynamic_ps_offload = enabled;
	struct morse *mors = wiphy_priv(wiphy);
	int ret;

	if (!morse_mac_ps_enabled(mors))
		return -EOPNOTSUPP;

	mutex_lock(&mors->lock);

	if (mors->config_ps == enabled) {
		ret = 0;
		goto out;
	}

	ret = morse_cmd_set_ps(mors, enabled, enable_dynamic_ps_offload);
	if (ret)
		goto out;

	mors->config_ps = enabled;
	ret = 0;

out:
	mutex_unlock(&mors->lock);
	return ret;
}

/* If we connected on an 8MHz channel, we may need to disable the configured RTS threshold.
 */
static void morse_wiphy_connected_update_rts(struct morse *mors, struct cfg80211_bss *bss)
{
	struct wiphy *wiphy = mors->wiphy;
	const struct ieee80211_vht_operation *vht_oper = NULL;
	const u8 *vht_oper_ie;
	bool is_8mhz = false;

	lockdep_assert_held(&mors->lock);

	/* 8MHz operating bandwidth becomes a VHT 160MHz channel width after our S1G-to-5GHz
	 * mapping has been done. So we need to check the channel width in the VHT operation IE.
	 */
	rcu_read_lock();
	vht_oper_ie = ieee80211_bss_get_ie(bss, WLAN_EID_VHT_OPERATION);
	if (vht_oper_ie && vht_oper_ie[1] >= sizeof(*vht_oper)) {
		vht_oper = (void *)(vht_oper_ie + 2);
		is_8mhz = vht_oper->chan_width == IEEE80211_VHT_CHANWIDTH_160MHZ;
	}
	rcu_read_unlock();

	mors->rts_allowed = !is_8mhz || morse_mac_is_rts_8mhz_enabled();

	if (!mors->rts_allowed) {
		mors->orig_rts_threshold = wiphy->rts_threshold;
		if (wiphy->rts_threshold != U32_MAX) {
			wiphy->rts_threshold = U32_MAX; /* (u32)-1 means RTS disabled. */
			(void)morse_cmd_set_rts_threshold(mors, 0);
			MORSE_INFO(mors, "RTS disabled for 8MHz operating bandwidth\n");
		}
	}
}

/* Handle any firmware interactions needed after connection is first established.
 * Note that this is not called for the roaming case (connected while already connected).
 */
static void morse_wiphy_connected_work(struct work_struct *work)
{
	struct morse_vif *mors_vif = container_of(work, struct morse_vif, connected_work);
	struct morse *mors = wiphy_priv(mors_vif->wdev.wiphy);
	int ret;

	mutex_lock(&mors->lock);

	if (mors_vif->connected_bss)
		morse_wiphy_connected_update_rts(mors, mors_vif->connected_bss);

	morse_wiphy_update_arp_filter(mors_vif);

	if (mors->custom_configs.enable_dhcpc_offload) {
		ret = morse_cmd_dhcpc_enable(mors, mors_vif->id);
		if (ret)
			MORSE_WARN(mors, "Failed to enable in-chip DHCP client\n");
	}

	morse_ps_enable(mors);

	mutex_unlock(&mors->lock);
}

/* Restore original RTS threshold if it was forcibly disabled before.
 */
static void morse_wiphy_disconnected_update_rts(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;

	lockdep_assert_held(&mors->lock);

	if (!mors->rts_allowed) {
		mors->rts_allowed = true;
		if (mors->orig_rts_threshold != U32_MAX) {
			wiphy->rts_threshold = mors->orig_rts_threshold;
			(void)morse_cmd_set_rts_threshold(mors, wiphy->rts_threshold);
		}
	}
}

static void morse_wiphy_disconnected_work(struct work_struct *work)
{
	struct morse_vif *mors_vif = container_of(work, struct morse_vif, disconnected_work);
	struct morse *mors = wiphy_priv(mors_vif->wdev.wiphy);
	struct wiphy *wiphy = mors_vif->wdev.wiphy;

	mutex_lock(&mors->lock);

	morse_ps_disable(mors);

	morse_wiphy_disconnected_update_rts(mors);

	if (mors_vif->connected_bss) {
		cfg80211_unlink_bss(wiphy, mors_vif->connected_bss);
		cfg80211_put_bss(wiphy, mors_vif->connected_bss);
		mors_vif->connected_bss = NULL;
	}

	mutex_unlock(&mors->lock);
}

#ifdef CONFIG_INET
static int morse_wiphy_ifa_changed(struct notifier_block *nb, unsigned long data, void *arg)
{
	struct morse_vif *mors_vif = container_of(nb, struct morse_vif, arp_filter.ifa_notifier);
	struct morse *mors = wiphy_priv(mors_vif->wdev.wiphy);
	struct in_ifaddr *ifa = arg;
	struct net_device *ndev = ifa->ifa_dev->dev;
	struct wireless_dev *wdev = ndev->ieee80211_ptr;
	struct in_device *idev;
	int c = 0;

	ASSERT_RTNL();

	if (!wdev || wdev != &mors_vif->wdev)
		return NOTIFY_DONE;

	idev = __in_dev_get_rtnl(ndev);
	if (!idev)
		return NOTIFY_DONE;

	mutex_lock(&mors->lock);

	ifa = rtnl_dereference(idev->ifa_list);
	while (ifa) {
		if (c < ARRAY_SIZE(mors_vif->arp_filter.addr_list))
			mors_vif->arp_filter.addr_list[c] = ifa->ifa_address;
		ifa = rtnl_dereference(ifa->ifa_next);
		c++;
	}

	mors_vif->arp_filter.addr_cnt = c;

	morse_wiphy_update_arp_filter(mors_vif);

	mutex_unlock(&mors->lock);

	return NOTIFY_OK;
}
#endif

static struct wireless_dev *
morse_wiphy_interface_add(struct wiphy *wiphy, const char *name, unsigned char name_assign_type,
			  enum nl80211_iftype type,
#if KERNEL_VERSION(4, 12, 0) > MAC80211_VERSION_CODE
			  u32 *flags,
#endif
			  struct vif_params *params)
{
	struct morse *mors = wiphy_priv(wiphy);
	struct net_device *ndev;
	struct morse_vif *mors_vif;
	int ret;

	ASSERT_RTNL();
#if KERNEL_VERSION(5, 12, 0) <= MAC80211_VERSION_CODE
	lockdep_assert_held(&wiphy->mtx);
#endif

	if (sta_vif || mors->monitor_mode)
		/* We only support one vif (STA or monitor), it's already been created. */
		return ERR_PTR(-EOPNOTSUPP);

	ndev = alloc_netdev(sizeof(*mors_vif), name, name_assign_type, ether_setup);
	if (!ndev)
		return ERR_PTR(-ENOMEM);

	mors_vif = netdev_priv(ndev);
	mors_vif->custom_configs = &mors->custom_configs;
	INIT_WORK(&mors_vif->connected_work, morse_wiphy_connected_work);
	INIT_WORK(&mors_vif->disconnected_work, morse_wiphy_disconnected_work);
	mors_vif->wdev.wiphy = mors->wiphy;
	mors_vif->ndev = ndev;
	mors_vif->wdev.netdev = ndev;
	mors_vif->wdev.iftype = type;
	mors_vif->wdev.use_4addr = params->use_4addr;
	ndev->ieee80211_ptr = &mors_vif->wdev;
	SET_NETDEV_DEV(ndev, wiphy_dev(mors_vif->wdev.wiphy));

	memcpy(ndev->perm_addr, mors->macaddr, ETH_ALEN);
#if KERNEL_VERSION(5, 17, 0) > LINUX_VERSION_CODE
	memcpy(ndev->dev_addr, mors->macaddr, ETH_ALEN);
#else
	dev_addr_set(ndev, mors->macaddr);
#endif

	morse_netdev_init(ndev, mors);
	netdev_set_default_ethtool_ops(ndev, &mors_ethtool_ops);

#if KERNEL_VERSION(5, 12, 0) <= MAC80211_VERSION_CODE
	ret = cfg80211_register_netdevice(ndev);
#else
	ret = register_netdevice(ndev);
#endif
	if (ret)
		goto err;

#ifdef CONFIG_INET
	mors_vif->arp_filter.ifa_notifier.notifier_call = morse_wiphy_ifa_changed;
	ret = register_inetaddr_notifier(&mors_vif->arp_filter.ifa_notifier);
	if (ret)
		goto err_unregister_netdevice;
#endif

	if (type == NL80211_IFTYPE_STATION)
		sta_vif = mors_vif;
	else if (type == NL80211_IFTYPE_MONITOR)
		mors->monitor_mode = true;

	return &mors_vif->wdev;

err_unregister_netdevice:
#if KERNEL_VERSION(5, 12, 0) <= MAC80211_VERSION_CODE
	cfg80211_unregister_netdevice(ndev);
#else
	unregister_netdevice(ndev);
#endif
err:
	free_netdev(ndev);
	return ERR_PTR(ret);
}

static int
morse_wiphy_interface_change(struct wiphy *wiphy, struct net_device *ndev,
			     enum nl80211_iftype type,
#if KERNEL_VERSION(4, 12, 0) > MAC80211_VERSION_CODE
			     u32 *flags,
#endif
			     struct vif_params *params)
{
	struct wireless_dev *wdev = ndev->ieee80211_ptr;
	struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);

	if (type != wdev->iftype)
		/* Changing vif type is not supported. */
		return -EOPNOTSUPP;

	switch (wdev->iftype) {
	case NL80211_IFTYPE_STATION:
		if (test_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state))
			/* 4-address mode must be configured before connecting. */
			return -EBUSY;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int morse_wiphy_interface_del(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct morse *mors = wiphy_priv(wiphy);
	struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);
	struct net_device *ndev = mors_vif->ndev;

	ASSERT_RTNL();

	netif_stop_queue(ndev);
	unregister_inetaddr_notifier(&mors_vif->arp_filter.ifa_notifier);
#if KERNEL_VERSION(5, 12, 0) <= MAC80211_VERSION_CODE
	cfg80211_unregister_netdevice(ndev);
#else
	unregister_netdevice(ndev);
#endif

	if (sta_vif == mors_vif)
		sta_vif = NULL;
	else if (wdev->iftype == NL80211_IFTYPE_MONITOR)
		mors->monitor_mode = false;

	return 0;
}

static int
morse_wiphy_mgmt_tx(struct wiphy *wiphy,
		    struct wireless_dev *wdev,
		    struct cfg80211_mgmt_tx_params *params,
		    u64 *cookie)
{
	struct morse *mors = morse_wiphy_to_morse(wiphy);
	struct morse_buff_skb_header *hdr;
	struct morse_skb_tx_info tx_info = { 0 };
	struct morse_skbq *mgmt_q;
	struct sk_buff *skb;
	u32 flags = 0;
	int ret;

	if (params->dont_wait_for_ack) {
		flags |= MORSE_TX_STATUS_FLAGS_NO_REPORT;
		tx_info.flags = cpu_to_le32(flags);
	}

	mgmt_q = mors->cfg->ops->skbq_mgmt_tc_q(mors);
	skb = morse_skbq_alloc_skb(mgmt_q, params->len);
	if (!skb) {
		ret = -ENOMEM;
		return ret;
	}
	memcpy(skb->data, params->buf, params->len);

	ret = morse_skbq_skb_tx(mgmt_q, &skb, &tx_info, MORSE_SKB_CHAN_MGMT);
	if (ret < 0)
		goto tx_err;

	hdr = (struct morse_buff_skb_header *)skb->data;
	*cookie = le32_to_cpu(hdr->tx_info.pkt_id);

	return 0;

tx_err:
	morse_mac_skb_free(mors, skb);
	return ret;
}

#if KERNEL_VERSION(5, 8, 0) <= MAC80211_VERSION_CODE
static void
morse_wiphy_update_mgmt_frame_registrations(struct wiphy *wiphy,
					    struct wireless_dev *wdev,
					    struct mgmt_frame_regs *upd)
{
	struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);

	bool new_stype = !(mors_vif->stypes & upd->interface_stypes);

	if (new_stype)
		mors_vif->stypes |= upd->interface_stypes;
}
#else
static void
morse_wiphy_mgmt_frame_register(struct wiphy *wiphy, struct wireless_dev *wdev,
				u16 frame_type, bool reg)
{
	struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);

	u32 stype = (frame_type & IEEE80211_FCTL_STYPE) >> 4;

	if (reg)
		mors_vif->stypes |= BIT(stype);
	else
		mors_vif->stypes &= ~BIT(stype);
}
#endif

static struct cfg80211_ops morse_wiphy_cfg80211_ops = {
	.scan = morse_wiphy_scan,
	.abort_scan = morse_wiphy_abort_scan,
	.connect = morse_wiphy_connect,
	.disconnect = morse_wiphy_disconnect,
	.get_channel = morse_wiphy_get_channel,
	.get_station = morse_wiphy_get_station,
	.set_wiphy_params = morse_wiphy_set_wiphy_params,
	.set_power_mgmt = morse_wiphy_set_power_mgmt,
	.add_virtual_intf = morse_wiphy_interface_add,
	.change_virtual_intf = morse_wiphy_interface_change,
	.del_virtual_intf = morse_wiphy_interface_del,
	.mgmt_tx = morse_wiphy_mgmt_tx,
#if KERNEL_VERSION(5, 8, 0) <= MAC80211_VERSION_CODE
	.update_mgmt_frame_registrations = morse_wiphy_update_mgmt_frame_registrations,
#else
	.mgmt_frame_register = morse_wiphy_mgmt_frame_register,
#endif
	/*
	 * TBD
	 * Place holder of what we need to do. Do not remove
	 */
	/*
	 * .join_ibss = mors_wiphy_join_ibss,
	 * .leave_ibss = mors_wiphy_leave_ibss,
	 * .dump_station = mors_wiphy_dump_station,
	 * .set_tx_power = mors_wiphy_set_tx_power,
	 * .get_tx_power = mors_wiphy_get_tx_power,
	 * .add_key = mors_wiphy_add_key,
	 * .del_key = mors_wiphy_del_key,
	 * .get_key = mors_wiphy_get_key,
	 * .set_default_key = mors_wiphy_config_default_key,
	 * .set_default_mgmt_key = mors_wiphy_config_default_mgmt_key,
	 * .suspend = mors_wiphy_suspend,
	 * .resume = mors_wiphy_resume,
	 * .set_pmksa = mors_wiphy_set_pmksa,
	 * .del_pmksa = mors_wiphy_del_pmksa,
	 * .flush_pmksa = mors_wiphy_flush_pmksa,
	 * .start_ap = mors_wiphy_start_ap,
	 * .stop_ap = mors_wiphy_stop_ap,
	 * .change_beacon = mors_wiphy_change_beacon,
	 * .del_station = mors_wiphy_del_station,
	 * .change_station = mors_wiphy_change_station,
	 * .sched_scan_start = mors_wiphy_sched_scan_start,
	 * .sched_scan_stop = mors_wiphy_sched_scan_stop,
	 * .remain_on_channel = mors_p2p_remain_on_channel,
	 * .cancel_remain_on_channel = mors_wiphy_cancel_remain_on_channel,
	 * .start_p2p_device = mors_p2p_start_device,
	 * .stop_p2p_device = mors_p2p_stop_device,
	 * .crit_proto_start = mors_wiphy_crit_proto_start,
	 * .crit_proto_stop = mors_wiphy_crit_proto_stop,
	 * .tdls_oper = mors_wiphy_tdls_oper,
	 * .update_connect_params = mors_wiphy_update_conn_params,
	 * .set_pmk = mors_wiphy_set_pmk,
	 * .del_pmk = mors_wiphy_del_pmk,
	 */
};

/**
 * morse_wiphy_create() -  Create wiphy device
 * @priv_size: extra size per structure to allocate
 * @dev: Bus device structure
 *
 * Allocate memory for wiphy device and do basic initialisation.
 *
 * Return: morse device struct, else NULL.
 */
struct morse *morse_wiphy_create(size_t priv_size, struct device *dev)
{
	struct wiphy *wiphy;
	struct morse *mors;

	wiphy = wiphy_new(&morse_wiphy_cfg80211_ops, sizeof(*mors) + priv_size);
	if (!wiphy)
		return NULL;

	wiphy->max_scan_ssids = SCAN_MAX_SSIDS;
	wiphy->max_scan_ie_len = MORSE_CMD_SCAN_EXTRA_IES_MAX_LEN;
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wiphy->bands[NL80211_BAND_5GHZ] = &mors_band_5ghz;
	wiphy->bands[NL80211_BAND_2GHZ] = NULL;
	wiphy->bands[NL80211_BAND_60GHZ] = NULL;
	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_MONITOR);
	wiphy->mgmt_stypes = morse_wiphy_txrx_stypes;
	wiphy->flags |= WIPHY_FLAG_4ADDR_STATION;
	wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
	set_wiphy_dev(wiphy, dev);

	wiphy->privid = morse_wiphy_privid;
	mors = wiphy_priv(wiphy);
	mors->wiphy = wiphy;

	return mors;
}

/**
 * morse_wiphy_init() -  Init wiphy device
 * @morse: morse device instance
 *
 * Initilise wiphy device
 *
 * Return: 0 success, else error.
 */
int morse_wiphy_init(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;
	/* TODO: ask the chip instead of hardcoding the list here. */
	static const u32 morse_wiphy_cipher_suites[] = {
		WLAN_CIPHER_SUITE_CCMP,
		WLAN_CIPHER_SUITE_AES_CMAC,
	};

	memcpy(wiphy->perm_addr, mors->macaddr, ETH_ALEN);

#if KERNEL_VERSION(5, 3, 0) <= MAC80211_VERSION_CODE
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_SAE_OFFLOAD);
#endif
#if KERNEL_VERSION(6, 7, 0) <= MAC80211_VERSION_CODE
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_OWE_OFFLOAD);
#endif

	wiphy->cipher_suites = morse_wiphy_cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(morse_wiphy_cipher_suites);

	mors->extra_tx_offset = EXTRA_TX_OFFSET;
	mors->rts_allowed = true;
	mors->wiphy_wq = create_singlethread_workqueue(wiphy_name(wiphy));
	INIT_WORK(&mors->scan_done_work, morse_wiphy_scan_done_work);

	return 0;
}

/**
 * morse_wiphy_register() -  Register wiphy device
 * @morse: morse device instance
 *
 * Register wiphy device
 *
 * Return: 0 success, else error.
 */
int morse_wiphy_register(struct morse *mors)
{
	int ret;
	struct wiphy *wiphy = mors->wiphy;
	struct device *dev = wiphy_dev(wiphy);
	struct wireless_dev *wdev;
	struct vif_params vif_params = {0};
#if KERNEL_VERSION(4, 12, 0) > MAC80211_VERSION_CODE
	u32 flags = 0;
#endif

	ret = wiphy_register(wiphy);
	if (ret < 0)
		dev_info(dev, "wiphy_register fail\r\n");
	else
		dev_info(dev, "wiphy_register success %d\r\n", ret);

	rtnl_lock();
#if KERNEL_VERSION(5, 12, 0) <= MAC80211_VERSION_CODE
	wiphy_lock(wiphy);
#endif

	/* Add an initial station interface */
	wdev = morse_wiphy_interface_add(wiphy, "wlan%d", NET_NAME_ENUM, NL80211_IFTYPE_STATION,
#if KERNEL_VERSION(4, 12, 0) > MAC80211_VERSION_CODE
					 &flags,
#endif
					 &vif_params);
	if (IS_ERR(wdev))
		ret = PTR_ERR(wdev);

#if KERNEL_VERSION(5, 12, 0) <= MAC80211_VERSION_CODE
	wiphy_unlock(wiphy);
#endif
	rtnl_unlock();

	return ret;
}

void morse_wiphy_stop(struct morse *mors)
{
	struct morse_vif *mors_vif = sta_vif;

	if (mors_vif && mors_vif->ndev)
		netif_stop_queue(mors_vif->ndev);
}

void morse_wiphy_cleanup(struct morse *mors)
{
	const bool disconnect_locally_generated = true;
	struct morse_vif *mors_vif = sta_vif;
	struct net_device *ndev = mors_vif ? mors_vif->ndev : NULL;

	lockdep_assert_held(&mors->lock);

	if (ndev)
		netif_carrier_off(ndev);

	if (mors_vif && test_and_clear_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state)) {
		cfg80211_disconnected(ndev, WLAN_REASON_UNSPECIFIED,
				      NULL, 0, disconnect_locally_generated, GFP_KERNEL);
		morse_ps_disable(mors);
	}

	if (mors_vif && test_and_clear_bit(MORSE_SME_STATE_CONNECTING, &mors_vif->sme_state))
		cfg80211_connect_timeout(ndev, NULL, NULL, 0, GFP_KERNEL
#if KERNEL_VERSION(4, 11, 0) <= MAC80211_VERSION_CODE
					 , NL80211_TIMEOUT_UNSPECIFIED
#endif
					);

	if (test_and_clear_bit(MORSE_SCAN_STATE_SCANNING, &mors->scan_state)) {
		struct cfg80211_scan_info info = {
			.aborted = true,
		};
		cfg80211_scan_done(mors->scan_req, &info);
		mors->scan_req = NULL;
	}
	clear_bit(MORSE_SCAN_STATE_ABORTED, &mors->scan_state);

	mors->started = false;
}

void morse_wiphy_restarted(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;
	u32 rts_threshold;
	int ret;

	lockdep_assert_held(&mors->lock);

	mors->started = true;

	ret = morse_cmd_set_country(mors, mors->country);
	if (ret)
		MORSE_ERR(mors, "error setting country after restart: %d\n", ret);

	rts_threshold = wiphy->rts_threshold != U32_MAX ? wiphy->rts_threshold : 0;
	ret = morse_cmd_set_rts_threshold(mors, rts_threshold);
	if (ret)
		MORSE_ERR(mors, "error setting RTS threshold after restart: %d\n", ret);

	ret = morse_cmd_set_frag_threshold(mors, wiphy->frag_threshold);
	if (ret)
		MORSE_ERR(mors, "error setting fragmentation threshold after restart: %d\n", ret);

	ret = morse_cmd_set_rate_control(mors);
	if (ret)
		MORSE_ERR(mors, "error setting rate control after restart: %d\n", ret);

	ret = morse_wiphy_fixed_rate(mors);
	if (ret)
		MORSE_ERR(mors, "error setting fixed transmission rate after restart: %d\n", ret);

	if (sta_vif) {
		struct morse_vif *mors_vif = sta_vif;
		struct net_device *ndev = mors_vif->ndev;

		/* Add back the STA vif, originally added in morse_ndev_open(). */
		ret = morse_cmd_add_if(mors, &mors_vif->id, ndev->dev_addr, NL80211_IFTYPE_STATION);
		if (ret)
			MORSE_ERR(mors,
				  "error adding station interface to chip after restart: %d\n",
				  ret);

		netif_wake_queue(mors_vif->ndev);
	}

	if (mors->monitor_mode) {
		struct morse_vif *mors_vif = &mors->mon_if;

		/* Add back the monitor vif, originally added in morse_ndev_open(). */
		ret = morse_cmd_add_if(mors, &mors_vif->id, mors->macaddr, NL80211_IFTYPE_MONITOR);
		if (ret)
			MORSE_ERR(mors,
				  "error adding monitor interface to chip after restart: %d\n",
				  ret);
	}
}

/**
 * morse_wiphy_deinit() -  Deinit wiphy device
 * @morse: morse device instance
 *
 * Deinitilise wiphy device
 *
 * Return: None.
 */
void morse_wiphy_deinit(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;
	struct wireless_dev *wdev;
	LIST_HEAD(unreg_list);

	mutex_lock(&mors->lock);
	morse_wiphy_cleanup(mors);
	mutex_unlock(&mors->lock);

	rtnl_lock();
	list_for_each_entry(wdev, &wiphy->wdev_list, list) {
		struct morse_vif *mors_vif = morse_wdev_to_morse_vif(wdev);

		unregister_inetaddr_notifier(&mors_vif->arp_filter.ifa_notifier);
		unregister_netdevice_queue(mors_vif->ndev, &unreg_list);
	}
	unregister_netdevice_many(&unreg_list);
	rtnl_unlock();

	if (wiphy->registered)
		wiphy_unregister(wiphy);

	flush_workqueue(mors->wiphy_wq);
	destroy_workqueue(mors->wiphy_wq);
	mors->wiphy_wq = NULL;
}

/**
 * morse_wiphy_destroy() -  Destroy wiphy device
 * @morse: morse device instance
 *
 * Free wiphy device
 *
 * Return: None.
 */
void morse_wiphy_destroy(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;

	WARN(!wiphy, "%s called with null wiphy", __func__);
	if (!wiphy)
		return;

	wiphy_free(wiphy);
}

void morse_wiphy_rx(struct morse *mors, struct sk_buff *skb)
{
	struct morse_vif *mors_vif = sta_vif;
	struct net_device *ndev = mors_vif->ndev;

	skb->dev = ndev;
	skb->protocol = eth_type_trans(skb, ndev);
	ndev->stats.rx_packets++;
	ndev->stats.rx_bytes += skb->len;
#if KERNEL_VERSION(5, 18, 0) <= LINUX_VERSION_CODE
	netif_rx(skb);
#else
	netif_rx_ni(skb);
#endif
}

void
morse_wiphy_rx_mgmt(struct morse *mors,
		    struct sk_buff *skb,
		    struct morse_skb_rx_status *hdr_rx_status)
{
	struct morse_vif *mors_vif = morse_wiphy_get_sta_vif(mors);
	struct wireless_dev *wdev = &mors_vif->wdev;

	cfg80211_rx_mgmt(wdev, 0, le16_to_cpu(hdr_rx_status->rssi), skb->data, skb->len, 0);
	morse_mac_skb_free(mors, skb);
}

/* Caller must kfree() the returned value on success. */
static u8 *morse_wiphy_translate_prob_resp_ies(u8 *ies_s1g, size_t ies_s1g_len,
					       int *length_11n_out)
{
	struct dot11ah_ies_mask *ies_mask = NULL;
	u8 *ies_11n = NULL;
	int length_11n;
	int ret;

	ies_mask = morse_dot11ah_ies_mask_alloc();
	if (!ies_mask) {
		ret = -ENOMEM;
		goto err;
	}

	ret = morse_dot11ah_parse_ies(ies_s1g, ies_s1g_len, ies_mask);
	if (ret)
		goto err;

	length_11n = morse_dot11ah_s1g_to_probe_resp_ies_size(ies_mask);
	ies_11n = kzalloc(length_11n, GFP_KERNEL);
	if (!ies_11n) {
		ret = -ENOMEM;
		goto err;
	}
	morse_dot11ah_s1g_to_probe_resp_ies(ies_11n, length_11n, ies_mask);

	kfree(ies_mask);
	*length_11n_out = length_11n;
	return ies_11n;

err:
	kfree(ies_mask);
	kfree(ies_11n);
	return ERR_PTR(ret);
}

int morse_wiphy_scan_result(struct morse *mors, struct morse_cmd_evt_scan_result *result)
{
	struct wiphy *wiphy = mors->wiphy;
	const struct morse_dot11ah_channel *chan_s1g;
	enum cfg80211_bss_frame_type ftype;
	struct ieee80211_channel *chan_5g;
	struct cfg80211_bss *bss;
	s16 signal_from_chip;
	u8 *ies_11n = NULL;
	int ies_11n_len = 0;
	s32 signal;
	int ret;

	chan_s1g = morse_dot11ah_s1g_freq_to_s1g(le32_to_cpu(result->channel_freq_hz),
						 result->bw_mhz);
	if (!chan_s1g) {
		MORSE_ERR(mors, "scan result channel is invalid: freq %uHz, bw %uMhz\n",
			  le32_to_cpu(result->channel_freq_hz), result->bw_mhz);
		return -EINVAL;
	}

	chan_5g = morse_wiphy_dot11ah_channel_to_5g(wiphy, chan_s1g);

	switch (result->frame_type) {
	case MORSE_CMD_SCAN_RESULT_FRAME_BEACON:
		ftype = CFG80211_BSS_FTYPE_BEACON;
		break;
	case MORSE_CMD_SCAN_RESULT_FRAME_PROBE_RESPONSE:
		ftype = CFG80211_BSS_FTYPE_PRESP;
		break;
	case MORSE_CMD_SCAN_RESULT_FRAME_UNKNOWN:
	default:
		ftype = CFG80211_BSS_FTYPE_UNKNOWN;
	}

	/* The chip gives us a signal indication in dBm as int16_t. */
	signal_from_chip = (s16)le16_to_cpu(result->rssi);

	/* cfg80211 wants the signal in mBm, even though we declare ourselves as SIGNAL_DBM. */
	signal = DBM_TO_MBM((s32)signal_from_chip);

	ies_11n = morse_wiphy_translate_prob_resp_ies(result->ies,
						      le16_to_cpu(result->ies_len), &ies_11n_len);
	if (IS_ERR(ies_11n)) {
		MORSE_INFO_RATELIMITED(mors, "invalid probe response IEs from BSS %pM\n",
				       result->bssid);
		ies_11n = NULL;
	}

	bss = cfg80211_inform_bss(wiphy, chan_5g, ftype, result->bssid,
				  le64_to_cpu(result->tsf), le16_to_cpu(result->capability_info),
				  le16_to_cpu(result->beacon_interval),
				  ies_11n, ies_11n_len, signal, GFP_KERNEL);
	if (bss) {
		MORSE_DBG(mors, "scan added BSS %pM\n", result->bssid);
		cfg80211_put_bss(wiphy, bss);
		ret = 0;
	} else {
		MORSE_ERR(mors, "%s failed to add BSS\n", __func__);
		ret = -ENOMEM;
	}

	kfree(ies_11n);
	return ret;
}

void morse_wiphy_scan_done(struct morse *mors, bool aborted)
{
	if (!mors->started)
		return;

	if (aborted)
		set_bit(MORSE_SCAN_STATE_ABORTED, &mors->scan_state);
	queue_work(mors->wiphy_wq, &mors->scan_done_work);
}

static void morse_wiphy_process_assoc_resp_ies(struct morse_vif *mors_vif,
					       const u8 *assoc_resp_ies,
					       const u16 assoc_resp_ies_len)
{
	struct dot11ah_ies_mask *ies_mask = NULL;
	struct dot11_morse_vendor_caps_ops_ie *ie = NULL;

	ies_mask = morse_dot11ah_ies_to_ies_mask(assoc_resp_ies, assoc_resp_ies_len);
	if (!ies_mask)
		return;

	ie = morse_vendor_find_vendor_ie(ies_mask);
	if (!ie)
		goto out;

	morse_vendor_fill_sta_vendor_info(mors_vif, ie);

out:
	morse_dot11ah_ies_mask_free(ies_mask);
}

void morse_wiphy_connected(struct morse *mors, const u8 *bssid,
			   const u8 *assoc_resp_ies, u16 assoc_resp_ies_len)
{
	struct morse_vif *mors_vif = sta_vif;
	struct net_device *ndev = mors_vif->ndev;
	struct wiphy *wiphy = mors->wiphy;
	struct cfg80211_bss *bss;
	bool roamed = false;

	if (test_and_set_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state)) {
		roamed = true;
		MORSE_INFO(mors, "roamed to BSS %pM\n", bssid);
	} else {
		MORSE_INFO(mors, "connected to BSS %pM\n", bssid);
	}
	WARN_ON(!test_and_clear_bit(MORSE_SME_STATE_CONNECTING, &mors_vif->sme_state));

	bss = cfg80211_get_bss(wiphy, NULL, bssid, NULL, 0,
			       IEEE80211_BSS_TYPE_ANY, IEEE80211_PRIVACY_ANY);
	/* The firmware should have informed us about the BSS via a "scan result" event
	 * before this "connected" event was received, so it should exist in the BSS cache.
	 */
	WARN_ON(!bss);
	if (mors_vif->connected_bss)
		cfg80211_put_bss(wiphy, mors_vif->connected_bss);
	mors_vif->connected_bss = bss;

	netif_carrier_on(ndev);

	cfg80211_ref_bss(wiphy, bss);
	if (!roamed) {
#if KERNEL_VERSION(6, 0, 0) <= MAC80211_VERSION_CODE
		struct cfg80211_connect_resp_params params = {
			.status = WLAN_STATUS_SUCCESS,
			.links[0].bssid = bssid,
			.links[0].bss = bss,
			.resp_ie = assoc_resp_ies,
			.resp_ie_len = assoc_resp_ies_len,
		};
		cfg80211_connect_done(ndev, &params, GFP_KERNEL);
#elif KERNEL_VERSION(4, 12, 0) <= MAC80211_VERSION_CODE
		struct cfg80211_connect_resp_params params = {
			.status = WLAN_STATUS_SUCCESS,
			.bssid = bssid,
			.bss = bss,
			.resp_ie = assoc_resp_ies,
			.resp_ie_len = assoc_resp_ies_len,
		};
		cfg80211_connect_done(ndev, &params, GFP_KERNEL);
#elif KERNEL_VERSION(4, 11, 0) <= MAC80211_VERSION_CODE
		cfg80211_connect_bss(ndev, bssid, bss, NULL, 0, assoc_resp_ies, assoc_resp_ies_len,
				     WLAN_STATUS_SUCCESS, GFP_KERNEL, 0);
#else
		cfg80211_connect_bss(ndev, bssid, bss, NULL, 0, assoc_resp_ies, assoc_resp_ies_len,
				     WLAN_STATUS_SUCCESS, GFP_KERNEL);
#endif
	} else {
#if KERNEL_VERSION(6, 0, 0) <= MAC80211_VERSION_CODE
		struct cfg80211_roam_info roam_info = {
			.links[0].bssid = bssid,
			.links[0].bss = bss,
			.resp_ie = assoc_resp_ies,
			.resp_ie_len = assoc_resp_ies_len,
		};
		cfg80211_roamed(ndev, &roam_info, GFP_KERNEL);
#elif KERNEL_VERSION(4, 12, 0) <= MAC80211_VERSION_CODE
		struct cfg80211_roam_info roam_info = {
			.bss = bss,
			.bssid = bssid,
			.resp_ie = assoc_resp_ies,
			.resp_ie_len = assoc_resp_ies_len,
		};
		cfg80211_roamed(ndev, &roam_info, GFP_KERNEL);
#else
		cfg80211_roamed_bss(ndev, bss, NULL, 0, assoc_resp_ies, assoc_resp_ies_len,
				    GFP_KERNEL);
#endif
	}

	if (assoc_resp_ies_len)
		morse_wiphy_process_assoc_resp_ies(mors_vif, assoc_resp_ies, assoc_resp_ies_len);

/* TODO: this should only be called if we connected with SAE (or OWE?) */
#if (KERNEL_VERSION(6, 2, 0) <= MAC80211_VERSION_CODE) || \
	(defined(CONFIG_ANDROID) && (KERNEL_VERSION(6, 1, 25) <= LINUX_VERSION_CODE))
	cfg80211_port_authorized(ndev, bssid, NULL, 0, GFP_KERNEL);
#elif KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE
	cfg80211_port_authorized(ndev, bssid, GFP_KERNEL);
#endif

	if (!roamed)
		queue_work(mors->wiphy_wq, &mors_vif->connected_work);
}

void morse_wiphy_disconnected(struct morse *mors)
{
	struct morse_vif *mors_vif = sta_vif;
	struct net_device *ndev = mors_vif->ndev;

	if (!mors->started)
		return;

	if (test_bit(MORSE_SME_STATE_CONNECTING, &mors_vif->sme_state))
		/* Roaming is in progress, so the chip is already trying to connect
		 * to the new BSS.
		 */
		return;

	if (WARN_ON(!test_and_clear_bit(MORSE_SME_STATE_CONNECTED, &mors_vif->sme_state)))
		return;

	MORSE_INFO(mors, "disconnected\n");

	netif_carrier_off(ndev);

	/* TODO: get reason, deassoc/deauth IEs */
	cfg80211_disconnected(ndev, WLAN_REASON_UNSPECIFIED,
			      /* ie */ NULL, /* ie_len */ 0,
			      /* locally_generated */ false, GFP_KERNEL);
	mors_vif->bss_vendor_info.valid = false;

	queue_work(mors->wiphy_wq, &mors_vif->disconnected_work);
}

int morse_wiphy_traffic_control(struct morse *mors, bool pause_data_traffic, int sources)
{
	int ret = -1;
	unsigned long *event_flags = &mors->chip_if->event_flags;
	bool sources_includes_twt = (sources & MORSE_CMD_UMAC_TRAFFIC_CONTROL_SOURCE_TWT);

	if (sources_includes_twt) {
		/* TWT not supported.. LMAC should not be signalling traffic control */
		WARN_ONCE(1, "TWT not supported on interface\n");
		goto exit;
	}

	if (pause_data_traffic) {
		set_bit(MORSE_DATA_TRAFFIC_PAUSE_PEND, event_flags);
		queue_work(mors->chip_wq, &mors->chip_if_work);
		/* TODO(SW-13279): pause watchdog here if sources_includes_twt */
	} else {
		set_bit(MORSE_DATA_TRAFFIC_RESUME_PEND, event_flags);
		queue_work(mors->chip_wq, &mors->chip_if_work);
		/* TODO(SW-13279): resume watchdog here if sources_includes_twt */
	}

	ret = 0;
exit:
	return ret;
}
