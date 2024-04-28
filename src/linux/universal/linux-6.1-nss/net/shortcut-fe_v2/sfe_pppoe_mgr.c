/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/module.h>
#include <linux/types.h>
#include <linux/rwlock_types.h>
#include <linux/hashtable.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/if_pppox.h>
#include "sfe_pppoe_mgr.h"
#include "sfe_debug.h"

#define HASH_BUCKET_SIZE 2  /* ( 2^ HASH_BUCKET_SIZE ) == 4 */

static DEFINE_HASHTABLE(pppoe_session_table, HASH_BUCKET_SIZE);

/*
 * sfe_pppoe_mgr_get_session_info()
 *	Retrieve PPPoE session info associated with this netdevice
 */
static bool sfe_pppoe_mgr_get_session_info(struct net_device *dev, struct pppoe_opt *addressing)
{
	struct ppp_channel *channel[1] = {NULL};
	int px_proto;
	int ppp_ch_count;

	if (ppp_is_multilink(dev)) {
		DEBUG_WARN("%s: channel is multilink PPP\n", dev->name);
		return false;
	}

	ppp_ch_count = ppp_hold_channels(dev, channel, 1);
	DEBUG_INFO("%s: PPP hold channel ret %d\n", dev->name, ppp_ch_count);
	if (ppp_ch_count != 1) {
		DEBUG_WARN("%s: hold channel for netdevice %px failed\n", dev->name, dev);
		return false;
	}

	px_proto = ppp_channel_get_protocol(channel[0]);
	if (px_proto != PX_PROTO_OE) {
		DEBUG_WARN("%s: session socket is not of type PX_PROTO_OE\n", dev->name);
		ppp_release_channels(channel, 1);
		return false;
	}

	if (pppoe_channel_addressing_get(channel[0], addressing)) {
		DEBUG_WARN("%s: failed to get addressing information\n", dev->name);
		ppp_release_channels(channel, 1);
		return false;
	}

	DEBUG_TRACE("dev=%px %s %d: opt_dev=%px opt_dev_name=%s opt_dev_ifindex=%d opt_ifindex=%d\n",
			dev, dev->name, dev->ifindex,
			addressing->dev, addressing->dev->name, addressing->dev->ifindex, addressing->ifindex);

	/*
	 * pppoe_channel_addressing_get returns held device.
	 * So, put it back here.
	 */
	dev_put(addressing->dev);
	ppp_release_channels(channel, 1);
	return true;
}

/*
 * sfe_pppoe_mgr_remove_session()
 *	Remove PPPoE session entry from hash table
 */
static void sfe_pppoe_mgr_remove_session(struct sfe_pppoe_mgr_session_entry *entry)
{
	struct sfe_pppoe_mgr_session_info *info;
	info = &entry->info;

	DEBUG_INFO("%px %s %d: Remove PPPoE session entry with session_id=%u server_mac=%pM\n",
			       entry, entry->dev->name, entry->dev->ifindex,
			       info->session_id, info->server_mac);

	hash_del_rcu(&entry->hash_list);
	synchronize_rcu();
	kfree(entry);
}

/*
 * sfe_pppoe_mgr_add_session()
 *	Create a PPPoE session entry and add it into hash table
 */
static struct sfe_pppoe_mgr_session_entry *sfe_pppoe_mgr_add_session(struct net_device *dev, struct pppoe_opt *opt)

{
	struct sfe_pppoe_mgr_session_entry *entry;
	struct sfe_pppoe_mgr_session_info *info;

	entry = kzalloc(sizeof(struct sfe_pppoe_mgr_session_entry), GFP_KERNEL);
	if (!entry) {
		DEBUG_WARN("%px: failed to allocate pppoe session entry\n", dev);
		return NULL;
	}

	info = &entry->info;

	/*
	 * Save session info
	 */
	info->session_id = (uint16_t)ntohs((uint16_t)opt->pa.sid);
	ether_addr_copy(info->server_mac, opt->pa.remote);

	entry->dev = dev;

	/*
	 * There is no need for protecting simultaneous addition &
	 * deletion of pppoe sesion entry as the PPP notifier chain
	 * call back is called with mutex lock.
	 */
	hash_add_rcu(pppoe_session_table,
		&entry->hash_list,
		dev->ifindex);

	DEBUG_INFO("%px %s %d: Add PPPoE session entry with session_id=%u server_mac=%pM\n",
			       entry, dev->name, dev->ifindex,
			       info->session_id, info->server_mac);

	return entry;
}

/*
 * sfe_pppoe_mgr_disconnect()
 *	PPPoE interface's disconnect event handler
 */
static int sfe_pppoe_mgr_disconnect(struct net_device *dev)
{
	struct sfe_pppoe_mgr_session_entry *entry;
	struct sfe_pppoe_mgr_session_entry *found = NULL;
	struct hlist_node *temp;
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
		DEBUG_WARN("%px: PPPoE session is not found for %s\n", dev, dev->name);
		return NOTIFY_DONE;
	}

	/*
	 * Remove entry from hash table
	 */
	sfe_pppoe_mgr_remove_session(found);

	return NOTIFY_DONE;
}

/*
 * sfe_pppoe_mgr_connect()
 *	PPPoE interface's connect event handler
 */
static int sfe_pppoe_mgr_connect(struct net_device *dev)
{
	struct pppoe_opt opt;
	struct sfe_pppoe_mgr_session_entry *entry;

	/*
	 * check whether the interface is of type PPP
	 */
	if (dev->type != ARPHRD_PPP || !(dev->flags & IFF_POINTOPOINT)) {
		return NOTIFY_DONE;
	}

	if (sfe_pppoe_mgr_get_session_info(dev, &opt) == false) {
		DEBUG_WARN("%px: Unable to get pppoe session info from %s\n", dev, dev->name);
		return NOTIFY_DONE;
	}

	/*
	 * Create an session entry and add it to hash table
	 */
	entry = sfe_pppoe_mgr_add_session(dev, &opt);
	if (!entry) {
		DEBUG_WARN("%s: PPPoE session add failed\n", dev->name);
	}

	return NOTIFY_DONE;
}

/*
 * sfe_pppoe_mgr_channel_notifier_handler()
 *	PPPoE channel notifier handler.
 */
static int sfe_pppoe_mgr_channel_notifier_handler(struct notifier_block *nb,
							unsigned long event,
							void *arg)
{
	struct net_device *dev = (struct net_device *)arg;

	switch (event) {
	case PPP_CHANNEL_CONNECT:
		DEBUG_INFO("%s: PPP_CHANNEL_CONNECT event\n", dev->name);
		return sfe_pppoe_mgr_connect(dev);

	case PPP_CHANNEL_DISCONNECT:
		DEBUG_INFO("%s: PPP_CHANNEL_DISCONNECT event\n", dev->name);
		return sfe_pppoe_mgr_disconnect(dev);

	default:
		DEBUG_INFO("%s: Unhandled channel event: %lu\n", dev->name, event);
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block sfe_pppoe_mgr_channel_notifier_nb = {
	.notifier_call = sfe_pppoe_mgr_channel_notifier_handler,
};

/*
 * sfe_pppoe_mgr_find_session()
 *	Find pppoe session entry given session ID and server MAC
 */
bool sfe_pppoe_mgr_find_session(uint16_t session_id, uint8_t *server_mac)
{
	struct sfe_pppoe_mgr_session_entry *entry;
	struct sfe_pppoe_mgr_session_info *info;
	struct hlist_node *temp;
	int bkt;

	hash_for_each_safe(pppoe_session_table, bkt, temp, entry, hash_list) {
		info = &entry->info;
		if ((uint16_t)info->session_id == session_id &&
		    ether_addr_equal(info->server_mac, server_mac)) {

			return true;
		}
	}

	DEBUG_INFO("PPPoE session entry not found: session_id %d server_mac %pM\n", session_id, server_mac);

	return false;
}

/*
 * sfe_pppoe_mgr_exit
 *     PPPoE mgr exit function
 */
void sfe_pppoe_mgr_exit(void)
{
	struct sfe_pppoe_mgr_session_entry *entry;
	struct hlist_node *temp;
	int bkt;

	/*
	 * Unregister the module from the PPP channel events.
	 */
	ppp_channel_connection_unregister_notify(&sfe_pppoe_mgr_channel_notifier_nb);

	hash_for_each_safe(pppoe_session_table, bkt, temp, entry, hash_list) {
		sfe_pppoe_mgr_remove_session(entry);
	}
}

/*
 * sfe_pppoe_mgr_init()
 *	PPPoE mgr init function
 */
int sfe_pppoe_mgr_init(void)
{
	/*
	 * Register the module to the PPP channel events.
	 */
	ppp_channel_connection_register_notify(&sfe_pppoe_mgr_channel_notifier_nb);
	return 0;
}
