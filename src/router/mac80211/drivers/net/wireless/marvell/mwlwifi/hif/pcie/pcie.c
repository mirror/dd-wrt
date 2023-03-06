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

/* Description:  This file implements functions needed for PCIe module. */

#include <linux/module.h>
#include <linux/etherdevice.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"
#include "vendor_cmd.h"
#include "hif/fwcmd.h"
#include "hif/pcie/dev.h"
#include "hif/pcie/fwdl.h"
#include "hif/pcie/tx.h"
#include "hif/pcie/rx.h"
#include "hif/pcie/tx_ndp.h"
#include "hif/pcie/rx_ndp.h"

#define PCIE_DRV_DESC "Marvell Mac80211 Wireless PCIE Network Driver"
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,30)
#define PCIE_DEV_NAME "Marvell 802.11ac PCIE Adapter"
#endif

#define MAX_WAIT_FW_COMPLETE_ITERATIONS 2000
#define CHECK_BA_TRAFFIC_TIME           300 /* msec */
#define CHECK_TX_DONE_TIME              50  /* msec */

static struct pci_device_id pcie_id_tbl[] = {
	{ PCI_VDEVICE(MARVELL, 0x2a55),     .driver_data = MWL8864, },
	{ PCI_VDEVICE(MARVELL, 0x2b38),     .driver_data = MWL8897, },
	{ PCI_VDEVICE(MARVELL, 0x2b40),     .driver_data = MWL8964, },
	{ PCI_VDEVICE(MARVELL_EXT, 0x2b42), .driver_data = MWL8997, },
	{ },
};

static struct mwl_chip_info pcie_chip_tbl[] = {
	[MWL8864] = {
		.part_name	= "88W8864",
		.fw_image	= "mwlwifi/88W8864.bin",
		.cal_file	= NULL,
		.txpwrlmt_file  = NULL,
		.antenna_tx	= ANTENNA_TX_4_AUTO,
		.antenna_rx	= ANTENNA_RX_4_AUTO,
	},
	[MWL8897] = {
		.part_name	= "88W8897",
		.fw_image	= "mwlwifi/88W8897.bin",
		.cal_file	= NULL,
		.txpwrlmt_file  = NULL,
		.antenna_tx	= ANTENNA_TX_2,
		.antenna_rx	= ANTENNA_RX_2,
	},
	[MWL8964] = {
		.part_name	= "88W8964",
		.fw_image	= "mwlwifi/88W8964.bin",
		.cal_file	= NULL,
		.txpwrlmt_file  = NULL,
		.antenna_tx	= ANTENNA_TX_4_AUTO,
		.antenna_rx	= ANTENNA_RX_4_AUTO,
	},
	[MWL8997] = {
		.part_name	= "88W8997",
		.fw_image	= "mwlwifi/88W8997.bin",
		.cal_file	= "mwlwifi/WlanCalData_ext.conf",
		.txpwrlmt_file  = "mwlwifi/txpwrlmt_cfg.conf",
		.antenna_tx	= ANTENNA_TX_2,
		.antenna_rx	= ANTENNA_RX_2,
	},
};

static int pcie_alloc_resource(struct pcie_priv *pcie_priv)
{
	struct pci_dev *pdev = pcie_priv->pdev;
	struct device *dev = &pdev->dev;
	void __iomem *addr;

	pcie_priv->next_bar_num = 1;	/* 32-bit */
	if (pci_resource_flags(pdev, 0) & 0x04)
		pcie_priv->next_bar_num = 2;	/* 64-bit */

	addr = devm_ioremap_resource(dev, &pdev->resource[0]);
	if (IS_ERR(addr)) {
		pr_err("%s: cannot reserve PCI memory region 0\n",
		       PCIE_DRV_NAME);
		goto err;
	}
	pcie_priv->iobase0 = addr;
	pr_debug("iobase0 = %p\n", pcie_priv->iobase0);

	addr = devm_ioremap_resource(dev,
				     &pdev->resource[pcie_priv->next_bar_num]);
	if (IS_ERR(addr)) {
		pr_err("%s: cannot reserve PCI memory region 1\n",
		       PCIE_DRV_NAME);
		goto err;
	}
	pcie_priv->iobase1 = addr;
	pr_debug("iobase1 = %p\n", pcie_priv->iobase1);

	return 0;

err:
	pr_err("pci alloc fail\n");

	return -EIO;
}

static u32 pcie_read_mac_reg(struct pcie_priv *pcie_priv, u32 offset)
{
	struct mwl_priv *priv = pcie_priv->mwl_priv;

	if (priv->chip_type == MWL8964) {
		u32 *addr_val = kmalloc(64 * sizeof(u32), GFP_ATOMIC);
		u32 val;

		if (addr_val) {
			mwl_fwcmd_get_addr_value(priv->hw,
						 0x8000a000 + offset, 4,
						 addr_val, 0);
			val = addr_val[0];
			kfree(addr_val);
			return val;
		}
		return 0;
	} else
		return le32_to_cpu(*(__le32 *)
		       (MAC_REG_ADDR_PCI(offset)));
}

static bool pcie_chk_adapter(struct pcie_priv *pcie_priv)
{
	struct mwl_priv *priv = pcie_priv->mwl_priv;
	u32 regval;

	regval = readl(pcie_priv->iobase1 + MACREG_REG_INT_CODE);

	if (regval == 0xffffffff) {
		wiphy_err(priv->hw->wiphy, "adapter does not exist\n");
		return false;
	}

	if (priv->cmd_timeout)
		wiphy_debug(priv->hw->wiphy, "MACREG_REG_INT_CODE: 0x%04x\n",
			    regval);

	return true;
}

static void pcie_send_cmd(struct pcie_priv *pcie_priv)
{
	writel(pcie_priv->mwl_priv->pphys_cmd_buf,
	       pcie_priv->iobase1 + MACREG_REG_GEN_PTR);
	writel(MACREG_H2ARIC_BIT_DOOR_BELL,
	       pcie_priv->iobase1 + MACREG_REG_H2A_INTERRUPT_EVENTS);
}

static int pcie_wait_complete(struct mwl_priv *priv, unsigned short cmd)
{
	unsigned int curr_iteration = MAX_WAIT_FW_COMPLETE_ITERATIONS;
	unsigned short int_code = 0;

	do {
		int_code = le16_to_cpu(*((__le16 *)&priv->pcmd_buf[0]));
		usleep_range(1000, 2000);
	} while ((int_code != cmd) && (--curr_iteration) && !priv->rmmod);

	if (curr_iteration == 0) {
		wiphy_err(priv->hw->wiphy, "cmd 0x%04x=%s timed out\n",
			  cmd, mwl_fwcmd_get_cmd_string(cmd));
		wiphy_err(priv->hw->wiphy, "return code: 0x%04x\n", int_code);
		return -EIO;
	}

	if (priv->chip_type != MWL8997)
		usleep_range(3000, 5000);

	return 0;
}

static int pcie_init(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	const struct hostcmd_get_hw_spec *get_hw_spec;
	struct hostcmd_set_hw_spec set_hw_spec;
	int rc, i;

	spin_lock_init(&pcie_priv->int_mask_lock);
	tasklet_init(&pcie_priv->tx_task,
		     (void *)pcie_tx_skbs, (unsigned long)hw);
	tasklet_disable(&pcie_priv->tx_task);
	tasklet_init(&pcie_priv->tx_done_task,
		     (void *)pcie_tx_done, (unsigned long)hw);
	tasklet_disable(&pcie_priv->tx_done_task);
	spin_lock_init(&pcie_priv->tx_desc_lock);
	tasklet_init(&pcie_priv->rx_task,
		     (void *)pcie_rx_recv, (unsigned long)hw);
	tasklet_disable(&pcie_priv->rx_task);
	tasklet_init(&pcie_priv->qe_task,
		     (void *)pcie_tx_flush_amsdu, (unsigned long)hw);
	tasklet_disable(&pcie_priv->qe_task);
	pcie_priv->txq_limit = PCIE_TX_QUEUE_LIMIT;
	pcie_priv->txq_wake_threshold = PCIE_TX_WAKE_Q_THRESHOLD;
	pcie_priv->is_tx_done_schedule = false;
	pcie_priv->recv_limit = PCIE_RECEIVE_LIMIT;
	pcie_priv->is_rx_schedule = false;
	pcie_priv->is_qe_schedule = false;
	pcie_priv->qe_trig_num = 0;
	pcie_priv->qe_trig_time = jiffies;

	rc = pcie_tx_init(hw);
	if (rc) {
		wiphy_err(hw->wiphy, "%s: fail to initialize TX\n",
			  PCIE_DRV_NAME);
		goto err_mwl_tx_init;
	}

	rc = pcie_rx_init(hw);
	if (rc) {
		wiphy_err(hw->wiphy, "%s: fail to initialize RX\n",
			  PCIE_DRV_NAME);
		goto err_mwl_rx_init;
	}

	/* get and prepare HW specifications */
	get_hw_spec = mwl_fwcmd_get_hw_specs(hw);
	if (!get_hw_spec) {
		wiphy_err(hw->wiphy, "%s: fail to get HW specifications\n",
			  PCIE_DRV_NAME);
		goto err_get_hw_specs;
	}
	ether_addr_copy(&priv->hw_data.mac_addr[0],
			get_hw_spec->permanent_addr);
	pcie_priv->desc_data[0].wcb_base =
		le32_to_cpu(get_hw_spec->wcb_base0) & 0x0000ffff;
	for (i = 1; i < SYSADPT_TOTAL_TX_QUEUES; i++)
		pcie_priv->desc_data[i].wcb_base =
			le32_to_cpu(get_hw_spec->wcb_base[i - 1]) & 0x0000ffff;
	pcie_priv->desc_data[0].rx_desc_read =
		le32_to_cpu(get_hw_spec->rxpd_rd_ptr) & 0x0000ffff;
	pcie_priv->desc_data[0].rx_desc_write =
		le32_to_cpu(get_hw_spec->rxpd_wr_ptr) & 0x0000ffff;
	priv->hw_data.fw_release_num = le32_to_cpu(get_hw_spec->fw_release_num);
	priv->hw_data.hw_version = get_hw_spec->version;
	if (priv->chip_type != MWL8997) {
		writel(pcie_priv->desc_data[0].pphys_tx_ring,
		       pcie_priv->iobase0 + pcie_priv->desc_data[0].wcb_base);
		for (i = 1; i < SYSADPT_TOTAL_TX_QUEUES; i++)
			writel(pcie_priv->desc_data[i].pphys_tx_ring,
			       pcie_priv->iobase0 +
			       pcie_priv->desc_data[i].wcb_base);
	}
	writel(pcie_priv->desc_data[0].pphys_rx_ring,
	       pcie_priv->iobase0 + pcie_priv->desc_data[0].rx_desc_read);
	writel(pcie_priv->desc_data[0].pphys_rx_ring,
	       pcie_priv->iobase0 + pcie_priv->desc_data[0].rx_desc_write);

	/* prepare and set HW specifications */
	memset(&set_hw_spec, 0, sizeof(set_hw_spec));
	if (priv->chip_type == MWL8997) {
		set_hw_spec.wcb_base[0] =
			cpu_to_le32(pcie_priv->txbd_ring_pbase);
		set_hw_spec.tx_wcb_num_per_queue =
			cpu_to_le32(PCIE_MAX_TXRX_BD);
		set_hw_spec.num_tx_queues = cpu_to_le32(1);
		set_hw_spec.features |= HW_SET_PARMS_FEATURES_HOST_PROBE_RESP;
	} else {
		set_hw_spec.wcb_base[0] =
			cpu_to_le32(pcie_priv->desc_data[0].pphys_tx_ring);
		for (i = 1; i < SYSADPT_TOTAL_TX_QUEUES; i++)
			set_hw_spec.wcb_base[i] = cpu_to_le32(
				pcie_priv->desc_data[i].pphys_tx_ring);
		set_hw_spec.tx_wcb_num_per_queue =
			cpu_to_le32(PCIE_MAX_NUM_TX_DESC);
		set_hw_spec.num_tx_queues = cpu_to_le32(PCIE_NUM_OF_DESC_DATA);
	}
	set_hw_spec.total_rx_wcb = cpu_to_le32(PCIE_MAX_NUM_RX_DESC);
	set_hw_spec.rxpd_wr_ptr =
		cpu_to_le32(pcie_priv->desc_data[0].pphys_rx_ring);
	rc = mwl_fwcmd_set_hw_specs(hw, &set_hw_spec);
	if (rc) {
		wiphy_err(hw->wiphy, "%s: fail to set HW specifications\n",
			  PCIE_DRV_NAME);
		goto err_set_hw_specs;
	}

	return rc;

err_set_hw_specs:
err_get_hw_specs:

	pcie_rx_deinit(hw);

err_mwl_rx_init:

	pcie_tx_deinit(hw);

err_mwl_tx_init:

	wiphy_err(hw->wiphy, "%s: init fail\n", PCIE_DRV_NAME);

	return rc;
}

static void pcie_deinit(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	pcie_rx_deinit(hw);
	pcie_tx_deinit(hw);
	tasklet_kill(&pcie_priv->qe_task);
	tasklet_kill(&pcie_priv->rx_task);
	tasklet_kill(&pcie_priv->tx_done_task);
	tasklet_kill(&pcie_priv->tx_task);
	pcie_reset(hw);
}

static int pcie_get_info(struct ieee80211_hw *hw, char *buf, size_t size)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	char *p = buf;
	int len = 0;

	len += scnprintf(p + len, size - len, "iobase0: %p\n",
			 pcie_priv->iobase0);
	len += scnprintf(p + len, size - len, "iobase1: %p\n",
			 pcie_priv->iobase1);
	len += scnprintf(p + len, size - len,
			 "tx limit: %d\n", pcie_priv->txq_limit);
	len += scnprintf(p + len, size - len,
			 "rx limit: %d\n", pcie_priv->recv_limit);
	len += scnprintf(p + len, size - len,
			 "qe trigger number: %d\n", pcie_priv->qe_trig_num);
	return len;
}

static void pcie_enable_data_tasks(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	tasklet_enable(&pcie_priv->tx_task);
	tasklet_enable(&pcie_priv->tx_done_task);
	tasklet_enable(&pcie_priv->rx_task);
	tasklet_enable(&pcie_priv->qe_task);
}

static void pcie_disable_data_tasks(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	tasklet_disable(&pcie_priv->tx_task);
	tasklet_disable(&pcie_priv->tx_done_task);
	tasklet_disable(&pcie_priv->rx_task);
	tasklet_disable(&pcie_priv->qe_task);
}

static int pcie_exec_cmd(struct ieee80211_hw *hw, unsigned short cmd)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	bool busy = false;

	might_sleep();

	if (!pcie_chk_adapter(pcie_priv)) {
		wiphy_err(priv->hw->wiphy, "adapter does not exist\n");
		priv->in_send_cmd = false;
		return -EIO;
	}

	if (!priv->in_send_cmd && !priv->rmmod) {
		priv->in_send_cmd = true;
		if (priv->dump_hostcmd)
			wiphy_debug(priv->hw->wiphy, "send cmd 0x%04x=%s\n",
				    cmd, mwl_fwcmd_get_cmd_string(cmd));
		pcie_send_cmd(pcie_priv);
		if (pcie_wait_complete(priv, 0x8000 | cmd)) {
			wiphy_err(priv->hw->wiphy, "timeout: 0x%04x\n", cmd);
			priv->in_send_cmd = false;
			priv->cmd_timeout = true;
			if (priv->heartbeat)
				vendor_cmd_basic_event(
					hw->wiphy,
					MWL_VENDOR_EVENT_CMD_TIMEOUT);
			return -EIO;
		}
	} else {
		wiphy_warn(priv->hw->wiphy,
			   "previous command is running or module removed\n");
		busy = true;
	}

	if (!busy)
		priv->in_send_cmd = false;

	return 0;
}

static int pcie_get_irq_num(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	return pcie_priv->pdev->irq;
}

static irqreturn_t pcie_isr(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	u32 int_status;

	int_status = readl(pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_CAUSE);

	if (int_status == 0x00000000)
		return IRQ_NONE;

	if (int_status == 0xffffffff) {
		wiphy_warn(hw->wiphy, "card unplugged?\n");
	} else {
		writel(~int_status,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_CAUSE);

		if (int_status & MACREG_A2HRIC_BIT_TX_DONE) {
			if (!pcie_priv->is_tx_done_schedule) {
				pcie_mask_int(pcie_priv,
					      MACREG_A2HRIC_BIT_TX_DONE, false);
				tasklet_schedule(&pcie_priv->tx_done_task);
				pcie_priv->is_tx_done_schedule = true;
			}
		}

		if (int_status & MACREG_A2HRIC_BIT_RX_RDY) {
			if (!pcie_priv->is_rx_schedule) {
				pcie_mask_int(pcie_priv,
					      MACREG_A2HRIC_BIT_RX_RDY, false);
				tasklet_schedule(&pcie_priv->rx_task);
				pcie_priv->is_rx_schedule = true;
			}
		}

		if (int_status & MACREG_A2HRIC_BIT_RADAR_DETECT) {
			wiphy_info(hw->wiphy, "radar detected by firmware\n");
			ieee80211_radar_detected(hw);
		}

		if (int_status & MACREG_A2HRIC_BIT_QUE_EMPTY) {
			if (!pcie_priv->is_qe_schedule) {
				if (time_after(jiffies,
					       (pcie_priv->qe_trig_time + 1))) {
					pcie_mask_int(pcie_priv,
					      MACREG_A2HRIC_BIT_QUE_EMPTY,
					      false);
					tasklet_schedule(&pcie_priv->qe_task);
					pcie_priv->qe_trig_num++;
					pcie_priv->is_qe_schedule = true;
					pcie_priv->qe_trig_time = jiffies;
				}
			}
		}

		if (int_status & MACREG_A2HRIC_BIT_CHAN_SWITCH)
			ieee80211_queue_work(hw, &priv->chnl_switch_handle);

		if (int_status & MACREG_A2HRIC_BA_WATCHDOG)
			ieee80211_queue_work(hw, &priv->watchdog_ba_handle);
	}

	return IRQ_HANDLED;
}

static void pcie_irq_enable(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	if (pcie_chk_adapter(pcie_priv)) {
		writel(0x00,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_MASK);
		writel(MACREG_A2HRIC_BIT_MASK,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_MASK);
	}
}

static void pcie_irq_disable(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	if (pcie_chk_adapter(pcie_priv))
		writel(0x00,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_MASK);
}

static void pcie_timer_routine(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	static int cnt;
	struct mwl_ampdu_stream *stream;
	struct mwl_sta *sta_info;
	struct mwl_tx_info *tx_stats;
	struct mwl_ampdu_stream *rm_stream = NULL;
	u32 rm_pkts = 0;
	bool ba_full = true;
	int i;

	if ((++cnt * SYSADPT_TIMER_WAKEUP_TIME) < CHECK_BA_TRAFFIC_TIME)
		return;
	cnt = 0;
	spin_lock_bh(&priv->stream_lock);
	for (i = 0; i < priv->ampdu_num; i++) {
		stream = &priv->ampdu[i];

		if (stream->state == AMPDU_STREAM_ACTIVE) {
			sta_info = mwl_dev_get_sta(stream->sta);
			tx_stats = &sta_info->tx_stats[stream->tid];

			if ((jiffies - tx_stats->start_time > HZ) &&
			    (tx_stats->pkts < SYSADPT_AMPDU_PACKET_THRESHOLD)) {
				if (rm_pkts) {
					if (tx_stats->pkts < rm_pkts) {
						rm_stream = stream;
						rm_pkts = tx_stats->pkts;
					}
				} else {
					rm_stream = stream;
					rm_pkts = tx_stats->pkts;
				}
			}

			if (jiffies - tx_stats->start_time > HZ) {
				tx_stats->pkts = 0;
				tx_stats->start_time = jiffies;
			}
		} else
			ba_full = false;
	}
	if (ba_full && rm_stream) {
		ieee80211_stop_tx_ba_session(rm_stream->sta,
					     rm_stream->tid);
		//wiphy_debug(hw->wiphy, "Stop BA %pM\n", rm_stream->sta->addr);
	}
	spin_unlock_bh(&priv->stream_lock);
}

static void pcie_tx_return_pkts(struct ieee80211_hw *hw)
{
	pcie_tx_done((unsigned long)hw);
}

static struct device_node *pcie_get_device_node(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct device_node *dev_node;

	dev_node = pci_bus_to_OF_node(pcie_priv->pdev->bus);
	wiphy_info(priv->hw->wiphy, "device node: %s\n", dev_node->full_name);

	return dev_node;
}

static void pcie_get_survey(struct ieee80211_hw *hw,
			    struct mwl_survey_info *survey_info)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	survey_info->filled = SURVEY_INFO_TIME |
			      SURVEY_INFO_TIME_BUSY |
			      SURVEY_INFO_TIME_TX |
			      SURVEY_INFO_TIME_RX |
			      SURVEY_INFO_NOISE_DBM;
	survey_info->time_period += pcie_read_mac_reg(pcie_priv, MCU_LAST_READ);
	survey_info->time_busy += pcie_read_mac_reg(pcie_priv, MCU_CCA_CNT);
	survey_info->time_tx += pcie_read_mac_reg(pcie_priv, MCU_TXPE_CNT);
	survey_info->time_rx += pcie_read_mac_reg(pcie_priv, MCU_RXPE_CNT);
	survey_info->noise = priv->noise;
}

static int pcie_reg_access(struct ieee80211_hw *hw, bool write)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	u8 set;
	u32 *addr_val;
	int ret = 0;

	set = write ? WL_SET : WL_GET;

	switch (priv->reg_type) {
	case MWL_ACCESS_RF:
		ret = mwl_fwcmd_reg_rf(hw, set, priv->reg_offset,
				       &priv->reg_value);
		break;
	case MWL_ACCESS_BBP:
		ret = mwl_fwcmd_reg_bb(hw, set, priv->reg_offset,
				       &priv->reg_value);
		break;
	case MWL_ACCESS_CAU:
		ret = mwl_fwcmd_reg_cau(hw, set, priv->reg_offset,
					&priv->reg_value);
		break;
	case MWL_ACCESS_ADDR0:
		if (set == WL_GET)
			priv->reg_value =
				readl(pcie_priv->iobase0 + priv->reg_offset);
		else
			writel(priv->reg_value,
			       pcie_priv->iobase0 + priv->reg_offset);
		break;
	case MWL_ACCESS_ADDR1:
		if (set == WL_GET)
			priv->reg_value =
				readl(pcie_priv->iobase1 + priv->reg_offset);
		else
			writel(priv->reg_value,
			       pcie_priv->iobase1 + priv->reg_offset);
		break;
	case MWL_ACCESS_ADDR:
		addr_val = kzalloc(64 * sizeof(u32), GFP_KERNEL);
		if (addr_val) {
			addr_val[0] = priv->reg_value;
			ret = mwl_fwcmd_get_addr_value(hw, priv->reg_offset,
						       4, addr_val, set);
			if ((!ret) && (set == WL_GET))
				priv->reg_value = addr_val[0];
			kfree(addr_val);
		} else {
			ret = -ENOMEM;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static struct mwl_hif_ops pcie_hif_ops = {
	.driver_name           = PCIE_DRV_NAME,
	.driver_version        = PCIE_DRV_VERSION,
	.tx_head_room          = PCIE_MIN_BYTES_HEADROOM,
	.ampdu_num             = PCIE_AMPDU_QUEUES,
	.reset                 = pcie_reset,
	.init                  = pcie_init,
	.deinit                = pcie_deinit,
	.get_info              = pcie_get_info,
	.enable_data_tasks     = pcie_enable_data_tasks,
	.disable_data_tasks    = pcie_disable_data_tasks,
	.exec_cmd              = pcie_exec_cmd,
	.get_irq_num           = pcie_get_irq_num,
	.irq_handler           = pcie_isr,
	.irq_enable            = pcie_irq_enable,
	.irq_disable           = pcie_irq_disable,
	.download_firmware     = pcie_download_firmware,
	.timer_routine         = pcie_timer_routine,
	.tx_xmit               = pcie_tx_xmit,
	.tx_del_pkts_via_vif   = pcie_tx_del_pkts_via_vif,
	.tx_del_pkts_via_sta   = pcie_tx_del_pkts_via_sta,
	.tx_del_ampdu_pkts     = pcie_tx_del_ampdu_pkts,
	.tx_del_sta_amsdu_pkts = pcie_tx_del_sta_amsdu_pkts,
	.tx_return_pkts        = pcie_tx_return_pkts,
	.get_device_node       = pcie_get_device_node,
	.get_survey            = pcie_get_survey,
	.reg_access            = pcie_reg_access,
};

static int pcie_init_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	const struct hostcmd_get_hw_spec *get_hw_spec;
	struct hostcmd_set_hw_spec set_hw_spec;
	int rc;

	spin_lock_init(&pcie_priv->int_mask_lock);
	tasklet_init(&pcie_priv->tx_task,
		     (void *)pcie_tx_skbs_ndp, (unsigned long)hw);
	tasklet_disable(&pcie_priv->tx_task);
	spin_lock_init(&pcie_priv->tx_desc_lock);
	tasklet_init(&pcie_priv->rx_task,
		     (void *)pcie_rx_recv_ndp, (unsigned long)hw);
	tasklet_disable(&pcie_priv->rx_task);
	pcie_priv->txq_limit = TX_QUEUE_LIMIT;
	pcie_priv->txq_wake_threshold = TX_WAKE_Q_THRESHOLD;
	pcie_priv->is_tx_schedule = false;
	pcie_priv->recv_limit = MAX_NUM_RX_DESC;
	pcie_priv->is_rx_schedule = false;

	rc = pcie_tx_init_ndp(hw);
	if (rc) {
		wiphy_err(hw->wiphy, "%s: fail to initialize TX\n",
			  PCIE_DRV_NAME);
		goto err_mwl_tx_init;
	}

	rc = pcie_rx_init_ndp(hw);
	if (rc) {
		wiphy_err(hw->wiphy, "%s: fail to initialize RX\n",
			  PCIE_DRV_NAME);
		goto err_mwl_rx_init;
	}

	/* get and prepare HW specifications */
	get_hw_spec = mwl_fwcmd_get_hw_specs(hw);
	if (!get_hw_spec) {
		wiphy_err(hw->wiphy, "%s: fail to get HW specifications\n",
			  PCIE_DRV_NAME);
		goto err_get_hw_specs;
	}
	ether_addr_copy(&priv->hw_data.mac_addr[0],
			get_hw_spec->permanent_addr);
	priv->hw_data.fw_release_num = le32_to_cpu(get_hw_spec->fw_release_num);
	priv->hw_data.hw_version = get_hw_spec->version;

	/* prepare and set HW specifications */
	memset(&set_hw_spec, 0, sizeof(set_hw_spec));
	set_hw_spec.wcb_base[0] =
		cpu_to_le32(pcie_priv->desc_data_ndp.pphys_tx_ring);
	set_hw_spec.wcb_base[1] =
		cpu_to_le32(pcie_priv->desc_data_ndp.pphys_tx_ring_done);
	set_hw_spec.wcb_base[2] =
		cpu_to_le32(pcie_priv->desc_data_ndp.pphys_rx_ring);
	set_hw_spec.wcb_base[3] =
		cpu_to_le32(pcie_priv->desc_data_ndp.pphys_rx_ring_done);
	set_hw_spec.acnt_base_addr =
		cpu_to_le32(pcie_priv->desc_data_ndp.pphys_acnt_ring);
	set_hw_spec.acnt_buf_size =
		cpu_to_le32(pcie_priv->desc_data_ndp.acnt_ring_size);
	rc = mwl_fwcmd_set_hw_specs(hw, &set_hw_spec);
	if (rc) {
		wiphy_err(hw->wiphy, "%s: fail to set HW specifications\n",
			  PCIE_DRV_NAME);
		goto err_set_hw_specs;
	}

	return rc;

err_set_hw_specs:
err_get_hw_specs:

	pcie_rx_deinit_ndp(hw);

err_mwl_rx_init:

	pcie_tx_deinit_ndp(hw);

err_mwl_tx_init:

	wiphy_err(hw->wiphy, "%s: init fail\n", PCIE_DRV_NAME);

	return rc;
}

static void pcie_deinit_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	pcie_rx_deinit_ndp(hw);
	pcie_tx_deinit_ndp(hw);
	tasklet_kill(&pcie_priv->rx_task);
	tasklet_kill(&pcie_priv->tx_task);
	pcie_reset(hw);
}

static int pcie_get_info_ndp(struct ieee80211_hw *hw, char *buf, size_t size)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	char *p = buf;
	int len = 0;

	len += scnprintf(p + len, size - len, "iobase0: %p\n",
			 pcie_priv->iobase0);
	len += scnprintf(p + len, size - len, "iobase1: %p\n",
			 pcie_priv->iobase1);
	len += scnprintf(p + len, size - len,
			 "tx limit: %d\n", pcie_priv->txq_limit);
	len += scnprintf(p + len, size - len,
			 "rx limit: %d\n", pcie_priv->recv_limit);
	return len;
}

static int pcie_get_tx_status_ndp(struct ieee80211_hw *hw, char *buf,
				  size_t size)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	char *p = buf;
	int len = 0;

	len += scnprintf(p + len, size - len, "tx_done_cnt: %d\n",
			 pcie_priv->tx_done_cnt);
	len += scnprintf(p + len, size - len, "tx_desc_busy_cnt: %d\n",
			 pcie_priv->desc_data_ndp.tx_desc_busy_cnt);
	len += scnprintf(p + len, size - len, "tx_sent_head: %d\n",
			 pcie_priv->desc_data_ndp.tx_sent_head);
	len += scnprintf(p + len, size - len, "tx_sent_tail: %d\n",
			 pcie_priv->desc_data_ndp.tx_sent_tail);
	len += scnprintf(p + len, size - len, "tx_done_head: %d\n",
			 readl(pcie_priv->iobase1 + MACREG_REG_TXDONEHEAD));
	len += scnprintf(p + len, size - len, "tx_done_tail: %d\n",
			 pcie_priv->desc_data_ndp.tx_done_tail);
	len += scnprintf(p + len, size - len, "tx_vbuflist_idx: %d\n",
			 pcie_priv->desc_data_ndp.tx_vbuflist_idx);
	return len;
}

static int pcie_get_rx_status_ndp(struct ieee80211_hw *hw, char *buf,
				  size_t size)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	char *p = buf;
	int len = 0;

	len += scnprintf(p + len, size - len, "rx_done_head: %d\n",
			 readl(pcie_priv->iobase1 + MACREG_REG_RXDONEHEAD));
	len += scnprintf(p + len, size - len, "rx_done_tail: %d\n",
			 readl(pcie_priv->iobase1 + MACREG_REG_RXDONETAIL));
	len += scnprintf(p + len, size - len, "rx_desc_head: %d\n",
			 readl(pcie_priv->iobase1 + MACREG_REG_RXDESCHEAD));
	len += scnprintf(p + len, size - len, "rx_skb_trace: %d\n",
			 skb_queue_len(&pcie_priv->rx_skb_trace));
	len += scnprintf(p + len, size - len, "rx_skb_unlink_err: %d\n",
			 pcie_priv->rx_skb_unlink_err);
	len += scnprintf(p + len, size - len, "signature_err: %d\n",
			 pcie_priv->signature_err);
	len += scnprintf(p + len, size - len, "recheck_rxringdone: %d\n",
			 pcie_priv->recheck_rxringdone);
	len += scnprintf(p + len, size - len, "fast_data_cnt: %d\n",
			 pcie_priv->rx_cnts.fast_data_cnt);
	len += scnprintf(p + len, size - len, "fast_bad_amsdu_cnt: %d\n",
			 pcie_priv->rx_cnts.fast_bad_amsdu_cnt);
	len += scnprintf(p + len, size - len, "slow_noqueue_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_noqueue_cnt);
	len += scnprintf(p + len, size - len, "slow_norun_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_norun_cnt);
	len += scnprintf(p + len, size - len, "slow_mcast_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_mcast_cnt);
	len += scnprintf(p + len, size - len, "slow_bad_sta_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_bad_sta_cnt);
	len += scnprintf(p + len, size - len, "slow_bad_mic_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_bad_mic_cnt);
	len += scnprintf(p + len, size - len, "slow_bad_pn_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_bad_pn_cnt);
	len += scnprintf(p + len, size - len, "slow_mgmt_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_mgmt_cnt);
	len += scnprintf(p + len, size - len, "slow_promisc_cnt: %d\n",
			 pcie_priv->rx_cnts.slow_promisc_cnt);
	len += scnprintf(p + len, size - len, "drop_cnt: %d\n",
			 pcie_priv->rx_cnts.drop_cnt);
	len += scnprintf(p + len, size - len, "offch_promisc_cnt: %d\n",
			 pcie_priv->rx_cnts.offch_promisc_cnt);
	len += scnprintf(p + len, size - len, "mu_pkt_cnt: %d\n",
			 pcie_priv->rx_cnts.mu_pkt_cnt);
	return len;
}

static void pcie_enable_data_tasks_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	tasklet_enable(&pcie_priv->tx_task);
	tasklet_enable(&pcie_priv->rx_task);
}

static void pcie_disable_data_tasks_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	tasklet_disable(&pcie_priv->tx_task);
	tasklet_disable(&pcie_priv->rx_task);
}

static irqreturn_t pcie_isr_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	u32 int_status;

	int_status = readl(pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_CAUSE);

	if (int_status == 0x00000000)
		return IRQ_NONE;

	if (int_status == 0xffffffff) {
		wiphy_warn(hw->wiphy, "card unplugged?\n");
	} else {
		writel(~int_status,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_CAUSE);

		if (int_status & MACREG_A2HRIC_ACNT_HEAD_RDY)
			ieee80211_queue_work(hw, &priv->account_handle);

		if (int_status & MACREG_A2HRIC_RX_DONE_HEAD_RDY) {
			if (!pcie_priv->is_rx_schedule) {
				pcie_mask_int(pcie_priv,
					      MACREG_A2HRIC_RX_DONE_HEAD_RDY,
					      false);
				tasklet_schedule(&pcie_priv->rx_task);
				pcie_priv->is_rx_schedule = true;
			}
		}

		if (int_status & MACREG_A2HRIC_NEWDP_DFS) {
			wiphy_info(hw->wiphy, "radar detected by firmware\n");
			ieee80211_radar_detected(hw);
		}

		if (int_status & MACREG_A2HRIC_NEWDP_CHANNEL_SWITCH)
			ieee80211_queue_work(hw, &priv->chnl_switch_handle);
	}

	return IRQ_HANDLED;
}

static void pcie_irq_enable_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;

	if (pcie_chk_adapter(pcie_priv)) {
		writel(0x00,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_MASK);
		writel(MACREG_A2HRIC_BIT_MASK_NDP,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_MASK);
	}
}

static void pcie_timer_routine_ndp(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	int num = SYSADPT_TX_WMM_QUEUES;
	static int cnt;

	if (!pcie_priv->is_tx_schedule) {
		while (num--) {
			if (skb_queue_len(&pcie_priv->txq[num]) > 0) {
				tasklet_schedule(&pcie_priv->tx_task);
				pcie_priv->is_tx_schedule = true;
				break;
			}
		}
	}

	if ((++cnt * SYSADPT_TIMER_WAKEUP_TIME) >= CHECK_TX_DONE_TIME) {
		pcie_tx_done_ndp(hw);
		cnt = 0;
	}
}

static void pcie_tx_return_pkts_ndp(struct ieee80211_hw *hw)
{
	pcie_tx_done_ndp(hw);
}

static void pcie_set_sta_id(struct ieee80211_hw *hw,
			    struct ieee80211_sta *sta,
			    bool sta_mode, bool set)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct mwl_sta *sta_info;
	u16 stnid;

	sta_info = mwl_dev_get_sta(sta);
	stnid = sta_mode ? 0 : sta_info->stnid;
	pcie_priv->sta_link[stnid] = set ? sta : NULL;
}

static void pcie_tx_account(struct mwl_priv *priv,
			    struct mwl_sta *sta_info,
			    struct acnt_tx_s *acnt_tx)
{
	u32 rate_info, tx_cnt;
	u8 index, type, rate_ac, format, bw, gi, mcs, nss;
	u16 ratemask;
	u8 i, found;
	struct mwl_tx_hist *tx_hist;
	struct mwl_tx_hist_data *tx_hist_data;

	rate_info = le32_to_cpu(acnt_tx->tx_info.rate_info);
	tx_cnt = le32_to_cpu(acnt_tx->tx_cnt);
	index = acnt_tx->rate_tbl_index;
	type = acnt_tx->type;

	if (!rate_info)
		return;
	sta_info->tx_rate_info = rate_info;

	tx_hist = &sta_info->tx_hist;
	if (!tx_hist || (type >= SU_MU_TYPE_CNT))
		return;

	format = rate_info & MWL_TX_RATE_FORMAT_MASK;
	bw = (rate_info & MWL_TX_RATE_BANDWIDTH_MASK) >>
		MWL_TX_RATE_BANDWIDTH_SHIFT;
	gi = (rate_info & MWL_TX_RATE_SHORTGI_MASK) >>
		MWL_TX_RATE_SHORTGI_SHIFT;
	mcs = (rate_info & MWL_TX_RATE_RATEIDMCS_MASK) >>
		MWL_TX_RATE_RATEIDMCS_SHIFT;

	tx_hist->cur_rate_info[type] = rate_info;

	/* Rate table index is valid */
	if (index != 0xff) {
		if (type == MU_MIMO) {
			rate_ac = mcs & 0xf;
			nss = mcs >> 4;
			if (nss < (QS_NUM_SUPPORTED_11AC_NSS - 1)) {
				tx_hist_data =
					&tx_hist->mu_rate[nss][bw][gi][rate_ac];
				tx_hist_data->rateinfo = rate_info;
				tx_hist_data->cnt++;
				tx_hist->total_tx_cnt[type] += tx_cnt;
			}
		} else {
			/* If legacy, skip legacy preamble bit 15 */
			if (format == TX_RATE_FORMAT_LEGACY)
				ratemask = 0xfff;
			else
				ratemask = 0xffff;
			tx_hist_data = &tx_hist->su_rate[0];
			if ((tx_hist_data[index].rateinfo & ratemask) ==
			    (rate_info & ratemask)) {
				tx_hist_data[index].cnt++;
				tx_hist->total_tx_cnt[type] += tx_cnt;
			}
		}
	} else {
		if (type == MU_MIMO) {
			rate_ac = mcs & 0xf;
			nss = mcs >> 4;
			if (nss < (QS_NUM_SUPPORTED_11AC_NSS - 1)) {
				tx_hist_data =
					&tx_hist->mu_rate[nss][bw][gi][rate_ac];
				tx_hist_data->rateinfo = rate_info;
				tx_hist_data->cnt++;
				tx_hist->total_tx_cnt[type] += tx_cnt;
			}
		} else {
			/* If legacy, skip legacy preamble bit 15 */
			if (format == TX_RATE_FORMAT_LEGACY)
				ratemask = 0xfff;
			else
				ratemask = 0xffff;
			tx_hist_data = &tx_hist->custom_rate[0];
			/* Go through non rate table buffer to see if any has
			 * been used. If all used up, recycle by using index 0
			 */
			for (i = 0; i < TX_RATE_HISTO_CUSTOM_CNT; i++) {
				if (!tx_hist_data[i].rateinfo ||
				    ((tx_hist_data[i].rateinfo & ratemask) ==
				    (rate_info & ratemask))) {
					found = 1;
					break;
				}
			}
			if (found)
				index = i;
			else
				index = 0; /* reuse index 0 buffer */
			tx_hist_data[index].rateinfo = rate_info;
			tx_hist_data[index].cnt++;
			tx_hist->total_tx_cnt[type] += tx_cnt;
		}
	}
}

static void pcie_rx_account(struct mwl_priv *priv,
			    struct mwl_sta *sta_info,
			    struct acnt_rx_s *acnt_rx)
{
	u32 sig1, sig2, rate, param;
	u16 format, nss, bw, gi, rate_mcs;

	sig1 = (le32_to_cpu(acnt_rx->rx_info.ht_sig1) >>
		RXINFO_HT_SIG1_SHIFT) & RXINFO_HT_SIG1_MASK;
	sig2 = (le32_to_cpu(acnt_rx->rx_info.ht_sig2_rate) >>
		RXINFO_HT_SIG2_SHIFT) & RXINFO_HT_SIG2_MASK;
	rate = (le32_to_cpu(acnt_rx->rx_info.ht_sig2_rate) >>
		RXINFO_RATE_SHIFT) & RXINFO_RATE_MASK;
	param = (le32_to_cpu(acnt_rx->rx_info.param) >>
		RXINFO_PARAM_SHIFT) & RXINFO_PARAM_MASK;

	format = (param >> 3) & 0x7;
	nss = 0;
	bw = RX_RATE_INFO_HT20;
	switch (format) {
	case RX_RATE_INFO_FORMAT_11A:
		rate_mcs = rate & 0xF;
		if (rate_mcs == 10)
			rate_mcs = 7; /* 12 Mbps */
		else
			rate_mcs = utils_get_rate_id(rate_mcs);
		gi = RX_RATE_INFO_SHORT_INTERVAL;
		if ((rate_mcs == 5) || (rate_mcs == 7) || (rate_mcs == 9))
			return;
		break;
	case RX_RATE_INFO_FORMAT_11B:
		rate_mcs = utils_get_rate_id(rate & 0xF);
		gi = RX_RATE_INFO_LONG_INTERVAL;
		if ((rate_mcs == 0) || (rate_mcs == 1))
			return;
		break;
	case RX_RATE_INFO_FORMAT_11N:
		if ((sig1 & 0x3f) >= 16)
			return;
		bw = (sig1 >> 7) & 0x1;
		gi = (sig2 >> 7) & 0x1;
		rate_mcs = sig1 & 0x3F;
		if (rate_mcs > 76)
			return;
		break;
	case RX_RATE_INFO_FORMAT_11AC:
		if (((sig2 >> 4) & 0xf) >= 10)
			return;
		nss = (sig1 >> 10) & 0x3;
		if (!nss)
			return;
		bw = sig1 & 0x3;
		gi = sig2 & 0x1;
		rate_mcs = (sig2 >> 4) & 0xF;
		if (rate_mcs > 9)
			return;
		break;
	default:
		return;
	}

	sta_info->rx_format = format;
	sta_info->rx_nss = nss;
	sta_info->rx_bw = bw;
	sta_info->rx_gi = gi;
	sta_info->rx_rate_mcs = rate_mcs;
	sta_info->rx_signal = ((le32_to_cpu(acnt_rx->rx_info.rssi_x) >>
		RXINFO_RSSI_X_SHIFT) & RXINFO_RSSI_X_MASK);
}

static void pcie_tx_per(struct mwl_priv *priv, struct mwl_sta *sta_info,
			struct acnt_ra_s *acnt_ra)
{
	u32 rate_info;
	u8 index, per, type, rate_ac, per_index, format, bw, gi, mcs, nss;
	u16 ratemask;
	u8 i, found;
	struct mwl_tx_hist *tx_hist;
	struct mwl_tx_hist_data *tx_hist_data;

	rate_info = le32_to_cpu(acnt_ra->rate_info);
	index = acnt_ra->rate_tbl_index;
	per = acnt_ra->per;
	type = acnt_ra->type;

	tx_hist = &sta_info->tx_hist;

	if (!tx_hist || !rate_info || (type >= SU_MU_TYPE_CNT))
		return;

	if ((type == SU_MIMO) && (index >= MAX_SUPPORTED_RATES) &&
	    (index != 0xFF))
		return;

	if (per >= TX_HISTO_PER_THRES[3])
		per_index = 4;
	else if (per >= TX_HISTO_PER_THRES[2])
		per_index = 3;
	else if (per >= TX_HISTO_PER_THRES[1])
		per_index = 2;
	else if (per >= TX_HISTO_PER_THRES[0])
		per_index = 1;
	else
		per_index = 0;

	format = rate_info & MWL_TX_RATE_FORMAT_MASK;
	bw = (rate_info & MWL_TX_RATE_BANDWIDTH_MASK) >>
		MWL_TX_RATE_BANDWIDTH_SHIFT;
	gi = (rate_info & MWL_TX_RATE_SHORTGI_MASK) >>
		MWL_TX_RATE_SHORTGI_SHIFT;
	mcs = (rate_info & MWL_TX_RATE_RATEIDMCS_MASK) >>
		MWL_TX_RATE_RATEIDMCS_SHIFT;

	/* Rate table index is valid */
	if (index != 0xff) {
		if (type == MU_MIMO) {
			rate_ac = mcs & 0xf;
			nss = mcs >> 4;
			if (nss < (QS_NUM_SUPPORTED_11AC_NSS - 1)) {
				tx_hist_data =
					&tx_hist->mu_rate[nss][bw][gi][rate_ac];
				tx_hist_data->rateinfo = rate_info;
				tx_hist_data->per[per_index]++;
			}
		} else {
			/* If legacy, skip legacy preamble bit 15 */
			if (format == TX_RATE_FORMAT_LEGACY)
				ratemask = 0xfff;
			else
				ratemask = 0xffff;
			tx_hist_data = &tx_hist->su_rate[0];
			if ((tx_hist_data[index].rateinfo & ratemask) ==
			    (rate_info & ratemask))
				tx_hist_data[index].per[per_index]++;
		}
	} else {
		if (type == MU_MIMO) {
			rate_ac = mcs & 0xf;
			nss = mcs >> 4;
			if (nss < (QS_NUM_SUPPORTED_11AC_NSS - 1)) {
				tx_hist_data =
					&tx_hist->mu_rate[nss][bw][gi][rate_ac];
				tx_hist_data->rateinfo = rate_info;
				tx_hist_data->per[per_index]++;
			}
		} else {
			/* If legacy, skip legacy preamble bit 15 */
			if (format == TX_RATE_FORMAT_LEGACY)
				ratemask = 0xfff;
			else
				ratemask = 0xffff;
			tx_hist_data = &tx_hist->custom_rate[0];
			/* Go through non rate table buffer to see if any has
			 * been used. If all used up, recycle by using index 0
			 */
			for (i = 0; i < TX_RATE_HISTO_CUSTOM_CNT; i++) {
				if (!tx_hist_data[i].rateinfo ||
				    ((tx_hist_data[i].rateinfo & ratemask) ==
				    (rate_info & ratemask))) {
					found = 1;
					break;
				}
			}
			if (found)
				index = i;
			else
				index = 0; /* reuse index 0 buffer */
			tx_hist_data[index].rateinfo = rate_info;
			tx_hist_data[index].per[per_index]++;
		}
	}
}

static void pcie_ba_account(struct mwl_priv *priv,
			    struct mwl_sta *sta_info,
			    struct acnt_ba_s *acnt_ba)
{
	struct mwl_tx_ba_hist *ba_hist = &sta_info->ba_hist;

	if (sta_info->stnid != le16_to_cpu(acnt_ba->stnid))
		return;

	if (ba_hist->enable && ba_hist->ba_stats &&
	    (ba_hist->index < ACNT_BA_SIZE)) {
		ba_hist->type = acnt_ba->type;
		ba_hist->ba_stats[ba_hist->index].ba_hole = acnt_ba->ba_hole;
		ba_hist->ba_stats[ba_hist->index].ba_expected =
			acnt_ba->ba_expected;
		ba_hist->ba_stats[ba_hist->index].no_ba = acnt_ba->no_ba;
		ba_hist->index++;
		if (ba_hist->index == ACNT_BA_SIZE)
			wiphy_info(priv->hw->wiphy,
				   "Aid:%d BA histo collection done\n",
				   priv->ba_aid);
	}
}

static void pcie_bf_mimo_ctrl_decode(struct mwl_priv *priv,
				     struct acnt_bf_mimo_ctrl_s *bf_mimo_ctrl)
{
	struct file *fp_data = NULL;
	const char filename[] = "/tmp/BF_MIMO_Ctrl_Field_Output.txt";
	char str_buf[256];
	char *buf = &str_buf[0];
	mm_segment_t oldfs;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
	oldfs = get_fs();
	set_fs( get_ds() );
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#else
	oldfs = force_uaccess_begin();
#endif

	buf += sprintf(buf, "\nMAC: %pM\n", bf_mimo_ctrl->rec_mac);
	buf += sprintf(buf, "SU_0_MU_1: %d\n", bf_mimo_ctrl->type);
	buf += sprintf(buf, "MIMO_Ctrl_Field: 0x%x\n",
		       le32_to_cpu(bf_mimo_ctrl->mimo_ctrl));
	buf += sprintf(buf, "Comp_BF_Report_First_8Bytes: 0x%llx\n",
		       le64_to_cpu(bf_mimo_ctrl->comp_bf_rep));

	fp_data = filp_open(filename, O_RDWR | O_CREAT | O_TRUNC, 0);

	if (!IS_ERR(fp_data)) {
		__kernel_write(fp_data, str_buf, strlen(str_buf),
			       &fp_data->f_pos);
		filp_close(fp_data, current->files);
	} else {
		wiphy_err(priv->hw->wiphy, "Error opening %s! %x\n",
			  filename, (unsigned int)fp_data);
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
	set_fs(oldfs);
#else
	force_uaccess_end(oldfs);
#endif
}

static void pcie_process_account(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	struct pcie_desc_data_ndp *desc = &pcie_priv->desc_data_ndp;
	u32 acnt_head, acnt_tail;
	u32 read_size;
	u8 *acnt_recds;
	u8 *pstart, *pend;
	struct acnt_s *acnt;
	struct acnt_tx_s *acnt_tx;
	struct acnt_rx_s *acnt_rx;
	struct acnt_ra_s *acnt_ra;
	struct acnt_ba_s *acnt_ba;
	struct acnt_bf_mimo_ctrl_s *acnt_bf_mimo_ctrl;
	struct pcie_dma_data *dma_data;
	struct mwl_sta *sta_info;
	u16 nf_a, nf_b, nf_c, nf_d;
	u16 stnid;
	u8 type;

	acnt_head = readl(pcie_priv->iobase1 + MACREG_REG_ACNTHEAD);
	acnt_tail = readl(pcie_priv->iobase1 + MACREG_REG_ACNTTAIL);

	if (acnt_tail == acnt_head)
		return;

	if (acnt_tail > acnt_head) {
		read_size = desc->acnt_ring_size - acnt_tail + acnt_head;
		if (read_size > desc->acnt_ring_size) {
			wiphy_err(hw->wiphy,
				  "account size overflow (%d %d %d)\n",
				  acnt_head, acnt_tail, read_size);
			goto process_next;
		}
		memset(desc->pacnt_buf, 0, desc->acnt_ring_size);
		memcpy(desc->pacnt_buf, desc->pacnt_ring + acnt_tail,
		       desc->acnt_ring_size - acnt_tail);
		memcpy(desc->pacnt_buf + desc->acnt_ring_size - acnt_tail,
		       desc->pacnt_ring, acnt_head);
		acnt_recds = desc->pacnt_buf;
	} else {
		read_size = acnt_head - acnt_tail;
		if (read_size > desc->acnt_ring_size) {
			wiphy_err(hw->wiphy,
				  "account size overflow (%d %d %d)\n",
				  acnt_head, acnt_tail, read_size);
			goto process_next;
		}
		acnt_recds = desc->pacnt_ring + acnt_tail;
	}

	pstart = acnt_recds;
	pend = pstart + read_size;
	while (pstart < pend) {
		acnt = (struct acnt_s *)pstart;

		switch (le16_to_cpu(acnt->code)) {
		case ACNT_CODE_BUSY:
			pcie_priv->acnt_busy++;
			break;
		case ACNT_CODE_WRAP:
			pcie_priv->acnt_wrap++;
			break;
		case ACNT_CODE_DROP:
			pcie_priv->acnt_drop++;
			break;
		case ACNT_CODE_TX_ENQUEUE:
			acnt_tx = (struct acnt_tx_s *)pstart;
			sta_info = utils_find_sta(priv, acnt_tx->hdr.wh.addr1);
			if (sta_info) {
				spin_lock_bh(&priv->sta_lock);
				pcie_tx_account(priv, sta_info, acnt_tx);
				spin_unlock_bh(&priv->sta_lock);
			}
			break;
		case ACNT_CODE_RX_PPDU:
			acnt_rx = (struct acnt_rx_s *)pstart;
			nf_a = (le32_to_cpu(acnt_rx->rx_info.nf_a_b) >>
				RXINFO_NF_A_SHIFT) & RXINFO_NF_A_MASK;
			nf_b = (le32_to_cpu(acnt_rx->rx_info.nf_a_b) >>
				RXINFO_NF_B_SHIFT) & RXINFO_NF_B_MASK;
			nf_c = (le32_to_cpu(acnt_rx->rx_info.nf_c_d) >>
				RXINFO_NF_C_SHIFT) & RXINFO_NF_C_MASK;
			nf_d = (le32_to_cpu(acnt_rx->rx_info.nf_c_d) >>
				RXINFO_NF_D_SHIFT) & RXINFO_NF_D_MASK;
			if ((nf_a >= 2048) && (nf_b >= 2048) &&
			    (nf_c >= 2048) && (nf_d >= 2048)) {
				nf_a = ((4096 - nf_a) >> 4);
				nf_b = ((4096 - nf_b) >> 4);
				nf_c = ((4096 - nf_c) >> 4);
				nf_d = ((4096 - nf_d) >> 4);
				priv->noise =
					-((nf_a + nf_b + nf_c + nf_d) / 4);
			}
			dma_data = (struct pcie_dma_data *)
				&acnt_rx->rx_info.hdr[0];
			sta_info = utils_find_sta(priv, dma_data->wh.addr2);
			if (sta_info) {
				spin_lock_bh(&priv->sta_lock);
				pcie_rx_account(priv, sta_info, acnt_rx);
				spin_unlock_bh(&priv->sta_lock);
			}
			break;
		case ACNT_CODE_RA_STATS:
			acnt_ra = (struct acnt_ra_s *)pstart;
			stnid = le16_to_cpu(acnt_ra->stn_id);
			if ((stnid > 0) && (stnid <= priv->stnid_num)) {
				type = acnt_ra->type;
				if (type < 2) {
					if (acnt_ra->tx_attempt_cnt >= 250)
						priv->ra_tx_attempt[type][5]++;
					else if (acnt_ra->tx_attempt_cnt >= 100)
						priv->ra_tx_attempt[type][4]++;
					else if (acnt_ra->tx_attempt_cnt >= 50)
						priv->ra_tx_attempt[type][3]++;
					else if (acnt_ra->tx_attempt_cnt >= 15)
						priv->ra_tx_attempt[type][2]++;
					else if (acnt_ra->tx_attempt_cnt >= 4)
						priv->ra_tx_attempt[type][1]++;
					else
						priv->ra_tx_attempt[type][0]++;
				}
				sta_info = utils_find_sta_by_id(priv, stnid);
				if (sta_info) {
					spin_lock_bh(&priv->sta_lock);
					pcie_tx_per(priv, sta_info, acnt_ra);
					spin_unlock_bh(&priv->sta_lock);
				}
			}
			break;
		case ACNT_CODE_BA_STATS:
			acnt_ba = (struct acnt_ba_s *)pstart;
			if (priv->ba_aid) {
				sta_info = utils_find_sta_by_aid(priv,
								 priv->ba_aid);
				if (sta_info) {
					spin_lock_bh(&priv->sta_lock);
					pcie_ba_account(priv, sta_info,
							acnt_ba);
					spin_unlock_bh(&priv->sta_lock);
				}
			}
			break;
		case ACNT_CODE_BF_MIMO_CTRL:
			acnt_bf_mimo_ctrl =
				(struct acnt_bf_mimo_ctrl_s *)pstart;
			pcie_bf_mimo_ctrl_decode(priv, acnt_bf_mimo_ctrl);
			break;
		default:
			break;
		}

		if (acnt->len)
			pstart += acnt->len * 4;
		else
			goto process_next;
	}
process_next:
	acnt_tail = acnt_head;
	writel(acnt_tail, pcie_priv->iobase1 + MACREG_REG_ACNTTAIL);
}

static int pcie_mcast_cts(struct ieee80211_hw *hw, bool enable)
{
	return mwl_fwcmd_mcast_cts(hw, enable ? 1 : 0);
}

static const struct mwl_hif_ops pcie_hif_ops_ndp = {
	.driver_name           = PCIE_DRV_NAME,
	.driver_version        = PCIE_DRV_VERSION,
	.tx_head_room          = PCIE_MIN_BYTES_HEADROOM,
	.ampdu_num             = AMPDU_QUEUES_NDP,
	.reset                 = pcie_reset,
	.init                  = pcie_init_ndp,
	.deinit                = pcie_deinit_ndp,
	.get_info              = pcie_get_info_ndp,
	.get_tx_status         = pcie_get_tx_status_ndp,
	.get_rx_status         = pcie_get_rx_status_ndp,
	.enable_data_tasks     = pcie_enable_data_tasks_ndp,
	.disable_data_tasks    = pcie_disable_data_tasks_ndp,
	.exec_cmd              = pcie_exec_cmd,
	.get_irq_num           = pcie_get_irq_num,
	.irq_handler           = pcie_isr_ndp,
	.irq_enable            = pcie_irq_enable_ndp,
	.irq_disable           = pcie_irq_disable,
	.download_firmware     = pcie_download_firmware,
	.timer_routine         = pcie_timer_routine_ndp,
	.tx_xmit               = pcie_tx_xmit_ndp,
	.tx_return_pkts        = pcie_tx_return_pkts_ndp,
	.get_device_node       = pcie_get_device_node,
	.get_survey            = pcie_get_survey,
	.reg_access            = pcie_reg_access,
	.set_sta_id            = pcie_set_sta_id,
	.process_account       = pcie_process_account,
	.mcast_cts             = pcie_mcast_cts,
};

static int pcie_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	static bool printed_version;
	struct ieee80211_hw *hw;
	struct mwl_priv *priv;
	struct pcie_priv *pcie_priv;
	const struct mwl_hif_ops *hif_ops;
	int rc = 0;

	if (id->driver_data >= MWLUNKNOWN)
		return -ENODEV;

	if (!printed_version) {
		pr_info("<<%s version %s>>\n",
			PCIE_DRV_DESC, PCIE_DRV_VERSION);
		printed_version = true;
	}

	rc = pci_enable_device(pdev);
	if (rc) {
		pr_err("%s: cannot enable new PCI device\n",
		       PCIE_DRV_NAME);
		return rc;
	}

	rc = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if (rc) {
		pr_err("%s: 32-bit PCI DMA not supported\n",
		       PCIE_DRV_NAME);
		goto err_pci_disable_device;
	}

	pci_set_master(pdev);

	if (id->driver_data == MWL8964)
		hif_ops = &pcie_hif_ops_ndp;
	else
		hif_ops = &pcie_hif_ops;
	hw = mwl_alloc_hw(MWL_BUS_PCIE, id->driver_data, &pdev->dev,
			  hif_ops, sizeof(*pcie_priv));
	if (!hw) {
		pr_err("%s: mwlwifi hw alloc failed\n",
		       PCIE_DRV_NAME);
		rc = -ENOMEM;
		goto err_pci_disable_device;
	}

	pci_set_drvdata(pdev, hw);

	priv = hw->priv;
	priv->antenna_tx = pcie_chip_tbl[priv->chip_type].antenna_tx;
	priv->antenna_rx = pcie_chip_tbl[priv->chip_type].antenna_rx;
	pcie_priv = priv->hif.priv;
	pcie_priv->mwl_priv = priv;
	pcie_priv->pdev = pdev;
	if (id->driver_data != MWL8964) {
		pcie_priv->tx_head_room = PCIE_MIN_BYTES_HEADROOM;
		if (id->driver_data == MWL8997) {
			if (NET_SKB_PAD < PCIE_MIN_TX_HEADROOM_KF2) {
				pcie_priv->tx_head_room =
					PCIE_MIN_TX_HEADROOM_KF2;
				pcie_hif_ops.tx_head_room =
					PCIE_MIN_TX_HEADROOM_KF2;
			}
		}
	}

	rc = pcie_alloc_resource(pcie_priv);
	if (rc)
		goto err_alloc_pci_resource;

	rc = mwl_init_hw(hw, pcie_chip_tbl[priv->chip_type].fw_image,
			 pcie_chip_tbl[priv->chip_type].cal_file,
			 pcie_chip_tbl[priv->chip_type].txpwrlmt_file);
	if (rc)
		goto err_wl_init;

	vendor_cmd_basic_event(hw->wiphy, MWL_VENDOR_EVENT_DRIVER_READY);

	return rc;

err_wl_init:

	pcie_reset(hw);

err_alloc_pci_resource:

	pci_set_drvdata(pdev, NULL);
	mwl_free_hw(hw);

err_pci_disable_device:

	pci_disable_device(pdev);

	return rc;
}

static void pcie_remove(struct pci_dev *pdev)
{
	struct ieee80211_hw *hw = pci_get_drvdata(pdev);
	struct mwl_priv *priv = hw->priv;

	priv->rmmod = true;
	while (priv->in_send_cmd)
		usleep_range(1000, 2000);
	vendor_cmd_basic_event(hw->wiphy, MWL_VENDOR_EVENT_DRIVER_START_REMOVE);
	mwl_deinit_hw(hw);
	pci_set_drvdata(pdev, NULL);
	mwl_free_hw(hw);
	pci_disable_device(pdev);
}

static struct pci_driver mwl_pcie_driver = {
	.name     = PCIE_DRV_NAME,
	.id_table = pcie_id_tbl,
	.probe    = pcie_probe,
	.remove   = pcie_remove
};

module_pci_driver(mwl_pcie_driver);

MODULE_DESCRIPTION(PCIE_DRV_DESC);
MODULE_VERSION(PCIE_DRV_VERSION);
MODULE_AUTHOR("Marvell Semiconductor, Inc.");
MODULE_LICENSE("GPL v2");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,15,30)
MODULE_SUPPORTED_DEVICE(PCIE_DEV_NAME);
#endif
MODULE_DEVICE_TABLE(pci, pcie_id_tbl);
