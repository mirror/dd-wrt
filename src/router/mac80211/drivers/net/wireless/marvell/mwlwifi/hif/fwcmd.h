/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file defines firmware host command related
 * functions.
 */

#ifndef _FWCMD_H_
#define _FWCMD_H_

#include "hif/hostcmd.h"

/*  Define OpMode for SoftAP/Station mode
 *
 *  The following mode signature has to be written to PCI scratch register#0
 *  right after successfully downloading the last block of firmware and
 *  before waiting for firmware ready signature
 */

#define HOSTCMD_STA_MODE                0x5A
#define HOSTCMD_SOFTAP_MODE             0xA5

#define HOSTCMD_STA_FWRDY_SIGNATURE     0xF0F1F2F4
#define HOSTCMD_SOFTAP_FWRDY_SIGNATURE  0xF1F2F4A5

#define GUARD_INTERVAL_STANDARD         1
#define GUARD_INTERVAL_SHORT            2
#define GUARD_INTERVAL_AUTO             3

#define	LINK_CS_STATE_CONSERV           0
#define	LINK_CS_STATE_AGGR              1
#define	LINK_CS_STATE_AUTO              2
#define	LINK_CS_STATE_AUTO_DISABLED     3

#define STOP_DETECT_RADAR               0
#define CAC_START                       1
#define MONITOR_START                   3

#define WDS_MODE                        4

enum {
	WL_ANTENNATYPE_RX = 1,
	WL_ANTENNATYPE_TX = 2,
};

enum encr_type {
	ENCR_TYPE_WEP = 0,
	ENCR_TYPE_DISABLE = 1,
	ENCR_TYPE_TKIP = 4,
	ENCR_TYPE_AES = 6,
	ENCR_TYPE_MIX = 7,
};

char *mwl_fwcmd_get_cmd_string(unsigned short cmd);

const struct hostcmd_get_hw_spec
*mwl_fwcmd_get_hw_specs(struct ieee80211_hw *hw);

int mwl_fwcmd_set_hw_specs(struct ieee80211_hw *hw,
			   struct hostcmd_set_hw_spec *spec);

int mwl_fwcmd_get_stat(struct ieee80211_hw *hw,
		       struct ieee80211_low_level_stats *stats);

int mwl_fwcmd_reg_bb(struct ieee80211_hw *hw, u8 flag, u32 reg, u32 *val);

int mwl_fwcmd_reg_rf(struct ieee80211_hw *hw, u8 flag, u32 reg, u32 *val);

int mwl_fwcmd_radio_enable(struct ieee80211_hw *hw);

int mwl_fwcmd_radio_disable(struct ieee80211_hw *hw);

int mwl_fwcmd_set_radio_preamble(struct ieee80211_hw *hw,
				 bool short_preamble);

int mwl_fwcmd_get_addr_value(struct ieee80211_hw *hw, u32 addr, u32 len,
			     u32 *val, u16 set);

int mwl_fwcmd_max_tx_power(struct ieee80211_hw *hw,
			   struct ieee80211_conf *conf, u8 fraction);

int mwl_fwcmd_tx_power(struct ieee80211_hw *hw,
		       struct ieee80211_conf *conf, u8 fraction);

int mwl_fwcmd_rf_antenna(struct ieee80211_hw *hw, int dir, int antenna);

int mwl_fwcmd_broadcast_ssid_enable(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif, bool enable);

int mwl_fwcmd_set_cfg_data(struct ieee80211_hw *hw, u16 type);

int mwl_fwcmd_set_rf_channel(struct ieee80211_hw *hw,
			     struct ieee80211_conf *conf);

int mwl_fwcmd_set_aid(struct ieee80211_hw *hw,
		      struct ieee80211_vif *vif, u8 *bssid, u16 aid);

int mwl_fwcmd_set_infra_mode(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif);

int mwl_fwcmd_set_rts_threshold(struct ieee80211_hw *hw,
				int threshold);

int mwl_fwcmd_set_edca_params(struct ieee80211_hw *hw, u8 index,
			      u16 cw_min, u16 cw_max, u8 aifs, u16 txop);

int mwl_fwcmd_set_radar_detect(struct ieee80211_hw *hw, u16 action);

int mwl_fwcmd_set_wmm_mode(struct ieee80211_hw *hw, bool enable);

int mwl_fwcmd_ht_guard_interval(struct ieee80211_hw *hw, u32 gi_type);

int mwl_fwcmd_use_fixed_rate(struct ieee80211_hw *hw,
			     int mcast, int mgmt);

int mwl_fwcmd_set_linkadapt_cs_mode(struct ieee80211_hw *hw,
				    u16 cs_mode);

int mwl_fwcmd_dump_otp_data(struct ieee80211_hw *hw);

int mwl_fwcmd_set_rate_adapt_mode(struct ieee80211_hw *hw,
				  u16 mode);

int mwl_fwcmd_set_mac_addr_client(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif, u8 *mac_addr);

int mwl_fwcmd_get_watchdog_bitmap(struct ieee80211_hw *hw,
				  u8 *bitmap);

int mwl_fwcmd_remove_mac_addr(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif, u8 *mac_addr);

int mwl_fwcmd_bss_start(struct ieee80211_hw *hw,
			struct ieee80211_vif *vif, bool enable);

int mwl_fwcmd_set_beacon(struct ieee80211_hw *hw,
			 struct ieee80211_vif *vif, u8 *beacon, int len);

int mwl_fwcmd_set_new_stn_add(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif,
			      struct ieee80211_sta *sta);

int mwl_fwcmd_set_new_stn_add_sc4(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif,
				  struct ieee80211_sta *sta,
				  u32 wds);

int mwl_fwcmd_set_new_stn_wds_sc4(struct ieee80211_hw *hw, u8 *addr);

int mwl_fwcmd_set_new_stn_add_self(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif);

int mwl_fwcmd_set_new_stn_del(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif, u8 *addr);

int mwl_fwcmd_set_apmode(struct ieee80211_hw *hw, u8 apmode);

int mwl_fwcmd_set_switch_channel(struct ieee80211_hw *hw,
				 struct ieee80211_channel_switch *ch_switch);

int mwl_fwcmd_update_encryption_enable(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif,
				       u8 *addr, u8 encr_type);

int mwl_fwcmd_encryption_set_key(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif, u8 *addr,
				 struct ieee80211_key_conf *key);

int mwl_fwcmd_encryption_remove_key(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif, u8 *addr,
				    struct ieee80211_key_conf *key);

int mwl_fwcmd_check_ba(struct ieee80211_hw *hw,
		       struct mwl_ampdu_stream *stream,
		       struct ieee80211_vif *vif,
		       u32 direction);

int mwl_fwcmd_create_ba(struct ieee80211_hw *hw,
			struct mwl_ampdu_stream *stream,
			struct ieee80211_vif *vif,
			u32 direction, u8 buf_size, u16 seqno, bool amsdu);

int mwl_fwcmd_destroy_ba(struct ieee80211_hw *hw,
			 struct mwl_ampdu_stream *stream,
			 u32 direction);

struct mwl_ampdu_stream *mwl_fwcmd_add_stream(struct ieee80211_hw *hw,
					      struct ieee80211_sta *sta,
					      u8 tid);

void mwl_fwcmd_del_sta_streams(struct ieee80211_hw *hw,
			       struct ieee80211_sta *sta);

int mwl_fwcmd_start_stream(struct ieee80211_hw *hw,
			   struct mwl_ampdu_stream *stream);

void mwl_fwcmd_remove_stream(struct ieee80211_hw *hw,
			     struct mwl_ampdu_stream *stream);

struct mwl_ampdu_stream *mwl_fwcmd_lookup_stream(struct ieee80211_hw *hw,
						 struct ieee80211_sta *sta,
						 u8 tid);

bool mwl_fwcmd_ampdu_allowed(struct ieee80211_sta *sta, u8 tid);

int mwl_fwcmd_set_optimization_level(struct ieee80211_hw *hw, u8 opt_level);

int mwl_fwcmd_set_wsc_ie(struct ieee80211_hw *hw, u8 len, u8 *data);

int mwl_fwcmd_get_ratetable(struct ieee80211_hw *hw, u8 *addr, u8 *rate_table,
			    u32 size, u8 type);

int mwl_fwcmd_get_seqno(struct ieee80211_hw *hw,
			struct mwl_ampdu_stream *stream, u16 *start_seqno);

int mwl_fwcmd_set_dwds_stamode(struct ieee80211_hw *hw, bool enable);

int mwl_fwcmd_set_fw_flush_timer(struct ieee80211_hw *hw, u32 value);

int mwl_fwcmd_set_cdd(struct ieee80211_hw *hw);

int mwl_fwcmd_set_bftype(struct ieee80211_hw *hw, int mode);

int mwl_fwcmd_reg_cau(struct ieee80211_hw *hw, u8 flag, u32 reg, u32 *val);

int mwl_fwcmd_get_temp(struct ieee80211_hw *hw, u32 *temp);

int mwl_fwcmd_led_ctrl(struct ieee80211_hw *hw, u8 enable, u8 rate);

int mwl_fwcmd_get_fw_region_code(struct ieee80211_hw *hw,
				 u32 *fw_region_code);

int mwl_fwcmd_get_device_pwr_tbl(struct ieee80211_hw *hw,
				 struct mwl_device_pwr_tbl *device_ch_pwrtbl,
				 u8 *region_code,
				 u8 *number_of_channels,
				 u32 channel_index);

int mwl_fwcmd_set_rate_drop(struct ieee80211_hw *hw, int enable,
			    int value, int staid);

int mwl_fwcmd_newdp_dmathread_start(struct ieee80211_hw *hw);

int mwl_fwcmd_get_fw_region_code_sc4(struct ieee80211_hw *hw,
				     u32 *fw_region_code);

int mwl_fwcmd_get_pwr_tbl_sc4(struct ieee80211_hw *hw,
			      struct mwl_device_pwr_tbl *device_ch_pwrtbl,
			      u8 *region_code,
			      u8 *number_of_channels,
			      u32 channel_index);

int mwl_fwcmd_quiet_mode(struct ieee80211_hw *hw, bool enable, u32 period,
			 u32 duration, u32 next_offset);

int mwl_fwcmd_core_dump_diag_mode(struct ieee80211_hw *hw, u16 status);

int mwl_fwcmd_get_fw_core_dump(struct ieee80211_hw *hw,
			       struct coredump_cmd *core_dump, char *buff);

int mwl_fwcmd_set_slot_time(struct ieee80211_hw *hw, bool short_slot);

int mwl_fwcmd_config_EDMACCtrl(struct ieee80211_hw *hw, int EDMAC_Ctrl);

int mwl_fwcmd_set_txpwrlmt_cfg_data(struct ieee80211_hw *hw);

int mwl_fwcmd_get_txpwrlmt_cfg_data(struct ieee80211_hw *hw);

int mwl_fwcmd_mcast_cts(struct ieee80211_hw *hw, u8 enable);

void mwl_fwcmd_get_survey(struct ieee80211_hw *hw, int idx);

#endif /* _FWCMD_H_ */
