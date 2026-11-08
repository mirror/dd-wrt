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

/* Description:  This file implements firmware download related
 * functions.
 */

#include <linux/io.h>
#include <linux/iopoll.h>

#include "sysadpt.h"
#include "core.h"
#include "hif/fwcmd.h"
#include "hif/pcie/dev.h"
#include "hif/pcie/sc4_ddr.h"
#include "hif/pcie/fwdl.h"

#define FW_DOWNLOAD_BLOCK_SIZE          256
#define FW_CHECK_MSECS                  3

/* The bootrom acknowledges a block within microseconds; these are only
 * there so that a dead device can't hang the host.
 */
#define FW_BOOTROM_READY_TIMEOUT_US     (5 * USEC_PER_SEC)
#define FW_BLOCK_ACK_TIMEOUT_US         USEC_PER_SEC
#define FW_BLOCK_ACK_POLL_US            2
#define FW_BLOCK_START_TIMEOUT_US       100
#define FW_READY_TIMEOUT_MS             (30 * MSEC_PER_SEC)

/* Copy the block the bootrom asked for into the command buffer. The size
 * comes from the device, so bound it by the buffer and zero-pad the last
 * block rather than reading past the end of the image.
 */
static int pcie_fill_block(struct mwl_priv *priv, const u8 *data,
			   size_t remaining, u32 len)
{
	u8 *buf = priv->pcmd_buf;

	if (len > MWL_CMD_BUF_SIZE) {
		wiphy_err(priv->hw->wiphy,
			  "bootrom requested %u byte block\n", len);
		return -EIO;
	}

	memcpy(buf, data, min_t(size_t, len, remaining));
	if (len > remaining)
		memset(buf + remaining, 0, len - remaining);

	return 0;
}

/* Wait for the bootrom to pick up the block sent by
 * pcie_trigger_pcicmd_bootcode(): it first sets C1C (~1.3 us after the
 * doorbell), then clears the doorbell bit once it is done with the block.
 * The bootrom may finish before C1C is ever seen non-zero, so missing that
 * window is not an error; only the doorbell clear is required.
 */
static int pcie_wait_block_ack(struct pcie_priv *pcie_priv)
{
	void __iomem *reg = pcie_priv->iobase1 + 0xc1c;
	u32 int_code;

	readl_poll_timeout_atomic(reg, int_code, int_code != 0,
				  0, FW_BLOCK_START_TIMEOUT_US);

	return readl_poll_timeout(reg, int_code,
				  !(int_code & MACREG_H2ARIC_BIT_DOOR_BELL),
				  FW_BLOCK_ACK_POLL_US, FW_BLOCK_ACK_TIMEOUT_US);
}

static void pcie_trigger_pcicmd_bootcode(struct pcie_priv *pcie_priv)
{
	writel(pcie_priv->mwl_priv->pphys_cmd_buf,
	       pcie_priv->iobase1 + MACREG_REG_GEN_PTR);
	writel(0x00, pcie_priv->iobase1 + MACREG_REG_INT_CODE);
	writel(MACREG_H2ARIC_BIT_DOOR_BELL,
	       pcie_priv->iobase1 + MACREG_REG_H2A_INTERRUPT_EVENTS);
}

static bool pcie_download_ddr_init(struct mwl_priv *priv)
{
	struct pcie_priv *pcie_priv = priv->hif.priv;
	u32 size_ddr_init = sizeof(sc4_ddr_init);
	u8 *p = &sc4_ddr_init[0];
	u32 size_ddr_init_downloaded = 0;
	u32 len = 0;

	/* download ddr init code */
	wiphy_dbg(priv->hw->wiphy, "ddr init: download start\n");

	while (size_ddr_init_downloaded < size_ddr_init) {
		len = readl(pcie_priv->iobase1 + 0xc40);

		if (!len)
			break;

		/* this copies the next chunk of fw binary to be delivered */
		if (pcie_fill_block(priv, p,
				    size_ddr_init - size_ddr_init_downloaded,
				    len))
			return false;
		/* this function writes pdata to c10, then write 2 to c18 */
		pcie_trigger_pcicmd_bootcode(pcie_priv);

		if (pcie_wait_block_ack(pcie_priv)) {
			wiphy_err(priv->hw->wiphy,
				  "timeout waiting for ddr init block ack\n");
			return false;
		}

		p += len;
		size_ddr_init_downloaded += len;
	}

	wiphy_dbg(priv->hw->wiphy, "ddr init: download complete\n");

	return true;
}

void pcie_reset(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	u32 regval;

	regval = readl(pcie_priv->iobase1 + MACREG_REG_INT_CODE);
	if (regval == 0xffffffff) {
		wiphy_err(priv->hw->wiphy, "adapter does not exist\n");
		return;
	}

	writel(ISR_RESET, pcie_priv->iobase1 + MACREG_REG_H2A_INTERRUPT_EVENTS);
}

int pcie_download_firmware(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;
	struct pcie_priv *pcie_priv = priv->hif.priv;
	const struct firmware *fw = priv->fw_ucode;
	u16 *pcmd = priv->pcmd_buf;
	unsigned long timeout;
	u32 size_fw_downloaded = 0;
	u32 int_code = 0;
	u32 len = 0;
	u32 fwreadysignature = HOSTCMD_SOFTAP_FWRDY_SIGNATURE;

	pcie_reset(hw);

	/* FW before jumping to boot rom, it will enable PCIe transaction retry,
	 * wait for boot code to stop it.
	 */
	usleep_range(FW_CHECK_MSECS * 1000, FW_CHECK_MSECS * 2000);

	if (priv->chip_type == MWL8964) {
		writel(MACREG_A2HRIC_BIT_MASK_NDP,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_CLEAR_SEL);
	} else {
		writel(MACREG_A2HRIC_BIT_MASK,
		       pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_CLEAR_SEL);
	}
	writel(0x00, pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_CAUSE);
	writel(0x00, pcie_priv->iobase1 + MACREG_REG_A2H_INTERRUPT_MASK);
	if (priv->chip_type == MWL8964) {
		writel(MACREG_A2HRIC_BIT_MASK_NDP,
		       pcie_priv->iobase1 +
		       MACREG_REG_A2H_INTERRUPT_STATUS_MASK);
	} else {
		writel(MACREG_A2HRIC_BIT_MASK,
		       pcie_priv->iobase1 +
		       MACREG_REG_A2H_INTERRUPT_STATUS_MASK);
	}

	/* this routine interacts with SC2 bootrom to download firmware binary
	 * to the device. After DMA'd to SC2, the firmware could be deflated to
	 * reside on its respective blocks such as ITCM, DTCM, SQRAM,
	 * (or even DDR, AFTER DDR is init'd before fw download
	 */
	wiphy_dbg(hw->wiphy, "fw download start\n");

	if (priv->chip_type != MWL8997)
		/* Disable PFU before FWDL */
		writel(0x100, pcie_priv->iobase1 + 0xE0E4);

	/* make sure SCRATCH2 C40 is clear, in case we are too quick */
	if (readl_poll_timeout(pcie_priv->iobase1 + 0xc40, len, len != 0,
			       10, FW_BOOTROM_READY_TIMEOUT_US)) {
		wiphy_err(hw->wiphy, "timeout waiting for bootrom\n");
		goto err_download;
	}

	if (priv->chip_type == MWL8964) {
		if (!pcie_download_ddr_init(priv)) {
			wiphy_err(hw->wiphy,
				  "ddr init: code download failed\n");
			goto err_download;
		}
	}

	while (size_fw_downloaded < fw->size) {
		len = readl(pcie_priv->iobase1 + 0xc40);

		if (!len)
			break;

		/* this copies the next chunk of fw binary to be delivered */
		if (pcie_fill_block(priv, fw->data + size_fw_downloaded,
				    fw->size - size_fw_downloaded, len))
			goto err_download;

		/* this function writes pdata to c10, then write 2 to c18 */
		pcie_trigger_pcicmd_bootcode(pcie_priv);

		if (pcie_wait_block_ack(pcie_priv)) {
			wiphy_err(hw->wiphy,
				  "timeout waiting for fw block ack\n");
			goto err_download;
		}

		size_fw_downloaded += len;
	}

	wiphy_dbg(hw->wiphy, "FwSize = %zu downloaded Size = %u\n",
		    fw->size, size_fw_downloaded);

	/* Now firmware is downloaded successfully, so this part is to check
	 * whether fw can properly execute to an extent that write back
	 * signature to indicate its readiness to the host. NOTE: if your
	 * downloaded fw crashes, this signature checking will fail. This
	 * part is similar as SC1
	 */
	*(u32 *)&pcmd[1] = 0;
	pcie_trigger_pcicmd_bootcode(pcie_priv);
	timeout = jiffies + msecs_to_jiffies(FW_READY_TIMEOUT_MS);
	do {
		writel(HOSTCMD_SOFTAP_MODE,
		       pcie_priv->iobase1 + MACREG_REG_GEN_PTR);
		usleep_range(FW_CHECK_MSECS * 1000, FW_CHECK_MSECS * 2000);
		int_code = readl(pcie_priv->iobase1 + MACREG_REG_INT_CODE);
	} while (int_code != fwreadysignature &&
		 time_before(jiffies, timeout));

	if (int_code != fwreadysignature) {
		wiphy_err(hw->wiphy,
			  "timeout waiting for fw ready signature (0x%08x)\n",
			  int_code);
		goto err_download;
	}

	wiphy_dbg(hw->wiphy, "fw download complete\n");
	writel(0x00, pcie_priv->iobase1 + MACREG_REG_INT_CODE);

	return 0;

err_download:

	pcie_reset(hw);

	return -EIO;
}
