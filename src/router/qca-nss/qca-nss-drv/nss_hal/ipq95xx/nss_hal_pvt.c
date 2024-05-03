/*
 **************************************************************************
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * nss_hal_pvt.c
 *	NSS HAL private APIs.
 */

#include <linux/err.h>
#include <linux/version.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/reset.h>
#include "nss_hal.h"
#include "nss_core.h"

#define NSS_QGIC_IPC_REG_OFFSET 0x4

#define NSS0_H2N_INTR_BASE 4
#define NSS1_H2N_INTR_BASE 14
#define NSS2_H2N_INTR_BASE 20
#define NSS3_H2N_INTR_BASE 26

/*
 * Common clocks
 */
#define NSS_CSR_CLK "nss-csr-clk"
#define NSS_NSSNOC_CSR_CLK "nss-nssnoc-csr-clk"
#define NSS_HAQ_AHB_CLK "nss-haq-ahb-clk"
#define NSS_HAQ_AXI_CLK "nss-haq-axi-clk"
#define NSS_NSSNOC_HAQ_AHB_CLK "nss-nssnoc-haq-ahb-clk"
#define NSS_NSSNOC_HAQ_AXI_CLK "nss-nssnoc-haq-axi-clk"
#define NSS_CE_AHB_CLK "nss-ce-ahb-clk"
#define NSS_CE_AXI_CLK "nss-ce-axi-clk"
#define NSS_NSSNOC_CE_AHB_CLK "nss-nssnoc-ce-ahb-clk"
#define NSS_NSSNOC_CE_AXI_CLK "nss-nssnoc-ce-axi-clk"
#define NSS_CLC_AXI_CLK "nss-clc-axi-clk"
#define NSS_NSSNOC_CLC_AXI_CLK "nss-nssnoc-clc-axi-clk"
#define NSS_IMEM_QSB_CLK "nss-imem-qsb-clk"
#define NSS_NSSNOC_IMEM_QSB_CLK "nss-nssnoc-imem-qsb-clk"
#define NSS_IMEM_AHB_CLK "nss-imem-ahb-clk"
#define NSS_NSSNOC_IMEM_AHB_CLK "nss-nssnoc-imem-ahb-clk"
#define NSS_NSSNOC_AHB0_CLK "nss-nssnoc-ahb0-clk"
#define NSS_NSSNOC_AXI0_CLK "nss-nssnoc-axi0-clk"
#define NSS_NSSNOC_INT0_AHB_CLK "nss-nssnoc-int0-ahb-clk"
#define NSS_NSSNOC_NC_AXI0_1_CLK "nss-nssnoc-nc-axi0-1-clk"
#define NSS_NSSNOC_NC_AXI0_CLK "nss-nssnoc-nc-axi0-clk"
#define NSS_DBG_CLK "nss-dbg-clk"
#define NSS_MEM_NOC_NSSNOC_CLK "nss-mem-noc-nssnoc-clk"
#define NSS_TBU_CLK "nss-tbu-clk"
#define NSS_TS_CLK "nss-ts-clk"
#define NSS_NSSCC_CLK "nss-nsscc-clk"
#define NSS_NSSCFG_CLK "nss-nsscfg-clk"
#define NSS_NSSCNOC_ATB_CLK "nss-nsscnoc-atb-clk"
#define NSS_NSSNOC_MEM_NOC_1_CLK "nss-nssnoc-mem-noc-1-clk"
#define NSS_NSSNOC_MEMNOC_CLK "nss-nssnoc-memnoc-clk"
#define NSS_NSSNOC_NSSCC_CLK "nss-nssnoc-nsscc-clk"
#define NSS_NSSNOC_PCNOC_1_CLK "nss-nssnoc-pcnoc-1-clk"
#define NSS_NSSNOC_QOSGEN_REF_CLK "nss-nssnoc-qosgen-ref-clk"
#define NSS_NSSNOC_SNOC_1_CLK "nss-nssnoc-snoc-1-clk"
#define NSS_NSSNOC_SNOC_CLK "nss-nssnoc-snoc-clk"
#define NSS_NSSNOC_TIMEOUT_REF_CLK "nss-nssnoc-timeout-ref-clk"
#define NSS_NSSNOC_XO_DCD_CLK "nss-nssnoc-xo-dcd-clk"

/*
 * Per core clocks
 */
#define NSS_CORE_CLK "nss-core-clk"
#define NSS_AHB_CLK "nss-ahb-clk"
#define NSS_AXI_CLK "nss-axi-clk"
#define NSS_INTR_AHB_CLK "nss-intr-ahb-clk"
#define NSS_NC_AXI_CLK "nss-nc-axi-clk"
#define NSS_UTCM_CLK "nss-utcm-clk"

/*
 * N2H interrupts
 */
#define NSS_IRQ_NAME_EMPTY_BUF_SOS "nss_empty_buf_sos"
#define NSS_IRQ_NAME_EMPTY_BUF_QUEUE "nss_empty_buf_queue"
#define NSS_IRQ_NAME_TX_UNBLOCK "nss-tx-unblock"
#define NSS_IRQ_NAME_QUEUE0 "nss_queue0"
#define NSS_IRQ_NAME_QUEUE1 "nss_queue1"
#define NSS_IRQ_NAME_QUEUE2 "nss_queue2"
#define NSS_IRQ_NAME_QUEUE3 "nss_queue3"
#define NSS_IRQ_NAME_COREDUMP_COMPLETE "nss_coredump_complete"
#define NSS_IRQ_NAME_PAGED_EMPTY_BUF_SOS "nss_paged_empty_buf_sos"
#define NSS_IRQ_NAME_PROFILE_DMA "nss_profile_dma"

/*
 * Voltage values
 */
#define NOMINAL_VOLTAGE 1
#define TURBO_VOLTAGE 2

/*
 * Core reset part 1
 */
#define NSS_CORE_GCC_RESET 0x0000000C

/*
 * Voltage regulator
 */
struct regulator *npu_reg;

/*
 * GCC reset
 */
void __iomem *nss_misc_reset;

/*
 * Purpose of each interrupt index: This should match the order defined in the NSS firmware
 */
enum nss_hal_n2h_intr_purpose {
	NSS_HAL_N2H_INTR_PURPOSE_EMPTY_BUFFER_SOS = 0,
	NSS_HAL_N2H_INTR_PURPOSE_EMPTY_BUFFER_QUEUE = 1,
	NSS_HAL_N2H_INTR_PURPOSE_TX_UNBLOCKED = 2,
	NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_0 = 3,
	NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_1 = 4,
	NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_2 = 5,
	NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_3 = 6,
	NSS_HAL_N2H_INTR_PURPOSE_COREDUMP_COMPLETE = 7,
	NSS_HAL_N2H_INTR_PURPOSE_PAGED_EMPTY_BUFFER_SOS = 8,
	NSS_HAL_N2H_INTR_PURPOSE_PROFILE_DMA = 9,
	NSS_HAL_N2H_INTR_PURPOSE_MAX
};

/*
 * Interrupt type to cause vector.
 */
static uint32_t intr_cause[NSS_NPU_CORES][NSS_H2N_INTR_TYPE_MAX] = {
				/* core0 */
				{(1 << (NSS0_H2N_INTR_BASE + NSS_H2N_INTR_EMPTY_BUFFER_QUEUE)),
				(1 << (NSS0_H2N_INTR_BASE + NSS_H2N_INTR_DATA_COMMAND_QUEUE)),
				(1 << (NSS0_H2N_INTR_BASE + NSS_H2N_INTR_TX_UNBLOCKED)),
				(1 << (NSS0_H2N_INTR_BASE + NSS_H2N_INTR_TRIGGER_COREDUMP)),
				(1 << (NSS0_H2N_INTR_BASE + (NSS_H2N_INTR_EMPTY_PAGED_BUFFER_QUEUE + 4)))},
};

/*
 * nss_hal_get_core_clk()
 *	Set clock for appropriate UBI core.
 */
void nss_hal_set_core_clk(struct platform_device *nss_dev, nss_core_id_t core_id)
{
	switch (core_id) {
	case NSS_CORE_0:
		nss_core0_clk = clk_get(&nss_dev->dev, NSS_CORE_CLK);
		break;
	default:
		nss_info_always("%p: Invalid Core %u.\n", nss_dev, core_id);
	}
 }

/*
 * nss_hal_get_core_clk()
 *	Get clock for appropriate UBI core.
 */
static struct clk *nss_hal_get_core_clk(nss_core_id_t core_id)
{
	switch (core_id) {
	case NSS_CORE_0:
		return nss_core0_clk;
	default:
		nss_info_always("Invalid core %u.\n", core_id);
		return NULL;
	}
 }

/*
 * nss_hal_wq_function
 *	Added to Handle BH requests to kernel
 *
 */
void nss_hal_wq_function(struct work_struct *work)
{
	nss_work_t *my_work = (nss_work_t *)work;
	uint8_t core_id = 0;
	struct clk *core_clk;
	mutex_lock(&nss_top_main.wq_lock);

	if (my_work->frequency > NSS_FREQ_1497) {
		regulator_set_voltage(npu_reg, TURBO_VOLTAGE, TURBO_VOLTAGE);
	}

	core_id = NSS_CORE_0;
	nss_freq_change(&nss_top_main.nss[core_id], my_work->frequency, my_work->stats_enable, 0);
	core_clk = nss_hal_get_core_clk(NSS_CORE_0);
	clk_set_rate(core_clk, my_work->frequency);
	nss_freq_change(&nss_top_main.nss[NSS_CORE_0], my_work->frequency, my_work->stats_enable, 1);

	if (my_work->frequency <= NSS_FREQ_1497) {
		regulator_set_voltage(npu_reg, NOMINAL_VOLTAGE, NOMINAL_VOLTAGE);
	}

	mutex_unlock(&nss_top_main.wq_lock);
	kfree((void *)work);
}

/*
 * nss_hal_handle_irq()
 */
static irqreturn_t nss_hal_handle_irq(int irq, void *ctx)
{
	struct int_ctx_instance *int_ctx = (struct int_ctx_instance *) ctx;

	disable_irq_nosync(irq);
	napi_schedule(&int_ctx->napi);

	return IRQ_HANDLED;
}

/*
 * __nss_hal_of_get_pdata()
 *	Retrieve platform data from device node.
 */
static struct nss_platform_data *__nss_hal_of_get_pdata(struct platform_device *pdev)
{
	struct device_node *np = of_node_get(pdev->dev.of_node);
	struct nss_platform_data *npd;
	struct nss_ctx_instance *nss_ctx = NULL;
	struct nss_top_instance *nss_top = &nss_top_main;
	struct resource res_nphys, res_vphys, res_qgic_phys;
	int32_t i;

	npd = devm_kzalloc(&pdev->dev, sizeof(struct nss_platform_data), GFP_KERNEL);
	if (!npd) {
		return NULL;
	}

	if (of_property_read_u32(np, "qcom,id", &npd->id)
	    || of_property_read_u32(np, "qcom,load-addr", &npd->load_addr)
	    || of_property_read_u32(np, "qcom,num-queue", &npd->num_queue)
	    || of_property_read_u32(np, "qcom,num-irq", &npd->num_irq)) {
		pr_err("%s: error reading critical device node properties\n", np->name);
		goto out;
	}

	/*
	 * Read frequencies. If failure, load default values.
	 */
	of_property_read_u32(np, "qcom,low-frequency", &nss_runtime_samples.freq_scale[NSS_FREQ_LOW_SCALE].frequency);
	of_property_read_u32(np, "qcom,mid-frequency", &nss_runtime_samples.freq_scale[NSS_FREQ_MID_SCALE].frequency);
	of_property_read_u32(np, "qcom,max-frequency", &nss_runtime_samples.freq_scale[NSS_FREQ_HIGH_SCALE].frequency);

	if (npd->num_irq > NSS_MAX_IRQ_PER_CORE) {
		pr_err("%s: (%d) exceeds maximum interrupt numbers per core\n", np->name, npd->num_irq);
		goto out;
	}

	nss_ctx = &nss_top->nss[npd->id];
	nss_ctx->id = npd->id;

	if (of_address_to_resource(np, 0, &res_nphys) != 0) {
		nss_info_always("%px: nss%d: of_address_to_resource() fail for nphys\n", nss_ctx, nss_ctx->id);
		goto out;
	}

	if (of_address_to_resource(np, 1, &res_vphys) != 0) {
		nss_info_always("%px: nss%d: of_address_to_resource() fail for vphys\n", nss_ctx, nss_ctx->id);
		goto out;
	}

	if (of_address_to_resource(np, 2, &res_qgic_phys) != 0) {
		nss_info_always("%px: nss%d: of_address_to_resource() fail for qgic_phys\n", nss_ctx, nss_ctx->id);
		goto out;
	}

	/*
	 * Save physical addresses
	 */
	npd->nphys = res_nphys.start;
	npd->vphys = res_vphys.start;
	npd->qgic_phys = res_qgic_phys.start;

	npd->nmap = ioremap_nocache(npd->nphys, resource_size(&res_nphys));
	if (!npd->nmap) {
		nss_info_always("%px: nss%d: ioremap() fail for nphys\n", nss_ctx, nss_ctx->id);
		goto out;
	}

	npd->vmap = ioremap_cache(npd->vphys, resource_size(&res_vphys));
	if (!npd->vmap) {
		nss_info_always("%px: nss%d: ioremap() fail for vphys\n", nss_ctx, nss_ctx->id);
		goto out;
	}

	npd->qgic_map = ioremap_nocache(npd->qgic_phys, resource_size(&res_qgic_phys));
	if (!npd->qgic_map) {
		nss_info_always("%px: nss%d: ioremap() fail for qgic map\n", nss_ctx, nss_ctx->id);
		goto out;
	}

	/*
	 * Get IRQ numbers
	 */
	for (i = 0 ; i < npd->num_irq; i++) {
		npd->irq[i] = irq_of_parse_and_map(np, i);
		if (!npd->irq[i]) {
			nss_info_always("%px: nss%d: irq_of_parse_and_map() fail for irq %d\n", nss_ctx, nss_ctx->id, i);
			goto out;
		}
	}

	nss_hal_dt_parse_features(np, npd);

	of_node_put(np);
	return npd;

out:
	if (npd->nmap) {
		iounmap(npd->nmap);
	}

	if (npd->vmap) {
		iounmap(npd->vmap);
	}

	devm_kfree(&pdev->dev, npd);
	of_node_put(np);
	return NULL;
}

/*
 * nss_hal_clock_set_and_enable()
 */
static int nss_hal_clock_set_and_enable(struct device *dev, const char *id, unsigned long rate)
{
	struct clk *nss_clk = NULL;
	int err;

	nss_clk = devm_clk_get(dev, id);
	if (IS_ERR(nss_clk)) {
		pr_err("%px: cannot get clock: %s\n", dev, id);
		return -EFAULT;
	}

	if (rate) {
		err = clk_set_rate(nss_clk, rate);
		if (err) {
			pr_err("%px: cannot set %s freq\n", dev, id);
			return -EFAULT;
		}
	}

	err = clk_prepare_enable(nss_clk);
	if (err) {
		pr_err("%px: cannot enable clock: %s\n", dev, id);
		return -EFAULT;
	}

	return 0;
}

/*
 * __nss_hal_core_reset()
 *	Reset the UBI cores
 *
 * TODO: Verify reset sequence with the HW team
 */
static int __nss_hal_core_reset(struct platform_device *nss_dev, void __iomem *map, uint32_t addr, uint32_t clk_src)
{
	uint32_t value;

	/*
	 * De-assert reset for Core clamps
	 */
	value = nss_read_32(nss_misc_reset, 0x0);
	value &= ~(NSS_CORE_GCC_RESET << ((nss_dev->id) * 2));
	nss_write_32(nss_misc_reset, 0x0, value);

	/*
	 * Apply ubi32 core reset
	 */
	nss_write_32(map, NSS_REGS_RESET_CTRL_OFFSET, 1);

	/*
	 * Program address configuration
	 */
	nss_write_32(map, NSS_REGS_CORE_BAR_OFFSET, 0x3e000000);
	nss_write_32(map, NSS_REGS_CORE_BOOT_ADDR_OFFSET, addr);

	/*
	 * Enable interrupt enable registers
	 */
	nss_write_32(map, NSS_REGS_CORE_INT_STAT0_ENABLE_OFFSET, 0xFFFFFFFF);
	nss_write_32(map, NSS_REGS_CORE_INT_STAT1_ENABLE_OFFSET, 0xFFFFFFFF);
	nss_write_32(map, NSS_REGS_CORE_INT_STAT2_ENABLE_OFFSET, 0xFFFFFFFF);
	nss_write_32(map, NSS_REGS_CORE_INT_STAT3_ENABLE_OFFSET, 0xFFFFFFFF);

	/*
	 * N2H and int_stat3 as level interrupt
	 */
	nss_write_32(map, NSS_REGS_CORE_INT_STAT3_TYPE_OFFSET, 0xFFFFFFFF);

	/*
	 * Enable Instruction Fetch range checking between 0x4000 0000 to 0xBFFF FFFF.
	 */
	nss_write_32(map, NSS_REGS_CORE_IFETCH_RANGE_OFFSET, 0xBF004001);

	/*
	 * De-assert ubi32 core reset
	 */
	nss_write_32(map, NSS_REGS_RESET_CTRL_OFFSET, 0);

	return 0;
}

/*
 * __nss_hal_debug_enable()
 *	Enable NSS debug
 */
static void __nss_hal_debug_enable(void)
{

}

/*
 * __nss_hal_common_reset
 *	Do reset/clock configuration common to all cores
 */
static int __nss_hal_common_reset(struct platform_device *nss_dev)
{
	struct device_node *cmn = NULL;
	struct resource res_nss_misc_reset;

	/*
	 * TODO: Talking to VI team about confirming clocks.
	 */
	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CSR_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_CSR_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_HAQ_AHB_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_HAQ_AXI_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_HAQ_AHB_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_HAQ_AXI_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CE_AHB_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CE_AXI_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_CE_AHB_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_CE_AXI_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CLC_AXI_CLK, 533330000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_CLC_AXI_CLK, 533330000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_IMEM_QSB_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_IMEM_QSB_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_IMEM_AHB_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_IMEM_AHB_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_AHB0_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_AXI0_CLK, 533330000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_INT0_AHB_CLK, 200000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_NC_AXI0_1_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_NC_AXI0_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_MEM_NOC_NSSNOC_CLK, 533333333)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_TBU_CLK, 533333333)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_TS_CLK, 24000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSCC_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSCFG_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSCNOC_ATB_CLK, 240000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_MEM_NOC_1_CLK, 533333333)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_MEMNOC_CLK, 533333333)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_NSSCC_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_PCNOC_1_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_QOSGEN_REF_CLK, 6000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_SNOC_1_CLK, 342857143)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_SNOC_CLK, 342857143)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_TIMEOUT_REF_CLK, 6000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NSSNOC_XO_DCD_CLK, 24000000)) {
		return -EFAULT;
	}

	nss_info("%p: Common clocks initialized successfully.\n", nss_dev);

	/*
	 * Get reference to NSS common device node
	 */
	cmn = of_find_node_by_name(NULL, "nss-common");
	if (!cmn) {
		pr_err("%px: Unable to find nss-common node\n", nss_dev);
		return -EFAULT;
	}

	if (of_address_to_resource(cmn, 0, &res_nss_misc_reset) != 0) {
		pr_err("%px: of_address_to_resource() return error for nss_misc_reset\n", nss_dev);
		of_node_put(cmn);
		return -EFAULT;
	}

	of_node_put(cmn);

	nss_misc_reset = ioremap_nocache(res_nss_misc_reset.start, resource_size(&res_nss_misc_reset));
	if (!nss_misc_reset) {
		pr_err("%px: ioremap fail for nss_misc_reset\n", nss_dev);
		return -EFAULT;
	}

	nss_top_main.nss_hal_common_init_done = true;
	nss_info("nss_hal_common_reset Done\n");

	return 0;
}

/*
 * __nss_hal_clock_configure()
 */
static int __nss_hal_clock_configure(struct nss_ctx_instance *nss_ctx, struct platform_device *nss_dev, struct nss_platform_data *npd)
{
	uint32_t i;
	uint32_t default_freq;

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_AHB_CLK, 100000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_AXI_CLK, 533000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_INTR_AHB_CLK, 200000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_NC_AXI_CLK, 353000000)) {
		return -EFAULT;
	}

	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_UTCM_CLK, 353000000)) {
		return -EFAULT;
	}

	nss_info("%p: Core %d clocks initialized successfully.\n", nss_dev, nss_ctx->id);

	/*
	 * For IPQ95xx, any rate above 1497 is Turbo Voltage
	 * Temporary set the voltage to turbo till we start scaling frequenices.
	 * This is to ensure probing is safe and autoscaling will correct the voltage.
	 * String "npu" means it searches for "npu-supply" in NSS node.
	 * TODO: Turbo voltage is set to the actual voltage. It should be set to voltage
	 * corner once the CPR is ready.
	 */
	if (!nss_ctx->id) {
		npu_reg = devm_regulator_get(&nss_dev->dev, "npu");
		if (IS_ERR(npu_reg)) {
			return PTR_ERR(npu_reg);
		}
		if (regulator_enable(npu_reg)) {
			return -EFAULT;
		}
		regulator_set_voltage(npu_reg, TURBO_VOLTAGE, TURBO_VOLTAGE);
	}

	/*
	 * No entries, then just load default
	 */
	if ((nss_runtime_samples.freq_scale[NSS_FREQ_LOW_SCALE].frequency == 0) ||
		(nss_runtime_samples.freq_scale[NSS_FREQ_MID_SCALE].frequency == 0) ||
		(nss_runtime_samples.freq_scale[NSS_FREQ_HIGH_SCALE].frequency == 0)) {
		nss_runtime_samples.freq_scale[NSS_FREQ_LOW_SCALE].frequency = NSS_FREQ_748;
		nss_runtime_samples.freq_scale[NSS_FREQ_MID_SCALE].frequency = NSS_FREQ_1497;
		nss_runtime_samples.freq_scale[NSS_FREQ_HIGH_SCALE].frequency = NSS_FREQ_1689;
		nss_info("Running default frequencies\n");
	}

	default_freq = nss_runtime_samples.freq_scale[NSS_FREQ_HIGH_SCALE].frequency;
	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CORE_CLK, default_freq)) {
		default_freq = nss_runtime_samples.freq_scale[NSS_FREQ_MID_SCALE].frequency;
		if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CORE_CLK, default_freq)) {
			return -EFAULT;
		}
	}

	nss_info("%p: Set UBI core %d frequency to frequency %u.\n", nss_dev, nss_ctx->id, default_freq);

	/*
	 * Setup ranges, test frequency, and display.
	 */
	for (i = 0; i < NSS_FREQ_MAX_SCALE; i++) {
		switch (nss_runtime_samples.freq_scale[i].frequency) {
		case NSS_FREQ_748:
			nss_runtime_samples.freq_scale[i].minimum = NSS_FREQ_748_MIN;
			nss_runtime_samples.freq_scale[i].maximum = NSS_FREQ_748_MAX;
			break;

		case NSS_FREQ_1497:
			nss_runtime_samples.freq_scale[i].minimum = NSS_FREQ_1497_MIN;
			nss_runtime_samples.freq_scale[i].maximum = NSS_FREQ_1497_MAX;
			break;

		case NSS_FREQ_1689:
			nss_runtime_samples.freq_scale[i].minimum = NSS_FREQ_1689_MIN;
			nss_runtime_samples.freq_scale[i].maximum = NSS_FREQ_1689_MAX;
			break;

		default:
			nss_info_always("%px: Frequency not found %d\n", nss_ctx, nss_runtime_samples.freq_scale[i].frequency);
			return -EFAULT;
		}

		/*
		 * Test the frequency, if fail, then default to safe frequency and abort
		 */
		if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CORE_CLK, nss_runtime_samples.freq_scale[i].frequency)) {
			return -EFAULT;
		}
	}

	nss_info_always("Supported Frequencies - ");
	for (i = 0; i < NSS_FREQ_MAX_SCALE; i++) {
		switch (nss_runtime_samples.freq_scale[i].frequency) {
		case NSS_FREQ_748:
			nss_info_always("748 MHz ");
			break;

		case NSS_FREQ_1497:
			nss_info_always("1.497 GHz ");
			break;

		case NSS_FREQ_1689:
			nss_info_always("1.689 GHz ");
			break;

		default:
			nss_info_always("%px: Error\nNo Table/Invalid Frequency Found\n", nss_ctx);
			return -EFAULT;
		}
	}
	nss_info_always("\n");

	nss_hal_set_core_clk(nss_dev, NSS_CORE_0);

	default_freq = nss_runtime_samples.freq_scale[NSS_FREQ_MID_SCALE].frequency;
	if (nss_hal_clock_set_and_enable(&nss_dev->dev, NSS_CORE_CLK, default_freq)) {
		return -EFAULT;
	}
	nss_info_always("%p: Set UBI core %d frequency to frequency %u.\n", nss_dev, nss_ctx->id, default_freq);

	return 0;
}

/*
 * __nss_hal_read_interrupt_cause()
 */
static void __nss_hal_read_interrupt_cause(struct nss_ctx_instance *nss_ctx, uint32_t shift_factor, uint32_t *cause)
{
}

/*
 * __nss_hal_clear_interrupt_cause()
 */
static void __nss_hal_clear_interrupt_cause(struct nss_ctx_instance *nss_ctx, uint32_t shift_factor, uint32_t cause)
{
}

/*
 * __nss_hal_disable_interrupt()
 */
static void __nss_hal_disable_interrupt(struct nss_ctx_instance *nss_ctx, uint32_t shift_factor, uint32_t cause)
{
}

/*
 * __nss_hal_enable_interrupt()
 */
static void __nss_hal_enable_interrupt(struct nss_ctx_instance *nss_ctx, uint32_t shift_factor, uint32_t cause)
{

}

/*
 * __nss_hal_send_interrupt()
 */
static void __nss_hal_send_interrupt(struct nss_ctx_instance *nss_ctx, uint32_t type)
{
	/*
	 * Check if core and type is Valid
	 */
	nss_assert(nss_ctx->id < nss_top_main.num_nss);
	nss_assert(type < NSS_H2N_INTR_TYPE_MAX);

	nss_write_32(nss_ctx->qgic_map, NSS_QGIC_IPC_REG_OFFSET, intr_cause[nss_ctx->id][type]);
}

/*
 * __nss_hal_request_irq()
 */
static int __nss_hal_request_irq(struct nss_ctx_instance *nss_ctx, struct nss_platform_data *npd, int irq_num)
{
	struct int_ctx_instance *int_ctx = &nss_ctx->int_ctx[irq_num];
	uint32_t cause, napi_wgt;
	int err = -1, irq = npd->irq[irq_num];
	int (*napi_poll_cb)(struct napi_struct *, int) = NULL;
	const char *irq_name;

	irq_set_status_flags(irq, IRQ_DISABLE_UNLAZY);

	switch (irq_num) {
	case NSS_HAL_N2H_INTR_PURPOSE_EMPTY_BUFFER_SOS:
		napi_poll_cb = nss_core_handle_napi_non_queue;
		napi_wgt = NSS_EMPTY_BUFFER_SOS_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_EMPTY_BUFFERS_SOS;
		irq_name = NSS_IRQ_NAME_EMPTY_BUF_SOS;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_EMPTY_BUFFER_QUEUE:
		napi_poll_cb = nss_core_handle_napi_queue;
		napi_wgt = NSS_EMPTY_BUFFER_RETURN_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_EMPTY_BUFFER_QUEUE;
		irq_name = NSS_IRQ_NAME_EMPTY_BUF_QUEUE;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_TX_UNBLOCKED:
		napi_poll_cb = nss_core_handle_napi_non_queue;
		napi_wgt = NSS_TX_UNBLOCKED_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_TX_UNBLOCKED;
		irq_name = NSS_IRQ_NAME_TX_UNBLOCK;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_0:
		napi_poll_cb = nss_core_handle_napi_queue;
		napi_wgt = NSS_DATA_COMMAND_BUFFER_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_DATA_QUEUE_0;
		irq_name = NSS_IRQ_NAME_QUEUE0;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_1:
		napi_poll_cb = nss_core_handle_napi_queue;
		napi_wgt = NSS_DATA_COMMAND_BUFFER_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_DATA_QUEUE_1;
		irq_name = NSS_IRQ_NAME_QUEUE1;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_2:
		napi_poll_cb = nss_core_handle_napi_queue;
		napi_wgt = NSS_DATA_COMMAND_BUFFER_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_DATA_QUEUE_2;
		irq_name = NSS_IRQ_NAME_QUEUE2;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_DATA_QUEUE_3:
		napi_poll_cb = nss_core_handle_napi_queue;
		napi_wgt = NSS_DATA_COMMAND_BUFFER_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_DATA_QUEUE_3;
		irq_name = NSS_IRQ_NAME_QUEUE3;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_COREDUMP_COMPLETE:
		napi_poll_cb = nss_core_handle_napi_emergency;
		napi_wgt = NSS_DATA_COMMAND_BUFFER_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_COREDUMP_COMPLETE;
		irq_name = NSS_IRQ_NAME_COREDUMP_COMPLETE;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_PAGED_EMPTY_BUFFER_SOS:
		napi_poll_cb = nss_core_handle_napi_non_queue;
		napi_wgt = NSS_EMPTY_BUFFER_SOS_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_PAGED_EMPTY_BUFFERS_SOS;
		irq_name = NSS_IRQ_NAME_PAGED_EMPTY_BUF_SOS;
		break;

	case NSS_HAL_N2H_INTR_PURPOSE_PROFILE_DMA:
		napi_poll_cb = nss_core_handle_napi_sdma;
		napi_wgt = NSS_DATA_COMMAND_BUFFER_PROCESSING_WEIGHT;
		cause = NSS_N2H_INTR_PROFILE_DMA;
		irq_name = NSS_IRQ_NAME_PROFILE_DMA;
		break;

	default:
		nss_warning("%px: nss%d: unsupported irq# %d\n", nss_ctx, nss_ctx->id, irq_num);
		return err;
	}

	netif_napi_add(&nss_ctx->napi_ndev, &int_ctx->napi, napi_poll_cb, napi_wgt);
	int_ctx->cause = cause;
	err = request_irq(irq, nss_hal_handle_irq, 0, irq_name, int_ctx);
	if (err) {
		nss_warning("%px: nss%d: request_irq failed for irq# %d\n", nss_ctx, nss_ctx->id, irq_num);
		return err;
	}
	int_ctx->irq = irq;
	return 0;
}

/*
 * __nss_hal_init_imem
 */
void __nss_hal_init_imem(struct nss_ctx_instance *nss_ctx)
{
	struct nss_meminfo_ctx *mem_ctx = &nss_ctx->meminfo_ctx;

	mem_ctx->imem_head = NSS_IMEM_START + NSS_IMEM_SIZE * nss_ctx->id;
	mem_ctx->imem_end = mem_ctx->imem_head + NSS_IMEM_SIZE;
	mem_ctx->imem_tail = mem_ctx->imem_head;

	nss_info("%px: IMEM init: head: 0x%x end: 0x%x tail: 0x%x\n", nss_ctx,
					mem_ctx->imem_head, mem_ctx->imem_end, mem_ctx->imem_tail);
}

/*
 * __nss_hal_init_utcm_shared
 */
bool __nss_hal_init_utcm_shared(struct nss_ctx_instance *nss_ctx, uint32_t *meminfo_start)
{
	return true;
}

/*
 * nss_hal_ipq95xx_ops
 */
struct nss_hal_ops nss_hal_ipq95xx_ops = {
	.common_reset = __nss_hal_common_reset,
	.core_reset = __nss_hal_core_reset,
	.clock_configure = __nss_hal_clock_configure,
	.firmware_load = nss_hal_firmware_load,
	.debug_enable = __nss_hal_debug_enable,
	.of_get_pdata = __nss_hal_of_get_pdata,
	.request_irq = __nss_hal_request_irq,
	.send_interrupt = __nss_hal_send_interrupt,
	.enable_interrupt = __nss_hal_enable_interrupt,
	.disable_interrupt = __nss_hal_disable_interrupt,
	.clear_interrupt_cause = __nss_hal_clear_interrupt_cause,
	.read_interrupt_cause = __nss_hal_read_interrupt_cause,
	.init_imem = __nss_hal_init_imem,
	.init_utcm_shared = __nss_hal_init_utcm_shared,
};
