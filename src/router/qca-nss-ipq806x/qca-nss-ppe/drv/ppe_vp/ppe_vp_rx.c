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

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <ppe_drv_port.h>
#include "ppe_vp_base.h"

extern struct ppe_vp_base vp_base;

/*
 * ppe_vp_rx_process_cb
 * 	Standard Rx handler for packets with Rx VP
 */
bool ppe_vp_rx_process_cb(struct ppe_vp_cb_info *info, void *cb_data)
{
	struct sk_buff *skb = info->skb;
	struct net_device *dev = skb->dev;

	skb->protocol = eth_type_trans(skb, dev);
	skb->fast_xmit = 0;

	/*
	 * Reset the below flags in case any of DS flow is exceptioned.
	 */
	skb->fast_recycled = 0;
	skb->recycled_for_ds = 0;

	netif_receive_skb(skb);
	return true;
}

/*
 * ppe_vp_rx_fwd_dvp_list
 *	Forward packets received from nss-dp.
 */
static inline void ppe_vp_rx_fwd_dvp_list(struct nss_dp_vp_skb_list *vp_list_head, struct ppe_vp **vpa)
{
	struct ppe_vp *dvp;

	/*
	 * Forward to destination VP
	 */
	if (likely(vp_list_head->dvp >= PPE_DRV_VIRTUAL_START)) {
		struct ppe_vp_rx_stats *rx_stats;

		rcu_read_lock();
		dvp = rcu_dereference(vpa[PPE_VP_BASE_PORT_TO_IDX(vp_list_head->dvp)]);
		if (unlikely(!dvp || !(dvp->flags & PPE_VP_FLAG_VP_ACTIVE))) {
			/*
			 * Drop this list as destination VP is not active anymore.
			 */
			atomic64_add(skb_queue_len(&vp_list_head->skb_list), &vp_base.base_stats.rx_dvp_inactive);
			rcu_read_unlock();
			skb_queue_purge(&vp_list_head->skb_list);
			if (net_ratelimit()) {
				ppe_vp_info("%px: Destination VP:%d is not active anymore, dropping \n", dvp, vp_list_head->dvp);
			}
			return;
		}

		rx_stats = this_cpu_ptr(dvp->vp_stats.rx_stats);
		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_pkts += skb_queue_len(&vp_list_head->skb_list);
		rx_stats->rx_bytes += vp_list_head->len;
		u64_stats_update_end(&rx_stats->syncp);

		/*
		 * DP depends on this to be set zero.
		 */
		vp_list_head->len = 0;

		/*
		 * Destination VP user would consume the skb.
		 * User's responsibility to update skb->dev.
		 */
		if (likely(dvp->dst_list_cb)) {
			dvp->dst_list_cb(dvp->netdev, &vp_list_head->skb_list, dvp->dst_cb_data);
			rcu_read_unlock();
		} else {
			atomic64_add(skb_queue_len(&vp_list_head->skb_list), &vp_base.base_stats.rx_dvp_no_listcb);
			rcu_read_unlock();
			skb_queue_purge(&vp_list_head->skb_list);
			if (net_ratelimit()) {
				ppe_vp_warn("%px: No list handler for Destination VP:%d  Tx dev:%s \
					dropping skbs\n", dvp, vp_list_head->dvp, dvp->netdev->name);
			}
		}
		return;
	}

	atomic64_add(skb_queue_len(&vp_list_head->skb_list), &vp_base.base_stats.rx_dvp_invalid);
	skb_queue_purge(&vp_list_head->skb_list);
	return;
}

/*
 * ppe_vp_rx_dp_list_cb
 *	Process packet received from nss-dp.
 */
void ppe_vp_rx_dp_list_cb(struct nss_dp_vp_skb_list *vp_list_head)
{

	struct ppe_vp **vpa = &vp_base.vp_table.vp_allocator[0];

	do {
		ppe_vp_rx_fwd_dvp_list(vp_list_head, vpa);
		vp_list_head = vp_list_head->next;

	} while (vp_list_head && vp_list_head->len);

}

/*
 * ppe_vp_rx_dp_cb
 *	Process packet received from nss-dp.
 */
void ppe_vp_rx_dp_cb(struct sk_buff *skb, struct nss_dp_vp_rx_info *rxi)
{

	struct ppe_vp **vpa = &vp_base.vp_table.vp_allocator[0];
	struct ppe_vp_cb_info client_cb_info = {0};
	struct ppe_vp *svp, *dvp;

	/*
	 * Forward to destination VP
	 */
	if (likely(rxi->dvp >= PPE_DRV_VIRTUAL_START)) {
		struct ppe_vp_rx_stats *rx_stats;
		struct net_device *dev;

		rcu_read_lock();
		dvp = rcu_dereference(vpa[PPE_VP_BASE_PORT_TO_IDX(rxi->dvp)]);
		if (unlikely(!dvp || !(dvp->flags & PPE_VP_FLAG_VP_ACTIVE))) {
			/*
			 * Drop this packet as destination VP is not active anymore.
			 */
			atomic64_inc(&vp_base.base_stats.rx_dvp_inactive);
			rcu_read_unlock();
			dev_kfree_skb_any(skb);
			ppe_vp_info("%px: Destination VP:%d is not active anymore, dropping skb:%p\n", dvp, rxi->dvp, skb);
			return;
		}

		dev = dvp->netdev;

		rx_stats = this_cpu_ptr(dvp->vp_stats.rx_stats);

		/*
		 * Make sure device is UP before handling the packets.
		 */
		if (unlikely(!(dev->flags & IFF_UP))) {
			rcu_read_unlock();

			u64_stats_update_begin(&rx_stats->syncp);
			rx_stats->rx_dev_not_up++;
			u64_stats_update_end(&rx_stats->syncp);

			dev_kfree_skb_any(skb);

			return;
		}

		/*
		 * Pull any fake MAC added by PPE for L3 interfaces.
		 */
		if (dvp->vp_type == PPE_VP_TYPE_SW_L3) {
			struct ethhdr *ethh;
			if (unlikely(!pskb_may_pull(skb, (sizeof(struct ethhdr))))) {
				rcu_read_unlock();

				u64_stats_update_begin(&rx_stats->syncp);
				rx_stats->rx_drops++;
				u64_stats_update_end(&rx_stats->syncp);

				dev_kfree_skb_any(skb);
				ppe_vp_info("%px: Tx VP:%d skb pull failed dropping skb:%p\n", dvp, rxi->dvp, skb);
				return;
			}

			ethh = (struct ethhdr *)skb->data;
			skb->protocol = ethh->h_proto;
			skb_pull(skb, (sizeof(struct ethhdr)));
		}

		if (rxi->svp >= PPE_DRV_VIRTUAL_START) {
			svp = rcu_dereference(vpa[PPE_VP_BASE_PORT_TO_IDX(rxi->svp)]);
			if (likely(svp && svp->flags & PPE_VP_FLAG_VP_ACTIVE)) {
				skb->skb_iif = svp->netdev_if_num;
				ppe_vp_info("%px: Destination VP:%d skb:%p received from an inactive Rx VP:%d\n", dvp, rxi->dvp, skb, rxi->svp);
			}
		}

		skb->dev = dev;

		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_pkts++;
		rx_stats->rx_bytes +=skb->len;
		u64_stats_update_end(&rx_stats->syncp);

		/*
		 * TODO: update l3 and l4 checksum if needed
		 */
		skb->ip_summed = CHECKSUM_COMPLETE;

		/*
		 * If it can be, try forwarding through fast_xmit.
		 */
		if (likely(dvp->flags & PPE_VP_FLAG_VP_FAST_XMIT)) {
			if (unlikely(!dev_fast_xmit_vp(skb, dev))) {
				atomic64_inc(&vp_base.base_stats.rx_fastxmit_fails);
				dev_queue_xmit(skb);
			}

			rcu_read_unlock();
			return;
		}

		/*
		 * Destination VP user would consume the skb.
		 */
		if (unlikely(dvp->dst_cb)) {
			client_cb_info.skb = skb;
			client_cb_info.ip_summed = rxi->ip_summed;
			client_cb_info.napi = rxi->napi;

			if (unlikely(!dvp->dst_cb(&client_cb_info, dvp->dst_cb_data))) {
				ppe_vp_info("%px: Destination VP:%d  Tx dev:%s skb:%p \
						dropped by user\n", dvp, rxi->dvp, dev->name, skb);
			}
		} else {
			/*
			 * No registered callback with vp, forward through kernel.
			 */
			struct ethhdr *ethh;
			ethh = (struct ethhdr *)skb->data;
			skb->protocol = ethh->h_proto;
			skb_set_network_header(skb, rxi->l3offset);
			dev_queue_xmit(skb);
		}

		rcu_read_unlock();
		return;
	}

	/*
	 * VP less than 64 is invalid
	 */
	if (unlikely(rxi->dvp > 0)) {
		atomic64_inc(&vp_base.base_stats.rx_dvp_invalid);
		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * It's exceptioned packet, give it to its source to be given
	 * to stack for Rx handling.
	 */
	if (rxi->svp >= PPE_DRV_VIRTUAL_START) {
		struct ppe_vp_rx_stats *rx_stats;
		struct net_device *dev;

		rcu_read_lock();
		svp = rcu_dereference(vpa[PPE_VP_BASE_PORT_TO_IDX(rxi->svp)]);
		if (unlikely(!svp || !(svp->flags & PPE_VP_FLAG_VP_ACTIVE))) {
			/*
			 * Drop this packet as source VP is not valid anymore.
			 */
			atomic64_inc(&vp_base.base_stats.rx_svp_inactive);
			rcu_read_unlock();
			dev_kfree_skb_any(skb);
			ppe_vp_info("%px: Rx VP:%d is not active anymore, dropping skb:%p\n", svp, rxi->svp, skb);
			return;
		}

		dev = svp->netdev;

		rx_stats = this_cpu_ptr(svp->vp_stats.rx_stats);

		if (svp->vp_type == PPE_VP_TYPE_SW_L3) {
			struct ethhdr *ethh;
			ppe_vp_trace("%px: Rx VP#%d, Rx dev:%p with name: %s: L3 VP \n", svp, rxi->svp, dev, dev->name);

			if (unlikely(!pskb_may_pull(skb, (sizeof(struct ethhdr))))) {
				rcu_read_unlock();

				u64_stats_update_begin(&rx_stats->syncp);
				rx_stats->rx_drops++;
				u64_stats_update_end(&rx_stats->syncp);

				dev_kfree_skb_any(skb);
				ppe_vp_info("%px: Rx VP:%d skb pull failed dropping skb:%p\n", svp, rxi->svp, skb);
				return;
			}

			ethh = (struct ethhdr *)skb->data;
			skb->protocol = ethh->h_proto;
			skb_pull(skb, (sizeof(struct ethhdr)));
		}

		u64_stats_update_begin(&rx_stats->syncp);
		rx_stats->rx_excp_pkts++;
		rx_stats->rx_excp_bytes += skb->len;
		u64_stats_update_end(&rx_stats->syncp);

		skb_reset_mac_header(skb);
		skb->dev = dev;
		skb->skb_iif = svp->netdev_if_num;

		client_cb_info.skb = skb;
		client_cb_info.ip_summed = rxi->ip_summed;
		client_cb_info.napi = rxi->napi;

		/*
		 * If not processed successfully VP receive handler would free the skb
		 */
		if (unlikely(!svp->src_cb(&client_cb_info, svp->src_cb_data))) {
			rcu_read_unlock();
			ppe_vp_info("%px: Rx VP:%d Rx dev:%s skb:%p dropped by user\n", svp, rxi->svp, dev->name, skb);
			return;
		}

		/*
		 * skb successfully given to Source VP user.
		 */
		rcu_read_unlock();
		return;
	}

	/*
	 * Packet has neither source or destination VP set
	 */
	atomic64_inc(&vp_base.base_stats.rx_svp_invalid);
	dev_kfree_skb_any(skb);
	return;
}
