/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
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

#include <nss_dp_dev.h>
#include <linux/irq.h>
#include "syn_dma_reg.h"

/*
 * syn_dp_napi_poll_rx()
 *	Scheduled by napi to process RX.
 */
static int syn_dp_napi_poll_rx(struct napi_struct *napi, int budget)
{
	int work_done, pending_refill;
	struct syn_dp_info_rx *rx_info = (struct syn_dp_info_rx *)napi;
	void __iomem *mac_base = rx_info->mac_base;

	work_done = syn_dp_rx(rx_info, budget);
	if (likely(!rx_info->page_mode)) {
		pending_refill = syn_dp_rx_refill(rx_info);
	} else {
		pending_refill = syn_dp_rx_refill_page_mode(rx_info);
	}

	/*
	 * Schedule the rx napi again if refill is not completely done
	 */
	if (unlikely(pending_refill)) {
		work_done = budget;
	}

	if (unlikely(work_done < budget)) {
		napi_complete(napi);
		syn_enable_rx_dma_interrupt(mac_base);
	}

	return work_done;
}

/*
 * syn_dp_napi_poll_tx()
 *	Scheduled by napi to process TX.
 */
static int syn_dp_napi_poll_tx(struct napi_struct *napi, int budget)
{
	struct syn_dp_info_tx *tx_info = (struct syn_dp_info_tx *)napi;
	void __iomem *mac_base = tx_info->mac_base;
	int work_done;

	work_done = syn_dp_tx_complete(tx_info, budget);

	if (unlikely(work_done < budget)) {
		napi_complete(napi);
		syn_enable_tx_dma_interrupt(mac_base);
	}

	return work_done;
}

/*
 * syn_dp_handle_irq()
 *	Process IRQ and schedule napi
 */
static irqreturn_t syn_dp_handle_irq(int irq, void *ctx)
{
	uint32_t status;
	struct syn_dp_info *dp_info = (struct syn_dp_info *)ctx;
	void __iomem *mac_base = dp_info->mac_base;

	status = hal_read_relaxed_reg(mac_base, SYN_DMA_STATUS);

	/*
	 * We need to clear all interrupts other than RX and TX complete.
	 * RX and TX complete interrupts should be cleared while scheduling Rx/Tx napi.
	 */
	hal_write_relaxed_reg(mac_base,
			SYN_DMA_STATUS,
			(status & (~(SYN_DMA_INT_TX_COMPLETED |
			SYN_DMA_INT_RX_COMPLETED))));

	/*
	 * Schedule Rx napi if Rx complete interrupt is triggered.
	 */
	if (status & SYN_DMA_INT_RX_COMPLETED) {
		syn_clear_rx_dma_status(mac_base);
		syn_disable_rx_dma_interrupt(mac_base);
		napi_schedule(&dp_info->dp_info_rx.napi_rx);
	}

	/*
	 * Schedule Tx napi if Tx complete interrupt is triggered.
	 */
	if (status & SYN_DMA_INT_TX_COMPLETED) {
		syn_clear_tx_dma_status(mac_base);
		syn_disable_tx_dma_interrupt(mac_base);
		napi_schedule(&dp_info->dp_info_tx.napi_tx);
	}

	return IRQ_HANDLED;
}

/*
 * syn_dp_if_init()
 *	Initialize the GMAC data plane operations
 */
static int syn_dp_if_init(struct nss_dp_data_plane_ctx *dpc)
{
	struct net_device *netdev = dpc->dev;
	struct nss_dp_dev *gmac_dev = (struct nss_dp_dev *)netdev_priv(netdev);
	void __iomem *mac_base = gmac_dev->gmac_hal_ctx->mac_base;
	uint32_t macid = gmac_dev->macid;
	struct syn_dp_info *dev_info = &gmac_dev->dp_info.syn_info;
	struct syn_dp_info_rx *rx_info = &dev_info->dp_info_rx;
	struct syn_dp_info_tx *tx_info = &dev_info->dp_info_tx;
	int err;

	if (!netdev) {
		netdev_err(netdev, "nss_dp_gmac: Invalid netdev pointer %px\n", netdev);
		return NSS_DP_FAILURE;
	}

	/*
	 * Initialize DMA operations.
	 */
	syn_dma_init(mac_base);

	/*
	 * If RX checksum is enabled, then configure DMA drop of the packet
	 * if error in encapsulated ethernet payload.
	 */
	if (test_bit(__NSS_DP_RXCSUM, &gmac_dev->flags)) {
		syn_rx_tcpip_chksum_drop_enable(mac_base);
	}

	dev_info->mac_base = rx_info->mac_base = tx_info->mac_base = mac_base;
	rx_info->netdev = tx_info->netdev = netdev;
	rx_info->dev = tx_info->dev = &gmac_dev->pdev->dev;
	netdev_info(netdev, "nss_dp_gmac: Registering netdev %s(qcom-id:%d) with GMAC, mac_base: 0x%px\n", netdev->name, macid, mac_base);

	/*
	 * Forcing the kernel to use 32-bit DMA addressing
	 */
	dma_set_coherent_mask(&gmac_dev->pdev->dev, DMA_BIT_MASK(32));

	/*
	 * Initialize Rx buffer mode setting and skb allocation length
	 * based on (page vs fraglist/jumbo-mru).
	 */
	rx_info->alloc_buf_len = dp_global_ctx.rx_buf_size;
	rx_info->page_mode = gmac_dev->rx_page_mode;
	if (rx_info->page_mode) {
		rx_info->alloc_buf_len = (SYN_DP_PAGE_MODE_SKB_SIZE + SYN_DP_SKB_HEADROOM + NET_IP_ALIGN);
	}

	if (gmac_dev->rx_jumbo_mru) {
		rx_info->alloc_buf_len = (gmac_dev->rx_jumbo_mru + NET_IP_ALIGN);
	}

	/*
	 * Initialize the Rx ring
	 */
	if (syn_dp_cfg_rx_setup_rings(dev_info)) {
		netdev_err(netdev, "nss_dp_gmac: Error initializing GMAC Rx rings %px\n", netdev);
		return NSS_DP_FAILURE;
	}

	/*
	 * Initialize the Tx ring
	 */
	if (syn_dp_cfg_tx_setup_rings(dev_info)) {
		syn_dp_cfg_rx_cleanup_rings(dev_info);
		netdev_err(netdev, "nss_dp_gmac: Error initializing GMAC Tx rings %px\n", netdev);
		return NSS_DP_FAILURE;
	}

	if (!dev_info->napi_added) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		netif_napi_add(netdev, &rx_info->napi_rx, syn_dp_napi_poll_rx, SYN_DP_NAPI_BUDGET_RX);
		netif_napi_add(netdev, &tx_info->napi_tx, syn_dp_napi_poll_tx, SYN_DP_NAPI_BUDGET_TX);
#else
		netif_napi_add_weight(netdev, &rx_info->napi_rx, syn_dp_napi_poll_rx, SYN_DP_NAPI_BUDGET_RX);
		netif_napi_add_weight(netdev, &tx_info->napi_tx, syn_dp_napi_poll_tx, SYN_DP_NAPI_BUDGET_TX);
#endif

		/*
		 * Requesting irq. Set IRQ_DISABLE_UNLAZY flag, this flag
		 * can be used for devices which cannot disable the interrupt
		 * at the device level under certain circumstances
		 * and have to use disable_irq[_nosync] instead.
		 */
		netdev->irq = platform_get_irq(gmac_dev->pdev, 0);
		irq_set_status_flags(netdev->irq, IRQ_DISABLE_UNLAZY);
		printk(KERN_INFO "request irq %d\n", netdev->irq);
		err = request_irq(netdev->irq, syn_dp_handle_irq, IRQF_SHARED, "nss-dp-gmac", &gmac_dev->dp_info.syn_info);
		if (unlikely(err)) {
			netif_napi_del(&rx_info->napi_rx);
			netif_napi_del(&tx_info->napi_tx);
			syn_dp_cfg_rx_cleanup_rings(dev_info);
			syn_dp_cfg_tx_cleanup_rings(dev_info);
			netdev_err(netdev, "err_code:%d, Mac %d IRQ %d request failed\n", err,
					gmac_dev->macid, netdev->irq);
			return NSS_DP_FAILURE;
		}

		gmac_dev->drv_flags |= NSS_DP_PRIV_FLAG(IRQ_REQUESTED);
		dev_info->napi_added = 1;
	}

	netdev_dbg(netdev,"Synopsys GMAC dataplane initialized\n");

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_if_open()
 *	Open the GMAC data plane operations
 */
static int syn_dp_if_open(struct nss_dp_data_plane_ctx *dpc, uint32_t tx_desc_ring,
					uint32_t rx_desc_ring, uint32_t mode)
{
	struct net_device *netdev = dpc->dev;
	struct nss_dp_dev *gmac_dev = (struct nss_dp_dev *)netdev_priv(netdev);
	void __iomem *mac_base = gmac_dev->gmac_hal_ctx->mac_base;
	struct nss_dp_hal_info *dp_info = &gmac_dev->dp_info;

	napi_enable(&dp_info->syn_info.dp_info_rx.napi_rx);
	napi_enable(&dp_info->syn_info.dp_info_tx.napi_tx);

	syn_enable_dma_tx(mac_base);
	syn_enable_dma_rx(mac_base);

	syn_enable_dma_interrupt(mac_base);

	netdev_dbg(netdev, "Synopsys GMAC dataplane opened\n");

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_if_close()
 *	Close the GMAC data plane operations
 */
static int syn_dp_if_close(struct nss_dp_data_plane_ctx *dpc)
{
	struct net_device *netdev = dpc->dev;
	struct nss_dp_dev *gmac_dev = (struct nss_dp_dev *)netdev_priv(netdev);
	void __iomem *mac_base = gmac_dev->gmac_hal_ctx->mac_base;
	struct nss_dp_hal_info *dp_info = &gmac_dev->dp_info;

	syn_disable_dma_interrupt(mac_base);

	syn_disable_dma_rx(mac_base);
	syn_disable_dma_tx(mac_base);

	napi_disable(&dp_info->syn_info.dp_info_rx.napi_rx);
	napi_disable(&dp_info->syn_info.dp_info_tx.napi_tx);

	netdev_dbg(netdev, "Synopsys GMAC dataplane closed\n");

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_if_link_state()
 *	Change of link state for the dataplane
 */
static int syn_dp_if_link_state(struct nss_dp_data_plane_ctx *dpc, uint32_t link_state)
{
	struct net_device *netdev = dpc->dev;

	if (link_state) {
		netdev_dbg(netdev, "Data plane link up\n");
	} else {
		netdev_dbg(netdev, "Data plane link down\n");
	}

	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_if_mac_addr()
 */
static int syn_dp_if_mac_addr(struct nss_dp_data_plane_ctx *dpc, uint8_t *addr)
{
	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_if_change_mtu()
 */
static int syn_dp_if_change_mtu(struct nss_dp_data_plane_ctx *dpc, uint32_t mtu)
{
	/*
	 * TODO: Work on MTU fix along with register update for frame length
	 */
	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_if_set_features()
 *	Set the supported net_device features
 */
static void syn_dp_if_set_features(struct nss_dp_data_plane_ctx *dpc)
{
	struct net_device *netdev = dpc->dev;

	netdev->features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM | NETIF_F_FRAGLIST | NETIF_F_SG;
	netdev->hw_features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM | NETIF_F_FRAGLIST | NETIF_F_SG;
	netdev->vlan_features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM | NETIF_F_FRAGLIST | NETIF_F_SG;
	netdev->wanted_features |= NETIF_F_HW_CSUM | NETIF_F_RXCSUM | NETIF_F_FRAGLIST | NETIF_F_SG;
}

/*
 * syn_dp_if_xmit()
 *	Dataplane method to transmit the packet
 */
static netdev_tx_t syn_dp_if_xmit(struct nss_dp_data_plane_ctx *dpc, struct sk_buff *skb)
{
	struct net_device *netdev = dpc->dev;
	struct nss_dp_dev *gmac_dev = (struct nss_dp_dev *)netdev_priv(netdev);
	struct syn_dp_info_tx *tx_info = &gmac_dev->dp_info.syn_info.dp_info_tx;
	uint16_t ret;

	ret = syn_dp_tx(tx_info, skb);
	if (likely(!ret)) {
		return NETDEV_TX_OK;
	}

	/*
	 * Handle the scenario when descriptors are not enough.
	 * Only one DMA channel is supported to assume queue 0.
	 */
	if (likely(ret == NETDEV_TX_BUSY)) {
		/*
		 * Stop the queue if the queue stop is not disabled and return
		 * NETDEV_TX_BUSY. Packet will be requeued or dropped by the caller.
		 * Queue will be re-enabled from Tx Complete.
		 */
		if (likely(!dp_global_ctx.tx_requeue_stop)) {
			netdev_dbg(netdev, "Stopping tx queue due to lack of tx descriptors");
			atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_packets_requeued);
			netif_stop_queue(netdev);
			return NETDEV_TX_BUSY;
		}
	}

	netdev_dbg(netdev, "Drop packet due to no Tx descriptor or invalid pkt");
	atomic64_inc((atomic64_t *)&tx_info->tx_stats.tx_dropped);
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/*
 * syn_dp_if_pause_on_off()
 */
static int syn_dp_if_pause_on_off(struct nss_dp_data_plane_ctx *dpc, uint32_t pause_on)
{
	return NSS_DP_SUCCESS;
}

/*
 * syn_dp_if_get_stats
 *	Get Synopsys GMAC data plane stats
 */
static void syn_dp_if_get_stats(struct nss_dp_data_plane_ctx *dpc,
				struct nss_dp_gmac_stats *stats)
{
	struct net_device *netdev = dpc->dev;
	struct nss_dp_dev *gmac_dev = (struct nss_dp_dev *)netdev_priv(netdev);
	struct nss_dp_hal_info *hal_info = &gmac_dev->dp_info;

	netdev_dbg(netdev, "GETTING stats: rx_packets:%llu rx_bytes:%llu",
			hal_info->syn_info.dp_info_rx.rx_stats.rx_packets,
			hal_info->syn_info.dp_info_rx.rx_stats.rx_bytes);

	/*
	 * TODO: Convert to per-cpu stats and use seqcount based update/read
	 */
	memcpy(&stats->stats.rx_stats, &hal_info->syn_info.dp_info_rx.rx_stats, sizeof(struct nss_dp_hal_gmac_stats_rx));
	memcpy(&stats->stats.tx_stats, &hal_info->syn_info.dp_info_tx.tx_stats, sizeof(struct nss_dp_hal_gmac_stats_tx));
}

/*
 * syn_dp_if_deinit()
 *	Free all the Synopsys GMAC resources
 */
static int syn_dp_if_deinit(struct nss_dp_data_plane_ctx *dpc)
{
	struct net_device *netdev = dpc->dev;
	struct nss_dp_dev *gmac_dev = (struct nss_dp_dev *)netdev_priv(netdev);
	struct nss_dp_hal_info *dp_info = &gmac_dev->dp_info;
	struct syn_dp_info *dev_info = &dp_info->syn_info;

	if (dev_info->napi_added) {

		/*
		 * Remove interrupt handlers and NAPI
		 */
		if (gmac_dev->drv_flags & NSS_DP_PRIV_FLAG(IRQ_REQUESTED)) {
			netdev_dbg(netdev, "Freeing IRQ %d for Mac %d\n", netdev->irq, gmac_dev->macid);
			synchronize_irq(netdev->irq);
			free_irq(netdev->irq, dev_info);
			gmac_dev->drv_flags &= ~NSS_DP_PRIV_FLAG(IRQ_REQUESTED);
		}

		netif_napi_del(&dev_info->dp_info_rx.napi_rx);
		netif_napi_del(&dev_info->dp_info_tx.napi_tx);
		dev_info->napi_added = 0;
	}

	/*
	 * Cleanup and free the rings
	 */
	syn_dp_cfg_rx_cleanup_rings(dev_info);
	syn_dp_cfg_tx_cleanup_rings(dev_info);

	/*
	 * Restore the DMA settings to avoid padding the descriptors
	 * to 64B. This is needed for NSS offload mode.
	 */
	syn_dma_bus_mode_init_clsize_32byte(dev_info->mac_base);

	return NSS_DP_SUCCESS;
}

/*
 * nss_dp_gmac_ops
 *	Data plane operations for Synopsys GMAC
 */
struct nss_dp_data_plane_ops nss_dp_gmac_ops = {
	.init		= syn_dp_if_init,
	.open		= syn_dp_if_open,
	.close		= syn_dp_if_close,
	.link_state	= syn_dp_if_link_state,
	.mac_addr	= syn_dp_if_mac_addr,
	.change_mtu	= syn_dp_if_change_mtu,
	.xmit		= syn_dp_if_xmit,
	.set_features	= syn_dp_if_set_features,
	.pause_on_off	= syn_dp_if_pause_on_off,
	.get_stats	= syn_dp_if_get_stats,
	.deinit		= syn_dp_if_deinit,
};
