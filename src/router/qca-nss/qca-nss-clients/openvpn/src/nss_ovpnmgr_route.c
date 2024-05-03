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
 * nss_ovpnmgr_route.c
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
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/skcipher.h>
#include <crypto/aes.h>
#include <crypto/authenc.h>
#include <crypto/des.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/hash.h>

#include <nss_api_if.h>
#include <nss_cryptoapi.h>
#include <nss_qvpn.h>
#include <nss_ovpnmgr.h>
#include "nss_ovpnmgr_crypto.h"
#include "nss_ovpnmgr_tun.h"
#include "nss_ovpnmgr_app.h"
#include "nss_ovpnmgr_debugfs.h"
#include "nss_ovpnmgr_priv.h"
#include "nss_ovpnmgr_route.h"

/*
 * nss_ovpnmgr_route_find()
 *	Return route for the given tunnel and address
 */
struct nss_ovpnmgr_route *nss_ovpnmgr_route_find(struct list_head *rt_list, struct nss_ovpnmgr_route_tuple *rt)
{
	struct nss_ovpnmgr_route *route;

	list_for_each_entry(route, rt_list, list) {
		if (route->rt.ip_version != rt->ip_version) {
			continue;
		}

		if (!memcmp(route->rt.ip_addr, rt->ip_addr, sizeof(rt->ip_addr))) {
			return route;
		}
	}

	return NULL;
}

/*
 * nss_ovpnmgr_route_set_active()
 *	Update OVPN tunnel route.
 */
int nss_ovpnmgr_route_set_active(struct list_head *rt_list, struct nss_ovpnmgr_route_tuple *rt)
{
	struct nss_ovpnmgr_route *route;

	/*
	 * This API should be called under lock.
	 */
	lockdep_assert_held(&ovpnmgr_ctx.lock);

	/*
	 * Search for route entry with from_addr.
	 */
	route = nss_ovpnmgr_route_find(rt_list, rt);
	if (!route) {
		nss_ovpnmgr_warn("%px: Route %x:%x:%x:%x not found\n", rt_list, rt->ip_addr[0],
				rt->ip_addr[1], rt->ip_addr[2], rt->ip_addr[3]);
		return -EINVAL;
	}

	route->in_use++;
	return 0;
}

/*
 * nss_ovpnmgr_route_is_active()
 *	Get route state.
 */
bool nss_ovpnmgr_route_is_active(uint32_t tunnel_id, struct nss_ovpnmgr_route_tuple *rt)
{
	struct nss_ovpnmgr_route *route;
	struct net_device *tun_dev;
	struct nss_ovpnmgr_tun *tun;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel: tunnel_id = %u\n", tunnel_id);
		return false;
	}

	tun = netdev_priv(tun_dev);

	read_lock_bh(&ovpnmgr_ctx.lock);

	route = nss_ovpnmgr_route_find(&tun->route_list, rt);
	if (!route) {
		/*
		 * Route is not found.
		 */
		read_unlock_bh(&ovpnmgr_ctx.lock);
		dev_put(tun_dev);
		return false;
	}

	/*
	 * in_use is updated when packets hit flow entries in firmware are active
	 */
	if (route->in_use) {
		route->in_use = 0;
		read_unlock_bh(&ovpnmgr_ctx.lock);
		nss_ovpnmgr_info("%px: Route is active, don't delete\n", tun);
		dev_put(tun_dev);
		return true;
	}

	read_unlock_bh(&ovpnmgr_ctx.lock);

	dev_put(tun_dev);
	return false;
}
EXPORT_SYMBOL(nss_ovpnmgr_route_is_active);

/*
 * nss_ovpnmgr_route_del()
 *	Delete route from the tunnel.
 */
int nss_ovpnmgr_route_del(uint32_t tunnel_id, struct nss_ovpnmgr_route_tuple *rt)
{
	struct nss_ovpnmgr_route *route;
	struct net_device *tun_dev;
	struct nss_ovpnmgr_tun *tun;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel: tunnel_id = %u\n\n", tunnel_id);
		return -ENODEV;
	}

	tun = netdev_priv(tun_dev);
	write_lock_bh(&ovpnmgr_ctx.lock);

	route = nss_ovpnmgr_route_find(&tun->route_list, rt);
	if (!route) {
		/*
		 * Route is not found.
		 */
		write_unlock_bh(&ovpnmgr_ctx.lock);
		dev_put(tun_dev);
		return -EINVAL;
	}

	nss_ovpnmgr_info("%px: Deleting route on tunnel id:%d\n", route, tunnel_id);

	list_del(&route->list);
	write_unlock_bh(&ovpnmgr_ctx.lock);

	kfree(route);
	dev_put(tun_dev);
	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_route_del);

/*
 * nss_ovpnmgr_route_add()
 *	Add new route for the given tunnel
 */
int nss_ovpnmgr_route_add(uint32_t tunnel_id, struct nss_ovpnmgr_route_tuple *rt)
{
	struct nss_ovpnmgr_route *route;
	struct net_device *tun_dev;
	struct nss_ovpnmgr_tun *tun;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel: tunnel_id = %u\n\n", tunnel_id);
		return -ENODEV;
	}

	tun = netdev_priv(tun_dev);

	/*
	 * Check if route is available.
	 */
	route = nss_ovpnmgr_route_find(&tun->route_list, rt);
	if (route) {
		nss_ovpnmgr_warn("%px: Route is available\n", tun);
		dev_put(tun_dev);
		return -EEXIST;
	}

	route = kzalloc(sizeof(*route), GFP_KERNEL);
	if (!route) {
		nss_ovpnmgr_warn("%px: Couldn't allocate memory for new route\n", tun);
		dev_put(tun_dev);
		return -ENOMEM;
	}

	memcpy(&route->rt, rt, sizeof(*rt));
	nss_ovpnmgr_info("%px: version = %d, IP = %x:%x:%x:%x\n", tun,
			route->rt.ip_version, route->rt.ip_addr[0],
			route->rt.ip_addr[1], route->rt.ip_addr[2],
			route->rt.ip_addr[3]);

	write_lock_bh(&ovpnmgr_ctx.lock);
	list_add(&route->list, &tun->route_list);
	write_unlock_bh(&ovpnmgr_ctx.lock);

	dev_put(tun_dev);
	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_route_add);
