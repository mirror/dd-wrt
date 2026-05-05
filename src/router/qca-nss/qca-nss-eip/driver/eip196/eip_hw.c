/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/mod_devicetable.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/firmware.h>
#include <linux/clk.h>
#ifdef CONFIG_IO_COHERENCY
#include <linux/tmelcom_ipc.h>
#include <linux/of_platform.h>
#endif

#include "eip_priv.h"

#ifdef CONFIG_IO_COHERENCY
/*
 * eip_hw_noc_reg_write()
 *	Write the value into EIP196 IO coherency registers
 */
static inline int eip_hw_noc_reg_write(uint32_t reg_base, uint32_t *regs_off, int count)
{
	int error;
	int i;

	for (i = 0; i < count; i++, regs_off += 2) {
		uint32_t reg_addr = reg_base + regs_off[0];
		uint32_t reg_val = regs_off[1];
		struct tmel_secure_io nss_noc = {0};

		/*
		 * Write the value into the registers using secure IO.
		 */
		nss_noc.reg_addr = reg_addr;
		nss_noc.reg_val = reg_val;
		error = tmelcom_secure_io_write(&nss_noc, sizeof(struct tmel_secure_io));
		if (error) {
			pr_err("Failed to configure NSS NOC reg = 0x%x, val=0x%x\n", reg_addr, reg_val);
			return error;
		}

		pr_debug("Configuring nss_noc for addr: (0x%x), val: (0x%x)\n", reg_addr, reg_val);
	}

	return 0;
}

/*
 * eip_hw_noc_reg_update()
 *	Update EIP196 IO coherency settings in NSS NOC register.
 *
 * Caller should call this function only after EIP196 clock is
 * enabled and before starting any transaction with EIP196.
 */
int eip_hw_noc_reg_update(struct platform_device *pdev)
{
	struct device_node *np = (&pdev->dev)->of_node;
	struct device_node *child = NULL;
	bool secure_write = false;
	uint32_t *regs_off = NULL;
	uint32_t reg_base;
	int ret, count;

	for_each_available_child_of_node(np, child) {
		/*
		 * Read the registers only if EIP IO coherency property (eip_ioc)
		 * is defined in the DTSI.
		 */
		if (of_property_match_string(child, "prop-name", "eip_ioc") >= 0) {
			break;
		}
	}

	if (!child) {
		pr_err("%px: Unable to find EIP IO coherency entry in DTSI\n", child);
		return -EINVAL;
	}

	/*
	 * Proceed only if secure write is enabled.
	 */
	secure_write = of_property_read_bool(child, "secure-write");
	if (!secure_write) {
		pr_err("%px: Secure write is not enabled \n", child);
		return -EINVAL;
	}

	/*
	 * Read the Register base address from the DTSI.
	 */
	ret = of_property_read_u32(child, "reg-base", &reg_base);
	if (ret) {
		pr_err("%px: Failed to read the Register base address\n", child);
		return ret;
	}

	/*
	 * Read the number of register elements.
	 */
	count = of_property_count_u32_elems(child, "reg-offset");
	if ((count == 0) || (count % 2)) {
		pr_err("%px: Invalid entries obtained from the DTSI\n", child);
		return -EINVAL;
	}

	/*
	 * Allocate memory for reading NOC register values from DTSI.
	 */
	regs_off = vmalloc(sizeof(u32) * count);
	if (!regs_off) {
		pr_err("%px: Failed to allocate memory for reading NOC regs\n", child);
		return -EINVAL;
	}

	/*
	 * Read the register address offsets.
	 */
	ret = of_property_read_u32_array(child, "reg-offset", regs_off, count);
	if (ret) {
		pr_err("%px: Error in fetching the offset address and value for Register\n", child);
		goto fail;
	}

	ret = eip_hw_noc_reg_write(reg_base, regs_off, count/2);
	if (ret) {
		goto fail;
	}

fail:
	vfree(regs_off);
	return ret;
}
#endif

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
	iowrite32(0x0, base_addr + EIP_HW_PE_IN_DBUF_THR);
	iowrite32(0x0, base_addr + EIP_HW_PE_IN_TBUF_THR);
	iowrite32(0x0, base_addr + EIP_HW_PE_OUT_DBUF_THRES);
	iowrite32(0x0, base_addr + EIP_HW_PE_OUT_TBUF_THRES);
	iowrite32(EIP_HW_TOKEN_CTRL_RST,
		base_addr + EIP_HW_PE_EIP96_TOKEN_CTRL);
	iowrite32(EIP_HW_TOKEN_CTRL2_RST, base_addr + EIP_HW_PE_EIP96_TOKEN_CTRL2);

	/*
	 * wait for 10ms before polling the status
	 */
	mdelay(10);

	return eip_hw_reset_status(base_addr);
}

/*
 * eip_hw_setup_core()
 *	setup eip196 core
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
	iowrite32(0x0, base_addr + EIP_HW_PE_EIP96_OUT_BUF_CTRL);

	/*
	 * LA master control write
	 */
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
 *	setup EIP196 simple transform record cache
 */
static void eip_hw_setup_cache(void __iomem *base_addr)
{
	/*
	 * Enable simple transform record cache
	 */
	iowrite32(EIP_HW_ENB_SIMPLE_TRANS_REC, base_addr + EIP_HW_CS_RAM_CTRL);

	/*
	 * Initialise the simple transform record cache
	 */
	iowrite32(EIP_HW_STRC_CONFIG_VAL, base_addr + EIP_HW_STRC_CONFIG);

	/*
	 * Configure FLUE_CONFIG register
	 */
	iowrite32(EIP_HW_ENB_FLUE, base_addr + EIP_HW_FLUE_CONFIG(0));

	iowrite32(0x0, base_addr + EIP_HW_FLUE_IFC_LUT0);
	iowrite32(0x0, base_addr + EIP_HW_FLUE_IFC_LUT1);
	iowrite32(0x0, base_addr + EIP_HW_FLUE_IFC_LUT2);
 }

/*
 * eip_hw_setup()
 *	Pre initialization function for eip196
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
		while (!eip_hw_reset_status(base_addr)){
			mdelay(100);
		}
	}

	/*
	 * Pre-initialization of EIP block, the following is performed
	 * - Data Fetch Engine (DFE) init
	 * - Data Store Engine (DSE) init
	 * - Ring Arbiter (HIA_RA) init
	 * - DRBG init
	 * - Simple transform record cache init
	 */
	eip_hw_setup_core(base_addr);
	eip_hw_setup_cache(base_addr);
}

/*
 * eip_hw_clock_init()
 *	Initialise EIP196 clock
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
 *	De-initialize the EIP196 HW
 */
void eip_hw_deinit(struct platform_device *pdev)
{
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	int i;

	/*
	 * We just need to deinitialize DMA. Other HW register does not require any deinit.
	 */
	for (i = 0; i < NR_CPUS; i++) {
		eip_dma_la_deinit(&ep->la[i]);
	}
}

/*
 * eip_hw_init()
 *	initialize the EIP196 HW
 */
int eip_hw_init(struct platform_device *pdev)
{
	struct device_node *np = of_node_get(pdev->dev.of_node);
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	void __iomem *base_addr = ep->dev_vaddr;
	struct device_node *child = NULL;
	uint8_t ring_id = 0;
	uint32_t data;
	int status;

	/*
	 * Initialize HW clock.
	 */
	status = eip_hw_clock_init(&pdev->dev, np);
	if (status < 0) {
		pr_err("%px: Failed to initialize clock status(%u)\n", pdev, status);
		goto fail_init;
	}

#ifdef CONFIG_IO_COHERENCY
	/*
	 * Update EIP196 IO coherency registers in NSS NOC.
	 */
	status = eip_hw_noc_reg_update(pdev);
	if (status) {
		pr_err("%px:Failed to configure IO coherency\n", pdev);
		goto fail_init;
	}
#endif
	eip_hw_setup(base_addr);

	/*
	 * Allow passing 16 bytes of metadata information from
	 * input to output.
	 */
	data = ioread32(base_addr + EIP_HW_PE_PE_DEBUG);
	iowrite32( (data | EIP_HW_ENB_MDATA), base_addr + EIP_HW_PE_PE_DEBUG);

	/*
	 * No inline support in EIP196
	 */
	pr_info("%px: EIP inline_support: no", pdev);

	/*
	 * Initialize all lookaside DMA.
	 */
	for_each_available_child_of_node(np, child) {
		struct eip_dma *dma;
		uint8_t tx_cpu = 0;
		uint8_t rx_cpu = 0;

		status = of_property_match_string(child, "ring-name", "lookaside");
		if (status < 0) {
			continue;
		}

		status = of_property_read_u8(child, "tx_cpu", &tx_cpu);
		if (status < 0) {
			pr_err("%px: Failed to read tx_cpu\n", pdev);
			goto fail_init;
		}

		if (tx_cpu >= NR_CPUS) {
			pr_err("%px: Invalid tx_cpu number(%u)\n", pdev, tx_cpu);
			goto fail_init;
		}

		status = of_property_read_u8(child, "rx_cpu", &rx_cpu);
		if (status < 0) {
			pr_err("%px: Failed to read rx_cpu\n", pdev);
			goto fail_init;
		}

		if (rx_cpu >= NR_CPUS) {
			pr_err("%px: Invalid rx_cpu number(%u)\n", pdev, rx_cpu);
			goto fail_init;
		}

		status = of_property_read_u8(child, "ring-id", &ring_id);
		if (status < 0) {
			pr_err("%px: Failed to read ring_id\n", pdev);
			goto fail_init;
		}

		dma = &ep->la[tx_cpu];
		status = eip_dma_la_init(dma, pdev, tx_cpu, rx_cpu, ring_id, base_addr);
		if (status < 0) {
			pr_err("%px: LA DMA initialization failed for cpu(%u)\n", pdev, tx_cpu);
			goto fail_init;
		}
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
