#ifndef _MORSE_COMMAND_H_
#define _MORSE_COMMAND_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include "morse.h"
#include "morse_commands.h"
#include "wiphy.h"

#define MORSE_CMD_IS_REQ(cmd)	(le16_to_cpu((cmd)->hdr.flags) & MORSE_CMD_TYPE_REQ)
#define MORSE_CMD_IS_RESP(cmd)	(le16_to_cpu((cmd)->hdr.flags) & MORSE_CMD_TYPE_RESP)
#define MORSE_CMD_IS_EVT(cmd)	(le16_to_cpu((cmd)->hdr.flags) & MORSE_CMD_TYPE_EVT)

/* Default IBSS ACK Timeout adjustment in usecs */
#define DEFAULT_MORSE_IBSS_ACK_TIMEOUT_ADJUST_US (1000)

struct morse_twt_agreement_data;
struct morse_queue_params;

/**
 * Error numbers the FW may return from commands
 * Defined here as errno numbers are not portable across architectures
 */
enum morse_cmd_return_code {
	MORSE_RET_SUCCESS	  = 0,
	MORSE_RET_EPERM		  = -1,
	MORSE_RET_ENOMEM	  = -12,
	MORSE_RET_CMD_NOT_HANDLED = -32757,
};

#define MORSE_CMD_HOST_ID_SEQ_MAX	0xfff
#define MORSE_CMD_HOST_ID_RETRY_MASK	0x000f
#define MORSE_CMD_HOST_ID_SEQ_SHIFT	4
#define MORSE_CMD_HOST_ID_SEQ_MASK	0xfff0

struct morse_cmd_req {
	struct morse_cmd_header hdr;
	u8 data[];
} __packed;

struct morse_cmd_resp {
	struct morse_cmd_header hdr;
	__le32 status;
	u8 data[];
} __packed;

struct morse_cmd_req_vendor {
	struct morse_cmd_header hdr;
	u8 data[2048];
} __packed;

struct morse_cmd_resp_vendor {
	struct morse_cmd_header hdr;
	__le32 status;
	u8 data[2048];
} __packed;

struct morse_cmd_event {
	struct morse_cmd_header hdr;
	u8 data[];
} __packed;

int morse_cmd_set_duty_cycle(struct morse *mors, enum morse_cmd_duty_cycle_mode mode,
			     int duty_cycle, bool omit_ctrl_resp);
int morse_cmd_set_mpsw(struct morse *mors, int min, int max, int window);

int morse_cmd_set_ps(struct morse *mors, bool enabled, bool enable_dynamic_ps_offload);
int morse_cmd_set_txpower(struct morse *mors, s32 *out_power, int txpower);
int morse_cmd_get_max_txpower(struct morse *mors, s32 *out_power);
int morse_cmd_add_if(struct morse *mors, u16 *id, const u8 *addr, enum nl80211_iftype type);
int morse_cmd_rm_if(struct morse *mors, u16 id);
int morse_cmd_resp_process(struct morse *mors, struct sk_buff *skb);
int morse_cmd_cfg_bss(struct morse *mors, u16 id, u16 beacon_int, u16 dtim_period, u32 cssid);

/**
 * morse_cmd_vendor() - Handle vendor command sent to the virtual interface (e.g. wlan0)
 *
 * @mors: morse chip struct
 * @mors_vif: morse interface
 * @cmd: vendor command request
 * @cmd_len: length of the request
 * @resp: vendor command response
 * @resp_len: length of the response
 *
 * @return 0 on success, else error code
 */
int morse_cmd_vendor(struct morse *mors, struct morse_vif *mors_vif,
		     const struct morse_cmd_req_vendor *cmd, int cmd_len,
		     struct morse_cmd_resp_vendor *resp, int *resp_len);

/**
 * morse_hw_cmd_vendor() - Handle vendor command sent to the wireless device (e.g. phy0)
 *
 * @mors: morse chip struct
 * @cmd: vendor command request
 * @cmd_len: length of the request
 * @resp: vendor command response
 * @resp_len: length of the response
 *
 * @return 0 on success, else error code
 */
int morse_hw_cmd_vendor(struct morse *mors,
			const struct morse_cmd_req_vendor *cmd, int cmd_len,
			struct morse_cmd_resp_vendor *resp, int *resp_len);
int morse_cmd_set_channel(struct morse *mors, u32 op_chan_freq_hz,
			  u8 pri_1mhz_chan_idx, u8 op_bw_mhz, u8 pri_bw_mhz, s32 *power_dbm);
int morse_cmd_get_current_channel(struct morse *mors, u32 *op_chan_freq_hz,
				  u8 *pri_1mhz_chan_idx, u8 *op_bw_mhz, u8 *pri_bw_mhz);
int morse_cmd_get_version(struct morse *mors);
int morse_cmd_cfg_scan(struct morse *mors, bool enabled, bool is_survey);
int morse_cmd_get_channel_usage(struct morse *mors, struct morse_survey_rx_usage_record *record);

int morse_cmd_sta_state(struct morse *mors, struct morse_vif *mors_vif,
			u16 aid, struct ieee80211_sta *sta, enum ieee80211_sta_state state);
int morse_cmd_disable_key(struct morse *mors, struct morse_vif *mors_vif,
			  u16 aid, struct ieee80211_key_conf *key);
int morse_cmd_install_key(struct morse *mors, struct morse_vif *mors_vif,
			  u16 aid, struct ieee80211_key_conf *key, enum morse_cmd_key_cipher cipher,
			  enum morse_cmd_aes_key_len length);
int morse_cmd_set_cr_bw(struct morse *mors, struct morse_vif *mors_vif, u8 direction,
			u8 cr_1mhz_en);
int morse_cmd_cfg_qos(struct morse *mors, struct morse_queue_params *params);
int morse_cmd_set_bss_color(struct morse *mors, struct morse_vif *mors_vif, u8 color);
int morse_cmd_health_check(struct morse *mors);
int morse_cmd_arp_offload_update_ip_table(struct morse *mors, u16 vif_id,
					  int arp_addr_count, __be32 *arp_addr_list);
int morse_cmd_get_capabilities(struct morse *mors,
			       u16 vif_id, struct morse_caps *capabilities);
/**
 * morse_cmd_config_non_tim_mode() - Configure non-TIM mode.
 *
 * @mors: morse chip struct
 * @enable: enable or disable non-TIM mode
 * @vif_id: interface id
 *
 * @return 0 on success, else error code
 */
int morse_cmd_config_non_tim_mode(struct morse *mors, bool enable, u16 vif_id);
int morse_cmd_enable_li_sleep(struct morse *mors,  u16 listen_interval, u16 vif_id);
int morse_cmd_dhcpc_enable(struct morse *mors, u16 vif_id);
int morse_cmd_twt_agreement_validate_req(struct morse *mors,
					 struct morse_twt_agreement_data *agreement, u16 iface_id);
int morse_cmd_twt_agreement_install_req(struct morse *mors,
					struct morse_twt_agreement_data *agreement, u16 iface_id);
int morse_cmd_twt_remove_req(struct morse *mors,
			     struct morse_cmd_req_twt_agreement_remove *twt_remove_cmd,
			     u16 iface_id);
int morse_cmd_cfg_ibss(struct morse *mors, u16 id,
		       const u8 *bssid, bool ibss_creator, bool stop_ibss);
int morse_cmd_cfg_offset_tsf(struct morse *mors, u16 vif_id, s64 offset_tsf);
int morse_cmd_config_beacon_timer(struct morse *mors, struct morse_vif *mors_vif, bool enabled);
int morse_cmd_store_pv1_hc_data(struct morse *mors, struct morse_vif *mors_vif,
				 struct ieee80211_sta *sta, u8 *a3, u8 *a4, bool is_store_in_rx);

/**
 * @brief Configure the OUI filter in the FW. If a beacon is received containing a vendor element
 * with an OUI matching one in the filter, it will unconditionally pass the beacon up to the host.
 *
 * @param mors Morse object
 * @param mors_vif morse interface
 * @return 0 on success, else error code
 */
int morse_cmd_update_beacon_vendor_ie_oui_filter(struct morse *mors, struct morse_vif *mors_vif);

int morse_cmd_cfg_multicast_filter(struct morse *mors, struct morse_vif *mors_vif);

int morse_cmd_get_available_channels(struct morse *mors, struct morse_cmd_resp *resp);
int morse_cmd_get_hw_version(struct morse *mors, struct morse_cmd_resp *resp);

int morse_cmd_set_frag_threshold(struct morse *mors, u32 frag_threshold);

/**
 * morse_cmd_cfg_mesh() -  Configure mesh bss parameters in the firmware.
 *
 * @mors: morse chip struct
 * @mors_vif: pointer to morse interface
 * @stop_mesh: Flag to start or stop the mesh interface.
 * @mesh_beaconing: Flag to enable or disable beaconing.
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_cfg_mesh(struct morse *mors, struct morse_vif *mors_vif, bool stop_mesh,
		       bool mesh_beaconing);

/**
 * morse_cmd_ack_timeout_adjust() - Configure ack timeout in the firmware.
 *
 * @mors: Morse object
 * @vif_id: interface id
 * @timeout_us: ACK timeout adjustment value in usecs
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_ack_timeout_adjust(struct morse *mors, u16 vif_id, u32 timeout_us);

/**
 * morse_cmd_pv1_set_rx_ampdu_state() - Configure RX AMPDU state for PV1 STA in the firmware.
 *
 * @mors_vif: morse interface object
 * @sta_addr: Station address for which BA session is established
 * @tid: TID for which BA session is established
 * @buf_size: A-MPDU reordering buffer size
 * @ba_session_enable: BA session enabled or disabled
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_pv1_set_rx_ampdu_state(struct morse_vif *mors_vif, u8 *sta_addr, u8 tid,
		u16 buf_size, bool ba_session_enable);

/**
 * morse_cmd_configure_page_slicing() - Configure page slicing in target.
 *
 * @mors_vif: morse interface object
 * @enable: enable status of page slicing.
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_configure_page_slicing(struct morse_vif *mors_vif, bool enable);

/**
 * morse_cmd_hw_scan() - Configure HW scanning
 *
 * @mors: Morse structure
 * @params: HW scan parameters
 * @store: Whether to save the HW scan configuration in chip (for standby mode)
 * @sched_req: Schedule scan request (can be null if not required)
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_hw_scan(struct morse *mors, struct morse_hw_scan_params *params, bool store,
		      struct cfg80211_sched_scan_request *sched_req);

/**
 * morse_cmd_get_disabled_channels() - Retrieve channels that are disabled by hardware.
 *
 * @mors: Morse structure
 * @resp: Allocated response to store disabled channels in
 * @resp_len: Size of allocated response buffer.
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_get_disabled_channels(struct morse *mors,
				    struct morse_cmd_resp_get_disabled_channels *resp,
				    uint resp_len);
int morse_cmd_set_slow_clock_mode(struct morse *mors, enum morse_cmd_slow_clock_mode mode);
int morse_cmd_set_country(struct morse *mors, const char *country_code);
int morse_cmd_set_rate_control(struct morse *mors);
int morse_cmd_set_fixed_transmission_rate(struct morse *mors, s32 bandwidth_mhz, s32 mcs_index,
					  s8 use_sgi, s8 nss_idx);
int morse_cmd_set_rts_threshold(struct morse *mors, u32 rts_threshold);
int morse_cmd_start_scan(struct morse *mors, u8 n_ssids, const u8 *ssid, size_t ssid_len,
			 const u8 *extra_ies, size_t extra_ies_len, u32 dwell_time_ms);
int morse_cmd_abort_scan(struct morse *mors);
int morse_cmd_connect(struct morse *mors, const struct morse_wiphy_connect_params *params);
int morse_cmd_disconnect(struct morse *mors);
int morse_cmd_get_connection_state(struct morse *mors, s8 *signal,
				   u32 *connected_time_s, u8 *dtim_period,
				   u16 *beacon_interval_tu);
int morse_cmd_set_cqm_rssi(struct morse *mors, u16 vif_id, s32 cqm_rssi_thold, u32 cqm_rssi_hyst);

/**
 * morse_cmd_get_apf_capabilities() - Get APF capabilities supported by firmware.
 *
 * @mors: Morse structure
 * @mors_vif: Pointer to morse vif struct.
 * @version: APF version
 * @max_len: Maximum length of the memory allocated for APF
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_get_apf_capabilities(struct morse *mors, struct morse_vif *mors_vif,
								   u32 *version, u32 *max_len);

/**
 * morse_cmd_read_write_apf() - Read or Write into APF memory
 *
 * @mors: Morse structure
 * @mors_vif: Pointer to morse vif struct.
 * @write: Indicates whether to read or write into packet filtering memory
 * @program_len: Length of the filter in bytes and valid only when write is set to 1
 * @program: Points to filter bytecode when write is set 1
 * @offset:  Offset in bytes within APF memory. Valid only when write is set to 0
 *
 * Return: 0 on success, else error code
 */
int morse_cmd_read_write_apf(struct morse *mors, struct morse_vif *mors_vif, bool write,
							 u16 program_len, u8 *program, u32 offset);

#endif /* !_MORSE_COMMAND_H_ */
