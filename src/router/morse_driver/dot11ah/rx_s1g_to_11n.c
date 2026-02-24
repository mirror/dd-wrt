/*
 * Copyright 2020 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include <linux/ieee80211.h>

#include "dot11ah.h"
#include "tim.h"
#include "debug.h"
#include "../debug.h"
#include "../morse.h"
#include "../mesh.h"
#include "../s1g_ies.h"

#define VHT_HT_PRIMARY_CH_OFFSET_80MHZ		(6)
#define VHT_HT_PRIMARY_CH_OFFSET_160MHZ		(14)

#define S1G_OPERATION_IE_BSS_OP_WIDTH_4_MHZ	(3)
#define S1G_OPERATION_IE_BSS_OP_WIDTH_8_MHZ	(7)

#if KERNEL_VERSION(4, 12, 0) > MAC80211_VERSION_CODE
/* Older Kernels appear 1 referenced where later ones are 0 referenced. */
#define CENTER_FREQ_SEG0_IDX center_freq_seg1_idx
#define CENTER_FREQ_SEG1_IDX center_freq_seg2_idx
#else
#define CENTER_FREQ_SEG0_IDX center_freq_seg0_idx
#define CENTER_FREQ_SEG1_IDX center_freq_seg1_idx
#endif

/* Operating class of 5GHz channel 50. It is not covered by
 * ieee80211_chandef_to_operating_class function.
 */
#define OPERATING_CLASS_OF_5GHZ_CHANNEL_NUM_50 129

/* Hard coded IEs in case they are missing */
static const struct ieee80211_ht_cap __ht_cap_ie = {
	.cap_info = cpu_to_le16(0x000C |
				IEEE80211_HT_CAP_SUP_WIDTH_20_40),
	.ampdu_params_info  = 0x00,
	.mcs = {
		.rx_mask = {
			0xff, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00
		},
		.rx_highest = cpu_to_le16(0x0041),
		.tx_params = 0x01,
	},
	.extended_ht_cap_info = cpu_to_le16(0x0000),
	.tx_BF_cap_info = cpu_to_le32(0x00000000),
	.antenna_selection_info = 0x00,
};

static struct ieee80211_ht_operation __ht_oper_ie = {
	.primary_chan = 0x04,
	.ht_param = 0x01,
	.operation_mode = cpu_to_le16(0x0000),
	.stbc_param = cpu_to_le16(0x0000),
	.basic_set = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
};

static const struct ieee80211_vht_cap __vht_cap_ie = {
	.vht_cap_info = cpu_to_le32(IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
				    IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK),
};

static struct ieee80211_vht_operation __vht_oper_ie = {
	.CENTER_FREQ_SEG0_IDX = 0,
	.CENTER_FREQ_SEG1_IDX = 0,
	.basic_mcs_set = cpu_to_le16(IEEE80211_VHT_MCS_SUPPORT_0_8 << 0
				    | IEEE80211_VHT_MCS_NOT_SUPPORTED << 2
				    | IEEE80211_VHT_MCS_NOT_SUPPORTED << 4
				    | IEEE80211_VHT_MCS_NOT_SUPPORTED << 6
				    | IEEE80211_VHT_MCS_NOT_SUPPORTED << 8
				    | IEEE80211_VHT_MCS_NOT_SUPPORTED << 10
				    | IEEE80211_VHT_MCS_NOT_SUPPORTED << 12
				    | IEEE80211_VHT_MCS_NOT_SUPPORTED << 14),
};

static const struct ieee80211_wmm_param_ie __wmm_ie = {
	.element_id = WLAN_EID_VENDOR_SPECIFIC,
	.len = 24,
	.oui = {0x00, 0x50, 0xf2},
	.oui_type = 2,
	.oui_subtype = 1,
	.version = 1,
	.qos_info = 0,
	.reserved = 0,
	.ac = {
		{
			.aci_aifsn = (0x0 << 4) | 3,
			.cw = (6 << 4) | 4,
			.txop_limit = 0
		}, {
			.aci_aifsn = (0x1 << 4) | 7,
			.cw = (10 << 4) | 4,
			.txop_limit = 0
		}, {
			.aci_aifsn = (0x2 << 4) | 1,
			.cw = (4 << 4) | 3,
			.txop_limit = cpu_to_le16(94)
		}, {
			.aci_aifsn = (0x3 << 4) | 1,
			.cw = (3 << 4) | 2,
			.txop_limit = cpu_to_le16(47)
		}
	}
};

/* Supported rates (including basic rates) in units of 0.5 Mbps */
static const u8  __s1g_supp_rates_ie[] = {
	0x02, /*  1.0 Mbps, basic rates for 2.4Ghz */
	0x04, /*  2.0 Mbps, basic rates for 2.4Ghz */
	0x0b, /*  5.5 Mbps, basic rates for 2.4Ghz */
	0x8c, /*  6.0 Mbps, basic rates for 5Ghz */
	0x16, /* 11.0 Mbps, basic rates for 2.4Ghz */
	0x98, /* 12.0 Mbps, basic rates for 5Ghz */
	0x24, /* 18.0 Mbps */
	0xb0, /* 24.0 Mbps, basic rates for 5Ghz */
};

/* APIs used to insert various 11n information elements (used only in this file) */

/* Parses fields from s1g capability field and maps them to ht capability */
static u8 *morse_dot11_insert_ht_cap_ie(u8 *pos, const struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_ht_cap ht_cap;
	u8 s1g_cap3;
	u8 ampdu_len_exp;
	u8 ampdu_mss;

	if (ies_mask->ies[WLAN_EID_HT_CAPABILITY].ptr)
		return morse_dot11_insert_ie_from_ies_mask(pos, ies_mask, WLAN_EID_HT_CAPABILITY);

	memcpy(&ht_cap, &__ht_cap_ie, sizeof(ht_cap));

	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr) {
		/* A-MPDU parameters */
		s1g_cap3 = ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[3];
		ampdu_len_exp = S1G_CAP3_GET_MAX_AMPDU_LEN_EXP(s1g_cap3);
		ampdu_mss = S1G_CAP3_GET_MIN_AMPDU_START_SPC(s1g_cap3);
		ht_cap.ampdu_params_info = ampdu_len_exp | (ampdu_mss << 2);

		/* SGI parameters
		 * If we have any SGI caps, assume we have all.
		 */
		if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[0] & (S1G_CAP0_SGI_1MHZ |
				       S1G_CAP0_SGI_2MHZ |
				       S1G_CAP0_SGI_4MHZ |
				       S1G_CAP0_SGI_8MHZ)) {
			ht_cap.cap_info |= cpu_to_le16(IEEE80211_HT_CAP_SGI_20);
			ht_cap.cap_info |= cpu_to_le16(IEEE80211_HT_CAP_SGI_40);
		}
	}

	return morse_dot11_insert_ie(pos, (const u8 *)&ht_cap,
				      WLAN_EID_HT_CAPABILITY, sizeof(ht_cap));
}

static void morse_dot11_s1g_oper_expand(const u8 *s1g_oper,
			struct s1g_operation_params_expanded *params)
{
	u8 ch_width_flags = s1g_oper[0];

	params->op_class = s1g_oper[1];
	params->pri_ch = s1g_oper[2];
	params->op_ch = s1g_oper[3];

	params->use_mcs10 = (ch_width_flags & (1 << 7)) ? false : true;
	params->primary_2mhz = (ch_width_flags & (1 << 0)) ? false : true;
	params->upper_1mhz = IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_LOC(ch_width_flags);
	switch ((ch_width_flags & 0x1E) >> 1) {
	case 0:
		params->op_bw = 1;
		break;
	case 1:
		params->op_bw = 2;
		break;
	case 3:
		params->op_bw = 4;
		break;
	case 7:
		params->op_bw = 8;
		break;
	case 15:
		params->op_bw = 16;
		break;
	default:
		params->op_bw = 0;
		break;
	}
}

static u8 *morse_dot11_insert_vht_cap_ie(u8 *pos, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_vht_cap vht_cap;
	u16 vht_mcs_rx_map = 0;
	u16 vht_mcs_tx_map = 0;
	int i;

	if (ies_mask->ies[WLAN_EID_VHT_CAPABILITY].ptr)
		return morse_dot11_insert_ie_from_ies_mask(pos, ies_mask, WLAN_EID_VHT_CAPABILITY);

	/* Initialise the vht cap with our known defaults */
	memcpy(&vht_cap, &__vht_cap_ie, sizeof(vht_cap));

	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr) {
		struct ieee80211_s1g_cap *s1g_capab_ie =
			(struct ieee80211_s1g_cap *)ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr;
		u32 vht_capab_info = le32_to_cpu(vht_cap.vht_cap_info);
		u8 *s1g_capab_info = s1g_capab_ie->capab_info;
		u8 *s1g_supp_mcs_nss = s1g_capab_ie->supp_mcs_nss;
		u8 s1g_rx_mcs_map = s1g_supp_mcs_nss[0];
		u8 s1g_tx_mcs_map = (s1g_supp_mcs_nss[2] >> 1) | (s1g_supp_mcs_nss[3] << 7);

		if ((s1g_capab_info[0] & S1G_CAP0_SUPP_CH_WIDTH) >= S1G_CAP0_SUPP_8MHZ)
			vht_capab_info |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;

		/* sgi parameters */
		if (s1g_capab_info[0] & S1G_CAP0_SGI_4MHZ)
			vht_capab_info |= IEEE80211_VHT_CAP_SHORT_GI_80;
		if (s1g_capab_info[0] & S1G_CAP0_SGI_8MHZ)
			vht_capab_info |= IEEE80211_VHT_CAP_SHORT_GI_160;

		if (s1g_capab_info[1] & S1G_CAP1_RX_LDPC)
			vht_capab_info |= IEEE80211_VHT_CAP_RXLDPC;
		if (s1g_capab_info[1] & S1G_CAP1_TX_STBC)
			vht_capab_info |= IEEE80211_VHT_CAP_TXSTBC;
		if (s1g_capab_info[1] & S1G_CAP1_RX_STBC)
			vht_capab_info |= IEEE80211_VHT_CAP_RXSTBC_1;
		if (s1g_capab_info[1] & S1G_CAP1_SU_BFER)
			vht_capab_info |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
		if (s1g_capab_info[1] & S1G_CAP1_SU_BFEE) {
			vht_capab_info |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
			vht_capab_info |= (S1G_CAP1_GET_BFEE_STS(s1g_capab_info[1])
					   << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT);
		} else if (s1g_capab_info[1] & S1G_CAP1_BFEE_STS) {
			dot11ah_warn_ratelimited("Beamformee STS set without being SU Beamformee capable");
		}
		vht_capab_info |= (S1G_CAP2_GET_SOUNDING_DIMENSIONS(s1g_capab_info[2])
			<< IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT);

		if (s1g_capab_info[2] & S1G_CAP2_MU_BFER)
			vht_capab_info |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;
		if (s1g_capab_info[2] & S1G_CAP2_MU_BFEE)
			vht_capab_info |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;

		vht_cap.vht_cap_info = cpu_to_le32(vht_capab_info);

		dot11ah_debug("s1g rx_mcs_map 0x%02x", s1g_rx_mcs_map);
		dot11ah_debug("s1g tx_mcs_map 0x%02x", s1g_tx_mcs_map);

		for (i = 0; i < NL80211_VHT_NSS_MAX; i++) {
			if (i >= 0 && i < 4) {
				vht_mcs_rx_map |= s1g_rx_mcs_map &
					(GENMASK(1, 0) << (S1G_CAP_BITS_PER_MCS_NSS * i));
				vht_mcs_tx_map |= s1g_tx_mcs_map &
					(GENMASK(1, 0) << (S1G_CAP_BITS_PER_MCS_NSS * i));
				continue;
			}

			vht_mcs_rx_map |= (IEEE80211_VHT_MCS_NOT_SUPPORTED <<
					   (i * S1G_CAP_BITS_PER_MCS_NSS));
			vht_mcs_tx_map |= (IEEE80211_VHT_MCS_NOT_SUPPORTED <<
					   (i * S1G_CAP_BITS_PER_MCS_NSS));
		}
	}

	dot11ah_debug("vht rx_mcs_map 0x%04x", vht_mcs_rx_map);
	dot11ah_debug("vht tx_mcs_map 0x%04x", vht_mcs_tx_map);
	vht_cap.supp_mcs.rx_mcs_map = cpu_to_le16(vht_mcs_rx_map);
	vht_cap.supp_mcs.tx_mcs_map = cpu_to_le16(vht_mcs_tx_map);

	return morse_dot11_insert_ie(pos, (const u8 *)&vht_cap,
				     WLAN_EID_VHT_CAPABILITY, sizeof(vht_cap));
}

static u8 *morse_dot11_insert_vht_oper_ie(u8 *pos, struct ieee80211_rx_status *rxs,
					  struct dot11ah_ies_mask *ies_mask)
{
	int op_chan = 0;
	struct s1g_operation_params_expanded s1g_oper_params;

	memset(&s1g_oper_params, 0, sizeof(s1g_oper_params));

	if (ies_mask->ies[WLAN_EID_VHT_OPERATION].ptr)
		return morse_dot11_insert_ie_from_ies_mask(pos, ies_mask, WLAN_EID_VHT_OPERATION);

	if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr)
		morse_dot11_s1g_oper_expand(ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr,
					    &s1g_oper_params);

	op_chan = morse_dot11ah_s1g_chan_to_5g_chan(s1g_oper_params.op_ch);

	__vht_oper_ie.CENTER_FREQ_SEG0_IDX = op_chan;

	if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr && s1g_oper_params.op_bw == 4)
		__vht_oper_ie.chan_width = IEEE80211_VHT_CHANWIDTH_80MHZ;
	else if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr && s1g_oper_params.op_bw == 8)
		__vht_oper_ie.chan_width = IEEE80211_VHT_CHANWIDTH_160MHZ;
	else
		__vht_oper_ie.chan_width = IEEE80211_VHT_CHANWIDTH_USE_HT;

	return morse_dot11_insert_ie(pos, (const u8 *)&__vht_oper_ie,
				    WLAN_EID_VHT_OPERATION, sizeof(__vht_oper_ie));
}

static u8 *morse_dot11_insert_wmm_ie(u8 *pos, const struct dot11ah_ies_mask *ies_mask)
{
	if (ies_mask->ies[WLAN_EID_EDCA_PARAM_SET].ptr) {
		struct ieee80211_wmm_param_ie wmm_ie;
		const struct __ieee80211_edca_ie *edca = (const struct __ieee80211_edca_ie *)
							ies_mask->ies[WLAN_EID_EDCA_PARAM_SET].ptr;

		/* Copy defaults and update ACs */
		memcpy(&wmm_ie, &__wmm_ie, sizeof(wmm_ie));
		wmm_ie.ac[0].aci_aifsn = edca->ac_be.aifsn;
		wmm_ie.ac[0].cw = edca->ac_be.ecw_min_max;
		wmm_ie.ac[0].txop_limit = cpu_to_le16(edca->ac_be.txop_limit);

		wmm_ie.ac[1].aci_aifsn = edca->ac_bk.aifsn;
		wmm_ie.ac[1].cw = edca->ac_bk.ecw_min_max;
		wmm_ie.ac[1].txop_limit = cpu_to_le16(edca->ac_bk.txop_limit);

		wmm_ie.ac[2].aci_aifsn = edca->ac_vi.aifsn;
		wmm_ie.ac[2].cw = edca->ac_vi.ecw_min_max;
		wmm_ie.ac[2].txop_limit = cpu_to_le16(edca->ac_vi.txop_limit);

		wmm_ie.ac[3].aci_aifsn = edca->ac_vo.aifsn;
		wmm_ie.ac[3].cw = edca->ac_vo.ecw_min_max;
		wmm_ie.ac[3].txop_limit = cpu_to_le16(edca->ac_vo.txop_limit);

		/* ieee80211_wmm_param_ie already contains both element_id and element_length as
		 * members. We do not want to insert those twice, so use the _no_header version.
		 */
		return morse_dot11_insert_ie_no_header(pos,
					(const u8 *)&wmm_ie,
					sizeof(struct ieee80211_wmm_param_ie));
	} else {
		return morse_dot11_insert_ie_no_header(pos,
					(const u8 *)&__wmm_ie,
					sizeof(struct ieee80211_wmm_param_ie));
	}
}

static u8 *morse_dot11_insert_ht_oper_ie(u8 *pos, struct ieee80211_rx_status *rxs,
					 struct dot11ah_ies_mask *ies_mask)
{
	struct s1g_operation_params_expanded s1g_oper_params;
	int pri_channel;
	int pri_1mhz_channel;
	int pri_ch_width_mhz;

	if (ies_mask->ies[WLAN_EID_HT_OPERATION].ptr)
		return morse_dot11_insert_ie_from_ies_mask(pos, ies_mask, WLAN_EID_HT_OPERATION);

	memset(&s1g_oper_params, 0, sizeof(s1g_oper_params));

	if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr)
		morse_dot11_s1g_oper_expand(ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr,
					    &s1g_oper_params);

	pri_channel = s1g_oper_params.pri_ch;

	pri_ch_width_mhz = (s1g_oper_params.op_bw > 1 && s1g_oper_params.primary_2mhz) ? 2 : 1;

	/* Regardless of the primary chan width (1 or 2MHz), set the HT primary channel
	 * corresponding to the 1MHz primary channel -> derived from the base primary
	 * channel and the lower/upper location of the 1MHz primary within the 2MHz block.
	 */
	pri_1mhz_channel = morse_dot11ah_get_pri_1mhz_chan(pri_channel,
		pri_ch_width_mhz, s1g_oper_params.upper_1mhz);

	/* TODO: Change this to rate-limited log function */
	if (pri_1mhz_channel <= 0)
		dot11ah_warn("%s: Primary 1MHz Channel %d is invalid\n",
			__func__, pri_1mhz_channel);

	__ht_oper_ie.primary_chan =
			morse_dot11ah_s1g_op_chan_pri_chan_to_5g(s1g_oper_params.op_ch,
								 pri_1mhz_channel);

	if (rxs)
		rxs->freq = ieee80211_channel_to_frequency(__ht_oper_ie.primary_chan,
							   rxs->band);

	if (s1g_oper_params.op_bw > 1) {
		if (s1g_oper_params.upper_1mhz)
			__ht_oper_ie.ht_param = IEEE80211_HT_PARAM_CHA_SEC_BELOW;
		else
			__ht_oper_ie.ht_param = IEEE80211_HT_PARAM_CHA_SEC_ABOVE;
		__ht_oper_ie.ht_param |= IEEE80211_HT_PARAM_CHAN_WIDTH_ANY;
	} else {
		__ht_oper_ie.ht_param = IEEE80211_HT_PARAM_CHA_SEC_NONE;
	}

	return morse_dot11_insert_ie(pos,
				    (const u8 *)&__ht_oper_ie,
				    WLAN_EID_HT_OPERATION,
				    sizeof(__ht_oper_ie));
}

static u8 *morse_dot11_insert_ht_and_vht_ie(u8 *pos, struct ieee80211_rx_status *rxs,
					    struct dot11ah_ies_mask *ies_mask)
{
	/* HT Capabilities / Operation conversion */
	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr)
		pos = morse_dot11_insert_ht_cap_ie(pos, ies_mask);
	if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr)
		pos = morse_dot11_insert_ht_oper_ie(pos, rxs, ies_mask);

	/* VHT Capabilities / Operation conversion */
	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr)
		pos = morse_dot11_insert_vht_cap_ie(pos, ies_mask);
	if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr)
		pos = morse_dot11_insert_vht_oper_ie(pos, rxs, ies_mask);

	return pos;
}

 /* Insert RSN and RSNX IE for the BSSID taken from CSSID list before
  * forwarding beacon to mac80211 or storing beacon IEs
  */
u8 *morse_dot11_insert_rsn_and_rsnx_ie(u8 *pos, struct dot11ah_update_rx_beacon_vals *vals,
		const struct dot11ah_ies_mask *ies_mask)
{
	const u8 *rsn_ie = morse_dot11_find_ie(WLAN_EID_RSN, vals->cssid_ies, vals->cssid_ies_len);
	const u8 *rsnx_ie = morse_dot11_find_ie(WLAN_EID_RSNX, vals->cssid_ies,
						vals->cssid_ies_len);

	if (!rsn_ie && !rsnx_ie)
		return pos;

	/* Insert RSN IE into beacon.
	 * Pass the pointer to insert the IE excluding IE id and length.
	 */
	if (rsn_ie && !ies_mask->ies[WLAN_EID_RSN].ptr)
		pos = morse_dot11_insert_ie(pos, rsn_ie + 2, WLAN_EID_RSN, *(rsn_ie + 1));

	/* Insert RSNX IE into beacon.
	 * Pass the pointer to insert the IE excluding IE id and length.
	 */
	if (rsnx_ie && !ies_mask->ies[WLAN_EID_RSNX].ptr)
		pos = morse_dot11_insert_ie(pos, rsnx_ie + 2, WLAN_EID_RSNX, *(rsnx_ie + 1));

	return pos;
}

static u8 *morse_dot11_insert_ssid_ie(u8 *pos, const struct dot11ah_ies_mask *ies_mask)
{
	if (ies_mask->ies[WLAN_EID_SSID].ptr)
		pos = morse_dot11_insert_ie_from_ies_mask(pos, ies_mask, WLAN_EID_SSID);
	else
		pos = morse_dot11_insert_ie(pos,
						IEEE80211AH_UNKNOWN_SSID,
						WLAN_EID_SSID,
						sizeof(IEEE80211AH_UNKNOWN_SSID));
	return pos;
}

static u8 *morse_dot11_insert_tim_ie(u8 *pos, const struct dot11ah_ies_mask *ies_mask)
{
	/*
	 * Allocate the max size of tim IE virtual map (actually 1 byte extra since tim_ie has
	 * virtual map array of 1 byte already).
	 */
	struct ieee80211_tim_ie *tim_ie = kzalloc(sizeof(*tim_ie) +
						  DOT11_MAX_TIM_VIRTUAL_MAP_LENGTH, GFP_ATOMIC);

	int length = sizeof(*tim_ie);

	if (!tim_ie)
		return pos;

	length = morse_dot11_s1g_to_tim(tim_ie,
		(const struct dot11ah_s1g_tim_ie *)ies_mask->ies[WLAN_EID_TIM].ptr,
		ies_mask->ies[WLAN_EID_TIM].len);

	pos = morse_dot11_insert_ie(pos, (const u8 *)tim_ie,
					WLAN_EID_TIM,
					length);

	kfree(tim_ie);

	return pos;
}

/* API's to convert the incoming S1G frames into 11n to pass to Linux */

/* Convert s1g listen interval to 11n listen interval. */
static u16 morse_dot11ah_s1g_to_listen_interval(u16 s1g_li)
{
	u16 usf = (s1g_li & IEEE80211_S1G_LI_USF) >> IEEE80211_S1G_LI_USF_SHIFT;
	u16 unscaled = s1g_li & IEEE80211_S1G_LI_UNSCALED_INTERVAL;
	u32 li = unscaled;

	switch (usf) {
	case IEEE80211_LI_USF_10:
		li *= 10;
	break;
	case IEEE80211_LI_USF_1000:
		li *= 1000;
	break;
	case IEEE80211_LI_USF_10000:
		li *= 10000;
	break;
	default:
		/* 1 */
		break;
	}

	if (li > U16_MAX)
		dot11ah_info("Listen interval > U16_MAX. Clip to max\n");

	/* clip if needed */
	return min_t(u16, li, U16_MAX);
}

static int morse_dot11_required_rx_ies_size(struct dot11ah_ies_mask *ies_mask,
	bool include_ht_vht, bool include_ssid, bool include_mesh_id, bool check_wmm)
{
	int ht_len = 0;
	int eid = 0;

	/* Supported rate will always be includes for all rx management frames */
	ies_mask->ies[WLAN_EID_SUPP_RATES].ptr = NULL;
	ht_len += sizeof(__s1g_supp_rates_ie) + 2;

	if (include_ht_vht) {
		ht_len += sizeof(struct ieee80211_ht_cap) + 2;
		ht_len += sizeof(struct ieee80211_ht_operation) + 2;
		if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr)
			ht_len += sizeof(struct ieee80211_vht_operation) + 2;
		if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr)
			ht_len += sizeof(struct ieee80211_vht_cap) + 2;
	}

	/* TODO: For now, assume TIM is 2 bytes (bitmap_ctrl & virt map). We need an s1g_to_tim_size
	 * API that just loops over the incoming S1G TIM's and calculate the needed size for 11n TIM
	 */
	for (eid = 0; eid < DOT11AH_MAX_EID; eid++) {
		struct ie_element *elem;

		if (ies_mask->ies[eid].ptr) {
			if (eid == WLAN_EID_S1G_OPERATION || eid == WLAN_EID_S1G_CAPABILITIES) {
				continue;
			} else if (!include_ssid && eid == WLAN_EID_SSID) {
				continue;
			} else if (!include_mesh_id && eid == WLAN_EID_MESH_ID) {
				continue;
			} else if (eid == WLAN_EID_S1G_BCN_COMPAT) {
				ht_len += sizeof(struct dot11ah_s1g_bcn_compat_ie) + 2;
			} else if (eid == WLAN_EID_S1G_SHORT_BCN_INTERVAL) {
				ht_len += sizeof(struct dot11ah_short_beacon_ie) + 2;
			} else if (eid == WLAN_EID_TIM && ies_mask->ies[WLAN_EID_TIM].ptr) {
				/* Allocate for max size tim. We will trim this down later */
				ht_len += sizeof(struct ieee80211_tim_ie) + 2 +
					  DOT11_MAX_TIM_VIRTUAL_MAP_LENGTH;

			} else if (check_wmm && eid == WLAN_EID_VENDOR_SPECIFIC) {
				if (ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC].ptr)
					ht_len += ies_mask->ies[eid].len + 2;
				else
					ht_len += sizeof(struct ieee80211_wmm_param_ie);
			} else {
				ht_len += (ies_mask->ies[eid].len + 2);
			}
			/* check for any extra elements with the same ID */
			for (elem = ies_mask->ies[eid].next; elem; elem = elem->next)
				ht_len += (elem->len + 2);
		}
	}
	return ht_len;
}

static u8 *morse_dot11ah_insert_required_rx_ie(struct dot11ah_ies_mask *ies_mask,
					       u8 *pos,
					       bool check_wmm)
{
	int eid = 0;

	/* Supported rate will always be included for all rx management frames */
	ies_mask->ies[WLAN_EID_SUPP_RATES].ptr = (u8 *)__s1g_supp_rates_ie;
	ies_mask->ies[WLAN_EID_SUPP_RATES].len = sizeof(__s1g_supp_rates_ie);

	for (eid = 0; eid < DOT11AH_MAX_EID; eid++) {
		if (ies_mask->ies[eid].ptr) {
			if (eid == WLAN_EID_S1G_OPERATION || eid == WLAN_EID_S1G_CAPABILITIES) {
				continue;
			} else if (check_wmm && eid == WLAN_EID_VENDOR_SPECIFIC) {
				if (ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC].ptr)
					pos = morse_dot11_insert_ie_from_ies_mask(pos, ies_mask,
										  eid);
				else
					pos = morse_dot11_insert_wmm_ie(pos, ies_mask);
			} else if (eid == WLAN_EID_TIM && ies_mask->ies[WLAN_EID_TIM].ptr) {
				pos = morse_dot11_insert_tim_ie(pos, ies_mask);
			} else {
				pos = morse_dot11_insert_ie_from_ies_mask(pos, ies_mask, eid);
			}
		}
	}
	return pos;
}

static void
morse_dot11ah_update_rx_beacon_elements(struct dot11ah_update_rx_beacon_vals *vals_to_update,
					struct dot11ah_ies_mask *ies_mask, u8 *bssid)
{
	struct morse_dot11ah_cssid_item *item = NULL;
	struct dot11ah_s1g_bcn_compat_ie *s1g_bcn_comp = (struct dot11ah_s1g_bcn_compat_ie *)
						ies_mask->ies[WLAN_EID_S1G_BCN_COMPAT].ptr;
	struct dot11ah_short_beacon_ie *s1g_short_bcn = (struct dot11ah_short_beacon_ie *)
						ies_mask->ies[WLAN_EID_S1G_SHORT_BCN_INTERVAL].ptr;

	/* Update Capab info from original beacon*/
	if (s1g_bcn_comp)
		vals_to_update->capab_info = s1g_bcn_comp->information;

	/* Extract beacon interval if exists */
	if (s1g_short_bcn)
		vals_to_update->bcn_int = s1g_short_bcn->short_beacon_int;
	else if (s1g_bcn_comp)
		vals_to_update->bcn_int = s1g_bcn_comp->beacon_interval;

	vals_to_update->tim_ie = ies_mask->ies[WLAN_EID_TIM].ptr;
	vals_to_update->tim_len = ies_mask->ies[WLAN_EID_TIM].len;

	spin_lock_bh(&cssid_list_lock);

	/* Try to find the CSSID item using source address and save a backup of IEs
	 * presumably stored from previous probe response or beacon, before it gets
	 * overwritten by IEs of incoming frame
	 */
	item = morse_dot11ah_find_bssid(bssid);

	if (item && item->ies) {
		vals_to_update->cssid_ies = kmalloc(item->ies_len, GFP_ATOMIC);

		if (vals_to_update->cssid_ies) {
			memcpy(vals_to_update->cssid_ies, item->ies, item->ies_len);
			vals_to_update->cssid_ies_len = item->ies_len;
		}
	}
	spin_unlock_bh(&cssid_list_lock);
}

static int morse_dot11ah_s1g_to_beacon_size(struct ieee80211_vif *vif, struct sk_buff *skb,
					    struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *beacon;
	struct ieee80211_ext *s1g_beacon = (struct ieee80211_ext *)skb->data;
	u8 *s1g_ies = s1g_beacon->u.s1g_beacon.variable;
	int header_length = s1g_ies - skb->data;
	int s1g_ies_len = skb->len - header_length;
	int beacon_len;
	struct morse_dot11ah_cssid_item *item = NULL;
	u8 *next_tbtt_ptr = NULL;
	u8 *ano_ptr = NULL;
	const u8 *rsn_ie = NULL;
	const u8 *rsnx_ie = NULL;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	u8 network_id_eid;
	const bool include_ht_vht = true;
	const bool include_ssid = false;
	const bool include_mesh_id = false;
	const bool check_wmm = false;

	/* Initially, the size equals to the 11n beacon header */
	beacon_len = sizeof(struct ieee80211_hdr_3addr) + sizeof(beacon->u.beacon);

	/* Some optional fields may appear before the variable IEs, make sure to account for this
	 * when determining the start of the IEs.
	 */
	if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_NEXT_TBTT) {
		next_tbtt_ptr = s1g_ies;
		s1g_ies += 3;
		s1g_ies_len -= 3;
	}

	if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_COMPRESS_SSID) {
		s1g_ies += 4;
		s1g_ies_len -= 4;
	}

	if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_ANO) {
		ano_ptr = s1g_ies;
		s1g_ies += 1;
		s1g_ies_len -= 1;
	}

	/* Mesh Beacons contains WLAN_EID_MESH_ID and wildcard SSID(length zero) IE.
	 * Use mesh EID instead of ssid EID.
	 */
	network_id_eid = morse_is_mesh_network(ies_mask) ? WLAN_EID_MESH_ID : WLAN_EID_SSID;

	spin_lock_bh(&cssid_list_lock);
	/* Try to find the CSSID item using source address */
	item = morse_dot11ah_find_bssid(s1g_beacon->u.s1g_beacon.sa);

	if (!item)
		spin_unlock_bh(&cssid_list_lock);

	if (!ies_mask->ies[network_id_eid].len) {
		if (item) {
			/* parse received beacons for any missing IEs */
			if (morse_dot11ah_parse_ies(item->ies, item->ies_len, ies_mask) < 0) {
				dot11ah_warn("Failed to parse missing IEs\n");
				dot11ah_hexdump_warn("IEs:", item->ies, item->ies_len);
				beacon_len = -EINVAL;
				goto exit;
			}
		} else {
			beacon_len = -EINVAL;
			goto exit;
		}
	}

	if (!ies_mask->ies[network_id_eid].len && item)
		beacon_len += item->ssid_len + 2;
	else
		beacon_len += sizeof(IEEE80211AH_UNKNOWN_SSID) + 2;

	beacon_len += morse_dot11_required_rx_ies_size(ies_mask,
		include_ht_vht, include_ssid, include_mesh_id, check_wmm);

	if (item) {
		rsn_ie = morse_dot11_find_ie(WLAN_EID_RSN, item->ies, item->ies_len);
		rsnx_ie = morse_dot11_find_ie(WLAN_EID_RSNX, item->ies, item->ies_len);

		/* Total RSN IE length including element id and length */
		if (rsn_ie)
			beacon_len += *(rsn_ie + 1) + 2;

		/* Total RSNX IE length including element id and length */
		if (rsnx_ie)
			beacon_len += *(rsnx_ie + 1) + 2;
	}

	/* Add size of secondary chan offset IE if ECSA IE is present & new op chan BW is 2MHz */
	if (ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr && mors_vif->is_sta_assoc &&
		ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].ptr) {
		const u8 *ie = cfg80211_find_ie(WLAN_EID_WIDE_BW_CHANNEL_SWITCH,
						ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].ptr,
						ies_mask->ies[WLAN_EID_CHANNEL_SWITCH_WRAPPER].len);

		if (ie) {
			struct ieee80211_wide_bw_chansw_ie *wbcsie =
				(struct ieee80211_wide_bw_chansw_ie *)(ie + 2);

			if ((wbcsie->new_channel_width & 0xF) == IEEE80211_S1G_CHANWIDTH_2MHZ)
				beacon_len += sizeof(struct ieee80211_sec_chan_offs_ie)	+ 2;
		}
	}
exit:
	if (item)
		spin_unlock_bh(&cssid_list_lock);

	/* NB: We do not need to strip out DS PARAMS, ERP INFO, or the Extended supported rates
	 * EID as we reconstruct the S1G beacon from scratch when we TX.
	 *
	 * This is unlike the Assoc Req/Resp & Probe Req/Resp where we add additional parameters to
	 * the frame sent by Linux.
	 */
	return beacon_len;
}

/*
 * Wide bandwidth channel switch IE handling
 */
static u8 *morse_convert_dot11ah_wide_bw_chansw_ie_to_5g(const u8 *ie, u8 *pos,
					    struct ieee80211_ext_chansw_ie *ecsa,
					    struct cfg80211_chan_def *chandef)
{
	struct ieee80211_wide_bw_chansw_ie *wbcsie = (struct ieee80211_wide_bw_chansw_ie *)ie;
	u8 pri_bw_mhz;
	bool pri_1mhz_loc_upper;
	u8 new_chan_width = wbcsie->new_channel_width;
	u8 op_chan_bw = IEEE80211AH_S1G_OPERATION_GET_OP_CHAN_BW(new_chan_width);
	u8 primary_1mhz_chan;

	/* Get primary channel bandwidth */
	pri_bw_mhz = IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_BW(new_chan_width);

	pri_1mhz_loc_upper = IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_LOC(new_chan_width);
	primary_1mhz_chan = morse_dot11ah_get_pri_1mhz_chan(ecsa->new_ch_num, pri_bw_mhz,
							pri_1mhz_loc_upper);

	dot11ah_info("%s: Primary Chan BW=%u, new_chan=%u, loc_upper=%u, 1mhz_chan=%u, ccfs0=%u\n",
					__func__, pri_bw_mhz, ecsa->new_ch_num,	pri_1mhz_loc_upper,
					primary_1mhz_chan, wbcsie->new_center_freq_seg0);
	/*
	 * Get 5GHz control channel based on S1G primary and operating channel.
	 * This is to handle overlapped channels in some countries, E.g Japan.
	 */
	ecsa->new_ch_num = morse_dot11ah_s1g_op_chan_pri_chan_to_5g(wbcsie->new_center_freq_seg0,
									primary_1mhz_chan);

	wbcsie->new_center_freq_seg0 =
				morse_dot11ah_s1g_chan_to_5g_chan(wbcsie->new_center_freq_seg0);
	chandef->center_freq1 = ieee80211_channel_to_frequency(wbcsie->new_center_freq_seg0,
							      NL80211_BAND_5GHZ);

	switch (op_chan_bw) {
	case 2:
		wbcsie->new_channel_width = IEEE80211_VHT_CHANWIDTH_USE_HT;
		/* Add secondary channel offset ie for operating bw 2 as
		 * current implementation in mac80211 calculates
		 * center_freq1 based on the secondary channel offset IE
		 * value and considers wide bw channel switch IE only for
		 * VHT 80 & 160MHz (S1G - 4 & 8MHz)
		 */
		*pos++ = WLAN_EID_SECONDARY_CHANNEL_OFFSET;
		*pos++ = sizeof(struct ieee80211_sec_chan_offs_ie);
		*pos++ = (wbcsie->new_center_freq_seg0 > ecsa->new_ch_num) ?
							IEEE80211_HT_PARAM_CHA_SEC_ABOVE :
							IEEE80211_HT_PARAM_CHA_SEC_BELOW;
		chandef->width = NL80211_CHAN_WIDTH_40;
		break;
	case 4:
		wbcsie->new_channel_width = IEEE80211_VHT_CHANWIDTH_80MHZ;
		chandef->width = NL80211_CHAN_WIDTH_80;
		break;
	case 8:
		/*
		 * As per IEEE 802.11-2020 std - Table 11-23—VHT BSS bandwidth:
		 * 1. new_channel_width value is 0 for 40 MHz, 1 for 80MHz, 160MHz and 80+80 MHz.
		 * 2. CCFS1 (Channel Center Frequency Segment 1) = 0 for 20/40/80 MHz bandwidths.
		 * 3. CCFS1 > 0 and |CCFS0 - CCFS1| = 8 for 160MHz.
		 */
		wbcsie->new_channel_width = IEEE80211_VHT_CHANWIDTH_80MHZ;
		chandef->width = NL80211_CHAN_WIDTH_160;
		wbcsie->new_center_freq_seg1 = wbcsie->new_center_freq_seg0;
		if (ecsa->new_ch_num < wbcsie->new_center_freq_seg0)
			wbcsie->new_center_freq_seg0 = wbcsie->new_center_freq_seg0 - 8;
		else
			wbcsie->new_center_freq_seg0 = wbcsie->new_center_freq_seg0 + 8;
		break;
	default:
		dot11ah_err("%s: IE New Channel width %d, not supported ?\n", __func__,
			(op_chan_bw & 0xF));
		break;
	}

	dot11ah_info("%s: 5GHz primary_ch=%d new_ch_width=%d, ccfs0=%d, ccfs1=%u\n",
		__func__,
		ecsa->new_ch_num,
		wbcsie->new_channel_width,
		wbcsie->new_center_freq_seg0,
		wbcsie->new_center_freq_seg1);

	return pos;
}

/* Convert ECSA Info to 5G and insert secondary channel offset IE if needed */
static u8 *morse_dot11ah_convert_ecsa_info_to_5g(u8 *frm_variable, u8 frm_variable_len, u8 *pos)
{
	const u8 *ie = NULL;
	struct ieee80211_ext_chansw_ie *ecsa = NULL;
	struct ieee80211_channel chan;
	struct cfg80211_chan_def chandef;
	u8 op_class_5g = OPERATING_CLASS_OF_5GHZ_CHANNEL_NUM_50;
	const u8 *wide_bw_chan_switch_ie = NULL;

	ie = cfg80211_find_ie(WLAN_EID_EXT_CHANSWITCH_ANN, frm_variable, frm_variable_len);

	/*
	 * Pointer returned by cfg80211_find_ie points to the element ID field of the IE.
	 * Increment it by the size of element ID(1 byte) and length (1byte) fields to point
	 * it to the data portion of the information element.
	 */
	if (ie)
		ecsa = (struct ieee80211_ext_chansw_ie *)(ie + 2);

	if (!ecsa)
		return pos;

	ie = cfg80211_find_ie(WLAN_EID_CHANNEL_SWITCH_WRAPPER, frm_variable,
					frm_variable_len);

	/*
	 * Get HT channel for s1g_primary_channel from channel switch wrapper IE.
	 */
	if (ie)
		wide_bw_chan_switch_ie = cfg80211_find_ie(WLAN_EID_WIDE_BW_CHANNEL_SWITCH,
									(ie + 2), ie[1]);

	chandef.chan = &chan;
	chandef.width = NL80211_CHAN_WIDTH_20_NOHT;

	if (wide_bw_chan_switch_ie) {
		struct ieee80211_wide_bw_chansw_ie *wbcsie;

		wbcsie = (struct ieee80211_wide_bw_chansw_ie *)(wide_bw_chan_switch_ie + 2);
		pos = morse_convert_dot11ah_wide_bw_chansw_ie_to_5g((u8 *)wbcsie, pos, ecsa,
								&chandef);
	} else {
		ecsa->new_ch_num = morse_dot11ah_s1g_chan_to_5g_chan(ecsa->new_ch_num);
	}

	dot11ah_info("%s: prim_chan updated=%u\n", __func__, ecsa->new_ch_num);
	chandef.chan->center_freq = ieee80211_channel_to_frequency(ecsa->new_ch_num,
									NL80211_BAND_5GHZ);

	if (chandef.width == NL80211_CHAN_WIDTH_20_NOHT)
		chandef.center_freq1 = ieee80211_channel_to_frequency(ecsa->new_ch_num,
								NL80211_BAND_5GHZ);

	/* Get operating class based on channel info */
	if (ieee80211_chandef_to_operating_class(&chandef, &op_class_5g)) {
		dot11ah_info("%s: op_class_5g=%d, op_class_s1g=%d, 5g-CHAN:[%d-%d-%d]\n",
			__func__,
			op_class_5g,
			ecsa->new_operating_class,
			chandef.width,
			chandef.chan->center_freq,
			chandef.center_freq1);
		ecsa->new_operating_class = op_class_5g;
	} else {
		morse_unii4_band_chan_to_op_class(&chandef, &op_class_5g);
		ecsa->new_operating_class = op_class_5g;
	}

	return pos;
}

static void morse_dot11ah_s1g_to_beacon(struct ieee80211_vif *vif, struct sk_buff *skb,
			int length_11n, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *beacon;
	struct ieee80211_ext *s1g_beacon = (struct ieee80211_ext *)skb->data;
	u8 *s1g_ies = s1g_beacon->u.s1g_beacon.variable;
	int header_length = s1g_ies - skb->data;
	int s1g_ies_len = skb->len - header_length;
	int beacon_len = length_11n;
	struct dot11ah_update_rx_beacon_vals updated_vals;
	struct ieee80211_rx_status *rxs = IEEE80211_SKB_RXCB(skb);
	struct morse_dot11ah_cssid_item *item = NULL;
	bool frame_good = false;
	u8 *pos = NULL;
	u8 *next_tbtt_ptr = NULL;
	u8 *ano_ptr = NULL;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	u8 network_id_eid;
	struct dot11ah_s1g_bcn_compat_ie *s1g_bcn_comp;
	struct dot11ah_short_beacon_ie *s1g_short_bcn;

	updated_vals.cssid_ies = NULL;
	updated_vals.cssid_ies_len = 0;
	updated_vals.capab_info = cpu_to_le16(WLAN_CAPABILITY_ESS);
	updated_vals.bcn_int = cpu_to_le16(100);

	if (beacon_len <= 0)
		goto exit;

	if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_NEXT_TBTT) {
		next_tbtt_ptr = s1g_ies;
		s1g_ies += 3;
		s1g_ies_len -= 3;
	}

	if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_COMPRESS_SSID) {
		s1g_ies += 4;
		s1g_ies_len -= 4;
	}

	if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_ANO) {
		ano_ptr = s1g_ies;
		s1g_ies += 1;
		s1g_ies_len -= 1;
	}

	if (morse_is_mesh_network(ies_mask)) {
		network_id_eid = WLAN_EID_MESH_ID;
		/* For mesh, both ESS and IBSS bits should be set to 0 */
		updated_vals.capab_info = 0;
	} else {
		network_id_eid = WLAN_EID_SSID;
	}

	/* Update Capab info from original beacon*/
	morse_dot11ah_update_rx_beacon_elements(&updated_vals, ies_mask,
						s1g_beacon->u.s1g_beacon.sa);

	/* Allocate beacon before spinlock section */
	beacon = kmalloc(beacon_len, GFP_KERNEL);
	if (!beacon)
		goto exit;

	memset(beacon, 0, beacon_len);
	frame_good = true;

	/* Store SSID or restore it */
	if (ies_mask->ies[network_id_eid].ptr) {
		morse_dot11ah_store_cssid(ies_mask, le16_to_cpu(updated_vals.capab_info),
				s1g_ies, s1g_ies_len, s1g_beacon->u.s1g_beacon.sa, &updated_vals);

		/* Fill in fc_bss_bw_subfield here, otherwise it will be
		 * always set to 255 when DTIM period is 1 (no short beacons)
		 */
		spin_lock_bh(&cssid_list_lock);
		item = morse_dot11ah_find_bssid(s1g_beacon->u.s1g_beacon.sa);
		if (item)
			item->fc_bss_bw_subfield =
				IEEE80211AH_GET_FC_BSS_BW(le16_to_cpu(s1g_beacon->frame_control));
		else
			spin_unlock_bh(&cssid_list_lock);
	} else {
		spin_lock_bh(&cssid_list_lock);
		/* Try to find the CSSID item using source address */
		item = morse_dot11ah_find_bssid(s1g_beacon->u.s1g_beacon.sa);

		if (item) {
			item->fc_bss_bw_subfield =
				IEEE80211AH_GET_FC_BSS_BW(le16_to_cpu(s1g_beacon->frame_control));
			/* Reparse for stored beacon */
			if (morse_dot11ah_parse_ies(item->ies, item->ies_len, ies_mask) < 0) {
				dot11ah_warn("Failed to parse stored beacon\n");
				dot11ah_hexdump_warn("IEs:", item->ies, item->ies_len);
			}

			/* If an SSID/Mesh ID element was not found, the beacon forwarded to
			 * mac80211 and wpa_supplicant will not have a valid SSID. Drop the
			 * frame to avoid generating the wrong scan results.
			 */
			if (!ies_mask->ies[network_id_eid].ptr) {
				frame_good = false;
				kfree(beacon);
				goto exit;
			}

			/* Overwrite history TIM with actual one */
			ies_mask->ies[WLAN_EID_TIM].ptr = (u8 *)updated_vals.tim_ie;
			ies_mask->ies[WLAN_EID_TIM].len = updated_vals.tim_len;
			/* Overwrite capab_info from stored */
			s1g_bcn_comp = (struct dot11ah_s1g_bcn_compat_ie *)
						ies_mask->ies[WLAN_EID_S1G_BCN_COMPAT].ptr;
			if (s1g_bcn_comp)
				updated_vals.capab_info = s1g_bcn_comp->information;
			else
				updated_vals.capab_info = cpu_to_le16(item->capab_info);
		} else {
			spin_unlock_bh(&cssid_list_lock);
		}
	}

	/* Overwrite bcn_int from stored */

	s1g_bcn_comp = (struct dot11ah_s1g_bcn_compat_ie *)
						ies_mask->ies[WLAN_EID_S1G_BCN_COMPAT].ptr;
	s1g_short_bcn = (struct dot11ah_short_beacon_ie *)
						ies_mask->ies[WLAN_EID_S1G_SHORT_BCN_INTERVAL].ptr;
	if (s1g_short_bcn)
		updated_vals.bcn_int = s1g_short_bcn->short_beacon_int;
	else if (s1g_bcn_comp)
		updated_vals.bcn_int = s1g_bcn_comp->beacon_interval;

	if (item) /* Update bcn interval in the cssid item */
		item->beacon_int = le16_to_cpu(updated_vals.bcn_int);

	beacon->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT) |
		cpu_to_le16(IEEE80211_STYPE_BEACON);
	eth_broadcast_addr(beacon->da);
	memcpy(beacon->sa, s1g_beacon->u.s1g_beacon.sa, ETH_ALEN);
	memcpy(beacon->bssid, s1g_beacon->u.s1g_beacon.sa, ETH_ALEN);

	/* Update capab_info and copy other fields */
	beacon->u.beacon.capab_info = updated_vals.capab_info;
	beacon->u.beacon.beacon_int = updated_vals.bcn_int;
	beacon->u.beacon.timestamp = cpu_to_le64(le32_to_cpu(s1g_beacon->u.s1g_beacon.timestamp));

	pos = beacon->u.beacon.variable;

	pos = morse_dot11_insert_ssid_ie(pos, ies_mask);
	/* NULL SSID pointer before calling `morse_dot11ah_insert_required_rx_ie()`.
	 * as it is already inserted with the function call
	 * `morse_dot11_insert_ssid_ie()` above. It is assumed that
	 * the ies mask after this point does not require the presence of
	 * the SSID ptr again.
	 */
	ies_mask->ies[WLAN_EID_SSID].ptr = NULL;

	pos = morse_dot11_insert_rsn_and_rsnx_ie(pos, &updated_vals, ies_mask);
	pos = morse_dot11_insert_ht_and_vht_ie(pos, rxs, ies_mask);
	pos = morse_dot11ah_insert_required_rx_ie(ies_mask, pos, false);

	if (ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr && mors_vif->is_sta_assoc)
		pos = morse_dot11ah_convert_ecsa_info_to_5g(beacon->u.beacon.variable,
							    (pos - beacon->u.beacon.variable), pos);

	/* Set the actual length. If everything went all right, this is redundant. */
	beacon_len = (pos - (u8 *)beacon);
	if (skb->len < beacon_len)
		skb_put(skb, beacon_len - skb->len);

	memcpy(skb->data, beacon, beacon_len);
	kfree(beacon);

	skb_trim(skb, beacon_len);
exit:
	if (item)
		spin_unlock_bh(&cssid_list_lock);

	/* Free the allocated IEs memory */
	kfree(updated_vals.cssid_ies);

	if (!frame_good)
		skb_trim(skb, 0);
}

static int morse_dot11ah_s1g_to_probe_req_size(struct ieee80211_vif *vif, struct sk_buff *skb,
					       struct dot11ah_ies_mask *ies_mask)
{
	const bool include_ht_vht = false;
	const bool include_ssid = false;
	const bool include_mesh_id = true;
	const bool check_wmm = false;
	struct ieee80211_mgmt *s1g_probe_req = (struct ieee80211_mgmt *)skb->data;
	u8 *s1g_ies = s1g_probe_req->u.probe_req.variable;
	int header_length = s1g_ies - skb->data;
	int probe_req_len;

	/* Initially, size equals incoming header length */
	probe_req_len = header_length;
	if (ies_mask->ies[WLAN_EID_SSID].ptr)
		probe_req_len += ies_mask->ies[WLAN_EID_SSID].len + 2;
	else
		/* Insert wild-card SSID (only EID and LEN=0) */
		probe_req_len += 2;

	probe_req_len += morse_dot11_required_rx_ies_size(ies_mask,
		include_ht_vht, include_ssid, include_mesh_id, check_wmm);

	return probe_req_len;
}

static void morse_dot11ah_s1g_to_probe_req(struct ieee80211_vif *vif, struct sk_buff *skb,
					   int length_11n, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *probe_req;
	struct ieee80211_mgmt *s1g_probe_req = (struct ieee80211_mgmt *)skb->data;
	u8 *s1g_ies = s1g_probe_req->u.probe_req.variable;
	int header_length = s1g_ies - skb->data;
	u8 *pos;
	bool frame_good = false;

	if (length_11n <= 0)
		goto exit;

	probe_req = kmalloc(length_11n, GFP_KERNEL);
	if (!probe_req)
		goto exit;

	memset(probe_req, 0, length_11n);
	frame_good = true;

	/* Fill in the new probe request header, copied from incoming frame */
	memcpy(probe_req, s1g_probe_req, header_length);

	pos = probe_req->u.probe_req.variable;

	if (ies_mask->ies[WLAN_EID_SSID].ptr)
		pos = morse_dot11_insert_ie_from_ies_mask(pos, ies_mask, WLAN_EID_SSID);

	else
		/* Insert wild-card SSID (only EID and LEN=0) */
		pos = morse_dot11_insert_ie(pos,
			NULL,
			WLAN_EID_SSID,
			0);

	/* No need to insert ssid as it has been inserted */
	ies_mask->ies[WLAN_EID_SSID].ptr = NULL;
	pos = morse_dot11ah_insert_required_rx_ie(ies_mask, pos, false);

	/* Set the actual length, if everything went alright this is redundant */
	length_11n = (pos - (u8 *)probe_req);
	if (skb->len < length_11n)
		skb_put(skb, length_11n - skb->len);

	memcpy(skb->data, probe_req, length_11n);
	kfree(probe_req);

	skb_trim(skb, length_11n);

exit:
	if (!frame_good)
		skb_trim(skb, 0);
}

static int morse_dot11ah_s1g_to_probe_resp_size(struct ieee80211_vif *vif, struct sk_buff *skb,
						struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *s1g_probe_resp = (struct ieee80211_mgmt *)skb->data;
	u8 *s1g_ies = s1g_probe_resp->u.probe_resp.variable;
	int header_length = s1g_ies - skb->data;
	int probe_resp_len;
	const bool include_ht_vht = true;
	const bool include_ssid = true;
	const bool include_mesh_id = true;
	const bool check_wmm = false;

	/* Initially, size equals incoming header length */
	probe_resp_len = header_length;

	probe_resp_len += morse_dot11_required_rx_ies_size(ies_mask,
		include_ht_vht, include_ssid, include_mesh_id, check_wmm);

	/* Note: The following parameters should be stripped if they exist, but in the current
	 * implementation we only insert the elements we are interested in. So by default they will
	 * not be added. For reference, WLAN_EID_EDCA_PARAM_SET
	 */

	return probe_resp_len;
}

static void morse_dot11ah_s1g_to_probe_resp(struct ieee80211_vif *vif, struct sk_buff *skb,
					    int length_11n, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *probe_resp;
	struct ieee80211_mgmt *s1g_probe_resp = (struct ieee80211_mgmt *)skb->data;
	struct ieee80211_rx_status *rxs = IEEE80211_SKB_RXCB(skb);
	u8 *s1g_ies = s1g_probe_resp->u.probe_resp.variable;
	int header_length = s1g_ies - skb->data;
	int	s1g_ies_len = skb->len - header_length;
	u8 *pos;
	u8 *da;
	bool frame_good = false;
	struct dot11ah_short_beacon_ie *s1g_short_bcn = (struct dot11ah_short_beacon_ie *)
						ies_mask->ies[WLAN_EID_S1G_SHORT_BCN_INTERVAL].ptr;

	if (length_11n <= 0)
		goto exit;

	/* The AP will respond to NDP probe requests with a broadcast
	 * probe response. For it to be considered by the upper layers,
	 * this VIF's address must be set as the da.
	 */
	da = ieee80211_get_DA((struct ieee80211_hdr *)skb->data);
	if (vif && vif->type == NL80211_IFTYPE_STATION &&
			is_broadcast_ether_addr(da))
		memcpy(da, vif->addr, ETH_ALEN);

	/* SW-2241: Restore short slot time bit for 80211g compatibility. */
	s1g_probe_resp->u.probe_resp.capab_info |= cpu_to_le16(WLAN_CAPABILITY_SHORT_SLOT_TIME);

	/* Create/Update the S1G IES for this cssid/bssid entry */
	morse_dot11ah_store_cssid(ies_mask,
				  le16_to_cpu(s1g_probe_resp->u.probe_resp.capab_info),
				  s1g_ies, s1g_ies_len, s1g_probe_resp->bssid, NULL);

	probe_resp = kmalloc(length_11n, GFP_KERNEL);
	if (!probe_resp)
		goto exit;

	memset(probe_resp, 0, length_11n);
	frame_good = true;

	/* Fill in the new probe request header, copied from incoming frame */
	memcpy(probe_resp, s1g_probe_resp, header_length);
	if (s1g_short_bcn)
		probe_resp->u.probe_resp.beacon_int = s1g_short_bcn->short_beacon_int;

	pos = probe_resp->u.probe_resp.variable;

	pos = morse_dot11ah_insert_required_rx_ie(ies_mask, pos, true);
	pos = morse_dot11_insert_ht_and_vht_ie(pos, rxs, ies_mask);

	/* Note: The following parameters should be stripped if they exist, but
	 * in the current implementation we only insert the elements we are
	 * interested in. So by default they will not be added. For reference,
	 * WLAN_EID_EDCA_PARAM_SET
	 */

	/* Set the actual length, if everything went alright this is redundant */
	length_11n = (pos - (u8 *)probe_resp);
	if (skb->len < length_11n)
		skb_put(skb, length_11n - skb->len);

	memcpy(skb->data, probe_resp, length_11n);
	kfree(probe_resp);

	skb_trim(skb, length_11n);

exit:
	if (!frame_good)
		skb_trim(skb, 0);
}

int morse_dot11ah_s1g_to_probe_resp_ies_size(struct dot11ah_ies_mask *ies_mask)
{
	const bool include_ht_vht = true;
	const bool check_wmm = true;
	const bool include_ssid = true;
	const bool include_mesh_id = true;

	return morse_dot11_required_rx_ies_size(ies_mask,
		include_ht_vht, include_ssid, include_mesh_id, check_wmm);
}
EXPORT_SYMBOL(morse_dot11ah_s1g_to_probe_resp_ies_size);

void morse_dot11ah_s1g_to_probe_resp_ies(u8 *ies_11n, int length_11n,
					 struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_rx_status *const rxs = NULL; /* We don't have a full skb here. */
	const bool check_wmm = true;
	u8 *pos;

	pos = ies_11n;
	pos = morse_dot11ah_insert_required_rx_ie(ies_mask, pos, check_wmm);
	pos = morse_dot11_insert_ht_and_vht_ie(pos, rxs, ies_mask);
	WARN_ON_ONCE(pos > ies_11n + length_11n);
}
EXPORT_SYMBOL(morse_dot11ah_s1g_to_probe_resp_ies);

static int morse_dot11ah_s1g_to_assoc_req_size(struct ieee80211_vif *vif, struct sk_buff *skb,
					       struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *s1g_assoc_req = (struct ieee80211_mgmt *)skb->data;
	u8 *s1g_ies = ieee80211_is_assoc_req(s1g_assoc_req->frame_control) ?
		s1g_assoc_req->u.assoc_req.variable :
		s1g_assoc_req->u.reassoc_req.variable;
	int header_length = s1g_ies - skb->data;
	int assoc_req_len;
	const bool include_ht_vht = true;
	const bool include_ssid = true;
	const bool include_mesh_id = true;
	const bool check_wmm = false;

	assoc_req_len = header_length; /* Initially, size equals the incoming header length */

	assoc_req_len += morse_dot11_required_rx_ies_size(ies_mask,
		include_ht_vht, include_ssid, include_mesh_id, check_wmm);
	assoc_req_len += ies_mask->fils_data_len;

	return assoc_req_len;
}

static void morse_dot11ah_s1g_to_assoc_req(struct ieee80211_vif *vif, struct sk_buff *skb,
					   int length_11n, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *assoc_req;
	struct ieee80211_mgmt *s1g_assoc_req = (struct ieee80211_mgmt *)skb->data;
	struct ieee80211_rx_status *rxs = IEEE80211_SKB_RXCB(skb);
	u8 *s1g_ies = ieee80211_is_assoc_req(s1g_assoc_req->frame_control) ?
		s1g_assoc_req->u.assoc_req.variable :
		s1g_assoc_req->u.reassoc_req.variable;
	int header_length = s1g_ies - skb->data;
	u8 *pos;
	bool frame_good = false;
	u16 s1g_li = ieee80211_is_assoc_req(s1g_assoc_req->frame_control) ?
		le16_to_cpu(s1g_assoc_req->u.assoc_req.listen_interval) :
		le16_to_cpu(s1g_assoc_req->u.reassoc_req.listen_interval);
	u16 li;

	if (length_11n <= 0)
		goto exit;

	assoc_req = kmalloc(length_11n, GFP_KERNEL);
	if (!assoc_req)
		goto exit;

	memset(assoc_req, 0, length_11n);
	frame_good = true;

	/* Fill in the new association request header, copied from incoming frame */
	memcpy(assoc_req, s1g_assoc_req, header_length);

	/* Update listen_interval to S1G */
	li = morse_dot11ah_s1g_to_listen_interval(s1g_li);
	if (ieee80211_is_assoc_req(assoc_req->frame_control))
		assoc_req->u.assoc_req.listen_interval = cpu_to_le16(li);
	else
		assoc_req->u.reassoc_req.listen_interval = cpu_to_le16(li);

	pos = ieee80211_is_assoc_req(assoc_req->frame_control) ?
		assoc_req->u.assoc_req.variable :
		assoc_req->u.reassoc_req.variable;

	pos = morse_dot11ah_insert_required_rx_ie(ies_mask, pos, false);
	pos = morse_dot11_insert_ht_and_vht_ie(pos, rxs, ies_mask);

	/* This must be last */
	if (ies_mask->fils_data)
		pos = morse_dot11_insert_ie_no_header(pos, ies_mask->fils_data,
								ies_mask->fils_data_len);

	/* Set the actual length, if everything went alright this is redundant */
	length_11n = (pos - (u8 *)assoc_req);
	if (skb->len < length_11n)
		skb_put(skb, length_11n - skb->len);

	memcpy(skb->data, assoc_req, length_11n);
	kfree(assoc_req);

	skb_trim(skb, length_11n);

exit:
	if (!frame_good)
		skb_trim(skb, 0);
}

static int morse_dot11ah_s1g_to_assoc_resp_size(struct ieee80211_vif *vif, struct sk_buff *skb,
						struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *assoc_resp;
	struct morse_dot11ah_s1g_assoc_resp *s1g_assoc_resp =
		(struct morse_dot11ah_s1g_assoc_resp *)skb->data;
	u8 *s1g_ies = s1g_assoc_resp->variable;
	int header_length = s1g_ies - skb->data;
	int assoc_resp_len;
	const bool include_ht_vht = true;
	const bool include_ssid = true;
	const bool include_mesh_id = true;
	const bool check_wmm = true;

	/* Initially, the size equals to the incoming header length */
	assoc_resp_len = header_length;

	/* AID is present in the HT header, but not in S1G header_length calculated above */
	assoc_resp_len += sizeof(assoc_resp->u.assoc_resp.aid);

	assoc_resp_len += morse_dot11_required_rx_ies_size(ies_mask,
		include_ht_vht, include_ssid, include_mesh_id, check_wmm);
	assoc_resp_len += ies_mask->fils_data_len;

	/* Note: The following parameters should be stripped if they exist, but
	 * in the current implementation we only insert the elements we are
	 * interested in. So by default they will not be added. For reference,
	 * WLAN_EID_EDCA_PARAM_SET
	 */
	return assoc_resp_len;
}

static void morse_dot11ah_s1g_to_assoc_resp(struct ieee80211_vif *vif, struct sk_buff *skb,
					    int length_11n, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *assoc_resp;
	struct morse_dot11ah_s1g_assoc_resp *s1g_assoc_resp =
		(struct morse_dot11ah_s1g_assoc_resp *)skb->data;
	struct ieee80211_rx_status *rxs = IEEE80211_SKB_RXCB(skb);
	u8 *s1g_ies = s1g_assoc_resp->variable;
	int header_length = s1g_ies - skb->data;
	u8 *pos;
	bool frame_good = false;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse_dot11ah_cssid_item *bssid_item = NULL;
	u8 *pri_bw_mhz = &mors_vif->custom_configs->channel_info.pri_bw_mhz;

	if (length_11n <= 0)
		goto exit;

	assoc_resp = kmalloc(length_11n, GFP_KERNEL);
	if (!assoc_resp)
		goto exit;

	memset(assoc_resp, 0, length_11n);
	frame_good = true;

	/* Fill in the new association request header, copied from incoming frame */
	memcpy(assoc_resp, s1g_assoc_resp, header_length);
	assoc_resp->u.assoc_resp.aid =
		cpu_to_le16(*((u16 *)&ies_mask->ies[WLAN_EID_AID_RESPONSE].ptr[0]));

	spin_lock_bh(&cssid_list_lock);
	bssid_item = morse_dot11ah_find_bssid(assoc_resp->bssid);

	if (bssid_item && MORSE_IS_FC_BSS_BW_SUBFIELD_VALID(bssid_item->fc_bss_bw_subfield)) {
		*pri_bw_mhz = s1g_fc_bss_bw_lookup_min[bssid_item->fc_bss_bw_subfield];
	} else {
		/* The min bss bw is == s1g op pri bw, if we don't have that then use 1MHz */
		if (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr) {
			*pri_bw_mhz = (ies_mask->ies[WLAN_EID_S1G_OPERATION].ptr[0]
				      & S1G_OPER_CH_WIDTH_PRIMARY_1MHZ) ? 1 : 2;
		} else {
			dot11ah_warn("Could not set bss primary bw, default to 1MHz\n");
			*pri_bw_mhz = 1;
		}
	}
	spin_unlock_bh(&cssid_list_lock);

	pos = assoc_resp->u.assoc_resp.variable;
	pos = morse_dot11ah_insert_required_rx_ie(ies_mask, pos, true);
	pos = morse_dot11_insert_ht_and_vht_ie(pos, rxs, ies_mask);

	/* This must be last */
	if (ies_mask->fils_data)
		pos = morse_dot11_insert_ie_no_header(pos, ies_mask->fils_data,
								ies_mask->fils_data_len);

	/* Note: The following parameters should be stripped if they exist, but
	 * in the current implementation we only insert the elements we are
	 * interested in. So by default they will not be added. For reference,
	 * WLAN_EID_EDCA_PARAM_SET
	 */

	/* Set the actual length, if everything went alright this is redundant */
	length_11n = (pos - (u8 *)assoc_resp);
	if (skb->len < length_11n)
		skb_put(skb, length_11n - skb->len);

	memcpy(skb->data, assoc_resp, length_11n);
	kfree(assoc_resp);

	skb_trim(skb, length_11n);

exit:
	if (!frame_good)
		skb_trim(skb, 0);
}

#if !defined(MORSE_MAC80211_S1G_FEATURE_NDP_BLOCKACK)
static void morse_dot11ah_s1g_to_blockack(struct ieee80211_vif *vif, struct sk_buff *skb)
{
	struct ieee80211_mgmt *back = (struct ieee80211_mgmt *)skb->data;
	/* Firmware leaves the CCMP header in place, so let's offset by that */
	if (ieee80211_has_protected(back->frame_control))
		back = (struct ieee80211_mgmt *)(skb->data + IEEE80211_CCMP_HDR_LEN);

	switch (back->u.action.u.addba_req.action_code) {
	case WLAN_ACTION_NDP_ADDBA_REQ:
		back->u.action.u.addba_req.action_code = WLAN_ACTION_ADDBA_REQ;
		break;
	case WLAN_ACTION_NDP_ADDBA_RESP:
		back->u.action.u.addba_req.action_code = WLAN_ACTION_ADDBA_RESP;
		break;
	case WLAN_ACTION_NDP_DELBA:
		back->u.action.u.addba_req.action_code = WLAN_ACTION_DELBA;
		break;
	default:
		break;
	}
}
#endif

/* Calculate the size of Mesh Peering Management (MPM) frame size Including HT/VHT IEs */
static int morse_dot11ah_s1g_to_mpm_frame_size(struct ieee80211_vif *vif,
			struct sk_buff *skb,
			struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *s1g_mpm_frm = (struct ieee80211_mgmt *)skb->data;
	u8 *s1g_ies = morse_dot11_mpm_frame_ies(s1g_mpm_frm);
	int header_length = s1g_ies - skb->data;
	int s1g_ies_len = skb->len - header_length;
	int action_frame_len;
	int ampe_len;
	const bool include_ht_vht = true;
	const bool include_ssid = true;
	const bool include_mesh_id = true;
	const bool check_wmm = false;

	action_frame_len = header_length;

	ampe_len = morse_dot11_get_mpm_ampe_len(skb);

	/* Note: Supplicant adds AMPE (Authenticated Mesh Peering Exchange) block, which is
	 * encrypted IE after the MIC IE. AMPE IE block is keys data encrypted and added at the end
	 * of frame just after MIC IE. Since AMPE is not in Std IE format, parsing of ies will throw
	 * an error. Reduce the AMPE Len to avoid IE parsing error and restore after parsing.
	 */
	s1g_ies_len -= ampe_len;

	action_frame_len += morse_dot11_required_rx_ies_size(ies_mask,
		include_ht_vht, include_ssid, include_mesh_id, check_wmm);
	/* Add the AMPE element length to the frame length */
	action_frame_len += ampe_len;

	dot11ah_debug("MPM Rx Frame: AMPE len: %d frame len %d\n", ampe_len, action_frame_len);

	return action_frame_len;
}

/* Convert Mesh Peering Management (MPM) frame to 11n i.e. Remove S1G IEs & add HT/VHT IEs */
static void morse_dot11ah_s1g_to_mpm_frame(struct ieee80211_vif *vif, struct sk_buff *skb,
					   int length_11n, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *mesh_peering_frame;
	struct ieee80211_mgmt *s1g_mesh_peering_frame = (struct ieee80211_mgmt *)skb->data;
	struct ieee80211_rx_status *rxs = IEEE80211_SKB_RXCB(skb);
	u8 *s1g_ies = morse_dot11_mpm_frame_ies(s1g_mesh_peering_frame);
	int header_length = s1g_ies - skb->data;
	int s1g_ies_len = skb->len - header_length;
	u8 *pos;
	bool frame_good = false;
	int ampe_len;
	u8 *mic_ie;

	/* Verify min size: action frame size + action code(1byte) + capab info(2bytes) */
	if (length_11n <= 0 || length_11n < IEEE80211_MIN_ACTION_SIZE + 3)
		goto exit;

	ampe_len = morse_dot11_get_mpm_ampe_len(skb);
	/* Reduce the AMPE Len to avoid IE parsing error and restore after parsing. See the note
	 * in  morse_dot11ah_s1g_to_mpm_frame_size
	 */
	s1g_ies_len -= ampe_len;

	mesh_peering_frame = kmalloc(length_11n, GFP_KERNEL);
	if (!mesh_peering_frame)
		goto exit;

	memset(mesh_peering_frame, 0, length_11n);
	frame_good = true;

	/* Fill in the header from incoming frame */
	memcpy(mesh_peering_frame, s1g_mesh_peering_frame, header_length);

	/* Get the starting address of information elements */
	pos = morse_dot11_mpm_frame_ies(mesh_peering_frame);

	/* Store the MIC IE ptr and make it null to skip adding in insert_required_rx_ie */
	if (ampe_len) {
		mic_ie = ies_mask->ies[WLAN_EID_MIC].ptr;
		ies_mask->ies[WLAN_EID_MIC].ptr = NULL;
	}
	/* Drop Vendor IE from Mesh Action frames, as it's internal to the driver */
	ies_mask->ies[WLAN_EID_VENDOR_SPECIFIC].ptr = NULL;

	pos = morse_dot11ah_insert_required_rx_ie(ies_mask, pos, false);
	pos = morse_dot11_insert_ht_and_vht_ie(pos, rxs, ies_mask);

	if (ampe_len) {
		/* Restore MIC IE pointer */
		ies_mask->ies[WLAN_EID_MIC].ptr = mic_ie;
		/* Insert MIC */
		pos = morse_dot11_insert_ie(pos, ies_mask->ies[WLAN_EID_MIC].ptr, WLAN_EID_MIC,
					ies_mask->ies[WLAN_EID_MIC].len);
		/* Insert AMPE */
		memcpy(pos, s1g_ies + s1g_ies_len, ampe_len);
		pos += ampe_len;
	}

	/* Set the actual length, if everything went alright this is redundant */
	length_11n = (pos - (u8 *)mesh_peering_frame);
	if (skb->len < length_11n)
		skb_put(skb, length_11n - skb->len);

	memcpy(skb->data, mesh_peering_frame, length_11n);
	kfree(mesh_peering_frame);

	skb_trim(skb, length_11n);
exit:
	if (!frame_good)
		skb_trim(skb, 0);
}

int morse_dot11ah_s1g_to_11n_rx_packet_size(struct ieee80211_vif *vif, struct sk_buff *skb,
					    struct dot11ah_ies_mask *ies_mask)
{
	int size;
	struct ieee80211_hdr *hdr;

	hdr = (struct ieee80211_hdr *)skb->data;
	size = skb->len + skb_tailroom(skb);

	if (ieee80211_is_s1g_beacon(hdr->frame_control))
		size = morse_dot11ah_s1g_to_beacon_size(vif, skb, ies_mask);
	else if (ieee80211_is_assoc_req(hdr->frame_control) ||
		ieee80211_is_reassoc_req(hdr->frame_control))
		size = morse_dot11ah_s1g_to_assoc_req_size(vif, skb, ies_mask);
	else if (ieee80211_is_assoc_resp(hdr->frame_control) ||
		ieee80211_is_reassoc_resp(hdr->frame_control))
		size = morse_dot11ah_s1g_to_assoc_resp_size(vif, skb, ies_mask);
	else if (ieee80211_is_probe_req(hdr->frame_control))
		size = morse_dot11ah_s1g_to_probe_req_size(vif, skb, ies_mask);
	else if (ieee80211_is_probe_resp(hdr->frame_control))
		size = morse_dot11ah_s1g_to_probe_resp_size(vif, skb, ies_mask);
	else if (ieee80211_is_action(hdr->frame_control) &&
		morse_dot11_is_mpm_frame((struct ieee80211_mgmt *)hdr))
		size = morse_dot11ah_s1g_to_mpm_frame_size(vif, skb, ies_mask);

	return size;
}
EXPORT_SYMBOL(morse_dot11ah_s1g_to_11n_rx_packet_size);

void morse_dot11ah_s1g_to_11n_rx_packet(struct ieee80211_vif *vif, struct sk_buff *skb,
					int length_11n, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_hdr *hdr;

	if (length_11n < 0)
		length_11n = 0; /* There was an error parsing the packet */

	hdr = (struct ieee80211_hdr *)skb->data;

	if (ieee80211_is_action(hdr->frame_control)) {
		struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)hdr;
		/* Firmware leaves the CCMP header in place, so let's offset by that */
		if (ieee80211_has_protected(hdr->frame_control))
			mgmt = (struct ieee80211_mgmt *)(skb->data + IEEE80211_CCMP_HDR_LEN);

		if (morse_dot11_is_mpm_frame(mgmt))
			morse_dot11ah_s1g_to_mpm_frame(vif, skb, length_11n, ies_mask);
#if !defined(MORSE_MAC80211_S1G_FEATURE_NDP_BLOCKACK)
		else if (mgmt->u.action.category == WLAN_CATEGORY_BACK)
			morse_dot11ah_s1g_to_blockack(vif, skb);
#endif
	}
	if (ieee80211_is_s1g_beacon(hdr->frame_control))
		morse_dot11ah_s1g_to_beacon(vif, skb, length_11n, ies_mask);
	else if (ieee80211_is_assoc_req(hdr->frame_control) ||
		ieee80211_is_reassoc_req(hdr->frame_control))
		morse_dot11ah_s1g_to_assoc_req(vif, skb, length_11n, ies_mask);
	else if (ieee80211_is_assoc_resp(hdr->frame_control) ||
		ieee80211_is_reassoc_resp(hdr->frame_control))
		morse_dot11ah_s1g_to_assoc_resp(vif, skb, length_11n, ies_mask);
	else if (ieee80211_is_probe_req(hdr->frame_control))
		morse_dot11ah_s1g_to_probe_req(vif, skb, length_11n, ies_mask);
	else if (ieee80211_is_probe_resp(hdr->frame_control))
		morse_dot11ah_s1g_to_probe_resp(vif, skb, length_11n, ies_mask);

	skb_trim(skb, length_11n);
}
EXPORT_SYMBOL(morse_dot11ah_s1g_to_11n_rx_packet);
