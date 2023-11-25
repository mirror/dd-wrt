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

/* Description:  This file defines host interface data structure. */

#ifndef _HIF_H_
#define _HIF_H_

/* memory/register access */
#define MWL_ACCESS_MAC        1
#define MWL_ACCESS_RF         2
#define MWL_ACCESS_BBP        3
#define MWL_ACCESS_CAU        4
#define MWL_ACCESS_ADDR0      5
#define MWL_ACCESS_ADDR1      6
#define MWL_ACCESS_ADDR       7

struct mwl_survey_info {
	struct ieee80211_channel channel;
	u32 filled;
	u64 time_period;
	u64 time_busy;
	u64 time_tx;
	u64 time_rx;
	s8 noise;
};

struct mwl_hif_ops {
	const char *driver_name;
	const char *driver_version;
	unsigned int tx_head_room;
	int ampdu_num;
	void (*reset)(struct ieee80211_hw *hw);
	int (*init)(struct ieee80211_hw *hw);
	void (*deinit)(struct ieee80211_hw *hw);
	int (*get_info)(struct ieee80211_hw *hw, char *buf, size_t size);
	int (*get_tx_status)(struct ieee80211_hw *hw, char *buf, size_t size);
	int (*get_rx_status)(struct ieee80211_hw *hw, char *buf, size_t size);
	void (*enable_data_tasks)(struct ieee80211_hw *hw);
	void (*disable_data_tasks)(struct ieee80211_hw *hw);
	int (*exec_cmd)(struct ieee80211_hw *hw, unsigned short cmd);
	int (*get_irq_num)(struct ieee80211_hw *hw);
	irqreturn_t (*irq_handler)(struct ieee80211_hw *hw);
	void (*irq_enable)(struct ieee80211_hw *hw);
	void (*irq_disable)(struct ieee80211_hw *hw);
	int (*download_firmware)(struct ieee80211_hw *hw);
	void (*timer_routine)(struct ieee80211_hw *hw);
	void (*tx_xmit)(struct ieee80211_hw *hw,
			struct ieee80211_tx_control *control,
			struct sk_buff *skb);
	void (*tx_del_pkts_via_vif)(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif);
	void (*tx_del_pkts_via_sta)(struct ieee80211_hw *hw,
				    struct ieee80211_sta *sta);
	void (*tx_del_ampdu_pkts)(struct ieee80211_hw *hw,
				  struct ieee80211_sta *sta, u8 desc_num);
	void (*tx_del_sta_amsdu_pkts)(struct ieee80211_hw *hw,
				      struct ieee80211_sta *sta);
	void (*tx_return_pkts)(struct ieee80211_hw *hw);
	struct device_node *(*get_device_node)(struct ieee80211_hw *hw);
	void (*get_survey)(struct ieee80211_hw *hw,
			   struct mwl_survey_info *survey_info);
	int (*reg_access)(struct ieee80211_hw *hw, bool write);
	void (*set_sta_id)(struct ieee80211_hw *hw,
			   struct ieee80211_sta *sta,
			   bool sta_mode, bool set);
	void (*process_account)(struct ieee80211_hw *hw);
	int (*mcast_cts)(struct ieee80211_hw *hw, bool enable);
};
#endif /* _HIF_H_ */
