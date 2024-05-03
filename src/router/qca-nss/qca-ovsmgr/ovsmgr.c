/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * ovsmgr.c
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/notifier.h>

/*
 * OpenVSwitch header files
 */
#include <datapath.h>

#include "ovsmgr.h"
#include "ovsmgr_priv.h"
#include "ovsmgr_debugfs.h"

/*
 * Registration/Unregistration methods for OVS datapath event notifications.
 */
static RAW_NOTIFIER_HEAD(dp_notifier_chain);
struct ovsmgr_ctx ovsmgr_ctx;

/*
 * ovsmgr_notifiers_call()
 *	Call registered notifiers.
 */
int ovsmgr_notifiers_call(struct ovsmgr_notifiers_info *info, unsigned long val)
{
	return raw_notifier_call_chain(&dp_notifier_chain, val, info);
}

/*
 * ovsmgr_notifier_register()
 *	Register OVS data path notifiers.
 */
void ovsmgr_notifier_register(struct notifier_block *nb)
{
	raw_notifier_chain_register(&dp_notifier_chain, nb);
}
EXPORT_SYMBOL(ovsmgr_notifier_register);

/*
 * ovsmgr_notifier_unregister()
 *	Unregister OVS data path notifiers.
 */
void ovsmgr_notifier_unregister(struct notifier_block *nb)
{
	raw_notifier_chain_unregister(&dp_notifier_chain, nb);
}
EXPORT_SYMBOL(ovsmgr_notifier_unregister);

/*
 * ovsmgr_dp_hook_register()
 *	Register OVS data path hook for processing packets.
 */
int ovsmgr_dp_hook_register(struct ovsmgr_dp_hook_ops *ops)
{
	switch (ops->hook_num) {
	case OVSMGR_DP_HOOK_PRE_FLOW_PROC:
		/*
		 * Allow only IGMP/ICMPV6 (MLD) packets in pre hook.
		 */
		if ((ops->protocol != IPPROTO_IGMP) && (ops->protocol != IPPROTO_ICMPV6)) {
			ovsmgr_warn("%px: Only IGMP/ICMPv6 packets will be forwarded in pre-flow hook\n", ops);
			return -EINVAL;
		}
	case OVSMGR_DP_HOOK_POST_FLOW_PROC:
		break;
	default:
		ovsmgr_warn("%px: Invalid hook number\n", ops);
		return -EINVAL;
	}

	write_lock_bh(&ovsmgr_ctx.lock);
	list_add(&ops->list, &ovsmgr_ctx.dp_hooks);
	write_unlock_bh(&ovsmgr_ctx.lock);

	return 0;
}
EXPORT_SYMBOL(ovsmgr_dp_hook_register);

/*
 * ovsmgr_dp_hook_unregister()
 *	Unregister OVS data path hook for processing packets.
 */
void ovsmgr_dp_hook_unregister(struct ovsmgr_dp_hook_ops *ops)
{
	struct ovsmgr_dp_hook_ops *h;

	write_lock_bh(&ovsmgr_ctx.lock);
	list_for_each_entry(h, &ovsmgr_ctx.dp_hooks, list) {
		if ((h->protocol == ops->protocol) &&
			(h->hook_num == ops->hook_num) &&
			(h->hook == ops->hook)) {
			list_del(&h->list);
			break;
		}
	}
	write_unlock_bh(&ovsmgr_ctx.lock);
}
EXPORT_SYMBOL(ovsmgr_dp_hook_unregister);

/*
 * ovsmgr_bridge_interface_stats_update()
 *	Update OVS bridge interface statistics.
 */
void ovsmgr_bridge_interface_stats_update(struct net_device *dev,
					  uint32_t rx_packets, uint32_t rx_bytes,
					  uint32_t tx_packets, uint32_t tx_bytes)
{
	ovsmgr_dp_bridge_interface_stats_update(dev,
						rx_packets, rx_bytes,
						tx_packets, tx_bytes);
}
EXPORT_SYMBOL(ovsmgr_bridge_interface_stats_update);

/*
 * ovsmgr_flow_stats_update()
 *	Update datapath flow statistics
 */
void ovsmgr_flow_stats_update(struct ovsmgr_dp_flow *flow, struct ovsmgr_dp_flow_stats *stats)
{
	ovsmgr_dp_flow_stats_update(flow, stats);
}
EXPORT_SYMBOL(ovsmgr_flow_stats_update);

/*
 * ovsmgr_dev_get_master()
 *	Find datapath bridge interface for given bridge port
 */
struct net_device *ovsmgr_dev_get_master(struct net_device *dev)
{
	return ovsmgr_dp_dev_get_master(dev);
}
EXPORT_SYMBOL(ovsmgr_dev_get_master);

/*
 * ovsmgr_is_ovs_master()
 *	Return true if dev is OVS bridge interface
 */
bool ovsmgr_is_ovs_master(struct net_device *dev)
{
	return ovsmgr_dp_dev_is_master(dev);
}
EXPORT_SYMBOL(ovsmgr_is_ovs_master);

/*
 * ovsmgr_port_find()
 *	Find datapath egress port for the given skb, bridge dev and flow
 */
struct net_device *ovsmgr_port_find(struct sk_buff *skb, struct net_device *dev, struct ovsmgr_dp_flow *flow)
{
	return ovsmgr_dp_port_dev_find(skb, dev, flow);
}
EXPORT_SYMBOL(ovsmgr_port_find);

/*
 * ovsmgr_port_find_by_mac()
 *	Find datapath egress port using MAC address
 */
struct net_device *ovsmgr_port_find_by_mac(struct sk_buff *skb, struct net_device *dev, struct ovsmgr_dp_flow *flow)
{
	return ovsmgr_dp_port_dev_find_by_mac(skb, dev, flow);
}
EXPORT_SYMBOL(ovsmgr_port_find_by_mac);

/*
 * ovsmgr_flow_info_get()
 *	Find datapath flow details using flow and skb
 */
enum ovsmgr_flow_status ovsmgr_flow_info_get(struct ovsmgr_dp_flow *flow,
					      struct sk_buff *skb, struct ovsmgr_flow_info *ofi)
{
	return ovsmgr_dp_flow_info_get(flow, skb, ofi);
}
EXPORT_SYMBOL(ovsmgr_flow_info_get);

/*
 * ovsmgr_init()
 *	Initialize OVS Manager
 */
int __init ovsmgr_init(void)
{
	rwlock_init(&ovsmgr_ctx.lock);
	INIT_LIST_HEAD(&ovsmgr_ctx.dp_list);
	INIT_LIST_HEAD(&ovsmgr_ctx.dp_hooks);

	if (ovsmgr_dp_init()) {
		ovsmgr_warn("Failed to initialize datapath\n");
		return -1;
	}

	if (ovsmgr_debugfs_init()) {
		ovsmgr_warn("Failed to initialize debugfs\n");
		ovsmgr_dp_exit();
		return -1;
	}

	ovsmgr_info("OVS Init successful\n");
	return 0;
}

/*
 * ovsmgr_exit()
 *	Cleanup OVS Manager and exit
 */
void __exit ovsmgr_exit(void)
{
	ovsmgr_debugfs_cleanup();
	ovsmgr_dp_exit();
	ovsmgr_info("OVS Manager Removed\n");
}

module_init(ovsmgr_init);
module_exit(ovsmgr_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("OVS Manager");
