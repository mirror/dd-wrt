/*
 * Copyright (c) 2013,2015-2017, The Linux Foundation. All rights reserved.
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
#include <mach/msm_iomap.h>
#include <mach/msm_nss_crypto.h>
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

#define REG(off)	(MSM_CLK_CTL_BASE + (off))
#define REG_GCC(off)	(MSM_APCS_GCC_BASE + (off))

#define CRYPTO_RESET_ENG(n)	REG(0x3E00 + (n * 0x4))
#define CRYPTO_RESET_AHB	REG(0x3E10)

#define CRYPTO_RESET_ENG_0	REG(0x3E00)
#define CRYPTO_RESET_ENG_1	REG(0x3E04)
#define CRYPTO_RESET_ENG_2	REG(0x3E08)
#define CRYPTO_RESET_ENG_3	REG(0x3E0C)

/* Poll time in ms */
#define CRYPTO_DELAYED_INIT_TIME	100

extern struct nss_crypto_ctrl gbl_crypto_ctrl;
extern struct nss_ctx_instance *nss_drv_hdl;

static int eng_count;

int nss_crypto_engine_init(uint32_t eng_count);
void nss_crypto_init(void);
void nss_crypto_user_attach_all(struct nss_crypto_ctrl *ctrl);

/*
 * nss_crypto_bam_init()
 * 	initialize the BAM for the given engine
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
 * nss_crypto_probe()
 * 	probe routine called per engine from MACH-MSM
 */
static int nss_crypto_probe(struct platform_device *pdev)
{
	struct nss_crypto_ctrl_eng *e_ctrl;
	struct nss_crypto_platform_data *res;
	struct nss_crypto_ctrl_eng *eng_ptr;
	int status = 0;
	size_t old_sz;
	size_t new_sz;

	if (nss_crypto_check_state(&gbl_crypto_ctrl, NSS_CRYPTO_STATE_NOT_READY)) {
		nss_crypto_info_always("exiting probe due to previous error\n");
		return -ENOMEM;
	}

	nss_crypto_info_always("probing engine - %d\n", eng_count);
	nss_crypto_assert(eng_count < NSS_CRYPTO_MAX_ENGINES);

	eng_ptr = gbl_crypto_ctrl.eng;

	old_sz = (gbl_crypto_ctrl.num_eng * sizeof(struct nss_crypto_ctrl_eng));
	new_sz = old_sz + sizeof(struct nss_crypto_ctrl_eng);

	eng_ptr = nss_crypto_mem_realloc(eng_ptr, old_sz, new_sz);
	if (!eng_ptr) {
		return -ENOMEM;
	}

	gbl_crypto_ctrl.eng = eng_ptr;

	e_ctrl = &gbl_crypto_ctrl.eng[eng_count];
	e_ctrl->dev = &pdev->dev;

	/* crypto engine resources */
	res = dev_get_platdata(e_ctrl->dev);
	nss_crypto_assert(res);

	e_ctrl->bam_ee = res->bam_ee;

	e_ctrl->cmd_base = res->crypto_pbase;
	e_ctrl->crypto_base = ioremap(res->crypto_pbase, res->crypto_pbase_sz);
	nss_crypto_assert(e_ctrl->crypto_base);

	e_ctrl->bam_pbase = res->bam_pbase;
	e_ctrl->bam_base = ioremap(res->bam_pbase, res->bam_pbase_sz);
	nss_crypto_assert(e_ctrl->bam_base);

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

	return status;
}

/*
 * nss_crypto_remove()
 * 	remove the crypto engine and deregister everything
 */
static int nss_crypto_remove(struct platform_device *pdev)
{
	struct nss_crypto_ctrl_eng *ctrl;

	ctrl = platform_get_drvdata(pdev);

	/**
	 * XXX: pipe deinit goes here
	 */
	return 0;
};

/*
 * platform device instance
 */
static struct platform_driver nss_crypto_drv = {
	.probe  	= nss_crypto_probe,
	.remove 	= nss_crypto_remove,
	.driver 	= {
		.owner  = THIS_MODULE,
		.name   = "nss-crypto",
	},
};

/*
 * nss_crypto_module_exit()
 * 	module exit for crypto driver
 */
static void __exit nss_crypto_module_exit(void)
{
	nss_crypto_info("module unloaded (IPQ806x)\n");

	platform_driver_unregister(&nss_crypto_drv);
}

/*
 * nss_crypto_delayed_init()
 *	Delayed worker function for crypto probe
 */
void nss_crypto_delayed_init(struct work_struct *work)
{
	struct nss_crypto_ctrl *ctrl;
	uint32_t status = 0;
	uint32_t i = 0;

	ctrl = container_of(to_delayed_work(work), struct nss_crypto_ctrl, crypto_work);

	/*
	 * if NSS FW is not initialized at this point, schedule a delayed work
	 * thread and wait for NSS FW to be up before doing a crypto probe
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
		nss_crypto_warn("%p:NSS Crypto probe failed, num_eng (%d)\n", ctrl, gbl_crypto_ctrl.num_eng);
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
 * 	module init for crypto driver
 */
static int __init nss_crypto_module_init(void)
{

	nss_crypto_info_always("module loaded (platform - IPQ806x, %s)\n", NSS_CRYPTO_BUILD_ID);

	nss_crypto_reset_state(&gbl_crypto_ctrl);

	/*
	 * bring the crypto out of reset
	 */
	iowrite32(0, CRYPTO_RESET_ENG(0));
	iowrite32(0, CRYPTO_RESET_ENG(1));
	iowrite32(0, CRYPTO_RESET_ENG(2));
	iowrite32(0, CRYPTO_RESET_ENG(3));

	iowrite32(0, CRYPTO_RESET_AHB);

	nss_crypto_init();

	nss_crypto_set_state(&gbl_crypto_ctrl, NSS_CRYPTO_STATE_READY);

	nss_crypto_info_always("Register with NSS driver-\n");

	INIT_DELAYED_WORK(&gbl_crypto_ctrl.crypto_work, nss_crypto_delayed_init);

	schedule_delayed_work(&gbl_crypto_ctrl.crypto_work, msecs_to_jiffies(CRYPTO_DELAYED_INIT_TIME));

	return 0;
}

module_init(nss_crypto_module_init);
module_exit(nss_crypto_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("QCA NSS Crypto driver");
