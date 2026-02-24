#ifndef _MORSE_VENDOR_IE_H_
#define _MORSE_VENDOR_IE_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "morse.h"
#include "command.h"

enum morse_vendor_ie_mgmt_type_flags {
	MORSE_VENDOR_IE_TYPE_NONE = 0,

	MORSE_VENDOR_IE_TYPE_BEACON = BIT(0),
	MORSE_VENDOR_IE_TYPE_PROBE_REQ = BIT(1),
	MORSE_VENDOR_IE_TYPE_PROBE_RESP = BIT(2),
	MORSE_VENDOR_IE_TYPE_ASSOC_REQ = BIT(3),
	MORSE_VENDOR_IE_TYPE_ASSOC_RESP = BIT(4),
	/* ... etc. */

	MORSE_VENDOR_IE_TYPE_ALL = GENMASK(15, 0)
};

/** Max amount of data in a vendor IE. Limited by the length field being 1 byte */
#define MORSE_MAX_VENDOR_IE_SIZE		(U8_MAX)

/**
 * Vendor information element list item to insert into management frames.
 */
struct vendor_ie_list_item {
	struct list_head list;
	/** Management type bitmask which this vendor IE should be inserted into */
	u16 mgmt_type_mask;
	/** The vendor information element to insert */
	struct ieee80211_vendor_ie ie;
};

/**
 * Vendor IE OUI filter list item. The callback will be called if a management frame with
 *	  a vendor element that matches an OUI in the list is found.
 */
struct vendor_ie_oui_filter_list_item {
	struct list_head list;
	/** Management type bitmask which this item applies to */
	u16 mgmt_type_mask;
	/** OUI to match */
	u8 oui[OUI_SIZE];
	/**
	 * Callback function for vendor IE match
	 *
	 * @wdev - wireless interface the frame was received on
	 * @frame_type - The frame type that was received
	 *			of type @ref enum morse_vendor_ie_mgmt_type_flags
	 * @vie - The vendor IE that was matched
	 * @return 0 on success, else error code
	 */
	int (*on_vendor_ie_match)(struct wireless_dev *wdev,
				  u16 frame_type, const struct ieee80211_vendor_ie *vie);
};

/**
 * Initialise the structures for vendor IE processing in the interface
 *
 * @mors_vif Interface to initialise
 */
void morse_vendor_ie_init_interface(struct morse_vif *mors_vif);

/**
 * Uninitialise and free the structures for vendor IE processing in the interface
 *
 * @mors_vif Interface to uninitialise
 */
void morse_vendor_ie_deinit_interface(struct morse_vif *mors_vif);

/**
 * Get the total length of the currently configured vendor IEs.
 *
 * @note Caller must hold the vendor IE lock before calling this function
 *
 * @mors_vif Interface to operate on
 * @mgmt_type_mask Bitmask of @ref morse_vendor_ie_mgmt_type_flags to specify which
 *			 management frames to calculate the lengths for
 * @return length in bytes, or 0 if none configured
 */
u16 morse_vendor_ie_get_ies_length(struct morse_vif *mors_vif, u16 mgmt_type_mask);

/**
 * Append configured vendor IEs onto an skb
 *
 * @note Caller must hold the vendor IE lock before calling this function
 *
 * @mors_vif Interface with configured vendor IEs
 * @ies_mask Contains array of information elements
 * @mgmt_type_mask Bitmask of @ref morse_vendor_ie_mgmt_type_flags to specify which
 *			 management frames to insert
 * @return 0 on success, else error code
 */
int morse_vendor_ie_add_ies(struct morse_vif *mors_vif,
			    struct dot11ah_ies_mask *ies_mask, u16 mgmt_type_mask);

/**
 * Process a received management frame (or S1G beacon) and call the callback configured
 *		for each vendor element received with an OUI that matches one in the OUI filter
 *
 * @vif virtual interface the beacon was received on
 * @skb the management frame SKB
 */
void morse_vendor_ie_process_rx_mgmt(struct ieee80211_vif *vif, const struct sk_buff *skb);

/**
 * Handle a vendor IE config command
 *
 * @note Only supports beacons for now
 *
 * @mors_vif Interface command was received for
 * @cfg received config information
 * @return 0 on success, else error code
 */
int morse_vendor_ie_handle_config_cmd(struct morse_vif *mors_vif,
				      struct morse_cmd_req_vendor_ie_config *cfg);

/**
 * Process received information elements. Will search for vendor IEs with OUIs that match
 * those in the virtual interface's OUI filter, and will call the call back for each match.
 *
 * @wdev: Wireless interface
 * @ies: Information elements of the frame to process
 * @length: total length of ies
 * @mgmt_type: management frame type (+ S1G beacon) of the received frame
 *			 of type @ref enum morse_vendor_ie_mgmt_type_flags
 * @return 0 on success, else error code
 */
int morse_vendor_ie_process_rx_ies(struct wireless_dev *wdev, const u8 *ies, u16 length,
				   u16 mgmt_type);

#endif /* !_MORSE_VENDOR_IE_H_ */
