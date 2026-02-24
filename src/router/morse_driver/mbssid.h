#ifndef _MBSSID_H_
#define _MBSSID_H_
/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <linux/types.h>
#include <linux/ieee80211.h>

#include "debug.h"
#include "morse.h"

#define MBSSID_IE_SIZE_MAX 255

#define MBSSID_SUBELEMENT_NONTX_BSSID_PROFILE 0

#if KERNEL_VERSION(5, 1, 0) >= MAC80211_VERSION_CODE
struct ieee80211_bssid_index {
	u8 bssid_index;
	u8 dtim_period;
	u8 dtim_count;
};
#endif

/**
 * struct sub_elem_ssid_ie - SSID IE definition
 *
 * @element_id:  ID for SSID IE
 * @len:         SSID len
 * @ssid:        SSID of this BSS
 */
struct sub_elem_ssid_ie {
	u8 element_id;
	u8 len;
	u8 ssid[];
} __packed;

/**
 * struct sub_elem_mbssid_idx_ie - MBSSID Index IE definition
 *
 * @element_id:     ID for Index IE
 * @len:            IE len
 * @mbssid_index:   MBSSID Index IE elements
 */
struct sub_elem_mbssid_idx_ie {
	u8 element_id;
	u8 len;
	struct ieee80211_bssid_index mbssid_index;
} __packed;

/**
 * struct mbssid_subelement - MBSSID Subelement IE definition
 *        This IE contains all the subelements like SSID, MBSSID Index IE representing
 *        non-transmitting BSS
 *
 * @element_id:   Multiple BSSID element id
 * @len:          Length of all IEs in this subelement
 * @ssid_ie:      SSID IE
 * @idx_ie:       MBSSID Index IE
 */
struct mbssid_subelement {
	u8 element_id;
	u8 len;
	struct sub_elem_ssid_ie ssid_ie;
	struct sub_elem_mbssid_idx_ie idx_ie;
} __packed;

/**
 * struct mbssid_ie - MBSSID IE definition
 *         The full MBSSID IE. Has only one subelement now representing one non-transmitting BSS
 *
 * @max_bssid_indicator:    Max number of non-transmitting bss
 * @sub_elem:               Non-transmitting Subelement IE
 */
struct mbssid_ie {
	u8 max_bssid_indicator;
	struct mbssid_subelement sub_elem;
} __packed;

/**
 * morse_mbssid_insert_ie - Insert MBSSID IE of non-transmitting BSS to the beacon of
 *                          transmitting BSS
 *
 * @mors_vif:    Transmitting AP iface
 * @mors:       The global Morse structure
 * @ies_mask:   IE mask pointer
 */
void morse_mbssid_insert_ie(struct morse_vif *mors_vif, struct morse *mors,
			    struct dot11ah_ies_mask *ies_mask);

/**
 * morse_mac_get_mbssid_beacon_ies - Saves IEs from beacon of non-transmitting iface
 *                                   to be filled in MBSSID IE in morse iface context
 * @mors_vif:   AP iface
 *
 * Return:  Beacon skb if got from mac80211, otherwise NULL
 */
struct sk_buff *morse_mac_get_mbssid_beacon_ies(struct morse_vif *mors_vif);

/**
 * morse_process_beacon_from_mbssid_ie - Process RX beacon with MBSSID IE on STA
 *             Makes a copy of receiving beacon skb, updates the bssid and SSID
 *             values based on subelements available in MBSSID IE and passes
 *             the frame to mac80211
 *
 * @morse:          The global Morse structure
 * @skb:            RX Beacon SKB
 * @ies_mask:       IE mask pointer
 * @vif:            Valid STA vif
 * @hdr_rx_status:  morse RX status buffer
 *
 * Return:  0 on success
 */
int morse_process_beacon_from_mbssid_ie(struct morse *mors, struct sk_buff *skb,
					struct dot11ah_ies_mask *ies_mask,
					struct ieee80211_vif *vif,
					const struct morse_skb_rx_status *hdr_rx_status);

/**
 * morse_command_process_bssid_info - Process morsectrl command coming from hostapd to decide
 *                                    which is transmitting and non-transmitting iface.
 *
 * @mors_vif:    AP iface
 * @req_mbssid: morsectrl command context
 *
 * Return:  0 on success, -EFAULT on failure
 */
int morse_command_process_bssid_info(struct morse_vif *mors_vif,
				     struct morse_cmd_req_mbssid *req_mbssid);
/**
 * morse_mbssid_ie_deinit_bss - Deinitialise MBSSID IE context buffer on this interface.
 *                              Free up beacon skb allocated from mac80211
 *
 * @mors_vif:    AP iface
 * @mors:       The global Morse structure
 *
 * Return:  0 on success, else relevant error
 */
int morse_mbssid_ie_deinit_bss(struct morse *mors, struct morse_vif *mors_vif);

/**
 * morse_mbssid_ie_enabled - Function to return MBSSID IE support flag value
 *
 * @mors:   The global Morse structure
 *
 * Return:  1 if enabled, 0 otherwise
 */
bool morse_mbssid_ie_enabled(struct morse *mors);
#endif /* !_MBSSID_H_ */
