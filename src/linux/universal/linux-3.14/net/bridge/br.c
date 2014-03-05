/*
 *	Generic parts
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/llc.h>
#include <net/llc.h>
#include <net/stp.h>

#include "br_private.h"

static void __net_exit br_net_exit(struct net *net)
{
	struct net_device *dev;
	LIST_HEAD(list);

	rtnl_lock();
	for_each_netdev(net, dev)
		if (dev->priv_flags & IFF_EBRIDGE)
			br_dev_delete(dev, &list);

	unregister_netdevice_many(&list);
	rtnl_unlock();

}

static struct pernet_operations br_net_ops = {
	.exit	= br_net_exit,
};

static const struct stp_proto br_stp_proto = {
	.rcv	= br_stp_rcv,
};

static void fake_update_pmtu(struct dst_entry *dst, struct sock *sk,
			     struct sk_buff *skb, u32 mtu)
{
}

static void fake_redirect(struct dst_entry *dst, struct sock *sk,
			  struct sk_buff *skb)
{
}

static u32 *fake_cow_metrics(struct dst_entry *dst, unsigned long old)
{
	return NULL;
}

static struct neighbour *fake_neigh_lookup(const struct dst_entry *dst,
					   struct sk_buff *skb,
					   const void *daddr)
{
	return NULL;
}

static unsigned int fake_mtu(const struct dst_entry *dst)
{
	return dst->dev->mtu;
}

static struct dst_ops fake_dst_ops = {
	.family =		AF_INET,
	.protocol =		cpu_to_be16(ETH_P_IP),
	.update_pmtu =		fake_update_pmtu,
	.redirect =		fake_redirect,
	.cow_metrics =		fake_cow_metrics,
	.neigh_lookup =		fake_neigh_lookup,
	.mtu =			fake_mtu,
};

/*
 * Initialize bogus route table used to keep netfilter happy.
 * Currently, we fill in the PMTU entry because netfilter
 * refragmentation needs it, and the rt_flags entry because
 * ipt_REJECT needs it.  Future netfilter modules might
 * require us to fill additional fields.
 */

static const u32 br_dst_default_metrics[RTAX_MAX] = {
	[RTAX_MTU - 1] = 1500,
};

void br_netfilter_rtable_init(struct net_bridge *br)
{
	struct rtable *rt = &br->fake_rtable;

	atomic_set(&rt->dst.__refcnt, 1);
	rt->dst.dev = br->dev;
	rt->dst.path = &rt->dst;
	dst_init_metrics(&rt->dst, br_dst_default_metrics, true);
	rt->dst.flags	= DST_NOXFRM | DST_NOPEER | DST_FAKE_RTABLE;
	rt->dst.ops = &fake_dst_ops;
}


static int __init br_init(void)
{
	int err;

	err = stp_proto_register(&br_stp_proto);
	if (err < 0) {
		pr_err("bridge: can't register sap for STP\n");
		return err;
	}

	err = br_fdb_init();
	if (err)
		goto err_out;

	err = register_pernet_subsys(&br_net_ops);
	if (err)
		goto err_out1;

	err = br_netfilter_init();
	if (err)
		goto err_out2;

	err = register_netdevice_notifier(&br_device_notifier);
	if (err)
		goto err_out3;

	err = br_netlink_init();
	if (err)
		goto err_out4;

	err = dst_entries_init(&fake_dst_ops);
	if (err)
		goto err_out4;

	brioctl_set(br_ioctl_deviceless_stub);

#if IS_ENABLED(CONFIG_ATM_LANE)
	br_fdb_test_addr_hook = br_fdb_test_addr;
#endif

	return 0;
err_out4:
	unregister_netdevice_notifier(&br_device_notifier);
err_out3:
//	br_netfilter_fini();
err_out2:
	unregister_pernet_subsys(&br_net_ops);
err_out1:
	br_fdb_fini();
err_out:
	stp_proto_unregister(&br_stp_proto);
	return err;
}

static void __exit br_deinit(void)
{
	stp_proto_unregister(&br_stp_proto);

	dst_entries_destroy(&fake_dst_ops);
	br_netlink_fini();
	unregister_netdevice_notifier(&br_device_notifier);
	brioctl_set(NULL);

	unregister_pernet_subsys(&br_net_ops);

	rcu_barrier(); /* Wait for completion of call_rcu()'s */

	br_netfilter_fini();
#if IS_ENABLED(CONFIG_ATM_LANE)
	br_fdb_test_addr_hook = NULL;
#endif

	br_fdb_fini();


}

module_init(br_init)
module_exit(br_deinit)
MODULE_LICENSE("GPL");
MODULE_VERSION(BR_VERSION);
MODULE_ALIAS_RTNL_LINK("bridge");
