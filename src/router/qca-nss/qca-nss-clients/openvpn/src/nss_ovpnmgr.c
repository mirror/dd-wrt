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
 * nss_ovpnmgr.c
 */
#include <linux/module.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/crypto.h>
#include <linux/if_tun.h>

#include <nss_api_if.h>
#include <nss_qvpn.h>
#include "nss_ovpnmgr.h"
#include "nss_ovpnmgr_crypto.h"
#include "nss_ovpnmgr_app.h"
#include "nss_ovpnmgr_tun.h"
#include "nss_ovpnmgr_debugfs.h"
#include "nss_ovpnmgr_priv.h"
#include "nss_ovpnmgr_route.h"
#include "nss_ovpn_sk_priv.h"

/*
 * OVPN manager context structure
 */
struct nss_ovpnmgr_context ovpnmgr_ctx;

/*
 * nss_ovpnmgr_netdevice_event()
 *	OpenVPN manager netdevice notifications.
 */
static int nss_ovpnmgr_netdevice_event(struct notifier_block *unused,
					unsigned long event, void *ptr)
{
	struct netdev_notifier_info *info = ptr;
	struct net_device *app_dev = netdev_notifier_info_to_dev(info);
	struct nss_ovpnmgr_tun *tun, *n;
	struct nss_ovpnmgr_app *app;
	struct net_device *nss_dev;

	/*
	 * Do not process any event other than UP and DOWN
	 */
	if ((event != NETDEV_UP) && (event != NETDEV_DOWN)) {
		return NOTIFY_DONE;
	}

	/*
	 * We should process notification only for TUN/TAP device
	 */
	if (!(app_dev->priv_flags_ext & IFF_EXT_TUN_TAP)) {
		return NOTIFY_DONE;
	}

	/*
	 * Find the application for the net_device triggering notification
	 */
	read_lock_bh(&ovpnmgr_ctx.lock);

	app = nss_ovpnmgr_app_find(app_dev);
	if (unlikely(!app)) {
		read_unlock_bh(&ovpnmgr_ctx.lock);
		nss_ovpnmgr_warn("%px: Application is not registered: app_dev = %s\n", app_dev, app_dev->name);
		return NOTIFY_DONE;
	}

	/*
	 * Listen to application netdevice UP and DOWN event. If UP event is triggered
	 * then bring all NSS device attached to application device UP. If down event is triggered
	 * then bring down all NSS device attached to application device DOWN. This will make ECM
	 * flush(DOWN)/add(UP) flow rules in the system.
	 *
	 * Iterate through all NSS device registered and change their state to UP
	 */
	list_for_each_entry_safe(tun, n, &app->tun_list, list) {
		nss_dev = __dev_get_by_index(&init_net, tun->tunnel_id);
		if (unlikely(!nss_dev)) {
			nss_ovpnmgr_warn("%px: Couldn't find tunnel: tunnel_id = %u\n\n", tun, tun->tunnel_id);
			continue;
		}

		/*
		 * dev_open and dev_close can sleep; hence calling it in non atomic context.
		 */
		read_unlock_bh(&ovpnmgr_ctx.lock);
		if (event == NETDEV_UP) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0))
			dev_open(nss_dev);
#else
			dev_open(nss_dev, NULL);
#endif

		} else {
			dev_close(nss_dev);
		}

		read_lock_bh(&ovpnmgr_ctx.lock);
	}

	read_unlock_bh(&ovpnmgr_ctx.lock);

	/*
	 * Notify done for all the events we don't care
	 */
	return NOTIFY_DONE;
}

/*
 * nss_ovpnmgr_netdevice_nb
 *	OpenVPN Netdevice notification callback
 */
static struct notifier_block nss_ovpnmgr_netdevice_nb __read_mostly = {
	.notifier_call = nss_ovpnmgr_netdevice_event,
};

/*
 * nss_ovpnmgr_init()
 *	Initialize NSS OVPN Manager
 */
int __init nss_ovpnmgr_init(void)
{
	if (!nss_cmn_get_nss_enabled()) {
		nss_ovpnmgr_warn("OVPN Manager is not compatible with this platform\n");
		return -1;
	}

	rwlock_init(&ovpnmgr_ctx.lock);
	INIT_LIST_HEAD(&ovpnmgr_ctx.app_list);

	if (nss_ovpn_sk_init()) {
		nss_ovpnmgr_warn("Failed to initialize socket interface\n");
		return -1;
	}

	if (nss_ovpnmgr_debugfs_init()) {
		nss_ovpnmgr_warn("Debugfs Initialization failed\n");
		nss_ovpn_sk_cleanup();
		return -1;
	}

	tun_register_offload_stats_callback(nss_ovpnmgr_tun_get_stats);
	register_netdevice_notifier(&nss_ovpnmgr_netdevice_nb);
	nss_ovpnmgr_info("OVPN Init successful\n");
	return 0;
}

/*
 * nss_ovpnmgr_exit()
 *	Cleanup NSS OVPN Manager and exit
 */
void __exit nss_ovpnmgr_exit(void)
{
	unregister_netdevice_notifier(&nss_ovpnmgr_netdevice_nb);
	nss_ovpnmgr_debugfs_cleanup();
	nss_ovpn_sk_cleanup();
	tun_unregister_offload_stats_callback();
	nss_ovpnmgr_info("OVPN Manager Removed\n");
}

module_init(nss_ovpnmgr_init);
module_exit(nss_ovpnmgr_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS OVPN Manager");
