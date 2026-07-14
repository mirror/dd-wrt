/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
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
#include <net/ip.h>

#include <ppe_drv_port.h>
#include "eip_priv.h"

/*
 * eip_dma_hy_notify()
 *	Notify HW for new descriptor.
 */
static inline void eip_dma_hy_notify(struct eip_hw_ring *ring, uint8_t desc, uint32_t prod_idx)
{
	/*
	 * Update producer.
	 */
	atomic_set(&ring->prod_idx, prod_idx);

	/*
	 * Add Data barrier to insure all descriptor write operation is done.
	 */
	eip_dsb();
	iowrite32(EIP_HW_PREP_CNT_DESC_SZ(EIP_HW_DESC_WORDS * desc), ring->prep_cnt);
}

/*
 * eip_dma_hy_tx_res()
 *	Schedule single command for transformation. The function must be called with prempt disabled.
 */
static inline void eip_dma_hy_tx_res(struct eip_dma *dma, struct sk_buff *skb, struct eip_frag *frag)
{
	struct eip_hw_desc *res;

	/*
	 * Invalidate the all cache line associated with output data to avoid any corruption due to cache eviction.
	 * This is new SKB so we dont care about the stored data.
	 */
	eip_dmac_inv_range_no_dsb(frag->data, frag->data + frag->len);

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

	dma->out.meta[frag->idx] = (uintptr_t)skb;
	eip_dmac_clean_range_no_dsb(res, res + 1);
}

/*
 * eip_dma_hy_tx_cmd()
 *	Schedule single command for transformation. The function must be called with prempt disabled.
 */
static inline void eip_dma_hy_tx_cmd(struct eip_dma *dma, struct eip_sw_desc *sw, struct eip_frag *frag)
{
	struct eip_hw_desc *cmd;

	/*
	 * Flush all the data to be read by hardware.
	 */
	eip_dmac_flush_range_no_dsb(frag->data, frag->data + frag->len);

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
		struct eip_tr *tr = sw->tr;

		cmd->frag[0] |= EIP_HW_CMD_TOKEN_WORDS(sw->tk_words);

		cmd->token[0] = sw->tk_addr;
		cmd->token[1] = 0;
		cmd->token[2] = sw->tk_hdr;
		cmd->token[3] = 0;
		cmd->token[4] = sw->tr_addr_type;
		cmd->token[5] = 0;
		cmd->token[6] = sw->hw_svc;
		cmd->token[7] = EIP_HW_CMD_OFFSET(sizeof(struct ethhdr));

		/*
		 * TODO: Copy with uint64_t?
		 */
		cmd->bypass[0] = tr->bypass[0];
		cmd->bypass[1] = tr->bypass[1];
		cmd->bypass[2] = tr->bypass[2];
		cmd->bypass[3] = tr->bypass[3];
	} else {
		cmd->token[0] = cmd->token[1] = cmd->token[2] = cmd->token[3] = 0;
		cmd->token[4] = cmd->token[5] = cmd->token[6] = cmd->token[7] = 0;
		cmd->bypass[0] = cmd->bypass[1] = cmd->bypass[2] = cmd->bypass[3] = 0;
	}

	dma->in.meta[frag->idx] = (frag->flag & EIP_HW_CMD_FLAGS_LAST) ? (uintptr_t)sw : 0;
	eip_dmac_clean_range_no_dsb(cmd, cmd + 1);
}

/*
 * eip_dma_hy_refill()
 *	Refill new SKB(s) in DMA output ring.
 */
static void eip_dma_hy_refill(struct eip_dma *dma)
{
	uint32_t count, processed;
	struct eip_frag frag;
	struct sk_buff *skb;
	uint32_t pidx;

	count = eip_dma_avail_idx_and_count(&dma->out, &pidx);
	if (!count)
		return;


	/*
	 * Refill start index.
	 * TODO: How do we support jumbo size? Do we just increase EIP_RX_BUFFER_SIZE?
	 */
	frag.idx = pidx;
	processed = count;

	while (count) {
		skb = dev_alloc_skb(EIP_RX_BUFFER_SIZE);
		if (unlikely(!skb))
			break;

		/*
		 * Put the data to largest possible value. In rx, we will trim the data with actual size.
		 */
		skb_reserve(skb, EIP_RX_BUFFER_HEADROOM);
		skb_put(skb, EIP_RX_BUFFER_DATA_LEN);

		frag.data = skb->data;
		frag.len = skb->len;
		frag.flag = EIP_HW_RES_FLAGS(1, 0);

		eip_dma_hy_tx_res(dma, skb, &frag);
		frag.idx = EIP_DMA_IDX_INC(frag.idx);
		count -= 1;
	}

	eip_dma_hy_notify(&dma->out, processed - count, frag.idx);
}

/*
 * eip_dma_hy_get_tr()
 *	Get the TR object using physical address.
 */
static inline struct eip_tr *eip_dma_hy_get_tr(struct eip_hw_desc *res)
{
	uint32_t tr_hw_addr = EIP_HW_RES_TR_ADDR(res->token[4]);

	/*
	 * higher 32bit must be zero as we use only 32bit bus address.
	 */
	if (unlikely(!tr_hw_addr || res->token[5] == 0xffffffff)) {
		return NULL;
	}

	/*
	 * TR reference does not work for packet received from inline channel.
	 * Instead we make sure TR invalidation & free is scheduled only after all the packet
	 * is read from queue to make sure memory is valid.
	 */
	return container_of(phys_to_virt(tr_hw_addr), struct eip_tr, hw_words);
}

/*
 * eip_dma_hy_tr_deliver()
 *	Send SKB to registered client's handler.
 */
static inline void eip_dma_hy_tr_deliver(struct eip_tr *tr, struct sk_buff *skb, uint8_t nsegs)
{
	struct eip_tr_stats *tr_stats = this_cpu_ptr(tr->stats_pcpu);

	BUG_ON(tr->svc != EIP_SVC_HYBRID_IPSEC);

	tr_stats->rx_frags += nsegs;
	tr_stats->rx_pkts++;
	tr_stats->rx_bytes += skb->len;

	/*
	 * Call completion callback.
	 */
	tr->ipsec.ops.cb(tr->ipsec.app_data, skb);
}

/*
 * eip_dma_hy_tr_deliver_err()
 *	Send SKB to registered client's error handler.
 */
static inline void eip_dma_hy_tr_deliver_err(struct eip_tr *tr, struct sk_buff *skb, uint8_t nsegs, uint16_t tr_err)
{
	struct eip_tr_stats *tr_stats = this_cpu_ptr(tr->stats_pcpu);

	BUG_ON(tr->svc != EIP_SVC_HYBRID_IPSEC);

	tr_stats->rx_frags += nsegs;
	tr_stats->rx_pkts++;
	tr_stats->rx_bytes += skb->len;
	tr_stats->rx_error ++;

	/*
	 * Call completion callback.
	 */
	tr->ipsec.ops.err_cb(tr->ipsec.app_data, skb, -EBADE);
}

/*
 * eip_dma_hy_deliver_linux()
 *	Send SKB to Linux stack.
 */
static inline bool eip_dma_hy_deliver_linux(struct sk_buff *skb, struct eip_hw_desc *res)
{
	uint16_t src_port = EIP_HW_RES_BYPASS_SRC_PORT(res->bypass[0]);
	struct net_device *dev = ppe_drv_port_num_to_dev(src_port);
	unsigned int ip_hdr_sz;
	uint16_t proto;

	if (!dev) {
		return false;
	}

	/* Reset General SKB fields for further processing */
	skb_scrub_packet(skb, false);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);

	skb->dev = dev;
	skb->skb_iif = dev->ifindex;
	skb->ip_summed = CHECKSUM_NONE;

	/* Detect IPv4 / IPv6 */
	if (ip_hdr(skb)->version == IPVERSION) {
		proto = htons(ETH_P_IP);
		ip_hdr_sz = sizeof(struct iphdr);
	} else {
		proto = htons(ETH_P_IPV6);
		ip_hdr_sz = sizeof(struct ipv6hdr);
	}

	skb->protocol = proto;
	skb_set_transport_header(skb, ip_hdr_sz);

	/* Inject into stack */
	netif_receive_skb(skb);

	return true;
}

/*
 * eip_dma_hy_classify_tr_err()
 *	Process packet with TR error.
 */
static inline bool eip_dma_hy_classify_err(struct eip_dma *dma, struct sk_buff *skb, struct eip_hw_desc *res)
{
	struct eip_dma_stats *dma_stats = &dma->stats;
	uint16_t cle_err, tr_err;

	tr_err = EIP_HW_RES_ERROR(res->token[0]);
	cle_err = EIP_HW_RES_ERROR_CLE(res->token[1]);

	/*
	 * Increase error counter only for non-bypass errors.
	 */
	dma_stats->rx_error++;
	dma_stats->rx_err_code[tr_err]++;

	/*
	 * Resubmit packet to the Linux stack.
	 * This includes control packets (e.g. NAT-T, IKE) and cases where
	 * hardware flow lookup fails.
	 */

	switch (tr_err) {
		case EIP_HW_RES_ERR_NAT_T:
		case EIP_HW_RES_ERR_IKE:
		case EIP_HW_RES_ERR_FLOW_FAIL:
			/*
			 * Pull the fake mac added by PPE.
			 * token[3] contains offset to start of network header.
			 */
			skb_pull(skb, EIP_HW_RES_OFFST(res->token[3]));
			if (!eip_dma_hy_deliver_linux(skb, res)) {
				consume_skb(skb);
				dma_stats->rx_dropped++;
			}
			return true;
		default:
			break;
	}

	/*
	 * Packet must be dropped when there is CLE error.
	 */
	if (cle_err) {
		dma_stats->rx_dropped++;
		consume_skb(skb);
		return true;
	}

	/*
	 * Other TR error should be processed by clients.
	 */
	return false;
}

/*
 * eip_dma_hy_refill_all()
 *	Refill new SKB(s) in all hybrid DMA output ring.
 */
void eip_dma_hy_refill_all(struct eip_ctx *ctx)
{
	struct eip_drv *drv = &eip_drv_g;
	struct eip_pdev *ep = ctx->ep;
	int cpu;

	for_each_cpu(cpu, &drv->hy_cpu_map) {
		eip_dma_hy_refill(&ep->hy[cpu]);
	}
}

/*
 * eip_dma_hy_rx()
 *	Process all recieved packet on DMA.
 */
int eip_dma_hy_rx(struct eip_dma *dma, int budget)
{
	uint32_t hw_pidx, out_cidx;
	uint32_t avail, processed;
	struct sk_buff_head list;
	uint32_t true_len = 0;
	uint32_t list_len = 0;
	uint32_t prod_words;
	uint32_t old_cidx;


	__skb_queue_head_init(&list);

	/*
	 * Calculate Descriptor produced by HW.
	 */
	prod_words = EIP_HW_PROC_DESC_SZ(ioread32(dma->out.proc));
	hw_pidx = prod_words / EIP_HW_DESC_WORDS;
	out_cidx = atomic_read(&dma->out.cons_idx);

	avail = EIP_DMA_AVAIL_COUNT(hw_pidx, out_cidx);
	if (!avail) {
		return 0;
	}

	avail = avail > budget ? budget : avail;
	old_cidx = out_cidx;

	/*
	 * Invalidate the descriptor we are going to read.
	 * We need to invalidate cache line from head and tail when hw index is behind sw index.
	 */
	if (hw_pidx > out_cidx) {
		eip_dmac_inv_range(dma->out.desc + out_cidx, dma->out.desc + hw_pidx);
	} else {
		eip_dmac_inv_range_no_dsb(dma->out.desc + out_cidx, dma->out.desc + EIP_DMA_DESC_MAX);
		eip_dmac_inv_range(dma->out.desc, dma->out.desc + hw_pidx);
	}

	while (avail--) {
		struct eip_dma_stats *dma_stats = &dma->stats;
		uint16_t cle_err, tr_err;
		struct sk_buff *head_skb;
		struct eip_hw_desc *res;
		struct sk_buff *skb;
		struct eip_tr *tr;
		uint16_t frag_len;
		uint8_t nsegs;
		void *data;
		bool err;

		res = &dma->out.desc[out_cidx];
		skb = (struct sk_buff *)dma->out.meta[out_cidx];
		out_cidx = EIP_DMA_IDX_INC(out_cidx);

		/*
		 * Fragment data needs to be invalidated for non zero fragment.
		 * This is needed as there may be speculative prefetch before completion.
		 */
		frag_len = EIP_HW_RES_FRAG_LEN(res->frag[0]);
		BUG_ON(!frag_len);
		data = phys_to_virt(res->frag[2]);
		eip_dmac_inv_range(data, data + frag_len);

		/*
		 * Trim the linear SKB frag to length produced by hardware.
		 */
		skb_trim(skb, frag_len);

		/*
		 * Linear data. Both First & Last flags are set.
		 */
		if (likely((EIP_HW_RES_FLAGS_MASK & res->frag[0]) == EIP_HW_RES_FLAGS_LINEAR)) {
			head_skb = skb;
			nsegs = 1;
			goto deliver;
		}

		/*
		 * Non-linear data. Set our head to descriptor with first flag.
		 * This condition will be always true for first descritor in chain. Hence, head_skb will be always assigned.
		 */
		if (EIP_HW_RES_FLAGS_FIRST & res->frag[0]) {
			head_skb = skb;
			continue;
		}

		/*
		 * Append SKB to head SKB frag list.
		 */
		list_len += skb->len;
		true_len += skb->truesize;
		__skb_queue_tail(&list, skb);
		skb = NULL;

		/*
		 * Continue to next descriptor if not last.
		 * We dont update consumer index if full chain is not received.
		 * So, will read the descriptor again if last is not received
		 * during this execution.
		 */
		if (!(EIP_HW_RES_FLAGS_LAST & res->frag[0])) {
			continue;
		}

		/*
		 * All fragments are received;
		 * Link the Head SKB with the list.
		 */
		skb_shinfo(head_skb)->frag_list = __skb_peek(&list);
		skb_shinfo(head_skb)->frag_list->prev = NULL;
		skb_peek_tail(&list)->next = NULL;
		head_skb->len += list_len;
		head_skb->data_len = list_len;
		head_skb->truesize += true_len;
		nsegs = skb_queue_len(&list) + 1;

		/*
		 * Reset the list.
		 */
		list_len = 0;
		true_len = 0;
		__skb_queue_head_init(&list);

deliver:
		/*
		 * Fetch error from result descriptor.
		 */
		tr_err = EIP_HW_RES_ERROR(res->token[0]);
		cle_err = EIP_HW_RES_ERROR_CLE(res->token[1]);
		err = cle_err || tr_err;
		WARN_ON(head_skb->len != EIP_HW_RES_DATA_LEN(res->token[0]));

		/*
		 * Update DMA Rx stats.
		 * We have not used atomic operation as there is no write contention for stats.
		 */
		dma_stats->rx_frags += nsegs;
		dma_stats->rx_pkts++;
		dma_stats->rx_bytes += head_skb->len;

		if (err && eip_dma_hy_classify_err(dma, head_skb, res)) {
			goto next;
		}

		tr = eip_dma_hy_get_tr(res);
		if (unlikely(!tr)) {
			dma_stats->rx_dropped++;
			consume_skb(head_skb);
			goto next;
		}

		/*
		 * Pull the fake mac added by PPE.
                 * token[3] contains offset to start of network header.
		 */
		skb_pull(head_skb, EIP_HW_RES_OFFST(res->token[3]));

		/*
		 * Send packet to the client's handler.
		 */
		if (err) {
			eip_dma_hy_tr_deliver_err(tr, head_skb, nsegs, tr_err);
			goto next;
		}

		eip_dma_hy_tr_deliver(tr, head_skb, nsegs);
next:
		/*
		 * Update output consumer indexes in dma object.
		 */
		atomic_set(&dma->out.cons_idx, out_cidx);
	}

	/*
	 * Acknowledge HW for result descriptor.
	 * We may have not processed last few descriptor due to incomplete SG.
	 * Hence, find actual processed count using diff of old consumer index and new consumer index.
	 */
	processed = EIP_DMA_AVAIL_COUNT(atomic_read(&dma->out.cons_idx), old_cidx);
	iowrite32(EIP_HW_PROC_CNT_DESC_SZ(EIP_HW_DESC_WORDS * processed), dma->out.proc_cnt);

	return processed;
}

/*
 * eip_dma_hy_tx_comp()
 *	Free transmitted buffer.
 */
int eip_dma_hy_tx_comp(struct eip_dma *dma, int budget)
{
	uint32_t hw_cidx, in_cidx;
	uint32_t cons_words;
	uint32_t processed;
	uint32_t avail;

	/*
	 * Calculate Descriptor produced by HW.
	 */
	cons_words = EIP_HW_PROC_DESC_SZ(ioread32(dma->in.proc));
	hw_cidx = cons_words / EIP_HW_DESC_WORDS;
	in_cidx = atomic_read(&dma->in.cons_idx);

	avail = EIP_DMA_AVAIL_COUNT(hw_cidx, in_cidx);
	if (!avail) {
		return 0;
	}

	avail = avail > budget ? budget : avail;
	processed = avail;

	while (avail--) {
		struct eip_sw_desc *sw;

		sw = (struct eip_sw_desc *)dma->in.meta[in_cidx];
		in_cidx = EIP_DMA_IDX_INC(in_cidx);

		if (!sw)
			continue;

		sw->comp(sw->tr, NULL, sw);
	}

	/*
	 * Acknowledge HW for command descriptor.
	 */
	iowrite32(EIP_HW_PROC_CNT_DESC_SZ(EIP_HW_DESC_WORDS * processed), dma->in.proc_cnt);

	/*
	 * Update input consumer index.
	 */
	atomic_set(&dma->in.cons_idx, in_cidx);
	return processed;
}

/*
 * eip_dma_hy_handle_cmd_irq()
 *	Interrupt handler for DMA Rx.
 */
static irqreturn_t eip_dma_hy_handle_cmd_irq(int irq, void *ctx)
{
	struct eip_dma *dma = ctx;

	/*
	 * Acknowledge interrupt. HW interrupt remain in disabled state
	 * until our bottom half re-enables it.
	 */
	if (ioread32(dma->in.proc_status) & EIP_HW_CDR_PROC_IRQ_STATUS) {
		iowrite32(EIP_HW_CDR_PROC_IRQ_STATUS, dma->in.proc_status);
		napi_schedule(&dma->in.napi);
	}

	return IRQ_HANDLED;
}

/*
 * eip_dma_hy_handle_rdr_irq()
 *	Interrupt handler for DMA Rx.
 */
static irqreturn_t eip_dma_hy_handle_rdr_irq(int irq, void *ctx)
{
	struct eip_dma *dma = ctx;

	/*
	 * Acknowledge interrupt. HW interrupt remain in disabled state
	 * until our bottom half re-enables it.
	 */
	if (ioread32(dma->out.proc_status) & EIP_HW_RDR_PROC_IRQ_STATUS) {
		iowrite32(EIP_HW_RDR_PROC_IRQ_STATUS, dma->out.proc_status);
		napi_schedule(&dma->out.napi);
	}

	return IRQ_HANDLED;
}

/*
 * eip_dma_hy_napi_tx_poll()
 *	NAPI poll callback to handle EIP197 interrupt.
 */
static int eip_dma_hy_napi_tx_poll(struct napi_struct *napi, int budget)
{
	struct eip_dma *dma = container_of(napi, struct eip_dma, in.napi);
	int processed;

	processed = eip_dma_hy_tx_comp(dma, budget);
	if (processed < budget) {
		napi_complete(napi);
		iowrite32(EIP_HW_CDR_HIA_THR_VAL, dma->in.proc_thresh);
	}

	return processed;
}

/*
 * eip_dma_hy_napi_rx_poll()
 *	NAPI poll callback to handle EIP197 interrupt.
 */
static int eip_dma_hy_napi_rx_poll(struct napi_struct *napi, int budget)
{
	struct eip_dma *dma = container_of(napi, struct eip_dma, out.napi);
	int processed;

	/*
	 * Reap Rx ring. If we have processed all packet in queue
	 * then disable napi and re-enable irq in EIP.
	 */
	processed = eip_dma_hy_rx(dma, budget);
	if (processed < budget) {
		napi_complete(napi);
		iowrite32(EIP_HW_RDR_HIA_THR_VAL, dma->out.proc_thresh);
	}

	/*
	 * Refill new SKB.
	 */
	eip_dma_hy_refill(dma);

	return processed;
}

/*
 * eip_dma_hy_free_skb()
 *	Refill new SKB(s) in DMA output ring.
 */
static void eip_dma_hy_free_skb(struct eip_dma *dma)
{
	/*
	 * TODO: Fix
	 */
	BUG_ON(1);
}

/*
 * eip_dma_hy_tx_skb_paged()
 *	Send SKB which may be paged.
 */
static uint32_t eip_dma_hy_tx_skb_paged(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb,
		uint32_t cmd_idx, bool last)
{
	uint8_t nr_frags = skb_shinfo(skb)->nr_frags;
	skb_frag_t *skb_frag;
	struct eip_frag src;

	/*
	 * Set fragment specific information for head SKB.
	 */
	src.data = skb->data;
	src.len = skb_headlen(skb);
	src.flag = EIP_HW_CMD_FLAGS(!sw->src_nsegs, (last && !nr_frags));
	src.idx = cmd_idx;
	eip_dma_hy_tx_cmd(dma, sw, &src);
	cmd_idx = EIP_DMA_IDX_INC(cmd_idx);

	/*
	 * Handle nr_frags.
	 */
	skb_frag = &skb_shinfo(skb)->frags[0];
	while(nr_frags--) {
		src.data = skb_frag_address(skb_frag);
		src.len = skb_frag_size(skb_frag);
		src.flag = EIP_HW_CMD_FLAGS(0, (last && !nr_frags));
		src.idx = cmd_idx;
		eip_dma_hy_tx_cmd(dma, sw, &src);
		cmd_idx = EIP_DMA_IDX_INC(cmd_idx);

		skb_frag++;
	}

	return cmd_idx;
}

/*
 * eip_dma_hy_tx_linear_skb()
 *	Schedule linear SKB for transformation. The function must be called with prempt disabled.
 */
int eip_dma_hy_tx_linear_skb(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb)
{
	struct eip_tr_stats *tr_stats = this_cpu_ptr(sw->tr->stats_pcpu);
	struct eip_dma_stats *dma_stats = &dma->stats;
	struct eip_frag src;
	uint32_t in_count;

	BUG_ON(skb_is_nonlinear(skb));
	if (unlikely(!dma->active)) {
		return -ENOTSUPP;
	}

	in_count = eip_dma_avail_idx_and_count(&dma->in, &src.idx);
	if (!in_count)
		return -EBUSY;

	/*
	 * Fill Source fragment for Command ring.
	 */
	src.data = skb->data;
	src.len = skb->len;
	src.flag = EIP_HW_CMD_FLAGS(1, 1);
	sw->src_nsegs = 1;
	eip_dma_hy_tx_cmd(dma, sw, &src);

	/*
	 * Increament Tx statistics.
	 */
	tr_stats->tx_frags += 1;
	tr_stats->tx_pkts++;
	tr_stats->tx_bytes += src.len;
	dma_stats->tx_frags += 1;
	dma_stats->tx_pkts++;
	dma_stats->tx_bytes += src.len;

	eip_dma_hy_notify(&dma->in, 1, EIP_DMA_IDX_INC(src.idx));
	return 0;
}

/*
 * eip_dma_hy_tx_nonlinear_skb()
 *	Schedule non-linear SKB for transformation. The function must be called with prempt disabled.
 */
int eip_dma_hy_tx_nonlinear_skb(struct eip_dma *dma, struct eip_sw_desc *sw, struct sk_buff *skb)
{
	struct eip_tr_stats *tr_stats = this_cpu_ptr(sw->tr->stats_pcpu);
	struct eip_dma_stats *dma_stats = &dma->stats;
	struct sk_buff *iter_skb = NULL;
	unsigned int len = skb->len;
	uint32_t in_count;
	uint32_t cmd_idx;
	uint8_t desc_req;
	bool last_frag;

	BUG_ON(!skb_is_nonlinear(skb));
	if (unlikely(!dma->active)) {
		return -ENOTSUPP;
	}

	in_count = eip_dma_avail_idx_and_count(&dma->in, &cmd_idx);
	sw->src_nsegs = 0;

	/*
	 * Send Head SKB which may be paged.
	 * Descriptor required is head SKB + nr_frags in it.
	 */
	desc_req = skb_shinfo(skb)->nr_frags + 1;
	if (in_count < desc_req) {
		return -EBUSY;
	}

	last_frag = !skb_has_frag_list(skb);
	cmd_idx = eip_dma_hy_tx_skb_paged(dma, sw, skb, cmd_idx, last_frag);
	in_count -= desc_req;
	sw->src_nsegs += desc_req;

	/*
	 * Walk through all fraglist skbs.
	 */
	skb_walk_frags(skb, iter_skb) {
		desc_req = skb_shinfo(iter_skb)->nr_frags + 1;
		if (in_count < desc_req) {
			return -EBUSY;
		}

		last_frag = !iter_skb->next;
		cmd_idx = eip_dma_hy_tx_skb_paged(dma, sw, iter_skb, cmd_idx, last_frag);
		in_count -= desc_req;
		sw->src_nsegs += desc_req;
	}

	/*
	 * Increament Tx statistics.
	 */
	tr_stats->tx_frags += sw->src_nsegs;
	tr_stats->tx_pkts++;
	tr_stats->tx_bytes += len;
	dma_stats->tx_frags += sw->src_nsegs;
	dma_stats->tx_pkts++;
	dma_stats->tx_bytes += len;

	eip_dma_hy_notify(&dma->in, sw->src_nsegs, cmd_idx);
	return 0;
}

/*
 * eip_dma_hy_deinit()
 *	De-Initialize the DMA HW ring.
 */
void eip_dma_hy_deinit(struct eip_dma *dma)
{
	if (!dma->active) {
		return;
	}

	debugfs_remove_recursive(dma->dentry);
	irq_set_affinity_hint(dma->in.irq, NULL);
	free_irq(dma->in.irq, dma);
	irq_set_affinity_hint(dma->out.irq, NULL);
	free_irq(dma->out.irq, dma);
	napi_disable(&dma->in.napi);
	napi_disable(&dma->out.napi);
	netif_napi_del(&dma->in.napi);
	netif_napi_del(&dma->out.napi);
	eip_dma_hy_free_skb(dma);
	kfree(dma->out.desc);
	kfree(dma->in.desc);

	memset(dma, 0, sizeof(*dma));
}

/*
 * eip_dma_hy_init()
 *	initialize the EIP197 DMA.
 */
int eip_dma_hy_init(struct eip_dma *dma, struct platform_device *pdev, struct device_node *child, int cpu)
{
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	void __iomem *base_addr = ep->dev_vaddr;
	struct eip_dma_ring_info cmd_info = {0};
	struct eip_dma_ring_info res_info = {0};
	uint8_t ring_id;
	int status;

	if (dma->active) {
		pr_err("%px: Hybrid DMA is already configured; cpu(%u)\n", dma, cpu);
		return -EINVAL;
	}

	status = of_property_read_u8(child, "ring-id", &ring_id);
	if (status < 0) {
		pr_err("%px: Failed to read ring_id\n", pdev);
		return -EINVAL;
	}

	cmd_info.irq = of_irq_get_byname(child, "cdr_proc_irq");
	if (!cmd_info.irq) {
		pr_err("%px: irq_of_parse_and_map() fail for irq, ring_id(%u)\n", dma, ring_id);
		return -EINVAL;
	}

	res_info.irq = of_irq_get_byname(child, "rdr_proc_irq");
	if (!res_info.irq) {
		pr_err("%px: irq_of_parse_and_map() fail for irq, ring_id(%u)\n", dma, ring_id);
		return -EINVAL;
	}

	dma->type = EIP_DMA_TYPE_HYBRID;
	dma->ring_id = ring_id;
	cmd_info.base_addr = res_info.base_addr = base_addr;
	cmd_info.cpu = res_info.cpu = cpu;
	cmd_info.handler = eip_dma_hy_handle_cmd_irq;
	res_info.handler = eip_dma_hy_handle_rdr_irq;


	/*
	 * Initialize and allocate ring.
	 */
	status = eip_dma_cmd_init(dma, &cmd_info, true);
	if (status < 0) {
		pr_err("%px: Failed to initialize cmd ring(%u)\n", dma, ring_id);
		return status;

	}

	status = eip_dma_res_init(dma, &res_info, true);
	if (status < 0) {
		pr_err("%px: Failed to initialize res ring(%u)\n", dma, ring_id);
		goto fail_res;
	}

	/*
	 * Initialize NAPI for transmit completion for input ring.
	 */
	init_dummy_netdev(&dma->in.ndev);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	netif_threaded_napi_add(&dma->in.ndev, &dma->in.napi, eip_dma_hy_napi_tx_poll, eip_dma_tx_compl_napi_weight);
#else
	netif_threaded_napi_add_weight(&dma->in.ndev, &dma->in.napi, eip_dma_hy_napi_tx_poll,
			eip_dma_tx_compl_napi_weight);
#endif
	napi_enable(&dma->in.napi);

	/*
	 * Initialize NAPI for receiving packet for output ring.
	 */
	init_dummy_netdev(&dma->out.ndev);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	netif_threaded_napi_add(&dma->out.ndev, &dma->out.napi, eip_dma_hy_napi_rx_poll, eip_dma_rx_napi_weight);
#else
	netif_threaded_napi_add_weight(&dma->out.ndev, &dma->out.napi, eip_dma_hy_napi_rx_poll,
			eip_dma_rx_napi_weight);
#endif
	napi_enable(&dma->out.napi);
	ep->dma_refill_req = true;

	dma->dentry = debugfs_create_file(dma_name[ring_id], S_IRUGO, ep->dentry, dma, &dma_stats_ops);
	dma->active = 1;
	pr_info("%px: DMA ring(%u) configured and Rx mapped to cpu(%u)\n", dma, ring_id, cpu);
	return 0;

fail_res:
	irq_set_affinity_hint(dma->in.irq, NULL);
	free_irq(dma->in.irq, dma);
	kfree(dma->in.kaddr);
	return status;
}
