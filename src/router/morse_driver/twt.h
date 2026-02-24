/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef _TWT_H_
#define _TWT_H_

#include <linux/types.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <net/netlink.h>

#include "morse.h"
#include "dot11ah/dot11ah.h"

/* For now limit to maximum as defined in P802.11REVme_D1.1 Section 9.4.2.199 */
#define MORSE_TWT_AGREEMENTS_MAX_PER_STA		(8)

#define TWT_WAKE_DURATION_UNIT				(256)
#define TWT_WAKE_INTERVAL_EXPONENT_MAX_VAL		(31)
#define TWT_WAKE_DURATION_MAX_US			(__UINT8_MAX__ * TWT_WAKE_DURATION_UNIT)
#define TWT_AGREEMENT_REQUEST_TYPE_OFFSET		(1)
#define TWT_AGREEMENT_TARGET_WAKE_TIME_OFFSET		(3)
#define TWT_AGREEMENT_WAKE_DURATION_OFFSET		(11)
#define TWT_AGREEMENT_WAKE_INTERVAL_MANTISSA_OFFSET	(12)
#define TWT_TEARDOWN_FLOW_ID_MASK GENMASK(2, 0)

enum morse_twt_state {
	MORSE_TWT_STATE_NO_AGREEMENT,
	MORSE_TWT_STATE_CONSIDER_REQUEST,
	MORSE_TWT_STATE_CONSIDER_SUGGEST,
	MORSE_TWT_STATE_CONSIDER_DEMAND,
	MORSE_TWT_STATE_CONSIDER_GROUPING,
	MORSE_TWT_STATE_AGREEMENT,
};

/* This structure is packed as control and params form the TWT IE. This allows the use of memcpy. */
struct morse_twt_agreement_data {
	/** First wakeup time in us with reference to the TSF */
	u64 wake_time_us;
	/** Interval between wake ups in us */
	u64 wake_interval_us;
	/** Wake nominal minimum duration in us */
	u32 wake_duration_us;
	/** TWT control field */
	u8 control;
	/** TWT agreement parameters */
	struct ieee80211_twt_params params;
} __packed;

struct morse_twt_agreement {
	struct list_head list;
	enum morse_twt_state state;
	struct morse_twt_agreement_data data;
};

enum morse_twt_event_type {
	MORSE_TWT_EVENT_SETUP,
	MORSE_TWT_EVENT_TEARDOWN
};

struct morse_twt_event {
	struct list_head list;
	enum morse_twt_event_type type;
	u8 addr[ETH_ALEN];
	u8 flow_id;
	union {
		struct {
			enum ieee80211_twt_setup_cmd cmd;
			struct morse_twt_agreement_data *agr_data;
		} setup;

		/* TODO Implement teardown. */
		struct {
			bool teardown;
		} teardown;
	};
};

/*
 * S1G action frame TWT action Category
 */
struct morse_dot11ah_s1g_twt_action {
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
		} __packed twt_action_setup;
		struct {
			u8 action_code;
			u8 flow;
		} __packed twt_action_teardown;
	} u;
} __packed;

struct morse_twt_sta {
	struct list_head list;
	u8 addr[ETH_ALEN];
	/* dialog token of pending action frame */
	u8 dialog_token;
	/* flag to notify pending action frame */
	bool action_is_pending;
	struct morse_twt_agreement agreements[MORSE_TWT_AGREEMENTS_MAX_PER_STA];
};

struct morse_twt_wake_interval {
	struct list_head list;
	struct list_head agreements;
};

static inline struct morse_vif *morse_twt_to_morse_vif(struct morse_twt *twt)
{
	return container_of(twt, struct morse_vif, twt);
}

/**
 * @morse_twt_is_enabled() - Check if TWT is enabled on a VIF
 *
 * @mors_vif    Morse virtual interface
 *
 * Return:      True if TWT is enabled
 */
bool morse_twt_is_enabled(struct morse_vif *mors_vif);

/**
 * @morse_twt_event_queue_purge() - Remove all events for a STA addr from the event queue
 *
 * @mors       Morse device
 * @mors_vif   Morse virtual interface
 * @addr       Address of the STA to remove events for
 */
void morse_twt_event_queue_purge(struct morse *mors, struct morse_vif *mors_vif, u8 *addr);

/**
 * morse_twt_sta_remove_addr() - Remove a station's TWT agreement.
 *
 * @mors       Morse device
 * @mors_vif   Morse virtual interface
 * @skb		The sk_buff which should contain a (re)assoc frame
 *
 * Return:	0 on success or relevant error.
 */
int morse_twt_sta_remove_addr(struct morse *mors, struct morse_vif *mors_vif, u8 *addr);

/**
 * morse_twt_insert_ie() - Insert a TWT IE into an sk_buff
 *
 * @mors	Morse device
 * @tx		The TWT data to send
 * @ies_mask	Array of information elements.
 * @size	Size of the TWT IE (can be found using morse_twt_get_ie_size())
 */
void morse_twt_insert_ie(struct morse *mors,
			 struct morse_twt_event *tx, struct dot11ah_ies_mask *ies_mask, u8 size);

/**
 * morse_twt_dequeue_tx() -	removes TX data from the queue it is in. It will also handle
 *				freeing the associated memory.
 *
 *
 * @mors	Morse device
 * @event	The event or TX data to get the TWT IE size of
 * @tx		The transmitted event to remove from the queue.
 *
 * Return:	Positive size of the TWT on success or negative error code
 */
int morse_twt_dequeue_tx(struct morse *mors, struct morse_vif *mors_vif,
			 struct morse_twt_event *tx);

/**
 * morse_twt_get_ie_size() - Gets the size of a TWT IE for an event or TX data
 *
 * @mors	Morse device
 * @event	The event or TX data to get the TWT IE size of
 *
 * Return:	Positive size of the TWT IE on success or negative error code
 */
int morse_twt_get_ie_size(struct morse *mors, struct morse_twt_event *event);

/**
 * morse_twt_peek_tx() - Get TWT TX data from the queue without removing it
 *
 * @mors	   Morse device
 * @mors_vif   Morse virtual interface
 * @addr	Destination address to get TX data for
 * @flow_id	Flow id to match if not NULL
 *
 * Return:	TX data for the destination address or NULL if none available or error
 */
struct morse_twt_event *morse_twt_peek_tx(struct morse *mors,
					  struct morse_vif *mors_vif,
					  const u8 *addr, const u8 *flow_id);

/**
 * morse_twt_parse_ie() - Parse a TWT IE and fills out an event
 *
 * @mors_vif   Morse virtual interface
 * @ie         The TWT IE to parse.
 * @event      The event to fill.
 * @src_addr   Address of the device sending the IE.
 *
 * @return 0 on success, else error code
 */
int morse_twt_parse_ie(struct morse_vif *mors_vif, struct ie_element *ie,
		       struct morse_twt_event *event, const u8 *src_addr);

/**
 * morse_twt_dump_element() - Prints out the information for an event
 *
 * @mors	Morse device
 * @event	The twt event to dump
 */
void morse_twt_dump_event(struct morse *mors, struct morse_twt_event *event);

/**
 * morse_twt_dump_wake_interval_tree() - Print the tree of wake intervals/agreements to debugfs
 *
 * @file       Seq file to print debug to
 * @mors_vif   Morse virtual interface
 */
void morse_twt_dump_wake_interval_tree(struct seq_file *file, struct morse_vif *mors_vif);

/**
 * morse_twt_dump_sta_agreements() - Print the tree of stations/agreements to debugfs
 *
 * @file	    Seq file to print debug to
 * @mors_vif    Morse virtual interface
 */
void morse_twt_dump_sta_agreements(struct seq_file *file, struct morse_vif *mors_vif);

/**
 * morse_twt_process_pending_cmds() - Installs/Uninstalls pending agreements to the firmware
 *
 * @mors	    Morse device
 * @mors_vif	Morse virtual interface
 */
void morse_twt_process_pending_cmds(struct morse *mors, struct morse_vif *mors_vif);

/**
 * morse_twt_handle_event() - Process a TWT event
 *
 * @mors_vif    Morse virtual interface
 * @addr        Address to filter on, if NULL proccess all events
 */
void morse_twt_handle_event(struct morse_vif *mors_vif, u8 *addr);

/**
 * @morse_mac_process_rx_twt_mgmt() - Process TWT IEs in management frames
 *
 * @mors      Morse device
 * @vif       VIF struct
 * @skb       Rx management SKB
 * @ies_mask  IEs Mask
 *
 * Return: -EACCESS to avoid forwarding, 0 to forward to mac80211,  or an error code.
 */
int morse_mac_process_rx_twt_mgmt(struct morse *mors, struct ieee80211_vif *vif,
			const struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask);

/**
 * @morse_mac_process_twt_action_tx_finish() - Process Tx completion of TWT action frames
 *
 * @mors      Morse device
 * @vif       VIF struct
 * @skb       Tx TWT action SKB
 */
void morse_mac_process_twt_action_tx_finish(struct morse *mors, struct ieee80211_vif *vif,
			const struct sk_buff *skb);

/**
 * morse_twt_init_vif() - Initialise TWT for a VIF
 *
 * @mors		Morse device
 * @mors_vif		Morse virtual interface
 * @enable_twt          True if TWT is configured
 * @is_ap		True if the interface is in AP mode
 * @is_sta		True if the interface is in STA mode
 * @ps_is_enabled	True if power save is enabled
 * @ps_is_offloaded	True if power save offload is enabled
 * @connection_monitor_is_enabled	True if the MAC80211 connection monitor is enabled
 */
void morse_twt_init_vif(struct morse *mors, struct morse_vif *mors_vif,
			bool enable_twt, bool is_ap, bool is_sta,
			bool ps_is_enabled, bool ps_is_offloaded,
			bool connection_monitor_is_enabled);

/**
 * morse_twt_finish_vif() - Initialise TWT for a VIF
 *
 * @mors       Morse device
 * @mors_vif   Morse virtual interface
 */
void morse_twt_finish_vif(struct morse *mors, struct morse_vif *mors_vif);

/**
 * morse_twt_initialise_agreement() - Initialises the twt agreement that needs to be sent to the FW
 * This is only used when we want to directly set the twt params in the FW bypassing the TWT IE
 * insertion
 *
 * @twt_conf	The twt configuration struct
 * @agreement	pointer to the agreement that need to be sent to the FW
 *
 * Return: The agreement length
 */
int morse_twt_initialise_agreement(struct morse_twt_agreement_data *twt_data, u8 *agreement);

/**
 * morse_process_twt_cmd() - Process TWT message triggered by morsectrl
 *
 * @mors        Morse device
 * @morse_vif   Morse virtual interface
 * @cmd         Incoming twt conf command parameters
 *
 * @return 0 on success, else error code
 */
int morse_process_twt_cmd(struct morse *mors, struct morse_vif *mors_vif,
			  struct morse_cmd_req *cmd);

 /**
  * morse_dot11_is_twt_setup_action_frame() - Checks if TWT setup action frame
  *
  * @twt_action	pointer to mgmt frame
  *
  * @return true, if TWT setup action frame, else false
  */
static inline bool morse_dot11_is_twt_setup_action_frame
		(struct morse_dot11ah_s1g_twt_action *twt_action)
{
	u8 action_code = twt_action->u.twt_action_setup.action_code;

	/*
	 * Check for TWT Setup frame
	 */
	return ((twt_action->category == WLAN_CATEGORY_S1G_PROTECTED &&
			(action_code == WLAN_S1G_PROTECTED_TWT_SETUP)) ||
			(twt_action->category == WLAN_CATEGORY_S1G_UNPROTECTED &&
			(action_code == WLAN_S1G_TWT_SETUP)));
}

 /**
  * morse_dot11_is_twt_teardown_action_frame() - Checks if TWT teardown action frame
  *
  * @twt_action	pointer to mgmt frame
  *
  * @return true, if TWT teardown action frame, else false
  */
static inline bool morse_dot11_is_twt_teardown_action_frame
		(struct morse_dot11ah_s1g_twt_action *twt_action)
{
	u8 action_code = twt_action->u.twt_action_setup.action_code;

	/*
	 * Check for TWT teardown frame only as TWT Teardown will not have IEs
	 */
	return ((twt_action->category == WLAN_CATEGORY_S1G_PROTECTED &&
			(action_code == WLAN_S1G_PROTECTED_TWT_TEARDOWN)) ||
			(twt_action->category == WLAN_CATEGORY_S1G_UNPROTECTED &&
			(action_code == WLAN_S1G_TWT_TEARDOWN)));
}

/**
 * morse_dot11_twt_action_ie_pos() - Get the position of the IE in TWT setup action frame
 *
 * @twt_action	pointer to mgmt frame
 *
 * @return position of IE in TWT setup action frame
 */
static inline u8 *morse_dot11_twt_action_ie_pos(struct morse_dot11ah_s1g_twt_action *twt_action)
{
	return twt_action->u.twt_action_setup.variable;
}

#endif
