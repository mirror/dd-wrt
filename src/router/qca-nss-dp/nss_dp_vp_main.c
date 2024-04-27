/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/version.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <net/switchdev.h>

#include "nss_dp_hal.h"
#include "nss_dp_dev.h"

extern struct net_device_ops nss_dp_netdev_ops;
nss_dp_vp_rx_cb_t nss_dp_vp_rx_reg_cb = NULL;

/*
 * nss_dp_vp_xmit()
 *	This is not a netdev ops. This is to send a packet to VP associated rings.
 */
netdev_tx_t nss_dp_vp_xmit(struct net_device *netdev, struct nss_dp_vp_tx_info *info, struct sk_buff *skb)
{
	struct nss_dp_dev *dp_priv;

	if (unlikely(!skb || !netdev)) {
		return NETDEV_TX_OK;
	}

	dp_priv = (struct nss_dp_dev *)netdev_priv(netdev);
	return dp_priv->data_plane_ops->vp_xmit(dp_priv->dpc, info, skb);
}
EXPORT_SYMBOL(nss_dp_vp_xmit);

/*
 * nss_dp_vp_rx_register_cb()
 *	Register VP callback
 */
bool nss_dp_vp_rx_register_cb(nss_dp_vp_rx_cb_t cb)
{
	rcu_assign_pointer(nss_dp_vp_rx_reg_cb, cb);
	synchronize_rcu();
	return true;
}
EXPORT_SYMBOL(nss_dp_vp_rx_register_cb);

/*
 * nss_dp_vp_rx_unregister_cb()
 *	Unregister VP callback
 */
void nss_dp_vp_rx_unregister_cb(void)
{
	rcu_assign_pointer(nss_dp_vp_rx_reg_cb, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL(nss_dp_vp_rx_unregister_cb);

/*
 * nss_dp_vp_init()
 *	Initializes the DP netdev used by VP.
 */
struct net_device *nss_dp_vp_init(void)
{
	struct net_device *netdev;
	struct nss_dp_dev *dp_priv;

	/*
	 * Allocate a dummy netdev for sending all packets from VP to EDMA
	 * We can do dev_queue_xmit onto this netdev.
	 */
	/* TODO: See if we need to do some SoC level common init */

	netdev = alloc_etherdev_mqs(sizeof(struct nss_dp_dev),
			NSS_DP_NETDEV_TX_QUEUE_NUM, NSS_DP_NETDEV_RX_QUEUE_NUM);
	if (!netdev) {
		pr_info("alloc_etherdev() failed\n");
		return NULL;
	}

	netdev_dbg(netdev, "Initializing new DP mac id for VP\n");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
	/* max_mtu is set to 1500 in ether_setup() */
	netdev->max_mtu = ETH_MAX_MTU;
#endif

	dp_priv = netdev_priv(netdev);
	memset((void *)dp_priv, 0, sizeof(struct nss_dp_dev));

	/*
	 * It is always the last ID after all physical ports.
	 * E.g. If there are 7 HW ports, we are adding 8 as the VP port.
	 */
	dp_priv->macid = NSS_DP_VP_MAC_ID;
	dp_priv->netdev = netdev;
	netdev->watchdog_timeo = 5 * HZ;
	netdev->netdev_ops = &nss_dp_netdev_ops;

	/*
	 * Use data plane ops as per the configured SoC
	 */
	dp_priv->data_plane_ops = nss_dp_hal_get_data_plane_ops();
	if (!dp_priv->data_plane_ops) {
		netdev_dbg(netdev, "Dataplane ops not found.\n");
		goto fail;
	}

	dp_priv->dpc = &dp_global_data_plane_ctx[nss_dp_get_idx_from_macid(dp_priv->macid)];
	dp_priv->dpc->dev = netdev;
	dp_priv->ctx = &dp_global_ctx;

	dp_priv->drv_flags |= NSS_DP_PRIV_FLAG(INIT_DONE);

	/*
	 * Since we are not operating over a MAC interface, we don't
	 * need to use the GMAC HAL ops.
	 * We will directly initialize the DP operations.
	 */
	if (dp_priv->data_plane_ops->init(dp_priv->dpc)) {
		netdev_dbg(netdev, "Data plane init failed\n");
		goto fail;
	}

	/*
	 * Open the Dataplane for Tx as soon as it is initialized.
	 * TODO: Think of a better time to do this.
	 */
	if (dp_priv->data_plane_ops->open(dp_priv->dpc, 0, 0, 0)) {
		netdev_dbg(netdev, "Data plane open failed\n");
		goto data_plane_open_fail;
	}

	/*
	 * We do not need to register this netdev. This is only
	 * needed for handling packets for virtual port
	 */
	netdev_dbg(netdev, "Init NSS DP GMAC%d\n", dp_priv->macid);
	dp_global_ctx.nss_dp[nss_dp_get_idx_from_macid(dp_priv->macid)] = dp_priv;

	return netdev;

data_plane_open_fail:
	dp_priv->data_plane_ops->deinit(dp_priv->dpc);
fail:
	free_netdev(netdev);
	return NULL;
}
EXPORT_SYMBOL(nss_dp_vp_init);

/*
 * nss_dp_vp_deinit()
 *	De-initializes the DP netdev used by VP.
 */
bool nss_dp_vp_deinit(struct net_device *netdev)
{
	free_netdev(netdev);
	return NULL;
}
EXPORT_SYMBOL(nss_dp_vp_deinit);
