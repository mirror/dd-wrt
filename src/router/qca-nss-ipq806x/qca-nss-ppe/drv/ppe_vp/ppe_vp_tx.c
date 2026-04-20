/*
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/types.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <nss_dp_vp.h>
#include <ppe_drv_port.h>
#include <ppe_drv_sc.h>
#include "ppe_vp_base.h"

extern struct ppe_vp_base vp_base;

/*
 * ppe_vp_tx_to_ppe()
 *	Process packet received from ppe vp user.
 *	When it returns false, user can Rx the skb to stack.
 *	Expect edma/dp to return false.
 */
bool ppe_vp_tx_to_ppe(int32_t vp_num, struct sk_buff *skb)
{

	struct ppe_vp **vpa = &vp_base.vp_table.vp_allocator[0];
	struct ppe_vp *svp;
	struct nss_dp_vp_tx_info dptxi = {};
	struct ppe_vp_tx_stats *tx_stats;
	unsigned int len = 0;

	/*
	 * Check if VP exists and forward to PPE
	 */
	rcu_read_lock();
	svp = rcu_dereference(vpa[vp_num - PPE_DRV_VIRTUAL_START]);
	if (unlikely(!svp || !(svp->flags & PPE_VP_FLAG_VP_ACTIVE))) {
		/*
		 * VP is not active, return skb to the user.
		 */
		atomic64_inc(&vp_base.base_stats.tx_vp_inactive);
		rcu_read_unlock();
		ppe_vp_info("%px: VP inactive for VP num %d, returning skb %px", svp, vp_num, skb);
		return false;
	}

	tx_stats = this_cpu_ptr(svp->vp_stats.tx_stats);
	dptxi.fake_mac = false;

	if (svp->vp_type == PPE_VP_TYPE_SW_L3) {
		dptxi.fake_mac = true;
		skb_push(skb, ETH_HLEN);
	}

	rcu_read_unlock();

	dptxi.svp = vp_num;
	dptxi.sc = PPE_DRV_SC_SPF_BYPASS;

	/*
	 * This would enable making list of skb's while freeing in edma tx complete
	 */
	if (likely(!skb_is_nonlinear(skb))) {
		skb->fast_xmit = 1;
	}

	len = skb->len;

	/*
	 * If enqueue to PPE fails, better drop else
	 * this could cause out of order packets. Hence returning
	 * status as true to the user.
	 */
	if (NETDEV_TX_OK != nss_dp_vp_xmit(vp_base.edma_vp_dev, &dptxi, skb)) {
		ppe_vp_info("Dropping skb %pxd, edma failed to enqueue to PPE, VP %d", skb, vp_num);
		u64_stats_update_begin(&tx_stats->syncp);
		tx_stats->tx_drops++;
		u64_stats_update_end(&tx_stats->syncp);
		skb->fast_xmit = 0;
		dev_kfree_skb_any(skb);
		return true;
	}

	u64_stats_update_begin(&tx_stats->syncp);
	tx_stats->tx_pkts++;
	tx_stats->tx_bytes += len;
	u64_stats_update_end(&tx_stats->syncp);

	return true;
}
EXPORT_SYMBOL(ppe_vp_tx_to_ppe);

/*
 * ppe_vp_tx_to_ppe_by_dev()
 *	Wrapper API to transmit the packet via PPE-VP when VP client
 *	has the context of netdevice but not the actual VP number.
 *	This API gets the VP from dev and calls the actual VP TX API.
 *	Ex: ATH driver VP TX
 */
bool ppe_vp_tx_to_ppe_by_dev(struct net_device *dev, struct sk_buff *skb)
{
	int32_t vp_num;

	if (unlikely(!dev)) {
		ppe_vp_warn(" Netdev is NULL in VP TX !");
		return false;
	}

	vp_num = ppe_drv_port_num_from_dev(dev);
	if (unlikely((vp_num < PPE_DRV_VIRTUAL_START) || (vp_num >= PPE_DRV_VIRTUAL_END))) {
		ppe_vp_warn("Not a valid Virtual Port number %d dev %s", vp_num, dev->name);
		return false;
	}

	return ppe_vp_tx_to_ppe(vp_num, skb);
}
EXPORT_SYMBOL(ppe_vp_tx_to_ppe_by_dev);
