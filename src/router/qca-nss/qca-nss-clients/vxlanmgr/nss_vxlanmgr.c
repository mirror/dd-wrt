/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
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
 * nss_vxlanmgr.c
 *	NSS to HLOS VxLAN manager
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <net/vxlan.h>
#include <nss_api_if.h>
#include "nss_vxlanmgr.h"
#include "nss_vxlanmgr_tun_stats.h"

/*
 * VxLAN context
 */
struct nss_vxlanmgr_ctx vxlan_ctx;

/*
 * nss_vxlanmgr_netdev_event()
 *	Netdevice notifier for NSS VxLAN manager module
 */
static int nss_vxlanmgr_netdev_event(struct notifier_block *nb, unsigned long event, void *dev)
{
	struct net_device *netdev = netdev_notifier_info_to_dev(dev);

	if (!netif_is_vxlan(netdev)) {
		/*
		 * Return if it's not a vxlan netdev
		 */
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_DOWN:
		nss_vxlanmgr_trace("%px: NETDEV_DOWN: event %lu name %s\n", netdev, event, netdev->name);
		return nss_vxlanmgr_tunnel_deconfig(netdev);
	case NETDEV_UP:
		nss_vxlanmgr_trace("%px: NETDEV_UP: event %lu name %s\n", netdev, event, netdev->name);
		return nss_vxlanmgr_tunnel_config(netdev);
	case NETDEV_UNREGISTER:
		nss_vxlanmgr_trace("%px: NETDEV_UNREGISTER: event %lu name %s\n", netdev, event, netdev->name);
		return nss_vxlanmgr_tunnel_destroy(netdev);
	case NETDEV_REGISTER:
		nss_vxlanmgr_trace("%px: NETDEV_REGISTER: event %lu name %s\n", netdev, event, netdev->name);
		return nss_vxlanmgr_tunnel_create(netdev);
	default:
		nss_vxlanmgr_trace("%px: Unhandled notifier event %lu name %s\n", netdev, event, netdev->name);
	}
	return NOTIFY_DONE;
}

/*
 * Linux Net device Notifier
 */
static struct notifier_block nss_vxlanmgr_netdev_notifier = {
	.notifier_call = nss_vxlanmgr_netdev_event,
};

/*
 * nss_vxlanmgr_exit_module()
 *	Tunnel vxlan module exit function
 */
void __exit nss_vxlanmgr_exit_module(void)
{
	int ret;
	struct nss_vxlanmgr_tun_ctx *tun_ctx, *temp;

	/*
	 * Check if there are any tunnels.
	 * Delete all the tunnels from NSS FW and free.
	 */
	list_for_each_entry_safe(tun_ctx, temp, &vxlan_ctx.list, head) {
		/*
		 * Send deconfigure and destroy message to FW.
		 */
		nss_vxlanmgr_trace("Removing tunnel %s\n", tun_ctx->dev->name);
		nss_vxlanmgr_tunnel_deconfig(tun_ctx->dev);
		nss_vxlanmgr_tunnel_destroy(tun_ctx->dev);
	}

	nss_vxlanmgr_tun_stats_dentry_deinit();
	ret = unregister_netdevice_notifier(&nss_vxlanmgr_netdev_notifier);
	if (ret) {
		nss_vxlanmgr_warn("failed to unregister netdevice notifier: error %d\n", ret);
		return;
	}

	nss_vxlanmgr_info("module unloaded\n");
}

/*
 * nss_vxlanmgr_init_module()
 *	Tunnel vxlan module init function
 */
int __init nss_vxlanmgr_init_module(void)
{
	int ret;
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		nss_vxlanmgr_warn("nss-common not found.\n");
		return -1;
	}

	INIT_LIST_HEAD(&vxlan_ctx.list);
	vxlan_ctx.nss_ctx = nss_vxlan_get_ctx();
	spin_lock_init(&vxlan_ctx.tun_lock);

	if (!nss_vxlanmgr_tun_stats_dentry_init()) {
		nss_vxlanmgr_warn("Failed to create debugfs entry\n");
		return -1;
	}

	ret = register_netdevice_notifier(&nss_vxlanmgr_netdev_notifier);
	if (ret) {
		nss_vxlanmgr_tun_stats_dentry_deinit();
		nss_vxlanmgr_warn("Failed to register netdevice notifier: error %d\n", ret);
		return -1;
	}

	nss_vxlanmgr_info("Module %s loaded\n", NSS_CLIENT_BUILD_ID);
	return 0;
}

module_init(nss_vxlanmgr_init_module);
module_exit(nss_vxlanmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS VxLAN manager");
