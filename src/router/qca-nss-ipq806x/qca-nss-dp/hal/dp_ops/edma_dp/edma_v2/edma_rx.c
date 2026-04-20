/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <asm/cacheflush.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <ppe_drv_public.h>
#include <nss_dp_vp.h>
#include <linux/phy.h>
#include "edma.h"
#include "edma_debug.h"
#include "edma_regs.h"
#include "nss_dp_dev.h"

extern nss_dp_vp_rx_cb_t nss_dp_vp_rx_reg_cb;
extern nss_dp_vp_list_rx_cb_t nss_dp_vp_list_rx_reg_cb;
extern struct nss_dp_vp_skb_list gvp_skb_list[];

/*
 * edma_rx_process_capwap_vp()
 *	Forward capwap packet to VP module for processing.
 */
static inline void edma_rx_process_capwap_vp(struct edma_rxdesc_ring *rxdesc_ring, struct edma_rxdesc_desc *rxdesc_desc, struct sk_buff *skb)
{
	uint32_t dst_port;
	struct nss_dp_vp_skb_list *vsl;
	uint8_t dvp, vpi;

	dst_port = EDMA_RXDESC_DST_INFO_GET(rxdesc_desc);
	if (unlikely((dst_port & ~EDMA_RXDESC_DST_PORT_ID_MASK) != EDMA_RXDESC_DST_PORT)) {

		struct edma_pcpu_stats *pcpu_stats;
		struct edma_rx_stats *rx_stats;
		struct nss_dp_dev *vp_dev;
		edma_warn(" Non-vp packet received on capwap ring skb:%px\n", skb);
		vp_dev = netdev_priv(skb->dev);
		dev_kfree_skb_any(skb);
		pcpu_stats = &vp_dev->dp_info.pcpu_stats;
		rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);
		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_vp_uninitialized++;
		u64_stats_update_end(&rx_stats->syncp);
		return;
	}

	dvp = EDMA_RXDESC_DST_PORT_ID_GET(rxdesc_desc);
	skb->ip_summed = CHECKSUM_COMPLETE;

	vpi = dvp - PPE_DRV_VIRTUAL_START;
	vsl = &gvp_skb_list[vpi];

	/*
	 * First packet seen for this VP in this iteration.
	 * Add the list to rxdesc_ring->vp_head
	 */
	if (unlikely(!vsl->len)) {
		skb_queue_head_init(&vsl->skb_list);
		vsl->next = rxdesc_ring->vp_head;
		rxdesc_ring->vp_head = vsl;
		vsl->dvp = dvp;
	}

	vsl->len += skb->len;
	__skb_queue_tail(&vsl->skb_list, skb);

	return;
}

/*
 * edma_rx_checksum_verify()
 *	get hw checksum status
 */
static inline uint8_t edma_rx_checksum_verify(struct edma_rxdesc_desc *rxdesc_desc,
							struct sk_buff* skb)
{
	uint8_t pid = EDMA_RXDESC_PID_GET(rxdesc_desc);

	skb_checksum_none_assert(skb);

	if (likely(EDMA_RX_PID_IS_IPV4(pid))) {
		if (likely(EDMA_RXDESC_L3CSUM_STATUS_GET(rxdesc_desc))
			&& likely(EDMA_RXDESC_L4CSUM_STATUS_GET(rxdesc_desc))) {
			return CHECKSUM_UNNECESSARY;
		}
	} else if (likely(EDMA_RX_PID_IS_IPV6(pid))) {
		if (likely(EDMA_RXDESC_L4CSUM_STATUS_GET(rxdesc_desc))) {
			return CHECKSUM_UNNECESSARY;
		}
	}

	return skb->ip_summed;
}

/*
 * edma_rx_process_vp()
 *	Forward packet to VP module for processing.
 */
static inline void edma_rx_process_vp(struct edma_rxdesc_desc *rxdesc_desc, struct edma_rxdesc_ring *rxdesc_ring, struct sk_buff *skb)
{
	uint32_t dst_port;
	nss_dp_vp_rx_cb_t edma_rx_vp_cb;
	struct nss_dp_vp_rx_info vprxi;

	rcu_read_lock();
	edma_rx_vp_cb = rcu_dereference(nss_dp_vp_rx_reg_cb);
	if (unlikely(!edma_rx_vp_cb)) {
		struct edma_pcpu_stats *pcpu_stats;
		struct edma_rx_stats *rx_stats;
		struct nss_dp_dev *vp_dev;

		rcu_read_unlock();
		if (net_ratelimit()) {
			edma_warn("VP packet recieved but edma vp callback \
					not registered yet, skb:%px\n", skb);
		}

		vp_dev = netdev_priv(skb->dev);
		dev_kfree_skb_any(skb);

		pcpu_stats = &vp_dev->dp_info.pcpu_stats;
		rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);
		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_vp_uninitialized++;
		u64_stats_update_end(&rx_stats->syncp);
		return;
	}

	dst_port = EDMA_RXDESC_DST_INFO_GET(rxdesc_desc);
	if (likely((dst_port & ~EDMA_RXDESC_DST_PORT_ID_MASK) == EDMA_RXDESC_DST_PORT)) {
		vprxi.dvp = EDMA_RXDESC_DST_PORT_ID_GET(rxdesc_desc);
	} else {
		vprxi.dvp = 0;
	}

	vprxi.l3offset = EDMA_RXDESC_L3_OFFSET_GET(rxdesc_desc);
	vprxi.svp = EDMA_RXDESC_SRC_INFO_GET(rxdesc_desc) & EDMA_RXDESC_PORTNUM_BITS;
	vprxi.napi = &rxdesc_ring->napi;
	vprxi.ip_summed = edma_rx_checksum_verify(rxdesc_desc, skb);

	/*
	 * Pass the packet to VP to process
	 */
	edma_rx_vp_cb(skb, &vprxi);
	rcu_read_unlock();
}

/*
 * edma_rx_alloc_buffer_list()
 *	Write a given list of Rx buffers to the Rx fill ring
 */
static inline int edma_rx_alloc_buffer_list(struct edma_rxfill_ring *rxfill_ring, int alloc_count)
{
	struct edma_rxfill_desc *rxfill_desc;
	struct edma_rx_fill_stats *rxfill_stats = &rxfill_ring->rx_fill_stats;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	struct list_head rx_skb_alloc;
	uint16_t prod_idx, start_idx, cons_idx;
	uint16_t num_alloc = 0;
	uint16_t avail_desc = 0;
	uint32_t rx_alloc_size = rxfill_ring->alloc_size;
	uint32_t buf_len = rxfill_ring->buf_len;
	bool page_mode = rxfill_ring->page_mode;
	INIT_LIST_HEAD(&rx_skb_alloc);

	/*
	 * Get RXFILL ring producer index
	 */
	prod_idx = rxfill_ring->prod_idx;
	start_idx = prod_idx;

	/*
	 * When tracking ring util stats is enabled via procfs,
	 * we will compute avail desc and compute how much percentage the ring is full.
	 * Above stats are maintained at ring level.
	 */
	if (unlikely(egc->enable_ring_util_stats)) {
		cons_idx = edma_reg_read(EDMA_REG_RXFILL_CONS_IDX(rxfill_ring->ring_id)) & EDMA_RXFILL_CONS_IDX_MASK;
		avail_desc = EDMA_DESC_AVAIL_COUNT(cons_idx, prod_idx, EDMA_RX_RING_SIZE);

		edma_update_ring_stats(avail_desc, EDMA_RX_RING_SIZE,
				       &rxfill_ring->rx_fill_stats.ring_stats);
	}

	while (likely(alloc_count--)) {
		struct sk_buff *skb_alloc;

		/*
		 * Allocate one skb and add to refill list
		 */
		skb_alloc = netdev_alloc_skb_fast(NULL, rx_alloc_size);
		if (likely(skb_alloc)) {
			list_add_tail(&skb_alloc->list, &rx_skb_alloc);
			num_alloc++;
		} else {
			u64_stats_update_begin(&rxfill_stats->syncp);
			++rxfill_stats->alloc_failed;
			u64_stats_update_end(&rxfill_stats->syncp);
		}
	}

	while (likely(!list_empty(&rx_skb_alloc))) {
		void *page_addr = NULL;
		struct page *pg;
		struct sk_buff *skb;
		dma_addr_t buff_addr;

		/*
		 * Get RXFILL descriptor
		 */
		rxfill_desc = EDMA_RXFILL_DESC(rxfill_ring, prod_idx);

		/*
		 * Detach the current SKB to use from the list,
		 * and prefetch the next SKB's cache lines.
		 */
		skb = list_first_entry(&rx_skb_alloc, struct sk_buff, list);
		if (likely(!list_entry_is_head(skb->next, &rx_skb_alloc, list))) {
			prefetch(skb->next);
			prefetch((uint8_t *)(skb->next) + 128);
			prefetch((uint8_t *)(skb->next) + 192);
		}
		list_del_init(&skb->list);
		skb->next = skb->prev = NULL;

		/*
		 * Reserve headroom
		 */
		skb_reserve(skb, EDMA_RX_SKB_HEADROOM + NET_IP_ALIGN);

		/*
		 * Map Rx buffer for DMA
		 */
		if (likely(!page_mode)) {
			buff_addr = (dma_addr_t)virt_to_phys(skb->data);
		} else {
			pg = alloc_page(GFP_ATOMIC);
			if (unlikely(!pg)) {
				u64_stats_update_begin(&rxfill_stats->syncp);
				++rxfill_stats->page_alloc_failed;
				u64_stats_update_end(&rxfill_stats->syncp);
				dev_kfree_skb_any(skb);
				edma_debug("edma_gbl_ctx:%px Unable to allocate page", &edma_gbl_ctx);
				break;
			}

			/*
			 * Get virtual address of allocated page
			 */
			page_addr = page_address(pg);
			buff_addr = (dma_addr_t)virt_to_phys(page_addr);
			skb_fill_page_desc(skb, 0, pg, 0, PAGE_SIZE);
			dmac_inv_range_no_dsb(page_addr, (page_addr + PAGE_SIZE));
		}

		EDMA_RXFILL_BUFFER_ADDR_SET(rxfill_desc, buff_addr);

		/*
		 * Store skb in opaque
		 */
		EDMA_RXFILL_OPAQUE_LO_SET(rxfill_desc, skb);
#ifdef __LP64__
		EDMA_RXFILL_OPAQUE_HI_SET(rxfill_desc, skb);
#endif

		/*
		 * Save buffer size in RXFILL descriptor
		 */
		EDMA_RXFILL_PACKET_LEN_SET(rxfill_desc,
				((uint32_t)(buf_len) & EDMA_RXFILL_BUF_SIZE_MASK));

		/*
		 * Invalidate skb->data
		 * A73 flush operation does an invalidate operation as well.
		 * If the packet is fast transmitted and hence fast recycled,
		 * we can be assured that invalidate was already done at the
		 * time of previous transmit
		 */
		if (unlikely(!skb->fast_recycled)) {
			dmac_inv_range_no_dsb((void *)skb->data,
					      (void *)(skb->data + rx_alloc_size -
					      EDMA_RX_SKB_HEADROOM -
					      NET_IP_ALIGN));

		}
		skb->fast_recycled = 0;

		prod_idx = (prod_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/*
		 * Perform endianness conversion before writing to HW
		 */
		EDMA_RXFILL_ENDIAN_SET(rxfill_desc);
	}

	if (likely(num_alloc)) {

		/*
		 * Make sure the information written to the descriptors
		 * is updated before writing to the hardware.
		 */
		dsb(st);

		edma_reg_write(EDMA_REG_RXFILL_PROD_IDX(rxfill_ring->ring_id),
								prod_idx);
		rxfill_ring->prod_idx = prod_idx;
	}

	return num_alloc;
}

/*
 * edma_rx_alloc_buffer()
 *	Alloc Rx buffers for one RxFill ring
 */
int edma_rx_alloc_buffer(struct edma_rxfill_ring *rxfill_ring, int alloc_count)
{
	return edma_rx_alloc_buffer_list(rxfill_ring, alloc_count);
}

/*
 * edma_rx_sawf_sc_stats_update()
 *	Update per service-class stats.
 */
static inline void edma_rx_sawf_sc_stats_update(uint64_t pkt_length, struct edma_sawf_sc_stats *sawf_sc_stats)
{
	u64_stats_update_begin(&sawf_sc_stats->syncp);
	sawf_sc_stats->rx_bytes += pkt_length;
	sawf_sc_stats->rx_packets++;
	u64_stats_update_end(&sawf_sc_stats->syncp);
}

/*
 * edma_rx_handle_wifi_qos_packets()
 *	Handle packets with wifi qos enabled.
 */
static void edma_rx_handle_wifi_qos_packets(struct edma_gbl_ctx *egc, struct edma_rxdesc_ring *rxdesc_ring, struct edma_rxdesc_desc *rxdesc_head, struct sk_buff *skb)
{
	uint16_t desc_index, peer_id, next_desc_index;
	uint8_t service_class, wifi_qos;
	struct edma_rxdesc_sec_desc *rxdesc_sec, *next_rxdesc_sec;
	ppe_drv_tree_id_type_t tree_id_type;
	uint32_t mlo_mark;

	desc_index = ((uint8_t *)rxdesc_head - (uint8_t *)rxdesc_ring->pdesc) >> EDMA_RXDESC_SIZE_SHIFT;
	rxdesc_sec = EDMA_RXDESC_SEC_DESC(rxdesc_ring, desc_index);

	/*
	 * Depending on the use-case, sometime PPE generate the same CPU
	 * code for every packet, prefetch the next secondary descriptor
	 * to handle such cases.
	 */
	next_desc_index = (desc_index + 1) & EDMA_RX_RING_SIZE_MASK;
	next_rxdesc_sec = EDMA_RXDESC_SEC_DESC(rxdesc_ring, next_desc_index);
	prefetch(next_rxdesc_sec);

	tree_id_type = EDMA_RXDESC_TREE_ID_TYPE_GET(rxdesc_sec);

	switch (tree_id_type) {
	case PPE_DRV_TREE_ID_TYPE_NONE:
		/*
		 * TODO: Get the remaining 20-bit tree_id and process.
		 */
		break;
	case PPE_DRV_TREE_ID_TYPE_SAWF:
		/*
		 * In case of SAWF, fetch the SAWF metadata from Tree ID.
		 */
		service_class = EDMA_RXDESC_SERVICE_CLASS_GET(rxdesc_sec);
		peer_id = EDMA_RXDESC_PEER_ID_GET(rxdesc_sec);
		wifi_qos = EDMA_RXDESC_WIFI_QOS_GET(rxdesc_head);

		/*
		 * Update stats for the SAWF service class.
		 */
		BUG_ON(!PPE_DRV_SERVICE_CLASS_IS_VALID(service_class));
		edma_rx_sawf_sc_stats_update(skb->len, &egc->sawf_sc_stats[service_class]);
		/*
		 * Configure skb->mark with SAWF metadata.
		 */
		skb->mark = EDMA_RX_SAWF_METADATA_CONSTRUCT(service_class, peer_id, wifi_qos);

		edma_debug("%px : SAWF mark configured = 0x%x\n", egc, skb->mark);
		break;

	case PPE_DRV_TREE_ID_TYPE_SCS:
		/*
		 * In case of SCS, fetch the wifi_qos from Tree ID.
		 */
		wifi_qos = EDMA_RXDESC_WIFI_QOS_GET(rxdesc_head);

		/*
		 * Configure skb->mark with wifi_qos metadata.
		 */
		skb->mark = wifi_qos;

		edma_debug("%px : SCS mark configured = 0x%x\n", egc, skb->mark);
		break;

	case PPE_DRV_TREE_ID_TYPE_WIFI_TID:
		/*
		 * In case of HLOS TID OVERRIDE MODE, fetch the metadata from Tree ID.
		 */
		wifi_qos = EDMA_RXDESC_WIFI_QOS_GET(rxdesc_head);

		/*
		 * Configure skb->skb_priority with metadata.
		 */
		skb->priority = wifi_qos;
		edma_debug("%px : HLOS TID OVERRIDE priority configured = 0x%d\n", egc, skb->priority);
		break;

	case PPE_DRV_TREE_ID_TYPE_MLO_ASSIST:
		/*
		 * In case of MLO, fetch the MLO metadata from Tree ID.
		 */
		wifi_qos = EDMA_RXDESC_WIFI_QOS_GET(rxdesc_head);
		mlo_mark = EDMA_RXDESC_MLO_MARK_GET(rxdesc_sec);

		/*
		 * Configure skb->mark with MLO metadata.
		 */
		skb->mark = EDMA_RX_MLO_METADATA_CONSTRUCT(mlo_mark, wifi_qos);

		edma_debug("%px : mlo mark configured = 0x%x\n", egc, skb->mark);
		break;

	default:
		edma_debug("%p : Invalid tree-id type = %u\n", egc, tree_id_type);
		break;
	}
}

/*
 * edma_rx_handle_sc_cc_packets()
 *	Handle packets with service code or CPU code.
 *
 * NOTE: We give higher priority to CPU code, since they are related
 * to exception and required to flush an existing decelerated flow by
 * hardware.
 */
static inline bool edma_rx_handle_sc_cc_packets(struct edma_gbl_ctx *egc,
		struct edma_rxdesc_ring *rxdesc_ring,
		struct edma_rxdesc_desc *rxdesc_head,
		struct sk_buff *skb)
{
	uint16_t desc_index, next_desc_index;
	uint32_t dst_port;
	uint8_t cpu_code, service_code;
	bool acl_info_valid = false;
	struct edma_rxdesc_sec_desc *rxdesc_sec, *next_rxdesc_sec;
	struct ppe_drv_cc_metadata cc_info = {0};
	struct ppe_drv_sc_metadata sc_info = {0};
	struct ppe_drv_acl_metadata acl_info = {0};

	/*
	 * The primary descriptor has CPU code valid indication bit while
	 * the CPU code is available in secondary descriptor.
	 */
	if (likely(EDMA_RXDESC_CPU_CODE_VALID_GET(rxdesc_head))) {
		desc_index = ((uint8_t *)rxdesc_head - (uint8_t *)rxdesc_ring->pdesc) >> EDMA_RXDESC_SIZE_SHIFT;
		rxdesc_sec = EDMA_RXDESC_SEC_DESC(rxdesc_ring, desc_index);
		cpu_code = EDMA_RXDESC_CPU_CODE_GET(rxdesc_sec);

		/*
		 * Depending on the use-case, sometime PPE generate the same CPU
		 * code for every packet, prefetch the next secondary descriptor
		 * to handle such cases.
		 */
		next_desc_index = (desc_index + 1) & EDMA_RX_RING_SIZE_MASK;
		next_rxdesc_sec = EDMA_RXDESC_SEC_DESC(rxdesc_ring, next_desc_index);
		prefetch(next_rxdesc_sec);

		/*
		 * Get the ACL id from EDMA secondary descriptor as well.
		 */
		if (unlikely(EDMA_RXDESC_ACL_IDX_VALID_GET(rxdesc_sec))) {
			acl_info.acl_hw_index = EDMA_RXDESC_ACL_IDX_GET(rxdesc_sec);
			acl_info.cpu_code = cpu_code;
			acl_info_valid = true;
			cc_info.acl_hw_index = acl_info.acl_hw_index;
			cc_info.acl_index_valid = true;
		}

		cc_info.cpu_code = cpu_code;
		if (cpu_code && ppe_drv_cc_process_skbuff(&cc_info, skb)) {
			return true;
		}
	}

	/*
	 * Process if there is any service code.
	 */
	service_code = EDMA_RXDESC_SERVICE_CODE_GET(rxdesc_head);
	if (likely(service_code)) {

		/*
		 * Fill the service code metadata structure.
		 */
		sc_info.service_code = service_code;
		dst_port = EDMA_RXDESC_DST_INFO_GET(rxdesc_head);
		if (likely(((dst_port & ~EDMA_RXDESC_DST_PORT_ID_MASK) == EDMA_RXDESC_DST_PORT) &&
				(EDMA_RXDESC_PORT_ID_GET(dst_port) & EDMA_RXDESC_VP_PORT_MASK))) {
			sc_info.vp_num = EDMA_RXDESC_DST_PORT_ID_GET(rxdesc_head);
		}

		/*
		 * Serivce codes can return true / false based on the callbacks registered to them.
		 */
		if (unlikely(ppe_drv_sc_process_skbuff(&sc_info, skb))) {
			return true;
		}
	}

	/*
	 * check if the ACL ID is valid or not, if yes,
	 * process the packet based on ACL ID post CPU and service code
	 * processing is done.
	 */
	if (unlikely(acl_info_valid)) {
		if (ppe_drv_acl_process_skbuff(&acl_info, skb)) {
			return true;
		}
	}

	return false;
}

/*
 * edma_rx_handle_scatter_frames()
 *	Handle scattered packets in Rx direction
 *
 * This function should free the SKB in case of failure.
 */
static void edma_rx_handle_scatter_frames(struct edma_gbl_ctx *egc,
		struct edma_rxdesc_ring *rxdesc_ring,
		struct edma_rxdesc_desc *rxdesc_desc,
		struct sk_buff *skb)
{
	struct nss_dp_dev *dp_dev;
	struct edma_pcpu_stats *pcpu_stats;
	struct edma_rx_stats *rx_stats;
	struct sk_buff *skb_head;
	struct net_device *dev;
	uint32_t pkt_length;
	skb_frag_t *frag = NULL;
	bool page_mode = rxdesc_ring->rxfill->page_mode;

	/*
	 * Get packet length
	 */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_desc);
	edma_debug("edma_gbl_ctx:%px skb:%px fragment pkt_length:%u\n", egc, skb, pkt_length);

	/*
	 * For fraglist case
	 */
	if (likely(!page_mode)) {

		/*
		 * Invalidate the buffer received from the HW
		 */
		dmac_inv_range((void *)skb->data,
				(void *)(skb->data + pkt_length));

		if (!(rxdesc_ring->head)) {
			skb_put(skb, pkt_length);
			rxdesc_ring->head = skb;
			rxdesc_ring->last = NULL;
			rxdesc_ring->pdesc_head = rxdesc_desc;
			/*
			 * TODO: It is safer to save the descriptor value here instead of the pointer,
			 * since descriptor may be overwritten by HW after function returns.
			 */
			return;
		}

		/*
		 * If head is present and got next desc.
		 * Append it to the fraglist of head if this is second frame
		 * If not second frame append to tail
		 */
		skb_put(skb, pkt_length);
		if (!skb_has_frag_list(rxdesc_ring->head)) {
			skb_shinfo(rxdesc_ring->head)->frag_list = skb;
		} else {
			rxdesc_ring->last->next = skb;
		}

		rxdesc_ring->last = skb;
		rxdesc_ring->last->next = NULL;
		rxdesc_ring->head->len += pkt_length;
		rxdesc_ring->head->data_len += pkt_length;
		rxdesc_ring->head->truesize += skb->truesize;

		goto process_next_scatter;
	}

	/*
	 * Manage fragments for page mode
	 */
	frag = &skb_shinfo(skb)->frags[0];
	dmac_inv_range((void *)skb_frag_page(frag), (void *)(skb_frag_page(frag) + pkt_length));

	if (!(rxdesc_ring->head)) {
		skb->len = pkt_length;
		skb->data_len = pkt_length;
		skb->truesize = SKB_TRUESIZE(PAGE_SIZE);
		rxdesc_ring->head = skb;
		rxdesc_ring->last = NULL;
		rxdesc_ring->pdesc_head = rxdesc_desc;
		return;
	}

	/*
	 * Append current frag at correct index as nr_frag of parent
	 */
	skb_add_rx_frag(rxdesc_ring->head, skb_shinfo(rxdesc_ring->head)->nr_frags,
			skb_frag_page(frag), 0, pkt_length, PAGE_SIZE);
	skb_shinfo(skb)->nr_frags = 0;

	/*
	 * Free the SKB after we have appended its frag page to the head skb
	 */
	dev_kfree_skb_any(skb);

process_next_scatter:
	/*
	 * If there are more segments for this packet,
	 * then we have nothing to do. Otherwise process
	 * last segment and send packet to stack
	 */
	if (EDMA_RXDESC_MORE_BIT_GET(rxdesc_desc)) {
		return;
	}

	skb_head = rxdesc_ring->head;
	dev = skb_head->dev;

	/*
	 * Check Rx checksum offload status.
	 */
	if (likely(dev->features & NETIF_F_RXCSUM)) {
		skb->ip_summed = edma_rx_checksum_verify(rxdesc_desc, skb_head);
	}

	/*
	 * Get stats for the netdevice
	 */
	dp_dev = netdev_priv(dev);
	pcpu_stats = &dp_dev->dp_info.pcpu_stats;
	rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);

	if (unlikely(page_mode)) {
		if (unlikely(!pskb_may_pull(skb_head, ETH_HLEN))) {
			/*
			 * Discard the SKB that we have been building,
			 * in addition to the SKB linked to current descriptor.
			 */
			dev_kfree_skb_any(skb_head);
			rxdesc_ring->head = NULL;
			rxdesc_ring->last = NULL;
			rxdesc_ring->pdesc_head = NULL;

			u64_stats_update_begin(&rx_stats->syncp);
			rx_stats->rx_nr_frag_headroom_err++;
			u64_stats_update_end(&rx_stats->syncp);

			return;
		}
	}

	/*
	 * In some cases like PPE tunnel when mode 1 is enabled
	 * the skb data will be pointing to outer header and if
	 * packet decap is successful then data offset will point
	 * to inner payload.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	if (unlikely(!__pskb_pull(skb_head, EDMA_RXDESC_DATA_OFFSET_GET(rxdesc_ring->pdesc_head)))) {
#else
	if (unlikely(!pskb_pull(skb_head, EDMA_RXDESC_DATA_OFFSET_GET(rxdesc_ring->pdesc_head)))) {
#endif
		/*
		 * Discard the SKB that we have been building,
		 * in addition to the SKB linked to current descriptor.
		 */
		dev_kfree_skb_any(skb_head);
		rxdesc_ring->head = NULL;
		rxdesc_ring->last = NULL;
		rxdesc_ring->pdesc_head = NULL;

		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_nr_frag_headroom_err++;
		u64_stats_update_end(&rx_stats->syncp);

		return;
	}

	/*
	 * TODO: Do a batched update of the stats per netdevice.
	 */
	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_pkts++;
	rx_stats->rx_bytes += skb_head->len;
	rx_stats->rx_nr_frag_pkts += (uint64_t)page_mode;
	rx_stats->rx_fraglist_pkts += (uint64_t)(!page_mode);
	u64_stats_update_end(&rx_stats->syncp);

	edma_debug("edma_gbl_ctx:%px skb:%px Jumbo pkt_length:%u\n", egc, skb_head, skb_head->len);

	/*
	 * Check if primary descriptor is not NULL.
	 */
	if (likely(rxdesc_ring->pdesc_head)) {
		/*
		 * NOTE:
		 * 1. We are combining both the checks together here to reduce
		 *    a branch instruction in regular data path processing.
		 *
		 * 2. If the service code/cpu code processing consumes the packet,
		 *    don't send it to stack otherwise continue with regular processing.
		 */
		struct edma_rxdesc_desc *pdesc_head = rxdesc_ring->pdesc_head;
		if (unlikely(EDMA_RXDESC_SC_CC_VALID_GET(rxdesc_ring->pdesc_head) && edma_rx_handle_sc_cc_packets(egc, rxdesc_ring, pdesc_head, skb_head))) {
			rxdesc_ring->head = NULL;
			rxdesc_ring->last = NULL;
			rxdesc_ring->pdesc_head = NULL;
			return;
		}

		/*
		 * See if this packet is tagged with valid wifi qos.
		 * WiFi-QoS flag needs to be set for tree_id processing.
		 */
		if (unlikely(EDMA_RXDESC_WIFI_QOS_FLAG_VALID_GET(rxdesc_ring->pdesc_head))) {
			edma_rx_handle_wifi_qos_packets(egc, rxdesc_ring, pdesc_head, skb_head);
		}
	}

	/*
	 * Check if packet is meant for VP processing
	 */
	if (unlikely(EDMA_RXDESC_SRC_DST_INFO_GET(rxdesc_desc) & EDMA_RXDESC_SRC_DST_VP_MASK)) {
		edma_rx_process_vp(rxdesc_ring->pdesc_head, rxdesc_ring, skb_head);
		rxdesc_ring->head = NULL;
		rxdesc_ring->last = NULL;
		rxdesc_ring->pdesc_head = NULL;
		return;
	}

	skb_head->protocol = eth_type_trans(skb_head, dev);

	/*
	 * Send packet up the stack
	 */
	if (unlikely(dev->features & NETIF_F_GRO))
		napi_gro_receive(&rxdesc_ring->napi, skb_head);
	else
		netif_receive_skb(skb_head);

	rxdesc_ring->head = NULL;
	rxdesc_ring->last = NULL;
	rxdesc_ring->pdesc_head = NULL;
}

/*
 * edma_rx_handle_capwap_inear_packets()
 *	Handle linear packets
 */
void edma_rx_handle_capwap_linear_packets(struct edma_gbl_ctx *egc,
		struct edma_rxdesc_ring *rxdesc_ring,
		struct edma_rxdesc_desc *rxdesc_desc,
		struct sk_buff *skb, struct net_device *dev)
{
	struct nss_dp_dev *dp_dev;
	struct edma_pcpu_stats *pcpu_stats;
	struct edma_rx_stats *rx_stats;
	uint32_t pkt_length;
	skb_frag_t *frag = NULL;
	bool page_mode = rxdesc_ring->rxfill->page_mode;

	/*
	 * Get stats for the netdevice
	 */
	dp_dev = netdev_priv(dev);
	pcpu_stats = &dp_dev->dp_info.pcpu_stats;
	rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);

	/*
	 * Get packet length
	 */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_desc);

	if (unlikely(page_mode)) {

		/*
		 * Handle linear packet in page mode
		 */
		frag = &skb_shinfo(skb)->frags[0];
		dmac_inv_range((void *)skb_frag_page(frag),
				(void *)(skb_frag_page(frag) + pkt_length));
		skb_add_rx_frag(skb, 0, skb_frag_page(frag), 0, pkt_length, PAGE_SIZE);

		/*
		 * Pull ethernet header into SKB data area for header processing
		 */
		if (unlikely(!pskb_may_pull(skb, ETH_HLEN))) {
			u64_stats_update_begin(&rx_stats->syncp);
			rx_stats->rx_nr_frag_headroom_err++;
			u64_stats_update_end(&rx_stats->syncp);
			dev_kfree_skb_any(skb);
			return;
		}

		goto send_to_vp;
	}

	/*
	 * Invalidate the buffer received from the HW
	 */
	dmac_inv_range_no_dsb((void *)skb->data,
			(void *)(skb->data + pkt_length));
	skb_put(skb, pkt_length);

send_to_vp:

	/*
	 * In some cases like PPE tunnel when mode 1 is enabled
	 * the skb data will be pointing to outer header and if
	 * packet decap is successful then data offset will point
	 * to inner payload.
	 */
	__skb_pull(skb, EDMA_RXDESC_DATA_OFFSET_GET(rxdesc_desc));

	/*
	 * TODO: Do a batched update of the stats per netdevice.
	 */
	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_pkts++;
	rx_stats->rx_bytes += pkt_length;
	rx_stats->rx_nr_frag_pkts += (uint64_t)page_mode;
	u64_stats_update_end(&rx_stats->syncp);

	edma_debug("edma_gbl_ctx:%px, skb:%px pkt_length:%u\n",
			egc, skb, skb->len);

	edma_rx_process_capwap_vp(rxdesc_ring, rxdesc_desc, skb);
}

/*
 * edma_rx_handle_linear_packets()
 *	Handle linear packets
 *
 * Return false if packet is consumed by this function for error cases or VP/SC cases.
 * Return true otherwise, for caller to deliver packet to the stack.
 */
static inline bool edma_rx_handle_linear_packets(struct edma_gbl_ctx *egc,
		struct edma_rxdesc_ring *rxdesc_ring,
		struct edma_rxdesc_desc *rxdesc_desc,
		struct sk_buff *skb)
{
	struct nss_dp_dev *dp_dev;
	struct edma_pcpu_stats *pcpu_stats;
	struct edma_rx_stats *rx_stats;
	uint32_t pkt_length;
	skb_frag_t *frag = NULL;
	bool page_mode = rxdesc_ring->rxfill->page_mode;

	/*
	 * Get stats for the netdevice
	 */
	dp_dev = netdev_priv(skb->dev);
	pcpu_stats = &dp_dev->dp_info.pcpu_stats;
	rx_stats = this_cpu_ptr(pcpu_stats->rx_stats);

	/*
	 * Get packet length
	 */
	pkt_length = EDMA_RXDESC_PACKET_LEN_GET(rxdesc_desc);

	if (likely(!page_mode)) {

		/*
		 * Invalidate the buffer received from the HW
		 */
		dmac_inv_range((void *)skb->data,
				(void *)(skb->data + pkt_length));
		skb_put(skb, pkt_length);
		goto send_to_stack;
	}

	/*
	 * Handle linear packet in page mode
	 */
	frag = &skb_shinfo(skb)->frags[0];
	dmac_inv_range((void *)skb_frag_page(frag),
			(void *)(skb_frag_page(frag) + pkt_length));
	skb_add_rx_frag(skb, 0, skb_frag_page(frag), 0, pkt_length, PAGE_SIZE);

	/*
	 * Pull ethernet header into SKB data area for header processing
	 */
	if (unlikely(!pskb_may_pull(skb, ETH_HLEN))) {
		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_nr_frag_headroom_err++;
		u64_stats_update_end(&rx_stats->syncp);
		dev_kfree_skb_any(skb);
		return false;
	}

send_to_stack:

	/*
	 * In some cases like PPE tunnel when mode 1 is enabled
	 * the skb data will be pointing to outer header and if
	 * packet decap is successful then data offset will point
	 * to inner payload.
	 */
	__skb_pull(skb, EDMA_RXDESC_DATA_OFFSET_GET(rxdesc_desc));

	/*
	 * Check Rx checksum offload status.
	 */
	if (likely(skb->dev->features & NETIF_F_RXCSUM)) {
		skb->ip_summed = edma_rx_checksum_verify(rxdesc_desc, skb);
	}

	/*
	 * TODO: Do a batched update of the stats per netdevice.
	 */
	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_pkts++;
	rx_stats->rx_bytes += pkt_length;
	rx_stats->rx_nr_frag_pkts += (uint64_t)page_mode;
	u64_stats_update_end(&rx_stats->syncp);

	edma_debug("edma_gbl_ctx:%px, skb:%px pkt_length:%u\n",
			egc, skb, skb->len);
	/*
	 * See if this packet is tagged with a special service code
	 * or CPU code.
	 */
	if (unlikely(EDMA_RXDESC_SC_CC_VALID_GET(rxdesc_desc))) {
		/*
		 * NOTE:
		 * 1. We are combining both the checks together here to reduce
		 *    a branch instruction in regular data path processing.
		 *
		 * 2. If the service code/cpu code processing consumes the packet,
		 *    don't send it to stack otherwise continue with regular processing.
		 */
		if (edma_rx_handle_sc_cc_packets(egc, rxdesc_ring, rxdesc_desc, skb)) {
			return false;
		}
	}

	/*
	 * See if this packet is tagged with valid wifi qos.
	 * WiFi-QoS flag needs to be set for tree_id processing.
	 */
	if (unlikely(EDMA_RXDESC_WIFI_QOS_FLAG_VALID_GET(rxdesc_desc))) {
		edma_rx_handle_wifi_qos_packets(egc, rxdesc_ring, rxdesc_desc, skb);
	}

	/*
	 * Check if packet is meant for VP processing
	 */
	if (EDMA_RXDESC_SRC_DST_INFO_GET(rxdesc_desc) & EDMA_RXDESC_SRC_DST_VP_MASK) {
		edma_rx_process_vp(rxdesc_desc, rxdesc_ring, skb);
		return false;
	}

	return true;
}

/*
 * edma_rx_get_src_port_and_dev()
 *	Get source port and corresponding net device.
 */
static inline struct net_device *edma_rx_get_src_dev(
		struct edma_gbl_ctx *egc,
		struct edma_rx_desc_stats *rxdesc_stats,
		struct edma_rxdesc_desc *rxdesc_desc,
		struct sk_buff *skb)
{
	struct net_device *ndev = NULL;
	uint32_t src_info = EDMA_RXDESC_SRC_INFO_GET(rxdesc_desc);
	uint8_t src_port_num;

	/*
	 * Check src_info
	 */
	if (likely((src_info & EDMA_RXDESC_SRCINFO_TYPE_MASK)
				== EDMA_RXDESC_SRCINFO_TYPE_PORTID)) {
		src_port_num = src_info & EDMA_RXDESC_PORTNUM_BITS;
	} else {
		if (net_ratelimit()) {
			edma_warn("Src_info_type:0x%x. Drop skb:%px\n",
					(src_info & EDMA_RXDESC_SRCINFO_TYPE_MASK), skb);
		}

		u64_stats_update_begin(&rxdesc_stats->syncp);
		++rxdesc_stats->src_port_inval_type;
		u64_stats_update_end(&rxdesc_stats->syncp);
		return NULL;
	}

	/*
	 * Packet with PP source
	 */
	if (unlikely(src_port_num <= NSS_DP_HAL_MAX_PORTS)) {
		if (unlikely(src_port_num < NSS_DP_START_IFNUM)) {
			if (net_ratelimit()) {
				edma_warn("Port number error :%d. \
						Drop skb:%px\n",
						src_port_num, skb);
			}

			u64_stats_update_begin(&rxdesc_stats->syncp);
			++rxdesc_stats->src_port_inval;
			u64_stats_update_end(&rxdesc_stats->syncp);
			return NULL;
		}

		/*
		 * Get netdev for this port using the source port
		 * number as index into the netdev array. We need to
		 * subtract one since the indices start form '0' and
		 * port numbers start from '1'.
		 */
		ndev = egc->netdev_arr[src_port_num - 1];
		goto done;
	}

	if (unlikely(src_port_num < PPE_DRV_VIRTUAL_START)) {
		if (net_ratelimit()) {
			edma_warn("Port number error :%d. \
				Drop skb:%px\n",
				src_port_num, skb);
		}

		u64_stats_update_begin(&rxdesc_stats->syncp);
		++rxdesc_stats->src_port_inval;
		u64_stats_update_end(&rxdesc_stats->syncp);
		return NULL;
	}

	/*
	 * Last netdev corresponds to VP dummy netdev
	 */
	ndev = egc->netdev_arr[NSS_DP_MAX_PORTS - 1];

done:
	if (likely(ndev))
		return ndev;

	if (net_ratelimit()) {
		edma_warn("Netdev Null src_info_type:0x%x. Drop skb:%px\n",
				src_port_num, skb);
	}

	u64_stats_update_begin(&rxdesc_stats->syncp);
	++rxdesc_stats->src_port_inval_netdev;
	u64_stats_update_end(&rxdesc_stats->syncp);
	return NULL;
}

#ifdef CONFIG_SKB_TIMESTAMP
/*
 * edma_rx_get_tstamp()
 *	Compute the timestamp received in the secondary descriptor
 */
static inline uint64_t edma_rx_get_tstamp(struct edma_rxdesc_sec_desc *rxsec_desc)
{
	uint32_t nsecs, secs;

	nsecs = EDMA_RX_SDESC_TSTAMP_LO_GET(rxsec_desc);
	secs = EDMA_RX_SDESC_TSTAMP_HI_GET(rxsec_desc);
	return EDMA_TIMESTAMP_TO_USEC(secs, nsecs);
}

/*
 * edma_rx_read_gmac_timer()()
 *	API to read the GMAC timer register
 */
static inline uint64_t edma_rx_read_gmac_timer(struct edma_gbl_ctx *egc)
{
	uint32_t nsecs, secs;

	nsecs = readl(egc->tstamp_nsec);
	secs = readl(egc->tstamp_sec) & EDMA_TIMESTAMP_SEC_MASK;
	return EDMA_TIMESTAMP_TO_USEC(secs, nsecs);
}
#endif

/*
 * edma_rx_get_src_capwap_dev()
 *	Get source port and corresponding net device.
 */
struct net_device *edma_rx_get_src_capwap_dev(struct edma_gbl_ctx *egc,
		struct edma_rx_desc_stats *rxdesc_stats,
		struct edma_rxdesc_desc *rxdesc_desc,
		struct sk_buff *skb)
{
	struct net_device *ndev;
	uint32_t src_info = EDMA_RXDESC_SRC_INFO_GET(rxdesc_desc);
	uint8_t src_port_num;

	/*
	 * Check src_info
	 */
	if (unlikely((src_info & EDMA_RXDESC_SRCINFO_TYPE_MASK) != EDMA_RXDESC_SRCINFO_TYPE_PORTID)) {
		if (net_ratelimit()) {
			edma_warn("Src_info_type:0x%x. Drop skb:%px\n",
					(src_info &
					 EDMA_RXDESC_SRCINFO_TYPE_MASK),
					skb);
		}
		u64_stats_update_begin(&rxdesc_stats->syncp);
		++rxdesc_stats->src_port_inval_type;
		u64_stats_update_end(&rxdesc_stats->syncp);
		return NULL;
	}

	src_port_num = src_info & EDMA_RXDESC_PORTNUM_BITS;
	if (likely(((src_port_num >= PPE_DRV_VIRTUAL_START) && (src_port_num < PPE_DRV_PORTS_MAX)))) {
		ndev = egc->netdev_arr[NSS_DP_MAX_PORTS - 1];
		if (likely(ndev)) {
			return ndev;
		}

		if (net_ratelimit()) {
			edma_warn("Netdev Null src_info_type:0x%x. Drop skb:%px\n",
								src_port_num, skb);
		}
		u64_stats_update_begin(&rxdesc_stats->syncp);
		++rxdesc_stats->src_port_inval_netdev;
		u64_stats_update_end(&rxdesc_stats->syncp);
		return NULL;
	}

	if (unlikely(src_port_num <= NSS_DP_HAL_MAX_PORTS)) {
		if (unlikely(src_port_num < NSS_DP_START_IFNUM)) {
			if (net_ratelimit()) {
				edma_warn("Port number error :%d. Drop skb:%px\n",
								src_port_num, skb);
			}
			u64_stats_update_begin(&rxdesc_stats->syncp);
			++rxdesc_stats->src_port_inval;
			u64_stats_update_end(&rxdesc_stats->syncp);
			return NULL;
		}

		/*
		 * Get netdev for this port using the source port
		 * number as index into the netdev array. We need to
		 * subtract one since the indices start form '0' and
		 * port numbers start from '1'.
		 */
		ndev = egc->netdev_arr[src_port_num - 1];
	}

	if (likely(ndev)) {
		return ndev;
	}

	if (net_ratelimit()) {
		edma_warn("Netdev Null src_info_type:0x%x. Drop skb:%px\n", src_port_num, skb);
	}
	u64_stats_update_begin(&rxdesc_stats->syncp);
	++rxdesc_stats->src_port_inval_netdev;
	u64_stats_update_end(&rxdesc_stats->syncp);
	return NULL;
}

/*
 * edma_rx_reap_capwap()
 *	Reap Rx descriptors
 */
static uint32_t edma_rx_reap_capwap(struct edma_gbl_ctx *egc, int budget,
				struct edma_rxdesc_ring *rxdesc_ring)
{
	struct edma_rxdesc_desc *rxdesc_desc, *pf_desc = NULL;
	struct edma_rx_desc_stats *rxdesc_stats = &rxdesc_ring->rx_desc_stats;
	uint32_t work_to_do, work_done = 0;
	uint16_t prod_idx, cons_idx, end_idx;
	uint16_t cons_idx_1, cons_idx_2;
	struct list_head rx_list;
	INIT_LIST_HEAD(&rx_list);

	/*
	 * Get Rx ring producer and consumer indices
	 */
	cons_idx = rxdesc_ring->cons_idx;

	if (likely(rxdesc_ring->work_leftover > budget)) {
		work_to_do = budget;
	} else {
		prod_idx =
			edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id)) &
			EDMA_RXDESC_PROD_IDX_MASK;
		work_to_do = EDMA_DESC_AVAIL_COUNT(prod_idx,
				cons_idx, EDMA_RX_RING_SIZE);
		rxdesc_ring->work_leftover = work_to_do;
		if (likely(work_to_do > budget)) {
			work_to_do = budget;
		}
	}

	rxdesc_ring->work_leftover -= work_to_do;

	end_idx = (cons_idx + work_to_do) & EDMA_RX_RING_SIZE_MASK;

	rxdesc_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);

	/*
	 * Invalidate all the cached descriptors
	 * that'll be processed.
	 */
	if (end_idx > cons_idx) {
		dmac_inv_range_no_dsb((void *)rxdesc_desc,
			(void *)(rxdesc_desc + work_to_do));
	} else {
		dmac_inv_range_no_dsb((void *)rxdesc_ring->pdesc,
			(void *)(rxdesc_ring->pdesc + end_idx));
		dmac_inv_range_no_dsb((void *)rxdesc_desc,
			(void *)(rxdesc_ring->pdesc + EDMA_RX_RING_SIZE));
	}

	/*
	 * TODO: Handle refill failures using retry
	 */
	edma_rx_alloc_buffer_list(rxdesc_ring->rxfill, work_to_do);

	/*
	 * Prefetch upto 3 Rx descriptors.
	 */
	prefetch(rxdesc_desc);
	if (likely(work_to_do >= 3)) {
		cons_idx_1 = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;
		pf_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx_1);
		prefetch(pf_desc);

		cons_idx_2 = (cons_idx_1 + 1) & EDMA_RX_RING_SIZE_MASK;
		pf_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx_2);
		prefetch(pf_desc);
	}

	while (likely(work_to_do--)) {
		struct net_device *ndev;
		struct sk_buff *skb;

		/*
		 * Get opaque from RXDESC
		 */
		skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(rxdesc_desc);

		if (likely(!(rxdesc_ring->head))) {
			ndev = edma_rx_get_src_capwap_dev(egc, rxdesc_stats, rxdesc_desc, skb);
			if(unlikely(!ndev)) {
				dev_kfree_skb_any(skb);

				/*
				 * Update work done
				 */
				work_done++;

				/*
				 * Update consumer index
				 */
				cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

				/*
				 * Get the next Rx descriptor.
				 */
				rxdesc_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);
				continue;
			}

			/*
			 * Prefetch the third skb and the fourth descriptor
			 */
			if (likely(work_to_do >= 3)) {
				struct sk_buff *pf_skb;
				pf_skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(pf_desc);
				prefetch(pf_skb);
				prefetch((uint8_t *)pf_skb + 64);
				prefetch((uint8_t *)pf_skb + 128);
				cons_idx_2 = (cons_idx_2 + 1) & EDMA_RX_RING_SIZE_MASK;

				pf_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx_2);
				prefetch(pf_desc);
			}

			/*
			 * Update skb fields for head skb
			 */
			skb->dev = ndev;
			skb->skb_iif = ndev->ifindex;

			/*
			 * Handle linear packets
			 */
			if (likely(!EDMA_RXDESC_MORE_BIT_GET(rxdesc_desc))) {
				edma_rx_handle_capwap_linear_packets(egc, rxdesc_ring, rxdesc_desc, skb, ndev);
				goto next_rx_desc;
			}
		}

		/*
		 * Handle scatter frame processing for first/middle/last segments
		 */
		edma_rx_handle_scatter_frames(egc, rxdesc_ring, rxdesc_desc, skb);

next_rx_desc:
		/*
		 * Update work done
		 */
		work_done++;

		/*
		 * Update consumer index
		 */
		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/*
		 * Get the next Rx descriptor.
		 */
		rxdesc_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);
	}

	dsb(st);

	if (likely(rxdesc_ring->vp_head)) {
		BUG_ON(!nss_dp_vp_list_rx_reg_cb);
		nss_dp_vp_list_rx_reg_cb(rxdesc_ring->vp_head);
		rxdesc_ring->vp_head = NULL;
	}

	edma_reg_write(EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id), cons_idx);
	rxdesc_ring->cons_idx = cons_idx;

	return work_done;
}

/*
 * edma_rx_reap()
 *	Reap Rx descriptors
 */
static uint32_t edma_rx_reap(struct edma_gbl_ctx *egc, int budget,
				struct edma_rxdesc_ring *rxdesc_ring)
{
	struct edma_rxdesc_desc *rxdesc_desc, *pf_desc = NULL;
	struct edma_rxdesc_sec_desc *rxdesc_sec;
	struct edma_rx_desc_stats *rxdesc_stats = &rxdesc_ring->rx_desc_stats;
	uint32_t work_to_do, work_done = 0;
	uint16_t prod_idx, cons_idx, end_idx;
	uint16_t cons_idx_1, cons_idx_2;
	struct sk_buff *cur_skb = NULL, *next_skb = NULL;
	struct list_head rx_list;
	INIT_LIST_HEAD(&rx_list);

	/*
	 * Get Rx ring producer and consumer indices
	 */
	cons_idx = rxdesc_ring->cons_idx;

	if (unlikely(egc->enable_ring_util_stats)) {
		prod_idx = edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id)) & EDMA_RXDESC_PROD_IDX_MASK;
		work_to_do = EDMA_DESC_AVAIL_COUNT(prod_idx, cons_idx, EDMA_RX_RING_SIZE);

		edma_update_ring_stats(work_to_do, EDMA_RX_RING_SIZE,
				       &rxdesc_ring->rx_desc_stats.ring_stats);
	}

	if (likely(rxdesc_ring->work_leftover > budget)) {
		work_to_do = budget;
	} else {
		prod_idx =
			edma_reg_read(EDMA_REG_RXDESC_PROD_IDX(rxdesc_ring->ring_id)) &
			EDMA_RXDESC_PROD_IDX_MASK;
		work_to_do = EDMA_DESC_AVAIL_COUNT(prod_idx,
				cons_idx, EDMA_RX_RING_SIZE);
		rxdesc_ring->work_leftover = work_to_do;
		if (likely(work_to_do > budget)) {
			work_to_do = budget;
		}
	}

	rxdesc_ring->work_leftover -= work_to_do;

	end_idx = (cons_idx + work_to_do) & EDMA_RX_RING_SIZE_MASK;

	rxdesc_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);
	rxdesc_sec = EDMA_RXDESC_SEC_DESC(rxdesc_ring, cons_idx);

	/*
	 * Invalidate all the cached descriptors
	 * that'll be processed.
	 */
	if (end_idx > cons_idx) {
		dmac_inv_range_no_dsb((void *)rxdesc_desc,
			(void *)(rxdesc_desc + work_to_do));
		dmac_inv_range_no_dsb((void *)rxdesc_sec,
			(void *)(rxdesc_sec + work_to_do));
	} else {
		dmac_inv_range_no_dsb((void *)rxdesc_ring->pdesc,
			(void *)(rxdesc_ring->pdesc + end_idx));
		dmac_inv_range_no_dsb((void *)rxdesc_ring->sdesc,
			(void *)(rxdesc_ring->sdesc + end_idx));
		dmac_inv_range_no_dsb((void *)rxdesc_desc,
			(void *)(rxdesc_ring->pdesc + EDMA_RX_RING_SIZE));
		dmac_inv_range_no_dsb((void *)rxdesc_sec,
			(void *)(rxdesc_ring->sdesc + EDMA_RX_RING_SIZE));
	}

	/*
	 * TODO: Handle refill failures using retry
	 */
	edma_rx_alloc_buffer_list(rxdesc_ring->rxfill, work_to_do);

	/*
	 * Prefetch upto 3 Rx descriptors.
	 */
	prefetch(rxdesc_desc);
	if (likely(work_to_do >= 3)) {
		cons_idx_1 = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;
		pf_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx_1);
		prefetch(pf_desc);

		cons_idx_2 = (cons_idx_1 + 1) & EDMA_RX_RING_SIZE_MASK;
		pf_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx_2);
		prefetch(pf_desc);
	}

	while (likely(work_to_do--)) {
		struct net_device *ndev;
		struct sk_buff *skb;

		/*
		 * Get opaque from RXDESC
		 */
		skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(rxdesc_desc);

#ifdef CONFIG_SKB_TIMESTAMP
	if (EDMA_RX_SDESC_TSTAMP_VALID_GET(rxdesc_sec)) {
		uint64_t pkt_time, cur_time;

		pkt_time = edma_rx_get_tstamp(rxdesc_sec);
		cur_time = edma_rx_read_gmac_timer(egc);

		if (likely(cur_time > pkt_time)) {
			skb->delta_ts0 = cur_time - pkt_time;
			skb->delta_ts1 = EDMA_TIMESTAMP_NSEC_TO_USEC(ktime_get_ns());
			edma_debug("skb: %p, pkt_time: %llu, cur_time: %llu, delta_ts0: %llu, delta_ts1: %llu\n",
					skb, pkt_time, cur_time,
					skb->delta_ts0, skb->delta_ts1);
		}
	}
#endif

		/*
		 * Handle linear packets or initial segments first
		 */
		if (likely(!(rxdesc_ring->head))) {
			ndev = edma_rx_get_src_dev(egc, rxdesc_stats, rxdesc_desc, skb);
			if(unlikely(!ndev)) {
				dev_kfree_skb_any(skb);
				goto next_rx_desc;
			}

			/*
			 * Prefetch the third skb and the fourth descriptor
			 */
			if (likely(work_to_do >= 3)) {
				struct sk_buff *pf_skb;
				pf_skb = (struct sk_buff *)EDMA_RXDESC_OPAQUE_GET(pf_desc);
				prefetch(pf_skb);
				prefetch((uint8_t *)pf_skb + 64);
				prefetch((uint8_t *)pf_skb + 128);
				prefetch((uint8_t *)pf_skb + 192);
				cons_idx_2 = (cons_idx_2 + 1) & EDMA_RX_RING_SIZE_MASK;

				pf_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx_2);
				prefetch(pf_desc);
			}

			/*
			 * Update skb fields for head skb
			 */
			skb->dev = ndev;
			skb->skb_iif = ndev->ifindex;

			/*
			 * Handle linear packets
			 */
			if (likely(!EDMA_RXDESC_MORE_BIT_GET(rxdesc_desc))) {
				if (likely(edma_rx_handle_linear_packets(egc, rxdesc_ring, rxdesc_desc, skb))) {
					if (unlikely(ndev->features & NETIF_F_GRO)) {
						skb->protocol = eth_type_trans(skb, ndev);
						napi_gro_receive(&rxdesc_ring->napi, skb);
					} else {
						list_add_tail(&skb->list, &rx_list);

					}
				}
				goto next_rx_desc;
			}
		}

		/*
		 * Handle scatter frame processing for first/middle/last segments
		 */
		edma_rx_handle_scatter_frames(egc, rxdesc_ring, rxdesc_desc, skb);

next_rx_desc:
		/*
		 * Update work done
		 */
		work_done++;

		/*
		 * Update consumer index
		 */
		cons_idx = (cons_idx + 1) & EDMA_RX_RING_SIZE_MASK;

		/*
		 * Get the next Rx descriptor.
		 */
		rxdesc_desc = EDMA_RXDESC_PRI_DESC(rxdesc_ring, cons_idx);
#ifdef CONFIG_SKB_TIMESTAMP
		rxdesc_sec = EDMA_RXDESC_SEC_DESC(rxdesc_ring, cons_idx);
#endif
	}

	edma_reg_write(EDMA_REG_RXDESC_CONS_IDX(rxdesc_ring->ring_id), cons_idx);
	rxdesc_ring->cons_idx = cons_idx;

	/*
	 * Prefetch the packet data for the next skbuff, and the skbuff
	 * structure for next and next-next skbuffs for optimal performance.
	 */
	list_for_each_entry_safe(cur_skb, next_skb, &rx_list, list) {
		if (likely(!list_entry_is_head(next_skb, &rx_list, list))) {
			prefetch(next_skb);
			prefetch((uint8_t *)(next_skb) + 64);
			prefetch((uint8_t *)(next_skb) + 128);
			prefetch((uint8_t *)(next_skb) + 192);
			prefetch(next_skb->data);
			prefetch(skb_shinfo(next_skb));
		}

		skb_list_del_init(cur_skb);
		cur_skb->protocol = eth_type_trans(cur_skb, cur_skb->dev);
		netif_receive_skb(cur_skb);
	}

	return work_done;
}

/*
 * edma_rx_napi_capwap_poll()
 *	EDMA RX NAPI handler
 */
int edma_rx_napi_capwap_poll(struct napi_struct *napi, int budget)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)napi;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	int32_t work_done = 0;
	uint32_t status;

	do {
		work_done += edma_rx_reap_capwap(egc, budget - work_done, rxdesc_ring);
		if (likely(work_done >= budget)) {
			return work_done;
		}

		/*
		 * Check if there are more packets to process
		 */
		status = EDMA_RXDESC_RING_INT_STATUS_MASK &
			edma_reg_read(
				EDMA_REG_RXDESC_INT_STAT(rxdesc_ring->ring_id));
	} while (likely(status));

	/*
	 * No more packets to process. Finish NAPI processing.
	 */
	napi_complete(napi);

	/*
	 * Set RXDESC ring interrupt mask
	 */
	edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id),
						egc->rxdesc_intr_mask);

	return work_done;
}

/*
 * edma_rx_napi_poll()
 *	EDMA RX NAPI handler
 */
int edma_rx_napi_poll(struct napi_struct *napi, int budget)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)napi;
	struct edma_gbl_ctx *egc = &edma_gbl_ctx;
	int32_t work_done = 0;
	uint32_t status;

	do {
		work_done += edma_rx_reap(egc, budget - work_done, rxdesc_ring);
		if (likely(work_done >= budget)) {
			return work_done;
		}

		/*
		 * Check if there are more packets to process
		 */
		status = EDMA_RXDESC_RING_INT_STATUS_MASK &
			edma_reg_read(
				EDMA_REG_RXDESC_INT_STAT(rxdesc_ring->ring_id));
	} while (likely(status));

	/*
	 * No more packets to process. Finish NAPI processing.
	 */
	napi_complete(napi);

	/*
	 * Set RXDESC ring interrupt mask
	 */
	edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id),
						egc->rxdesc_intr_mask);

	return work_done;
}

/*
 * edma_rx_handle_irq()
 *	Process RX IRQ and schedule napi
 */
irqreturn_t edma_rx_handle_irq(int irq, void *ctx)
{
	struct edma_rxdesc_ring *rxdesc_ring = (struct edma_rxdesc_ring *)ctx;

	edma_debug("irq: irq=%d rxdesc_ring_id=%u\n", irq, rxdesc_ring->ring_id);

	if (likely(napi_schedule_prep(&rxdesc_ring->napi))) {

		/*
		 * Disable RxDesc interrupt
		 */
		edma_reg_write(EDMA_REG_RXDESC_INT_MASK(rxdesc_ring->ring_id),
							EDMA_MASK_INT_DISABLE);
		__napi_schedule(&rxdesc_ring->napi);
	}

	return IRQ_HANDLED;
}

/*
 * edma_rx_phy_tstamp_buf()
 *	Receive skb for PHY timestamping
 */
bool edma_rx_phy_tstamp_buf(__attribute__((unused))void *app_data, struct sk_buff *skb, __attribute__((unused))void *sc_data)
{
	struct net_device *ndev = skb->dev;

	/*
	 * The PTP_CLASS_ value 0 is passed to phy driver, which will be
	 * set to the correct PTP class value by calling ptp_classify_raw
	 * in drv->rxtstamp function.
	 */
	if (ndev && ndev->phydev && ndev->phydev->drv
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
			&& ndev->phydev->drv->rxtstamp
#else
			&& phy_has_rxtstamp(ndev->phydev)
#endif
			) {
		skb->protocol = eth_type_trans(skb, ndev);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		if (likely(ndev->phydev->drv->rxtstamp(ndev->phydev, skb, 0))) {
#else
		if (likely(phy_rxtstamp(ndev->phydev, skb, 0))) {
#endif
			return true;
		} else {
			__skb_push(skb, ETH_HLEN);
		}
	}

	return false;
}

#ifdef NSS_DP_PPEDS_SUPPORT
/*
 * edma_rxfill_handle_irq()
 *	Process RXFill IRQ and schedule napi
 */
irqreturn_t edma_rxfill_handle_irq(int irq, void *ctx)
{
	struct edma_rxfill_ring *rxfill_ring = (struct edma_rxfill_ring *)ctx;

	edma_debug("irq: irq=%d rxfill_ring_id=%u\n", irq, rxfill_ring->ring_id);

	if (likely(napi_schedule_prep(&rxfill_ring->napi))) {

		/*
		 * Disable Rxfill interrupt
		 */
		edma_reg_write(EDMA_REG_RXFILL_INT_MASK(rxfill_ring->ring_id),
							EDMA_MASK_INT_DISABLE);
		__napi_schedule(&rxfill_ring->napi);
	}

	return IRQ_HANDLED;
}
#endif
