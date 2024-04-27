/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include "edma.h"
#include "edma_regs.h"
#include "edma_debug.h"

/*
 * edma_misc_stats_alloc()
 *	API to allocate memory for per-CPU miscellaneous stats
 */
int32_t edma_misc_stats_alloc()
{
	uint32_t i;

	edma_gbl_ctx.misc_stats = alloc_percpu(struct edma_misc_stats);
	if (!edma_gbl_ctx.misc_stats) {
		edma_err("Unable to allocate miscellaneous percpu stats\n");
		return -ENOMEM;
	}

	for_each_possible_cpu(i) {
		struct edma_misc_stats *stats;

		stats = per_cpu_ptr(edma_gbl_ctx.misc_stats, i);
		u64_stats_init(&stats->syncp);
	}

	return 0;
}

/*
 * edma_misc_stats_free()
 *	API to free memory of per-CPU miscellaneous stats
 */
void edma_misc_stats_free()
{
	if (edma_gbl_ctx.misc_stats) {
		free_percpu(edma_gbl_ctx.misc_stats);
		edma_gbl_ctx.misc_stats = NULL;
	}
}

/*
 * edma_misc_handle_irq()
 *	Process miscellaneous interrupts from EDMA
 */
irqreturn_t edma_misc_handle_irq(int irq, void *ctx)
{
	uint32_t misc_intr_status, reg_data;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	struct edma_misc_stats *stats = this_cpu_ptr(edma_gbl_ctx.misc_stats);

	/*
	 * Read Misc intr status
	 */
	reg_data = edma_reg_read(EDMA_REG_MISC_INT_STAT);
	misc_intr_status = reg_data & egc->misc_intr_mask;

	edma_debug("Received misc irq %d, status: %d\n", irq, misc_intr_status);

	if (EDMA_MISC_AXI_RD_ERR_STATUS_GET(misc_intr_status)) {
		edma_err("MISC AXI read error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_axi_read_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (EDMA_MISC_AXI_WR_ERR_STATUS_GET(misc_intr_status)) {
		edma_err("MISC AXI write error received\n");
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_axi_write_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (EDMA_MISC_RX_DESC_FIFO_FULL_STATUS_GET(misc_intr_status)) {
		if (net_ratelimit()) {
			edma_err("MISC Rx descriptor fifo full error received\n");
		}
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_rx_desc_fifo_full;
		u64_stats_update_end(&stats->syncp);
	}

	if (EDMA_MISC_RX_ERR_BUF_SIZE_STATUS_GET(misc_intr_status)) {
		if (net_ratelimit()) {
			edma_err("MISC Rx buffer size error received\n");
		}
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_rx_buf_size_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (EDMA_MISC_TX_SRAM_FULL_STATUS_GET(misc_intr_status)) {
		if (net_ratelimit()) {
			edma_err("MISC Tx SRAM full error received\n");
		}
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_tx_sram_full;
		u64_stats_update_end(&stats->syncp);
	}

	if (EDMA_MISC_TX_CMPL_BUF_FULL_STATUS_GET(misc_intr_status)) {
		if (net_ratelimit()) {
			edma_err("MISC Tx complete buffer full error received\n");
		}
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_tx_cmpl_buf_full;
		u64_stats_update_end(&stats->syncp);
	}

	if (EDMA_MISC_DATA_LEN_ERR_STATUS_GET(misc_intr_status)) {
		if (net_ratelimit()) {
			edma_err("MISC data length error received\n");
		}
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_tx_data_len_err;
		u64_stats_update_end(&stats->syncp);
	}

	if (EDMA_MISC_TX_TIMEOUT_STATUS_GET(misc_intr_status)) {
		if (net_ratelimit()) {
			edma_err("MISC Tx timeout error received\n");
		}
		u64_stats_update_begin(&stats->syncp);
		++stats->edma_misc_tx_timeout;
		u64_stats_update_end(&stats->syncp);
	}

	return IRQ_HANDLED;
}
