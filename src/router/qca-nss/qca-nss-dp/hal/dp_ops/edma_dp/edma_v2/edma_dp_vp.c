/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "nss_dp_dev.h"
#include "edma_regs.h"
#include "edma_debug.h"
#include "edma.h"

/*
 * edma_dp_vp_xmit()
 *	Transmit a packet using EDMA from VP.
 */
netdev_tx_t edma_dp_vp_xmit(struct nss_dp_data_plane_ctx *dpc, struct nss_dp_vp_tx_info *dptxi,
					struct sk_buff *skb)
{
	struct net_device *vpdev = dpc->dev;
	struct nss_dp_dev *dp_dev = netdev_priv(vpdev);
	struct edma_txdesc_ring *txdesc_ring;
	struct edma_pcpu_stats *pcpu_stats;
	struct edma_tx_stats *stats;
	struct sk_buff *segs;
	enum edma_tx_gso result;
	int ret;

#ifdef NSS_DP_MHT_SW_PORT_MAP
#ifndef NSS_DP_EDMA_MHT_SW_WITH_VP_RING
	/*
	 * Drop the packets received on vp port
	 * when mht per port Tx ring mapping is enabled.
	 */
	if (dp_dev->nss_dp_mht_dev) {
		dev_kfree_skb_any(skb);
		u64_stats_update_begin(&stats->syncp);
		++stats->tx_drops;
		u64_stats_update_end(&stats->syncp);
		return NETDEV_TX_OK;
	}
#endif
#endif

	/*
	 * Select a TX ring based on current core
	 */
	txdesc_ring = (struct edma_txdesc_ring *)dp_dev->dp_info.txr_map[0][smp_processor_id()];

	pcpu_stats = &dp_dev->dp_info.pcpu_stats;
	stats = this_cpu_ptr(pcpu_stats->tx_stats);

	/*
	 * HW does not support TSO for packets with more than or equal to
	 * 32 segments. HW hangs up if it sees more than 32 segments.
	 * Perform SW GSO for such packets.
	 */
	result = edma_tx_gso_segment(skb, vpdev, &segs);
	if (likely(result == EDMA_TX_GSO_NOT_NEEDED)) {

		/*
		 * Transmit the packet
		 */
		ret = edma_tx_ring_xmit(vpdev, dptxi, skb, txdesc_ring, stats);
		if (unlikely(ret != EDMA_TX_OK)) {
			dev_kfree_skb_any(skb);
			u64_stats_update_begin(&stats->syncp);
			++stats->tx_drops;
			u64_stats_update_end(&stats->syncp);
		}


		return NETDEV_TX_OK;
	}

	if (unlikely(result == EDMA_TX_GSO_FAIL)) {
		edma_debug("%px: SW GSO failed for segment size: (%d)\n", skb, skb_shinfo(skb)->gso_segs);
		dev_kfree_skb_any(skb);
		u64_stats_update_begin(&stats->syncp);
		++stats->tx_gso_drop_pkts;
		u64_stats_update_end(&stats->syncp);
		return NETDEV_TX_OK;
	}

	dev_kfree_skb_any(skb);
	while (segs) {
		skb = segs;
		segs = segs->next;

		/*
		 * Transmit the packet
		 */
		ret = edma_tx_ring_xmit(vpdev, dptxi, skb, txdesc_ring, stats);
		if (unlikely(ret != EDMA_TX_OK)) {
			dev_kfree_skb_any(skb);
			u64_stats_update_begin(&stats->syncp);
			++stats->tx_drops;
			u64_stats_update_end(&stats->syncp);
		}
	}

	return NETDEV_TX_OK;
}
