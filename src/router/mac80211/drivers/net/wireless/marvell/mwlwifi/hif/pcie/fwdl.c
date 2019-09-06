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

#include "sysadpt.h"
#include "core.h"
#include "hif/fwcmd.h"
#include "hif/pcie/dev.h"
#include "hif/pcie/sc4_ddr.h"
#include "hif/pcie/fwdl.h"

#define FW_DOWNLOAD_BLOCK_SIZE          256
#define FW_CHECK_MSECS                  3

#define FW_MAX_NUM_CHECKS               0xffff

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
	struct pcie_priv *pcie_priv = (struct pcie_priv *)priv->hif.priv;
	u32 size_ddr_init = sizeof(sc4_ddr_init);
	u8 *p = (u8 *)&sc4_ddr_init[0];
	u32 curr_iteration = 0;
	u32 size_ddr_init_downloaded = 0;
	u32 int_code = 0;
	u32 len = 0;

	/* download ddr init code */
	wiphy_debug(priv->hw->wiphy, "ddr init: download start\n");

	while (size_ddr_init_downloaded < size_ddr_init) {
		len = readl(pcie_priv->iobase1 + 0xc40);

		if (!len)
			break;

		/* this copies the next chunk of fw binary to be delivered */
		memcpy((char *)&priv->pcmd_buf[0], p, len);
		/* this is arbitrary per your platform; we use 0xffff */
		curr_iteration = (FW_MAX_NUM_CHECKS * 500);
		/* this function writes pdata to c10, then write 2 to c18 */
		pcie_trigger_pcicmd_bootcode(pcie_priv);

		/* NOTE: the following back to back checks on C1C is time
		 * sensitive, hence may need to be tweaked dependent on host
		 * processor. Time for SC2 to go from the write of event 2 to
		 * C1C == 2 is ~1300 nSec. Hence the checkings on host has to
		 * consider how efficient your code can be to meet this timing,
		 * or you can alternatively tweak this routines to fit your
		 * platform
		 */
		do {
			int_code = readl(pcie_priv->iobase1 + 0xc1c);
			if (int_code != 0)
				break;
			cond_resched();
			curr_iteration--;
		} while (curr_iteration);

		do {
			int_code = readl(pcie_priv->iobase1 + 0xc1c);
			if ((int_code & MACREG_H2ARIC_BIT_DOOR_BELL) !=
			    MACREG_H2ARIC_BIT_DOOR_BELL)
				break;
			cond_resched();
			curr_iteration--;
		} while (curr_iteration);

		if (curr_iteration == 0) {
			/* This limited loop check allows you to exit gracefully
			 * without locking up your entire system just because fw
			 * download failed
			 */
			wiphy_err(priv->hw->wiphy,
				  "Exhausted curr_iteration during download\n");
			return false;
		}

		p += len;
		size_ddr_init_downloaded += len;
	}

	wiphy_debug(priv->hw->wiphy, "ddr init: download complete\n");

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
	u32 curr_iteration = 0;
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
	wiphy_debug(hw->wiphy, "fw download start\n");

	if (priv->chip_type != MWL8997)
		/* Disable PFU before FWDL */
		writel(0x100, pcie_priv->iobase1 + 0xE0E4);

	/* make sure SCRATCH2 C40 is clear, in case we are too quick */
	while (readl(pcie_priv->iobase1 + 0xc40) == 0)
		cond_resched();

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
		memcpy((char *)&priv->pcmd_buf[0],
		       (fw->data + size_fw_downloaded), len);

		/* this function writes pdata to c10, then write 2 to c18 */
		pcie_trigger_pcicmd_bootcode(pcie_priv);

		/* this is arbitrary per your platform; we use 0xffff */
		curr_iteration = FW_MAX_NUM_CHECKS;

		/* NOTE: the following back to back checks on C1C is time
		 * sensitive, hence may need to be tweaked dependent on host
		 * processor. Time for SC2 to go from the write of event 2 to
		 * C1C == 2 is ~1300 nSec. Hence the checkings on host has to
		 * consider how efficient your code can be to meet this timing,
		 * or you can alternatively tweak this routines to fit your
		 * platform
		 */
		do {
			int_code = readl(pcie_priv->iobase1 + 0xc1c);
			if ((int_code & MACREG_H2ARIC_BIT_DOOR_BELL) !=
			    MACREG_H2ARIC_BIT_DOOR_BELL)
				break;
			cond_resched();
			curr_iteration--;
		} while (curr_iteration);

		if (curr_iteration == 0) {
			/* This limited loop check allows you to exit gracefully
			 * without locking up your entire system just because fw
			 * download failed
			 */
			wiphy_err(hw->wiphy,
				  "Exhausted curr_iteration for fw download\n");
			goto err_download;
		}

		size_fw_downloaded += len;
	}

	wiphy_debug(hw->wiphy,
		    "FwSize = %d downloaded Size = %d curr_iteration %d\n",
		    (int)fw->size, size_fw_downloaded, curr_iteration);

	/* Now firware is downloaded successfully, so this part is to check
	 * whether fw can properly execute to an extent that write back
	 * signature to indicate its readiness to the host. NOTE: if your
	 * downloaded fw crashes, this signature checking will fail. This
	 * part is similar as SC1
	 */
	*((u32 *)&priv->pcmd_buf[1]) = 0;
	pcie_trigger_pcicmd_bootcode(pcie_priv);
	curr_iteration = FW_MAX_NUM_CHECKS;
	do {
		curr_iteration--;
		writel(HOSTCMD_SOFTAP_MODE,
		       pcie_priv->iobase1 + MACREG_REG_GEN_PTR);
		usleep_range(FW_CHECK_MSECS * 1000, FW_CHECK_MSECS * 2000);
		int_code = readl(pcie_priv->iobase1 + MACREG_REG_INT_CODE);
		if (!(curr_iteration % 0xff) && (int_code != 0))
			wiphy_err(hw->wiphy, "%x;", int_code);
	} while ((curr_iteration) &&
		 (int_code != fwreadysignature));

	if (curr_iteration == 0) {
		wiphy_err(hw->wiphy,
			  "Exhausted curr_iteration for fw signature\n");
		goto err_download;
	}

	wiphy_debug(hw->wiphy, "fw download complete\n");
	writel(0x00, pcie_priv->iobase1 + MACREG_REG_INT_CODE);

	return 0;

err_download:

	pcie_reset(hw);

	return -EIO;
}
