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

/* Description:  This file implements debug fs related functions. */

#include <linux/debugfs.h>
#include <linux/etherdevice.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"
#include "thermal.h"
#include "hif/fwcmd.h"
#include "hif/hif-ops.h"
#include "debugfs.h"

#define MWLWIFI_DEBUGFS_ADD_FILE(name) do { \
	if (!debugfs_create_file(#name, 0644, priv->debugfs_phy, \
				 priv, &mwl_debugfs_##name##_fops)) \
		return; \
} while (0)

#define MWLWIFI_DEBUGFS_FILE_OPS(name) \
static const struct file_operations mwl_debugfs_##name##_fops = { \
	.read = mwl_debugfs_##name##_read, \
	.write = mwl_debugfs_##name##_write, \
	.open = simple_open, \
}

#define MWLWIFI_DEBUGFS_FILE_READ_OPS(name) \
static const struct file_operations mwl_debugfs_##name##_fops = { \
	.read = mwl_debugfs_##name##_read, \
	.open = simple_open, \
}

#define MWLWIFI_DEBUGFS_FILE_WRITE_OPS(name) \
static const struct file_operations mwl_debugfs_##name##_fops = { \
	.write = mwl_debugfs_##name##_write, \
	.open = simple_open, \
}

static const char chipname[MWLUNKNOWN][8] = {
	"88W8864",
	"88W8897",
	"88W8964",
	"88W8997"
};

static void dump_data(char *p, int size, int *len, u8 *data,
		      int data_len, char *title)
{
	int cur_byte = 0;
	int i;

	*len += scnprintf(p + *len, size - *len, "%s\n", title);

	for (cur_byte = 0; cur_byte < data_len; cur_byte += 8) {
		if ((cur_byte + 8) < data_len) {
			for (i = 0; i < 8; i++)
				*len += scnprintf(p + *len, size - *len,
						  "0x%02x ",
						  *(data + cur_byte + i));
			*len += scnprintf(p + *len, size - *len, "\n");
		} else {
			for (i = 0; i < (data_len - cur_byte); i++)
				*len += scnprintf(p + *len, size - *len,
						  "0x%02x ",
						  *(data + cur_byte + i));
			*len += scnprintf(p + *len, size - *len, "\n");
			break;
		}
	}
}

static void _dump_tx_hist_mu(char *p, int size, int *len, bool *printed,
			     u32 *total, u8 nss, u8 bw, u8 mcs, u8 sgi,
			     struct mwl_sta *sta_info)
{
	char *bw_str[4] = {"ht20", "ht40", "ht80", "ht160"};
	char *sgi_str[2] = {"lgi", "sgi"};
	struct mwl_tx_hist_data *tx_hist_data;
	u32 cnt, rateinfo, per0, per1, per2, per3, per4, ratemask;

	tx_hist_data = &sta_info->tx_hist.mu_rate[nss][bw][sgi][mcs];
	cnt = le32_to_cpu(tx_hist_data->cnt);
	rateinfo = le32_to_cpu(tx_hist_data->rateinfo);
	if (cnt && (rateinfo > 0)) {
		*total += cnt;
		per4 = le32_to_cpu(tx_hist_data->per[4]);
		per3 = le32_to_cpu(tx_hist_data->per[3]);
		per2 = le32_to_cpu(tx_hist_data->per[2]);
		per1 = le32_to_cpu(tx_hist_data->per[1]);
		per0 = le32_to_cpu(tx_hist_data->per[0]);
		if (!*printed) {
			*len += scnprintf(p + *len, size - *len,
				"%s %26s  <%2d %8s%2d%8s%2d%8s%2d%8s%2d\n",
				"MU_MIMO rate", " PER%", TX_HISTO_PER_THRES[0],
				">=", TX_HISTO_PER_THRES[0],
				">=", TX_HISTO_PER_THRES[1],
				">=", TX_HISTO_PER_THRES[2],
				">=", TX_HISTO_PER_THRES[3]);
			*len += scnprintf(p + *len, size - *len,
				"TOTAL MPDU tx pkt: %d\n",
				sta_info->tx_hist.total_tx_cnt[MU_MIMO]);
				*printed = true;
		}
		if ((rateinfo & 0x3) == 0)
			ratemask = 0xfff;
		else
			ratemask = 0xffff;
		if ((sta_info->tx_hist.cur_rate_info[MU_MIMO] & ratemask) ==
		    (rateinfo & ratemask))
			 /* mark as current rate */
			*len += scnprintf(p + *len, size - *len, "*");
		else
			*len += scnprintf(p + *len, size - *len, " ");
		*len += scnprintf(p + *len, size - *len,
			"%5s_%3s_%1dSS_MCS%2d: %10u, %9d, %9d, %9d, %9d, %9d\n",
			bw_str[bw], sgi_str[sgi], (nss + 1), mcs, cnt, per0,
			per1, per2, per3, per4);
	}
}

static void dump_tx_hist_mu(char *p, int size, int *len, bool *printed,
			    u32 *total, struct mwl_sta *sta_info)
{
	u8 nss, bw, mcs, sgi;

	for (nss = 0; nss < (QS_NUM_SUPPORTED_11AC_NSS - 1); nss++) {
		for (bw = 0; bw < QS_NUM_SUPPORTED_11AC_BW; bw++) {
			for (mcs = 0; mcs < QS_NUM_SUPPORTED_11AC_MCS; mcs++) {
				for (sgi = 0; sgi < QS_NUM_SUPPORTED_GI;
				     sgi++) {
					_dump_tx_hist_mu(p, size, len, printed,
							 total, nss, bw, mcs,
							 sgi, sta_info);
				}
			}
		}
	}
}


static void dump_tx_hist_su(char *p, int size, int *len, bool su, bool *printed,
			    u32 *total, struct mwl_sta *sta_info)
{
	int g_rate[14] = {1, 2, 5, 11, 22, 6, 9, 12, 18, 24, 36, 48, 54, 72};
	char *bw_str[4] = {"ht20", "ht40", "ht80", "ht160"};
	char *sgi_str[2] = {"lgi", "sgi"};
	char title_str[32];
	struct mwl_tx_hist *tx_hist;
	struct mwl_tx_hist_data *tx_hist_data;
	u32 j, loopcnt;
	u32 cnt, rateinfo, per0, per1, per2, per3, per4, ratemask;
	u8 format, bw, sgi, mcs, nss;

	tx_hist = &sta_info->tx_hist;
	if (su) {
		loopcnt = MAX_SUPPORTED_RATES;
		tx_hist_data = &tx_hist->su_rate[0];
	} else {
		loopcnt = TX_RATE_HISTO_CUSTOM_CNT;
		tx_hist_data = &tx_hist->custom_rate[0];
	}

	for (j = 0; j < loopcnt; j++) {
		cnt = le32_to_cpu(tx_hist_data[j].cnt);
		rateinfo = le32_to_cpu(tx_hist_data[j].rateinfo);
		if (cnt && (rateinfo > 0)) {
			*total += cnt;
			per4 = le32_to_cpu(tx_hist_data[j].per[4]);
			per3 = le32_to_cpu(tx_hist_data[j].per[3]);
			per2 = le32_to_cpu(tx_hist_data[j].per[2]);
			per1 = le32_to_cpu(tx_hist_data[j].per[1]);
			per0 = le32_to_cpu(tx_hist_data[j].per[0]);
			if (!*printed) {
				*len += scnprintf(p + *len, size - *len,
				"%s %26s  <%2d %8s%2d%8s%2d%8s%2d%8s%2d\n",
				su ? "SU_MIMO rate" : " Custom rate",
				" PER%", TX_HISTO_PER_THRES[0],
				">=", TX_HISTO_PER_THRES[0],
				">=", TX_HISTO_PER_THRES[1],
				">=", TX_HISTO_PER_THRES[2],
				">=", TX_HISTO_PER_THRES[3]);
			*len += scnprintf(p + *len, size - *len,
				"TOTAL MPDU tx pkt: %d\n",
				tx_hist->total_tx_cnt[SU_MIMO]);
				*printed = true;
			}
			format = rateinfo & MWL_TX_RATE_FORMAT_MASK;
			bw = (rateinfo & MWL_TX_RATE_BANDWIDTH_MASK) >>
				MWL_TX_RATE_BANDWIDTH_SHIFT;
			sgi = (rateinfo & MWL_TX_RATE_SHORTGI_MASK) >>
				MWL_TX_RATE_SHORTGI_SHIFT;
			mcs = (rateinfo & MWL_TX_RATE_RATEIDMCS_MASK) >>
				MWL_TX_RATE_RATEIDMCS_SHIFT;
			if (format == TX_RATE_FORMAT_LEGACY)
				ratemask = 0xfff;
			else
				ratemask = 0xffff;
			if ((tx_hist->cur_rate_info[SU_MIMO] & ratemask) ==
			    (rateinfo & ratemask))
				/* mark as current rate */
				*len += scnprintf(p + *len, size - *len, "*");
			else
				*len += scnprintf(p + *len, size - *len, " ");
			if (format == TX_RATE_FORMAT_LEGACY) {
				if (mcs == 2) {
					*len += scnprintf(p + *len, size - *len,
					"%s %10u, %9d, %9d, %9d, %9d, %9d\n",
					"5.5Mbps             :", cnt, per0,
					per1, per2, per3, per4);
				} else {
					sprintf(title_str,
						"%-3dMbps             :",
						g_rate[mcs]);
					*len += scnprintf(p + *len, size - *len,
					"%s %10u, %9d, %9d, %9d, %9d, %9d\n",
					title_str, cnt, per0, per1, per2, per3,
					per4);
				}
			} else if (format ==  TX_RATE_FORMAT_11N) {
				sprintf(title_str, "%4s_%3s_MCS%2d	    :",
					bw_str[bw], sgi_str[sgi], mcs);
				*len += scnprintf(p + *len, size - *len,
					"%s %10u, %9d, %9d, %9d, %9d, %9d\n",
					title_str, cnt, per0, per1, per2, per3,
					per4);
			} else {
				nss = (mcs >> 4);
				sprintf(title_str, "%5s_%3s_%1dSS_MCS%2d :",
					bw_str[bw], sgi_str[sgi], (nss+1),
					(mcs & 0xf));
				*len += scnprintf(p + *len, size - *len,
					"%s %10u, %9d, %9d, %9d, %9d, %9d\n",
					title_str, cnt, per0, per1, per2, per3,
					per4);
			}
		}
	}
}

static void dump_tx_hist(char *p, int size, int *len, struct mwl_sta *sta_info)
{
	int type;
	bool printed, su;
	u32 total;

	for (type = 0; type <= SU_MU_TYPE_CNT; type++) {
		printed = false;
		total = 0;
		if (type == MU_MIMO) {
			dump_tx_hist_mu(p, size, len, &printed,
					&total, sta_info);
		} else {
			su = (type == SU_MIMO) ? true : false;
			dump_tx_hist_su(p, size, len, su, &printed,
					&total, sta_info);
		}
		if (printed)
			*len += scnprintf(p + *len, size - *len,
					  "  TOTAL              : %10u\n\n",
					  total);
	}
}

static void core_dump_file(u8 *valbuf, u32 length, u32 region, u32 address,
			   u32 append, u32 totallen, bool textmode)
{
	struct file *filp_core = NULL;
	char file_name[40];
	u8 *buf = kmalloc(length * 3, GFP_KERNEL);
	u8 *data_p = buf;
	u32 i, j = 0;

	if (!buf)
		return;

	memset(file_name, 0, sizeof(file_name));
	sprintf(file_name, "/dev/shm/coredump-%x-%x",
		region, (region + totallen));

	if (append)
		filp_core = filp_open(file_name, O_RDWR | O_APPEND, 0);
	else
		filp_core = filp_open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0);

	if (!IS_ERR(filp_core)) {
		if (textmode) {
			for (i = 0; i < length; i += 4) {
				u32 val = 0;

				val = le32_to_cpu(*(__le32 *)(&valbuf[i]));

				if (i % 16 == 0) {
					sprintf(buf + j, "\n0x%08x",
						(int)(address + i));
					j = strlen(buf);
				}
				sprintf(buf + j, "  %08x", val);
				j = strlen(buf);
			}
			data_p = buf + j;
			data_p += sprintf(data_p, "\n");
			__kernel_write(filp_core, buf, strlen(buf),
				       &filp_core->f_pos);
		} else
			__kernel_write(filp_core, valbuf, length,
				       &filp_core->f_pos);

		filp_close(filp_core, current->files);
	}

	kfree(buf);
}

static ssize_t mwl_debugfs_info_read(struct file *file, char __user *ubuf,
				     size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	int tx_num = 4, rx_num = 4;
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len,
			 "driver name: %s\n",
			 mwl_hif_get_driver_name(priv->hw));
	len += scnprintf(p + len, size - len, "chip type: %s\n",
			 chipname[priv->chip_type]);
	len += scnprintf(p + len, size - len,
			 "hw version: %X\n", priv->hw_data.hw_version);
	len += scnprintf(p + len, size - len,
			 "driver version: %s\n",
			 mwl_hif_get_driver_version(priv->hw));
	len += scnprintf(p + len, size - len, "firmware version: 0x%08x\n",
			 priv->hw_data.fw_release_num);
	len += scnprintf(p + len, size - len,
			 "power table loaded from dts: %s\n",
			 priv->forbidden_setting ? "no" : "yes");
	len += scnprintf(p + len, size - len, "firmware region code: 0x%x\n",
			 priv->fw_region_code);
	len += scnprintf(p + len, size - len,
			 "mac address: %pM\n", priv->hw_data.mac_addr);
	len += scnprintf(p + len, size - len,
			 "2g: %s\n", priv->disable_2g ? "disable" : "enable");
	len += scnprintf(p + len, size - len,
			 "5g: %s\n", priv->disable_5g ? "disable" : "enable");
	if (priv->antenna_tx == ANTENNA_TX_2)
		tx_num = 2;
	else if (priv->antenna_tx == ANTENNA_TX_3)
		tx_num = 3;
	if (priv->antenna_rx == ANTENNA_RX_2)
		rx_num = 2;
	else if (priv->antenna_rx == ANTENNA_RX_3)
		rx_num = 3;
	len += scnprintf(p + len, size - len, "antenna: %d %d\n",
			 tx_num, rx_num);
	len += scnprintf(p + len, size - len, "irq number: %d\n", priv->irq);
	len += scnprintf(p + len, size - len, "ap macid support: %08x\n",
			 priv->ap_macids_supported);
	len += scnprintf(p + len, size - len, "sta macid support: %08x\n",
			 priv->sta_macids_supported);
	len += scnprintf(p + len, size - len,
			 "macid used: %08x\n", priv->macids_used);
	len += scnprintf(p + len, size - len,
			 "radio: %s\n", priv->radio_on ? "enable" : "disable");
	len += mwl_hif_get_info(priv->hw, p + len, size - len);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_tx_status_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += mwl_hif_get_tx_status(priv->hw, p + len, size - len);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_rx_status_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += mwl_hif_get_rx_status(priv->hw, p + len, size - len);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_vif_read(struct file *file, char __user *ubuf,
				    size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_vif *mwl_vif;
	struct ieee80211_vif *vif;
	char ssid[IEEE80211_MAX_SSID_LEN + 1];
	struct cfg80211_chan_def *chan_def;
	struct beacon_info *beacon_info;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	spin_lock_bh(&priv->vif_lock);
	list_for_each_entry(mwl_vif, &priv->vif_list, list) {
		vif = container_of((void *)mwl_vif, struct ieee80211_vif,
				   drv_priv);
		len += scnprintf(p + len, size - len,
				 "macid: %d\n", mwl_vif->macid);
		switch (vif->type) {
		case NL80211_IFTYPE_AP:
			len += scnprintf(p + len, size - len, "type: ap\n");
			memcpy(ssid, vif->bss_conf.ssid,
			       vif->bss_conf.ssid_len);
			ssid[vif->bss_conf.ssid_len] = 0;
			len += scnprintf(p + len, size - len,
					 "ssid: %s\n", ssid);
			len += scnprintf(p + len, size - len,
					 "mac address: %pM\n", mwl_vif->bssid);
			break;
		case NL80211_IFTYPE_MESH_POINT:
			len += scnprintf(p + len, size - len, "type: mesh\n");
			len += scnprintf(p + len, size - len,
					 "mac address: %pM\n", mwl_vif->bssid);
			break;
		case NL80211_IFTYPE_STATION:
			len += scnprintf(p + len, size - len, "type: sta\n");
			len += scnprintf(p + len, size - len,
					 "mac address: %pM\n",
					 mwl_vif->sta_mac);
			break;
		default:
			len += scnprintf(p + len, size - len,
					 "type: unknown\n");
			break;
		}
		if (vif->chanctx_conf) {
			chan_def = &vif->chanctx_conf->def;
			len += scnprintf(p + len, size - len,
					 "channel: %d: width: %d\n",
					 chan_def->chan->hw_value,
					 chan_def->width);
			len += scnprintf(p + len, size - len,
					 "freq: %d freq1: %d freq2: %d\n",
					 chan_def->chan->center_freq,
					 chan_def->center_freq1,
					 chan_def->center_freq2);
		}
		len += scnprintf(p + len, size - len, "hw_crypto_enabled: %s\n",
				 mwl_vif->is_hw_crypto_enabled ?
				 "true" : "false");
		len += scnprintf(p + len, size - len,
				 "key idx: %d\n", mwl_vif->keyidx);
		len += scnprintf(p + len, size - len,
				 "IV: %08x%04x\n", mwl_vif->iv32,
				 mwl_vif->iv16);
		beacon_info = &mwl_vif->beacon_info;
		dump_data(p, size, &len, beacon_info->ie_wmm_ptr,
			  beacon_info->ie_wmm_len, "WMM:");
		dump_data(p, size, &len, beacon_info->ie_rsn_ptr,
			  beacon_info->ie_rsn_len, "RSN:");
		dump_data(p, size, &len, beacon_info->ie_rsn48_ptr,
			  beacon_info->ie_rsn48_len, "RSN48:");
		dump_data(p, size, &len, beacon_info->ie_mde_ptr,
			  beacon_info->ie_mde_len, "MDE:");
		dump_data(p, size, &len, beacon_info->ie_ht_ptr,
			  beacon_info->ie_ht_len, "HT:");
		dump_data(p, size, &len, beacon_info->ie_vht_ptr,
			  beacon_info->ie_vht_len, "VHT:");
		if (vif->type == NL80211_IFTYPE_MESH_POINT) {
			dump_data(p, size, &len, beacon_info->ie_meshid_ptr,
				  beacon_info->ie_meshid_len, "MESHID:");
			dump_data(p, size, &len, beacon_info->ie_meshcfg_ptr,
				  beacon_info->ie_meshcfg_len, "MESHCFG:");
			dump_data(p, size, &len, beacon_info->ie_meshchsw_ptr,
				  beacon_info->ie_meshchsw_len, "MESHCHSW:");
		}
		len += scnprintf(p + len, size - len, "\n");
	}
	spin_unlock_bh(&priv->vif_lock);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_sta_read(struct file *file, char __user *ubuf,
				    size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv);
		len += scnprintf(p + len, size - len,
				 "mac address: %pM\n", sta->addr);
		len += scnprintf(p + len, size - len, "aid: %u\n", sta->aid);
		len += scnprintf(p + len, size - len, "ampdu: %s\n",
				 sta_info->is_ampdu_allowed ? "true" : "false");
		len += scnprintf(p + len, size - len, "amsdu: %s\n",
				 sta_info->is_amsdu_allowed ? "true" : "false");
		len += scnprintf(p + len, size - len, "wds: %s\n",
				 sta_info->wds ? "true" : "false");
		len += scnprintf(p + len, size - len, "ba_hist: %s\n",
				 sta_info->ba_hist.enable ?
				 "enable" : "disable");
		if (sta_info->is_amsdu_allowed) {
			len += scnprintf(p + len, size - len,
					 "amsdu cap: 0x%02x\n",
					 sta_info->amsdu_ctrl.cap);
		}
		if (sta->ht_cap.ht_supported) {
			len += scnprintf(p + len, size - len,
					 "ht_cap: 0x%04x, ampdu: %02x, %02x\n",
					 sta->ht_cap.cap,
					 sta->ht_cap.ampdu_factor,
					 sta->ht_cap.ampdu_density);
			len += scnprintf(p + len, size - len,
					 "rx_mask: 0x%02x, %02x, %02x, %02x\n",
					 sta->ht_cap.mcs.rx_mask[0],
					 sta->ht_cap.mcs.rx_mask[1],
					 sta->ht_cap.mcs.rx_mask[2],
					 sta->ht_cap.mcs.rx_mask[3]);
		}
		if (sta->vht_cap.vht_supported) {
			len += scnprintf(p + len, size - len,
					 "vht_cap: 0x%08x, mcs: %02x, %02x\n",
					 sta->vht_cap.cap,
					 sta->vht_cap.vht_mcs.rx_mcs_map,
					 sta->vht_cap.vht_mcs.tx_mcs_map);
		}
		len += scnprintf(p + len, size - len, "rx_bw: %d, rx_nss: %d\n",
				 sta->bandwidth, sta->rx_nss);
		len += scnprintf(p + len, size - len,
				 "tdls: %d, tdls_init: %d\n",
				 sta->tdls, sta->tdls_initiator);
		len += scnprintf(p + len, size - len, "wme: %d, mfp: %d\n",
				 sta->wme, sta->mfp);
		len += scnprintf(p + len, size - len, "IV: %08x%04x\n",
				 sta_info->iv32, sta_info->iv16);
		len += scnprintf(p + len, size - len, "\n");
	}
	spin_unlock_bh(&priv->sta_lock);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_ampdu_read(struct file *file, char __user *ubuf,
				      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_ampdu_stream *stream;
	int i;
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	spin_lock_bh(&priv->stream_lock);
	for (i = 0; i < priv->ampdu_num; i++) {
		stream = &priv->ampdu[i];
		if (!stream->state)
			continue;
		len += scnprintf(p + len, size - len, "stream: %d\n", i);
		len += scnprintf(p + len, size - len, "idx: %u\n", stream->idx);
		len += scnprintf(p + len, size - len,
				 "state: %u\n", stream->state);
		if (stream->sta) {
			len += scnprintf(p + len, size - len,
					 "mac address: %pM\n",
					 stream->sta->addr);
			len += scnprintf(p + len, size - len,
					 "tid: %u\n", stream->tid);
		}
	}
	spin_unlock_bh(&priv->stream_lock);
	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		for (i = 0; i < MWL_MAX_TID; i++) {
			if (sta_info->check_ba_failed[i]) {
				sta = container_of((void *)sta_info,
						   struct ieee80211_sta,
						   drv_priv);
				len += scnprintf(p + len, size - len,
						 "%pM(%d): %d\n",
						 sta->addr, i,
						 sta_info->check_ba_failed[i]);
			}
		}
	}
	spin_unlock_bh(&priv->sta_lock);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_stnid_read(struct file *file, char __user *ubuf,
				      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_stnid *stnid;
	int i;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	spin_lock_bh(&priv->stnid_lock);
	for (i = 0; i < priv->stnid_num; i++) {
		stnid = &priv->stnid[i];
		if (!stnid->aid)
			continue;
		len += scnprintf(p + len, size - len,
				 "stnid: %d macid: %d aid: %d\n",
				 i + 1, stnid->macid, stnid->aid);
	}
	spin_unlock_bh(&priv->stnid_lock);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_device_pwrtbl_read(struct file *file,
					      char __user *ubuf,
					      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	int i, j;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len,
			 "power table loaded from dts: %s\n",
			 priv->forbidden_setting ? "no" : "yes");
	len += scnprintf(p + len, size - len, "firmware region code: 0x%x\n",
			 priv->fw_region_code);
	len += scnprintf(p + len, size - len, "number of channel: %d\n",
			 priv->number_of_channels);
	for (i = 0; i < priv->number_of_channels; i++) {
		len += scnprintf(p + len, size - len, "%3d ",
				 priv->device_pwr_tbl[i].channel);
		for (j = 0; j < SYSADPT_TX_POWER_LEVEL_TOTAL; j++)
			len += scnprintf(p + len, size - len, "%3d ",
					 priv->device_pwr_tbl[i].tx_pwr[j]);
		len += scnprintf(p + len, size - len, "%3d ",
				 priv->device_pwr_tbl[i].dfs_capable);
		len += scnprintf(p + len, size - len, "%3d ",
				 priv->device_pwr_tbl[i].ax_ant);
		len += scnprintf(p + len, size - len, "%3d\n",
				 priv->device_pwr_tbl[i].cdd);
	}
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_txpwrlmt_read(struct file *file,
					 char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;

	return simple_read_from_buffer(ubuf, count, ppos,
				       priv->txpwrlmt_data.buf,
				       priv->txpwrlmt_data.len);
}

static ssize_t mwl_debugfs_tx_amsdu_read(struct file *file,
					 char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "tx amsdu: %s\n",
			 priv->tx_amsdu ? "enable" : "disable");
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_tx_amsdu_write(struct file *file,
					  const char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int value;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &value)) {
		ret = -EINVAL;
		goto err;
	}

	priv->tx_amsdu = value ? true : false;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_dump_hostcmd_read(struct file *file,
					     char __user *ubuf,
					     size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "dump_hostcmd: %s\n",
			 priv->dump_hostcmd ? "enable" : "disable");
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_dump_hostcmd_write(struct file *file,
					      const char __user *ubuf,
					      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int value;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &value)) {
		ret = -EINVAL;
		goto err;
	}

	priv->dump_hostcmd = value ? true : false;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_dump_probe_read(struct file *file,
					   char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "dump_probe: %s\n",
			 priv->dump_probe ? "enable" : "disable");
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_dump_probe_write(struct file *file,
					    const char __user *ubuf,
					    size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int value;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &value)) {
		ret = -EINVAL;
		goto err;
	}

	priv->dump_probe = value ? true : false;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_heartbeat_read(struct file *file,
					  char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "heartbeat: %d\n",
			 priv->heartbeat);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_heartbeat_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &priv->heartbeat)) {
		ret = -EINVAL;
		goto err;
	}
	priv->pre_jiffies = jiffies;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_dfs_test_read(struct file *file,
					 char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "dfs_test: %s\n",
			 priv->dfs_test ? "enable" : "disable");
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_dfs_test_write(struct file *file,
					  const char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int value;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &value)) {
		ret = -EINVAL;
		goto err;
	}

	priv->dfs_test = value ? true : false;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_dfs_channel_read(struct file *file,
					    char __user *ubuf,
					    size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *channel;
	int i;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	sband = priv->hw->wiphy->bands[NL80211_BAND_5GHZ];
	if (!sband) {
		ret = -EINVAL;
		goto err;
	}

	len += scnprintf(p + len, size - len, "\n");
	for (i = 0; i < sband->n_channels; i++) {
		channel = &sband->channels[i];
		if (channel->flags & IEEE80211_CHAN_RADAR) {
			len += scnprintf(p + len, size - len,
					 "%d(%d): flags: %08x dfs_state: %d\n",
					 channel->hw_value,
					 channel->center_freq,
					 channel->flags, channel->dfs_state);
			len += scnprintf(p + len, size - len,
					 "cac timer: %d ms\n",
					 channel->dfs_cac_ms);
		}
	}
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);

err:
	free_page(page);
	return ret;
}

static ssize_t mwl_debugfs_dfs_channel_write(struct file *file,
					     const char __user *ubuf,
					     size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	struct ieee80211_supported_band *sband;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int dfs_state = 0;
	int cac_time = -1;
	struct ieee80211_channel *channel;
	int i;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	sband = priv->hw->wiphy->bands[NL80211_BAND_5GHZ];
	if (!sband) {
		ret = -EINVAL;
		goto err;
	}

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	ret = sscanf(buf, "%d %d", &dfs_state, &cac_time);

	if ((ret < 1) || (ret > 2)) {
		ret = -EINVAL;
		goto err;
	}

	for (i = 0; i < sband->n_channels; i++) {
		channel = &sband->channels[i];
		if (channel->flags & IEEE80211_CHAN_RADAR) {
			channel->dfs_state = dfs_state;
			if (cac_time != -1)
				channel->dfs_cac_ms = cac_time * 1000;
		}
	}
	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_dfs_radar_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len,
			 "csa_active: %d\n", priv->csa_active);
	len += scnprintf(p + len, size - len,
			 "dfs_region: %d\n", priv->dfs_region);
	len += scnprintf(p + len, size - len,
			 "chirp_count_min: %d\n", priv->dfs_chirp_count_min);
	len += scnprintf(p + len, size - len, "chirp_time_interval: %d\n",
			 priv->dfs_chirp_time_interval);
	len += scnprintf(p + len, size - len,
			 "pw_filter: %d\n", priv->dfs_pw_filter);
	len += scnprintf(p + len, size - len,
			 "min_num_radar: %d\n", priv->dfs_min_num_radar);
	len += scnprintf(p + len, size - len,
			 "min_pri_count: %d\n", priv->dfs_min_pri_count);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_dfs_radar_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;

	wiphy_info(priv->hw->wiphy, "simulate radar detected\n");
	ieee80211_radar_detected(priv->hw);

	return count;
}

static ssize_t mwl_debugfs_thermal_read(struct file *file,
					char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	mwl_fwcmd_get_temp(priv->hw, &priv->temperature);

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "quiet period: %d\n",
			 priv->quiet_period);
	len += scnprintf(p + len, size - len, "throttle state: %d\n",
			 priv->throttle_state);
	len += scnprintf(p + len, size - len, "temperature: %d\n",
			 priv->temperature);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_thermal_write(struct file *file,
					 const char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int throttle_state;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &throttle_state)) {
		ret = -EINVAL;
		goto err;
	}

	if (throttle_state > SYSADPT_THERMAL_THROTTLE_MAX) {
		wiphy_warn(priv->hw->wiphy,
			   "throttle state %d is exceeding the limit %d\n",
			   throttle_state, SYSADPT_THERMAL_THROTTLE_MAX);
		ret = -EINVAL;
		goto err;
	}

	priv->throttle_state = throttle_state;
	mwl_thermal_set_throttling(priv);
	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_led_ctrl_read(struct file *file,
					 char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "led blink %s\n",
			 priv->led_blink_enable ? "enable" : "disable");
	len += scnprintf(p + len, size - len, "led blink rate: %d\n",
			 priv->led_blink_rate);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_led_ctrl_write(struct file *file,
					  const char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int enable, rate;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	ret = sscanf(buf, "%x %x", &enable, &rate);

	if ((ret != 1) && (ret != 2)) {
		ret = -EINVAL;
		goto err;
	}

	if (enable && (ret != 2)) {
		ret = -EINVAL;
		goto err;
	}

	ret = mwl_fwcmd_led_ctrl(priv->hw, enable, rate);

	if (ret)
		goto err;

	priv->led_blink_enable = enable;
	if (enable)
		priv->led_blink_rate = rate;
	else
		priv->led_blink_rate = 0;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_regrdwr_read(struct file *file, char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (*ppos)
		return len;

	if (!p)
		return -ENOMEM;

	if (!priv->reg_type) {
		/* No command has been given */
		len += scnprintf(p + len, size - len, "0");
		ret = -EINVAL;
		goto none;
	}

	/* Set command has been given */
	if (priv->reg_value != UINT_MAX) {
		ret = mwl_hif_reg_access(priv->hw, true);
		goto done;
	}
	/* Get command has been given */
	ret = mwl_hif_reg_access(priv->hw, false);

done:
	if (!ret)
		len += scnprintf(p + len, size - len, "%u 0x%08x 0x%08x\n",
				 priv->reg_type, priv->reg_offset,
				 priv->reg_value);
	else
		len += scnprintf(p + len, size - len,
				 "error: %d(%u 0x%08x 0x%08x)\n",
				 ret, priv->reg_type, priv->reg_offset,
				 priv->reg_value);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);

none:
	free_page(page);
	return ret;
}

static ssize_t mwl_debugfs_regrdwr_write(struct file *file,
					 const char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	ssize_t ret;
	u32 reg_type = 0, reg_offset = 0, reg_value = UINT_MAX;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	ret = sscanf(buf, "%u %x %x", &reg_type, &reg_offset, &reg_value);

	if (!reg_type) {
		ret = -EINVAL;
		goto err;
	} else {
		priv->reg_type = reg_type;
		priv->reg_offset = reg_offset;
		priv->reg_value = reg_value;
		ret = count;
	}

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_ratetable_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;
	u8 addr[ETH_ALEN];
	int table_size = (sizeof(__le32) * 2 * SYSADPT_MAX_RATE_ADAPT_RATES);
	u8 *rate_table, *rate_idx;
	u32 rate_info;
	u8 fmt, stbc, bw, sgi, mcs, preamble_gf, power_id, ldpc, bf, ant;
	int idx, rate, nss;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	if (!priv->ra_aid) {
		ret = -EINVAL;
		goto err;
	}

	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv);
		if (priv->ra_aid == sta->aid) {
			ether_addr_copy(addr, sta->addr);
			break;
		}
	}
	spin_unlock_bh(&priv->sta_lock);

	rate_table = kzalloc(size, GFP_KERNEL);
	if (!rate_table) {
		ret = -ENOMEM;
		goto err;
	}

	ret = mwl_fwcmd_get_ratetable(priv->hw, addr, rate_table,
				      table_size, 0);
	if (ret) {
		kfree(rate_table);
		goto err;
	}

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len,
		"%3s %6s %5s %5s %5s %5s %5s %4s %2s %5s %4s %5s %5s\n",
		"Num", "Fmt", "STBC", "BW", "SGI", "Nss", "RateId",
		"GF/Pre", "PId", "LDPC", "BF", "TxAnt", "Rate");
	idx = 0;
	rate_idx = rate_table;
	rate_info = le32_to_cpu(*(__le32 *)rate_idx);
	while (rate_info) {
		fmt = rate_info & MWL_TX_RATE_FORMAT_MASK;
		stbc = (rate_info & MWL_TX_RATE_STBC_MASK) >>
			MWL_TX_RATE_STBC_SHIFT;
		bw = (rate_info & MWL_TX_RATE_BANDWIDTH_MASK) >>
			MWL_TX_RATE_BANDWIDTH_SHIFT;
		sgi = (rate_info & MWL_TX_RATE_SHORTGI_MASK) >>
			MWL_TX_RATE_SHORTGI_SHIFT;
		mcs = (rate_info & MWL_TX_RATE_RATEIDMCS_MASK) >>
			MWL_TX_RATE_RATEIDMCS_SHIFT;
		preamble_gf = (rate_info & MWL_TX_RATE_PREAMBLE_MASK) >>
			MWL_TX_RATE_PREAMBLE_SHIFT;
		power_id = (rate_info & MWL_TX_RATE_POWERID_MASK) >>
			MWL_TX_RATE_POWERID_SHIFT;
		ldpc = (rate_info & MWL_TX_RATE_ADVCODING_MASK) >>
			MWL_TX_RATE_ADVCODING_SHIFT;
		bf = (rate_info & MWL_TX_RATE_BF_MASK) >>
			MWL_TX_RATE_BF_SHIFT;
		ant = (rate_info & MWL_TX_RATE_ANTSELECT_MASK) >>
			MWL_TX_RATE_ANTSELECT_SHIFT;

		if (fmt == TX_RATE_FORMAT_11AC) {
			rate = mcs & 0xf; /* 11ac, mcs[3:0]: rate      */
			nss = mcs >> 4;   /* 11ac, mcs[6:4] = nss code */
			nss++;  /* ddd 1 to correct Nss representation */
		} else {
			rate = mcs;
			nss = 0;
			if (fmt == TX_RATE_FORMAT_11N) {
				if ((mcs >= 0) && (mcs < 8))
					nss = 1;
				else if ((mcs >= 8) && (mcs < 16))
					nss = 2;
				else if ((mcs >= 16) && (mcs < 24))
					nss = 3;
			}
		}

		len += scnprintf(p + len, size - len,
			"%3d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d\n",
			idx, (int)fmt, (int)stbc, (int)bw, (int)sgi, nss, rate,
			(int)preamble_gf, (int)power_id, (int)ldpc, (int)bf,
			(int)ant,
			utils_get_phy_rate(fmt, bw, sgi, mcs));

		idx++;
		rate_idx += (2 * sizeof(__le32));
		rate_info = le32_to_cpu(*(__le32 *)rate_idx);
	}
	len += scnprintf(p + len, size - len, "\n");

	kfree(rate_table);
	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);

err:
	free_page(page);
	return ret;
}

static ssize_t mwl_debugfs_ratetable_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int sta_aid;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &sta_aid)) {
		ret = -EINVAL;
		goto err;
	}

	if ((sta_aid <= 0) || (sta_aid > SYSADPT_MAX_STA_SC4)) {
		wiphy_warn(priv->hw->wiphy,
			   "station aid is exceeding the limit %d\n", sta_aid);
		ret = -EINVAL;
		goto err;
	}

	priv->ra_aid = sta_aid;
	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_tx_hist_read(struct file *file, char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct ieee80211_sta *sta;
	struct mwl_sta *sta_info;
	ssize_t ret;

	if (priv->chip_type != MWL8964)
		return -EPERM;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len,
			 "SU: <4:%d >=4:%d >=15:%d >=50:%d >=100:%d >=250:%d\n",
			 priv->ra_tx_attempt[SU_MIMO][0],
			 priv->ra_tx_attempt[SU_MIMO][1],
			 priv->ra_tx_attempt[SU_MIMO][2],
			 priv->ra_tx_attempt[SU_MIMO][3],
			 priv->ra_tx_attempt[SU_MIMO][4],
			 priv->ra_tx_attempt[SU_MIMO][5]);
	len += scnprintf(p + len, size - len,
			 "MU: <4:%d >=4:%d >=15:%d >=50:%d >=100:%d >=250:%d\n",
			 priv->ra_tx_attempt[MU_MIMO][0],
			 priv->ra_tx_attempt[MU_MIMO][1],
			 priv->ra_tx_attempt[MU_MIMO][2],
			 priv->ra_tx_attempt[MU_MIMO][3],
			 priv->ra_tx_attempt[MU_MIMO][4],
			 priv->ra_tx_attempt[MU_MIMO][5]);
	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv);
		len += scnprintf(p + len, size - len, "\nSTA %pM\n", sta->addr);
		len += scnprintf(p + len, size - len,
				 "============================\n");
		dump_tx_hist(p, size, &len, sta_info);
		len += scnprintf(p + len, size - len,
				 "============================\n");
	}
	spin_unlock_bh(&priv->sta_lock);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);
	return ret;
}

static ssize_t mwl_debugfs_tx_hist_write(struct file *file,
					 const char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int reset;
	struct mwl_sta *sta_info;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &reset)) {
		ret = -EINVAL;
		goto err;
	}

	if (!reset) {
		memset(&priv->ra_tx_attempt, 0, 2 * 6 * sizeof(u32));
		spin_lock_bh(&priv->sta_lock);
		list_for_each_entry(sta_info, &priv->sta_list, list) {
			memset(&sta_info->tx_hist, 0,
			       sizeof(sta_info->tx_hist));
		}
		spin_unlock_bh(&priv->sta_lock);
	}

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_ba_hist_read(struct file *file, char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_sta *sta_info;
	struct mwl_tx_ba_stats *ba_stats;
	u32 i, data;
	u32 baholecnt, baexpcnt, bmap0cnt, nobacnt;
	u8 bmap0flag, nobaflag;
	char buff[500], file_location[20];
	struct file *filp_bahisto;
	u8 *data_p = buff;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	if (!priv->ba_aid) {
		ret = -EINVAL;
		goto err;
	}

	memset(buff, 0, sizeof(buff));
	memset(file_location, 0, sizeof(file_location));
	sprintf(file_location, "/tmp/ba_histo-%d", priv->ba_aid);

	filp_bahisto = filp_open(file_location,
				 O_RDWR | O_CREAT | O_TRUNC, 0);

	if (IS_ERR(filp_bahisto)) {
		ret = -EIO;
		goto err;
	}

	sta_info = utils_find_sta_by_aid(priv, priv->ba_aid);
	if (sta_info && sta_info->ba_hist.enable &&
	    sta_info->ba_hist.ba_stats) {
		ba_stats = sta_info->ba_hist.ba_stats;
		len += scnprintf(p + len, size - len,
				 "BA histogram aid: %d, stnid: %d type: %s\n",
				 priv->ba_aid, sta_info->stnid,
				 sta_info->ba_hist.type ? "MU" : "SU");
		data_p += sprintf(data_p,
				  "BA histogram aid: %d, stnid: %d type: %s\n",
				  priv->ba_aid, sta_info->stnid,
				  sta_info->ba_hist.type ? "MU" : "SU");
		data_p += sprintf(data_p, "%8s,%8s,%8s,%8s\n",
				  "BAhole", "Expect", "Bmap0", "NoBA");
		data = *(u32 *)&ba_stats[0];
		baholecnt = 0;
		baexpcnt = 0;
		bmap0cnt = 0;
		nobacnt = 0;
		for (i = 0; i < ACNT_BA_SIZE && data; i++) {
			data = *(u32 *)&ba_stats[i];
			if (data == 0)
				break;

			/* If no BA event does not happen, check BA hole and BA
			 * expected to mark BA bitmap all 0 event
			 */
			if (!ba_stats[i].no_ba)
				bmap0flag = (ba_stats[i].ba_hole ==
					     ba_stats[i].ba_expected) ? 1 : 0;
			else
				bmap0flag = 0;
			nobaflag = ba_stats[i].no_ba;

			/* Buffer is full. Write to file and reset buf */
			if ((strlen(buff) + 36) >= 500) {
				__kernel_write(filp_bahisto, buff, strlen(buff),
					       &filp_bahisto->f_pos);
				mdelay(2);
				memset(buff, 0, sizeof(buff));
				data_p = buff;
			}

			data_p += sprintf(data_p, "%8d,%8d,",
					  ba_stats[i].ba_hole,
					  ba_stats[i].ba_expected);

			baholecnt += ba_stats[i].ba_hole;
			baexpcnt += ba_stats[i].ba_expected;
			if (bmap0flag) {
				data_p += sprintf(data_p, "       #,");
				bmap0cnt++;
			} else
				data_p += sprintf(data_p, "%8d,", bmap0flag);
			if (nobaflag) {
				data_p += sprintf(data_p, "       *\n");
				nobacnt++;
			} else
				data_p += sprintf(data_p, "%8d\n", nobaflag);
		}

		__kernel_write(filp_bahisto, buff, strlen(buff),
			       &filp_bahisto->f_pos);
		len += scnprintf(p + len, size - len,
				 "hole: %d, expect: %d, bmap0: %d, noba: %d\n",
				 baholecnt, baexpcnt, bmap0cnt, nobacnt);
		len += scnprintf(p + len, size - len,
				 "BA histogram data written to %s\n",
				 file_location);
	} else
		len += scnprintf(p + len, size - len,
				 "No BA histogram for sta aid: %d\n",
				 priv->ba_aid);

	filp_close(filp_bahisto, current->files);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);

err:
	free_page(page);
	return ret;
}

static ssize_t mwl_debugfs_ba_hist_write(struct file *file,
					 const char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int sta_aid;
	struct mwl_sta *sta_info;
	int size;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &sta_aid)) {
		ret = -EINVAL;
		goto err;
	}

	if ((sta_aid <= 0) || (sta_aid > SYSADPT_MAX_STA_SC4)) {
		wiphy_warn(priv->hw->wiphy,
			   "station aid is exceeding the limit %d\n", sta_aid);
		ret = -EINVAL;
		goto err;
	}

	if (priv->ba_aid) {
		sta_info = utils_find_sta_by_aid(priv, priv->ba_aid);
		if (sta_info) {
			sta_info->ba_hist.enable = false;
			kfree(sta_info->ba_hist.ba_stats);
		}
	}
	priv->ba_aid = 0;
	sta_info = utils_find_sta_by_aid(priv, sta_aid);
	if (sta_info) {
		sta_info->ba_hist.enable = true;
		sta_info->ba_hist.index = 0;
		size = sizeof(struct mwl_tx_ba_stats) * ACNT_BA_SIZE;
		sta_info->ba_hist.ba_stats = kmalloc(size, GFP_KERNEL);
		if (sta_info->ba_hist.ba_stats) {
			memset(sta_info->ba_hist.ba_stats, 0, size);
			priv->ba_aid = sta_aid;
		}
		ret = count;
	} else
		ret = -EINVAL;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_fixed_rate_read(struct file *file, char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "fixed rate: 0x%08x\n",
			 priv->fixed_rate);
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);
	return ret;
}

static ssize_t mwl_debugfs_fixed_rate_write(struct file *file,
					    const char __user *ubuf,
					    size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	ssize_t ret;
	int fixed_rate = 0, fwcmd_ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	ret = sscanf(buf, "%08x", &fixed_rate);
	if (!ret) {
		ret = -EIO;
		goto err;
	}

	priv->fixed_rate = fixed_rate;

	if (fixed_rate != 0)
		fwcmd_ret = mwl_fwcmd_set_rate_drop(priv->hw, 3,
						    priv->fixed_rate, 0);
	else
		fwcmd_ret = mwl_fwcmd_set_rate_drop(priv->hw, 1,
						    priv->fixed_rate, 0);
	if (fwcmd_ret)
		ret = -EIO;
	else
		ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_core_dump_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct coredump_cmd *core_dump = NULL;
	struct coredump *cd = NULL;
	char  *buff = NULL;
	u32 i, offset;
	u32 address, length;
	ssize_t ret;

	if (priv->chip_type != MWL8964)
		return -EPERM;

	if (*ppos)
		return len;

	if (!p)
		return -ENOMEM;

	core_dump = kmalloc(sizeof(*core_dump), GFP_ATOMIC);
	if (!core_dump) {
		ret = -ENOMEM;
		goto err;
	}

	buff = kmalloc(MAX_CORE_DUMP_BUFFER, GFP_ATOMIC);
	if (!buff) {
		ret = -ENOMEM;
		goto err;
	}
	memset((char *)buff, 0, MAX_CORE_DUMP_BUFFER);

	cd = kmalloc(sizeof(*cd), GFP_ATOMIC);
	if (!cd) {
		ret = -ENOMEM;
		goto err;
	}

	core_dump->context = 0;
	core_dump->flags = 0;
	core_dump->size_kb = 0;
	if (mwl_fwcmd_get_fw_core_dump(priv->hw, core_dump, buff)) {
		ret = -EIO;
		goto err;
	}
	memcpy(cd, buff, sizeof(*cd));

	len += scnprintf(p + len, size - len, "\n");
	len += scnprintf(p + len, size - len, "Major Version : %d\n",
			 cd->version_major);
	len += scnprintf(p + len, size - len, "Minor Version : %d\n",
			 cd->version_minor);
	len += scnprintf(p + len, size - len, "Patch Version : %d\n",
			 cd->version_patch);
	len += scnprintf(p + len, size - len, "Num of Regions: %d\n",
			 cd->num_regions);
	len += scnprintf(p + len, size - len, "Num of Symbols: %d\n",
			 cd->num_symbols);

	for (i = 0; i < cd->num_regions; i++) {
		address = le32_to_cpu(cd->region[i].address);
		length = le32_to_cpu(cd->region[i].length);
		len += scnprintf(p + len, size - len,
				 "\ncd.region[%d]: address=%x, length=%x\n",
				 i, address, length);

		for (offset = 0; offset < length;
		     offset += MAX_CORE_DUMP_BUFFER) {
			core_dump->context = cpu_to_le32((i << 28) | offset);
			core_dump->flags = 0;
			core_dump->size_kb = 0;
			if (mwl_fwcmd_get_fw_core_dump(priv->hw,
			    core_dump, buff)) {
				wiphy_info(priv->hw->wiphy,
					   "region:%d offset:%x\n", i, offset);
				break;
			}
			core_dump_file(buff, MAX_CORE_DUMP_BUFFER,
				       address, address + offset,
				       offset, length, priv->coredump_text);
		}
	}
	len += scnprintf(p + len, size - len, "\n");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);

err:
	kfree(core_dump);
	kfree(buff);
	kfree(cd);
	free_page(page);
	return ret;
}

static ssize_t mwl_debugfs_core_dump_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int text_mode;
	ssize_t ret;

	if (priv->chip_type != MWL8964)
		return -EPERM;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &text_mode)) {
		ret = -EINVAL;
		goto err;
	}

	if ((text_mode < 0) || (text_mode > 1)) {
		wiphy_warn(priv->hw->wiphy,
			   "text mode should be 0 (false) or 1 (true): %d\n",
			   text_mode);
		ret = -EINVAL;
		goto err;
	}

	mwl_fwcmd_core_dump_diag_mode(priv->hw, 1);
	priv->coredump_text = text_mode ? true : false;
	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_mcast_cts_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	int cts_enable = 0;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	if (kstrtoint(buf, 0, &cts_enable)) {
		ret = -EINVAL;
		goto err;
	}

	ret = mwl_hif_mcast_cts(priv->hw, cts_enable ? true : false);
	if (ret)
		goto err;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_wmmedcaap_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = (struct mwl_priv *)file->private_data;
	unsigned long addr = get_zeroed_page(GFP_KERNEL);
	char *buf = (char *)addr;
	size_t buf_size = min_t(size_t, count, PAGE_SIZE - 1);
	u32 index = 0, cw_min = 0, cw_max = 0, aifsn = 0, txop = 0;
	ssize_t ret;

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, ubuf, buf_size)) {
		ret = -EFAULT;
		goto err;
	}

	ret = sscanf(buf, "%u %x %x %u %x", &index, &cw_min,
			&cw_max, &aifsn, &txop);
	if (ret != 5) {
		ret = -EINVAL;
		goto err;
	}
	wiphy_info(priv->hw->wiphy, "set TCQ%d wmm edca with:\n", index);
	wiphy_info(priv->hw->wiphy,
		   "cw_min=0x%x, cw_max=0x%x, aifs_num=%d, txop=0x%x\n",
		   cw_min, cw_max, aifsn, txop);

	ret = mwl_fwcmd_set_edca_params(priv->hw, index,
			cw_min, cw_max, aifsn, txop);
	if (ret)
		goto err;

	ret = count;

err:
	free_page(addr);
	return ret;
}

MWLWIFI_DEBUGFS_FILE_READ_OPS(info);
MWLWIFI_DEBUGFS_FILE_READ_OPS(tx_status);
MWLWIFI_DEBUGFS_FILE_READ_OPS(rx_status);
MWLWIFI_DEBUGFS_FILE_READ_OPS(vif);
MWLWIFI_DEBUGFS_FILE_READ_OPS(sta);
MWLWIFI_DEBUGFS_FILE_READ_OPS(ampdu);
MWLWIFI_DEBUGFS_FILE_READ_OPS(stnid);
MWLWIFI_DEBUGFS_FILE_READ_OPS(device_pwrtbl);
MWLWIFI_DEBUGFS_FILE_READ_OPS(txpwrlmt);
MWLWIFI_DEBUGFS_FILE_OPS(tx_amsdu);
MWLWIFI_DEBUGFS_FILE_OPS(dump_hostcmd);
MWLWIFI_DEBUGFS_FILE_OPS(dump_probe);
MWLWIFI_DEBUGFS_FILE_OPS(heartbeat);
MWLWIFI_DEBUGFS_FILE_OPS(dfs_test);
MWLWIFI_DEBUGFS_FILE_OPS(dfs_channel);
MWLWIFI_DEBUGFS_FILE_OPS(dfs_radar);
MWLWIFI_DEBUGFS_FILE_OPS(thermal);
MWLWIFI_DEBUGFS_FILE_OPS(led_ctrl);
MWLWIFI_DEBUGFS_FILE_OPS(regrdwr);
MWLWIFI_DEBUGFS_FILE_OPS(ratetable);
MWLWIFI_DEBUGFS_FILE_OPS(tx_hist);
MWLWIFI_DEBUGFS_FILE_OPS(ba_hist);
MWLWIFI_DEBUGFS_FILE_OPS(fixed_rate);
MWLWIFI_DEBUGFS_FILE_OPS(core_dump);
MWLWIFI_DEBUGFS_FILE_WRITE_OPS(mcast_cts);
MWLWIFI_DEBUGFS_FILE_WRITE_OPS(wmmedcaap);

void mwl_debugfs_init(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (!priv->debugfs_phy)
		priv->debugfs_phy = debugfs_create_dir("mwlwifi",
						       hw->wiphy->debugfsdir);

	if (!priv->debugfs_phy)
		return;

	MWLWIFI_DEBUGFS_ADD_FILE(info);
	MWLWIFI_DEBUGFS_ADD_FILE(tx_status);
	MWLWIFI_DEBUGFS_ADD_FILE(rx_status);
	MWLWIFI_DEBUGFS_ADD_FILE(vif);
	MWLWIFI_DEBUGFS_ADD_FILE(sta);
	MWLWIFI_DEBUGFS_ADD_FILE(ampdu);
	MWLWIFI_DEBUGFS_ADD_FILE(stnid);
	MWLWIFI_DEBUGFS_ADD_FILE(device_pwrtbl);
	MWLWIFI_DEBUGFS_ADD_FILE(txpwrlmt);
	MWLWIFI_DEBUGFS_ADD_FILE(tx_amsdu);
	MWLWIFI_DEBUGFS_ADD_FILE(dump_hostcmd);
	MWLWIFI_DEBUGFS_ADD_FILE(dump_probe);
	MWLWIFI_DEBUGFS_ADD_FILE(heartbeat);
	MWLWIFI_DEBUGFS_ADD_FILE(dfs_test);
	MWLWIFI_DEBUGFS_ADD_FILE(dfs_channel);
	MWLWIFI_DEBUGFS_ADD_FILE(dfs_radar);
	MWLWIFI_DEBUGFS_ADD_FILE(thermal);
	MWLWIFI_DEBUGFS_ADD_FILE(led_ctrl);
	MWLWIFI_DEBUGFS_ADD_FILE(regrdwr);
	MWLWIFI_DEBUGFS_ADD_FILE(ratetable);
	MWLWIFI_DEBUGFS_ADD_FILE(tx_hist);
	MWLWIFI_DEBUGFS_ADD_FILE(ba_hist);
	MWLWIFI_DEBUGFS_ADD_FILE(fixed_rate);
	MWLWIFI_DEBUGFS_ADD_FILE(core_dump);
	MWLWIFI_DEBUGFS_ADD_FILE(mcast_cts);
	MWLWIFI_DEBUGFS_ADD_FILE(wmmedcaap);
}

void mwl_debugfs_remove(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	debugfs_remove(priv->debugfs_phy);
	priv->debugfs_phy = NULL;
}
