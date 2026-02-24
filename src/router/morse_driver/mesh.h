#ifndef _MORSE_MESH_H_
#define _MORSE_MESH_H_

/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "dot11ah/dot11ah.h"
#include "command.h"

#define MORSE_MESH_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_MESH, _m, _f, ##_a)
#define MORSE_MESH_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_MESH, _m, _f, ##_a)
#define MORSE_MESH_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_MESH, _m, _f, ##_a)
#define MORSE_MESH_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_MESH, _m, _f, ##_a)

/** AMPE (Authenticated Mesh Peering Exchange) Block Size for MPM Open Frame */
#define AMPE_BLOCK_SIZE_OPEN_FRAME		98
/** length of the IGTK data in AMPE block (KeyID - 2bytes, IPN - 6 bytes, Key Len - 16 bytes) */
#define AMPE_BLOCK_IGTK_DATA_LEN		24
/** AMPE (Authenticated Mesh Peering Exchange) Block Size for MPM Confirm Frame */
#define AMPE_BLOCK_SIZE_CONFIRM_FRAME		70

/** Length of the Cipher or AKM suit selector in RSN IE */
#define RSN_SELECTOR_LEN 4
/** MFP capable and required bits of RSN capabilities in RSN IE */
#define RSN_CAPABILITY_MFPR BIT(6)
#define RSN_CAPABILITY_MFPC BIT(7)

/** Category code of self protected action frames */
#define WLAN_ACTION_SELF_PROTECTED 15

/** IEEE 802.11s - Mesh Capability */
#define MESH_CAP_ACCEPT_ADDITIONAL_PEER	BIT(0)

/** Mesh Formation Info field (byte) offset in Mesh configuration IE */
#define MESH_CONF_IE_FORMATION_INFO_BYTE_OFFSET	5

/** Capabilities field (byte) offset in Mesh configuration IE */
#define MESH_CONF_IE_CAPABILITY_FLAG_BYTE_OFFSET	6

/** Mesh MBCA Capability bit */
#define MESH_CAP_MBCA_ENABLED	BIT(4)

#define MESH_NO_OF_PEERINGS_MASK 0x7E

/**
 * Maximum number of beacon info fields in a beacon timing element
 * (same as FW configuration)
 */
#define MORSE_MESH_MAX_BEACON_INFO_ENTRIES	20

/**
 * Maximum number of queued TX packets for Mesh
 */
#define MORSE_MESH_MAX_TXQ_LENGTH 256

/**
 * Neighbor entry validity time in terms of TUs as per standard
 * Ref:802.11-2020 section 14.13.4.2.3 Beacon timing information.
 */
#define MESH_CONFIG_NEIGHBOR_ENTRY_VALIDITY_IN_TU	524288

/* Mesh MBCA TBTT selection and adjustment configuration to enable
 * in target LMAC firmware.
 */
#define MESH_MBCA_CFG_TBTT_SEL_ENABLE BIT(0)
#define MESH_MBCA_CFG_TBTT_ADJ_ENABLE BIT(1)

/** Report interval at which Beacon timing elements are included in Beacons */
#define DEFAULT_MESH_BCN_TIMING_REPORT_INT 10
/** TBTT Adjustment interval */
#define DEFAULT_TBTT_ADJ_INTERVAL_MSEC 60000
/** Min beacon gap between our and neighbor beacon */
#define DEFAULT_MBCA_MIN_BEACON_GAP_MS 25
/** Initial scan duration to find other mesh peers in MBSS */
#define DEFAULT_MBSS_START_SCAN_DURATION_MS 2048
/** default configuration of dynamic mesh peering */
#define DEFAULT_DYNAMIC_MESH_PEERING 0

#define MESH_PARSE_NO_OF_PEERINGS(__info) (((__info) & MESH_NO_OF_PEERINGS_MASK) >> 1)

struct mesh_neighbor_beacon_info {
	/** ID of neighbor station */
	u8 neighbor_sta_id;

	/** TBTT offset of neighbor station */
	u8 neighbor_tbtt[3];

	/** Neighbor station beacon interval */
	u16 beacon_interval;
};

struct beacon_timing_element {
	/** Information about beacon information tuple in beacon timing element */
	u8 report_control_field;

	/** One or more Beacon Timing Information fields */
	u8 beacon_timing_info_list[];
};

/**
 * enum plink_action_field - action codes of mesh peer link action frames.
 * @PLINK_OPEN: peer link open frame
 * @PLINK_CONFIRM: peer link confirm frame
 * @PLINK_CLOSE: peer link close frame
 */
enum plink_action_field {
	PLINK_OPEN = 1,
	PLINK_CONFIRM,
	PLINK_CLOSE
};

/** Returns true if the frame is mesh peering management (MPM) open frame */
static inline bool morse_dot11_is_mpm_open_frame(const struct ieee80211_mgmt *mesh_mpm_frm)
{
	return (mesh_mpm_frm->u.action.u.self_prot.action_code == WLAN_SP_MESH_PEERING_OPEN);
}

/** Returns true if the frame is mesh peering management (MPM) confirm frame */
static inline bool morse_dot11_is_mpm_confirm_frame(struct ieee80211_mgmt *mesh_mpm_frm)
{
	return (mesh_mpm_frm->u.action.u.self_prot.action_code == WLAN_SP_MESH_PEERING_CONFIRM);
}

/** Returns start addr of IEs in a mesh peering management (MPM) frame */
static inline u8 *morse_dot11_mpm_frame_ies(struct ieee80211_mgmt *mesh_mpm_frm)
{
	/* variable points to capab info in action frame, so account for
	 * size of capab info and AID size of 2 bytes in case the frame
	 * is peering confirm frame
	 */
	return (mesh_mpm_frm->u.action.u.self_prot.variable + 2 +
		(morse_dot11_is_mpm_confirm_frame(mesh_mpm_frm) ? 2 : 0));
}

 /** Returns true if the action frame is Mesh Peering Management (MPM) Frame. */
static inline bool morse_dot11_is_mpm_frame(struct ieee80211_mgmt *mgmt)
{
	/* MPM frames are not protected */
	if (ieee80211_has_protected(mgmt->frame_control))
		return false;
	/*
	 * Check for Peering open and confirm frames only as peering close frame
	 * will not have any S1G IEs
	 */
	if (mgmt->u.action.category == WLAN_CATEGORY_SELF_PROTECTED &&
	    (mgmt->u.action.u.self_prot.action_code == WLAN_SP_MESH_PEERING_OPEN ||
	     mgmt->u.action.u.self_prot.action_code == WLAN_SP_MESH_PEERING_CONFIRM))
		return true;
	else
		return false;
}

/** Returns true if mesh id element is present in the frame */
static inline bool morse_is_mesh_network(struct dot11ah_ies_mask *ies_mask)
{
	return ies_mask->ies[WLAN_EID_MESH_ID].ptr ? true : false;
}

static inline void morse_enable_mbca_capability(u8 *mesh_config_ie)
{
	mesh_config_ie[MESH_CONF_IE_CAPABILITY_FLAG_BYTE_OFFSET] |= MESH_CAP_MBCA_ENABLED;
}

/**
 * morse_restore_mesh_config() - Restores the mesh config to VIF mesh context,
 *				which is stored earlier when the config is received from
 *				wpa_supplicant via morsectrl
 *
 * @mors_vif : The mesh VIF in which config is being restored
 *
 * return 0 on success, relevant error on failure
 */
int morse_restore_mesh_config(struct morse_vif *mors_vif);

/**
 * morse_dot11_get_mpm_ampe_len() - Finds length of AMPE element (Authenticated
 *	Mesh Peering Exchange) in Mesh Peer management (MPM) frames
 *
 * @mgmt: Pointer to peering frame skb
 *
 * Return: Length of AMPE element
 */
int morse_dot11_get_mpm_ampe_len(struct sk_buff *skb);

/**
 * morse_cmd_set_mesh_config() - Saves Mesh config in driver
 *
 * @mors_vif: pointer to morse interface
 * @mesh_config: pointer to mesh config structure
 * @stored_config: pointer to mesh config from the stored list if its a restore
 *
 * Return: 0 on success and error code on failure
 */
int morse_cmd_set_mesh_config(struct morse_vif *mors_vif,
			      struct morse_cmd_req_set_mesh_config *mesh_config,
			      struct morse_mesh_config_list *stored_config);

/**
 * morse_insert_beacon_timing_element() - Inserts Beacon Timing Element in Beacon or Probe Response
 *					 frame.
 *
 * @skb: Pointer to Beacon or Probe Response frame
 * @ies_mask: Pointer to information elements
 *
 * Return: None
 */
void morse_insert_beacon_timing_element(struct morse_vif *mors_vif, struct sk_buff *skb,
					struct dot11ah_ies_mask *ies_mask);

/**
 * morse_mac_process_rx_mesh_probe_req() - Process Rx probe req on Mesh interface
 *
 * Evaluate if this directed probe req is from a known peer or a new peer. Pass on to
 * mac80211, only if this is a new peer and mac80211 will respond with a probe resp,
 * so that in beaconless mode, both peers will have information each other peer.
 *
 * @mors_vif: pointer to morse interface
 * @ies_mask: pointer to ies mask structure
 * @rx_status: Rx status of the received probe request
 * @src_addr: Source address of the Rx probe request
 *
 * Return: 0 on success and error code on failure
 */
int morse_mac_process_rx_mesh_probe_req(struct morse_vif *mors_vif,
					struct dot11ah_ies_mask *ies_mask,
					const struct ieee80211_rx_status *rx_status,
					const u8 *src_addr);

/**
 * morse_mac_process_mesh_rx_mgmt() - Process Rx mgmt frame on Mesh interface
 *
 * Evaluates Rx Mgmt frame for beaconless mode and dynamic peering.
 *
 * @mors_vif: pointer to morse interface
 * @skb: pointer to the packet buffer
 * @ies_mask: pointer to ies mask structure
 * @rx_status: Rx status of the received frame
 *
 * Return: 0 on success, -EACCES to drop the frame and error code on failure
 */
int morse_mac_process_mesh_rx_mgmt(struct morse_vif *mors_vif, struct sk_buff *skb,
				   struct dot11ah_ies_mask *ies_mask,
				   const struct ieee80211_rx_status *rx_status);

/**
 * morse_mac_add_meshid_ie() - Fill in Mesh ID from SSID IE
 *
 * @mors_vif: pointer to morse interface
 * @skb: pointer to the packet buffer to add mesh id to
 * @ies_mask: pointer to ies mask of target packet
 *
 * Return: 0 on success and error code on failure
 */
int morse_mac_add_meshid_ie(struct morse_vif *mors_vif, struct sk_buff *skb,
			    struct dot11ah_ies_mask *ies_mask);

/**
 * morse_mac_tx_mesh_probe_req() - Send Mesh Probe request with specified dest addr
 *
 * @mors_vif: pointer to morse interface
 * @dest_addr: destination address to send probe req to
 *
 * Return: 0 on success and error code on failure
 */
int morse_mac_tx_mesh_probe_req(struct morse_vif *mors_vif, const u8 *dest_addr);

/**
 * morse_cmd_cfg_mesh_bss() - Mesh network start/stop of Mesh BSS in firmware
 *
 * @mors_vif: pointer to morse interface
 * @stop_mesh: flag to notify start or stop of mesh network
 *
 * Return: 0 on success and error code on failure
 */
int morse_cmd_cfg_mesh_bss(struct morse_vif *mors_vif, bool stop_mesh);

/**
 * morse_cmd_process_mbca_conf() - Process MBCA configuration command.
 *
 * @mors_vif: pointer to morse interface
 * @mbca: pointer to MBCA configuration
 *
 * Return: 0 on success and error code on failure
 */
int morse_cmd_process_mbca_conf(struct morse_vif *mors_vif,
				struct morse_cmd_req_set_mcba_conf *mbca);

/**
 * morse_cmd_process_dynamic_peering_conf() - Process mesh dynamic peering configuration
 * command.
 *
 * @mors_vif: pointer to morse interface
 * @conf: pointer to dynamic peering configuration
 *
 * Return: 0 on success and error code on failure
 */
int morse_cmd_process_dynamic_peering_conf(struct morse_vif *mors_vif,
					   struct morse_cmd_req_dynamic_peering_config *conf);

/**
 * morse_mac_process_mesh_tx_mgmt() - Process Tx mgmt frame on Mesh interface
 * Process the probe request to add mesh id, if it's in beaconless mode. Process
 * the probe resp frame to simulate probe resp from the neighbor peer (instead of
 * additional probe req & probe resp frames), so that local mac80211 and peer will
 * have peer notification at same time for initiating the mesh peering.
 *
 * @mors_vif: pointer to morse interface
 * @skb: pointer to the packet buffer
 * @ies_mask: pointer to ies mask of target packet
 *
 * Return: 0 on success and error code on failure
 */
int morse_mac_process_mesh_tx_mgmt(struct morse_vif *mors_vif,
				   struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask);

/**
 * morse_mac_clear_mesh_list() - Free up the mesh config list when device restarts
 *
 * @mors: Global morse struct
 */
void morse_mac_clear_mesh_list(struct morse *mors);

/**
 * morse_mesh_config_list_init () - Initializes local mesh config context
 *
 * @mors: Global morse struct
 */
void morse_mesh_config_list_init(struct morse *mors);

/**
 * morse_mesh_deinit() - Mesh de-initialization
 *
 * @mors_vif: pointer to morse interface
 *
 * Return: 0 on success and error code on failure
 */
int morse_mesh_deinit(struct morse_vif *mors_vif);

/**
 * morse_mesh_init() - Mesh initialization routine
 *
 * @mors_vif: pointer to morse interface
 *
 * Return: 0 on success and error code on failure
 */
int morse_mesh_init(struct morse_vif *mors_vif);

#endif /* _MORSE_MESH_H_ */
