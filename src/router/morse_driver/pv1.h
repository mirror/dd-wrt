/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef _PV1_FRAME_H_
#define _PV1_FRAME_H_

#include <linux/types.h>
#include <net/mac80211.h>
#include <linux/ieee80211.h>
#include "dot11ah/dot11ah.h"

#define DOT11_PV1_PROTOCOL_VERSION	0x0001

/*
 * PV1 definitions.
 */
#define DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA_SID    0x0000
#define DOT11_MAC_PV1_FRAME_TYPE_MGMT            0x0004
#define DOT11_MAC_PV1_FRAME_TYPE_CTRL            0x0008
#define DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA        0x000C

/*
 * SID Definitions
 */
#define DOT11_MAC_PV1_SID_AID_MASK       GENMASK(12, 0)
#define DOT11_MAC_PV1_SID_A3_PRESENT      BIT(13)
#define DOT11_MAC_PV1_SID_A4_PRESENT      BIT(14)
#define DOT11_MAC_PV1_SID_AMSDU_PRESENT   BIT(15)

/*
 * PV1 Management Subtype
 */
#define DOT11_MAC_PV1_MGMT_SUB_TYPE_ACTION           0x0000
#define DOT11_MAC_PV1_MGMT_SUB_TYPE_ACTION_NO_ACK    0x0020
#define DOT11_MAC_PV1_MGMT_SUB_TYPE_PROBE_RESP       0x0040
#define DOT11_MAC_PV1_MGMT_SUB_TYPE_RA               0x0060

#define DOT11_MAC_PV1_STYPE_OFFSET     (5)
#if KERNEL_VERSION(5, 7, 19) > MAC80211_VERSION_CODE
/* PV1 Layout 11ah 9.8.3.1 */
#define IEEE80211_PV1_FCTL_VERS	0x0003
#define IEEE80211_PV1_FCTL_FTYPE	0x001c
#define IEEE80211_PV1_FCTL_STYPE	0x00e0
#define IEEE80211_PV1_FCTL_MOREFRAGS	0x0200
#define IEEE80211_PV1_FCTL_PM		0x0400
#define IEEE80211_PV1_FCTL_MOREDATA	0x0800
#define IEEE80211_PV1_FCTL_PROTECTED	0x1000
#define IEEE80211_PV1_FCTL_END_SP	0x2000
#define IEEE80211_PV1_FCTL_RELAYED	0x4000
#define IEEE80211_PV1_FCTL_ACK_POLICY	0x8000
#endif

/*
 * PV1 Header Compression Control Subfields
 */
#define DOT11AH_PV1_HEADER_COMPRESSION_REQ_RESPONSE    BIT(0)
#define DOT11AH_PV1_HEADER_COMPRESSION_STORE_A3    BIT(1)
#define DOT11AH_PV1_HEADER_COMPRESSION_STORE_A4    BIT(2)
#define DOT11AH_PV1_HEADER_COMPRESSION_CCMP_UPDATE    BIT(3)
#define DOT11AH_PV1_HEADER_COMPRESSION_TYPE3_SUPPORT    BIT(4)
#define DOT11AH_PV1_HEADER_COMPRESSION_RESERVED    GENMASK(7, 5)

/*
 * Morse command to firmware for PV1 bitmap
 */
#define MORSE_PV1_CMD_STORE_A3    BIT(0)
#define MORSE_PV1_CMD_STORE_A4    BIT(1)
#define MORSE_PV1_CMD_STORE_RX    BIT(2)

/**
 * Maximum size of optional elements in Header Compression IE
 * Header Compression Control + A3(6) + A4(6) + CCMP Update(5)
 */
#define HC_IE_SIZE_MAX    18

/**
 * Maximum PV1 header size including all the optional address
 * QoS type 0 - Size of SID header (12) + Size of A3 (6) + Size of A4 (6)
 * QoS type 3 - Size of QoS data header (16)
 */
#define DOT11_PV1_MAC_HEADER_SIZE_MAX  (24)
#define BPN_LEN	(4)

/**
 * Timeout value to wait for Header Compression Response after sending Request
 */
#define HC_RESPONSE_TIMEOUT 10

/** Number of Partial TIDs */
#define IEEE80211_NUM_PTIDS	8
/* As per section 9.8.3 PTID is 3 LSB bits of TID */
#define IEEE80211_PTID_MASK         0x07
/* Conversion from TID to PTID */
#define TID_TO_PTID(_tid)        (((_tid) & IEEE80211_PTID_MASK))

struct dot11ah_mac_pv1_hdr {
	__le16 frame_ctrl;
	u8 variable[];
} __packed;

struct dot11ah_mac_pv1_qos_data_sid_hdr {
	__le16 frame_ctrl;
	union {
		struct {
			__le16 addr1_sid;
			u8 addr2[ETH_ALEN];
		} __packed from_ds;
		struct {
			u8 addr1[ETH_ALEN];
			__le16 addr2_sid;
		} __packed to_ds;
	} u;
	__le16 sequence_ctrl;
	u8 variable[];
} __packed;

struct dot11ah_mac_pv1_qos_data_hdr {
	__le16 frame_ctrl;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	__le16 sequence_ctrl;
} __packed;

/**
 * PV1 Header Compression element format
 *
 * @header_compression_control:   Header Compression control field
 * @variable:    Header Compression IE
 */
struct dot11ah_pv1_header_compression {
	u8 header_compression_control;
	u8 variable[];
};

/*
 * S1G action frame including PV1 Header Compression action Category
 */
struct morse_dot11ah_s1g_action {
	__le16 frame_control;
	__le16 duration;
	u8 da[ETH_ALEN];
	u8 sa[ETH_ALEN];
	u8 bssid[ETH_ALEN];
	__le16 seq_ctrl;
	u8 category;
	union {
		struct {
			u8 action_code;
			u8 dialog_token;
			u8 variable[];
		} __packed pv1_action;
	} u;
} __packed;

struct morse_pv1_hc_request {
	/** A3 from data to be filled in request */
	u8 header_compression_a3[ETH_ALEN];
	/** A4 from data to be filled in request */
	u8 header_compression_a4[ETH_ALEN];
	/** Latest A3 stored at TX for next request comparison */
	u8 stored_a3[ETH_ALEN];
	/** Latest A4 stored at TX for next request comparison */
	u8 stored_a4[ETH_ALEN];
	/** Status of latest A1 A3 difference */
	bool a1_a3_differ;
	/** Status of latest A2 A4 difference */
	bool a2_a4_differ;
	/** Flag to indicate if data is 4 address, decided based on from/to DS */
	bool a4_included;
	/** Dialog token of each action frame */
	u8 action_dialog_token;
	/** Flag to check if any header compression action is in progress */
	bool action_in_progress;
};

/**
 * struct morse_pv1 - Contains PV1 state and configuration information
 *
 * @tx_request:          Header Compresion Request context at TX
 * @rx_request:          Header Compresion Request context at RX
 * @fw_stored_response_status:  Flag to track storage status in firmware
 * @rx_pv1_sta_addr:     STA for which Header Compression Request is received
 * @tx_pv1_sta_addr:     STA for which Header Compression Request is sent
 * @hc_req_work:         Work queue to process PV1 Header Compression Request
 * @hc_resp_work:        Work queue to process PV1 Header Compression Response
 * @hc_response_timeout: Timeout value to keep track RX of Header Compression Response
 * @lock:                Mutex used to control access to lists/memory
 */
struct morse_pv1 {
	struct morse_pv1_hc_request tx_request;
	struct morse_pv1_hc_request rx_request;
	bool fw_stored_response_status;
	u8 rx_pv1_sta_addr[ETH_ALEN];
	u8 tx_pv1_sta_addr[ETH_ALEN];
	struct work_struct hc_req_work;
	struct work_struct hc_resp_work;
	u32 hc_response_timeout;
	/* Mutex to serialse PV1 operations */
	struct mutex lock;
};

struct morse_sta_pv1 {
	/** Store A3 flag set in Header Compression Response */
	bool a3_stored;
	/** Store A4 flag set in Header Compression Response */
	bool a4_stored;
	/** Latest A3 stored for STA at TX for next request comparison */
	u8 stored_a3[ETH_ALEN];
	/** Latest A4 stored for STA at TX for next request comparison */
	u8 stored_a4[ETH_ALEN];
	/** Last sequence number per PTID, 1 additional for mgmt frames */
	u16 last_seq_num[IEEE80211_NUM_PTIDS + 1];
	/** Base Packet Number (BPN) per PTID, 1 additional for mgmt frames */
	u32 bpn[IEEE80211_NUM_PTIDS + 1];
};

static inline bool morse_dot11ah_is_protocol_version_1(u16 fc)
{
	return ((fc & IEEE80211_PV1_FCTL_VERS) == 1);
}

static inline bool morse_dot11ah_is_pv1_qos_data(u16 fc)
{
	return (morse_dot11ah_is_protocol_version_1(fc) &&
		((fc & IEEE80211_PV1_FCTL_FTYPE) == DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA_SID ||
		 (fc & IEEE80211_PV1_FCTL_FTYPE) == DOT11_MAC_PV1_FRAME_TYPE_QOS_DATA));
}

/**
 * morse_pv1_a3_a4_check - Check if the TX frame if 3 address of 4 address frame.
 *            If its a 3 address frame compare A1 with A3,
 *            if different save the A1 A3 differ status to PV1 context of this VIF.
 *            If its a 4 address frame compare A1 with A3 and A2 with A4,
 *            if different save A1 A3 and A2 A4 differ status to PV1 context of this VIF.
 *
 * @mors_vif:  TX iface
 * @pubsta:    Peer STA associated to this VIF
 * @skb:       TX data SKB
 */
void morse_pv1_a3_a4_check(struct morse_vif *mors_vif, struct ieee80211_sta *pubsta,
						struct sk_buff *skb);
/**
 * morse_mac_send_pv1_hc_action_frame - Generate and send PV1 Header Compression action frame
 *
 * @mors:        The global Morse structure
 * @vif:         Valid VIF on which action frame gets transmitted
 * @sta:         Peer STA to which action frame is being transmitted
 * @skb_data:    Data SKB
 * @is_response: Header Compression request/response
 */
void morse_mac_send_pv1_hc_action_frame(struct morse *mors, struct ieee80211_vif *vif,
						struct ieee80211_sta *sta,
						struct sk_buff *skb_data, bool is_response);

/**
 * morse_dot11ah_insert_pv1_hc_ie - Insert Header Compression IE to Management frames
 *
 * @vif:         Valid VIF
 * @ies_mask:    IE mask pointer
 * @is_response: Header Compression request/response
 *
 * Return:  Length on IE
 */
int morse_dot11ah_insert_pv1_hc_ie(struct ieee80211_vif *vif,
					struct dot11ah_ies_mask *ies_mask, bool is_response);

/**
 * morse_mac_process_pv1_action_frame - Process Header Compression Request/Response on RX
 *
 * @mgmt:    Received Header Compression frame
 * @mors:    The global Morse structure
 * @vif:     Valid VIF
 */
void morse_mac_process_pv1_action_frame(struct morse_dot11ah_s1g_action *mgmt,
					struct morse *mors, struct ieee80211_vif *vif);

/**
 * mors_pv1_init_vif -  Init PV1 context for VIF
 *
 * @mors_vif:    Valid AP/STA VIF
 */
void mors_pv1_init_vif(struct morse_vif *mors_vif);

/**
 * morse_pv1_finish_vif - Deinit PV1 context for VIF
 *
 * @mors_vif:    Valid AP/STA VIF
 */
void morse_pv1_finish_vif(struct morse_vif *mors_vif);

/**
 * morse_mac_convert_pv0_to_pv1 - Convert PV0 to PV1 frame
 *
 * @mors:    The global Morse structure
 * @mors_vif: Morse vif structure
 * @sta:     Peer STA to which frame is being transmitted
 * @skb:     Tx data SKB
 *
 * @return: 0 on success, error on failure
 */
int morse_mac_convert_pv0_to_pv1(struct morse *mors, struct morse_vif *mors_vif,
		struct ieee80211_sta *sta, struct sk_buff *skb);

/**
 * morse_mac_convert_pv1_to_pv0 - Replaces PV1 MAC Header with PV0 Header of Qos Data
 *               on RX. If VIF does not support PV1 no conversion is done
 *
 * @mors:           The global Morse structure
 * @mors_vif:        Valid AP/STA VIF
 * @skb:            RX PV1 data SKB
 * @hdr_rx_status:  RX status buffer
 * @pv1_hdr:        Pointer to RX packet PV1 frame header
 *
 * @return     0 on success, error on failure
 */
int morse_mac_convert_pv1_to_pv0(struct morse *mors,
				 struct morse_vif *mors_vif, struct sk_buff *skb,
				 const struct morse_skb_rx_status *hdr_rx_status,
				 struct dot11ah_mac_pv1_hdr *pv1_hdr);

/**
 * morse_pv1_find_sta    Get peer STA that has PV1 context based on PV1 header of RX frame
 *
 * @vif:     Pointer to valid VIF iface
 * @hdr:     PV1 header
 *
 * @return:  Pointer to STA context
 *
 * @note:	The RCU lock must be held when calling this function and while using the returned
 *		pointer.
 */
struct ieee80211_sta *morse_pv1_find_sta(struct ieee80211_vif *vif,
				struct dot11ah_mac_pv1_hdr *hdr);

/**
 * morse_is_pv1_protected_frame -  Function to check if PV1 QoS data is protected
 *
 * @skb:         PV1 QoS data packet
 *
 * @return:     true if protected, false on failure
 */
bool morse_is_pv1_protected_frame(struct sk_buff *skb);
#endif /* _PV1_FRAME_H_ */
