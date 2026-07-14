/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/atomic.h>
#include <linux/dma-mapping.h>
#include <linux/cache.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "eip_priv.h"

const char *irq_name[] = {"eip_irq_ring_0", "eip_irq_ring_1", "eip_irq_ring_2", "eip_irq_ring_3",
	"eip_irq_ring_4", "eip_irq_ring_5", "eip_irq_ring_6", "eip_irq_ring_7"};
const char *dma_name[] = {"ring_0", "ring_1", "ring_2", "ring_3",
	"ring_4", "ring_5", "ring_6", "ring_7"};

/*
 * eip_dma_notify()
 *	Notify HW for new descriptor.
 */
static inline void eip_dma_notify(struct eip_dma *dma, uint8_t in_desc, uint8_t out_desc,
		uint32_t in_prod_idx, uint32_t out_prod_idx)
{
	/*
	 * Update producer & consumer index.
	 */
	atomic_set(&dma->in.prod_idx, in_prod_idx);
	atomic_set(&dma->out.prod_idx, out_prod_idx);

	/*
	 * Add Data barrier to insure all descriptor write operation is done.
	 */
	eip_dsb();
	iowrite32(EIP_HW_PREP_CNT_DESC_SZ(EIP_HW_DESC_WORDS * out_desc), dma->out.prep_cnt);
	iowrite32(EIP_HW_PREP_CNT_DESC_SZ(EIP_HW_DESC_WORDS * in_desc), dma->in.prep_cnt);
}

/*
 * eip_dma_tx_res()
 *	Schedule single command for transformation. The function must be called with prempt disabled.
 */
static inline void eip_dma_tx_res(struct eip_dma *dma, struct eip_sw_desc *sw, struct eip_frag *frag)
{
	struct eip_hw_desc *res;

	/*
	 * Invalidate the all cache line associated with output data to avoid any corruption due to cache eviction.
	 * We are performing clean as we may be sharing the cache line with others.
	 */
	eip_dmac_clean_range_no_dsb(frag->data, frag->data + frag->len);

	/*
	 * Fill result descriptor for single buffer.
	 */
	res = &dma->out.desc[frag->idx];


	res->frag[0] = frag->flag;
	res->frag[0] |= EIP_HW_RES_FRAG_LEN(frag->len);
	res->frag[1] = 0;
	res->frag[2] = virt_to_phys(frag->data);
	res->frag[3] = 0;

	res->token[0] = res->token[1] = res->token[2] = res->token[3] = 0;
	res->token[4] = res->token[5] = res->token[6] = res->token[7] = 0;
	res->bypass[0] = res->bypass[1] = res->bypass[2] = res->bypass[3] = 0;

	eip_dmac_clean_range_no_dsb(res, res + 1);
}

/*
 * eip_dma_tx_cmd()
 *	Schedule single command for transformation. The function must be called with prempt disabled.
 */
static inline void eip_dma_tx_cmd(struct eip_dma *dma, struct eip_sw_desc *sw, struct eip_frag *frag)
{
	struct eip_hw_desc *cmd;

	/*
	 * Flush all the data to be read by hardware.
	 */
	eip_dmac_clean_range_no_dsb(frag->data, frag->data + frag->len);

	/*
	 * Fill command descriptor for single buffer.
	 */
	cmd = &dma->in.desc[frag->idx];

	cmd->frag[0] = frag->flag;
	cmd->frag[0] |= EIP_HW_CMD_FRAG_LEN(frag->len);
	cmd->frag[1] = 0;
	cmd->frag[2] = virt_to_phys(frag->data);
	cmd->frag[3] = 0;

	if (frag->flag & EIP_HW_CMD_FLAGS_FIRST) {
		cmd->frag[0] |= EIP_HW_CMD_TOKEN_WORDS(sw->tk_words);

		cmd->token[0] = sw->tk_addr;
		cmd->token[1] = 0;
		cmd->token[2] = sw->tk_hdr;
		cmd->token[3] = 0;
		cmd->token[4] = sw->tr_addr_type;
		cmd->token[5] = 0;
		cmd->token[6] = sw->hw_svc;
		cmd->token[7] = 0;

		*((struct eip_sw_desc **)&cmd->bypass) = sw;
	} else {
		cmd->token[0] = cmd->token[1] = cmd->token[2] = cmd->token[3] = 0;
		cmd->token[4] = cmd->token[5] = cmd->token[6] = cmd->token[7] = 0;
		cmd->bypass[0] = cmd->bypass[1] = 0;
	}

	eip_dmac_clean_range_no_dsb(cmd, cmd + 1);
}

/*
 * eip_dma_read_dma_stats()
 *	Read DMA statistics.
 */
static ssize_t eip_dma_read_dma_stats(struct file *filep, char __user *ubuf, size_t count, loff_t *ppos)
{
	struct eip_dma *dma = filep->private_data;
	struct eip_dma_stats *stats = &dma->stats;
	ssize_t max_buf_len;
	ssize_t len;
	ssize_t ret;
	char *buf;
	int i;

	/*
	 * We need to allocate space for the string and the value
	 */
	max_buf_len = (sizeof(*stats)/sizeof(uint64_t)) * EIP_DEBUGFS_MAX_NAME;
	max_buf_len += (4 * EIP_DEBUGFS_MAX_NAME);

	buf = vzalloc(max_buf_len);
	if (!buf)
		return 0;

	/*
	 * Create strings
	 */
	len = snprintf(buf, max_buf_len, "Tx Packets - %llu\n", stats->tx_pkts);
	len += snprintf(buf + len, max_buf_len - len, "Tx fragments - %llu\n", stats->tx_frags);
	len += snprintf(buf + len, max_buf_len - len, "Tx bytes - %llu\n", stats->tx_bytes);
	len += snprintf(buf + len, max_buf_len - len, "Tx Error - %llu\n", stats->tx_error);
	len += snprintf(buf + len, max_buf_len - len, "Rx Packets - %llu\n", stats->rx_pkts);
	len += snprintf(buf + len, max_buf_len - len, "Rx fragment - %llu\n", stats->rx_frags);
	len += snprintf(buf + len, max_buf_len - len, "Rx bytes - %llu\n", stats->rx_bytes);
	len += snprintf(buf + len, max_buf_len - len, "Rx Error - %llu\n", stats->rx_error);
	len += snprintf(buf + len, max_buf_len - len, "Rx Dropped - %llu\n", stats->rx_dropped);

	len += snprintf(buf + len, max_buf_len - len, "Cmd Prod Idx - %u\n",
		atomic_read(&dma->in.prod_idx));
	len += snprintf(buf + len, max_buf_len - len, "Cmd Cons Idx - %u\n",
		atomic_read(&dma->in.cons_idx));
	len += snprintf(buf + len, max_buf_len - len, "Res Prod Idx - %u\n",
		atomic_read(&dma->out.prod_idx));
	len += snprintf(buf + len, max_buf_len - len, "Res Cons Idx - %u\n",
		atomic_read(&dma->out.cons_idx));

	for (i = 0; i < ARRAY_SIZE(stats->rx_err_code); i++) {
		if (stats->rx_err_code[i]) {
			len += snprintf(buf + len, max_buf_len - len, "Rx error seen for %u = %u\n",
					i, stats->rx_err_code[i]);
		}
	}

	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * eip_dma_rx()
 *	Process all recieved packet on DMA.
 */
static int eip_dma_rx(struct eip_dma *dma, int budget)
{
	uint32_t hw_prod_idx, out_cons_idx, in_cons_idx;
	uint32_t avail, processed;
	uint32_t prod_words;

	/*
	 * Calculate Descriptor produced by HW.
	 */
	prod_words = EIP_HW_PROC_DESC_SZ(ioread32(dma->out.proc));
	hw_prod_idx = prod_words / EIP_HW_DESC_WORDS;

	out_cons_idx = atomic_read(&dma->out.cons_idx);
	in_cons_idx = atomic_read(&dma->in.cons_idx);

	avail = EIP_DMA_AVAIL_COUNT(hw_prod_idx, out_cons_idx);
	if (!avail) {
		return 0;
	}

	avail = avail > budget ? budget : avail;
	processed = avail;

	/*
	 * Invalidate the descriptor we are going to read.
	 * We need to invalidate cache line from head and tail when hw index is behind sw index.
	 */
	if (hw_prod_idx > out_cons_idx) {
		eip_dmac_inv_range(dma->out.desc + out_cons_idx, dma->out.desc + hw_prod_idx);
	} else {
		eip_dmac_inv_range_no_dsb(dma->out.desc + out_cons_idx, dma->out.desc + EIP_DMA_DESC_MAX);
		eip_dmac_inv_range(dma->out.desc, dma->out.desc + hw_prod_idx);
	}

	while (avail--) {
		struct eip_dma_stats *dma_stats;
		struct eip_tr_stats *tr_stats;
		struct eip_hw_desc *res;
		struct eip_sw_desc *sw;
		uint16_t frag_len;
		uint16_t data_len;
		uint16_t cle_err;
		uint16_t tr_err;
		void *data;
		bool err;

		res = &dma->out.desc[out_cons_idx];
		out_cons_idx = EIP_DMA_IDX_INC(out_cons_idx);

		/*
		 * Fragment data needs to be invalidated for non zero fragment.
		 * This is needed as there may be speculative prefetch before completion.
		 */
		frag_len = EIP_HW_RES_FRAG_LEN(res->frag[0]);
		if (likely(frag_len)) {
			data = phys_to_virt(res->frag[2]);
			eip_dmac_inv_range(data, data + frag_len);
		}

		/*
		 * Continue to next descriptor if not last.
		 */
		if (!(EIP_HW_RES_FLAGS_LAST & res->frag[0])) {
			continue;
		}

		/*
		 * Fetch SW object associated with this tranformation.
		 * Bypass will be NULL for any skipped descriptor (hole).
		 */
		sw = *((struct eip_sw_desc **)&res->bypass);
		if (!sw) {
			continue;
		}

		/*
		 * Increment input ring consumer index.
		 */
		in_cons_idx = EIP_DMA_IDX_ADD(in_cons_idx, sw->src_nsegs);

		/*
		 * Fetch error from result descriptor.
		 */
		tr_err = EIP_HW_RES_ERROR(res->token[0]);
		cle_err = EIP_HW_RES_ERROR_CLE(res->token[1]);
		err = cle_err || tr_err;
		data_len = EIP_HW_RES_DATA_LEN(res->token[0]);

		/*
		 * Update DMA Rx stats.
		 * We have not used atomic operation as there is no write contention for stats.
		 */
		tr_stats = this_cpu_ptr(sw->tr->stats_pcpu);
		tr_stats->rx_frags += sw->dst_nsegs;
		tr_stats->rx_pkts++;
		tr_stats->rx_bytes += data_len;
		tr_stats->rx_error += err;

		dma_stats = &dma->stats;
		dma_stats->rx_frags += sw->dst_nsegs;
		dma_stats->rx_pkts++;
		dma_stats->rx_bytes += data_len;
		dma_stats->rx_error += err;

		/*
		 * Call completion callback.
		 */
		if (unlikely(err)) {
			dma->stats.rx_err_code[tr_err]++;
			sw->err_comp(sw->tr, res, sw, cle_err, tr_err);
			continue;
		}

		sw->comp(sw->tr, res, sw);
	}

	/*
	 * Acknowledge HW for consumed result descriptor.
	 */
	iowrite32(EIP_HW_PROC_CNT_DESC_SZ(EIP_HW_DESC_WORDS * processed), dma->out.proc_cnt);

	/*
	 * Update consumer indexes in dma object.
	 */
	atomic_set(&dma->out.cons_idx, out_cons_idx);
	atomic_set(&dma->in.cons_idx, in_cons_idx);

	return processed;
}

/*
 * eip_dma_handle_irq()
 *	Interrupt handler for DMA Rx.
 */
static irqreturn_t eip_dma_handle_irq(int irq, void *ctx)
{
	struct eip_dma *dma = ctx;

	/*
	 * Acknowledge interrupt. HW interrupt remain in disabled state
	 * until our bottom half re-enables it.
	 */
	iowrite32(EIP_HW_RDR_PROC_IRQ_STATUS, dma->out.proc_status);

	/*
	 * Schedule tasklet to process interrupt.
	 */
	napi_schedule(&dma->out.napi);
	return IRQ_HANDLED;
}

/*
 * eip_dma_napi_rx_poll()
 *	NAPI poll callback to handle EIP197 interrupt.
 */
static int eip_dma_napi_rx_poll(struct napi_struct *napi, int budget)
{
	struct eip_dma *dma = container_of(napi, struct eip_dma, out.napi);
	int processed;

	/*
	 * Reap Rx ring. If we have processed all packet in queue
	 * then disable napi and re-enable irq in EIP.
	 */
	processed = eip_dma_rx(dma, budget);
	if (processed < budget) {
		napi_complete(napi);
		iowrite32(EIP_HW_RDR_HIA_THR_VAL, dma->out.proc_thresh);
	}

	return processed;
}

/*
 * eip_dma_tx_linear_skb()
 *	Schedule linear SKB for transformation. The function must be called with prempt disabled.
 */
int eip_dma_tx_linear_skb(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb)
{
	struct eip_tr_stats *tr_stats = this_cpu_ptr(sw->tr->stats_pcpu);
	struct eip_frag src;
	struct eip_frag dst;
	uint32_t out_count;
	uint32_t in_count;

	BUG_ON(skb_is_nonlinear(skb));

	/*
	 * We may be in process context. disable bottom half to prevent preemption.
	 * We have per cpu DMA object. So this ensures that there is no race condition.
	 */
	local_bh_disable();

	in_count = eip_dma_avail_idx_and_count(&dma->in, &src.idx);
	out_count = eip_dma_avail_idx_and_count(&dma->out, &dst.idx);

	if ((!in_count | !out_count)) {
		goto fail;
	}

	/*
	 * Fill Source fragment for Command ring.
	 */
	src.data = skb->data;
	src.len = skb->len;
	src.flag = EIP_HW_CMD_FLAGS(1, 1);
	sw->src_nsegs = 1;
	eip_dma_tx_cmd(dma, sw, &src);

	/*
	 * Fill Destination fragment for result ring.
	 * Output length can be higher for encap. Hence, we include tailroom length.
	 */
	dst.data = skb->data;
	dst.len = skb->len + skb_tailroom(skb);
	dst.flag = EIP_HW_RES_FLAGS(1, 1);
	sw->dst_nsegs = 1;
	eip_dma_tx_res(dma, sw, &dst);

	/*
	 * Increament Tx statistics.
	 */
	tr_stats->tx_frags += 1;
	dma->stats.tx_frags += 1;
	tr_stats->tx_pkts++;
	dma->stats.tx_pkts++;
	tr_stats->tx_bytes += src.len;
	dma->stats.tx_bytes += src.len;

	eip_dma_notify(dma, 1, 1, EIP_DMA_IDX_INC(src.idx), EIP_DMA_IDX_INC(dst.idx));

	/*
	 * Re-enable preemption.
	 */
	local_bh_enable();
	return 0;

fail:
	local_bh_enable();
	return -EBUSY;
}

/*
 * eip_dma_tx_nonlinear_skb()
 *	Schedule non-linear SKB for transformation. The function must be called with prempt disabled.
 */
int eip_dma_tx_nonlinear_skb(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb)
{
	/*
	 * TODO: Add support.
	 */
	return -EINVAL;
}

/*
 * eip_dma_tx_sg()
 *	Schedule scatterlist buffer for transformation.
 */
int eip_dma_tx_sg(struct eip_dma *dma, struct eip_sw_desc *sw, struct scatterlist *src, struct scatterlist *dst)
{
	struct eip_tr_stats *tr_stats = this_cpu_ptr(sw->tr->stats_pcpu);
	struct eip_dma_stats *dma_stats = &dma->stats;
	struct scatterlist *iter = NULL;
	uint32_t src_nsegs, dst_nsegs;
	uint32_t cmd_idx, res_idx;
	uint32_t src_len, len;
	uint32_t count;

	/*
	 * We may be in process context. disable bottom half to prevent preemption.
	 * We have per cpu DMA object. So this ensures that there is no race condition.
	 */
	local_bh_disable();

	/*
	 * Map only required data to descriptor.
	 * Application may send larger buffer then actual data length. (To accomodate hash output).
	 */
	src_len = EIP_HW_CMD_DATA_LEN(sw->tk_hdr);

	/*
	 * Walk through all source scatterlist.
	 * There is no cleanup for error as actual HW index will be updated in notify().
	 */
	count = eip_dma_avail_idx_and_count(&dma->in, &cmd_idx);

	for (src_nsegs = 0, iter = src, len = src_len; iter; src_nsegs++, iter = sg_next(iter)) {
		struct eip_frag frag;

		if (unlikely(!count--)) {
			goto fail;
		}

		/*
		 * Set fragment specific information.
		 */
		frag.data = sg_virt(iter);
		frag.len = (len < iter->length) ? len : iter->length;
		frag.idx = cmd_idx;
		frag.flag = EIP_HW_CMD_FLAGS(!src_nsegs, !!sg_is_last(iter));

		/*
		 * Fill transmit command descriptor.
		 */
		eip_dma_tx_cmd(dma, sw, &frag);
		cmd_idx = EIP_DMA_IDX_INC(cmd_idx);
		len -= iter->length;
	}

	/*
	 * Walk through all destination scatterlist.
	 * There is no cleanup for error as actual HW index will be updated in notify().
	 */
	count = eip_dma_avail_idx_and_count(&dma->out, &res_idx);

	for (dst_nsegs = 0, iter = dst; iter; dst_nsegs++, iter = sg_next(iter)) {
		struct eip_frag frag;

		if (unlikely(!count--)) {
			goto fail;
		}

		/*
		 * Set fragment specific information.
		 */
		frag.data = sg_virt(iter);
		frag.len = iter->length;
		frag.idx = res_idx;
		frag.flag = EIP_HW_RES_FLAGS(!dst_nsegs, !!sg_is_last(iter));

		/*
		 * Fill transmit result descriptor.
		 */
		eip_dma_tx_res(dma, sw, &frag);
		res_idx = EIP_DMA_IDX_INC(res_idx);
	}

	sw->src_nsegs = src_nsegs;
	sw->dst_nsegs = dst_nsegs;

	/*
	 * Increament Tx statistics.
	 */
	tr_stats->tx_frags += src_nsegs;
	tr_stats->tx_pkts++;
	tr_stats->tx_bytes += src_len;
	dma_stats->tx_frags += src_nsegs;
	dma_stats->tx_pkts++;
	dma_stats->tx_bytes += src_len;

	/*
	 * Notify the HW for new descriptor.
	 */
	eip_dma_notify(dma, src_nsegs, dst_nsegs, cmd_idx, res_idx);

	/*
	 * Re-enable preemption.
	 */
	local_bh_enable();
	return 0;

fail:
	local_bh_enable();
	return -ENOMEM;
}

struct file_operations dma_stats_ops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.llseek = default_llseek,
	.read = eip_dma_read_dma_stats
};

/*
 * eip_dma_cmd_init()
 *	Initialize the DMA HW ring.
 */
int eip_dma_cmd_init(struct eip_dma *dma, void __iomem *base_addr, bool irq, uint8_t rx_cpu)
{
	struct eip_hw_ring *cmd = &dma->in;
	uint32_t ring_id = dma->ring_id;
	size_t total_sz;
	size_t dma_sz;
	void *addr;

	/*
	 * Allocate memory for
	 * - DMA FIFO
	 * - Extra padding for cache alignment
	 * - Software shadow FIFO for meta pointers.
	 */
	dma_sz = sizeof(struct eip_hw_desc) * EIP_DMA_DESC_MAX;
	total_sz = dma_sz + L1_CACHE_BYTES + L1_CACHE_BYTES;
	total_sz += (sizeof(uintptr_t) * EIP_DMA_DESC_MAX);
	addr = kzalloc(total_sz, GFP_KERNEL);
	if (!addr) {
		pr_err("%px: Failed to allocate in ring memory\n", cmd);
		return -ENOMEM;
	}

	cmd->kaddr = addr;
	cmd->desc = PTR_ALIGN(addr, L1_CACHE_BYTES);
	cmd->meta = (void *)cmd->desc + dma_sz;
	atomic_set(&cmd->prod_idx, 0);
	atomic_set(&cmd->cons_idx, 0);

	/*
	 * Initialize register address.
	 */
	cmd->prep = base_addr + EIP_HW_HIA_CDR_PREP_PNTR(ring_id);
	cmd->proc = base_addr + EIP_HW_HIA_CDR_PROC_PNTR(ring_id);
	cmd->prep_cnt = base_addr + EIP_HW_HIA_CDR_PREP_COUNT(ring_id);
	cmd->proc_cnt = base_addr + EIP_HW_HIA_CDR_PROC_COUNT(ring_id);
	cmd->proc_thresh = base_addr + EIP_HW_HIA_CDR_THRESH(ring_id);
	cmd->proc_status = base_addr + EIP_HW_HIA_CDR_STAT(ring_id);

	/*
	 * Reset Ring.
	 */
	iowrite32(EIP_HW_HIA_CDR_PREP_COUNT_RST, base_addr + EIP_HW_HIA_CDR_PREP_COUNT(ring_id));
	iowrite32(EIP_HW_HIA_CDR_PROC_COUNT_RST, base_addr + EIP_HW_HIA_CDR_PROC_COUNT(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_PREP_PNTR(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_PROC_PNTR(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_RING_BASE_ADDR_LO(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_RING_BASE_ADDR_HI(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_RING_SIZE(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_DESC_SIZE(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_CFG(ring_id));
	iowrite32(EIP_HW_HIA_CDR_DMA_CFG_RST, base_addr + EIP_HW_HIA_CDR_DMA_CFG(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_CDR_THRESH(ring_id));
	iowrite32(0x1f, base_addr + EIP_HW_HIA_CDR_STAT(ring_id));

	/*
	 * Configure Ring
	 */
	iowrite32(virt_to_phys(cmd->desc), base_addr + EIP_HW_HIA_CDR_RING_BASE_ADDR_LO(ring_id));
	iowrite32(EIP_HW_RING_DESC_SZ(dma_sz/sizeof(uint32_t)), base_addr + EIP_HW_HIA_CDR_RING_SIZE(ring_id));
	iowrite32(EIP_HW_CDR_DESC_SIZE_VAL, base_addr + EIP_HW_HIA_CDR_DESC_SIZE(ring_id));
	iowrite32(EIP_HW_CDR_CFG_VAL, base_addr + EIP_HW_HIA_CDR_CFG(ring_id));
	iowrite32(EIP_HW_CDR_DMA_CFG_VAL, base_addr + EIP_HW_HIA_CDR_DMA_CFG(ring_id));

	if (irq) {
		uint32_t val;
		iowrite32(EIP_HW_CDR_HIA_THR_VAL, base_addr + EIP_HW_HIA_CDR_THRESH(ring_id));
		val = ioread32(base_addr + EIP_HW_HIA_AIC_R_ENABLE_CTRL(rx_cpu));
		val |= EIP_HW_HIA_AIC_R_ENABLE_CTRL_CDR(ring_id);
		iowrite32(val, base_addr + EIP_HW_HIA_AIC_R_ENABLE_CTRL(rx_cpu));
	}

	return 0;
}

/*
 * eip_dma_res_init()
 *	Initialize the DMA HW ring.
 */
int eip_dma_res_init(struct eip_dma *dma, void __iomem *base_addr, bool irq, uint8_t rx_cpu)
{
	struct eip_hw_ring *res = &dma->out;
	uint32_t ring_id = dma->ring_id;
	size_t total_sz;
	size_t dma_sz;
	void *addr;

	/*
	 * Allocate memory for
	 * - DMA FIFO
	 * - Extra padding for cache alignment
	 * - Software shadow FIFO for meta pointers.
	 */
	dma_sz = sizeof(struct eip_hw_desc) * EIP_DMA_DESC_MAX;
	total_sz = dma_sz + L1_CACHE_BYTES + L1_CACHE_BYTES;
	total_sz += (sizeof(uintptr_t) * EIP_DMA_DESC_MAX);
	addr = kzalloc(total_sz, GFP_KERNEL);
	if (!addr) {
		pr_err("%px: Failed to allocate in ring memory\n", res);
		return -ENOMEM;
	}

	res->kaddr = addr;
	res->desc = PTR_ALIGN(addr, L1_CACHE_BYTES);
	res->meta = (uintptr_t *)((uint8_t *)res->desc + dma_sz);
	atomic_set(&res->prod_idx, 0);
	atomic_set(&res->cons_idx, 0);

	/*
	 * Initialize register address.
	 */
	res->prep = base_addr + EIP_HW_HIA_RDR_PREP_PNTR(ring_id);
	res->proc = base_addr + EIP_HW_HIA_RDR_PROC_PNTR(ring_id);
	res->prep_cnt = base_addr + EIP_HW_HIA_RDR_PREP_COUNT(ring_id);
	res->proc_cnt = base_addr + EIP_HW_HIA_RDR_PROC_COUNT(ring_id);
	res->proc_thresh = base_addr + EIP_HW_HIA_RDR_THRESH(ring_id);
	res->proc_status = base_addr + EIP_HW_HIA_RDR_STAT(ring_id);

	/*
	 * Reset Ring.
	 */
	iowrite32(EIP_HW_HIA_RDR_PREP_COUNT_RST, base_addr + EIP_HW_HIA_RDR_PREP_COUNT(ring_id));
	iowrite32(EIP_HW_HIA_RDR_PROC_COUNT_RST, base_addr + EIP_HW_HIA_RDR_PROC_COUNT(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_PREP_PNTR(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_PROC_PNTR(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_RING_BASE_ADDR_LO(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_RING_BASE_ADDR_HI(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_RING_SIZE(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_DESC_SIZE(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_CFG(ring_id));
	iowrite32(EIP_HW_HIA_RDR_DMA_CFG_RST, base_addr + EIP_HW_HIA_RDR_DMA_CFG(ring_id));
	iowrite32(0x0, base_addr + EIP_HW_HIA_RDR_THRESH(ring_id));
	iowrite32(0x1f, base_addr + EIP_HW_HIA_RDR_STAT(ring_id));

	/*
	 * Configure Ring
	 */
	iowrite32(virt_to_phys(res->desc), base_addr + EIP_HW_HIA_RDR_RING_BASE_ADDR_LO(ring_id));
	iowrite32(EIP_HW_RING_DESC_SZ(dma_sz/sizeof(uint32_t)), base_addr + EIP_HW_HIA_RDR_RING_SIZE(ring_id));
	iowrite32(EIP_HW_RDR_DESC_SIZE_VAL, base_addr + EIP_HW_HIA_RDR_DESC_SIZE(ring_id));
	iowrite32(EIP_HW_RDR_CFG_VAL, base_addr + EIP_HW_HIA_RDR_CFG(ring_id));
	iowrite32(EIP_HW_RDR_DMA_CFG_VAL, base_addr + EIP_HW_HIA_RDR_DMA_CFG(ring_id));

	if (irq) {
		uint32_t val;
		iowrite32(EIP_HW_RDR_HIA_THR_VAL, base_addr + EIP_HW_HIA_RDR_THRESH(ring_id));
		val = ioread32(base_addr + EIP_HW_HIA_AIC_R_ENABLE_CTRL(rx_cpu));
		val |= EIP_HW_HIA_AIC_R_ENABLE_CTRL_RDR(ring_id);
		iowrite32(val, base_addr + EIP_HW_HIA_AIC_R_ENABLE_CTRL(rx_cpu));
	}

	return 0;
}

/*
 * eip_dma_la_deinit()
 *	De-Initialize the DMA HW ring.
 */
void eip_dma_la_deinit(struct eip_dma *dma)
{
	if (!dma->active) {
		return;
	}

	debugfs_remove_recursive(dma->dentry);
	free_irq(dma->irq, dma);
	napi_disable(&dma->out.napi);
	netif_napi_del(&dma->out.napi);
	kfree(dma->out.kaddr);
	kfree(dma->in.kaddr);

	memset(dma, 0, sizeof(*dma));
}

/*
 * eip_dma_la_init()
 *	initialize the EIP197 DMA.
 */
int eip_dma_la_init(struct eip_dma *dma, struct platform_device *pdev, uint8_t tx_cpu,
		uint8_t rx_cpu,	uint32_t ring_id, void __iomem *base_addr)
{
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	int status;

	if (dma->active) {
		pr_err("%px: DMA is already configured; ring_id(%u)\n", dma, ring_id);
		return -EINVAL;
	}

	dma->type = EIP_DMA_TYPE_LA;
	dma->ring_id = ring_id;
	dma->irq_mask = EIP_HW_RDR_PROC_IRQ_STATUS;
	dma->irq_status = base_addr + EIP_HW_HIA_RDR_STAT(ring_id);
	dma->irq = platform_get_irq(ep->pdev, rx_cpu);
	if (!dma->irq) {
		pr_err("%px: irq_of_parse_and_map() fail for irq, ring_id(%u)\n", dma, ring_id);
		return -EINVAL;
	}

	/*
	 * Initialize and allocate ring.
	 * Enable interrupt only for result ring.
	 */
	status = eip_dma_cmd_init(dma, base_addr, false, rx_cpu);
	if (status < 0) {
		pr_err("%px: Failed to initialize cmd ring(%u)\n", dma, ring_id);
		return status;

	}

	status = eip_dma_res_init(dma, base_addr, true, rx_cpu);
	if (status < 0) {
		pr_err("%px: Failed to initialize res ring(%u)\n", dma, ring_id);
		goto fail_res;
	}

	/*
	 * Initialize the interrupt and NAPI.
	 */
	status = request_irq(dma->irq, eip_dma_handle_irq, IRQF_SHARED, irq_name[ring_id], dma);
	if (status < 0) {
		pr_err("%px: IRQ %u request failed\n", dma, dma->irq);
		goto fail_irq;
	}

	init_dummy_netdev(&dma->out.ndev);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	netif_threaded_napi_add(&dma->out.ndev, &dma->out.napi, eip_dma_napi_rx_poll, eip_dma_rx_napi_weight);
#else
	netif_threaded_napi_add_weight(&dma->out.ndev, &dma->out.napi, eip_dma_napi_rx_poll,
			eip_dma_rx_napi_weight);
#endif
	napi_enable(&dma->out.napi);

	/*
	 * Interrupt can only be handled by the specified CPU.
	 */
	irq_set_affinity_hint(dma->irq, cpumask_of(rx_cpu));

	dma->dentry = debugfs_create_file(dma_name[ring_id], S_IRUGO, ep->dentry, dma, &dma_stats_ops);
	dma->active = 1;
	pr_info("%px: DMA ring(%u) configured and Rx mapped to cpu(%u)\n", dma, ring_id, rx_cpu);
	return 0;

fail_irq:
	kfree(dma->out.kaddr);
fail_res:
	kfree(dma->in.kaddr);
	return status;
}
