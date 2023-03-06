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

/* Description:  This file implements core layer related functions. */

#include <linux/etherdevice.h>

#include "sysadpt.h"
#include "core.h"
#include "vendor_cmd.h"
#include "thermal.h"
#include "debugfs.h"
#include "hif/fwcmd.h"
#include "hif/hif-ops.h"

#define CMD_BUF_SIZE     0x4000
#define INVALID_WATCHDOG 0xAA

static const struct ieee80211_channel mwl_channels_24[] = {
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2412, .hw_value = 1, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2417, .hw_value = 2, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2422, .hw_value = 3, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2427, .hw_value = 4, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2432, .hw_value = 5, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2437, .hw_value = 6, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2442, .hw_value = 7, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2447, .hw_value = 8, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2452, .hw_value = 9, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2457, .hw_value = 10, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2462, .hw_value = 11, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2467, .hw_value = 12, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2472, .hw_value = 13, },
	{ .band = NL80211_BAND_2GHZ, .center_freq = 2484, .hw_value = 14, },
};

static const struct ieee80211_rate mwl_rates_24[] = {
	{ .bitrate = 10, .hw_value = 2, },
	{ .bitrate = 20, .hw_value = 4, },
	{ .bitrate = 55, .hw_value = 11, },
	{ .bitrate = 110, .hw_value = 22, },
	{ .bitrate = 220, .hw_value = 44, },
	{ .bitrate = 60, .hw_value = 12, },
	{ .bitrate = 90, .hw_value = 18, },
	{ .bitrate = 120, .hw_value = 24, },
	{ .bitrate = 180, .hw_value = 36, },
	{ .bitrate = 240, .hw_value = 48, },
	{ .bitrate = 360, .hw_value = 72, },
	{ .bitrate = 480, .hw_value = 96, },
	{ .bitrate = 540, .hw_value = 108, },
};

static const struct ieee80211_channel mwl_channels_50[] = {
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5180, .hw_value = 36, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5200, .hw_value = 40, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5220, .hw_value = 44, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5240, .hw_value = 48, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5260, .hw_value = 52, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5280, .hw_value = 56, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5300, .hw_value = 60, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5320, .hw_value = 64, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5500, .hw_value = 100, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5520, .hw_value = 104, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5540, .hw_value = 108, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5560, .hw_value = 112, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5580, .hw_value = 116, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5600, .hw_value = 120, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5620, .hw_value = 124, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5640, .hw_value = 128, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5660, .hw_value = 132, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5680, .hw_value = 136, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5700, .hw_value = 140, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5720, .hw_value = 144, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5745, .hw_value = 149, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5765, .hw_value = 153, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5785, .hw_value = 157, },
	{ .band = NL80211_BAND_5GHZ, .center_freq = 5805, .hw_value = 161, },
};

static const struct ieee80211_rate mwl_rates_50[] = {
	{ .bitrate = 60, .hw_value = 12, },
	{ .bitrate = 90, .hw_value = 18, },
	{ .bitrate = 120, .hw_value = 24, },
	{ .bitrate = 180, .hw_value = 36, },
	{ .bitrate = 240, .hw_value = 48, },
	{ .bitrate = 360, .hw_value = 72, },
	{ .bitrate = 480, .hw_value = 96, },
	{ .bitrate = 540, .hw_value = 108, },
};

static const struct ieee80211_iface_limit ap_if_limits[] = {
	{ .max = SYSADPT_NUM_OF_AP, .types = BIT(NL80211_IFTYPE_AP) },
#if defined(CPTCFG_MAC80211_MESH) || defined(CONFIG_MAC80211_MESH)
	{ .max = SYSADPT_NUM_OF_MESH, .types = BIT(NL80211_IFTYPE_MESH_POINT) },
#endif
	{ .max = SYSADPT_NUM_OF_CLIENT, .types = BIT(NL80211_IFTYPE_STATION) },
};

static const struct ieee80211_iface_combination ap_if_comb = {
	.limits = ap_if_limits,
	.n_limits = ARRAY_SIZE(ap_if_limits),
	.max_interfaces = SYSADPT_NUM_OF_AP,
	.num_different_channels = 1,
	.radar_detect_widths =	BIT(NL80211_CHAN_WIDTH_20_NOHT) |
				BIT(NL80211_CHAN_WIDTH_20) |
				BIT(NL80211_CHAN_WIDTH_40) |
				BIT(NL80211_CHAN_WIDTH_80) |
				BIT(NL80211_CHAN_WIDTH_160),
};

struct region_code_mapping {
	const char *alpha2;
	u32 region_code;
};

static const struct region_code_mapping regmap[] = {
	{"US", 0x10}, /* US FCC */
	{"CA", 0x20}, /* Canada */
	{"DE", 0x30}, /* France */
	{"ES", 0x31}, /* Spain  */
	{"FR", 0x32}, /* France */
	{"JP", 0x40}, /* Japan  */
	{"TW", 0x80}, /* Taiwan */
	{"AU", 0x81}, /* Australia */
	{"CN", 0x90}, /* China (Asia) */
};

static int mwl_prepare_cmd_buf(struct mwl_priv *priv)
{
	priv->pcmd_buf =
		(unsigned short *)dmam_alloc_coherent(priv->dev,
						      CMD_BUF_SIZE,
						      &priv->pphys_cmd_buf,
						      GFP_KERNEL);
	if (!priv->pcmd_buf) {
		wiphy_err(priv->hw->wiphy,
			  "cannot alloc memory for command buffer\n");
		goto err;
	}
	wiphy_debug(priv->hw->wiphy,
		    "priv->pcmd_buf = %p  priv->pphys_cmd_buf = %p\n",
		    priv->pcmd_buf,
		    (void *)priv->pphys_cmd_buf);
	memset(priv->pcmd_buf, 0x00, CMD_BUF_SIZE);

	return 0;

err:
	wiphy_err(priv->hw->wiphy, "command buffer alloc fail\n");

	return -EIO;
}

static int mwl_init_firmware(struct mwl_priv *priv, const char *fw_name,
			     const char *cal_name, const char *txpwrlmt_name)
{
	int rc = 0;

	rc = request_firmware((const struct firmware **)&priv->fw_ucode,
			      fw_name, priv->dev);

	if (rc) {
		wiphy_err(priv->hw->wiphy,
			  "cannot find firmware image <%s>\n", fw_name);
		goto err_load_fw;
	}

	rc = mwl_hif_download_firmware(priv->hw);
	if (rc) {
		wiphy_err(priv->hw->wiphy,
			  "cannot download firmware image <%s>\n", fw_name);
		goto err_download_fw;
	}

	if (cal_name) {
		if ((request_firmware((const struct firmware **)&priv->cal_data,
		     cal_name, priv->dev)) < 0)
			wiphy_debug(priv->hw->wiphy,
				    "cannot find calibtration data\n");
	}

	if (txpwrlmt_name) {
		if ((request_firmware(
		     (const struct firmware **)&priv->txpwrlmt_file,
		     txpwrlmt_name, priv->dev)) < 0)
			wiphy_debug(priv->hw->wiphy,
				    "cannot find tx power limit data\n");
	}

	return rc;

err_download_fw:

	release_firmware(priv->fw_ucode);

err_load_fw:

	wiphy_err(priv->hw->wiphy, "firmware init fail\n");

	return rc;
}

static void mwl_process_of_dts(struct mwl_priv *priv)
{
#ifdef CONFIG_OF
	struct property *prop;
	u32 prop_value;

	priv->dt_node =
		of_find_node_by_name(mwl_hif_device_node(priv->hw),
				     "mwlwifi");
	if (!priv->dt_node)
		return;

	/* look for all matching property names */
	for_each_property_of_node(priv->dt_node, prop) {
		if (strcmp(prop->name, "marvell,2ghz") == 0)
			priv->disable_2g = true;
		if (strcmp(prop->name, "marvell,5ghz") == 0)
			priv->disable_5g = true;
		if (strcmp(prop->name, "marvell,chainmask") == 0) {
			prop_value = be32_to_cpu(*((__be32 *)prop->value));
			if (prop_value == 2)
				priv->antenna_tx = ANTENNA_TX_2;
			else if (prop_value == 3)
				priv->antenna_tx = ANTENNA_TX_3;

			prop_value = be32_to_cpu(*((__be32 *)
						 (prop->value + 4)));
			if (prop_value == 2)
				priv->antenna_rx = ANTENNA_RX_2;
			else if (prop_value == 3)
				priv->antenna_rx = ANTENNA_RX_3;
		}
	}

	priv->pwr_node = of_find_node_by_name(priv->dt_node,
					      "marvell,powertable");
#endif
}

static void mwl_reg_notifier(struct wiphy *wiphy,
			     struct regulatory_request *request)
{
	struct ieee80211_hw *hw;
	struct mwl_priv *priv;
#ifdef CONFIG_OF
	struct property *prop;
	struct property *fcc_prop = NULL;
	struct property *etsi_prop = NULL;
	struct property *specific_prop = NULL;
	u32 prop_value;
	int i, j, k;
#endif

	hw = wiphy_to_ieee80211_hw(wiphy);
	priv = hw->priv;

	if (priv->forbidden_setting) {
		if (!priv->regulatory_set) {
			regulatory_hint(wiphy, priv->fw_alpha2);
			priv->regulatory_set = true;
		} else {
			if (memcmp(priv->fw_alpha2, request->alpha2, 2))
				regulatory_hint(wiphy, priv->fw_alpha2);
		}
		return;
	}

	priv->dfs_region = request->dfs_region;

#ifdef CONFIG_OF
	if ((priv->chip_type != MWL8997) && (priv->pwr_node)) {
		for_each_property_of_node(priv->pwr_node, prop) {
			if (strcmp(prop->name, "FCC") == 0)
				fcc_prop = prop;
			if (strcmp(prop->name, "ETSI") == 0)
				etsi_prop = prop;
			if ((prop->name[0] == request->alpha2[0]) &&
			    (prop->name[1] == request->alpha2[1]))
				specific_prop = prop;
		}

		prop = NULL;

		if (specific_prop) {
			prop = specific_prop;
		} else {
			if (priv->dfs_region == NL80211_DFS_ETSI)
				prop = etsi_prop;
			else
				prop = fcc_prop;
		}

		if (prop) {
			/* Reset the whole table */
			for (i = 0; i < SYSADPT_MAX_NUM_CHANNELS; i++)
				memset(&priv->tx_pwr_tbl[i], 0,
				       sizeof(struct mwl_tx_pwr_tbl));

			/* Load related power table */
			i = 0;
			j = 0;
			while (i < prop->length) {
				prop_value =
					be32_to_cpu(*(__be32 *)
						    (prop->value + i));
				priv->tx_pwr_tbl[j].channel = prop_value;
				i += 4;
				prop_value =
					be32_to_cpu(*(__be32 *)
						    (prop->value + i));
				priv->tx_pwr_tbl[j].setcap = prop_value;
				i += 4;
				for (k = 0; k < SYSADPT_TX_POWER_LEVEL_TOTAL;
				     k++) {
					prop_value =
						be32_to_cpu(*(__be32 *)
							    (prop->value + i));
					priv->tx_pwr_tbl[j].tx_power[k] =
						prop_value;
					i += 4;
				}
				prop_value =
					be32_to_cpu(*(__be32 *)
						    (prop->value + i));
				priv->tx_pwr_tbl[j].cdd =
					(prop_value == 0) ? false : true;
				i += 4;
				prop_value =
					be32_to_cpu(*(__be32 *)
						    (prop->value + i));
				priv->tx_pwr_tbl[j].txantenna2 = prop_value;
				i += 4;
				j++;
			}

			/* Dump loaded power tabel */
			wiphy_debug(hw->wiphy, "regdomain: %s\n", prop->name);
			for (i = 0; i < SYSADPT_MAX_NUM_CHANNELS; i++) {
				struct mwl_tx_pwr_tbl *pwr_tbl;
				char disp_buf[64];
				char *disp_ptr;

				pwr_tbl = &priv->tx_pwr_tbl[i];
				if (pwr_tbl->channel == 0)
					break;
				wiphy_debug(hw->wiphy,
					    "Channel: %d: 0x%x 0x%x 0x%x\n",
					    pwr_tbl->channel,
					    pwr_tbl->setcap,
					    pwr_tbl->cdd,
					    pwr_tbl->txantenna2);
				disp_ptr = disp_buf;
				for (j = 0; j < SYSADPT_TX_POWER_LEVEL_TOTAL;
				     j++) {
					disp_ptr +=
						sprintf(disp_ptr, "%x ",
							pwr_tbl->tx_power[j]);
				}
				wiphy_debug(hw->wiphy, "%s\n", disp_buf);
			}
		}
	}
#endif
}

static void mwl_regd_init(struct mwl_priv *priv)
{
	u8 region_code;
	int rc;
	int i;

	/* hook regulatory domain change notification */
	priv->hw->wiphy->reg_notifier = mwl_reg_notifier;

	if (priv->chip_type == MWL8964)
		rc = mwl_fwcmd_get_pwr_tbl_sc4(priv->hw,
					       &priv->device_pwr_tbl[0],
					       &region_code,
					       &priv->number_of_channels,
					       0);
	else
		rc = mwl_fwcmd_get_device_pwr_tbl(priv->hw,
						  &priv->device_pwr_tbl[0],
						  &region_code,
						  &priv->number_of_channels,
						  0);
	if (rc)
		return;

	priv->forbidden_setting = true;

	for (i = 1; i < priv->number_of_channels; i++) {
		if (priv->chip_type == MWL8964)
			mwl_fwcmd_get_pwr_tbl_sc4(priv->hw,
						  &priv->device_pwr_tbl[i],
						  &region_code,
						  &priv->number_of_channels,
						  i);
		else
			mwl_fwcmd_get_device_pwr_tbl(priv->hw,
						     &priv->device_pwr_tbl[i],
						     &region_code,
						     &priv->number_of_channels,
						     i);
	}

	for (i = 0; i < ARRAY_SIZE(regmap); i++)
		if (regmap[i].region_code == priv->fw_region_code) {
			memcpy(priv->fw_alpha2, regmap[i].alpha2, 2);
			break;
		}
}

static void mwl_set_ht_caps(struct mwl_priv *priv,
			    struct ieee80211_supported_band *band)
{
	struct ieee80211_hw *hw;
	const u8 ant_rx_no[ANTENNA_RX_MAX] = { 3, 1, 2, 3};
	int i;

	hw = priv->hw;

	band->ht_cap.ht_supported = 1;
	if (priv->chip_type == MWL8964)
		band->ht_cap.cap |= IEEE80211_HT_CAP_MAX_AMSDU;
	band->ht_cap.cap |= IEEE80211_HT_CAP_LDPC_CODING;
	band->ht_cap.cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	band->ht_cap.cap |= IEEE80211_HT_CAP_SM_PS;
	band->ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;
	band->ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;
	band->ht_cap.cap |= IEEE80211_HT_CAP_DSSSCCK40;

	if ((priv->chip_type == MWL8997) &&
	    (priv->antenna_tx != ANTENNA_TX_1)) {
		band->ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;
		band->ht_cap.cap |= (1 << IEEE80211_HT_CAP_RX_STBC_SHIFT);
	}

	ieee80211_hw_set(hw, AMPDU_AGGREGATION);
	ieee80211_hw_set(hw, SUPPORTS_AMSDU_IN_AMPDU);
	band->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
	band->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_4;

	for (i = 0; i < ant_rx_no[priv->antenna_rx]; i++)
		band->ht_cap.mcs.rx_mask[i] = 0xff;
	band->ht_cap.mcs.rx_mask[4] = 0x01;

	band->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
	if  (priv->antenna_tx == ANTENNA_TX_1)
		band->ht_cap.mcs.rx_highest = cpu_to_le16(150);
	if  (priv->antenna_tx == ANTENNA_TX_2)
		band->ht_cap.mcs.rx_highest = cpu_to_le16(300);
	if  (priv->antenna_tx == ANTENNA_TX_4_AUTO)
		band->ht_cap.mcs.rx_highest = cpu_to_le16(450);
}

static void mwl_set_vht_caps(struct mwl_priv *priv,
			     struct ieee80211_supported_band *band, bool on)
{
	u32 antenna_num = 4;
	__le16 highest;
	if (!on) {
		band->vht_cap.vht_supported = 0;
		band->vht_cap.cap = 0;
		return;
	} 
	band->vht_cap.vht_supported = 1;

	if (priv->chip_type == MWL8964) {
		band->vht_cap.cap |= IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454;
		band->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_160;
		band->vht_cap.cap |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
	} else
		band->vht_cap.cap |= IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895;
	band->vht_cap.cap |= IEEE80211_VHT_CAP_RXLDPC;
	band->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
	band->vht_cap.cap |= IEEE80211_VHT_CAP_RXSTBC_1;
	if (priv->antenna_tx != ANTENNA_TX_1) {
		band->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
		if (priv->chip_type == MWL8964)
			band->vht_cap.cap |=
				IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;
	}
	band->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
	if (priv->chip_type == MWL8964)
		band->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
	band->vht_cap.cap |= IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
	band->vht_cap.cap |= IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN;
	band->vht_cap.cap |= IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN;
	if (priv->chip_type == MWL8997) {
		if (priv->antenna_tx != ANTENNA_TX_1)
			band->vht_cap.cap |= IEEE80211_VHT_CAP_TXSTBC;
	}

	if (priv->antenna_rx == ANTENNA_RX_1)
		band->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xfffe);
	else if (priv->antenna_rx == ANTENNA_RX_2)
		band->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xfffa);
	else
		band->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xffea);

	if (priv->antenna_tx == ANTENNA_TX_1) {
		band->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xfffe);
		antenna_num = 1;
		highest = cpu_to_le16(390);
	} else if (priv->antenna_tx == ANTENNA_TX_2) {
		band->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xfffa);
		antenna_num = 2;
		highest = cpu_to_le16(780);
	} else{
		band->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xffea);
		highest = cpu_to_le16(1170);
	}

	band->vht_cap.vht_mcs.rx_highest=highest;
	band->vht_cap.vht_mcs.tx_highest=highest;

	if (band->vht_cap.cap & (IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
	    IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE)) {
		band->vht_cap.cap |=
			((antenna_num - 1) <<
			IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT) &
			IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK;
	}

	if (band->vht_cap.cap & (IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE |
	    IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE)) {
		band->vht_cap.cap |=
			((antenna_num - 1) <<
			IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT) &
			IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK;
	}
}

void mwl_set_caps(struct mwl_priv *priv, bool qam256)
{
	struct ieee80211_hw *hw;

	hw = priv->hw;

	/* set up band information for 2.4G */
	if (!priv->disable_2g) {
		BUILD_BUG_ON(sizeof(priv->channels_24) !=
			     sizeof(mwl_channels_24));
		memcpy(priv->channels_24, mwl_channels_24,
		       sizeof(mwl_channels_24));

		BUILD_BUG_ON(sizeof(priv->rates_24) != sizeof(mwl_rates_24));
		memcpy(priv->rates_24, mwl_rates_24, sizeof(mwl_rates_24));

		priv->band_24.band = NL80211_BAND_2GHZ;
		priv->band_24.channels = priv->channels_24;
		priv->band_24.n_channels = ARRAY_SIZE(mwl_channels_24);
		priv->band_24.bitrates = priv->rates_24;
		priv->band_24.n_bitrates = ARRAY_SIZE(mwl_rates_24);

		mwl_set_ht_caps(priv, &priv->band_24);
//		mwl_set_vht_caps(priv, &priv->band_24, qam256);

		hw->wiphy->bands[NL80211_BAND_2GHZ] = &priv->band_24;
	}

	/* set up band information for 5G */
	if (!priv->disable_5g) {
		BUILD_BUG_ON(sizeof(priv->channels_50) !=
			     sizeof(mwl_channels_50));
		memcpy(priv->channels_50, mwl_channels_50,
		       sizeof(mwl_channels_50));

		BUILD_BUG_ON(sizeof(priv->rates_50) != sizeof(mwl_rates_50));
		memcpy(priv->rates_50, mwl_rates_50, sizeof(mwl_rates_50));

		priv->band_50.band = NL80211_BAND_5GHZ;
		priv->band_50.channels = priv->channels_50;
		priv->band_50.n_channels = ARRAY_SIZE(mwl_channels_50);
		priv->band_50.bitrates = priv->rates_50;
		priv->band_50.n_bitrates = ARRAY_SIZE(mwl_rates_50);

		mwl_set_ht_caps(priv, &priv->band_50);
		mwl_set_vht_caps(priv, &priv->band_50, true);

		hw->wiphy->bands[NL80211_BAND_5GHZ] = &priv->band_50;
	}
}

static void mwl_heartbeat_handle(struct work_struct *work)
{
	struct mwl_priv *priv =
		container_of(work, struct mwl_priv, heartbeat_handle);
	u32 val;

	mwl_fwcmd_get_addr_value(priv->hw, 0, 1, &val, 0);
	priv->heartbeating = false;
}

static void mwl_watchdog_ba_events(struct work_struct *work)
{
	int rc;
	u8 bitmap = 0, stream_index;
	struct mwl_ampdu_stream *streams;
	struct mwl_priv *priv =
		container_of(work, struct mwl_priv, watchdog_ba_handle);

	rc = mwl_fwcmd_get_watchdog_bitmap(priv->hw, &bitmap);

	if (rc)
		return;

	spin_lock_bh(&priv->stream_lock);

	/* the bitmap is the hw queue number.  Map it to the ampdu queue. */
	if (bitmap != INVALID_WATCHDOG) {
		if (bitmap == priv->ampdu_num)
			stream_index = 0;
		else if (bitmap > priv->ampdu_num)
			stream_index = bitmap - priv->ampdu_num;
		else
			stream_index = bitmap + 3; /** queue 0 is stream 3*/

		if (bitmap != 0xFF) {
			/* Check if the stream is in use before disabling it */
			streams = &priv->ampdu[stream_index];

			if (streams->state == AMPDU_STREAM_ACTIVE)
				ieee80211_stop_tx_ba_session(streams->sta,
							     streams->tid);
		} else {
			for (stream_index = 0;
			     stream_index < priv->ampdu_num;
			     stream_index++) {
				streams = &priv->ampdu[stream_index];

				if (streams->state != AMPDU_STREAM_ACTIVE)
					continue;

				ieee80211_stop_tx_ba_session(streams->sta,
							     streams->tid);
			}
		}
	}

	spin_unlock_bh(&priv->stream_lock);
}

static void mwl_account_handle(struct work_struct *work)
{
	struct mwl_priv *priv =
		container_of(work, struct mwl_priv, account_handle);

	mwl_hif_process_account(priv->hw);
}

static void mwl_wds_check_handle(struct work_struct *work)
{
	struct mwl_priv *priv =
		container_of(work, struct mwl_priv, wds_check_handle);
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;
	bool wds_sta = false;

	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		if (sta_info->wds)
			continue;
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv);
		if (ether_addr_equal(sta->addr, priv->wds_check_sta)) {
			wds_sta = true;
			break;
		}
	}
	spin_unlock_bh(&priv->sta_lock);

	if (wds_sta) {
		mwl_fwcmd_set_new_stn_wds_sc4(priv->hw, sta->addr);
		sta_info->wds = true;
	}

	priv->wds_check = false;
}

static void mwl_chnl_switch_event(struct work_struct *work)
{
	struct mwl_priv *priv =
		container_of(work, struct mwl_priv, chnl_switch_handle);
	struct mwl_vif *mwl_vif;
	struct ieee80211_vif *vif;

	if (!priv->csa_active) {
		wiphy_err(priv->hw->wiphy,
			  "csa is not active (got channel switch event)\n");
		return;
	}

	spin_lock_bh(&priv->vif_lock);
	list_for_each_entry(mwl_vif, &priv->vif_list, list) {
		vif = container_of((void *)mwl_vif, struct ieee80211_vif,
				   drv_priv);

		if (vif->csa_active)
			ieee80211_csa_finish(vif);
	}
	spin_unlock_bh(&priv->vif_lock);

	wiphy_info(priv->hw->wiphy, "channel switch is done\n");

	priv->csa_active = false;
}

static irqreturn_t mwl_isr(int irq, void *dev_id)
{
	struct ieee80211_hw *hw = dev_id;

	return mwl_hif_irq_handler(hw);
}

static void timer_routine(unsigned long data)
{
	struct ieee80211_hw *hw = (struct ieee80211_hw *)data;
	struct mwl_priv *priv = hw->priv;

	if (priv->heartbeat) {
		if ((jiffies - priv->pre_jiffies) >=
		    msecs_to_jiffies(priv->heartbeat * 1000)) {
			if (!priv->heartbeating) {
				priv->heartbeating = true;
				ieee80211_queue_work(hw,
						     &priv->heartbeat_handle);
			}
			priv->pre_jiffies = jiffies;
		}
	}

	mwl_hif_timer_routine(hw);

	mod_timer(&priv->period_timer, jiffies +
		  msecs_to_jiffies(SYSADPT_TIMER_WAKEUP_TIME));
}

static int mwl_wl_init(struct mwl_priv *priv)
{
	struct ieee80211_hw *hw = priv->hw;
	int rc;
	u16 addr_num;
	struct mac_address *mac_addr;
	u8 last_nibble;
	static const u32 cipher_suites[] = {
		/* keep WEP first, it may be removed below */
		WLAN_CIPHER_SUITE_WEP40,
		WLAN_CIPHER_SUITE_WEP104,
		WLAN_CIPHER_SUITE_TKIP,
		WLAN_CIPHER_SUITE_CCMP,
		WLAN_CIPHER_SUITE_AES_CMAC,
		WLAN_CIPHER_SUITE_BIP_CMAC_256,
		WLAN_CIPHER_SUITE_BIP_GMAC_128,
		WLAN_CIPHER_SUITE_BIP_GMAC_256,
	};

	hw->extra_tx_headroom = mwl_hif_get_tx_head_room(hw);
	hw->queues = SYSADPT_TX_WMM_QUEUES;

	/* Set rssi values to dBm */
	ieee80211_hw_set(hw, SIGNAL_DBM);
	ieee80211_hw_set(hw, HAS_RATE_CONTROL);

	/* Ask mac80211 not to trigger PS mode
	 * based on PM bit of incoming frames.
	 */
	ieee80211_hw_set(hw, AP_LINK_PS);

	ieee80211_hw_set(hw, SUPPORTS_PER_STA_GTK);
	ieee80211_hw_set(hw, MFP_CAPABLE);

	hw->wiphy->flags |= WIPHY_FLAG_IBSS_RSN;
	hw->wiphy->flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
	hw->wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS;
	hw->wiphy->flags |= WIPHY_FLAG_AP_UAPSD;

	hw->vif_data_size = sizeof(struct mwl_vif);
	hw->sta_data_size = sizeof(struct mwl_sta);

	hw->wiphy->cipher_suites = cipher_suites;
	hw->wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);

	priv->ap_macids_supported = 0x0000ffff;
	priv->sta_macids_supported = 0x00010000;
	priv->macids_used = 0;
	INIT_LIST_HEAD(&priv->vif_list);
	INIT_LIST_HEAD(&priv->sta_list);

	/* Set default radio state, preamble and wmm */
	priv->noise = -104;
	priv->radio_on = false;
	priv->radio_short_preamble = false;
	priv->wmm_enabled = false;
	priv->powinited = 0;
	priv->wds_check = false;
	if (priv->chip_type == MWL8997)
		priv->pwr_level = SYSADPT_TX_GRP_PWR_LEVEL_TOTAL;
	else
		priv->pwr_level = SYSADPT_TX_POWER_LEVEL_TOTAL;
	priv->dfs_test = false;
	priv->csa_active = false;
	priv->dfs_chirp_count_min = 5;
	priv->dfs_chirp_time_interval = 1000;
	priv->dfs_pw_filter = 0;
	priv->dfs_min_num_radar = 5;
	priv->dfs_min_pri_count = 4;
	priv->bf_type = TXBF_MODE_AUTO;

	/* Handle watchdog ba events */
	INIT_WORK(&priv->heartbeat_handle, mwl_heartbeat_handle);
	INIT_WORK(&priv->watchdog_ba_handle, mwl_watchdog_ba_events);
	INIT_WORK(&priv->account_handle, mwl_account_handle);
	INIT_WORK(&priv->wds_check_handle, mwl_wds_check_handle);
	INIT_WORK(&priv->chnl_switch_handle, mwl_chnl_switch_event);

	mutex_init(&priv->fwcmd_mutex);
	spin_lock_init(&priv->vif_lock);
	spin_lock_init(&priv->sta_lock);
	spin_lock_init(&priv->stream_lock);
	spin_lock_init(&priv->stnid_lock);

	rc = mwl_thermal_register(priv);
	if (rc) {
		wiphy_err(hw->wiphy, "fail to register thermal framework\n");
		goto err_thermal_register;
	}

	rc = mwl_hif_init(hw);
	if (rc) {
		wiphy_err(hw->wiphy, "fail to initialize host interface\n");
		goto err_hif_init;
	}

	SET_IEEE80211_PERM_ADDR(hw, priv->hw_data.mac_addr);

	if (priv->chip_type == MWL8964) {
		addr_num = SYSADPT_NUM_OF_AP + SYSADPT_NUM_OF_CLIENT;
		hw->wiphy->n_addresses = addr_num;
		hw->wiphy->addresses =
			kzalloc(addr_num * sizeof(*mac_addr), GFP_KERNEL);

		mac_addr = &hw->wiphy->addresses[0];
		ether_addr_copy(mac_addr->addr, priv->hw_data.mac_addr);
		last_nibble = mac_addr->addr[5] & 0x0F;
		for (addr_num = 0; addr_num < SYSADPT_NUM_OF_AP; addr_num++) {
			mac_addr = &hw->wiphy->addresses[addr_num + 1];
			ether_addr_copy(mac_addr->addr, priv->hw_data.mac_addr);
			if (!strcmp(wiphy_name(hw->wiphy), "phy0")) {
				last_nibble++;
				if (last_nibble == 0x10)
					last_nibble = 0;
			} else {
				last_nibble--;
				if (last_nibble == 0xFF)
					last_nibble = 0x0F;
			}
			mac_addr->addr[5] =
				(mac_addr->addr[5] & 0xF0) | last_nibble;
			mac_addr->addr[0] |= 0x2;
		}
	}

	wiphy_info(hw->wiphy,
		   "firmware version: 0x%x\n", priv->hw_data.fw_release_num);

	if (priv->chip_type == MWL8997) {
		mwl_fwcmd_set_cfg_data(hw, 2);
		mwl_fwcmd_set_txpwrlmt_cfg_data(hw);
		mwl_fwcmd_get_txpwrlmt_cfg_data(hw);
	}

	if (priv->chip_type == MWL8964)
		rc = mwl_fwcmd_get_fw_region_code_sc4(hw,
						      &priv->fw_region_code);
	else
		rc = mwl_fwcmd_get_fw_region_code(hw, &priv->fw_region_code);
	if (!rc) {
		priv->fw_device_pwrtbl = true;
		mwl_regd_init(priv);
		wiphy_info(hw->wiphy,
			   "firmware region code: %x\n", priv->fw_region_code);
	}

	if (priv->chip_type == MWL8997)
		mwl_fwcmd_dump_otp_data(hw);

	mwl_fwcmd_radio_disable(hw);
	mwl_fwcmd_rf_antenna(hw, WL_ANTENNATYPE_TX, priv->antenna_tx);
	mwl_fwcmd_rf_antenna(hw, WL_ANTENNATYPE_RX, priv->antenna_rx);

	hw->wiphy->interface_modes = 0;
	hw->wiphy->interface_modes |= BIT(NL80211_IFTYPE_AP);
#if defined(CPTCFG_MAC80211_MESH) || defined(CONFIG_MAC80211_MESH)
	hw->wiphy->interface_modes |= BIT(NL80211_IFTYPE_MESH_POINT);
#endif
	hw->wiphy->interface_modes |= BIT(NL80211_IFTYPE_STATION);
	hw->wiphy->iface_combinations = &ap_if_comb;
	hw->wiphy->n_iface_combinations = 1;

	if (priv->antenna_tx == ANTENNA_TX_1) {
		hw->wiphy->available_antennas_rx = 0x1;
		hw->wiphy->available_antennas_tx = 0x1;
	} else if (priv->antenna_tx == ANTENNA_TX_2) {
		hw->wiphy->available_antennas_rx = 0x2;
		hw->wiphy->available_antennas_tx = 0x2;
	} else{
		hw->wiphy->available_antennas_rx = 0x4;
		hw->wiphy->available_antennas_tx = 0x4;
	}

	mwl_set_caps(priv, false);

	priv->led_blink_enable = 1;
	priv->led_blink_rate = LED_BLINK_RATE_MID;
	mwl_fwcmd_led_ctrl(hw, priv->led_blink_enable, priv->led_blink_rate);

	vendor_cmd_register(hw->wiphy);

	rc = ieee80211_register_hw(hw);
	if (rc) {
		wiphy_err(hw->wiphy, "fail to register device\n");
		goto err_register_hw;
	}

	priv->irq = mwl_hif_get_irq_num(hw);
	rc = request_irq(priv->irq, mwl_isr, IRQF_SHARED,
			 mwl_hif_get_driver_name(hw), hw);
	if (rc) {
		priv->irq = -1;
		wiphy_err(hw->wiphy, "fail to register IRQ handler\n");
		goto err_register_irq;
	}

	setup_timer(&priv->period_timer, timer_routine, (unsigned long)hw);
	mod_timer(&priv->period_timer, jiffies +
		  msecs_to_jiffies(SYSADPT_TIMER_WAKEUP_TIME));

	return rc;

err_register_hw:
err_register_irq:
	mwl_hif_deinit(hw);

err_hif_init:
err_thermal_register:

	wiphy_err(hw->wiphy, "init fail\n");

	return rc;
}

static void mwl_wl_deinit(struct mwl_priv *priv)
{
	struct ieee80211_hw *hw = priv->hw;

	del_timer_sync(&priv->period_timer);

	if (priv->irq != -1) {
		free_irq(priv->irq, hw);
		priv->irq = -1;
	}

	if (priv->chip_type == MWL8964)
		kfree(hw->wiphy->addresses);
	ieee80211_unregister_hw(hw);
	mwl_thermal_unregister(priv);
	cancel_work_sync(&priv->chnl_switch_handle);
	cancel_work_sync(&priv->account_handle);
	cancel_work_sync(&priv->wds_check_handle);
	cancel_work_sync(&priv->watchdog_ba_handle);
	cancel_work_sync(&priv->heartbeat_handle);
	mwl_hif_deinit(hw);
}

struct ieee80211_hw *mwl_alloc_hw(int bus_type,
				  int chip_type,
				  struct device *dev,
				  const struct mwl_hif_ops *ops,
				  size_t hif_data_len)
{
	struct ieee80211_hw *hw;
	struct mwl_priv *priv;
	int priv_size;

	priv_size = ALIGN(sizeof(*priv), NETDEV_ALIGN) + hif_data_len;

	hw = ieee80211_alloc_hw(priv_size, &mwl_mac80211_ops);
	if (!hw) {
		pr_err("ieee80211 alloc hw failed\n");
		return NULL;
	}

	priv = hw->priv;
	priv->hw = hw;
	priv->dev = dev;
	priv->chip_type = chip_type;
	priv->fw_device_pwrtbl = false;
	priv->forbidden_setting = false;
	priv->regulatory_set = false;
	priv->use_short_slot = false;
	priv->use_short_preamble = false;
	priv->disable_2g = false;
	priv->disable_5g = false;
	priv->tx_amsdu = true;
	priv->hif.bus = bus_type;
	priv->hif.ops = ops;
	priv->hif.priv = (char *)priv + ALIGN(sizeof(*priv), NETDEV_ALIGN);
	priv->ampdu_num = mwl_hif_get_ampdu_num(hw);
	priv->ampdu =
		kzalloc(priv->ampdu_num * sizeof(*priv->ampdu), GFP_KERNEL);
	if (!priv->ampdu) {
		ieee80211_free_hw(hw);
		pr_err("alloc ampdu stream failed\n");
		return NULL;
	}

	if (chip_type == MWL8964)
		priv->stnid_num = SYSADPT_MAX_STA_SC4;
	else
		priv->stnid_num = SYSADPT_MAX_STA;
	priv->stnid =
		kzalloc(priv->stnid_num * sizeof(struct mwl_stnid), GFP_KERNEL);
	if (!priv->stnid) {
		kfree(priv->ampdu);
		ieee80211_free_hw(hw);
		pr_err("alloc stnid failed\n");
		return NULL;
	}
	priv->available_stnid = 0;

	SET_IEEE80211_DEV(hw, dev);

	return hw;
}

void mwl_free_hw(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	kfree(priv->stnid);
	kfree(priv->ampdu);
	ieee80211_free_hw(hw);
}

int mwl_init_hw(struct ieee80211_hw *hw, const char *fw_name,
		const char *cal_name, const char *txpwrlmt_name)
{
	struct mwl_priv *priv = hw->priv;
	int rc;
	int tx_num = 4, rx_num = 4;


	rc = mwl_prepare_cmd_buf(priv);
	if (rc) {
		wiphy_err(hw->wiphy, "fail to prepare command buffer\n");
		return -ENOMEM;
	}

	rc = mwl_init_firmware(priv, fw_name, cal_name, txpwrlmt_name);
	if (rc) {
		wiphy_err(hw->wiphy, "fail to initialize firmware\n");
		return -EIO;
	}

	/* firmware is loaded to H/W, it can be released now */
	release_firmware(priv->fw_ucode);

	mwl_process_of_dts(priv);

	rc = mwl_wl_init(priv);
	if (rc) {
		wiphy_err(hw->wiphy, "fail to initialize wireless lan\n");
		return -EIO;
	}

	wiphy_info(priv->hw->wiphy, "2G %s, 5G %s\n",
		   priv->disable_2g ? "disabled" : "enabled",
		   priv->disable_5g ? "disabled" : "enabled");

	if (priv->antenna_tx == ANTENNA_TX_2)
		tx_num = 2;
	else if (priv->antenna_tx == ANTENNA_TX_3)
		tx_num = 3;
	if (priv->antenna_rx == ANTENNA_RX_2)
		rx_num = 2;
	else if (priv->antenna_rx == ANTENNA_RX_3)
		rx_num = 3;
	wiphy_info(priv->hw->wiphy, "%d TX antennas, %d RX antennas\n",
		   tx_num, rx_num);

#ifdef CONFIG_DEBUG_FS
	mwl_debugfs_init(hw);
#endif

	return 0;
}

void mwl_deinit_hw(struct ieee80211_hw *hw)
{
#ifdef CONFIG_DEBUG_FS
	mwl_debugfs_remove(hw);
#endif

	mwl_wl_deinit(hw->priv);
}
