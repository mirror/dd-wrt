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
 * nss_ovpnmgr_app.c
 */
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <net/udp.h>
#include <net/ip6_checksum.h>
#include <linux/ipv6.h>
#include <linux/crypto.h>

#include <nss_api_if.h>
#include <nss_qvpn.h>
#include "nss_ovpnmgr.h"
#include "nss_ovpnmgr_crypto.h"
#include "nss_ovpnmgr_tun.h"
#include "nss_ovpnmgr_app.h"
#include "nss_ovpnmgr_debugfs.h"
#include "nss_ovpnmgr_priv.h"
#include "nss_ovpnmgr_route.h"
#include "nss_ovpn_sk_priv.h"

/*
 * nss_ovpnmgr_app_find_dev()
 *	Returns net_device on which route tuple is routed
 */
static struct net_device *nss_ovpnmgr_app_find_dev(struct nss_ovpnmgr_route_tuple *rt)
{
	struct net_device *dev;
	struct rt6_info *rt6;
	struct rtable *rt4;

	if (rt->ip_version == IPVERSION) {
		rt4 = ip_route_output(&init_net, rt->ip_addr[0], 0, 0, 0);
		if (IS_ERR(rt4)) {
			return NULL;
		}

		dev = rt4->dst.dev;
		ip_rt_put(rt4);

		return dev;
	}

	rt6 = rt6_lookup(&init_net, (const struct in6_addr *)rt->ip_addr, NULL, 0, 0);
	if (!rt6) {
		return NULL;
	}

	dev = rt6->dst.dev;
	ip6_rt_put(rt6);

	return dev;
}

/*
 * nss_ovpnmgr_app_find_tun()
 *	Returns NSS ifnum.
 */
struct net_device *nss_ovpnmgr_app_find_tun(struct net_device *app_dev, struct nss_ovpnmgr_route_tuple *rt, uint32_t *ifnum)
{
	struct nss_ovpnmgr_app *app;
	struct nss_ovpnmgr_tun *tun;
	struct net_device *dev = NULL;
	uint32_t tun_ifnum = NSS_MAX_NET_INTERFACES;
	ssize_t addr_size;

	addr_size = (rt->ip_version == IPVERSION) ? sizeof(rt->ip_addr[0]) : sizeof(rt->ip_addr);

	read_lock_bh(&ovpnmgr_ctx.lock);

	app = nss_ovpnmgr_app_find(app_dev);
	if (!app) {
		read_unlock_bh(&ovpnmgr_ctx.lock);
		nss_ovpnmgr_warn("%px: Application is not registered, app_dev=%s\n", app_dev, app_dev->name);
		return NULL;
	}

	/*
	 * Check if application device is in UP state. If it is UP then only
	 * allow ECM to push flow rules. This logic will make sure that openvpn
	 * flows are not accelerated when tunnel device is down
	 */
	if (!(app->dev->flags & IFF_UP)) {
		read_unlock_bh(&ovpnmgr_ctx.lock);
		nss_ovpnmgr_warn("%px: Application device not up, app_dev=%s\n", app->dev, app->dev->name);
		return NULL;
	}

	if (app->mode == NSS_OVPNMGR_APP_MODE_CLIENT) {
		/*
		 * There are no routes in client mode.
		 */

		list_for_each_entry(tun, &app->tun_list, list) {

			dev = nss_ovpnmgr_app_find_dev(rt);
			/*
			 * If routed interface is tun/tap return inner interface.
			 */
			if (dev == app->dev) {
				nss_ovpnmgr_info("%px: Found dev = %s\n", app, dev->name);
				tun_ifnum = tun->inner.ifnum;
				dev = tun->dev;
				goto done;
			}

			/*
			 * Check if rt->ip_addr matches tunnel IP.
			 */
			if (!memcmp(rt->ip_addr, &tun->tun_hdr.src_ip, addr_size)) {
				nss_ovpnmgr_info("%px: Encapsulated packet.\n", app);
				tun_ifnum = tun->outer.ifnum;
				dev = tun->dev;
				goto done;
			}
		}
		read_unlock_bh(&ovpnmgr_ctx.lock);
		return NULL;
	}

	list_for_each_entry(tun, &app->tun_list, list) {
		struct nss_ovpnmgr_route *route;

		route = nss_ovpnmgr_route_find(&tun->route_list, rt);
		if (route) {
			tun_ifnum = tun->inner.ifnum;
			dev = tun->dev;
			goto done;
		}

		/*
		 * Check if rt->ip_addr matches tunnel IP.
		 */
		if (!memcmp(rt->ip_addr, &tun->tun_hdr.src_ip, addr_size)) {
			tun_ifnum = tun->outer.ifnum;
			dev = tun->dev;
			goto done;
		}
	}
done:
	read_unlock_bh(&ovpnmgr_ctx.lock);
	*ifnum = tun_ifnum;
	return dev;
}
EXPORT_SYMBOL(nss_ovpnmgr_app_find_tun);

/*
 * nss_ovpnmgr_app_forward()
 *	Send packet to Host
 */
void nss_ovpnmgr_app_forward(struct nss_ovpnmgr_app *app, struct nss_ovpnmgr_tun *tun, struct sk_buff *skb)
{
	struct nss_ovpnmgr_metadata *metadata = (struct nss_ovpnmgr_metadata *)skb->cb;

	BUILD_BUG_ON(sizeof(skb->cb) < sizeof(struct nss_ovpnmgr_metadata));

	metadata->flags = NSS_OVPNMGR_METADATA_FLAG_PKT_DECAP | NSS_OVPNMGR_METADATA_FLAG_PKT_TYPE_CTRL;
	metadata->tunnel_id = tun->tunnel_id;

	if (nss_ovpn_sk_send(skb, app->app_data)) {
		nss_ovpnmgr_warn("%px: failed to send packet to OVPN application.\n", tun);
		tun->inner.stats.host_pkt_drop++;
		dev_kfree_skb_any(skb);
	}
}

/*
 * nss_ovpnmgr_app_find()
 *	Find OVPN application instance.
 */
struct nss_ovpnmgr_app *nss_ovpnmgr_app_find(struct net_device *dev)
{
	struct nss_ovpnmgr_app *app;

	list_for_each_entry(app, &ovpnmgr_ctx.app_list, list) {
		if (app->dev == dev) {
			return app;
		}
	}

	return NULL;
}

/*
 * nss_ovpnmgr_app_del()
 *	Delete OpenVPN application.
 */
int nss_ovpnmgr_app_del(struct net_device *app_dev)
{
	struct nss_ovpnmgr_app *app;
	struct nss_ovpnmgr_tun *tun, *n;

	write_lock_bh(&ovpnmgr_ctx.lock);

	app = nss_ovpnmgr_app_find(app_dev);
	if (!app) {
		write_unlock_bh(&ovpnmgr_ctx.lock);
		nss_ovpnmgr_warn("%px: Application is not registered: app_dev = %s\n", app_dev, app_dev->name);
		return -ENODEV;
	}

	list_del(&app->list);
	write_unlock_bh(&ovpnmgr_ctx.lock);

	nss_ovpnmgr_info("%px: Uninitializing application instance\n", app);

	/*
	 * Application is removed from list.
	 * Check if there are any tunnels.
	 * Delete all the tunnels from NSS FW and free.
	 */
	list_for_each_entry_safe(tun, n, &app->tun_list, list) {
		nss_ovpnmgr_tun_del(tun->tunnel_id);
	}

	dev_put(app_dev);
	nss_ovpnmgr_debugfs_remove(app->dentry);

	kfree(app);
	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_app_del);

/*
 * nss_ovpnmgr_app_add()
 *	Add OpenVPN application.
 */
int nss_ovpnmgr_app_add(struct net_device *app_dev, enum nss_ovpnmgr_app_mode mode, void *app_data)
{
	struct nss_ovpnmgr_app *app;

	BUG_ON(!app_dev);

	nss_ovpnmgr_info("Adding new APP :tun_dev=%s:", app_dev->name);

	app = kzalloc(sizeof(*app), GFP_KERNEL);
	if (!app) {
		nss_ovpnmgr_warn("Failed to allocate memory for app\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&app->tun_list);
	INIT_LIST_HEAD(&app->list);
	/*
	 * Hold tun/tap netdev and release when deleting application.
	 */
	dev_hold(app_dev);

	/*
	 * Initialize application instance.
	 */
	app->mode = mode;
	app->dev = app_dev;
	app->app_data = app_data;
	nss_ovpnmgr_debugfs_create(app);

	write_lock_bh(&ovpnmgr_ctx.lock);
	/*
	 * Check if the application is already registered.
	 */
	if (nss_ovpnmgr_app_find(app_dev)) {
		write_unlock_bh(&ovpnmgr_ctx.lock);
		nss_ovpnmgr_info("%px: Application is already registered: app_dev=%s\n", app_dev, app_dev->name);
		dev_put(app_dev);
		nss_ovpnmgr_debugfs_remove(app->dentry);
		kfree(app);
		return -EEXIST;
	}

	list_add(&app->list, &ovpnmgr_ctx.app_list);
	write_unlock_bh(&ovpnmgr_ctx.lock);

	nss_ovpnmgr_info("%px: Application is registered successfully\n", app);
	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_app_add);
