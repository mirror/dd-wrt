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
#include <linux/bitfield.h>

#include "dot11ah.h"
#include "tim.h"
#include "debug.h"
#include "../morse.h"
#include "../mesh.h"
#include "../utils.h"
#include "../pv1.h"

#define HZ_TO_KHZ(x) ((x) / 1000)

/*
 * APIs used to insert various S1G information elements (used only in this file)
 */

static void morse_dot11ah_insert_s1g_aid_request(struct dot11ah_ies_mask *ies_mask)
{
	/* For now we won't request anything */
	u8 s1g_aid_request[] = {
		0x00
	};

	morse_dot11ah_insert_element(ies_mask, WLAN_EID_AID_REQUEST,
				     s1g_aid_request, sizeof(s1g_aid_request));
}

static void morse_dot11ah_insert_s1g_aid_response(struct dot11ah_ies_mask *ies_mask, __le16 aid)
{
	struct ie_element *element;
	u8 s1g_aid_response[] = {
		0x00, 0x00, 0x00, 0x00, 0x00
	};

	element = morse_dot11_ies_create_ie_element(ies_mask, WLAN_EID_AID_RESPONSE,
		sizeof(s1g_aid_response), true, true);

	if (!element)
		return;

	/* The 1st and 2nd octet are the AID */
	*element->ptr = le16_to_cpu(aid) & 0xFF;
	*(element->ptr + 1) = (le16_to_cpu(aid) >> 8) & 0xFF;
}

static int morse_dot11ah_insert_s1g_compatibility(struct dot11ah_ies_mask *ies_mask,
						u16 beacon_int,
						u16 capab_info,
						u32 tsf_completion)
{
	const struct dot11ah_s1g_bcn_compat_ie s1g_compatibility = {
		.beacon_interval = cpu_to_le16(beacon_int),
		.information = cpu_to_le16(capab_info),
		.tsf_completion = cpu_to_le32(tsf_completion),
	};

	morse_dot11ah_insert_element(ies_mask,
				     WLAN_EID_S1G_BCN_COMPAT,
				     (u8 *)&s1g_compatibility,
				     sizeof(s1g_compatibility));

	return sizeof(s1g_compatibility) + 2;
}

/** Inserts the S1G capability element */
static int morse_dot11ah_insert_s1g_capability(struct ieee80211_vif *vif,
					const struct ieee80211_ht_cap *ht_cap,
					struct dot11ah_ies_mask *ies_mask,
					u8 type)
{
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;

	morse_dot11ah_insert_element(ies_mask,
				     WLAN_EID_S1G_CAPABILITIES,
				     (u8 *)&mors_vif->s1g_cap_ie,
				     sizeof(mors_vif->s1g_cap_ie));

	return sizeof(mors_vif->s1g_cap_ie) + 2;
}

int morse_dot11ah_insert_pv1_hc_ie(struct ieee80211_vif *vif,
					struct dot11ah_ies_mask *ies_mask, bool is_response)
{
	int header_compression_size;
	u8 header_compression_buf[HC_IE_SIZE_MAX] = {0};
	struct dot11ah_pv1_header_compression *header_compression =
			(struct dot11ah_pv1_header_compression *)header_compression_buf;
	u8 *tmp = header_compression->variable;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse_pv1_hc_request *hc = is_response ?
			&mors_vif->pv1.rx_request : &mors_vif->pv1.tx_request;

	if (!ies_mask || !mors_vif || !mors_vif->enable_pv1)
		return 0;

	header_compression->header_compression_control =
			(is_response ? DOT11AH_PV1_HEADER_COMPRESSION_REQ_RESPONSE : 0) |
			DOT11AH_PV1_HEADER_COMPRESSION_TYPE3_SUPPORT;

	/* As per section 9.4.2.212 in Draft P802.11REVme_D4.0 Store A3/A4
	 * subfield is set
	 * In request  - when indended receiver has to store A3/A4
	 * In response - when the receiver confirm storage of A3/A4
	 *
	 * A3/A4 field is present if Request/Response subfield is 0 (Request) and the
	 * Store A3/A4 subfield is 1
	 */
	if (hc->a1_a3_differ) {
		header_compression->header_compression_control |=
				DOT11AH_PV1_HEADER_COMPRESSION_STORE_A3;

		if (!is_response) {
			memcpy(tmp, hc->header_compression_a3, ETH_ALEN);
			memcpy(hc->stored_a3, hc->header_compression_a3, ETH_ALEN);
			tmp += sizeof(hc->header_compression_a3);
		}
	}

	if (hc->a2_a4_differ) {
		header_compression->header_compression_control |=
				DOT11AH_PV1_HEADER_COMPRESSION_STORE_A4;

		if (!is_response) {
			memcpy(tmp, hc->header_compression_a4, ETH_ALEN);
			memcpy(hc->stored_a4, hc->header_compression_a4, ETH_ALEN);
			tmp += sizeof(hc->header_compression_a4);
		}
	}

	header_compression_size = tmp - header_compression_buf;
	morse_dot11ah_insert_element(ies_mask,
				     WLAN_EID_HEADER_COMPRESSION,
				     header_compression_buf,
				     header_compression_size);

	return header_compression_size + 2;
}
EXPORT_SYMBOL(morse_dot11ah_insert_pv1_hc_ie);

static int morse_dot11ah_insert_s1g_short_beacon_interval(struct dot11ah_ies_mask *ies_mask,
							u16 beacon_int)
{
	struct dot11ah_short_beacon_ie short_beacon_int = {
		.short_beacon_int = cpu_to_le16(beacon_int)
	};

	morse_dot11ah_insert_element(ies_mask,
				     WLAN_EID_S1G_SHORT_BCN_INTERVAL,
				     (u8 *)&short_beacon_int,
				     sizeof(short_beacon_int));

	return sizeof(short_beacon_int) + 2;
}

static int morse_dot11ah_insert_s1g_operation(struct dot11ah_ies_mask *ies_mask,
					struct s1g_operation_parameters *params)
{
	u8 op_bw_mhz = 2;
	u8 pri_bw_mhz = 2;
	u8 chan_centre_freq_num = 38;
	u8 pri_1mhz_chan_idx = 0;
	u8 pri_1mhz_chan_location = 0;
	u8 s1g_operating_class = 0;
	/** Basic S1G-MCS and NSS Set */
	u8 s1g_mcs_and_nss_set[] = {0xCC, 0xC4};

	u8 s1g_operation[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		s1g_mcs_and_nss_set[1],
		s1g_mcs_and_nss_set[0]
	};

	if (params) {
		op_bw_mhz = params->op_bw_mhz;

		pri_bw_mhz = params->pri_bw_mhz;

		pri_1mhz_chan_idx = params->pri_1mhz_chan_idx;

		pri_1mhz_chan_location =
			(pri_1mhz_chan_idx % 2);

		chan_centre_freq_num = params->chan_centre_freq_num;

		s1g_operating_class = params->s1g_operating_class;
	}

	s1g_operation[0] =
		IEEE80211AH_S1G_OPERATION_SET_PRIM_CHAN_BW(pri_bw_mhz) |
		IEEE80211AH_S1G_OPERATION_SET_OP_CHAN_BW(op_bw_mhz) |
		IEEE80211AH_S1G_OPERATION_SET_PRIM_CHAN_LOC(pri_1mhz_chan_location);

	/* TODO: Set this to the actual operating class E.g. 71 for AU 8MHz Channel*/
	/* Operating Class subfield */
	s1g_operation[1] = s1g_operating_class;

	/* Primary Channel Number subfield */
	s1g_operation[2] = morse_dot11ah_calc_prim_s1g_chan(op_bw_mhz, pri_bw_mhz,
								       chan_centre_freq_num,
								       pri_1mhz_chan_idx);

	/* Channel Centre Frequency subfield */
	s1g_operation[3] = chan_centre_freq_num;

	morse_dot11ah_insert_element(ies_mask,
				     WLAN_EID_S1G_OPERATION,
				     (u8 *)&s1g_operation,
				     sizeof(s1g_operation));

	return sizeof(s1g_operation) + 2;
}

static int morse_dot11ah_insert_country_ie(struct dot11ah_ies_mask *ies_mask,
					   struct s1g_operation_parameters *params)
{
	struct dot11ah_country_ie country_ie;
	const struct morse_regdomain *regdom;

	const char *region = morse_dot11ah_get_region_str();

	memset(&country_ie, 0, sizeof(country_ie));

	regdom = morse_reg_alpha_lookup(region);
	if (!regdom)
		return 0;

	if (params)
		morse_mac_set_country_info_from_regdom(regdom, params, &country_ie);

	morse_dot11ah_insert_element(ies_mask,
				     WLAN_EID_COUNTRY,
				     (u8 *)&country_ie,
				     sizeof(country_ie));

	return sizeof(country_ie) + 2;
}

/* APIs to convert the 11n frames coming from Linux to S1G ready to transmit */
static u16 morse_dot11ah_listen_interval_to_s1g(u16 li)
{
	u16 s1g_li;

	/* if multiple of 10, directly use 10 scale */
	if (li > 0x3FFF || li % 10 == 0) {
		u16 usf =
			IEEE80211_LI_USF_10 << IEEE80211_S1G_LI_USF_SHIFT;

		s1g_li = li / 10;
		s1g_li |= usf;
	} else {
		s1g_li = li;
	}

	return s1g_li;
}

static void morse_dot11ah_assoc_req_to_s1g(struct ieee80211_vif *vif,
					struct sk_buff *skb,
					struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *assoc_req = (struct ieee80211_mgmt *)skb->data;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	const struct ieee80211_ht_cap *ht_cap;
	u16 li = ieee80211_is_assoc_req(assoc_req->frame_control) ?
		le16_to_cpu(assoc_req->u.assoc_req.listen_interval) :
		le16_to_cpu(assoc_req->u.reassoc_req.listen_interval);
	u16 s1g_li;

	s1g_li = morse_dot11ah_listen_interval_to_s1g(li);

	if (ieee80211_is_assoc_req(assoc_req->frame_control))
		assoc_req->u.assoc_req.listen_interval = cpu_to_le16(s1g_li);
	else
		assoc_req->u.reassoc_req.listen_interval  = cpu_to_le16(s1g_li);

	ht_cap = (const struct ieee80211_ht_cap *)ies_mask->ies[WLAN_EID_HT_CAPABILITY].ptr;
	morse_dot11ah_mask_ies(ies_mask, false, false);

	/* Enable ECSA */
	if (ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr) {
		u8 *ext_capa1 = (u8 *)ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr;

		ext_capa1[0] |= WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING;
	}

	morse_dot11ah_insert_s1g_aid_request(ies_mask);

	morse_dot11ah_insert_s1g_capability(vif, ht_cap,
		ies_mask, mors_vif->custom_configs->sta_type);

	morse_dot11ah_insert_pv1_hc_ie(vif, ies_mask, false);
}

static void morse_dot11ah_assoc_resp_to_s1g(struct ieee80211_vif *vif,
					struct sk_buff *skb,
					int s1g_hdr_length,
					struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *assoc_resp = (struct ieee80211_mgmt *)skb->data;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse_dot11ah_s1g_assoc_resp *s1g_assoc_resp;
	const struct ieee80211_ht_cap *ht_cap;
	u8 *s1g_ies = NULL;
	__le16 aid = assoc_resp->u.assoc_resp.aid & cpu_to_le16(0x3FFF);

	struct s1g_operation_parameters s1g_oper_params = {
		.chan_centre_freq_num =
			morse_dot11ah_freq_khz_bw_mhz_to_chan(HZ_TO_KHZ
				(mors_vif->custom_configs->channel_info.op_chan_freq_hz),
				mors_vif->custom_configs->channel_info.op_bw_mhz),
		.op_bw_mhz = mors_vif->custom_configs->channel_info.op_bw_mhz,
		.pri_bw_mhz = mors_vif->custom_configs->channel_info.pri_bw_mhz,
		.pri_1mhz_chan_idx = mors_vif->custom_configs->channel_info.pri_1mhz_chan_idx,
		.s1g_operating_class = mors_vif->custom_configs->channel_info.s1g_operating_class
	};

	/* Atomic allocation is required as this function can be called from the beacon tasklet. */
	s1g_assoc_resp = kmalloc(s1g_hdr_length, GFP_ATOMIC);
	if (!s1g_assoc_resp)
		return;

	memcpy(s1g_assoc_resp, assoc_resp, s1g_hdr_length);

	if (ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].ptr) {
		/* Update to S1G format */
		struct ieee80211_bss_max_idle_period_ie *bss_max_idle_period =
				(struct ieee80211_bss_max_idle_period_ie *)
				ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].ptr;
		u16 idle_period = le16_to_cpu(bss_max_idle_period->max_idle_period);
		u16 s1g_period = morse_dot11ah_listen_interval_to_s1g(idle_period);

		/* Convert to S1G (USF/UI) format */
		bss_max_idle_period->max_idle_period = cpu_to_le16(s1g_period);

		ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].ptr = (u8 *)bss_max_idle_period;
		ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].len = sizeof(*bss_max_idle_period);
	}

	ht_cap = (const struct ieee80211_ht_cap *)ies_mask->ies[WLAN_EID_HT_CAPABILITY].ptr;
	morse_dot11ah_mask_ies(ies_mask, false, false);

	/* Enable ECSA */
	if (ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr) {
		u8 *ext_capa1 = (u8 *)ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr;

		ext_capa1[0] |= WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING;
	}

	morse_dot11ah_insert_s1g_aid_response(ies_mask, aid);

	morse_dot11ah_insert_s1g_capability(vif,
		ht_cap,
		ies_mask,
		mors_vif->custom_configs->sta_type);

	morse_dot11ah_insert_s1g_operation(ies_mask, &s1g_oper_params);

	morse_dot11ah_insert_pv1_hc_ie(vif, ies_mask, true);

	/* This must be last */
	if (ies_mask->fils_data)
		s1g_ies = morse_dot11_insert_ie_no_header(s1g_ies, ies_mask->fils_data,
								ies_mask->fils_data_len);

	s1g_hdr_length = s1g_assoc_resp->variable - (u8 *)s1g_assoc_resp;
	if (skb->len < s1g_hdr_length)
		skb_put(skb, s1g_hdr_length - skb->len);

	memcpy(skb->data, s1g_assoc_resp, s1g_hdr_length);
	kfree(s1g_assoc_resp);
}

/* Check for ECSA IE in beacon/probe resp right after switching to new channel */
static void morse_dot11ah_check_for_ecsa_in_new_channel(struct ieee80211_vif *vif,
							struct dot11ah_ies_mask *ies_mask)
{
	struct morse_vif *mors_vif;
	const u8 *ie;
	struct ieee80211_ext_chansw_ie *ecsa_ie_info = (struct ieee80211_ext_chansw_ie *)
					ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr;
	struct ieee80211_wide_bw_chansw_ie *wbcsie;
	u8 pri_bw_mhz, pri_1mhz_chan_idx, op_chan_bw;
	u32 op_chan_freq_hz;
	u32 channel_flags;

	mors_vif = (struct morse_vif *)vif->drv_priv;

	ecsa_ie_info =
		(struct ieee80211_ext_chansw_ie *)ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr;
	if (ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].ptr) {
		ie = cfg80211_find_ie(WLAN_EID_WIDE_BW_CHANNEL_SWITCH,
					ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].ptr,
					ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].len);
	} else {
		ie = NULL;
	}

	if (ie) {
		u8 prim_1mhz_chan_loc = 0;

		wbcsie = (struct ieee80211_wide_bw_chansw_ie *)(ie + 2);
		op_chan_freq_hz = morse_dot11ah_s1g_chan_to_s1g_freq(wbcsie->new_center_freq_seg0);
		op_chan_bw = IEEE80211AH_S1G_OPERATION_GET_OP_CHAN_BW(wbcsie->new_channel_width);
		pri_bw_mhz = IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_BW(wbcsie->new_channel_width);
		prim_1mhz_chan_loc =
			IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_LOC(wbcsie->new_channel_width);
		pri_1mhz_chan_idx = morse_dot11_calc_prim_s1g_chan_loc(HZ_TO_KHZ
			(morse_dot11ah_s1g_chan_to_s1g_freq(ecsa_ie_info->new_ch_num)),
			HZ_TO_KHZ(op_chan_freq_hz), op_chan_bw);
		/*
		 * pri_1mhz_chan_idx points to lower channel now, update it based on the 1MHz
		 * primary channel location when primary channel bandwidth is 2MHz.
		 */
		if (pri_bw_mhz == 2 && prim_1mhz_chan_loc)
			pri_1mhz_chan_idx++;
	} else {
		channel_flags = morse_dot11ah_channel_get_flags(ecsa_ie_info->new_ch_num);
		if (channel_flags) {
			pri_bw_mhz = ch_flag_to_chan_bw(channel_flags);
		} else {
			pri_bw_mhz = 1;
			DOT11AH_WARN_ON(1);
		}
		op_chan_freq_hz =
			morse_dot11ah_s1g_chan_to_s1g_freq(ecsa_ie_info->new_ch_num);
		op_chan_bw = pri_bw_mhz;
		pri_1mhz_chan_idx = 0;
	}

	/*
	 * There is a rare case where mac80211 is taking time to update the beacon content while
	 * reserving & configuring hw for new channel announced in ECSA. This is resulting in old
	 * beacon content(ECSA IE) in new channel (only in 1st beacon and/or probe response).
	 */
	if (pri_1mhz_chan_idx == mors_vif->custom_configs->default_bw_info.pri_1mhz_chan_idx &&
	    pri_bw_mhz == mors_vif->custom_configs->default_bw_info.pri_bw_mhz) {
		if (op_chan_freq_hz == mors_vif->custom_configs->channel_info.op_chan_freq_hz &&
		    op_chan_bw == mors_vif->custom_configs->channel_info.op_bw_mhz) {
			/* mask the ECSA IEs */
			ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr = NULL;
			ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].len = false;
			ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].ptr = NULL;
			ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].len = false;
			dot11ah_info("Mask ECSA And Channel Switch Wrapper IEs. op_chan=%d, [%d-%d-%d]\n",
				      op_chan_freq_hz,
				      op_chan_bw,
				      pri_bw_mhz,
				      pri_1mhz_chan_idx);
		}
	}
}

static void morse_dot11ah_convert_ecsa_info_to_s1g(struct morse_vif *mors_vif,
					struct dot11ah_ies_mask *ies_mask)
{
	struct morse_channel_info *ecsa_chan_info = &mors_vif->ecsa_channel_info;
	struct ieee80211_ext_chansw_ie *pecsa =
		(struct ieee80211_ext_chansw_ie *)ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr;
	const u8 *ie = NULL;
	struct ieee80211_wide_bw_chansw_ie *wbcs_elem = NULL;
	u8 op_class_5g;
	u8 op_bw_mhz = 1;

	if (!pecsa)
		return;

	/* Update 5G Channels Info in ECSA IE and Wide Bandwidth channel switch IE to S1G */

	/* Disable legacy channel switch IE */
	ies_mask->ies[WLAN_EID_CHANNEL_SWITCH].ptr = NULL;

	op_class_5g = pecsa->new_operating_class;
	/*
	 * Maintaining two conditions:
	 * 1: S1G frequency as input to initiate chan_switch
	 * When S1G frequency is used hostapd sets S1G data to driver
	 * with MORSE_CMD_ID_SET_ECSA_S1G_INFO.
	 *
	 * 2: HT frequency as input to initiate chan_switch
	 * When ht frequecny is used no valid S1G data sets to driver
	 * but we still want to process ECSA
	 */
	if (ecsa_chan_info->op_chan_freq_hz > KHZ_TO_HZ(MORSE_S1G_FREQ_MIN_KHZ)) {
		int s1g_op_chan;

		s1g_op_chan = morse_dot11ah_freq_khz_bw_mhz_to_chan(HZ_TO_KHZ
				(ecsa_chan_info->op_chan_freq_hz),
				ecsa_chan_info->op_bw_mhz);
		pecsa->new_ch_num =
			morse_dot11ah_calc_prim_s1g_chan(ecsa_chan_info->op_bw_mhz,
				ecsa_chan_info->pri_bw_mhz,
				s1g_op_chan,
				ecsa_chan_info->pri_1mhz_chan_idx);

		/* Update with S1G operating class */
		pecsa->new_operating_class = ecsa_chan_info->s1g_operating_class;
	} else {
		pecsa->new_ch_num = morse_dot11ah_5g_chan_to_s1g_ch(pecsa->new_ch_num,
								    pecsa->new_operating_class);
	}

	if (ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].ptr)
		ie = cfg80211_find_ie(WLAN_EID_WIDE_BW_CHANNEL_SWITCH,
				ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].ptr,
				ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].len);

	if (ie)
		wbcs_elem = (struct ieee80211_wide_bw_chansw_ie *)(ie + 2);
	else
		return;

	dot11ah_info("%s: primary_bw=%u, op_bw=%u, prim_1mhz_chan=%u, ccfs0=%u, ccfs1=%u, width=%u\n",
				 __func__,
				ecsa_chan_info->pri_bw_mhz,
				ecsa_chan_info->op_bw_mhz,
				ecsa_chan_info->pri_1mhz_chan_idx,
				wbcs_elem->new_center_freq_seg0,
				wbcs_elem->new_center_freq_seg1,
				wbcs_elem->new_channel_width);

	/*
	 * As per IEEE 802.11-2020 std - Table 11-23—VHT BSS bandwidth:
	 * 1. new_channel_width value is 0 for 40 MHz, 1 for 80MHz, 160MHz and 80+80 MHz.
	 * 2. CCFS1 (Channel Center Frequency Segment 1) = 0 for 20/40/80 MHz bandwidths.
	 * 3. CCFS1 > 0 and |CCFS0 - CCFS1| = 8 for 160MHz.
	 */
	switch (wbcs_elem->new_channel_width) {
	case IEEE80211_VHT_CHANWIDTH_USE_HT:
		op_bw_mhz = 2;
		break;
	case IEEE80211_VHT_CHANWIDTH_80MHZ:
		if (!wbcs_elem->new_center_freq_seg1) {
			op_bw_mhz = 4;
		} else if (abs(wbcs_elem->new_center_freq_seg1 -
					wbcs_elem->new_center_freq_seg0) == 8) {
			op_bw_mhz = 8;
			/*
			 * The channel center frequence segment 1 contains the center frequency
			 * index of 160MHz channel, So Assign it to new_center_freq_seg0 as CCFS0
			 * contains operating channel center frequency in S1G.
			 */
			wbcs_elem->new_center_freq_seg0 = wbcs_elem->new_center_freq_seg1;
			/* new_center_freq_seg1 is reserved in S1G */
			wbcs_elem->new_center_freq_seg1 = 0;
		}
		break;
	default:
		dot11ah_err("%s: Invalid Bandwidth in Wide Bandwidth Channel Switch IE\n",
					__func__);
		break;
	}

	wbcs_elem->new_channel_width =
		IEEE80211AH_S1G_OPERATION_SET_PRIM_CHAN_BW(ecsa_chan_info->pri_bw_mhz) |
		IEEE80211AH_S1G_OPERATION_SET_OP_CHAN_BW(op_bw_mhz) |
		IEEE80211AH_S1G_OPERATION_SET_PRIM_CHAN_LOC(ecsa_chan_info->pri_1mhz_chan_idx % 2);

	/* Convert and update with S1G channel number */
	wbcs_elem->new_center_freq_seg0 =
		morse_dot11ah_5g_chan_to_s1g_ch(wbcs_elem->new_center_freq_seg0, op_class_5g);
}

static void morse_dot11ah_probe_resp_to_s1g(struct ieee80211_vif *vif,
					struct sk_buff *skb,
					struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *probe_resp = (struct ieee80211_mgmt *)skb->data;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	const struct ieee80211_ht_cap *ht_cap;

	struct s1g_operation_parameters s1g_oper_params = {
		.chan_centre_freq_num =
			morse_dot11ah_freq_khz_bw_mhz_to_chan(HZ_TO_KHZ
				(mors_vif->custom_configs->channel_info.op_chan_freq_hz),
				mors_vif->custom_configs->channel_info.op_bw_mhz),
		.op_bw_mhz = mors_vif->custom_configs->channel_info.op_bw_mhz,
		.pri_bw_mhz = mors_vif->custom_configs->channel_info.pri_bw_mhz,
		.pri_1mhz_chan_idx = mors_vif->custom_configs->channel_info.pri_1mhz_chan_idx,
		.s1g_operating_class = mors_vif->custom_configs->channel_info.s1g_operating_class,
		.prim_global_op_class =
			mors_vif->custom_configs->channel_info.pri_global_operating_class
	};

	/* SW-2241: The capabilities field is advertising short slot time.
	 * Short slot time is relevant to 80211g (2.4GHz). We should set that to 0
	 * so that in future we can repurpose the bit for some other 802.11ah use.
	 */
	probe_resp->u.probe_resp.capab_info &= ~cpu_to_le16(WLAN_CAPABILITY_SHORT_SLOT_TIME);

	ht_cap = (const struct ieee80211_ht_cap *)ies_mask->ies[WLAN_EID_HT_CAPABILITY].ptr;

	morse_dot11ah_mask_ies(ies_mask, false, false);

	/* Enable ECSA */
	if (ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr) {
		u8 *ext_capa1 = (u8 *)ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr;

		ext_capa1[0] |= WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING;
	}

	if (ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr) {
		struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;

		morse_dot11ah_convert_ecsa_info_to_s1g(mors_vif, ies_mask);
		if (mors_vif->mask_ecsa_info_in_beacon)
			morse_dot11ah_check_for_ecsa_in_new_channel(vif, ies_mask);
	}
	/* Clear Country IE, before inserting S1G country IE */
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_COUNTRY);
	morse_dot11ah_insert_country_ie(ies_mask, &s1g_oper_params);

	morse_dot11ah_insert_s1g_capability(vif,
		ht_cap,
		ies_mask,
		mors_vif->custom_configs->sta_type);

	morse_dot11ah_insert_s1g_operation(ies_mask, &s1g_oper_params);

	if (vif->bss_conf.dtim_period > 0)
		morse_dot11ah_insert_s1g_short_beacon_interval(ies_mask, vif->bss_conf.beacon_int);
}

static void morse_dot11ah_probe_req_to_s1g(struct ieee80211_vif *vif,
					  struct sk_buff *skb,
					  struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *probe_req = (struct ieee80211_mgmt *)skb->data;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	const struct ieee80211_ht_cap *ht_cap;
	u8 zero_mac[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	/* In IBSS mode, scan is triggered from mac80211 and does not set
	 * broadcast bssid to the probe request which resulted in no probe
	 * response from the nodes. Fill probe request with broadcast mac here
	 */
	if (ether_addr_equal_unaligned(probe_req->da, zero_mac))
		eth_broadcast_addr(probe_req->da);

	if (ether_addr_equal_unaligned(probe_req->bssid, zero_mac))
		eth_broadcast_addr(probe_req->bssid);

	ht_cap = (const struct ieee80211_ht_cap *)ies_mask->ies[WLAN_EID_HT_CAPABILITY].ptr;
	morse_dot11ah_mask_ies(ies_mask, false, false);
	/* Enable ECSA */
	if (ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr) {
		u8 *ext_capa1 = (u8 *)ies_mask->ies[WLAN_EID_EXT_CAPABILITY].ptr;

		ext_capa1[0] |= WLAN_EXT_CAPA1_EXT_CHANNEL_SWITCHING;
	}
	morse_dot11ah_insert_s1g_capability(vif,
					    ht_cap,
					    ies_mask,
					    mors_vif->custom_configs->sta_type);
}

#if !defined(MORSE_MAC80211_S1G_FEATURE_NDP_BLOCKACK)
static void morse_dot11ah_blockack_to_s1g(struct ieee80211_vif *vif, struct sk_buff *skb)
{
	struct ieee80211_mgmt *back = (struct ieee80211_mgmt *)skb->data;

	switch (back->u.action.u.addba_req.action_code) {
	case WLAN_ACTION_ADDBA_REQ:
		back->u.action.u.addba_req.action_code = WLAN_ACTION_NDP_ADDBA_REQ;
		break;
	case WLAN_ACTION_ADDBA_RESP:
		back->u.action.u.addba_req.action_code = WLAN_ACTION_NDP_ADDBA_RESP;
		break;
	case WLAN_ACTION_DELBA:
		back->u.action.u.addba_req.action_code = WLAN_ACTION_NDP_DELBA;
		break;
	default:
		break;
	}
}
#endif

/**
 * Utility function to find EDCA parameter IE data in incoming beacon
 * from mac80211. The parameter data can either be part of EDCA IE or
 * WMM IE, which is Vendor specific IE. IEs of the beacon are already
 * parsed and stored in ies_mask and for vendor IEs, there can be multiple
 * elements stored in ies[WLAN_EID_VENDOR_SPECIFIC] list.
 */
static u8 *morse_dot11ah_find_edca_param_set_ie(struct ieee80211_vif *vif, struct sk_buff *skb,
						struct dot11ah_ies_mask *ies_mask,
						u16 *edca_ie_len)
{
	struct __ieee80211_vendor_ie_elem  *ven_ie = NULL;
	struct ie_element *elem = NULL;

	/*
	 * Check for EDCA IE presence
	 */
	if (ies_mask->ies[WLAN_EID_EDCA_PARAM_SET].ptr) {
		*edca_ie_len = ies_mask->ies[WLAN_EID_EDCA_PARAM_SET].len;

		return ((u8 *)ies_mask->ies[WLAN_EID_EDCA_PARAM_SET].ptr);
	} else if (ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC].ptr) {
		/*
		 * Check for WMM IE in the list of vendor specific IEs
		 */
		ven_ie = (struct __ieee80211_vendor_ie_elem  *)
					ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC].ptr;
		elem = &ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC];

		while (elem && elem->ptr) {
			ven_ie = (struct __ieee80211_vendor_ie_elem  *)elem->ptr;
			if (IS_WMM_IE(ven_ie)) {
				*edca_ie_len = ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC].len -
						sizeof(*ven_ie);

				return ((u8 *)ven_ie->attr);
			};

			elem = elem->next;
		}
	}

	return NULL;
}

/**
 * Utility function to find if beacon is changed as per IEEE-2020 sec 10.46.2
 * System information update procedure :
 *
 * ##
 * The S1G AP shall increase the value (modulo 256) of the Change Sequence field
 * in the next transmitted S1G Beacon frame(s) when a critical update occurs to any
 * of the elements inside the S1G Beacon frame. The following events shall classify
 * as a critical update:
 *   a) Inclusion of an Extended Channel Switch Announcement
 *   b) Modification of the EDCA parameters
 *   c) Modification of the S1G Operation element
 * ##
 *
 * 1st one is checked for presence of IE in incoming beacon from mac80211
 * 2nd & 3rd  IE changes are tracked using CRC values of prior beacon frames.
 *
 */
static int morse_dot11ah_find_beacon_change(struct ieee80211_vif *vif,
					    struct sk_buff *skb,
					    struct dot11ah_ies_mask *ies_mask,
					    struct s1g_operation_parameters *s1g_oper_params)
{
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	u8 update_change_seq = 0;
	u32 ncrc = 0, op_param_crc = 0;
	u16 edca_ie_len = 0;

	u8 *edca_param_set = NULL;

	edca_param_set = morse_dot11ah_find_edca_param_set_ie(vif, skb, ies_mask, &edca_ie_len);
	op_param_crc = ~crc32(~0, (void *)s1g_oper_params,
			      sizeof(struct s1g_operation_parameters));

	/*
	 * Find the channel switch announcement or extended channel switch announcement
	 */
	if (ies_mask->ies[WLAN_EID_CHANNEL_SWITCH].ptr ||
	    ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr) {
		if (!mors_vif->chan_switch_in_progress) {
			update_change_seq = 1;
			mors_vif->chan_switch_in_progress = true;
			dot11ah_info("Detected CSA parameters IE change\n");
		}
	} else {
		mors_vif->chan_switch_in_progress = false;
	}

	/*
	 * EDCA parameters
	 */
	if (edca_param_set) {
		ncrc = ~crc32(~0, (void *)edca_param_set, edca_ie_len);
		/**
		 * Check for any EDCA parameters update
		 */
		if (!mors_vif->edca_param_crc) {
			mors_vif->edca_param_crc = ncrc;
		} else if (ncrc != mors_vif->edca_param_crc) {
			update_change_seq = 1;
			mors_vif->edca_param_crc = ncrc;
			dot11ah_info("Detected EDCA parameters IE change\n");
		}
	}

	/*
	 * S1G operational parameters
	 */
	if (!mors_vif->s1g_oper_param_crc) {
		mors_vif->s1g_oper_param_crc = op_param_crc;
	} else if (op_param_crc != mors_vif->s1g_oper_param_crc) {
		/**
		 * Check for any S1G operational IE updates
		 */
		update_change_seq = 1;
		mors_vif->s1g_oper_param_crc = op_param_crc;
		dot11ah_info("Detected S1G operation parameters IEs\n");
	}

	return update_change_seq;
}

static void morse_dot11ah_beacon_to_s1g(struct ieee80211_vif *vif,
					struct sk_buff *skb,
					int s1g_hdr_length,
					bool short_beacon,
					struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *beacon =
				(struct ieee80211_mgmt *)skb->data;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct ieee80211_ext *s1g_beacon;
	const struct ieee80211_ht_cap *ht_cap;
	u8 *s1g_beacon_opt_fields = NULL;
	u8 *rsn_ie;
	u8 rsn_ie_len;
	u16 frame_control = IEEE80211_FTYPE_EXT | IEEE80211_STYPE_S1G_BEACON;
	struct s1g_operation_parameters s1g_oper_params = {
		.chan_centre_freq_num = morse_dot11ah_freq_khz_bw_mhz_to_chan(HZ_TO_KHZ
				(mors_vif->custom_configs->channel_info.op_chan_freq_hz),
				mors_vif->custom_configs->channel_info.op_bw_mhz),
		.op_bw_mhz = mors_vif->custom_configs->channel_info.op_bw_mhz,
		.pri_bw_mhz = mors_vif->custom_configs->channel_info.pri_bw_mhz,
		.pri_1mhz_chan_idx = mors_vif->custom_configs->channel_info.pri_1mhz_chan_idx,
		.s1g_operating_class = mors_vif->custom_configs->channel_info.s1g_operating_class
	};

	/* An atomic allocation is required as this function can be called from
	 * the beacon tasklet.
	 */
	s1g_beacon = kmalloc(s1g_hdr_length, GFP_ATOMIC);
	if (!s1g_beacon)
		return;

	if (ies_mask->ies[WLAN_EID_SSID].ptr && short_beacon)
		frame_control |= IEEE80211_FC_COMPRESS_SSID;

	/* SW-1974: Use the presence of the RSN element in the 80211n beacon
	 * to determine if the security supported bit should be set.
	 */
	if (ies_mask->ies[WLAN_EID_RSN].ptr)
		frame_control |= IEEE80211_FC_S1G_SECURITY_SUPPORTED;

	frame_control |= ieee80211ah_s1g_fc_bss_bw_lookup
		[mors_vif->custom_configs->channel_info.pri_bw_mhz]
		[mors_vif->custom_configs->channel_info.op_bw_mhz];

	/* Fill in the new beacon header, copied from incoming frame */

	s1g_beacon->frame_control = cpu_to_le16(frame_control);
	s1g_beacon->duration = 0;

	/* SW-4741: for IBSS, SA address MUST be set to the randomly generated BSSID
	 * This will not break infrastructure BSS mode anyway as for this both SA and
	 * BSSID in beacon are equivalent
	 */
	memcpy(s1g_beacon->u.s1g_beacon.sa, beacon->bssid, sizeof(s1g_beacon->u.s1g_beacon.sa));

	/* The position of the last field in S1G beacon before any IE */
	s1g_beacon_opt_fields = s1g_beacon->u.s1g_beacon.variable;
	ht_cap = (const struct ieee80211_ht_cap *)ies_mask->ies[WLAN_EID_HT_CAPABILITY].ptr;

	/* Take backup of RSN IE to restore it for mesh interface, after masking */
	rsn_ie = ies_mask->ies[WLAN_EID_RSN].ptr;
	rsn_ie_len = ies_mask->ies[WLAN_EID_RSN].len;

	morse_dot11ah_mask_ies(ies_mask, true, true);
	/* Include RSN IE for Beacon in Mesh for SAE connection */
	if (ieee80211_vif_is_mesh(vif)) {
		ies_mask->ies[WLAN_EID_RSN].ptr = rsn_ie;
		ies_mask->ies[WLAN_EID_RSN].len = rsn_ie_len;
	}

	/* The SSID is 2 octets into the value returned by find ie, and the
	 * length is the second octet
	 */
	if (short_beacon) {
		if (ies_mask->ies[WLAN_EID_SSID].ptr) {
			/* Do not create CSSID entry for mesh beacons, it is created on reception.
			 * Also skip updating cssid for mesh beacons. This is to avoid confusion
			 * for Infrastructure stations.
			 */
			if (!morse_is_mesh_network(ies_mask)) {
				u32 cssid = morse_generate_cssid(ies_mask->ies[WLAN_EID_SSID].ptr,
					ies_mask->ies[WLAN_EID_SSID].len);

				/* Insert CSSID (as first entry in s1g_beacon->variable for short
				 * beacon)
				 */
				*((u32 *)s1g_beacon_opt_fields) = cssid;

			} else {
				*((u32 *)s1g_beacon_opt_fields) = 0;
			}
			s1g_beacon_opt_fields += 4;
			morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_SSID);
		}
	} else {
		struct morse_channel_info *chan_info = &mors_vif->custom_configs->channel_info;
		struct s1g_operation_parameters s1g_oper_params = {
			.chan_centre_freq_num = morse_dot11ah_freq_khz_bw_mhz_to_chan(HZ_TO_KHZ
					(chan_info->op_chan_freq_hz),
					chan_info->op_bw_mhz),
			.op_bw_mhz = chan_info->op_bw_mhz,
			.pri_bw_mhz = chan_info->pri_bw_mhz,
			.pri_1mhz_chan_idx = chan_info->pri_1mhz_chan_idx,
			.s1g_operating_class = chan_info->s1g_operating_class
		};
		u64 now_usecs = jiffies_to_usecs((get_jiffies_64() - mors_vif->epoch));

		morse_dot11ah_insert_s1g_compatibility(ies_mask,
						       le16_to_cpu(beacon->u.beacon.beacon_int) *
						       vif->bss_conf.dtim_period,
						       le16_to_cpu(beacon->u.beacon.capab_info),
						       UPPER_32_BITS(now_usecs));

		morse_dot11ah_insert_s1g_capability(vif,
			ht_cap,
			ies_mask,
			mors_vif->custom_configs->sta_type);

		morse_dot11ah_insert_s1g_operation(ies_mask, &s1g_oper_params);
		morse_dot11ah_insert_s1g_short_beacon_interval(ies_mask,
							  le16_to_cpu(beacon->u.beacon.beacon_int));

		if (ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr) {
			morse_dot11ah_convert_ecsa_info_to_s1g(mors_vif, ies_mask);
			if (mors_vif->mask_ecsa_info_in_beacon)
				morse_dot11ah_check_for_ecsa_in_new_channel(vif, ies_mask);
		}
	}

	/* Clear Country IE from beacon, if it's inserted by hostapd conf with 11d = 1 */
	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_COUNTRY);

	/* Detect the change in beacon IEs and update the change seq number.
	 * Add mode check as beacon change sequence is not applicable for adhoc mode
	 */
	if (vif->type == NL80211_IFTYPE_AP &&
	    morse_dot11ah_find_beacon_change(vif, skb, ies_mask, &s1g_oper_params)) {
		mors_vif->s1g_bcn_change_seq++;
		mors_vif->s1g_bcn_change_seq = (mors_vif->s1g_bcn_change_seq % 256);
		dot11ah_info("Updating the change seq num to %d\n", mors_vif->s1g_bcn_change_seq);
	}

	s1g_beacon->u.s1g_beacon.change_seq = mors_vif->s1g_bcn_change_seq;

	s1g_hdr_length = s1g_beacon_opt_fields - (u8 *)s1g_beacon;
	memcpy(skb->data, s1g_beacon, s1g_hdr_length);
	kfree(s1g_beacon);
}

/**
 * morse_dot11_get_rsn_caps: Parse RSN IE and return RSN capabilities
 *
 * @rsn_ie: pointer to RSN IE.
 * @rsn_caps: RSN capabilities.
 *
 * @Return: 0 on success or relevant error.
 */
static int morse_dot11_get_rsn_caps(const u8 *rsn_ie, u16 *rsn_caps)
{
	u8 rsn_ie_len = *(rsn_ie + 1);
	u16 count;

	/* validate RSN IE length */
	if (rsn_ie_len < 2)
		return -EINVAL;

	/* skip eid and length */
	rsn_ie += 2;

	/* skip version */
	rsn_ie += 2;
	rsn_ie_len -= 2;

	/* verify and skip group cipher len*/
	if (rsn_ie_len < RSN_SELECTOR_LEN)
		return 0;
	rsn_ie += RSN_SELECTOR_LEN;
	rsn_ie_len -= RSN_SELECTOR_LEN;

	/* verify and skip pairwise count(2 bytes) and cipher len*/
	count = le16_to_cpu(*(__le16 *)(rsn_ie));
	if (rsn_ie_len < (count * RSN_SELECTOR_LEN))
		return 0;
	rsn_ie += (2 + count * RSN_SELECTOR_LEN);
	rsn_ie_len -= (2 + count * RSN_SELECTOR_LEN);

	/* verify and skip akm count(2 byte) and akm len*/
	count = le16_to_cpu(*(__le16 *)(rsn_ie));
	if (rsn_ie_len < (count * RSN_SELECTOR_LEN))
		return 0;
	rsn_ie += (2 + count * RSN_SELECTOR_LEN);
	rsn_ie_len -= (2 + count * RSN_SELECTOR_LEN);

	/* verify length and copy rsn caps(2 bytes) */
	if (rsn_ie_len < 2)
		return 0;

	*rsn_caps = le16_to_cpu(*((__le16 *)rsn_ie));

	return 0;
}

int morse_dot11_get_mpm_ampe_len(struct sk_buff *skb)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
	u16 cap_info;
	int ampe_len = 0;

	cap_info = le16_to_cpu(*(__le16 *)mgmt->u.action.u.self_prot.variable);

	if (cap_info & WLAN_CAPABILITY_PRIVACY) {
		if (mgmt->u.action.u.self_prot.action_code == WLAN_SP_MESH_PEERING_OPEN) {
			u8 *peering_frame_ies = morse_dot11_mpm_frame_ies(mgmt);
			const u8 *rsn_ie;
			u16 rsn_caps = 0;
			int header_length = peering_frame_ies - skb->data;
			int peering_frame_ies_len = skb->len - header_length;

			ampe_len = AMPE_BLOCK_SIZE_OPEN_FRAME;
			rsn_ie = cfg80211_find_ie(WLAN_EID_RSN, peering_frame_ies,
									peering_frame_ies_len);
			if (rsn_ie && !morse_dot11_get_rsn_caps(rsn_ie, &rsn_caps)) {
				if ((rsn_caps & RSN_CAPABILITY_MFPR) &&
							(rsn_caps & RSN_CAPABILITY_MFPC))
					ampe_len += AMPE_BLOCK_IGTK_DATA_LEN;
			}
		} else if (mgmt->u.action.u.self_prot.action_code == WLAN_SP_MESH_PEERING_CONFIRM) {
			ampe_len = AMPE_BLOCK_SIZE_CONFIRM_FRAME;
		}
	}

	return ampe_len;
}
EXPORT_SYMBOL(morse_dot11_get_mpm_ampe_len);

/* Convert Mesh Peering Management (MPM) frame to S1G i.e. Remove HT/VHT IEs IEs & add S1G IEs */
static void morse_dot11ah_mpm_frame_to_s1g(struct ieee80211_vif *vif,
				struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	const struct ieee80211_ht_cap *ht_cap;
	struct s1g_operation_parameters s1g_oper_params = {
		.chan_centre_freq_num =
			morse_dot11ah_freq_khz_bw_mhz_to_chan(HZ_TO_KHZ
				(mors_vif->custom_configs->channel_info.op_chan_freq_hz),
				mors_vif->custom_configs->channel_info.op_bw_mhz),
		.op_bw_mhz = mors_vif->custom_configs->channel_info.op_bw_mhz,
		.pri_bw_mhz = mors_vif->custom_configs->channel_info.pri_bw_mhz,
		.pri_1mhz_chan_idx = mors_vif->custom_configs->channel_info.pri_1mhz_chan_idx,
		.s1g_operating_class = mors_vif->custom_configs->channel_info.s1g_operating_class,
		.prim_global_op_class =
			mors_vif->custom_configs->channel_info.pri_global_operating_class
	};
	ht_cap = (const struct ieee80211_ht_cap *)ies_mask->ies[WLAN_EID_HT_CAPABILITY].ptr;

	morse_dot11ah_mask_ies(ies_mask, true, false);

	morse_dot11ah_insert_s1g_capability(vif, ht_cap,
		ies_mask, mors_vif->custom_configs->sta_type);

	morse_dot11ah_insert_s1g_operation(ies_mask, &s1g_oper_params);
}

void morse_dot11ah_11n_to_s1g_tx_packet(struct ieee80211_vif *vif,
					struct sk_buff *skb, int s1g_hdr_length,
					bool short_beacon,
					struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_hdr *hdr;

	if (!ies_mask)
		return;

	hdr = (struct ieee80211_hdr *)skb->data;

	if (ieee80211_is_action(hdr->frame_control)) {
		struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)hdr;

		if (morse_dot11_is_mpm_frame(mgmt))
			morse_dot11ah_mpm_frame_to_s1g(vif, skb, ies_mask);
#if !defined(MORSE_MAC80211_S1G_FEATURE_NDP_BLOCKACK)
		else if (mgmt->u.action.category == WLAN_CATEGORY_BACK)
			morse_dot11ah_blockack_to_s1g(vif, skb);
#endif
	}

	if (ieee80211_is_beacon(hdr->frame_control))
		morse_dot11ah_beacon_to_s1g(vif, skb, s1g_hdr_length, short_beacon, ies_mask);
	else if (ieee80211_is_probe_req(hdr->frame_control))
		morse_dot11ah_probe_req_to_s1g(vif, skb, ies_mask);
	else if (ieee80211_is_probe_resp(hdr->frame_control))
		morse_dot11ah_probe_resp_to_s1g(vif, skb, ies_mask);
	else if (ieee80211_is_assoc_req(hdr->frame_control) ||
		ieee80211_is_reassoc_req(hdr->frame_control))
		morse_dot11ah_assoc_req_to_s1g(vif, skb, ies_mask);
	else if (ieee80211_is_assoc_resp(hdr->frame_control) ||
		ieee80211_is_reassoc_resp(hdr->frame_control))
		morse_dot11ah_assoc_resp_to_s1g(vif, skb, s1g_hdr_length, ies_mask);
}
EXPORT_SYMBOL(morse_dot11ah_11n_to_s1g_tx_packet);
