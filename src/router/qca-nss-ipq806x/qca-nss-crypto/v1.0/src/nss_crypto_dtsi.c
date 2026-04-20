/* Copyright (c) 2013, 2015-2017, 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/memory.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/reset.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <nss_crypto_hlos.h>
#include <nss_api_if.h>
#include <nss_crypto.h>
#include <nss_crypto_if.h>
#include <nss_crypto_hw.h>
#include <nss_crypto_ctrl.h>
#include <nss_crypto_dbg.h>
#include <nss_crypto_debugfs.h>

/* Poll time in ms */
#define CRYPTO_DELAYED_INIT_TIME	100

/*
 * Crypto Clock Freq Table
 * +------------------+-------------+--------------+
 * | Clock            | Nominal     |  Turbo       |
 * +------------------+-------------+--------------+
 * | NSS_CE5_CORE_CLK | 150 Mhz     |  213.2 Mhz   |
 * +------------------+-------------+--------------+
 * | NSS_CE5_AHB_CLK  | 160 Mhz     |  213.2 Mhz   |
 * +------------------+-------------+--------------+
 * | NSS_CE5_AXI_CLK  | 160 Mhz     |  213.2 Mhz   |
 * +------------------+-------------+--------------+
 *
 */
#define NSS_CRYPTO_CLOCK_TURBO 213200000	/* Turbo Freq for Core, AXI and AHB */
#define NSS_CRYPTO_CLOCK_CORE_NOMINAL 150000000	/* Nominal freq for Core Clock */
#define NSS_CRYPTO_CLOCK_AUX_NOMINAL 160000000	/* Nominal freq for AXI and AHB */

/*
 * Crypto resource index in device tree
 */
enum nss_crypto_dt_res {
	NSS_CRYPTO_DT_CRYPTO_RES = 0,
	NSS_CRYPTO_DT_BAM_RES = 1,
	NSS_CRYPTO_DT_MAX_RES
};

extern struct nss_crypto_ctrl gbl_crypto_ctrl;
extern struct nss_ctx_instance *nss_drv_hdl;

static int eng_count;

int nss_crypto_engine_init(uint32_t eng_count);
void nss_crypto_init(void);
void nss_crypto_user_attach_all(struct nss_crypto_ctrl *ctrl);

/*
 * nss_crypto_bam_init()
 *	initialize the BAM for the given engine
 */
static void nss_crypto_bam_init(uint8_t *bam_iobase)
{
	uint32_t cfg_bits;
	uint32_t ctrl_reg;

	ctrl_reg = ioread32(bam_iobase + CRYPTO_BAM_CTRL);

	ctrl_reg |= CRYPTO_BAM_CTRL_SW_RST;
	iowrite32(ctrl_reg, bam_iobase + CRYPTO_BAM_CTRL);

	ctrl_reg &= ~CRYPTO_BAM_CTRL_SW_RST;
	iowrite32(ctrl_reg, bam_iobase + CRYPTO_BAM_CTRL);

	ctrl_reg |= CRYPTO_BAM_CTRL_BAM_EN;
	iowrite32(ctrl_reg, bam_iobase + CRYPTO_BAM_CTRL);

	iowrite32(CRYPTO_BAM_DESC_CNT_TRSHLD_VAL, bam_iobase +  CRYPTO_BAM_DESC_CNT_TRSHLD);

	/* disabling this is recommended from H/W specification*/
	cfg_bits = ~((uint32_t)CRYPTO_BAM_CNFG_BITS_BAM_FULL_PIPE);
	iowrite32(cfg_bits, bam_iobase + CRYPTO_BAM_CNFG_BITS);
}

/*
 * nss_crypto_pm_event_cb()
 * 	Crypto PM event callback
 */
bool nss_crypto_pm_event_cb(void *app_data, bool turbo, bool auto_scale)
{
	struct nss_crypto_ctrl *ctrl = (struct nss_crypto_ctrl *)app_data;
	struct nss_crypto_clock *clock = ctrl->clocks;
	bool session;
	int count;

	BUG_ON(!clock);
	count = ctrl->max_clocks;

	spin_lock_bh(&ctrl->lock); /* index lock*/
	session = !bitmap_empty(ctrl->idx_bitmap, NSS_CRYPTO_MAX_IDXS);
	spin_unlock_bh(&ctrl->lock); /* index unlock */

	/*
	 * if, there is no-sessions & the system is not in
	 * turbo then crypto doesn't need to scale. Once,
	 * the system has moved to turbo then we cannot
	 * roll back
	 */
	if (!session || !turbo)
		return false;

	/*
	 * notify NSS to switch NSS subsystem to turbo
	 */
	for (; count--; clock++) {
		if (clk_set_rate(clock->clk, clock->turbo_freq)) {
			nss_crypto_err("Error in setting clock(%d) to %d\n", count, clock->turbo_freq);
			continue;
		}
	}

	/*
	 * we will remove the callback at this point to ensure that crypto doesn't
	 * scale down anymore
	 */
	nss_crypto_pm_notify_unregister();

	atomic_set(&ctrl->perf_level, NSS_PM_PERF_LEVEL_TURBO);
	complete(&ctrl->perf_complete);

	return true;
}

/*
 * nss_crypto_clock_init()
 * 	initialize crypto clock
 */
static void nss_crypto_clock_init(struct platform_device *pdev, struct device_node *np)
{
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;
	struct nss_crypto_clock *clock;
	const char *clk_string;
	size_t clock_array_sz;
	int count, i;

	count = of_property_count_strings(np, "clock-names");
	if (count < 0) {
		nss_crypto_info("crypto clock instance not found\n");
		return;
	}

	clock_array_sz = sizeof(struct nss_crypto_clock) * count;

	clock = kzalloc(clock_array_sz, GFP_KERNEL);
	if (!clock) {
		nss_crypto_err("Error in allocating clock array (size:%d)\n", clock_array_sz);
		return;
	}

	ctrl->max_clocks = count;
	ctrl->clocks = clock;

	for (i = 0; i < count; i++, clock++) {
		/*
		 * parse clock names from DTSI
		 */
		of_property_read_string_index(np, "clock-names", i, &clk_string);

		clock->clk = devm_clk_get(&pdev->dev, clk_string);
		BUG_ON(!clock->clk);

		nss_crypto_info("clock mapping clock(%d) --> %s\n", i, clk_string);

		clk_prepare_enable(clock->clk);

		clock->nominal_freq = NSS_CRYPTO_CLOCK_AUX_NOMINAL;
		clock->turbo_freq = NSS_CRYPTO_CLOCK_TURBO;

		/*
		 * switch the core clock for nominal frequency
		 */
		if (unlikely(!strncmp(clk_string, "ce5_core", sizeof("ce5_core")))) {
			clock->nominal_freq = NSS_CRYPTO_CLOCK_CORE_NOMINAL;
		}
	}

	atomic_set(&ctrl->perf_level, NSS_PM_PERF_LEVEL_NOMINAL);
	nss_crypto_pm_notify_register(nss_crypto_pm_event_cb, ctrl);
}

/*
 * nss_crypto_probe()
 *	probe routine called per engine from MACH-MSM
 */
static int nss_crypto_probe(struct platform_device *pdev)
{
	struct reset_control *rst_ctl __attribute__((unused));
	struct nss_crypto_ctrl_eng *eng_ptr;
	struct nss_crypto_ctrl_eng *e_ctrl;
	struct resource crypto_res = {0};
	struct resource bam_res = {0};
	struct device_node *np;
	uint32_t bam_ee = 0;
	size_t old_sz;
	size_t new_sz;

	if (nss_crypto_check_state(&gbl_crypto_ctrl, NSS_CRYPTO_STATE_NOT_READY)) {
		nss_crypto_info_always("exiting probe due to previous error\n");
		return -ENOMEM;
	}

	nss_crypto_info_always("probing engine - %d\n", eng_count);
	nss_crypto_assert(eng_count < NSS_CRYPTO_MAX_ENGINES);

	if (!pdev->dev.of_node) {
		nss_crypto_info_always("%sError Accessing dev node\n", __func__);
		return -ENODEV;
	}

	/* crypto engine resources */
	nss_crypto_info_always("Device Tree node found\n");
	np = of_node_get(pdev->dev.of_node);

	/*
	 * initialize crypto clock
	 */
	nss_crypto_clock_init(pdev, np);

#if defined CONFIG_RESET_CONTROLLER
	/*
	 * Reset Crypto AHB, when first crypto engine is probed
	 */
	rst_ctl = devm_reset_control_get(&pdev->dev, "rst_ahb");
	if (!IS_ERR(rst_ctl) && (reset_control_deassert(rst_ctl) > 0)) {
		nss_crypto_info_always("Crypto AHB pulled out-of-reset\n");
	}

	/*
	 * Reset the Crypto Engine
	 */
	rst_ctl = devm_reset_control_get(&pdev->dev, "rst_eng");
	if (!IS_ERR(rst_ctl) && (reset_control_deassert(rst_ctl) > 0)) {
		nss_crypto_info_always("Crypto Engine (%d) pulled out-of-reset\n", eng_count);
	}
#endif

	/*
	 * Crypto Registers
	 */
	if (of_address_to_resource(np, NSS_CRYPTO_DT_CRYPTO_RES, &crypto_res) != 0) {
		nss_crypto_err("Error in retreiving crypto_base\n");
		return -EINVAL;
	}

	/*
	 * BAM Registers
	 */
	if (of_address_to_resource(np, NSS_CRYPTO_DT_BAM_RES, &bam_res) != 0) {
		nss_crypto_err("Error in retreiving bam_base\n");
		return -EINVAL;
	}

	/*
	 * BAM Execution Environment
	 */
	if (of_property_read_u32(np, "qcom,ee", &bam_ee)) {
		nss_crypto_err("Error retreiving crypto EE for engine(%d)\n", eng_count);
		return -EINVAL;
	}

	eng_ptr = gbl_crypto_ctrl.eng;
	xchg(&gbl_crypto_ctrl.eng, NULL);
	old_sz = (gbl_crypto_ctrl.num_eng * sizeof(struct nss_crypto_ctrl_eng));
	new_sz = old_sz + sizeof(struct nss_crypto_ctrl_eng);

	eng_ptr = nss_crypto_mem_realloc(eng_ptr, old_sz, new_sz);
	if (!eng_ptr) {
		return -ENOMEM;
	}

	xchg(&gbl_crypto_ctrl.eng, eng_ptr);

	e_ctrl = &gbl_crypto_ctrl.eng[eng_count];
	e_ctrl->dev = &pdev->dev;

	e_ctrl->cmd_base = crypto_res.start;
	e_ctrl->crypto_base = ioremap(e_ctrl->cmd_base, resource_size(&crypto_res));
	nss_crypto_assert(e_ctrl->crypto_base);

	e_ctrl->bam_pbase = bam_res.start;
	e_ctrl->bam_base = ioremap(e_ctrl->bam_pbase, resource_size(&bam_res));
	nss_crypto_assert(e_ctrl->bam_base);

	e_ctrl->bam_ee = bam_ee;

	/*
	 * Link address of engine ctrl
	 */
	platform_set_drvdata(pdev, e_ctrl);

	/*
	 * intialize the BAM and the engine
	 */
	nss_crypto_bam_init(e_ctrl->bam_base);

	if (nss_crypto_engine_init(eng_count) != NSS_CRYPTO_STATUS_OK) {
		nss_crypto_info_always("Error in Engine Init\n");
		nss_crypto_reset_state(&gbl_crypto_ctrl);
		return -ENOMEM;
	}

	eng_count++;
	gbl_crypto_ctrl.num_eng = eng_count;

	return 0;
}

/*
 * nss_crypto_remove()
 *	remove the crypto engine and deregister everything
 */
static int nss_crypto_remove(struct platform_device *pdev)
{
	struct nss_crypto_ctrl_eng *e_ctrl;
	struct nss_crypto_ctrl *ctrl;

	e_ctrl = platform_get_drvdata(pdev);

	/*
	 * free clock references if any
	 */
	ctrl = container_of(&e_ctrl, struct nss_crypto_ctrl, eng);
	if (ctrl->clocks) {
		kfree(ctrl->clocks);
	}

	return 0;
};

static struct of_device_id nss_crypto_dt_ids[] = {
	{ .compatible =  "qcom,nss-crypto" },
	{},
};
MODULE_DEVICE_TABLE(of, nss_crypto_dt_ids);

/*
 * platform device instance
 */
static struct platform_driver nss_crypto_drv = {
	.probe  	= nss_crypto_probe,
	.remove 	= nss_crypto_remove,
	.driver 	= {
		.owner	= THIS_MODULE,
		.name	= "nss-crypto",
		.of_match_table = of_match_ptr(nss_crypto_dt_ids),
	},
};

/*
 * nss_crypto_delayed_init
 * 	delayed sequence to initialize crypto after NSS FW is initialized
 */
void nss_crypto_delayed_init(struct work_struct *work)
{
	struct nss_crypto_ctrl *ctrl;
	uint32_t status = 0;
	uint32_t i = 0;

	ctrl = container_of(to_delayed_work(work), struct nss_crypto_ctrl, crypto_work);

	/*
	 * check if NSS FW is initialized
	 */
	if (nss_get_state(nss_drv_hdl) != NSS_STATE_INITIALIZED) {
		schedule_delayed_work(&ctrl->crypto_work, msecs_to_jiffies(CRYPTO_DELAYED_INIT_TIME));
		return;
	}

	nss_crypto_info_always("NSS Firmware initialized\n");

	/*
	 * reserve the index if certain pipe pairs are locked out for
	 * trust zone use
	 */
	memset(ctrl->idx_bitmap, 0, sizeof(ctrl->idx_bitmap));

	status = platform_driver_register(&nss_crypto_drv);
	if (status) {
		nss_crypto_err("unable to register the driver : %d\n", status);
		return;
	}

	/*
	 * If crypto probe has failed, no need for further initialization
	 */
	if (nss_crypto_check_state(ctrl, NSS_CRYPTO_STATE_NOT_READY)) {
		nss_crypto_warn("%px:NSS Crypto probe failed, num_eng (%d)\n", ctrl, gbl_crypto_ctrl.num_eng);
		return;
	}

	nss_crypto_set_state(ctrl, NSS_CRYPTO_STATE_INITIALIZED);
	nss_crypto_user_attach_all(ctrl);

	/*
	 * Initialize the engine stats
	 */
	for (i = 0; i < ctrl->num_eng ; i++)
		nss_crypto_debugfs_add_engine(ctrl, i);
}

/*
 * nss_crypto_module_init()
 *	module init for crypto driver
 */
static int __init nss_crypto_module_init(void)
{
	struct device_node *np;

	nss_crypto_info_always("module loaded (platform - IPQ806x, build - %s)\n", NSS_CRYPTO_BUILD_ID);

	nss_crypto_reset_state(&gbl_crypto_ctrl);

	np = of_find_compatible_node(NULL, NULL, "qcom,nss-crypto");
	if (!np) {
		nss_crypto_info_always("qca-nss-crypto.ko is loaded for symbol link\n");
		return 0;
	}

	nss_crypto_init();

	nss_crypto_set_state(&gbl_crypto_ctrl, NSS_CRYPTO_STATE_READY);

	nss_crypto_info_always("Register with NSS driver-\n");

	INIT_DELAYED_WORK(&gbl_crypto_ctrl.crypto_work, nss_crypto_delayed_init);

	schedule_delayed_work(&gbl_crypto_ctrl.crypto_work, msecs_to_jiffies(CRYPTO_DELAYED_INIT_TIME));

	return 0;
}

/*
 * nss_crypto_module_exit()
 *	module exit for crypto driver
 */
static void __exit nss_crypto_module_exit(void)
{
	nss_crypto_info("module unloaded (IPQ806x)\n");

	platform_driver_unregister(&nss_crypto_drv);
}

module_init(nss_crypto_module_init);
module_exit(nss_crypto_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("QCA NSS Crypto driver");
