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
#include <linux/devcoredump.h>
#include <linux/etherdevice.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"
#include "thermal.h"
#include "hif/fwcmd.h"
#include "hif/pcie/dev.h"
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

static ssize_t mwl_debugfs_info_read(struct file *file, char __user *ubuf,
				     size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	unsigned long flags;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	int tx_num = 4, rx_num = 4;
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;
	const struct hostcmd_get_hw_spec *get_hw_spec;
	int i;

	if (!p)
		return -ENOMEM;

	/* get and prepare HW specifications */
	get_hw_spec = mwl_fwcmd_get_hw_specs(priv->hw);
	if (!get_hw_spec) {
		wiphy_err(priv->hw->wiphy, "fail to get HW specifications\n");
		goto err_get_hw_specs;
	}
	len += scnprintf(p + len, size - len,
			 "host_if: %d\n", get_hw_spec->host_if);
	len += scnprintf(p + len, size - len,
			 "num_antenna: %d\n", le16_to_cpu(get_hw_spec->num_antenna));
	len += scnprintf(p + len, size - len,
			 "region_code: %d\n", le16_to_cpu(get_hw_spec->region_code));
	len += scnprintf(p + len, size - len,
			 "num_wcb: %d\n", le32_to_cpu(get_hw_spec->num_wcb));

	if (priv->chip_type == MWL8864) {
		len += scnprintf(p + len, size - len,
			 "-------------  address|qlen|fw_desc_cnt\n");
		spin_lock_irqsave(&pcie_priv->tx_desc_lock, flags);
		len += scnprintf(p + len, size - len,
				"wcb_base0   : %8x|%4d|%d\n",
				le32_to_cpu(get_hw_spec->wcb_base0),
				skb_queue_len(&pcie_priv->txq[0]),
				pcie_priv->fw_desc_cnt[0]);
		for(i = 0; i < SYSADPT_TOTAL_TX_QUEUES - 1; i++)
			len += scnprintf(p + len, size - len,
				"wcb_base[%2d]: %8x|%4d|%d\n", i,
				le32_to_cpu(get_hw_spec->wcb_base[i]),
				skb_queue_len(&pcie_priv->txq[i + 1]),
				pcie_priv->fw_desc_cnt[i + 1]);
		spin_unlock_irqrestore(&pcie_priv->tx_desc_lock, flags);
	}

	len += scnprintf(p + len, size - len,
			 "num_mcast_addr: %X\n", le16_to_cpu(get_hw_spec->num_mcast_addr));
	len += scnprintf(p + len, size - len,
			 "permanent mac address: %pM\n", get_hw_spec->permanent_addr);
	len += scnprintf(p + len, size - len,
			 "fw_awake_cookie: %d\n", le32_to_cpu(get_hw_spec->fw_awake_cookie));
err_get_hw_specs:
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
	len += scnprintf(p + len, size - len,
			 "radio_short_preamble: %s\n", priv->radio_short_preamble ? "enable" : "disable");
	len += scnprintf(p + len, size - len,
			 "use_short_preamble: %s\n", priv->use_short_preamble ? "enable" : "disable");
	len += mwl_hif_get_info(priv->hw, p + len, size - len);
	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_rx_decrypt_read(struct file *file, char __user *ubuf,
					   size_t count, loff_t *ppos) {
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "%5s\n", priv->rx_decrypt ? "true" : "false");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;

}

static ssize_t mwl_debugfs_rx_decrypt_write(struct file *file,
					  const char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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

	priv->rx_decrypt = value ? true : false;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_tx_status_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += mwl_hif_get_tx_status(priv->hw, p + len, size - len);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_rx_status_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += mwl_hif_get_rx_status(priv->hw, p + len, size - len);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_vif_read(struct file *file, char __user *ubuf,
				    size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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

	spin_lock_bh(&priv->vif_lock);
	list_for_each_entry(mwl_vif, &priv->vif_list, list) {
		vif = container_of((void *)mwl_vif, struct ieee80211_vif,
				   drv_priv);
		len += scnprintf(p + len, size - len,
				 "macid: %d\n", mwl_vif->macid);
		switch (vif->type) {
		case NL80211_IFTYPE_AP:
			len += scnprintf(p + len, size - len, "type: ap\n");
			memcpy(ssid, vif->cfg.ssid,
			       vif->cfg.ssid_len);
			ssid[vif->cfg.ssid_len] = 0;
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
		if (vif->bss_conf.chanctx_conf) {
			chan_def = &vif->bss_conf.chanctx_conf->def;
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
		beacon_info = &mwl_vif->beacon_info;
		dump_data(p, size, &len, beacon_info->ie_wmm_ptr,
			  beacon_info->ie_wmm_len, "WMM:");
		dump_data(p, size, &len, beacon_info->ie_wpa_ptr,
			  beacon_info->ie_wpa_len, "WPA:");
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
	}
	spin_unlock_bh(&priv->vif_lock);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_sta_read(struct file *file, char __user *ubuf,
				    size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "       macaddress|aid|ampdu|amsdu|  wds|ba_hist|amsdu_cap|  ht cap, ampdu,         rx_mask|   vht_cap|           mcs|rx bw,nss|tdls|init|wme|mfp\n");

	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv);

		len += scnprintf(p + len, size - len, "%pM|%3d|%5s|%5s|%5s|%7s|     0x%02x|0x%06x, %02x-%02x,0x%02x, %02x, %02x, %02x|0x%08x|0x%04x, 0x%04x|rx %2d, %2d|%4d|%4d|%3d|%3d\n",
			sta->addr,
			sta->aid,
			sta_info->is_ampdu_allowed ? "true" : "false",
			sta_info->is_amsdu_allowed ? "true" : "false",
			sta_info->wds ? "true" : "false",
			sta_info->ba_hist.enable ? "enable" : "disable",
			sta_info->is_amsdu_allowed ? sta_info->amsdu_ctrl.cap : 0 ,
			sta->deflink.ht_cap.ht_supported ? sta->deflink.ht_cap.cap : 0,
			sta->deflink.ht_cap.ht_supported ? sta->deflink.ht_cap.ampdu_factor : 0,
			sta->deflink.ht_cap.ht_supported ? sta->deflink.ht_cap.ampdu_density : 0,
			sta->deflink.ht_cap.ht_supported ? sta->deflink.ht_cap.mcs.rx_mask[0] : 0,
			sta->deflink.ht_cap.ht_supported ? sta->deflink.ht_cap.mcs.rx_mask[1] : 0,
			sta->deflink.ht_cap.ht_supported ? sta->deflink.ht_cap.mcs.rx_mask[2] : 0,
			sta->deflink.ht_cap.ht_supported ? sta->deflink.ht_cap.mcs.rx_mask[3] : 0,
			sta->deflink.vht_cap.vht_supported ? sta->deflink.vht_cap.cap : 0,
			sta->deflink.vht_cap.vht_supported ? sta->deflink.vht_cap.vht_mcs.rx_mcs_map : 0,
			sta->deflink.vht_cap.vht_supported ? sta->deflink.vht_cap.vht_mcs.tx_mcs_map : 0,
			sta->deflink.bandwidth,
			sta->deflink.rx_nss,
			sta->tdls,
			sta->tdls_initiator,
			sta->wme,
			sta->mfp
			);
	}
	spin_unlock_bh(&priv->sta_lock);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_ampdu_read(struct file *file, char __user *ubuf,
				      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_ampdu_stream *stream;
	int i;
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;
	ssize_t ret;
	struct mwl_tx_info *tx_stats;

	if (!p)
		return -ENOMEM;

	if (priv->chip_type == MWL8964)
		return -EPERM;

	len += scnprintf(p + len, size - len, "debug_ampdu: %d\n", priv->debug_ampdu);
	len += scnprintf(p + len, size - len, "=================\n");

	spin_lock_bh(&priv->stream_lock);
	len += scnprintf(p + len, size - len, "stream|idx|state|       macaddress|tid|pkts|remain|start time\n");
	for (i = 0; i < priv->ampdu_num; i++) {
		stream = &priv->ampdu[i];
		if (!stream->state)
			continue;

		if (stream->sta) {
			sta_info = mwl_dev_get_sta(stream->sta);
			tx_stats = &sta_info->tx_stats[stream->tid];
			len += scnprintf(p + len, size - len, "%6d|%3u|%5u|%pM|%3u|%4u|%6lu|%lu\n", i,
			stream->idx,
			stream->state,
			stream->sta->addr,
			stream->tid,
			tx_stats->pkts,
			(SYSADPT_TIMER_AMPDU_KEEPALIVE - (jiffies - stream->jiffies))/ HZ,
			abs(jiffies - stream->start_time) / HZ
			);

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

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_ampdu_write(struct file *file,
					  const char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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

	priv->debug_ampdu = value ? true : false;

	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_stnid_read(struct file *file, char __user *ubuf,
				      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	struct mwl_stnid *stnid;
	int i;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

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

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_device_pwrtbl_read(struct file *file,
					      char __user *ubuf,
					      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	int i, j;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

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

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_txpwrlmt_read(struct file *file,
					 char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;

	return simple_read_from_buffer(ubuf, count, ppos,
				       priv->txpwrlmt_data.buf,
				       priv->txpwrlmt_data.len);
}

static ssize_t mwl_debugfs_tx_amsdu_read(struct file *file,
					 char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "tx amsdu: %s\n",
			 priv->tx_amsdu ? "enable" : "disable");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_tx_amsdu_write(struct file *file,
					  const char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "dump_hostcmd: %s\n",
			 priv->dump_hostcmd ? "enable" : "disable");

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_dump_hostcmd_write(struct file *file,
					      const char __user *ubuf,
					      size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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

static ssize_t mwl_debugfs_heartbeat_read(struct file *file,
					  char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	len += scnprintf(p + len, size - len, "heartbeat: %d\n",
			 priv->heartbeat);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_heartbeat_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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

static ssize_t mwl_debugfs_thermal_read(struct file *file,
					char __user *ubuf,
					size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page = get_zeroed_page(GFP_KERNEL);
	char *p = (char *)page;
	int len = 0, size = PAGE_SIZE;
	ssize_t ret;

	if (!p)
		return -ENOMEM;

	mwl_fwcmd_get_temp(priv->hw, &priv->temperature);

	len += scnprintf(p + len, size - len, "quiet period: %d\n",
			 priv->quiet_period);
	len += scnprintf(p + len, size - len, "throttle state: %d\n",
			 priv->throttle_state);
	len += scnprintf(p + len, size - len, "temperature: %d\n",
			 priv->temperature);

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);
	free_page(page);

	return ret;
}

static ssize_t mwl_debugfs_thermal_write(struct file *file,
					 const char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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

static ssize_t mwl_debugfs_ratetable_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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
			idx, fmt, stbc, bw, sgi, nss, rate,
			preamble_gf, power_id, ldpc, bf,
			ant,
			utils_get_phy_rate(fmt, bw, sgi, mcs));

		idx++;
		rate_idx += (2 * sizeof(__le32));
		rate_info = le32_to_cpu(*(__le32 *)rate_idx);
	}

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
	struct mwl_priv *priv = file->private_data;
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
	struct mwl_priv *priv = file->private_data;
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
	struct mwl_priv *priv = file->private_data;
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
	struct mwl_priv *priv = file->private_data;
	/* header and summary lines, plus one 36 byte line per entry */
	size_t size = 256 + ACNT_BA_SIZE * 36;
	char *p;
	int len = 0;
	struct mwl_sta *sta_info;
	struct mwl_tx_ba_stats *ba_stats;
	u32 i, data;
	u32 baholecnt, baexpcnt, bmap0cnt, nobacnt;
	u8 bmap0flag, nobaflag;
	ssize_t ret;

	if (!priv->ba_aid)
		return -EINVAL;

	p = kvzalloc(size, GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	sta_info = utils_find_sta_by_aid(priv, priv->ba_aid);
	if (sta_info && sta_info->ba_hist.enable &&
	    sta_info->ba_hist.ba_stats) {
		ba_stats = sta_info->ba_hist.ba_stats;
		len += scnprintf(p + len, size - len,
				 "BA histogram aid: %d, stnid: %d type: %s\n",
				 priv->ba_aid, sta_info->stnid,
				 sta_info->ba_hist.type ? "MU" : "SU");
		len += scnprintf(p + len, size - len, "%8s,%8s,%8s,%8s\n",
				 "BAhole", "Expect", "Bmap0", "NoBA");
		baholecnt = 0;
		baexpcnt = 0;
		bmap0cnt = 0;
		nobacnt = 0;
		for (i = 0; i < ACNT_BA_SIZE; i++) {
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

			len += scnprintf(p + len, size - len, "%8d,%8d,",
					 ba_stats[i].ba_hole,
					 ba_stats[i].ba_expected);

			baholecnt += ba_stats[i].ba_hole;
			baexpcnt += ba_stats[i].ba_expected;
			if (bmap0flag) {
				len += scnprintf(p + len, size - len,
						 "       #,");
				bmap0cnt++;
			} else {
				len += scnprintf(p + len, size - len, "%8d,",
						 bmap0flag);
			}
			if (nobaflag) {
				len += scnprintf(p + len, size - len,
						 "       *\n");
				nobacnt++;
			} else {
				len += scnprintf(p + len, size - len, "%8d\n",
						 nobaflag);
			}
		}

		len += scnprintf(p + len, size - len,
				 "hole: %d, expect: %d, bmap0: %d, noba: %d\n",
				 baholecnt, baexpcnt, bmap0cnt, nobacnt);
	} else {
		len += scnprintf(p + len, size - len,
				 "No BA histogram for sta aid: %d\n",
				 priv->ba_aid);
	}

	ret = simple_read_from_buffer(ubuf, count, ppos, p, len);

	kvfree(p);
	return ret;
}

static ssize_t mwl_debugfs_ba_hist_write(struct file *file,
					 const char __user *ubuf,
					 size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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

/* Upper bound on the dump size, in case the firmware reports nonsense */
#define MWL_COREDUMP_MAX_SIZE (16 * 1024 * 1024)

/* Collect the firmware memory regions and hand them to devcoredump. The
 * dump is the struct coredump header followed by each region's contents
 * in order. The read returns a summary of the regions.
 */
static ssize_t mwl_debugfs_core_dump_read(struct file *file, char __user *ubuf,
					  size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
	unsigned long page;
	char *p;
	int len = 0, size = PAGE_SIZE;
	struct coredump_cmd *core_dump = NULL;
	struct coredump *cd = NULL;
	char *buff = NULL;
	u8 *dump = NULL;
	size_t dump_size, pos;
	u32 i, offset, chunk;
	u32 address, length;
	ssize_t ret;

	if (priv->chip_type != MWL8964)
		return -EPERM;

	if (!IS_ENABLED(CONFIG_DEV_COREDUMP))
		return -EOPNOTSUPP;

	if (*ppos)
		return 0;

	page = get_zeroed_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;
	p = (char *)page;

	core_dump = kzalloc_obj(*core_dump);
	buff = kzalloc(MAX_CORE_DUMP_BUFFER, GFP_KERNEL);
	cd = kmalloc_obj(*cd);
	if (!core_dump || !buff || !cd) {
		ret = -ENOMEM;
		goto err;
	}

	if (mwl_fwcmd_get_fw_core_dump(priv->hw, core_dump, buff)) {
		ret = -EIO;
		goto err;
	}
	memcpy(cd, buff, sizeof(*cd));

	if (cd->num_regions > MAX_CORE_REGIONS) {
		wiphy_err(priv->hw->wiphy, "core dump: %d regions\n",
			  cd->num_regions);
		ret = -EIO;
		goto err;
	}

	/* check each region before adding it so the sum can't wrap */
	dump_size = sizeof(*cd);
	for (i = 0; i < cd->num_regions; i++) {
		length = le32_to_cpu(cd->region[i].length);
		if (length > MWL_COREDUMP_MAX_SIZE - dump_size) {
			wiphy_err(priv->hw->wiphy,
				  "core dump: region %u length %x too large\n",
				  i, length);
			ret = -EIO;
			goto err;
		}
		dump_size += length;
	}

	dump = vzalloc(dump_size);
	if (!dump) {
		ret = -ENOMEM;
		goto err;
	}
	memcpy(dump, cd, sizeof(*cd));
	pos = sizeof(*cd);

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
			chunk = min_t(u32, length - offset,
				      MAX_CORE_DUMP_BUFFER);
			memcpy(dump + pos + offset, buff, chunk);
		}
		pos += length;
	}

	/* devcoredump takes ownership of the buffer */
	dev_coredumpv(wiphy_dev(priv->hw->wiphy), dump, dump_size,
		      GFP_KERNEL);
	len += scnprintf(p + len, size - len,
			 "\nDump of %zu bytes available through devcoredump\n",
			 dump_size);

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
	struct mwl_priv *priv = file->private_data;
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

	/* The dump used to be written to /dev/shm in text or binary form;
	 * it now goes through devcoredump, so the value is only validated.
	 */
	if (kstrtoint(buf, 0, &text_mode) || text_mode < 0 || text_mode > 1) {
		ret = -EINVAL;
		goto err;
	}

	mwl_fwcmd_core_dump_diag_mode(priv->hw, 1);
	ret = count;

err:
	free_page(addr);
	return ret;
}

static ssize_t mwl_debugfs_mcast_cts_write(struct file *file,
					   const char __user *ubuf,
					   size_t count, loff_t *ppos)
{
	struct mwl_priv *priv = file->private_data;
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
	struct mwl_priv *priv = file->private_data;
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
MWLWIFI_DEBUGFS_FILE_READ_OPS(stnid);
MWLWIFI_DEBUGFS_FILE_READ_OPS(device_pwrtbl);
MWLWIFI_DEBUGFS_FILE_READ_OPS(txpwrlmt);
MWLWIFI_DEBUGFS_FILE_OPS(ampdu);
MWLWIFI_DEBUGFS_FILE_OPS(tx_amsdu);
MWLWIFI_DEBUGFS_FILE_OPS(rx_decrypt);
MWLWIFI_DEBUGFS_FILE_OPS(dump_hostcmd);
MWLWIFI_DEBUGFS_FILE_OPS(heartbeat);
MWLWIFI_DEBUGFS_FILE_OPS(thermal);
MWLWIFI_DEBUGFS_FILE_OPS(ratetable);
MWLWIFI_DEBUGFS_FILE_OPS(tx_hist);
MWLWIFI_DEBUGFS_FILE_OPS(ba_hist);
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
	MWLWIFI_DEBUGFS_ADD_FILE(rx_decrypt);
	MWLWIFI_DEBUGFS_ADD_FILE(rx_status);
	MWLWIFI_DEBUGFS_ADD_FILE(vif);
	MWLWIFI_DEBUGFS_ADD_FILE(sta);
	MWLWIFI_DEBUGFS_ADD_FILE(ampdu);
	MWLWIFI_DEBUGFS_ADD_FILE(stnid);
	MWLWIFI_DEBUGFS_ADD_FILE(device_pwrtbl);
	MWLWIFI_DEBUGFS_ADD_FILE(txpwrlmt);
	MWLWIFI_DEBUGFS_ADD_FILE(tx_amsdu);
	MWLWIFI_DEBUGFS_ADD_FILE(dump_hostcmd);
	MWLWIFI_DEBUGFS_ADD_FILE(heartbeat);
	MWLWIFI_DEBUGFS_ADD_FILE(thermal);
	MWLWIFI_DEBUGFS_ADD_FILE(ratetable);
	MWLWIFI_DEBUGFS_ADD_FILE(tx_hist);
	MWLWIFI_DEBUGFS_ADD_FILE(ba_hist);
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
