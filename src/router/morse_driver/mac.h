#ifndef _MORSE_MAC_H_
#define _MORSE_MAC_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/skbuff.h>
#include <linux/crc32.h>
#include "morse.h"
#include "command.h"
#include "skb_header.h"

/* The maximum number of frames to send after a DTIM to firmware */
#define MORSE_MAX_MC_FRAMES_AFTER_DTIM (10)

/**
 * struct morse_queue_params - QoS parameters
 *
 * @uapsd: access category status for UAPSD
 * @aci: access category index
 * @aifs: arbitration interframe space [0..255]
 * @cw_min: minimum contention window
 * @cw_max: maximum contention window
 * @txop: maximum burst time in units of usecs, 0 meaning disabled
 */
struct morse_queue_params {
	u8 uapsd;
	u8 aci;
	u8 aifs;
	u16 cw_min;
	u16 cw_max;
	u32 txop;
};

/* Check if MAC80211_MESH is enabled in .config */
#define MESH_CONFIG_ENABLED(value) \
		(IS_ENABLED(CONFIG_##value) || IS_ENABLED(CPTCFG_##value) || \
		 IS_ENABLED(CONFIG_BACKPORT_##value))

extern struct ieee80211_supported_band mors_band_5ghz;

void morse_mac_send_buffered_bc(struct ieee80211_vif *vif);
struct morse *morse_mac_create(size_t priv_size, struct device *dev);
void morse_mac_destroy(struct morse *mors);
void morse_mac_skb_recv(struct morse *mors, struct sk_buff *skb,
		       struct morse_skb_rx_status *hdr_rx_status);
int morse_mac_event_recv(struct morse *mors, struct sk_buff *skb);
int morse_mac_register(struct morse *mors);
void morse_mac_unregister(struct morse *mors);
void morse_mac_rx_status(struct morse *mors,
			 const struct morse_skb_rx_status *hdr_rx_status,
			 struct ieee80211_rx_status *rx_status, struct sk_buff *skb);
void morse_mac_skb_free(struct morse *mors, struct sk_buff *skb);

void morse_mac_update_custom_s1g_capab(struct morse_vif *mors_vif,
				       struct dot11ah_ies_mask *ies_mask,
				       enum nl80211_iftype vif_type);
int morse_mac_pkt_to_s1g(struct morse *mors, const struct ieee80211_sta *sta,
			 struct sk_buff **skb, int *tx_bw_mhz);

/**
 * morse_mac_ps_enabled() - Check whether powersave can be enabled.
 * @mors: Global Morse structure
 *
 * Return: true if powersave can be enabled.
 */
bool morse_mac_ps_enabled(struct morse *mors);

/**
 * Get slow clock mode
 *
 * Return: Slow clock mode. It will be a value from @ref enum morse_cmd_slow_clock_mode
 */
enum morse_cmd_slow_clock_mode morse_mac_slow_clock_mode(void);

int morse_mac_watchdog_create(struct morse *mors);
void morse_mac_mcs0_10_stats_dump(struct morse *mors, struct seq_file *file);
void morse_mac_fill_tx_info(struct morse *mors, struct morse_skb_tx_info *tx_info,
				   struct sk_buff *skb, struct ieee80211_vif *vif,
				   int tx_bw_mhz, struct ieee80211_sta *sta);

bool is_fullmac_mode(void);
bool is_thin_lmac_mode(void);
bool is_virtual_sta_test_mode(void);
bool is_sw_crypto_mode(void);

/* Return a pointer to vif from vif id of tx status */
struct ieee80211_vif *morse_get_vif_from_tx_status(struct morse *mors,
						   struct morse_skb_tx_status *hdr_tx_status);

/* Return a pointer to vif from vif id */
struct ieee80211_vif *morse_get_vif_from_vif_id(struct morse *mors, int vif_id);
/* same as above but does not take the vif array lock */
struct ieee80211_vif *__morse_get_vif_from_vif_id(struct morse *mors, int vif_id);

/**
 * Return a pointer to the 1st valid VIF.
 * NOTE: Please don't use this func. This will be deprecated soon.
 */
struct ieee80211_vif *morse_get_vif(struct morse *mors);

/* Return a pointer to vif from vif id of rx status */
struct ieee80211_vif *morse_get_vif_from_rx_status(struct morse *mors,
						const struct morse_skb_rx_status *hdr_rx_status);

/* Return a pointer to the AP vif if present otherwise NULL */
struct ieee80211_vif *morse_get_ap_vif(struct morse *mors);

/* Return a pointer to the STA vif if present otherwise NULL */
struct ieee80211_vif *morse_get_sta_vif(struct morse *mors);

/* Return a pointer to the IBSS vif if present otherwise NULL */
struct ieee80211_vif *morse_get_ibss_vif(struct morse *mors);

/* Return a pointer to the MESH vif if present otherwise NULL */
struct ieee80211_vif *morse_get_mesh_vif(struct morse *mors);

/* Return iface name for the valid vif */
char *morse_vif_name(struct ieee80211_vif *vif);
/**
 * @brief Determine if the iface is AP type (AP or Ad-hoc or Mesh Point).
 *
 * @param vif Interface pointer to check
 *
 * @returns TRUE if iface is AP type
 */
static inline bool morse_mac_is_iface_ap_type(struct ieee80211_vif *vif)
{
	return (vif &&
		(vif->type == NL80211_IFTYPE_AP ||
		 vif->type == NL80211_IFTYPE_ADHOC ||
		 ieee80211_vif_is_mesh(vif)));
}

/**
 * @brief Determine if the iface is Infrastructure BSS type (AP or STA).
 *
 * @param vif Interface pointer to check
 *
 * @returns TRUE if iface is Infrastructure BSS type
 */
static inline bool morse_mac_is_iface_infra_bss_type(struct ieee80211_vif *vif)
{
	return (vif &&
		(vif->type == NL80211_IFTYPE_AP ||
		 vif->type == NL80211_IFTYPE_STATION));
}

/**
 * @brief Determine if the iface is valid type.
 *
 * @param vif Interface pointer to check
 *
 * @returns TRUE if iface is valid type
 */
static inline bool morse_mac_is_iface_type_supported(struct ieee80211_vif *vif)
{
	/* Station, AP, Adhoc or Mesh Point */
	return ((vif->type == NL80211_IFTYPE_STATION) || morse_mac_is_iface_ap_type(vif));
}

/**
 * @brief Get the association status of the VIF.
 *
 * @param vif Interface pointer to check
 *
 * @returns TRUE if status is associated
 */
static inline bool morse_mac_is_sta_vif_associated(struct ieee80211_vif *vif)
{
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
		return vif->bss_conf.assoc;
#else
		return vif->cfg.assoc;
#endif
}

/**
 * @brief Get if a channel switch operation is active.
 *
 * @param vif Interface pointer to check
 *
 * @returns TRUE if channel switch is active
 */
static inline bool morse_mac_is_csa_active(struct ieee80211_vif *vif)
{
#if KERNEL_VERSION(5, 19, 2) > MAC80211_VERSION_CODE
		return vif->csa_active;
#else
		return vif->bss_conf.csa_active;
#endif
}

/**
 * @brief Get channel context for VIF.
 *
 * @param vif Interface pointer to get channel context from
 *
 * @returns Pointer to channel context
 */
static inline
struct ieee80211_chanctx_conf *morse_mac_get_chantx_conf(struct ieee80211_vif *vif)
{
#if KERNEL_VERSION(5, 19, 2) > MAC80211_VERSION_CODE
	return rcu_access_pointer(vif->chanctx_conf);
#else
	return rcu_access_pointer(vif->bss_conf.chanctx_conf);
#endif
}

/**
 * @brief Get if a new IBSS network is being created.
 *
 * @param vif Interface pointer to check
 *
 * @returns TRUE if there is an IBSS creator
 */
static inline bool morse_mac_is_ibss_creator(struct ieee80211_vif *vif)
{
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
		return vif->bss_conf.ibss_creator;
#else
		return vif->cfg.ibss_creator;
#endif
}

/**
 * @brief Get HT capabilities of STA.
 *
 * @param sta Station context
 *
 * @returns Pointer to HT capabilities
 */
static inline struct ieee80211_sta_ht_cap *morse_mac_sta_ht_cap(struct ieee80211_sta *sta)
{
#if KERNEL_VERSION(5, 19, 0) > MAC80211_VERSION_CODE
	return &sta->ht_cap;
#else
	return &sta->deflink.ht_cap;
#endif
}

/**
 * @brief Get VHT capabilities of STA.
 *
 * @param sta Station context
 *
 * @returns Pointer to VHT capabilities
 */
static inline struct ieee80211_sta_vht_cap *morse_mac_sta_vht_cap(struct ieee80211_sta *sta)
{
#if KERNEL_VERSION(5, 19, 0) > MAC80211_VERSION_CODE
	return &sta->vht_cap;
#else
	return &sta->deflink.vht_cap;
#endif
}

/**
 * @brief Set HT supported flag for STA.
 *
 * @param sta Station context
 */
static inline void morse_mac_sta_set_ht_support(struct ieee80211_sta *sta, bool supported)
{
#if KERNEL_VERSION(5, 19, 0) > MAC80211_VERSION_CODE
		sta->ht_cap.ht_supported = supported;
#else
		sta->deflink.ht_cap.ht_supported = supported;
#endif
}

/**
 * @brief Get association ID for STA
 *
 * @param vif Interface pointer to get AID
 *
 * @returns Valid AID
 */
static inline u16 morse_mac_sta_aid(struct ieee80211_vif *vif)
{
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
	return vif->bss_conf.aid;
#else
	return vif->cfg.aid;
#endif
}

/**
 * @brief Get CSSID from SSID and its length for VIF
 *
 * @param vif Interface pointer to VIF
 *
 * @returns Derived CSSID
 */

static inline u32 morse_vif_generate_cssid(struct ieee80211_vif *vif)
{
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
	return morse_generate_cssid(vif->bss_conf.ssid, vif->bss_conf.ssid_len);
#else
	return morse_generate_cssid(vif->cfg.ssid, vif->cfg.ssid_len);
#endif
}

/**
 * @brief Notify RSSI event to mac80211
 *
 * @param vif Interface pointer to VIF
 * @param event The RSSI event
 * @param rssi Current RSSI level
 * @param gfp context flags
 */
static inline void morse_mac_cqm_rssi_notify(struct ieee80211_vif *vif,
					enum nl80211_cqm_rssi_threshold_event event, s32 rssi,
					gfp_t gfp)
{
#if KERNEL_VERSION(4, 11, 0) <= MAC80211_VERSION_CODE
	ieee80211_cqm_rssi_notify(vif, event, rssi, GFP_KERNEL);
#else
	ieee80211_cqm_rssi_notify(vif, event, GFP_KERNEL);
#endif
}

/**
 * @brief Check if MESH config is enabled and set in interface modes
 *
 * @param wiphy the wiphy device registered with cfg80211
 *
 * @returns TRUE if enabled, else FALSE
 */
static inline bool morse_mac_mesh_enabled(struct wiphy *wiphy)
{
	return MESH_CONFIG_ENABLED(MAC80211_MESH) &&
		(wiphy->interface_modes & BIT(NL80211_IFTYPE_MESH_POINT));
}

bool morse_mac_is_subband_enable(void);
uint morse_mac_get_max_rate_tries(void);
uint morse_mac_get_max_rate(void);

int morse_mac_get_watchdog_interval_secs(void);

int morse_mac_send_vendor_wake_action_frame(struct morse *mors, const u8 *dest_addr,
					    const u8 *payload, int payload_len);

int morse_mac_traffic_control(struct morse *mors, int interface_id,
			      bool pause_data_traffic, int sources);

int morse_cqm_rssi_notify_event(struct morse *mors, struct ieee80211_vif *vif,
		struct morse_cmd_evt_cqm_rssi_notify *cqm_notify);

/**
 * Function for filling the Tx meta info (rate info) for driver
 * generated management frames.
 */
void morse_fill_tx_info(struct morse *mors,
			struct morse_skb_tx_info *tx_info,
			struct sk_buff *skb, struct morse_vif *mors_vif, int tx_bw_mhz);

/* Process ECSA IE and store the channel info. Also starts chan switch timer in sta mode */
void morse_mac_process_ecsa_ie(struct morse *mors, struct ieee80211_vif *vif, struct sk_buff *skb);

/**
 * morse_mac_ecsa_beacon_tx_done - Process tx status completion of beacon to trigger the
 * channel switch.
 *
 * @mors: pointer to morse struct
 * @skb: pointer to beacon buffer
 *
 * Return: None.
 */
void morse_mac_ecsa_beacon_tx_done(struct morse *mors, struct sk_buff *skb);

s32 morse_mac_set_txpower(struct morse *mors, s32 power);

/**
 * morse_mac_get_ie_pos() - Parse 802.11n and S1G skb data header and tail positions
 * @skb: The SKB to get positions from
 * @ies_len: Total length of the skb information elements.
 * @header_length: Total length skb 802.11 header.
 * @is_s1g_pkt: Indicates if the skb data is an s1g packet
 *
 * Return: the start position of the information elements if management
 * frame / s1g beacon, else NULL
 */
u8 *morse_mac_get_ie_pos(struct sk_buff *skb, int *ies_len, int *header_length, bool is_s1g_pkt);

/**
 * morse_mac_tx_mgmt_frame - Utility func to transmit driver generated management frames
 *
 * @vif: pointer to the virtual interface
 * @skb: pointer to the management packet buffer
 *
 * Return: 0 on success, else relevant error
 */
int morse_mac_tx_mgmt_frame(struct ieee80211_vif *vif, struct sk_buff *skb);

/**
 * morse_mac_get_tx_attempts - Utility func to calculate tx attempts from status
 *
 * @mors: pointer to morse struct
 * @tx_sts: Tx status
 *
 * Return: 0 on success, else relevant error
 */
int morse_mac_get_tx_attempts(struct morse *mors, struct morse_skb_tx_status *tx_sts);

/**
 * morse_mac_process_tx_finish - Process Tx completion of frames
 *
 * @mors: pointer to morse struct
 * @skb:  pointer to the management packet buffer
 */
void morse_mac_process_tx_finish(struct morse *mors, struct sk_buff *skb);

u64 morse_mac_generate_timestamp_for_frame(struct morse_vif *mors_vif);

/**
 * morse_mac_is_1mhz_probe_req_enabled - Are 1MHz probe requests enabled.
 *
 * Return: true if enabled
 */
bool morse_mac_is_1mhz_probe_req_enabled(void);

u8 morse_mac_get_mcs10_mode(void);
u16 morse_mac_get_mcs_mask(void);
bool morse_mac_is_rts_8mhz_enabled(void);

#endif /* !_MORSE_MAC_H_ */
