/*
 * Copyright (c) 2014-2015, 2019, 2021 The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include "mc_osdep.h"
#include "mc_api.h"
#include "mc_private.h"
#include "mc_snooping.h"
#include "mc_ecm.h"

static mc_bridge_ipv4_update_callback_t __rcu mc_ipv4_event_cb = NULL;

/* mc_bridge_if_source_filter
 *	validate the source in the source filter list
 */
static int mc_bridge_if_source_filter(struct mc_mdb_entry *mdb, uint32_t ifindex, struct mc_ip *mc_source)
{
	struct mc_port_group *pg;
	struct hlist_node *pgh;


	/*no bridge port joining*/
	if (hlist_empty(&mdb->pslist))
		return 1;

	os_hlist_for_each_entry_rcu(pg, pgh, &mdb->pslist, pslist) {
		struct mc_fdb_group *fg;
		struct hlist_node *fgh;
		int i;

		if (ifindex != ((struct net_device *)pg->port)->ifindex)
			continue;

		/*no client joining*/
		if (hlist_empty(&pg->fslist))
			return 1;

		/*anyone who would like to receive stream from the source*/
		os_hlist_for_each_entry_rcu(fg, fgh, &pg->fslist, fslist) {
			if (!fg->filter_mode || (fg->filter_mode == MC_DEF_FILTER_EXCLUDE && !fg->a.nsrcs))
				return 0;

			if (fg->filter_mode == MC_DEF_FILTER_INCLUDE && !fg->a.nsrcs)
				continue;

			if (mdb->group.pro == htons(ETH_P_IP)) {
				u_int32_t ip4 = mc_source->u.ip4;
				u_int32_t *srcs = (u_int32_t *) fg->a.srcs;

				for (i = 0; i < fg->a.nsrcs; i++) {
					if (srcs[i] == ip4)
						break;
				}
			}
#ifdef MC_SUPPORT_MLD
			else {
				struct in6_addr *ip6 = &mc_source->u.ip6;
				struct in6_addr *srcs = (struct in6_addr *)fg->a.srcs;

				for (i = 0; i < fg->a.nsrcs; i++) {
					if (!ipv6_addr_cmp(&srcs[i], ip6))
						break;
				}
			}
#endif

			if ((fg->filter_mode == MC_DEF_FILTER_INCLUDE && i != fg->a.nsrcs) ||
				(fg->filter_mode == MC_DEF_FILTER_EXCLUDE && i == fg->a.nsrcs))
				return 0;

		}
	}

	return 1;

}

/* __mc_bridge_get_ifs
 *	get the port list for a given group and source
 */
static int __mc_bridge_get_ifs(struct net_device *brdev, struct mc_ip *mc_group,
			       struct mc_ip *mc_source, uint32_t max_dst, uint32_t dst_dev[])
{
	struct mc_struct *mc;
	struct hlist_head *head;
	struct mc_mdb_entry *mdb;
	int i;
	int ifnum;

	mc = MC_DEV(brdev);
	if (!mc)
		return -1;

	if (!mc->started) {
		return -1;
	}

	head = &mc->hash[mc_group_hash(mc->salt, mc_group->u.ip4)];
	mdb = mc_mdb_find(head, mc_group);
	if (!mdb || !atomic_read(&mdb->users)) {
		return 0;
	}

	read_lock(&mdb->rwlock);
	for (i = 0, ifnum = 0; i < mdb->flood_ifcnt; i++) {
		if (mc_bridge_if_source_filter(mdb, mdb->flood_ifindex[i], mc_source)) {

			if (mc_group->pro == htons(ETH_P_IP))
				MC_PRINT("Group %pI4 Source %pI4 ignored for port %d\n",
					&mc_group->u.ip4, &mc_source->u.ip4, mdb->flood_ifindex[i]);
#ifdef MC_SUPPORT_MLD
			else
				MC_PRINT("Group %pI6 Source %pI6  ignored for port %d\n",
					&mc_group->u.ip6, &mc_source->u.ip6, mdb->flood_ifindex[i]);
#endif
			continue;
		}

		if (ifnum + 1 > max_dst) {
			MC_PRINT("Multicast interfaces overflow %d/%d\n", mdb->flood_ifcnt, max_dst);
			ifnum = -1;
			break;
		}

		dst_dev[ifnum] = mdb->flood_ifindex[i];
		ifnum++;

	}
	read_unlock(&mdb->rwlock);

	return ifnum;

}

/* mc_bridge_ipv4_get_if
 *	get the port list for a group
 */
int mc_bridge_ipv4_get_if(struct net_device *brdev, __be32 origin, __be32 group,
			 uint32_t max_dst, uint32_t dst_dev[])
{
	struct mc_ip mc_group;
	struct mc_ip mc_source;
	int ret;

	memset(&mc_group, 0, sizeof(struct mc_ip));
	mc_group.u.ip4 = group;
	mc_group.pro = htons(ETH_P_IP);

	memset(&mc_source, 0, sizeof(struct mc_ip));
	mc_source.u.ip4 = origin;
	mc_source.pro = htons(ETH_P_IP);

	rcu_read_lock();
	ret = __mc_bridge_get_ifs(brdev, &mc_group, &mc_source, max_dst, dst_dev);
	rcu_read_unlock();

	return ret;
}
EXPORT_SYMBOL(mc_bridge_ipv4_get_if);

/* mc_bridge_ipv4_update_callback_register
 *	register the callback for the group change event
 */
int mc_bridge_ipv4_update_callback_register(mc_bridge_ipv4_update_callback_t snoop_event_cb)
{
	mc_bridge_ipv4_update_callback_t event_cb;

	event_cb = rcu_dereference(mc_ipv4_event_cb);
	if (event_cb && event_cb != snoop_event_cb) {
		printk("MC callback function is using by another module\n");
		return -1;
	}

	rcu_assign_pointer(mc_ipv4_event_cb, snoop_event_cb);
	return 0;
}
EXPORT_SYMBOL(mc_bridge_ipv4_update_callback_register);

/* mc_bridge_ipv4_update_callback_deregister
 *	unregister the callback
 */
int mc_bridge_ipv4_update_callback_deregister(void)
{
	rcu_assign_pointer(mc_ipv4_event_cb, NULL);
	return 0;

}
EXPORT_SYMBOL(mc_bridge_ipv4_update_callback_deregister);

/* mc_bridge_ipv4_update_callback_get
 *	get the event cb
 */
mc_bridge_ipv4_update_callback_t mc_bridge_ipv4_update_callback_get(void)
{
	return rcu_dereference(mc_ipv4_event_cb);
}

#ifdef MC_SUPPORT_MLD

static mc_bridge_ipv6_update_callback_t __rcu mc_ipv6_event_cb = NULL;

/* mc_bridge_ipv6_get_if
 *	get the port list for a given ipv6 group
 */
int mc_bridge_ipv6_get_if(struct net_device *brdev, struct in6_addr *origin, struct in6_addr *group,
						  uint32_t max_dst, uint32_t dst_dev[])
{
	struct mc_ip mc_group;
	struct mc_ip mc_source;
	int ret;

	memset(&mc_group, 0, sizeof(struct mc_ip));
	mc_ipv6_addr_copy(&mc_group.u.ip6, group);
	mc_group.pro = htons(ETH_P_IPV6);

	memset(&mc_source, 0, sizeof(struct mc_ip));
	mc_ipv6_addr_copy(&mc_source.u.ip6, origin);
	mc_source.pro = htons(ETH_P_IPV6);

	rcu_read_lock();
	ret = __mc_bridge_get_ifs(brdev, &mc_group, &mc_source, max_dst, dst_dev);
	rcu_read_unlock();

	return ret;
}
EXPORT_SYMBOL(mc_bridge_ipv6_get_if);

/* mc_bridge_ipv6_update_callback_register
 *	register for the ipv6 group change event
 */
int mc_bridge_ipv6_update_callback_register(mc_bridge_ipv6_update_callback_t snoop_event_cb)
{
	mc_bridge_ipv6_update_callback_t event_cb;

	event_cb = rcu_dereference(mc_ipv6_event_cb);
	if (event_cb && event_cb != snoop_event_cb) {
		printk("MC callback function is using by another module\n");
		return -1;
	}

	rcu_assign_pointer(mc_ipv6_event_cb, snoop_event_cb);
	return 0;
}
EXPORT_SYMBOL(mc_bridge_ipv6_update_callback_register);

/* mc_bridge_ipv6_update_callback_deregister
 *	unregister the ipv6 group event cb
 */
int mc_bridge_ipv6_update_callback_deregister(void)
{
	rcu_assign_pointer(mc_ipv6_event_cb, NULL);
	return 0;
}
EXPORT_SYMBOL(mc_bridge_ipv6_update_callback_deregister);

/* mc_bridge_ipv6_update_callback_get
 *	get ipv6 event cb
 */
mc_bridge_ipv6_update_callback_t mc_bridge_ipv6_update_callback_get(void)
{
	return rcu_dereference(mc_ipv6_event_cb);
}

#endif

#ifdef _MC_ECM_TEST
void _test_mc_ipv4_callback(struct net_device *brdev, uint32_t group)
{

}

void _test_mc_ipv6_callback(struct net_device *brdev, struct in6_addr *group)
{

}

void _test_mc_get_ifs(void)
{
	struct net_device *brdev;
	__be32 origin;
	__be32 group;
	struct in6_addr origin6;
	struct in6_addr group6;
	uint32_t max_dst = 16;
	uint32_t dst_dev[16];

	int ifnum;
	int i;

	brdev = dev_get_by_name(&init_net, "br-lan");
	if (!brdev) {
		printk("Can't get bridge\n");
		return;
	}

	/*239.1.2.3*/
	origin = 0;
	group = htonl(0xef010203);

	ifnum = mc_bridge_ipv4_get_if(brdev, origin, group, max_dst, dst_dev);
	if (ifnum < 0)
		printk("Failed to get IPv4 mc interfaces\n");
	else {
		printk("Got IPv4 %d mc interfaces\n", ifnum);
		for (i = 0; i < ifnum; i++)
			printk("%d ", dst_dev[i]);
		printk("\n");
	}

	/*ff15::0001*/
	ipv6_addr_set(&origin6, 0, 0, 0, 0);
	ipv6_addr_set(&group6, htonl(0xff150000), 0, 0, htonl(1));

	ifnum = mc_bridge_ipv6_get_if(brdev, &origin6, &group6, max_dst, dst_dev);
	if (ifnum < 0)
		printk("Failed to get IPv4 mc interfaces\n");
	else {
		printk("Got IPv6 %d mc interfaces\n", ifnum);
		for (i = 0; i < ifnum; i++)
			printk("%d ", dst_dev[i]);
		printk("\n");
	}

	dev_put(brdev);
}

int _test_mc_ecm_init(void)
{
	printk("MC callback init\n");

	if (mc_bridge_ipv4_update_callback_register(_test_mc_ipv4_callback) < 0)
		printk("Failed to register IPv4 callbak\n");

	if (mc_bridge_ipv6_update_callback_register(_test_mc_ipv6_callback) < 0)
		printk("Failed to register IPv6 callbak\n");

	return 0;
}

int _test_mc_ecm_deinit(void)
{
	printk("MC callback deinit\n");

	if (mc_bridge_ipv4_update_callback_deregister() < 0)
		printk("Failed to register IPv6 callbak\n");

	if (mc_bridge_ipv6_update_callback_deregister() < 0)
		printk("Failed to register IPv6 callbak\n");

	return 0;
}

#endif
