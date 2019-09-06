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

/* Description:  This file defines core layer related functions. */

#ifndef _CORE_H_
#define _CORE_H_

#include <linux/interrupt.h>
#include <linux/firmware.h>
#include <linux/of.h>
#include <net/mac80211.h>

#include "hif/hif.h"

/* antenna control */
#define ANTENNA_TX_4_AUTO             0
#define ANTENNA_TX_1                  1
#define ANTENNA_TX_2                  3
#define ANTENNA_TX_3                  7
#define ANTENNA_RX_4_AUTO             0
#define ANTENNA_RX_1                  1
#define ANTENNA_RX_2                  2
#define ANTENNA_RX_3                  3
#define ANTENNA_RX_MAX                4

/* band related constants */
#define BAND_24_CHANNEL_NUM           14
#define BAND_24_RATE_NUM              13
#define BAND_50_CHANNEL_NUM           24
#define BAND_50_RATE_NUM              8

#define NUM_WEP_KEYS                  4
#define MWL_MAX_TID                   8
#define MWL_AMSDU_SIZE_4K             1
#define MWL_AMSDU_SIZE_8K             2
#define MWL_AMSDU_SIZE_11K            3

/* power init */
#define MWL_POWER_INIT_1              1
#define MWL_POWER_INIT_2              2

/* tx rate information constants */
#define TX_RATE_FORMAT_LEGACY         0
#define TX_RATE_FORMAT_11N            1
#define TX_RATE_FORMAT_11AC           2

#define TX_RATE_BANDWIDTH_20          0
#define TX_RATE_BANDWIDTH_40          1
#define TX_RATE_BANDWIDTH_80          2
#define TX_RATE_BANDWIDTH_160         3

#define TX_RATE_INFO_STD_GI           0
#define TX_RATE_INFO_SHORT_GI         1

/* tx rate information */
/* 0: legacy format 1: 11n format 2: 11ac format */
#define MWL_TX_RATE_FORMAT_MASK       0x00000003
#define MWL_TX_RATE_STBC_MASK         0x00000004
#define MWL_TX_RATE_STBC_SHIFT        2
/* 0: 20 MHz 1: 40 MHz 2: 80 MHz 3: 160 MHz      */
#define MWL_TX_RATE_BANDWIDTH_MASK    0x00000030
#define MWL_TX_RATE_BANDWIDTH_SHIFT   4
/* 0: normal 1: short                            */
#define MWL_TX_RATE_SHORTGI_MASK      0x00000040
#define MWL_TX_RATE_SHORTGI_SHIFT     6
#define MWL_TX_RATE_RATEIDMCS_MASK    0x00007F00
#define MWL_TX_RATE_RATEIDMCS_SHIFT   8
/* 0: long   1: short                            */
#define MWL_TX_RATE_PREAMBLE_MASK     0x00008000
#define MWL_TX_RATE_PREAMBLE_SHIFT    15
#define MWL_TX_RATE_POWERID_MASK      0x003F0000
#define MWL_TX_RATE_POWERID_SHIFT     16
#define MWL_TX_RATE_ADVCODING_MASK    0x00400000
#define MWL_TX_RATE_ADVCODING_SHIFT   22
/* 0: beam forming off 1: beam forming on        */
#define MWL_TX_RATE_BF_MASK           0x00800000
#define MWL_TX_RATE_BF_SHIFT          23
#define MWL_TX_RATE_ANTSELECT_MASK    0xFF000000
#define MWL_TX_RATE_ANTSELECT_SHIFT   24

#define ACNT_BA_SIZE                  1000

/* Q stats */
#define QS_MAX_DATA_RATES_G           14
#define QS_NUM_SUPPORTED_11N_BW       2
#define QS_NUM_SUPPORTED_GI           2
#define QS_NUM_SUPPORTED_MCS          24
#define QS_NUM_SUPPORTED_11AC_NSS     3
#define QS_NUM_SUPPORTED_11AC_BW      4
#define QS_NUM_SUPPORTED_11AC_MCS     10
#define TX_RATE_HISTO_CUSTOM_CNT      1
#define TX_RATE_HISTO_PER_CNT         5
#define MAX_DATA_RATES_G              14
#define MAX_SUPPORTED_MCS             24
#define MAX_SUPPORTED_11AC_RATES      20
/* MAX_DATA_RATES_G + MAX_SUPPORTED_MCS + MAX_SUPPORTED_11AC_RATES */
#define MAX_SUPPORTED_RATES           58
#define SU_MIMO                       0
#define MU_MIMO                       1
#define SU_MU_TYPE_CNT                2 /* traffic type, SU and MU */

/* BF operation mode */
#define TXBF_MODE_OFF                 0x05
#define TXBF_MODE_AUTO                0x06
#define TXBF_MODE_BFMER_AUTO          0x07

static const u8 TX_HISTO_PER_THRES[TX_RATE_HISTO_PER_CNT - 1] = {6, 12, 20, 30};

enum {
	MWL8864 = 0,
	MWL8897,
	MWL8964,
	MWL8997,
	MWLUNKNOWN,
};

enum mwl_bus {
	MWL_BUS_PCIE,
	MWL_BUS_SDIO,
};

enum {
	AP_MODE_11AC = 0x10,         /* generic 11ac indication mode */
	AP_MODE_2_4GHZ_11AC_MIXED = 0x17,
};

enum {
	AMPDU_NO_STREAM = 0,
	AMPDU_STREAM_NEW,
	AMPDU_STREAM_IN_PROGRESS,
	AMPDU_STREAM_ACTIVE,
};

enum {
	LED_BLINK_RATE_LOW = 0x1,
	LED_BLINK_RATE_MID,
	LED_BLINK_RATE_HIGH,
};

struct mwl_chip_info {
	const char *part_name;
	const char *fw_image;
	const char *cal_file;
	const char *txpwrlmt_file;
	int antenna_tx;
	int antenna_rx;
};

struct mwl_device_pwr_tbl {
	u8 channel;
	u8 tx_pwr[SYSADPT_TX_PWR_LEVEL_TOTAL_SC4];
	u8 dfs_capable;
	u8 ax_ant;
	u8 cdd;
};

struct mwl_tx_pwr_tbl {
	u8 channel;
	u8 setcap;
	u16 txantenna2;
	u16 tx_power[SYSADPT_TX_POWER_LEVEL_TOTAL];
	bool cdd;
};

struct mwl_hw_data {
	u32 fw_release_num;          /* MajNbr:MinNbr:SubMin:PatchLevel */
	u8 hw_version;               /* plain number indicating version */
	unsigned char mac_addr[ETH_ALEN]; /* well known -> AA:BB:CC:DD:EE:FF */
};

struct mwl_ampdu_stream {
	struct ieee80211_sta *sta;
	u8 tid;
	u8 state;
	int idx;
};

struct mwl_stnid {
	int macid;                  /* keep macid for related stnid */
	u16 aid;                    /* keep aid for related stnid   */
};

struct otp_data {
	u8 buf[SYSADPT_OTP_BUF_SIZE];
	u32 len; /* Actual size of data in buf[] */
};

struct txpwrlmt_cfg_data {
	u8 buf[SYSADPT_TXPWRLMT_CFG_BUF_SIZE];
	u32 len; /* Actual size of data in buf[] */
};

struct mwl_priv {
	struct ieee80211_hw *hw;
	struct device *dev;
	struct firmware *fw_ucode;
	struct firmware *cal_data;
	struct firmware *txpwrlmt_file;
	struct otp_data otp_data;
	struct txpwrlmt_cfg_data txpwrlmt_data;
	bool fw_device_pwrtbl;
	bool forbidden_setting;
	bool regulatory_set;
	u32 fw_region_code;
	char fw_alpha2[2];
	u8 number_of_channels;
	struct mwl_device_pwr_tbl device_pwr_tbl[SYSADPT_MAX_NUM_CHANNELS];
	int chip_type;

	bool use_short_slot;
	bool use_short_preamble;

	struct {
		enum mwl_bus bus;
		const struct mwl_hif_ops *ops;
		void *priv;
	} hif;

	struct device_node *dt_node;
	struct device_node *pwr_node;
	bool disable_2g;
	bool disable_5g;
	int antenna_tx;
	int antenna_rx;
	bool tx_amsdu;
	bool dump_hostcmd;
	bool dump_probe;

	struct mwl_tx_pwr_tbl tx_pwr_tbl[SYSADPT_MAX_NUM_CHANNELS];
	bool cdd;
	u16 txantenna2;
	u8 powinited;
	u8 pwr_level;
	u16 max_tx_pow[SYSADPT_TX_GRP_PWR_LEVEL_TOTAL]; /* max tx power (dBm) */
	u16 target_powers[SYSADPT_TX_GRP_PWR_LEVEL_TOTAL]; /* target powers   */

	struct mutex fwcmd_mutex;    /* for firmware command         */
	unsigned short *pcmd_buf;    /* pointer to CmdBuf (virtual)  */
	dma_addr_t pphys_cmd_buf;    /* pointer to CmdBuf (physical) */
	bool in_send_cmd;
	bool cmd_timeout;
	bool rmmod;
	int heartbeat;
	u32 pre_jiffies;
	bool heartbeating;
	struct work_struct heartbeat_handle;

	int irq;
	struct mwl_hw_data hw_data;  /* Adapter HW specific info     */

	struct timer_list period_timer;

	/* keep survey information */
	bool sw_scanning;
	int survey_info_idx;
	struct mwl_survey_info survey_info[SYSADPT_MAX_NUM_CHANNELS];
	struct mwl_survey_info cur_survey_info;

	s8 noise;                    /* Most recently reported noise in dBm */

	struct ieee80211_supported_band band_24;
	struct ieee80211_channel channels_24[BAND_24_CHANNEL_NUM];
	struct ieee80211_rate rates_24[BAND_24_RATE_NUM];
	struct ieee80211_supported_band band_50;
	struct ieee80211_channel channels_50[BAND_50_CHANNEL_NUM];
	struct ieee80211_rate rates_50[BAND_50_RATE_NUM];

	u32 ap_macids_supported;
	u32 sta_macids_supported;
	u32 macids_used;
	u32 running_bsses;           /* bitmap of running BSSes             */

	struct {
		spinlock_t vif_lock;         /* for private interface info  */
		struct list_head vif_list;   /* List of interfaces.         */
	} ____cacheline_aligned_in_smp;

	struct {
		spinlock_t sta_lock;         /* for private sta info        */
		struct list_head sta_list;   /* List of stations            */
	} ____cacheline_aligned_in_smp;

	/* ampdu stream information */
	/* for ampdu stream */
	int ampdu_num;
	struct {
		spinlock_t stream_lock;      /* for BA stream               */
		struct mwl_ampdu_stream *ampdu;
	} ____cacheline_aligned_in_smp;
	struct work_struct watchdog_ba_handle;

	/* station id */
	int stnid_num;
	struct {
		spinlock_t stnid_lock;      /* for station id              */
		struct mwl_stnid *stnid;
		u16 available_stnid;
	} ____cacheline_aligned_in_smp;

	bool radio_on;
	bool radio_short_preamble;
	bool wmm_enabled;
	struct ieee80211_tx_queue_params wmm_params[SYSADPT_TX_WMM_QUEUES];

	struct work_struct account_handle;

	bool wds_check;
	struct work_struct wds_check_handle;
	u8 wds_check_sta[ETH_ALEN];

	bool dfs_test;
	bool csa_active;
	struct work_struct chnl_switch_handle;
	enum nl80211_dfs_regions dfs_region;
	u16 dfs_chirp_count_min;
	u16 dfs_chirp_time_interval;
	u16 dfs_pw_filter;
	u16 dfs_min_num_radar;
	u16 dfs_min_pri_count;

	u8 bf_type;

	struct thermal_cooling_device *cdev;
	u32 throttle_state;
	u32 quiet_period;
	int temperature;

	u8 led_blink_enable;
	u8 led_blink_rate;

	struct dentry *debugfs_phy;
	u32 reg_type;
	u32 reg_offset;
	u32 reg_value;
	int ra_aid;
	int ba_aid;
	int fixed_rate;
	bool coredump_text;
	u32 ra_tx_attempt[2][6];
};

struct beacon_info {
	bool valid;
	u16 cap_info;
	u8 power_constraint;
	u8 b_rate_set[SYSADPT_MAX_DATA_RATES_G];
	u8 op_rate_set[SYSADPT_MAX_DATA_RATES_G];
	u8 ie_list_ht[148];
	u8 ie_list_vht[24];
	u8 *ie_wmm_ptr;
	u8 *ie_wsc_ptr;
	u8 *ie_rsn_ptr;
	u8 *ie_rsn48_ptr;
	u8 *ie_mde_ptr;
	u8 *ie_ht_ptr;
	u8 *ie_vht_ptr;
	u8 *ie_country_ptr;
	u8 *ie_meshid_ptr;
	u8 *ie_meshcfg_ptr;
	u8 *ie_meshchsw_ptr;
	u8 ie_wmm_len;
	u8 ie_wsc_len;
	u8 ie_rsn_len;
	u8 ie_rsn48_len;
	u8 ie_mde_len;
	u8 ie_ht_len;
	u8 ie_vht_len;
	u8 ie_country_len;
	u8 ie_meshid_len;
	u8 ie_meshcfg_len;
	u8 ie_meshchsw_len;
};

struct mwl_vif {
	struct list_head list;
	enum nl80211_iftype type;
	int macid;       /* Firmware macid for this vif.  */
	u16 seqno;       /* Non AMPDU sequence number assigned by driver.  */
	struct {         /* Saved WEP keys */
		u8 enabled;
		u8 key[sizeof(struct ieee80211_key_conf) + WLAN_KEY_LEN_WEP104];
	} wep_key_conf[NUM_WEP_KEYS];
	u8 bssid[ETH_ALEN];          /* BSSID */
	u8 sta_mac[ETH_ALEN];        /* Station mac address */
	/* A flag to indicate is HW crypto is enabled for this bssid */
	bool is_hw_crypto_enabled;
	/* Indicate if this is station mode */
	struct beacon_info beacon_info;
	bool set_beacon;
	int basic_rate_idx;
	u8 broadcast_ssid;
	u16 iv16;
	u32 iv32;
	s8 keyidx;
};

struct mwl_tx_info {
	unsigned long start_time;
	u32 pkts;
};

struct mwl_amsdu_frag {
	struct sk_buff *skb;
	u8 *cur_pos;
	unsigned long jiffies;
	u8 pad;
	u8 num;
};

struct mwl_amsdu_ctrl {
	struct mwl_amsdu_frag frag[SYSADPT_TX_WMM_QUEUES];
	u8 cap;
};

struct mwl_tx_ba_stats {
	u8 ba_hole;     /* Total pkt not acked in a BA bitmap */
	u8 ba_expected; /* Total Tx pkt expected to be acked  */
	u8 no_ba;       /* No BA is received                  */
	u8 pad;         /* Unused                             */
};

struct mwl_tx_ba_hist {
	u16 index;      /* Current buffer index               */
	u8 type;        /* 0:SU, 1: MU                        */
	bool enable;
	struct mwl_tx_ba_stats *ba_stats;
};

struct mwl_tx_hist_data {
	u32 rateinfo;
	u32 cnt;
	/* store according to TX_HISTO_PER_THRES threshold    */
	u32 per[TX_RATE_HISTO_PER_CNT];
};

struct mwl_tx_hist {
	struct mwl_tx_hist_data su_rate[MAX_SUPPORTED_RATES];
	struct mwl_tx_hist_data mu_rate
		[QS_NUM_SUPPORTED_11AC_NSS - 1][QS_NUM_SUPPORTED_11AC_BW]
		[QS_NUM_SUPPORTED_GI][QS_NUM_SUPPORTED_11AC_MCS];
	struct mwl_tx_hist_data custom_rate[TX_RATE_HISTO_CUSTOM_CNT];
	/* Current rate for 0:SU, 1:MU         */
	u32 cur_rate_info[SU_MU_TYPE_CNT];
	/* Total tx attempt cnt for 0:SU, 1:MU */
	u32 total_tx_cnt[SU_MU_TYPE_CNT];
};

struct mwl_sta {
	struct list_head list;
	struct mwl_vif *mwl_vif;
	u16 stnid;
	u16 sta_stnid;
	bool wds;
	bool is_mesh_node;
	bool is_ampdu_allowed;
	struct mwl_tx_info tx_stats[MWL_MAX_TID];
	u32 check_ba_failed[MWL_MAX_TID];
	struct mwl_tx_ba_hist ba_hist;
	bool is_amsdu_allowed;
	bool is_key_set;
	/* for amsdu aggregation */
	struct {
		spinlock_t amsdu_lock;   /* for amsdu */
		struct mwl_amsdu_ctrl amsdu_ctrl;
	} ____cacheline_aligned_in_smp;
	struct mwl_tx_hist tx_hist;
	u32 tx_rate_info;
	u16 rx_format;
	u16 rx_nss;
	u16 rx_bw;
	u16 rx_gi;
	u16 rx_rate_mcs;
	u8 rx_signal;
	u16 iv16;
	u32 iv32;
};

static inline struct mwl_vif *mwl_dev_get_vif(const struct ieee80211_vif *vif)
{
	return (struct mwl_vif *)&vif->drv_priv;
}

static inline struct mwl_sta *mwl_dev_get_sta(const struct ieee80211_sta *sta)
{
	return (struct mwl_sta *)&sta->drv_priv;
}

struct ieee80211_hw *mwl_alloc_hw(int bus_type,
				  int chip_type,
				  struct device *dev,
				  const struct mwl_hif_ops *ops,
				  size_t hif_data_len);

void mwl_free_hw(struct ieee80211_hw *hw);

int mwl_init_hw(struct ieee80211_hw *hw, const char *fw_name,
		const char *cal_name, const char *txpwrlmt_name);

void mwl_deinit_hw(struct ieee80211_hw *hw);

/* Defined in mac80211.c. */
extern const struct ieee80211_ops mwl_mac80211_ops;

#endif /* _CORE_H_ */
