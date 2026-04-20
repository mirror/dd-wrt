/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>

#include "ppe_ds_stats.h"
#include "ppe_ds.h"

/*
 * PPE-DS ring processing mode selection parameter.
 * The default mode PPE_DS_INTR_MODE (0) will use interrupt mode
 * and PPE_DS_POLL_MODE (1) will use polling mode.
 *
 */
unsigned int polling_for_idx_update = PPE_DS_INTR_MODE;
module_param(polling_for_idx_update,
		uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(polling_for_idx_update, "Enable/Disable PPE DS poll mode");

/*
 * PPE-DS timer frequency for polling function.
 * The final timer frequency will be in nanosecond and calculated as
 * NSEC_PER_SEC/idx_mgmt_freq
 */
unsigned int idx_mgmt_freq = 32768;
module_param(idx_mgmt_freq, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(idx_mgmt_freq, "Idx Management hrtimer freq");

unsigned int cpu_mask_2g = 0x2;
module_param(cpu_mask_2g, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cpu_mask_2g, "CPU mask for the 2G radio VAP");

unsigned int cpu_mask_5g_lo = 0x1;
module_param(cpu_mask_5g_lo, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cpu_mask_5g_lo, "CPU mask for the 5G low radio VAP");

unsigned int cpu_mask_5g_hi = 0x2;
module_param(cpu_mask_5g_hi, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cpu_mask_5g_hi, "CPU mask for the 5G high radio VAP");

unsigned int cpu_mask_6g = 0x1;
module_param(cpu_mask_6g, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cpu_mask_6g, "CPU mask for the 6G radio VAP");

unsigned int ppe2tcl_rxfill_num_desc = 2048;
module_param(ppe2tcl_rxfill_num_desc, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(ppe2tcl_rxfill_num_desc, "PPE2TCL Rxfill ring descriptor count");

unsigned int reo2ppe_txcmpl_num_desc = 16384;
module_param(reo2ppe_txcmpl_num_desc, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(reo2ppe_txcmpl_num_desc, "REO2PPE Tx complete ring descriptor count");

#if defined(NSS_PPE_IPQ53XX)
unsigned int rxfill_low_threshold = 1024;
#else
unsigned int rxfill_low_threshold = 256;
#endif
module_param(rxfill_low_threshold, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(rxfill_low_threshold, "RxFill low threshold value");

unsigned int txcmpl_budget = 256;
module_param(txcmpl_budget, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(txcmpl_budget, "PPE Tx complete budget");

struct ppe_ds_node_config ppe_ds_node_cfg[PPE_DS_MAX_NODE];

/*
 * ppe_ds_module_init()
 *	Module init for PPE-DS driver
 */
static int __init ppe_ds_module_init(void)
{
	uint32_t i;

	for (i = 0; i < PPE_DS_MAX_NODE; i++) {
		rwlock_init(&ppe_ds_node_cfg[i].lock);
		ppe_ds_node_cfg[i].node_state = PPE_DS_NODE_STATE_AVAIL;
	}
	ppe_ds_node_stats_debugfs_init();
	ppe_ds_info("PPE-DS module loaded successfully %d", idx_mgmt_freq);
	return 0;
}
module_init(ppe_ds_module_init);

/*
 * ppe_ds_module_exit()
 *	Module exit for PPE-DS driver
 */
static void __exit ppe_ds_module_exit(void)
{
	uint32_t i;

	ppe_ds_node_stats_debugfs_exit();
	for (i = 0; i < PPE_DS_MAX_NODE; i++) {
		ppe_ds_node_cfg[i].node_state = PPE_DS_NODE_STATE_AVAIL;
	}
	ppe_ds_info("PPE-DS module unloaded successfully %d", idx_mgmt_freq);
}
module_exit(ppe_ds_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("QCA PPE DS driver");
