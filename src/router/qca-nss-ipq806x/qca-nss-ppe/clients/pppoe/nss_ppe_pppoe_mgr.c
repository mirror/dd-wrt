/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/rwlock_types.h>
#include <linux/hashtable.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <net/route.h>
#include <linux/if_pppox.h>
#include <net/ip.h>
#include <linux/if_bridge.h>
#include <net/bonding.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif
#include <ppe_drv.h>
#include <ppe_drv_pppoe_session.h>
#include "nss_ppe_pppoe_mgr.h"

#define HASH_BUCKET_SIZE 2  /* ( 2^ HASH_BUCKET_SIZE ) == 4 */

static DEFINE_HASHTABLE(pppoe_session_table, HASH_BUCKET_SIZE);

/*
 * nss_ppe_pppoe_mgr_get_session()
 *	Retrieve PPPoE session associated with this netdevice if any
 */
static bool nss_ppe_pppoe_mgr_get_session(struct net_device *dev, struct pppoe_opt *addressing)
{
	struct ppp_channel *channel[1] = {NULL};
	int px_proto;
	int ppp_ch_count;

	if (ppp_is_multilink(dev)) {
		nss_ppe_pppoe_mgr_warn("%px: channel is multilink PPP\n", dev);
		return false;
	}

	ppp_ch_count = ppp_hold_channels(dev, channel, 1);
	nss_ppe_pppoe_mgr_info("%px: PPP hold channel ret %d\n", dev, ppp_ch_count);
	if (ppp_ch_count != 1) {
		nss_ppe_pppoe_mgr_warn("%px: hold channel for netdevice failed\n", dev);
		return false;
	}

	px_proto = ppp_channel_get_protocol(channel[0]);
	if (px_proto != PX_PROTO_OE) {
		nss_ppe_pppoe_mgr_warn("%px: session socket is not of type PX_PROTO_OE\n", dev);
		ppp_release_channels(channel, 1);
		return false;
	}

	if (pppoe_channel_addressing_get(channel[0], addressing)) {
		nss_ppe_pppoe_mgr_warn("%px: failed to get addressing information\n", dev);
		ppp_release_channels(channel, 1);
		return false;
	}

	/*
	 * pppoe_channel_addressing_get returns held device.
	 * So, put it back here.
	 */
	dev_put(addressing->dev);
	ppp_release_channels(channel, 1);
	return true;
}

/*
 * nss_ppe_pppoe_mgr_remove_session()
 *	Add PPPoE session entry into Hash table
 */
static void nss_ppe_pppoe_mgr_remove_session(struct nss_ppe_pppoe_mgr_session_entry *entry)
{
	struct nss_ppe_pppoe_mgr_session_info *info;
	info = &entry->info;

	nss_ppe_pppoe_mgr_info("%px: Remove PPPoE session with session_id=%u server_mac=%pM local_mac %pM\n",
			       entry, info->session_id, info->server_mac, info->local_mac);

	hash_del_rcu(&entry->hash_list);
	synchronize_rcu();
	kfree(entry);
}

/*
 * nss_ppe_pppoe_mgr_add_session()
 *	Add PPPoE session entry into Hash table
 */
static struct nss_ppe_pppoe_mgr_session_entry *nss_ppe_pppoe_mgr_add_session(struct net_device *dev, struct pppoe_opt *opt, struct ppe_drv_iface *iface)

{
	struct nss_ppe_pppoe_mgr_session_entry *entry = NULL;
	struct nss_ppe_pppoe_mgr_session_info *info;

	entry = kmalloc(sizeof(struct nss_ppe_pppoe_mgr_session_entry),
				      GFP_KERNEL);
	if (!entry) {
		nss_ppe_pppoe_mgr_warn("%px: failed to allocate pppoe session entry\n", dev);
		return NULL;
	}

	entry->iface = iface;
	info = &entry->info;

	/*
	 * Get session info
	 */
	info->session_id = (uint16_t)ntohs((uint16_t)opt->pa.sid);
	ether_addr_copy(info->server_mac, opt->pa.remote);
	ether_addr_copy(info->local_mac, opt->dev->dev_addr);

	nss_ppe_pppoe_mgr_info("%px: Added PPPoE session with session_id=%u server_mac=%pM local_mac %pM\n",
			       dev, info->session_id, info->server_mac, info->local_mac);

	entry->dev = dev;

	/*
	 * There is no need for protecting simultaneous addition &
	 * deletion of pppoe sesion entry as the PPP notifier chain
	 * call back is called with mutex lock.
	 */
	hash_add_rcu(pppoe_session_table,
		&entry->hash_list,
		dev->ifindex);

	return entry;
}

/*
 * nss_ppe_pppoe_mgr_disconnect()
 *	pppoe interface's disconnect event handler
 */
static int nss_ppe_pppoe_mgr_disconnect(struct net_device *dev)
{
	struct nss_ppe_pppoe_mgr_session_entry *entry;
	struct nss_ppe_pppoe_mgr_session_entry *found = NULL;
	struct hlist_node *temp;
	ppe_drv_ret_t ret;
	struct ppe_drv_iface *iface;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		return NOTIFY_DONE;
	}

	hash_for_each_possible_safe(pppoe_session_table, entry,
				     temp, hash_list, dev->ifindex) {
		if (entry->dev != dev) {
			continue;
		}

		/*
		 * In the hash list, there must be only one entry match with this net device.
		 */
		found = entry;
		break;
	}

	if (!found) {
		nss_ppe_pppoe_mgr_warn("%px: PPPoE session is not found for device: %s\n", dev, dev->name);
		return NOTIFY_DONE;
	}

	iface = found->iface;
	ret = ppe_drv_pppoe_session_deinit(iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_pppoe_mgr_warn("%px: Unable to deinitialize PPPoE session in PPE\n", dev);
	}

	nss_ppe_pppoe_mgr_remove_session(found);
	ppe_drv_iface_deref(iface);
	return NOTIFY_DONE;
}

/*
 * nss_ppe_pppoe_mgr_connect()
 *	PPPoE interface's connect event handler
 */
static int nss_ppe_pppoe_mgr_connect(struct net_device *dev)
{
	struct pppoe_opt opt;
	struct nss_ppe_pppoe_mgr_session_entry *entry = NULL;
	struct nss_ppe_pppoe_mgr_session_info *info;
	struct net_device *actual_dev = dev;
	ppe_drv_ret_t ret;
	struct ppe_drv_iface *iface;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		return NOTIFY_DONE;
	}

	if (!nss_ppe_pppoe_mgr_get_session(dev, &opt)) {
		nss_ppe_pppoe_mgr_warn("%px: Unable to get pppoe session from the netdev\n", dev);
		return NOTIFY_DONE;
	}

	iface = ppe_drv_iface_alloc(PPE_DRV_IFACE_TYPE_PPPOE, dev);
	if (!iface) {
		nss_ppe_pppoe_mgr_warn("%px: PPPoE PPE iface alloc failed\n", dev);
		return NOTIFY_DONE;
	}

	entry = nss_ppe_pppoe_mgr_add_session(dev, &opt, iface);
	if (!entry) {
		nss_ppe_pppoe_mgr_warn("%px: PPPoE session add failed\n", dev);
		ppe_drv_iface_deref(iface);
		return NOTIFY_DONE;
	}

	info = &entry->info;

	/*
	 * PPPoE connection could be on a bridge device like br-wan or on bond device like bond0.
	 * So, check if the opt device is bridge or bond.
	 */
	actual_dev = opt.dev;
	if (netif_is_bond_master(opt.dev)) {
		int32_t bondid = -1;
#if defined(BONDING_SUPPORT)
		bondid = bond_get_id(opt.dev);
#endif
		if (bondid < 0) {
			nss_ppe_pppoe_mgr_warn("%px: Invalid LAG group id 0x%x\n", dev, bondid);
			goto fail;
		}
	}

	ret = ppe_drv_pppoe_session_init(entry->iface, actual_dev, info->session_id, info->server_mac, info->local_mac);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_pppoe_mgr_warn("%px: Unable to initialize PPPoE session in PPE\n", dev);
		goto fail;
	}

	ret = ppe_drv_iface_mtu_set(entry->iface, actual_dev->mtu - PPPOE_SES_HLEN);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_pppoe_mgr_warn("%px: failed to set mtu, error = %d \n", dev, ret);
		goto fail2;
	}

	entry->mtu = dev->mtu;

	nss_ppe_pppoe_mgr_info("%px: session_id %d server_mac %pM local_mac %pM base_if %s\n",
			       dev, info->session_id,
			       info->server_mac, info->local_mac, opt.dev->name);

	return NOTIFY_DONE;

fail2:
	ret = ppe_drv_pppoe_session_deinit(entry->iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_pppoe_mgr_warn("%px: Unable to deinitialize PPPoE session in PPE\n", dev);
	}

fail:
	nss_ppe_pppoe_mgr_remove_session(entry);
	ppe_drv_iface_deref(iface);
	return NOTIFY_DONE;
}


/*
 * nss_ppe_pppoe_mgr_changemtu_event()
 *     Change PPPoE MTU.
 */
static int nss_ppe_pppoe_mgr_changemtu_event(struct net_device *dev)
{
	struct nss_ppe_pppoe_mgr_session_entry *entry;
	struct nss_ppe_pppoe_mgr_session_entry *found = NULL;
	struct hlist_node *temp;
	ppe_drv_ret_t ret;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		return NOTIFY_DONE;
	}

	hash_for_each_possible_safe(pppoe_session_table, entry,
				     temp, hash_list, dev->ifindex) {
		if (entry->dev != dev) {
			continue;
		}

		/*
		 * In the hash list, there must be only one entry match with this net device.
		 */
		found = entry;
		break;
	}

	if (!found) {
		nss_ppe_pppoe_mgr_warn("%px: PPPoE session is not found for device: %s\n", dev, dev->name);
		return NOTIFY_DONE;
	}

	if (entry->mtu == dev->mtu) {
		return NOTIFY_DONE;
	}

	nss_ppe_pppoe_mgr_info("%px: MTU changed to %d, \n", dev, dev->mtu);
	ret = ppe_drv_iface_mtu_set(entry->iface, dev->mtu);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_pppoe_mgr_warn("%px: failed to set mtu, error = %d \n", dev, ret);
		return NOTIFY_BAD;
	}

	entry->mtu = dev->mtu;
	return NOTIFY_DONE;
}

/*
 * nss_ppe_pppoe_mgr_channel_notifier_handler()
 *	PPPoE channel notifier handler.
 */
static int nss_ppe_pppoe_mgr_channel_notifier_handler(struct notifier_block *nb,
							unsigned long event,
							void *arg)
{
	struct net_device *dev = (struct net_device *)arg;

	switch (event) {
	case PPP_CHANNEL_CONNECT:
		return nss_ppe_pppoe_mgr_connect(dev);
	case PPP_CHANNEL_DISCONNECT:
		return nss_ppe_pppoe_mgr_disconnect(dev);
	case NETDEV_CHANGEMTU:
		return nss_ppe_pppoe_mgr_changemtu_event(dev);

	default:
		nss_ppe_pppoe_mgr_info("%px: Unhandled channel event: %lu\n", dev, event);
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block nss_ppe_pppoe_mgr_channel_notifier_nb = {
	.notifier_call = nss_ppe_pppoe_mgr_channel_notifier_handler,
};

/*
 * nss_ppe_pppoe_mgr_exit_module
 *     PPPoE PPE module exit function
 */
void __exit nss_ppe_pppoe_mgr_exit_module(void)
{
	/*
	 * Unregister the module from the PPP channel events.
	 */
	ppp_channel_connection_unregister_notify(&nss_ppe_pppoe_mgr_channel_notifier_nb);
}

/*
 * nss_ppe_pppoe_mgr_init_module()
 *	PPPoE PPE module init function
 */
int __init nss_ppe_pppoe_mgr_init_module(void)
{
	/*
	 * Register the module to the PPP channel events.
	 */
	ppp_channel_connection_register_notify(&nss_ppe_pppoe_mgr_channel_notifier_nb);
	return 0;
}

module_init(nss_ppe_pppoe_mgr_init_module);
module_exit(nss_ppe_pppoe_mgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS pppoe PPE manager");
