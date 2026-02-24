#ifndef _IEEE80211_H_
#define _IEEE80211_H_

#include <linux/version.h>

#ifdef MAC80211_BACKPORT_VERSION_CODE
#define MAC80211_VERSION_CODE MAC80211_BACKPORT_VERSION_CODE
#else
#define MAC80211_VERSION_CODE LINUX_VERSION_CODE
#endif

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

/*
 * This should include all modified/missing bits from ieee80211
 */

/** As per IEEE-802.11-2020 Table 9-155, default TXOP for S1G is 15.008 msecs */
#define S1G_WMM_DEFAULT_TXOP_USECS	(15008)

/* EDCA IE and access category parameters */
struct __ieee80211_edca_ac_rec {
	u8 aifsn;
	u8 ecw_min_max;
	u16 txop_limit;
} __packed;

struct __ieee80211_edca_ie {
	u8 wme_qos_info;
	u8 update_edca_info;
	struct __ieee80211_edca_ac_rec ac_be;
	struct __ieee80211_edca_ac_rec ac_bk;
	struct __ieee80211_edca_ac_rec ac_vi;
	struct __ieee80211_edca_ac_rec ac_vo;
} __packed;

/**
 * VENDOR IE data format for easy type casting
 */
struct __ieee80211_vendor_ie_elem {
	u8 oui[3];
	u8 oui_type;
	u8 oui_sub_type;
	u8 attr[];
} __packed;

#define IS_WMM_IE(_ven_ie) \
	((_ven_ie)->oui[0] == 0x00 && \
	 (_ven_ie)->oui[1] == 0x50 && \
	 (_ven_ie)->oui[2] == 0xf2 && \
	 (_ven_ie)->oui_type == 2 && \
	 (_ven_ie)->oui_sub_type == 1)

#if KERNEL_VERSION(5, 10, 0) > MAC80211_VERSION_CODE
/**
 * enum ieee80211_s1g_chanwidth
 * These are defined in IEEE802.11-2016ah Table 10-20
 * as BSS Channel Width
 *
 * @IEEE80211_S1G_CHANWIDTH_1MHZ: 1MHz operating channel
 * @IEEE80211_S1G_CHANWIDTH_2MHZ: 2MHz operating channel
 * @IEEE80211_S1G_CHANWIDTH_4MHZ: 4MHz operating channel
 * @IEEE80211_S1G_CHANWIDTH_8MHZ: 8MHz operating channel
 * @IEEE80211_S1G_CHANWIDTH_16MHZ: 16MHz operating channel
 */
enum ieee80211_s1g_chanwidth {
	IEEE80211_S1G_CHANWIDTH_1MHZ = 0,
	IEEE80211_S1G_CHANWIDTH_2MHZ = 1,
	IEEE80211_S1G_CHANWIDTH_4MHZ = 3,
	IEEE80211_S1G_CHANWIDTH_8MHZ = 7,
	IEEE80211_S1G_CHANWIDTH_16MHZ = 15,
};
#endif

/* PV1 standard suggests the bit to be From DS unlike the
 * kernel definition of IEEE80211_PV1_FCTL_TODS
 */
#define IEEE80211_PV1_FCTL_FROMDS          0x0100

#if KERNEL_VERSION(5, 8, 0) > MAC80211_VERSION_CODE
/**
 * struct ieee80211_channel - channel definition
 *
 * The new ieee80211_channel from K5.8 supporting S1G.
 */
struct ieee80211_channel_s1g {
	enum nl80211_band band;
	u32 center_freq;
	u16 freq_offset;
	u16 hw_value;
	u32 flags;
	int max_antenna_gain;
	int max_power; /* Units: mBm */
	int max_reg_power; /* Units: mBm */
	bool beacon_found;
	u32 orig_flags;
	int orig_mag, orig_mpwr;
	enum nl80211_dfs_state dfs_state;
	unsigned long dfs_state_entered;
	unsigned int dfs_cac_ms;
};

#define IEEE80211_STYPE_S1G_BEACON	0x0010

#define IEEE80211_S1G_BCN_NEXT_TBTT	0x100

/* convert frequencies */
#define MHZ_TO_KHZ(freq) ((freq) * 1000)
#define KHZ_TO_MHZ(freq) ((freq) / 1000)

/**
 * ieee80211_is_ext - check if type is IEEE80211_FTYPE_EXT
 * @fc: frame control bytes in little-endian byteorder
 */
static inline int ieee80211_is_ext(__le16 fc)
{
	return (fc & cpu_to_le16(IEEE80211_FCTL_FTYPE)) ==
		   cpu_to_le16(IEEE80211_FTYPE_EXT);
}

/**
 * ieee80211_is_s1g_beacon - check if type is S1G Beacon
 * @fc: frame control bytes in little-endian byteorder
 */
static inline int ieee80211_is_s1g_beacon(__le16 fc)
{
	return (ieee80211_is_ext(fc) &&
		((fc & cpu_to_le16(IEEE80211_FCTL_STYPE)) ==
			cpu_to_le16(IEEE80211_STYPE_S1G_BEACON)));
}

/**
 * ieee80211_channel_to_khz - convert ieee80211_channel to frequency in KHz
 * @chan: struct ieee80211_channel to convert
 * Return: The corresponding frequency (in KHz)
 */
static inline u32
ieee80211_channel_to_khz(const struct ieee80211_channel_s1g *chan)
{
	return MHZ_TO_KHZ(chan->center_freq) + chan->freq_offset;
}

struct ieee80211_s1g_cap {
	u8 capab_info[10];
	u8 supp_mcs_nss[5];
} __packed;
#else

#define ieee80211_channel_s1g ieee80211_channel

#endif

/* These structures are already in K5.10, lets use them */
#if KERNEL_VERSION(5, 10, 11) > MAC80211_VERSION_CODE

#define ieee80211_ext mm_ieee80211_ext
struct mm_ieee80211_ext {
	__le16 frame_control;
	__le16 duration;
	union {
		struct {
			u8 sa[ETH_ALEN];
			__le32 timestamp;
			u8 change_seq;
			u8 variable[];
		} __packed s1g_beacon;
		struct {
			u8 sa[ETH_ALEN];
			__le32 timestamp;
			u8 change_seq;
			u8 next_tbtt[3];
			u8 variable[];
		} __packed s1g_short_beacon;
	} u;
} __packed __aligned(2);

/**
 * enum morse_dot11ah_channel_flags - channel flags
 *
 * These are in Linux 5.10, should be disabled
 *
 * @IEEE80211_CHAN_1MHZ: 1 MHz bandwidth is permitted
 *	on this channel.
 * @IEEE80211_CHAN_2MHZ: 2 MHz bandwidth is permitted
 *	on this channel.
 * @IEEE80211_CHAN_4MHZ: 4 MHz bandwidth is permitted
 *	on this channel.
 * @IEEE80211_CHAN_8MHZ: 8 MHz bandwidth is permitted
 *	on this channel.
 * @IEEE80211_CHAN_16MHZ: 16 MHz bandwidth is permitted
 *	on this channel.
 *
 */
enum morse_dot11ah_channel_flags {
	IEEE80211_CHAN_1MHZ		= BIT(14),
	IEEE80211_CHAN_2MHZ		= BIT(15),
	IEEE80211_CHAN_4MHZ		= BIT(16),
	IEEE80211_CHAN_8MHZ		= BIT(17),
	IEEE80211_CHAN_16MHZ		= BIT(18),
};

/**
 * ieee80211_next_tbtt_present - check if IEEE80211_FTYPE_EXT &&
 * IEEE80211_STYPE_S1G_BEACON && IEEE80211_S1G_BCN_NEXT_TBTT
 * @fc: frame control bytes in little-endian byteorder
 */
static inline bool ieee80211_next_tbtt_present(__le16 fc)
{
	return (fc & cpu_to_le16(IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
	       cpu_to_le16(IEEE80211_FTYPE_EXT | IEEE80211_STYPE_S1G_BEACON) &&
	       fc & cpu_to_le16(IEEE80211_S1G_BCN_NEXT_TBTT);
}

/**
 * ieee80211_is_s1g_short_beacon - check if next tbtt present bit is set. Only
 * true for S1G beacons when they're short.
 * @fc: frame control bytes in little-endian byteorder
 */
static inline bool ieee80211_is_s1g_short_beacon(__le16 fc)
{
	return ieee80211_is_s1g_beacon(fc) && ieee80211_next_tbtt_present(fc);
}

#endif

#if KERNEL_VERSION(4, 12, 0) > MAC80211_VERSION_CODE
struct ieee80211_bss_max_idle_period_ie {
	__le16 max_idle_period;
	u8 idle_options;
} __packed;
#endif

#if KERNEL_VERSION(5, 10, 0) > MAC80211_VERSION_CODE
struct ieee80211_mgmt_s1g {
	__le16 frame_control;
	__le16 duration;
	u8 da[ETH_ALEN];
	u8 sa[ETH_ALEN];
	u8 bssid[ETH_ALEN];
	__le16 seq_ctrl;
	union {
		struct {
			__le16 capab_info;
			__le16 status_code;
			u8 variable[];
		} __packed s1g_assoc_resp, s1g_reassoc_resp;
	} u;
} __packed __aligned(2);
#else

#define ieee80211_mgmt_s1g ieee80211_mgmt

#endif

/*
 * This function need to be patched for Linux 5.10, so bring it here for now.
 */

/**
 * ieee80211_freq_khz_to_channel - convert frequency to channel number
 * @freq: center frequency in KHz
 * Return: The corresponding channel, or 0 if the conversion failed.
 */
int __ieee80211_freq_khz_to_channel(u32 freq);

/**
 * morse_unii4_band_chan_to_op_class : Returns operating class for U-NII 4 band channel.
 *
 * @chandef: The channel definition.
 * @op_class: Operating class for given channel definition.
 *
 * @note: This function is implemented temporarily until support for U-NII 4 band is added in
 * cfg80211.
 */
void morse_unii4_band_chan_to_op_class(struct cfg80211_chan_def *chandef, u8 *op_class);

/*
 * Below function is an extension. This needs to be patched on 5.10 kernel.
 * Kernel func in include/linux/ieee80211.h is checking only for the presence
 * of next TBTT bit in frame control, whereas compressed SSID is more acccurate
 * flag to check if it's a short beacon. For now we will use below function.
 */
#define IEEE80211_FCTL_COMPR_SSID	0x0200
/**
 * ieee80211_is_s1g_short_beacon_local - check if type is S1G Beacon is short beacon
 * @fc: frame control bytes in little-endian byteorder
 */
static inline int ieee80211_is_s1g_short_beacon_local(__le16 fc)
{
	return (ieee80211_is_s1g_beacon(fc) &&
		((fc & cpu_to_le16(IEEE80211_FCTL_COMPR_SSID)) != 0));
}

#endif  /* !_IEEE80211_H_ */
