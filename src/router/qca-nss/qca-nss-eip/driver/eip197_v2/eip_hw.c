/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/mod_devicetable.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/firmware.h>
#include <linux/clk.h>
#include <linux/reset.h>

#include "eip_priv.h"
#include <linux/of.h>

/*
 * eip_hw_next_compatible_child()
 *	Find the next child node with a matching compatible string
 *
 * @parent: Parent device node
 * @prev: Previous child node (or NULL to start from the beginning)
 * @compatible: Compatible string to match
 */
static struct device_node *eip_hw_next_compatible_child(struct device_node *parent,
		struct device_node *prev, const char *compatible)
{
	struct device_node *child = prev;

	while ((child = of_get_next_child(parent, child))) {
		if (of_device_is_compatible(child, compatible))
			return child;
	}

	return NULL;
}

/*
 * eip_hw_external_reset()
 *	Reset EIP hardware.
 */
static bool eip_hw_external_reset(struct platform_device *pdev)
{
	struct device_node *np = of_node_get(pdev->dev.of_node);
	const char *rst_string;
	int count, i;

	/*
	 * Get Reset singal count
	 */
	count = of_property_count_strings(np, "reset-names");
	if (count < 0) {
		pr_warn("%px:EIP reset-names entry not found\n", np);
		return false;
	}

	for (i = 0; i < count; i++) {
		struct reset_control *hw_rst;

		/*
		 * parse reset control from DTSI
		 */
		of_property_read_string_index(np, "reset-names", i, &rst_string);

		hw_rst = devm_reset_control_get(&pdev->dev, rst_string);
		if (!hw_rst) {
			pr_warn("%px:EIP reset control not found\n", np);
			return false;
		}

		reset_control_assert(hw_rst);
		udelay(100);

		reset_control_deassert(hw_rst);
		udelay(100);
	}

	return true;
}

/*
 * eip_hw_reset_status()
 *	returns true if reset is successful
 */
static bool eip_hw_reset_status(void __iomem *base_addr)
{
	uint32_t status;

	/*
	 * Data Fetch Engine and Data Store Engine status are checked
	 * in order to determine whether reset is completed.
	 */
	status = ioread32(base_addr + EIP_HW_HIA_DFE_THR_STAT);
	if (status != EIP_HW_DFE_RST_DONE)
		return false;

	status = ioread32(base_addr + EIP_HW_HIA_DSE_THR_STAT);
	if (status != EIP_HW_DSE_RST_DONE)
		return false;

	iowrite32(0x0, base_addr + EIP_HW_HIA_DFE_THR_CTRL);
	iowrite32(0x0, base_addr + EIP_HW_HIA_DSE_THR_CTRL);

	return true;
}

/*
 * eip_hw_reset()
 *	HW reset routine
 */
static int eip_hw_reset(void __iomem *base_addr)
{
	/*
	 * Reset Host Interface Adapter(HIA) module
	 */
	iowrite32(EIP_HW_HIA_RST, base_addr + EIP_HW_HIA_DFE_THR_CTRL);
	iowrite32(0x0, base_addr + EIP_HW_HIA_DFE_CFG);
	iowrite32(0x0, base_addr + EIP_HW_HIA_RA_PRIO0);
	iowrite32(0x0, base_addr + EIP_HW_HIA_RA_PRIO1);
	iowrite32(0x0, base_addr + EIP_HW_HIA_RA_PRIO2);
	iowrite32(EIP_HW_HIA_RST, base_addr + EIP_HW_HIA_RA_PE_CTRL);
	iowrite32(EIP_HW_HIA_RST, base_addr + EIP_HW_HIA_DSE_THR_CTRL);
	iowrite32(EIP_HW_DSE_CFG_RST, base_addr + EIP_HW_HIA_DSE_CFG);
	iowrite32(0x0, base_addr + EIP_HW_HIA_LASIDE_BASE_ADDR_LO);
	iowrite32(0x0, base_addr + EIP_HW_HIA_LASIDE_BASE_ADDR_HI);
	iowrite32(0x0, base_addr + EIP_HW_HIA_LASIDE_SLAVE_CTRL0);
	iowrite32(0x0, base_addr + EIP_HW_HIA_LASIDE_MASTER_CTRL1);
	iowrite32(0x0, base_addr + EIP_HW_HIA_INLINE_CTRL0);
	iowrite32(0x0, base_addr + EIP_HW_PE_IN_DBUF_THR);
	iowrite32(0x0, base_addr + EIP_HW_PE_IN_TBUF_THR);
	iowrite32(0x0, base_addr + EIP_HW_PE_OUT_DBUF_THRES);
	iowrite32(0x0, base_addr + EIP_HW_PE_OUT_DBUF_THRES);
	iowrite32(EIP_HW_TOKEN_CTRL_RST,
		base_addr + EIP_HW_PE_EIP96_TOKEN_CTRL);

	/*
	 * wait for 10ms before polling the status
	 */
	mdelay(10);

	return eip_hw_reset_status(base_addr);
}

/*
 * eip_hw_setup_core()
 *	setup eip197 core
 */
static void eip_hw_setup_core(void __iomem *base_addr)
{
	uint32_t data;
	uint32_t val;

	/*
	 * MASTER BURST SIZE update
	 */
	iowrite32(EIP_HW_MST_CFG, base_addr + EIP_HW_MST_CTRL);

	/*
	 * HIA MASTER BURST SIZE update
	 */
	iowrite32(EIP_HW_HIA_MST_CFG, base_addr + EIP_HW_HIA_MST_CTRL);

	/*
	 * Data Fetch Engine configuration
	 */
	iowrite32(EIP_HW_DFE_CFG, base_addr + EIP_HW_HIA_DFE_CFG);

	/*
	 * Data Store engine configuration
	 */
	iowrite32(EIP_HW_DSE_CFG, base_addr + EIP_HW_HIA_DSE_CFG);
	iowrite32(EIP_HW_INDATA_THR, base_addr + EIP_HW_PE_IN_DBUF_THR);
	iowrite32(EIP_HW_INTOKEN_THR, base_addr + EIP_HW_PE_IN_TBUF_THR);
	iowrite32(EIP_HW_OUTDATA_THR, base_addr + EIP_HW_PE_OUT_DBUF_THRES);

	/*
	 * This enable timeout counter, if packet engine hangs, this timeout can
	 * help to recover from hang
	 */
	iowrite32(EIP_HW_TOKEN_CFG, base_addr + EIP_HW_PE_EIP96_TOKEN_CTRL);
	iowrite32(EIP_HW_TOKEN2_CFG, base_addr + EIP_HW_PE_EIP96_TOKEN_CTRL2);
	iowrite32(0x0, base_addr + EIP_HW_PE_EIP96_OUT_BUF_CTRL);

	/*
	 * Write LA FIFO BASE ADDR LOW WRITE
	 */
	iowrite32(0x0, base_addr + EIP_HW_HIA_LASIDE_BASE_ADDR_LO);

	/*
	 * This bit is set for a command descriptor error, write 1 to clear the error
	 */
	iowrite32(EIP_HW_HIA_RST, base_addr + EIP_HW_HIA_LASIDE_SLAVE_CTRL0);

	/*
	 * LA master control write
	 */
	iowrite32(EIP_HW_HIA_RST, base_addr + EIP_HW_HIA_LASIDE_MASTER_CTRL1);
	iowrite32(EIP_HW_INLINE_CFG, base_addr + EIP_HW_HIA_INLINE_CTRL0);
	iowrite32(EIP_HW_ENB_ALL_RINGS, base_addr + EIP_HW_HIA_RA_PE_CTRL);

	/*
	 * Enable Data store engine processing thread
	 */
	data = ioread32(base_addr + EIP_HW_HIA_DSE_THR_CTRL);
	data |= EIP_HW_ENB_DSE_THREAD;
	iowrite32(data, base_addr + EIP_HW_HIA_DSE_THR_CTRL);

	/* DRBG INIT */

	/*
	 * Set the engine in idle mode
	 */
	iowrite32(0x0, base_addr + EIP_HW_DRBG_CONTROL);
	iowrite32(0x0, base_addr + EIP_HW_DRBG_CONTROL);

	/*
	 * poll the ps_ai_write_ok bit
	 */
	val = ioread32(base_addr + EIP_HW_DRBG_STATUS);
	while (!(val & EIP_HW_DRBG_AI_STATUS_MASK)) {
		pr_info("DRBG module not ready for reseed");
		val = ioread32(base_addr + EIP_HW_DRBG_STATUS);
	}

	/*
	 * TODO; check EIP_HW_DRBG_GEN_BLOCK_SIZE default value
	 */

	/*
	 * Program the seed values
	 */
	iowrite32(EIP_HW_DRBG_INITIAL_SEED0, base_addr + EIP_HW_DRBG_PS_AI(0));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED1, base_addr + EIP_HW_DRBG_PS_AI(1));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED2, base_addr + EIP_HW_DRBG_PS_AI(2));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED3, base_addr + EIP_HW_DRBG_PS_AI(3));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED4, base_addr + EIP_HW_DRBG_PS_AI(4));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED5, base_addr + EIP_HW_DRBG_PS_AI(5));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED6, base_addr + EIP_HW_DRBG_PS_AI(6));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED7, base_addr + EIP_HW_DRBG_PS_AI(7));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED8, base_addr + EIP_HW_DRBG_PS_AI(8));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED9, base_addr + EIP_HW_DRBG_PS_AI(9));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED10, base_addr + EIP_HW_DRBG_PS_AI(10));
	iowrite32(EIP_HW_DRBG_INITIAL_SEED11, base_addr + EIP_HW_DRBG_PS_AI(11));

	/*
	 * Enable DRBG
	 */
	iowrite32(EIP_HW_DRBG_CTRL_STUCKOUT | EIP_HW_DRBG_CTRL_EN,
		base_addr + EIP_HW_DRBG_CONTROL);

	/*
	 * Reset Global interrupt registers and advance interrupt control register
	 */
	iowrite32(EIP_HW_HIA_IRQ_DISABLE, base_addr + EIP_HW_HIA_AIC_G_ENABLE_CTRL);
	iowrite32(EIP_HW_HIA_IRQ_CLEAR, base_addr + EIP_HW_HIA_AIC_G_ACK);
	iowrite32(EIP_HW_HIA_IRQ_DISABLE, base_addr + EIP_HW_HIA_AIC_R0_ENABLE_CTRL);
	iowrite32(EIP_HW_HIA_IRQ_DISABLE, base_addr + EIP_HW_HIA_AIC_R1_ENABLE_CTRL);
	iowrite32(EIP_HW_HIA_IRQ_DISABLE, base_addr + EIP_HW_HIA_AIC_R2_ENABLE_CTRL);
	iowrite32(EIP_HW_HIA_IRQ_DISABLE, base_addr + EIP_HW_HIA_AIC_R3_ENABLE_CTRL);
	iowrite32(EIP_HW_HIA_IRQ_CLEAR, base_addr + EIP_HW_HIA_AIC_R0_ACK);
	iowrite32(EIP_HW_HIA_IRQ_CLEAR, base_addr + EIP_HW_HIA_AIC_R1_ACK);
	iowrite32(EIP_HW_HIA_IRQ_CLEAR, base_addr + EIP_HW_HIA_AIC_R2_ACK);
	iowrite32(EIP_HW_HIA_IRQ_CLEAR, base_addr + EIP_HW_HIA_AIC_R3_ACK);
}

/*
 * eip_hw_setup_cache()
 *	setup EIP197 flow and transform cache
 */
static void eip_hw_setup_cache(void __iomem *base_addr)
{
	void __iomem *addr;
	uint32_t val;
	int i;

	/*
	 * Enable Flow record cache 0
	 */
	iowrite32(EIP_HW_ENB_FLOW_REC, base_addr + EIP_HW_CS_RAM_CTRL);
	iowrite32(0x0, base_addr + EIP_HW_FRC_ECCCTRL);

	/*
	 * Put Flow record cache into SW reset
	 */
	iowrite32(EIP_HW_RST_FLOW_REC, base_addr + EIP_HW_FRC_PARAMS);

	/*
	 * Initialise FLow record Cache
	 * Set Record administrative words
	 *	- Set Hash collision Head and Next pointers
	 *	- Set Previous and Next free list pointers
	 * Hash table words
	 *	- Set Hash collision head
	 */
	for (i = 0 ; i < EIP_HW_MAX_FLOW_REC; i++) {
		addr = base_addr + EIP_HW_CS_RAM_START + (i * 16);

		/*
		 * if its the first record then, then free_list_prev is loaded
		 * with 0x3ff, while all other records are loaded with record_index - 1
		 *
		 * if its the last record the, then free_list_next is loaded
		 * with 0x3ff, while all other records are loaded with record_index + 1
		 */
		if (i == 0)
			val = (EIP_HW_EMPTY_REC << 10) | 0x1;
		else if (i == (EIP_HW_MAX_FLOW_REC - 1))
			val = ((i - 1) << 10) | EIP_HW_EMPTY_REC;
		else
			val =  ((i - 1) << 10) | (i + 1);

		iowrite32(EIP_HW_RST_HASH_COL, addr);
		iowrite32(val, addr + 0x4);
		iowrite32(0, addr + 0x8);
		iowrite32(0, addr + 0xC);
	}

	/*
	 * reset hash collision bucket
	 */
	addr = base_addr + EIP_HW_CS_RAM_START + EIP_HW_FLOW_REC_END;
	for (i = 0; i < EIP_HW_MAX_FRC_HASH_BUCKETS; i++)
		iowrite32(U32_MAX, addr + (0x4 * i));

	/*
	 * configure FRC control registers
	 */
	iowrite32(0x0, base_addr + EIP_HW_CS_RAM_CTRL);
	iowrite32(EIP_HW_FRC_FREECHAIN_CFG, base_addr + EIP_HW_FRC_FREECHAIN);
	iowrite32(EIP_HW_FRC_PARAMS2_CFG, base_addr + EIP_HW_FRC_PARAMS2);
	iowrite32(EIP_HW_FRC_PARAMS_CFG, base_addr + EIP_HW_FRC_PARAMS);

	/*
	 * Enable Transform record cache 0
	 */
	iowrite32(EIP_HW_ENB_TRANS_REC, base_addr + EIP_HW_CS_RAM_CTRL);

	/*
	 * Put Transform record cache into SW reset
	 */
	iowrite32(0x1, base_addr + EIP_HW_TRC_PARAMS);

	/*
	 * Initialise Transform record Cache
	 * Set Record administrative words
	 *	- Set Hash collision Head and Next pointers
	 *	- Set Previous and Next free list pointers
	 * Hash table words
	 *	- Set Hash collision head
	 */
	for (i = 0 ; i < EIP_HW_MAX_TRANS_REC; i++) {
		addr = base_addr + EIP_HW_CS_RAM_START + (i * 16);

		/*
		 * if its the first record them, then free_list_prev is loaded
		 * with 0x3ff, while all other records are loaded with record_index - 1
		 *
		 * if its the last record the, then free_list_next is loaded
		 * with 0x3ff, while all other records are loaded with record_index + 1
		 */
		if (i == 0)
			val = ((EIP_HW_EMPTY_REC << 10) | 0x1);
		else if (i == (EIP_HW_MAX_TRANS_REC - 1))
			val = (((i - 1) << 10) | EIP_HW_EMPTY_REC);
		else
			val =  (((i - 1) << 10) | (i + 1));

		iowrite32(EIP_HW_RST_HASH_COL, addr);
		iowrite32(val, addr + 0x4);
		iowrite32(0, addr + 0x8);
		iowrite32(0, addr + 0xC);
	}

	/*
	 * reset hash collision bucket
	 */
	addr = base_addr + EIP_HW_CS_RAM_START + EIP_HW_TRANS_REC_END;
	for (i = 0; i < EIP_HW_MAX_TRC_HASH_BUCKETS; i++)
		iowrite32(U32_MAX, addr + (0x4 * i));

	/*
	 * configure TRC configuration parameters
	 */
	iowrite32(0x0, base_addr + EIP_HW_CS_RAM_CTRL);
	iowrite32(EIP_HW_TRC_FREECHAIN_CFG, base_addr + EIP_HW_TRC_FREECHAIN);
	iowrite32(EIP_HW_TRC_PARAMS2_CFG, base_addr + EIP_HW_TRC_PARAMS2);
	iowrite32(EIP_HW_TRC_PARAMS_CFG, base_addr + EIP_HW_TRC_PARAMS);

	/*
	 * Configure FLUE engine initial Hash value, This initial hash
	 * will be used by the engine to  compute flow hash id
	 */
	iowrite32(EIP_HW_FHASH_IV0_CFG, base_addr + EIP_HW_FHASH_IV0);
	iowrite32(EIP_HW_FHASH_IV1_CFG, base_addr + EIP_HW_FHASH_IV1);
	iowrite32(EIP_HW_FHASH_IV2_CFG, base_addr + EIP_HW_FHASH_IV2);
	iowrite32(EIP_HW_FHASH_IV3_CFG, base_addr + EIP_HW_FHASH_IV3);

	/*
	 * Configure FLUE_CONFIG register
	 */
	iowrite32(EIP_HW_ENB_FLUE, base_addr + EIP_HW_FLUE_CONFIG(0));
	iowrite32(EIP_HW_FLUE_OFFSET_CFG, base_addr + EIP_HW_FLUE_OFFSET);

	iowrite32(0x0, base_addr + EIP_HW_FLUE_IFC_LUT0);
	iowrite32(0x0, base_addr + EIP_HW_FLUE_IFC_LUT1);
	iowrite32(0x0, base_addr + EIP_HW_FLUE_IFC_LUT2);
 }

/*
 * eip_hw_setup_fw()
 *	setup EIP197 firmware resets and initializes
 */
static void eip_hw_setup_fw(void __iomem *base_addr)
{
	int i;

	/*
	 * Disable ICE SCRATCHRAM PAD statistics update
	 */
	iowrite32(0x1, base_addr + EIP_HW_ICE_DISABLE_STATS);

	/*
	 * Init ICE scratch control register
	 * Reset ICE scratch RAM area
	 */
	iowrite32(EIP_HW_ENB_SCRATCH_RAM, base_addr + EIP_HW_PE_ICE_SCRATCH_CTRL);
	for (i = 0; i < EIP_HW_MAX_SCRATCH_RAM; i++)
		iowrite32(0x0, base_addr + EIP_HW_PE_ICE_SCRATCH_RAM + (i * 0x4));

	/*
	 * Init OCE scratch control register
	 * Reset OCE scratch RAM area
	 */
	iowrite32(EIP_HW_ENB_SCRATCH_RAM, base_addr + EIP_HW_PE_OCE_SCRATCH_CTRL);
	for (i = 0; i < EIP_HW_MAX_SCRATCH_RAM; i++)
		iowrite32(0x0, base_addr + EIP_HW_PE_OCE_SCRATCH_RAM + (i * 0x4));

}

/*
 * eip_hw_boot_ofpp()
 *	load output flow post processor (OFPP) firmware
 */
static void eip_hw_boot_ofpp(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(EIP_HW_RST_PE, base_addr + EIP_HW_PE_OCE_FPP_CTRL);
	iowrite32(EIP_HW_ENB_OFPP, base_addr + EIP_HW_PE_OCE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		pr_warn("%px: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + EIP_HW_CS_RAM_START + i);

	/*
	 * put OCE RAM into reset so that the next engine can load
	 */
	iowrite32(0x0, base_addr + EIP_HW_PE_OCE_RAM_CTRL);
	iowrite32(EIP_HW_ENB_OFPP_DBG(fw->size), base_addr + EIP_HW_PE_OCE_FPP_CTRL);

	release_firmware(fw);

	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	pr_info("%px: %s version(0x%x)\n", dev, fw_name,
			ioread32(base_addr + EIP_HW_OCE_OFPP_VERSION_REG));
}

/*
 * eip_hw_boot_opue()
 *	load output pull-up engine (OPUE) firmware
 */
static void eip_hw_boot_opue(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(EIP_HW_RST_PE, base_addr + EIP_HW_PE_OCE_PUE_CTRL);
	iowrite32(EIP_HW_ENB_OPUE, base_addr + EIP_HW_PE_OCE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		pr_warn("%px: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + EIP_HW_CS_RAM_START + i);

	/*
	 * put OCE RAM into reset
	 */
	iowrite32(0x0, base_addr + EIP_HW_PE_OCE_RAM_CTRL);
	iowrite32(EIP_HW_ENB_OPUE_DBG(fw->size), base_addr + EIP_HW_PE_OCE_PUE_CTRL);

	release_firmware(fw);

	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	pr_info("%px: %s version(0x%x)\n", dev, fw_name,
			ioread32(base_addr + EIP_HW_OCE_OPUE_VERSION_REG));
}

/*
 * eip_hw_boot_ifpp()
 *	load input flow post processor (IFPP) firmware
 */
static void eip_hw_boot_ifpp(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(EIP_HW_RST_PE, base_addr + EIP_HW_PE_ICE_FPP_CTRL);
	iowrite32(EIP_HW_ENB_IFPP, base_addr + EIP_HW_PE_ICE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		pr_warn("%px: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + EIP_HW_CS_RAM_START + i);

	/*
	 * put ICE (input classification engine) RAM into reset
	 */
	iowrite32(0x0, base_addr + EIP_HW_PE_ICE_RAM_CTRL);
	iowrite32(EIP_HW_ENB_IFPP_DBG(fw->size), base_addr + EIP_HW_PE_ICE_FPP_CTRL);

	release_firmware(fw);

	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	pr_info("%px: %s version(0x%x)\n", dev, fw_name,
			ioread32(base_addr + EIP_HW_ICE_IFPP_VERSION_REG));
}

/*
 * eip_hw_boot_ipue()
 *	load input pull-up engine (IPUE) firmware
 */
static void eip_hw_boot_ipue(struct device *dev, void __iomem *base_addr, const char *fw_name)
{
	const struct firmware *fw = NULL;
	uint32_t *data;
	int i;

	/*
	 * pulling engine out of reset
	 */
	iowrite32(EIP_HW_RST_PE, base_addr + EIP_HW_PE_ICE_PUE_CTRL);
	iowrite32(EIP_HW_ENB_IPUE, base_addr + EIP_HW_PE_ICE_RAM_CTRL);

	/*
	 * load the firmware to the engine
	 */
	if (request_firmware(&fw, fw_name, dev)) {
		pr_warn("%px: FW(%s) load failed\n", dev, fw_name);
		return;
	}

	for (i = 0, data = (uint32_t *)fw->data; i < fw->size; i += 4, data++)
		iowrite32(*data, base_addr + EIP_HW_CS_RAM_START + i);

	/*
	 * put ICE (input classification engine) RAM into reset
	 */
	iowrite32(0x0, base_addr + EIP_HW_PE_ICE_RAM_CTRL);
	iowrite32(EIP_HW_ENB_IPUE_DBG(fw->size), base_addr + EIP_HW_PE_ICE_PUE_CTRL);

	release_firmware(fw);

	/*
	 * put a delay of 100ms before accessing the FW specific registers
	 */
	mdelay(100);

	pr_info("%px: %s version(0x%x)\n", dev, fw_name,
		ioread32(base_addr + EIP_HW_ICE_IPUE_VERSION_REG));
}

/*
 * eip_hw_setup()
 *	Pre initialization function for eip197
 */
static void eip_hw_setup(void __iomem *base_addr)
{
	/*
	 * Reset EIP blocks and check if reset is complete
	 */
	if (!eip_hw_reset(base_addr)) {
		/*
		 * If reset is not complete yet, wait for reset to
		 * complete before doing any other initialization
		 */
		while (!eip_hw_reset_status(base_addr))
			mdelay(100);
	}

	/*
	 * Pre-initialization of EIP block, the following is performed
	 * - Data Fetch Engine (DFE) init
	 * - Data Store Engine (DSE) init
	 * - Ring Arbiter (HIA_RA) init
	 * - DRBG init
	 * - Flow record cache init
	 * - Transform record cache init
	 * - ICE/OCE firmware init
	 */
	eip_hw_setup_core(base_addr);
	eip_hw_setup_cache(base_addr);
	eip_hw_setup_fw(base_addr);
}

/*
 * eip_hw_clock_init()
 *	Initialise EIP197 clock
 */
static int eip_hw_clock_init(struct device *dev, struct device_node *np)
{
	const char *clk_string;
	struct property *prop;
	uint64_t *clk_rate;
	size_t clk_entries;
	int count, len, i;
	struct clk *clk;
	int status = 0;

	if (of_machine_is_compatible("qcom,ipq9679-emulation")) {
		pr_info("%px:Skipped clock for RUMI emulation.\n", np);
		return 0;
	}

	/*
	 * Enable crypto clock
	 */
	count = of_property_count_strings(np, "clock-names");
	if (count < 0) {
		pr_info("%px:EIP clock entry not found\n", np);
		return 0;
	}

	/*
	 * Parse clock frequency from crypto DTSI node
	 */
	prop = of_find_property(np, "clock-frequency", &len);
	if (!prop) {
		pr_info("%px:EIP clock frequency entry not found\n", np);
		return 0;
	}

	BUG_ON(len < sizeof(*clk_rate));
	clk_entries = len / sizeof(*clk_rate);

	clk_rate = kzalloc(len, GFP_KERNEL);
	if (!clk_rate)
		return -ENOMEM;

	if (of_property_read_u64_array(np, "clock-frequency", clk_rate, clk_entries) < 0) {
		status = -EINVAL;
		goto free;
	}

	for (i = 0; i < count; i++) {
		/*
		 * parse clock names from DTSI
		 */
		of_property_read_string_index(np, "clock-names", i, &clk_string);

		clk = devm_clk_get(dev, clk_string);
		if (IS_ERR(clk)) {
			pr_err("%px: cannot get crypto clock: %s\n", dev, clk_string);
			status = -ENOENT;
			goto free;
		}

		if (clk_set_rate(clk, clk_rate[i])) {
			pr_err("%px: cannot set %llx freq for %s\n", dev, clk_rate[i], clk_string);
			status = -EINVAL;
			goto free;
		}

		if (clk_prepare_enable(clk)) {
			pr_err("%px: cannot enable clock: %s\n", dev, clk_string);
			status = -EFAULT;
			goto free;
		}
	}

free:
	kfree(clk_rate);
	return status;
}

/*
 * eip_hw_deinit()
 *	De-initialize the EIP197 HW
 */
void eip_hw_deinit(struct platform_device *pdev)
{
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	int i;

	eip_hw_external_reset(pdev);

	/*
	 * We just need to deinitialize DMA. Other HW register does not require any deinit.
	 */
	for (i = 0; i < NR_CPUS; i++) {
		eip_dma_la_deinit(&ep->la[i]);
	}

	if (!ep->redirect_en) {
		return;
	}

	for (i = 0; i < NR_CPUS; i++) {
		eip_dma_hy_deinit(&ep->hy[i]);
	}

	/*
	 * Deinitialize the flow table
	 */
	eip_flow_table_deinit(pdev);
}

/*
 * eip_hw_init()
 *	initialize the EIP197 HW
 */
int eip_hw_init(struct platform_device *pdev)
{
	struct device_node *np = of_node_get(pdev->dev.of_node);
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	void __iomem *base_addr = ep->dev_vaddr;
	struct eip_drv *drv = &eip_drv_g;
	struct device_node *child = NULL;
	struct device_node *prev = NULL;
	uint32_t redirect_en = 0;
	uint8_t max_ring;
	int status, cpu;
	uint32_t flags = 0;

	/*
	 * Initialize HW clock.
	 */
	status = eip_hw_clock_init(&pdev->dev, np);
	if (status < 0) {
		pr_err("%px: Failed to initialize clock status(%u)\n", pdev, status);
		goto fail_init;
	}

	status = of_property_read_u8(np, "max-ring", &max_ring);
	if (status < 0) {
		pr_err("%px: Failed to read max_ring\n", pdev);
		goto fail_init;
	}

	if ((cpumask_weight(&drv->la_cpu_map) + cpumask_weight(&drv->hy_cpu_map)) > max_ring) {
		pr_err("%px: Hardware DMA not available - Requested LA(%u) HY(%u) Available(%u)\n", ep,
			cpumask_weight(&drv->la_cpu_map), cpumask_weight(&drv->hy_cpu_map), max_ring);
		goto fail_init;
	}

	eip_hw_setup(base_addr);

	/*
	 * load engine firmware;
	 */
	eip_hw_boot_ifpp(&pdev->dev, base_addr, "ifpp.bin");
	eip_hw_boot_ipue(&pdev->dev, base_addr, "ipue.bin");
	eip_hw_boot_ofpp(&pdev->dev, base_addr, "ofpp.bin");
	eip_hw_boot_opue(&pdev->dev, base_addr, "opue.bin");

	/*
	 * Allow passing 16 bytes of metadata information from
	 * input to output.
	 */
	iowrite32(EIP_HW_ENB_MDATA, base_addr + EIP_HW_METADATA_EN_OFFSET);

	/*
	 * Force CDR clock enable to workaround PROC Counter update.
	 */
	iowrite32(EIP_HW_CDR_CLK_ON, base_addr + EIP_HW_FORCE_CLOCK_ON);

	/*
	 * Disable ECN check.
	 */
	iowrite32(EIP_HW_PE_EIP96_ECN_DISABLE, base_addr + EIP_HW_PE_EIP96_ECN_TABLE(0));
	iowrite32(EIP_HW_PE_EIP96_ECN_DISABLE, base_addr + EIP_HW_PE_EIP96_ECN_TABLE(1));
	iowrite32(EIP_HW_PE_EIP96_ECN_DISABLE, base_addr + EIP_HW_PE_EIP96_ECN_TABLE(2));
	iowrite32(EIP_HW_PE_EIP96_ECN_DISABLE, base_addr + EIP_HW_PE_EIP96_ECN_TABLE(3));

	/*
	 * Enabled Packet ID increment for IPv4.
	 * TODO: ICE also has same config. Do we need that as well?
	 */
	iowrite32(EIP_HW_OCE_OPUE_PACKET_ID_START | EIP_HW_OCE_OPUE_PACKET_ID_INC,
			base_addr + EIP_HW_OCE_OPUE_PACKET_ID_CFG);

	/*
	 * Check for inline support
	 */
	ep->inline_support = of_property_read_bool(np, "qcom,inline-enabled");
	pr_info("%px: EIP inline_support: %s", pdev, ep->inline_support ? "yes" : "no");

	/*
	 * Check for outer and inner flow offload support
	 */
	flags |= of_property_read_bool(np, "qcom,outer-offload") ? EIP_OFFLOAD_OUTER_FLOW : 0;
	flags |= of_property_read_bool(np, "qcom,inner-offload") ? EIP_OFFLOAD_INNER_FLOW : 0;
	ep->flags |= flags;

	pr_info("%px: EIP Supported features : %x", pdev, ep->flags);

	/*
	 * Find first dma copmatible node.
	 */
	child = eip_hw_next_compatible_child(np, prev, "qcom,eip-dma");
	if (!child) {
		pr_err("%px: No DMA nodes found in device tree\n", pdev);
		status = -ENODEV;
		goto fail_init;
	}

	/*
	 * Initialize & map DMA nodes to Lookaside as per provided CPU Map.
	 */
	for_each_cpu(cpu, &drv->la_cpu_map) {
		struct eip_dma *dma;

		dma = &ep->la[cpu];
		status = eip_dma_la_init(dma, pdev, child, cpu);
		if (status < 0) {
			pr_err("%px: LA DMA initialization failed for cpu(%u)\n", pdev, cpu);
			of_node_put(child);
			goto fail_init;
		}

		prev = child;
		child = eip_hw_next_compatible_child(np, prev, "qcom,eip-dma");
		of_node_put(prev);
	}

	/*
	 * Initialize & map DMA nodes to Hybrid as per provided CPU MAP.
	 */
	for_each_cpu(cpu, &drv->hy_cpu_map) {
		struct eip_dma *dma;

		dma = &ep->hy[cpu];
		status = eip_dma_hy_init(dma, pdev, child, cpu);
		if (status < 0) {
			pr_err("%px: LA DMA initialization failed for cpu(%u)\n", pdev, cpu);
			of_node_put(child);
			goto fail_init;
		}

		redirect_en |= EIP_HW_ICE_REDIRECT_EN(dma->ring_id);

		prev = child;
		child = eip_hw_next_compatible_child(np, prev, "qcom,eip-dma");
		of_node_put(prev);
	}

	/*
	 * Configure HW for Redirection.
	 */
	if (redirect_en) {
		uint8_t err_redir_ring = __builtin_ffs(redirect_en) - 1;
		uint32_t cfg;

		ep->redirect_en = !!redirect_en;

		/*
		 * Enable inorder bit when redirection is used.
		 */
		cfg = EIP_HW_INLINE_CFG | EIP_HW_INLINE_INORDER;
		iowrite32(cfg, base_addr + EIP_HW_HIA_INLINE_CTRL0);

		/*
		 * Allow redirection from those interface.
		 */
		redirect_en |= EIP_HW_ICE_REDIRECT_EN(EIP_HW_ICE_INLINE_BIT);
		iowrite32(redirect_en, base_addr + EIP_HW_ICE_REDIRECT_CFG);

		/*
		 * Redirect all error packet to hybrid ring with lowest ring index (4).
		 */
		cfg = EIP_HW_OCE_OPUE_REDIRECT_ENABLE | EIP_HW_OCE_OPUE_REDIRECT_RING_ID(err_redir_ring);
		iowrite32(cfg, base_addr + EIP_HW_OCE_OPUE_REDIRECT_CFG);

		/*
		 * Initialize hardware flow lookup table.
		 */
		if(!(eip_flow_table_init(pdev))) {
			pr_err("%px: Failed to initializ flow table\n",pdev);
			goto fail_init;
		}

		pr_info("%px: Redirection enabled, map(%x) error ring(%u)\n", pdev, redirect_en, err_redir_ring);
	}


	of_node_put(np);
	return 0;

/*
 * We have active flag in dma to handle partial deinitialization.
 */
fail_init:
	eip_hw_deinit(pdev);
	of_node_put(np);
	return status;
}
