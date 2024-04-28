/*
 **************************************************************************
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
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
#ifdef CONFIG_OF
#include <linux/of.h>
#endif
#include <nss_api_if.h>
#include <nss_dynamic_interface.h>
#include "nss_connmgr_pppoe.h"

#define HASH_BUCKET_SIZE 2  /* ( 2^ HASH_BUCKET_SIZE ) == 4 */

static DEFINE_HASHTABLE(pppoe_session_table, HASH_BUCKET_SIZE);

/*
 * nss_connmgr_pppoe_get_session()
 *	Retrieve pppoe session associated with this netdevice if any
 */
static int nss_connmgr_pppoe_get_session(struct net_device *dev,  struct pppoe_opt *addressing)
{
	struct ppp_channel *channel[1] = {NULL};
	int px_proto;
	int ppp_ch_count;

	if (ppp_is_multilink(dev)) {
		nss_connmgr_pppoe_warn("%p: channel is multilink PPP\n", dev);
		return -1;
	}

	ppp_ch_count = ppp_hold_channels(dev, channel, 1);
	nss_connmgr_pppoe_info("%p: PPP hold channel ret %d\n", dev, ppp_ch_count);
	if (ppp_ch_count != 1) {
		nss_connmgr_pppoe_warn("%p: hold channel for netdevice failed\n", dev);
		return -1;
	}

	px_proto = ppp_channel_get_protocol(channel[0]);
	if (px_proto != PX_PROTO_OE) {
		nss_connmgr_pppoe_warn("%p: session socket is not of type PX_PROTO_OE\n", dev);
		ppp_release_channels(channel, 1);
		return -1;
	}

	pppoe_channel_addressing_get(channel[0], addressing);

	dev_put(addressing->dev);
	ppp_release_channels(channel, 1);
	return 0;
}

/*
 * nss_connmgr_add_pppoe_session()
 *	Add PPPoE session entry into Hash table
 */
static struct nss_connmgr_pppoe_session_entry *nss_connmgr_add_pppoe_session(struct net_device *dev, struct pppoe_opt *opt)

{
	struct nss_connmgr_pppoe_session_entry *entry = NULL;
	struct nss_connmgr_pppoe_session_info *info;

	entry = kmalloc(sizeof(struct nss_connmgr_pppoe_session_entry),
				      GFP_KERNEL);
	if (!entry) {
		nss_connmgr_pppoe_warn("%p: failed to allocate pppoe session entry\n", dev);
		return NULL;
	}

	info = &entry->info;

	/*
	 * Get session info
	 */
	info->session_id = (uint16_t)ntohs((uint16_t)opt->pa.sid);
	ether_addr_copy(info->server_mac, opt->pa.remote);
	ether_addr_copy(info->local_mac, opt->dev->dev_addr);

	nss_connmgr_pppoe_info("%p: Added PPPoE session with session_id=%u server_mac=%pM local_mac %pM\n",
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
 * nss_connmgr_pppoe_disconnect()
 *	pppoe interface's disconnect event handler
 */
static int nss_connmgr_pppoe_disconnect(struct net_device *dev)
{
	struct nss_connmgr_pppoe_session_entry *entry;
	struct nss_connmgr_pppoe_session_entry *found = NULL;
	struct hlist_node *temp;
	struct nss_pppoe_msg npm;
	struct nss_pppoe_destroy_msg *npm_destroy;
	nss_tx_status_t status;
	int if_number;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		return NOTIFY_DONE;
	}

	/*
	 * Check if pppoe is registered ?
	 */
	if_number = nss_cmn_get_interface_number_by_dev(dev);
	if (if_number < 0) {
		nss_connmgr_pppoe_warn("%p: Net device is not registered with nss\n", dev);
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
		nss_connmgr_pppoe_warn("%p: PPPoE session is not found for device: %s\n", dev, dev->name);
		return NOTIFY_DONE;
	}

	hash_del_rcu(&entry->hash_list);
	synchronize_rcu();

	memset(&npm, 0, sizeof(struct nss_pppoe_msg));
	npm_destroy = &npm.msg.destroy;
	npm_destroy->session_id = entry->info.session_id;
	ether_addr_copy(npm_destroy->server_mac, entry->info.server_mac);
	ether_addr_copy(npm_destroy->local_mac, entry->info.local_mac);

	nss_pppoe_msg_init(&npm, if_number, NSS_PPPOE_MSG_SESSION_DESTROY, sizeof(struct nss_pppoe_destroy_msg), NULL, NULL);
	status = nss_pppoe_tx_msg_sync(nss_pppoe_get_context(), &npm);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pppoe_warn("%p: pppoe session destroy command failed, if_number = %d\n", dev, if_number);
		goto done;
	}
	nss_unregister_pppoe_session_if(if_number);
	status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_PPPOE);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pppoe_warn("%p: pppoe dealloc node failure for if_number=%d\n", dev, if_number);
	} else {
		nss_connmgr_pppoe_info("%p: PPPoE session is destroyed with if_number %d, session_id %d, server_mac %pM, local_mac %pM\n",
				       dev, if_number, entry->info.session_id,
				       entry->info.server_mac, entry->info.local_mac);
	}
done:
	kfree(entry);
	return NOTIFY_DONE;
}

/*
 * nss_connmgr_pppoe_connect()
 *	pppoe interface's connect event handler
 */
static int nss_connmgr_pppoe_connect(struct net_device *dev)
{
	struct pppoe_opt opt;
	struct nss_connmgr_pppoe_session_entry *entry = NULL;
	struct nss_connmgr_pppoe_session_info *info;
	nss_tx_status_t status;
	struct nss_ctx_instance *nss_ctx;
	uint32_t features = 0;
	int32_t if_number;
	struct nss_pppoe_msg npm;
	struct nss_pppoe_create_msg *npm_create;
	int ret;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		return NOTIFY_DONE;
	}

	/*
	 * Check if pppoe is already registered.
	 */
	if_number = nss_cmn_get_interface_number_by_dev(dev);
	if (if_number >= 0) {
		nss_connmgr_pppoe_warn("%p: Net device is already registered with nss\n", dev);
		return NOTIFY_DONE;
	}

	ret = nss_connmgr_pppoe_get_session(dev, &opt);
	if (ret < 0) {
		nss_connmgr_pppoe_warn("%p: Unable to get pppoe session from the netdev\n", dev);
		return NOTIFY_DONE;
	}

	/*
	 * Create nss dynamic interface and register
	 */
	if_number = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_PPPOE);
	if (if_number == -1) {
		nss_connmgr_pppoe_warn("%p: Request interface number failed\n", dev);
		return NOTIFY_DONE;
	}

	if (!nss_is_dynamic_interface(if_number)) {
		nss_connmgr_pppoe_warn("%p: Invalid NSS dynamic I/F number %d\n", dev, if_number);
		goto connect_fail1;
	}

	nss_connmgr_pppoe_info("%p: PPPoE dynamic interface allocation is successful with if_number %d\n", dev, if_number);

	entry = nss_connmgr_add_pppoe_session(dev, &opt);
	if (!entry) {
		nss_connmgr_pppoe_warn("%p: PPPoE session add failed %d\n", dev, if_number);
		goto connect_fail1;
	}

	/*
	 * Register pppoe  tunnel with NSS
	 */
	nss_ctx = nss_register_pppoe_session_if(if_number,
				       NULL,
				       dev,
				       features,
				       entry);
	if (!nss_ctx) {
		nss_connmgr_pppoe_warn("%p: nss_register_pppoe_session_if failed\n", dev);
		goto connect_fail2;
	}

	nss_connmgr_pppoe_info("%p: PPPoE session interface registration is successful\n", nss_ctx);


	memset(&npm, 0, sizeof(struct nss_pppoe_msg));
	npm_create = &npm.msg.create;
	info = &entry->info;
	npm_create->session_id = info->session_id;

	/*
	 * PPPoE connection could be on a bridge device like br-wan or on bond device like bond0.
	 * So, check if the opt device is bridge or bond.
	 */
	if (netif_is_bond_master(opt.dev)) {
		int32_t bondid = -1;
#if IS_ENABLED(CONFIG_BONDING)
		bondid = bond_get_id(opt.dev);
#endif
		if (bondid < 0) {
			nss_connmgr_pppoe_warn("%p: Invalid LAG group id 0x%x\n", dev, bondid);
			goto connect_fail3;
		}
		npm_create->base_if_num = bondid + NSS_LAG0_INTERFACE_NUM;
	} else if (opt.dev->priv_flags & IFF_EBRIDGE) {
		/*
		 * Device is bridge. We need to get the actual physical port.
		 * Searching this physical port in the fdb database with the server mac
		 * address of the session.
		 */
		struct net_device *port = br_port_dev_get(opt.dev, info->server_mac, NULL, 0);
		if (!port) {
			nss_connmgr_pppoe_warn("%p: Unable to get the bridge port device from the bridge interface: %s\n",
						dev, opt.dev->name);
			goto connect_fail3;
		}

		/*
		 * Get the interface number of the base interface.
		 */
		npm_create->base_if_num = nss_cmn_get_interface_number_by_dev(port);
		nss_connmgr_pppoe_info("local_mac: %pM server_mac: %pM opt.dev: %s base_if: %d\n",
					info->local_mac, info->server_mac, opt.dev->name, npm_create->base_if_num);
		/*
		 * Release the port which was held by the br_port_dev_get() call.
		 */
		dev_put(port);
	} else {
		/*
		 * PPPoE sessions can be created over either a physical or virtual interface. They may not be
		 * created over a LAG or bridge interface. This interface is the base interface which will be used
		 * for HW next_hop. On SoCs, which do not have HW acceleration, this interface number is not used
		 * and -1 is returned.
		 */
		npm_create->base_if_num = nss_cmn_get_interface_number_by_dev(opt.dev);
	}

	ether_addr_copy(npm_create->server_mac, info->server_mac);
	ether_addr_copy(npm_create->local_mac, info->local_mac);
	npm_create->mtu = dev->mtu;

	nss_connmgr_pppoe_info("%p: pppoe info\n", dev);
	nss_connmgr_pppoe_info("%p: session_id %d server_mac %pM local_mac %pM base_if %s (%d)\n",
			       dev, npm_create->session_id,
			       npm_create->server_mac, npm_create->local_mac, opt.dev->name, npm_create->base_if_num);
	nss_connmgr_pppoe_info("%p: Sending pppoe session create command to NSS\n", dev);

	nss_pppoe_msg_init(&npm, if_number, NSS_PPPOE_MSG_SESSION_CREATE, sizeof(struct nss_pppoe_create_msg), NULL, NULL);

	status = nss_pppoe_tx_msg_sync(nss_ctx, &npm);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pppoe_warn("%p: nss pppoe session creation command error %d\n", dev, status);
		goto connect_fail3;
	}
	nss_connmgr_pppoe_info("%p: PPPoE session creation is successful\n", dev);

	return NOTIFY_DONE;

connect_fail3:
	nss_unregister_pppoe_session_if(if_number);

connect_fail2:
	hash_del_rcu(&entry->hash_list);
	synchronize_rcu();
	kfree(entry);

connect_fail1:
	status = nss_dynamic_interface_dealloc_node(if_number, NSS_DYNAMIC_INTERFACE_TYPE_PPPOE);
	if (status != NSS_TX_SUCCESS) {
		nss_connmgr_pppoe_warn("%p: Unable to dealloc the node[%d] in the NSS fw!\n", dev, if_number);
	}

	return NOTIFY_DONE;
}

/*
 * nss_connmgr_pppoe_channel_notifier_handler()
 *	PPPoE channel notifier handler.
 */
static int nss_connmgr_pppoe_channel_notifier_handler(struct notifier_block *nb,
							unsigned long event,
							void *arg)
{
	struct net_device *dev = (struct net_device *)arg;

	switch (event) {
	case PPP_CHANNEL_CONNECT:
		return nss_connmgr_pppoe_connect(dev);
	case PPP_CHANNEL_DISCONNECT:
		return nss_connmgr_pppoe_disconnect(dev);
	default:
		nss_connmgr_pppoe_info("%p: Unhandled channel event: %lu\n", dev, event);
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block nss_connmgr_pppoe_channel_notifier_nb = {
        .notifier_call = nss_connmgr_pppoe_channel_notifier_handler,
};

/*
 * nss_connmgr_pppoe_exit_module
 *	pppoe module exit function
 */
void __exit nss_connmgr_pppoe_exit_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

	/*
	 * Unregister the module from the PPP channel events.
	 */
	ppp_channel_connection_unregister_notify(&nss_connmgr_pppoe_channel_notifier_nb);
}

/*
 * nss_connmgr_pppoe_init_module()
 *	pppoe module init function
 */
int __init nss_connmgr_pppoe_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	/*
	 * Register the module to the PPP channel events.
	 */
	ppp_channel_connection_register_notify(&nss_connmgr_pppoe_channel_notifier_nb);
	return 0;
}

module_init(nss_connmgr_pppoe_init_module);
module_exit(nss_connmgr_pppoe_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS pppoe offload manager");
