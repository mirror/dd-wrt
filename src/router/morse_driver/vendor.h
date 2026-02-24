#ifndef _MORSE_VENDOR_H_
#define _MORSE_VENDOR_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <net/mac80211.h>
#ifdef CONFIG_ANDROID
#include <net/netlink.h>
#endif

#include "mac.h"
#include "vendor_ie.h"

/** MORSE OUI as u32 */
#define MORSE_OUI	0x0CBF74

/** MORSE OUI as const array */
static const u8 morse_oui[] = { 0x0C, 0xBF, 0x74 };

/** Operational bits in MM vendor IE */
#define MORSE_VENDOR_IE_OPS0_DTIM_CTS_TO_SELF   BIT(0)
#define MORSE_VENDOR_IE_OPS0_LEGACY_AMSDU       BIT(1)
#define MORSE_VENDOR_IE_OPS0_RATE_CONTROL       GENMASK(3, 2)

/** Capability bits in MM vendor IE */
#define MORSE_VENDOR_IE_CAP0_MMSS_OFFSET        GENMASK(1, 0)
#define MORSE_VENDOR_IE_CAP0_SHORT_ACK_TIMEOUT  BIT(2)
/** Capability bit in MM Vendor IE to advertise PV1 data only support */
#define MORSE_VENDOR_IE_CAP0_PV1_DATA_FRAME_SUPPORT  BIT(3)
/**
 * Capability bit in MM Vendor IE to advertise Page slicing exclusive support.
 * It expects all the stations in the network to be page slicing capable.
 */
#define MORSE_VENDOR_IE_CAP0_PAGE_SLICING_EXCLUSIVE_SUPPORT  BIT(4)

#define MORSE_VENDOR_IE_CAP0_SET_MMSS_OFFSET(x) ((x) & MORSE_VENDOR_IE_CAP0_MMSS_OFFSET)
#define MORSE_VENDOR_IE_CAP0_GET_MMSS_OFFSET(x) ((x) & MORSE_VENDOR_IE_CAP0_MMSS_OFFSET)

/** OUI type of the caps & ops MM vendor IE */
#define MORSE_VENDOR_IE_CAPS_OPS_OUI_TYPE    (0)

/** Morse vendor specific frame sub categories */
#define MORSE_VENDOR_SPECIFIC_FRAME_SUBCAT_WAKE (0x01)

enum morse_vendor_cmds {
	MORSE_VENDOR_CMD_TO_MORSE = 0,
	MORSE_VENDOR_HW_CMD_TO_MORSE = 1,
#ifdef CONFIG_ANDROID
	MORSE_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES = 2,
	MORSE_VENDOR_SUBCMD_GET_PACKET_FILTER_CAPABILITIES = 3,
	MORSE_VENDOR_SUBCMD_SET_PACKET_FILTER = 4,
	MORSE_VENDOR_SUBCMD_READ_PACKET_FILTER_DATA = 5,
#endif
};

enum morse_vendor_events {
	MORSE_VENDOR_EVENT_BCN_VENDOR_IE_FOUND = 0,	/* To be deprecated in a future version */
	MORSE_VENDOR_EVENT_OCS_DONE = 1,
	MORSE_VENDOR_EVENT_MGMT_VENDOR_IE_FOUND = 2,
	MORSE_VENDOR_EVENT_MESH_PEER_ADDR = 3,
	MORSE_VENDOR_EVENT_BSS_STATS = 4
};

enum morse_vendor_attributes {
	MORSE_VENDOR_ATTR_DATA = 0,
	/* Bitmask of type @ref enum morse_vendor_ie_mgmt_type_flags */
	MORSE_VENDOR_ATTR_MGMT_FRAME_TYPE = 1,
};

struct morse_mesh_peer_addr_vendor_evt {
	u8 addr[ETH_ALEN];
} __packed;

/** Morse vendor capability & operations IE */
struct dot11_morse_vendor_caps_ops_ie {
	u8 oui[3];
	u8 oui_type;
	struct {
		u8 major;
		u8 minor;
		u8 patch;
		u8 reserved;
	} __packed sw_ver;
	u32 hw_ver;
	u8 cap0;
	u8 ops0;
} __packed;

/**
 * Get the IE length of the vendor IE for a given OUI type.
 *
 * @pkt The packet to potentially insert into.
 * @oui_type The OUI Type.
 * @return int Non-zero if IE should be inserted into pkt.
 */
int morse_vendor_get_ie_len_for_pkt(struct sk_buff *pkt, int oui_type);

/**
 * morse_vendor_insert_caps_ops_ie() - Inserts a morse vendor capability and
 *  operation vendor IE into a given packet.
 *
 * @mors: The mors object which will dictate how the IE is filled.
 * @vif: pointer to interface.
 * @skb: The packet to insert the data into.
 * @ies_mask: Contains array of information elements.
 */
void morse_vendor_insert_caps_ops_ie(struct morse *mors, struct ieee80211_vif *vif,
				     struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask);

/**
 * Receive and process our vendor caps ops IE on reception
 *        of a management frame.
 *
 * @mors_vif: pointer to morse interface
 * @mgmt: pointer to the mgmt frame.
 * @ies_mask: The ies_mask which points to ies in the frame.
 */
void morse_vendor_rx_caps_ops_ie(struct morse_vif *mors_vif,
				 const struct ieee80211_mgmt *mgmt,
				 struct dot11ah_ies_mask *ies_mask);

/**
 * Reset any transient info filled via a vendor IE on a STA object.
 *        Nominally called on disassociation.
 *
 * @vif  The interface this reset is occurring for.
 * @mors_sta A pointer to the sta object.
 */
void morse_vendor_reset_sta_transient_info(struct ieee80211_vif *vif, struct morse_sta *mors_sta);

/**
 * Send a vendor_ie_found netlink event
 *
 * @wdev: Wireless interface
 * @frame_type: Frame type that the vendor IE was found in
 *		     of type @ref enum morse_vendor_ie_mgmt_type_flags
 * @vie: Vendor IE which was found
 * @return int 0 on success else error code
 */
int morse_vendor_send_mgmt_vendor_ie_found_event(struct wireless_dev *wdev, u16 frame_type,
						 const struct ieee80211_vendor_ie *vie);

/**
 * Send an Off Channel Scan (OCS) netlink event
 *
 * @vif Interface to send the event on
 * @vie Vendor IE which was found
 * @return int 0 on success else error code
 */
int morse_vendor_send_ocs_done_event(struct ieee80211_vif *vif, struct morse_cmd_evt_ocs_done *evt);

/**
 * Send mesh peer addr netlink event
 *
 * @vif: Interface to send the event on
 * @event: kick out peer address event
 *
 * Return: 0 on success else error code
 */
int morse_vendor_send_peer_addr_event(struct ieee80211_vif *vif,
				      struct morse_mesh_peer_addr_vendor_evt *event);

void morse_vendor_update_ack_timeout_on_assoc(struct morse *mors,
					      struct ieee80211_vif *vif, struct ieee80211_sta *sta);

void morse_set_vendor_commands_and_events(struct wiphy *wiphy);

/**
 * Send BSS statistics netlink event
 *
 * @vif interface to send the event on
 * @evt station statistics event
 * @evt_data_len size of event data
 * @return int 0 on success else error code
 */
int morse_vendor_send_bss_stats_event(struct ieee80211_vif *vif,
			struct morse_evt_bss_stats *evt, size_t evt_data_len);

/**
 * morse-vendor_find_vendor_ie()	- Find vendor IE with morse OUI.
 *
 * @ies_mask: Parsed S1G IEs to search morse OUI from.
 *
 * Return: Pointer to vendor IEs with morse OUI within ies_mask.
 *	   NULL if it couldn't find any morse OUI.
 */
struct dot11_morse_vendor_caps_ops_ie *
morse_vendor_find_vendor_ie(struct dot11ah_ies_mask *ies_mask);
/**
 * morse_vendor_fill_sta_vendor_info()	- Fill vendor info into station interface from vendor IEs.
 *
 * @mors_vif: Station morse interface to fill vendor info into.
 * @ie: IEs that contain vendor info.
 */
void morse_vendor_fill_sta_vendor_info(struct morse_vif *mors_vif,
				       struct dot11_morse_vendor_caps_ops_ie *ie);

#endif /* !_MORSE_VENDOR_H_ */
