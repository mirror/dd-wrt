/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * nss_ppe_vxlanmgr.c
 *	VxLAN netdev events
 */

#include <linux/sysctl.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/rcupdate.h>
#include <linux/rwlock_types.h>
#include <linux/hashtable.h>
#include <net/vxlan.h>
#include "nss_ppe_vxlanmgr_priv.h"
#include "nss_ppe_vxlanmgr_tun_stats.h"
#include "nss_ppe_tun_drv.h"
#include "ppe_drv_tun_cmn_ctx.h"
#include "nss_ppe_bridge_mgr.h"

/*
 * Module parameter to configure the destination port of VXLAN tunnel.
 * PPE supports single destination port for all the VXLAN tunnels.
 */
int dstport = IANA_VXLAN_UDP_PORT;

/*
 * VxLAN context
 */
struct nss_ppe_vxlanmgr_ctx vxlan_ctx;

/*
 * Extern variable for VXLAN fdb notifier.
 */
extern struct notifier_block nss_ppe_vxlanmgr_switchdev_fdb_notifier;

/*
 * nss_ppe_vxlanmgr_netdev_event()
 *	Netdevice notifier for NSS VxLAN manager module
 */
int nss_ppe_vxlanmgr_netdev_event(struct notifier_block *nb, unsigned long event, void *dev)
{
	struct net_device *netdev = netdev_notifier_info_to_dev(dev);

	/*
	 * Return if it's not a vxlan netdev
	 */
	if (!netif_is_vxlan(netdev)) {
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_CHANGEMTU:
		nss_ppe_vxlanmgr_trace("%px: NETDEV_CHANGEMTU: name %s", netdev, netdev->name);
		nss_ppe_vxlanmgr_all_remotes_set_mtu(netdev, netdev->mtu);
		break;

	case NETDEV_BR_LEAVE:
		nss_ppe_vxlanmgr_trace("%px: NETDEV_BR_LEAVE: name %s", netdev, netdev->name);
		nss_ppe_vxlanmgr_all_remotes_decap_disable(netdev);
		nss_ppe_vxlanmgr_all_remotes_leave_bridge(netdev);
		break;

	case NETDEV_BR_JOIN:
		nss_ppe_vxlanmgr_trace("%px: NETDEV_BR_JOIN: name %s", netdev, netdev->name);
		nss_ppe_vxlanmgr_all_remotes_join_bridge(netdev);
		nss_ppe_vxlanmgr_all_remotes_decap_enable(netdev);
		break;

	default:
		nss_ppe_vxlanmgr_trace("%px: Unhandled notifier event %lu name %s", netdev, event, netdev->name);
	}
	return NOTIFY_DONE;
}

/*
 * Linux Net device Notifier
 */
static struct notifier_block nss_ppe_vxlanmgr_netdev_notifier = {
	.notifier_call = nss_ppe_vxlanmgr_netdev_event,
};

/*
 * nss_ppe_vxlanmgr_exit_module()
 *	Tunnel vxlan module exit function
 */
void __exit nss_ppe_vxlanmgr_exit_module(void)
{
	int ret;

	if (!ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN, false)) {
		nss_ppe_vxlanmgr_warn("failed to disable the VXLAN tunnels.");
	}

	nss_ppe_vxlanmgr_tun_stats_dentry_deinit();

	nss_ppe_vxlanmgr_delete_all_remotes();

	ret = unregister_netdevice_notifier(&nss_ppe_vxlanmgr_netdev_notifier);
	if (ret) {
		nss_ppe_vxlanmgr_warn("failed to unregister netdevice notifier: error %d", ret);
	}

	unregister_switchdev_notifier(&nss_ppe_vxlanmgr_switchdev_fdb_notifier);
	nss_ppe_vxlanmgr_wq_exit();

	nss_ppe_vxlanmgr_info("disabled all vxlan tunnels. VXLAN module unloaded");
}

/*
 * nss_ppe_vxlanmgr_init_module()
 *	Tunnel vxlan module init function
 */
int __init nss_ppe_vxlanmgr_init_module(void)
{
	int ret;

	vxlan_ctx.nack_limit = NSS_PPE_VXLANMGR_VP_STATUS_DEFAULT_MAX_NACK;

	if ((dstport < NSS_PPE_VXLANMGR_DST_PORT_MIN) || (dstport > NSS_PPE_VXLANMGR_DST_PORT_MAX)) {
		nss_ppe_vxlanmgr_warn("Invalid VXLAN dport:%u", dstport);
		return -1;
	}

	if (!ppe_tun_configure_vxlan_dport(dstport)) {
		nss_ppe_vxlanmgr_warn("configuring the destination port of the VXLAN failed");
		return -1;
	}

	if (nss_ppe_vxlanmgr_wq_init() < 0) {
		nss_ppe_vxlanmgr_warn("Failed to initialize work queue");
		return -1;
	}

	if (!nss_ppe_vxlanmgr_tun_dentry_init()) {
		nss_ppe_vxlanmgr_warn("Failed to create debugfs entry");
		goto wq_exit;
	}

	ret = register_netdevice_notifier(&nss_ppe_vxlanmgr_netdev_notifier);
	if (ret) {
		nss_ppe_vxlanmgr_warn("Failed to register netdevice notifier: error %d", ret);
		goto stats_dentry_deinit;
	}

	register_switchdev_notifier(&nss_ppe_vxlanmgr_switchdev_fdb_notifier);

	nss_ppe_vxlanmgr_info("Module %s loaded", NSS_PPE_BUILD_ID);

	return 0;

stats_dentry_deinit:
	nss_ppe_vxlanmgr_tun_stats_dentry_deinit();

wq_exit:
	nss_ppe_vxlanmgr_wq_exit();

	return -1;
}

module_init(nss_ppe_vxlanmgr_init_module);
module_exit(nss_ppe_vxlanmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE VxLAN manager");

module_param(dstport, int, 0644);
MODULE_PARM_DESC(dstport, "VXLAN destination port number");
