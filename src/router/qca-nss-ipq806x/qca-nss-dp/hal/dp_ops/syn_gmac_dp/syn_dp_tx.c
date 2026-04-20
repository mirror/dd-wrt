/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/netdevice.h>
#include <nss_dp_dev.h>
#include <asm/cacheflush.h>
#include "syn_dma_reg.h"

/*
 * syn_dp_tx_error_cnt()
 *	Set the error counters.
 */
static inline void syn_dp_tx_error_cnt(struct syn_dp_info_tx *tx_info, uint32_t status)
{
	atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_errors);
	(status & DESC_TX_TIMEOUT) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_jabber_timeout_errors) : 0;
	(status & DESC_TX_FRAME_FLUSHED) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_frame_flushed_errors) : 0;
	(status & DESC_TX_LOST_CARRIER) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_loss_of_carrier_errors) : 0;
	(status & DESC_TX_NO_CARRIER) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_no_carrier_errors) : 0;
	(status & DESC_TX_LATE_COLLISION) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_late_collision_errors) : 0;
	(status & DESC_TX_EXC_COLLISIONS) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_excessive_collision_errors) : 0;
	(status & DESC_TX_EXC_DEFERRAL) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_excessive_deferral_errors) : 0;
	(status & DESC_TX_UNDERFLOW) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_underflow_errors) : 0;
	(status & DESC_TX_IPV4_CHK_ERROR) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_ip_header_errors) : 0;
	(status & DESC_TX_PAY_CHK_ERROR) ? atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_ip_payload_errors) : 0;
}

/*
 * syn_dp_tx_clear_buf_entry()
 *	Clear the Tx info after Tx is over.
 */
static inline void syn_dp_tx_clear_buf_entry(struct syn_dp_info_tx *tx_info, uint32_t tx_skb_index)
{
	tx_info->tx_buf_pool[tx_skb_index].len = 0;
	tx_info->tx_buf_pool[tx_skb_index].skb = NULL;
	tx_info->tx_buf_pool[tx_skb_index].shinfo_addr_virt = 0;
}

/*
 * syn_dp_tx_set_desc_sg()
 *	Populate the tx desc structure with the buffer address for SG packets
 */
static inline struct dma_desc_tx *syn_dp_tx_set_desc_sg(struct syn_dp_info_tx *tx_info,
					   uint32_t buffer, unsigned int length, uint32_t status)
{
	uint32_t tx_idx = tx_info->tx_idx;
	struct dma_desc_tx *txdesc = tx_info->tx_desc + tx_idx;

#ifdef SYN_DP_DEBUG
	BUG_ON(atomic_read((atomic_t *)&tx_info->busy_tx_desc_cnt) > SYN_DP_TX_DESC_SIZE);
	BUG_ON(txdesc != (tx_info->tx_desc + tx_idx));
	BUG_ON(!syn_dp_gmac_is_tx_desc_empty(txdesc));
	BUG_ON(syn_dp_gmac_is_tx_desc_owned_by_dma(txdesc));
#endif

	if (likely(length <= SYN_DP_MAX_DESC_BUFF_LEN)) {
		txdesc->length  = ((length << DESC_SIZE1_SHIFT) & DESC_SIZE1_MASK);
		txdesc->buffer2 = 0;
	} else {
		txdesc->length  = (SYN_DP_MAX_DESC_BUFF_LEN << DESC_SIZE1_SHIFT) & DESC_SIZE1_MASK;
		txdesc->length |= ((length - SYN_DP_MAX_DESC_BUFF_LEN) << DESC_SIZE2_SHIFT) & DESC_SIZE2_MASK;
		txdesc->buffer2 = buffer + SYN_DP_MAX_DESC_BUFF_LEN;
	}

	txdesc->buffer1 = buffer;

	txdesc->status = (status | ((tx_idx == (SYN_DP_TX_DESC_SIZE - 1)) ? DESC_TX_DESC_END_OF_RING : 0));

	tx_info->tx_idx = syn_dp_tx_inc_index(tx_idx, 1);
	return txdesc;
}

/*
 * syn_dp_tx_process_nr_frags()
 *	Process nr frags for the SG packets
 */
static inline struct dma_desc_tx *syn_dp_tx_process_nr_frags(struct syn_dp_info_tx *tx_info,
				struct sk_buff *skb, uint32_t *total_length, uint32_t nr_frags)
{
	struct dma_desc_tx *tx_desc;
	dma_addr_t dma_addr;
	unsigned int length, i = 0;

	do {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[i++];
		void *frag_addr = skb_frag_address(frag);
		length = skb_frag_size(frag);

#ifdef SYN_DP_DEBUG
		BUG_ON(!length);
#endif

		dma_addr = (dma_addr_t)virt_to_phys(frag_addr);

		dmac_clean_range_no_dsb(frag_addr, frag_addr + length);

		*total_length += length;
		tx_desc = syn_dp_tx_set_desc_sg(tx_info, dma_addr, length, DESC_OWN_BY_DMA);
	} while ( i < nr_frags);

	return tx_desc;
}

/*
 * syn_dp_tx_nr_frags()
 *	TX routine for Synopsys GMAC SG packets with nr frags
 */
int syn_dp_tx_nr_frags(struct syn_dp_info_tx *tx_info, struct sk_buff *skb)
{
	dma_addr_t dma_addr;
	unsigned int length = skb_headlen(skb);
	struct dma_desc_tx *first_desc, *tx_desc;
	unsigned int desc_needed = 0, total_len = 0;
	unsigned int nr_frags = skb_shinfo(skb)->nr_frags;
	uint32_t last_idx;

#ifdef SYN_DP_DEBUG
	BUG_ON(nr_frags > MAX_SKB_FRAGS);
	BUG_ON(!length);
#endif

	/*
	 * Total descriptor needed will be sum of number of nr_frags and head skb.
	 */
	desc_needed = 1 + nr_frags;

	/*
	 * If we don't have enough tx descriptor for this pkt, return busy.
	 */
	if (unlikely((SYN_DP_TX_DESC_SIZE - atomic_read((atomic_t *)&tx_info->busy_tx_desc_cnt)) < desc_needed)) {
		atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_desc_not_avail);
		netdev_dbg(tx_info->netdev, "Not enough descriptors available %d", desc_needed);
		return NETDEV_TX_BUSY;
	}

	/*
	 * Flush the dma for non-paged skb data
	 */
	dma_addr = (dma_addr_t)virt_to_phys(skb->data);
	dmac_clean_range_no_dsb((void *)skb->data, (void *)(skb->data + length));

	total_len = length;

	/*
	 * Fill the non paged data (skb->data) in the first descriptor.
	 * We defer setting the desc_own_by_dma for first fragment until
	 * all the descriptors for this frame are ready.
	 */
	first_desc = syn_dp_tx_set_desc_sg(tx_info, dma_addr, length, DESC_TX_FIRST);

	/*
	 * Fill other fragments which are part of nr_frags in the remaining descriptors
	 * and returns the last descriptor.
	 */
	tx_desc = syn_dp_tx_process_nr_frags(tx_info, skb, &total_len, nr_frags);

	/*
	 * Save the tx index of the last descriptor of the segment
	 */
	last_idx = ((tx_info->tx_idx - 1) & SYN_DP_TX_DESC_MAX_INDEX);


	/*
	 * Fill the buffer pool in the last segment of the fragment only
	 * instead of filling in all the descriptors for the fragments
	 */
	tx_info->tx_buf_pool[last_idx].skb = skb;
	tx_info->tx_buf_pool[last_idx].len = total_len;
	tx_info->tx_buf_pool[last_idx].shinfo_addr_virt = (size_t)skb->end;

	/*
	 * For the last fragment, Enable the interrupt and set LS bit
	 */
	tx_desc->status |= (DESC_TX_LAST | DESC_TX_INT_ENABLE);

	/*
	 * Ensure all write completed before setting own by dma bit so when gmac
	 * HW takeover this descriptor, all the fields are filled correctly
	 */
	wmb();

	/*
	 * We've now written all of the descriptors in our scatter-gather list
	 * and need to write the "OWN" bit of the first one to mark the chain
	 * as available to the DMA engine. Also, add checksum calculation bit
	 * for the first descriptor if needed
	 */
	first_desc->status |= (DESC_OWN_BY_DMA | ((skb->ip_summed == CHECKSUM_PARTIAL) ? DESC_TX_CIS_TCP_PSEUDO_CS : 0));

	atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_nr_frags_pkts);
	atomic_add(desc_needed, (atomic_t *)&tx_info->busy_tx_desc_cnt);
	syn_resume_dma_tx(tx_info->mac_base);

	return 0;
}

/*
 * syn_dp_tx_frag_list()
 *	TX routine for Synopsys GMAC SG packets with frag lists
 */
int syn_dp_tx_frag_list(struct syn_dp_info_tx *tx_info, struct sk_buff *skb)
{
	dma_addr_t dma_addr;
	struct sk_buff *iter_skb;
	struct dma_desc_tx *first_desc, *tx_desc;
	unsigned int length = skb_headlen(skb);
	unsigned int desc_needed = 1, total_len = 0;
	unsigned int nr_frags = skb_shinfo(skb)->nr_frags, fraglist_nr_frags = 0;
	uint32_t last_idx;

	/*
	 * When skb is fragmented, count the number of descriptors needed
	 */
	if (unlikely(nr_frags)) {
#ifdef SYN_DP_DEBUG
		BUG_ON(nr_frags > MAX_SKB_FRAGS);
		BUG_ON(!length);
#endif
		desc_needed += nr_frags;
	}

	/*
	 * Walk through fraglist skbs, also making note of nr_frags
	 */
	skb_walk_frags(skb, iter_skb) {
		fraglist_nr_frags = skb_shinfo(iter_skb)->nr_frags;

#ifdef SYN_DP_DEBUG
		/* It is unlikely below check hits, BUG_ON */
		BUG_ON(fraglist_nr_frags > MAX_SKB_FRAGS);
#endif
		/* One descriptor for skb->data and more for nr_frags */
		desc_needed +=  (1 + fraglist_nr_frags);
	}

	/*
	 * If we don't have enough tx descriptor for this pkt, return busy.
	 */
	if (unlikely((SYN_DP_TX_DESC_SIZE - atomic_read((atomic_t *)&tx_info->busy_tx_desc_cnt)) < desc_needed)) {
		atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_desc_not_avail);
		netdev_dbg(tx_info->netdev, "Not enough descriptors available %d", desc_needed);
		return NETDEV_TX_BUSY;
	}

	dma_addr = (dma_addr_t)virt_to_phys(skb->data);

	/*
	 * Flush the data area of the head skb
	 */
	dmac_clean_range_no_dsb((void *)skb->data, (void *)(skb->data + length));

	total_len = length;

	/*
	 * Fill the first skb of the frag list chain in the first descriptor
	 * We defer setting the desc_own_by_dma bit for first fragment until
	 * all the descriptors for this frame are ready.
	 */
	first_desc = syn_dp_tx_set_desc_sg(tx_info, dma_addr, length, DESC_TX_FIRST);

	/*
	 * Fill other fragments which are part of nr_frags in the remaining descriptors
	 * and returns the last descriptor.
	 */
	if (unlikely(nr_frags)) {
		tx_desc = syn_dp_tx_process_nr_frags(tx_info, skb, &total_len, nr_frags);
	}

	/*
	 * Walk through fraglist skbs, also making note of nr_frags and filling it in descriptors
	 */
	skb_walk_frags(skb, iter_skb) {
		length = skb_headlen(iter_skb);

#ifdef SYN_DP_DEBUG
		BUG_ON(!length);
#endif

		dma_addr = (dma_addr_t)virt_to_phys(iter_skb->data);

		dmac_clean_range_no_dsb((void *)iter_skb->data, (void *)(iter_skb->data + length));

		total_len += length;

		/*
		 * Fill the non paged data skb->data.
		 */
		tx_desc = syn_dp_tx_set_desc_sg(tx_info, dma_addr, length, DESC_OWN_BY_DMA);

		/*
		 * Check if nr_frags is available for the skb. If so, fill the
		 * fragments in the descriptors else, continue
		 */
		fraglist_nr_frags = skb_shinfo(iter_skb)->nr_frags;

		/*
		 * Fill other fragments which are part of nr_frags in the remaining descriptors
		 * and returns the last descriptor.
		 */
		if (unlikely(fraglist_nr_frags)) {
			tx_desc = syn_dp_tx_process_nr_frags(tx_info, iter_skb, &total_len, fraglist_nr_frags);
		}
	}

	/*
	 * Save the tx index of the last descriptor of the segment
	 */
	last_idx = ((tx_info->tx_idx - 1) & SYN_DP_TX_DESC_MAX_INDEX);


	/*
	 * Fill the buffer pool in the last segment of the fragment only
	 * instead of filling in all the descriptors for the fragments
	 */
	tx_info->tx_buf_pool[last_idx].skb = skb;
	tx_info->tx_buf_pool[last_idx].len = total_len;
	tx_info->tx_buf_pool[last_idx].shinfo_addr_virt = (size_t)skb->end;

	/*
	 * For the last fragment, Enable the interrupt and set LS bit
	 */
	tx_desc->status |= (DESC_TX_LAST | DESC_TX_INT_ENABLE);

	/*
	 * Ensure all write completed before setting own by dma bit so when gmac
	 * HW takeover this descriptor, all the fields are filled correctly
	 */
	wmb();

	/*
	 * We've now written all of the descriptors in our scatter-gather list
	 * and need to write the "OWN" bit of the first one to mark the chain
	 * as available to the DMA engine. Also, add checksum calculation
	 * bit in the first descriptor if needed.
	 */
	first_desc->status |= (DESC_OWN_BY_DMA | ((skb->ip_summed == CHECKSUM_PARTIAL) ? DESC_TX_CIS_TCP_PSEUDO_CS : 0));

	atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_fraglist_pkts);
	atomic_add(desc_needed, (atomic_t *)&tx_info->busy_tx_desc_cnt);
	syn_resume_dma_tx(tx_info->mac_base);

	return 0;
}

/*
 * syn_dp_tx_set_desc()
 *	Populate the tx desc structure with the buffer address.
 */
static inline void syn_dp_tx_set_desc(struct syn_dp_info_tx *tx_info,
					   uint32_t buffer, struct sk_buff *skb, uint32_t offload_needed,
					   uint32_t status)
{
	uint32_t tx_idx = tx_info->tx_idx;
	struct dma_desc_tx *txdesc = tx_info->tx_desc + tx_idx;
	unsigned int length = skb->len;

#ifdef SYN_DP_DEBUG
	BUG_ON(atomic_read((atomic_t *)&tx_info->busy_tx_desc_cnt) > SYN_DP_TX_DESC_SIZE);
	BUG_ON(txdesc != (tx_info->tx_desc + tx_idx));
	BUG_ON(!syn_dp_gmac_is_tx_desc_empty(txdesc));
	BUG_ON(syn_dp_gmac_is_tx_desc_owned_by_dma(txdesc));
#endif

	if (likely(length <= SYN_DP_MAX_DESC_BUFF_LEN)) {
		txdesc->length = ((length << DESC_SIZE1_SHIFT) & DESC_SIZE1_MASK);
		txdesc->buffer2 = 0;
	} else {
		txdesc->length = (SYN_DP_MAX_DESC_BUFF_LEN << DESC_SIZE1_SHIFT) & DESC_SIZE1_MASK;
		txdesc->length |= ((length - SYN_DP_MAX_DESC_BUFF_LEN) << DESC_SIZE2_SHIFT) & DESC_SIZE2_MASK;
		txdesc->buffer2 = buffer + SYN_DP_MAX_DESC_BUFF_LEN;
	}

	txdesc->buffer1 = buffer;

	tx_info->tx_buf_pool[tx_idx].skb = skb;
	tx_info->tx_buf_pool[tx_idx].len = length;
	tx_info->tx_buf_pool[tx_idx].shinfo_addr_virt = (size_t)skb->end;

	/*
	 * Ensure all write completed before setting own by dma bit so when gmac
	 * HW takeover this descriptor, all the fields are filled correctly
	 */
	wmb();

	txdesc->status = (status | ((offload_needed) ? DESC_TX_CIS_TCP_PSEUDO_CS : 0) | ((tx_idx == (SYN_DP_TX_DESC_SIZE - 1)) ? DESC_TX_DESC_END_OF_RING : 0));

	tx_info->tx_idx = syn_dp_tx_inc_index(tx_idx, 1);
}

/*
 * syn_dp_tx_complete()
 *	Xmit complete, clear descriptor and free the skb
 */
int syn_dp_tx_complete(struct syn_dp_info_tx *tx_info, int budget)
{
	int busy;
	uint32_t status;
	struct dma_desc_tx *desc = NULL;
	struct sk_buff *skb;
	uint32_t tx_skb_index, len;
	uint32_t tx_packets = 0, total_len = 0;
	uint32_t count = 0;
	uint32_t free_idx;
	struct syn_dp_tx_buf *tx_buf;
	struct netdev_queue *nq;

	busy = atomic_read((atomic_t *)&tx_info->busy_tx_desc_cnt);

	if (unlikely(!busy)) {

		/*
		 * No descriptors are held by GMAC DMA, we are done
		 */
		netdev_dbg(tx_info->netdev, "No descriptors held by DMA");
		return 0;
	}

	if (likely(busy > budget)) {
		busy = budget;
	}

	tx_skb_index = syn_dp_tx_comp_index_get(tx_info);
	do {
		desc = syn_dp_tx_comp_desc_get(tx_info);
		status = desc->status;
		if (unlikely(syn_dp_gmac_is_tx_desc_owned_by_dma(status))) {

			/*
			 * Descriptor still held by gmac dma, so we are done.
			 */
			break;
		}


		if (likely(status & DESC_TX_LAST)) {
			tx_skb_index = syn_dp_tx_comp_index_get(tx_info);
			tx_buf = &tx_info->tx_buf_pool[tx_skb_index];
			skb = tx_info->skb_free_list[count] = tx_buf->skb;
			len = tx_buf->len;

#ifdef SYN_DP_DEBUG
			BUG_ON(!skb);
#endif

			tx_info->shinfo_addr_virt[count++] = tx_buf->shinfo_addr_virt;
			syn_dp_tx_clear_buf_entry(tx_info, tx_skb_index);

			if (likely(!(status & DESC_TX_ERROR))) {

				/*
				 * No error, record tx pkts/bytes and collision.
				 */
				tx_packets++;
				total_len += len;
			}
		}

		if (unlikely(status & DESC_TX_ERROR)) {
			/*
			 * Some error happened, collect error statistics.
			 */
			syn_dp_tx_error_cnt(tx_info, status);
		}

		tx_info->tx_comp_idx = syn_dp_tx_inc_index(tx_info->tx_comp_idx, 1);

		/*
		 * Busy is used to count the workdone with assigned budget.
		 */
		atomic_dec((atomic_t *)&tx_info->busy_tx_desc_cnt);
	} while (--busy);

	/*
	 * Prefetching the shinfo area before releasing to skb recycler gives benefit
	 * in performance.
	 * All the completed skb's shinfo are prefetched and skb's are freed in batch.
	 */
	for (free_idx = 0; free_idx < count; free_idx++) {
		if (likely((free_idx + 1) < count)) {
			prefetch((void *)tx_info->shinfo_addr_virt[free_idx+1]);
		}
		dev_kfree_skb_any(tx_info->skb_free_list[free_idx]);
		tx_info->skb_free_list[free_idx] = NULL;
		tx_info->shinfo_addr_virt[free_idx] = 0;
	}

	atomic64_add(tx_packets, (atomic64_t *)&tx_info->tx_stats.tx_packets);
	atomic64_add(total_len, (atomic64_t *)&tx_info->tx_stats.tx_bytes);

	nq = netdev_get_tx_queue(tx_info->netdev, SYN_DP_QUEUE_INDEX);

	/*
	 * Wake up queue if stopped earlier due to lack of descriptors
	 */
	if (unlikely(netif_tx_queue_stopped(nq)) && netif_carrier_ok(tx_info->netdev)) {
		netif_wake_queue(tx_info->netdev);
	}

	return budget - busy;
}

/*
 * syn_dp_tx_sg()
 *	Tx routine for Synopsys GMAC scatter gather packets
 */
int syn_dp_tx_sg(struct syn_dp_info_tx *tx_info, struct sk_buff *skb)
{
	struct net_device *netdev = tx_info->netdev;
	bool has_frag_list;
	unsigned int nr_frags;

	has_frag_list = skb_has_frag_list(skb);
	nr_frags = skb_shinfo(skb)->nr_frags;

	/*
	 * Handle SG for below skb types.
	 * 1. skb has fraglist
	 * 2. skb has fraglist and any of the fragments can have nr_frags
	 */
	if (has_frag_list) {
		return syn_dp_tx_frag_list(tx_info, skb);
	}

	/*
	 * Handle SG for nr_frags
	 */
	if (nr_frags) {
		return syn_dp_tx_nr_frags(tx_info, skb);
	}

	netdev_dbg(netdev, "Not a valid non-linear packet");
	return -1;
}

/*
 * syn_dp_tx()
 *	TX routine for Synopsys GMAC
 */
int syn_dp_tx(struct syn_dp_info_tx *tx_info, struct sk_buff *skb)
{
	struct net_device *netdev = tx_info->netdev;
	dma_addr_t dma_addr;

	/*
	 * Check if it's a Scatter Gather packet
	 */
	if (unlikely(skb_is_nonlinear(skb))) {
		return syn_dp_tx_sg(tx_info, skb);
	}

	/*
	 * Linear skb processing
	 */
	if (unlikely((SYN_DP_TX_DESC_SIZE - atomic_read((atomic_t *)&tx_info->busy_tx_desc_cnt)) < 1)) {
		atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_desc_not_avail);
		netdev_dbg(netdev, "Not enough descriptors available");
		return NETDEV_TX_BUSY;
	}

	dma_addr = (dma_addr_t)virt_to_phys(skb->data);

	dmac_clean_range_no_dsb((void *)skb->data, (void *)(skb->data + skb->len));

	/*
	 * Queue packet to the GMAC rings
	 */
	syn_dp_tx_set_desc(tx_info, dma_addr, skb, (skb->ip_summed == CHECKSUM_PARTIAL),
			(DESC_TX_LAST | DESC_TX_FIRST | DESC_TX_INT_ENABLE | DESC_OWN_BY_DMA));

	syn_resume_dma_tx(tx_info->mac_base);
	atomic_inc((atomic_t *)&tx_info->busy_tx_desc_cnt);
	return 0;
}
