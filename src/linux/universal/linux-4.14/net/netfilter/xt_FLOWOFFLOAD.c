/*
 * Copyright (C) 2018 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <net/ip.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_flow_table.h>

static struct nf_flowtable nf_flowtable;
static HLIST_HEAD(hooks);
static DEFINE_SPINLOCK(hooks_lock);
static struct delayed_work hook_work;

struct xt_flowoffload_hook {
	struct hlist_node list;
	struct nf_hook_ops ops;
	bool registered;
	bool used;
};

static unsigned int
xt_flowoffload_net_hook(void *priv, struct sk_buff *skb,
			  const struct nf_hook_state *state)
{
	switch (skb->protocol) {
	case htons(ETH_P_IP):
		return nf_flow_offload_ip_hook(priv, skb, state);
	case htons(ETH_P_IPV6):
		return nf_flow_offload_ipv6_hook(priv, skb, state);
	}

	return NF_ACCEPT;
}

static int
xt_flowoffload_create_hook(struct net_device *dev)
{
	struct xt_flowoffload_hook *hook;
	struct nf_hook_ops *ops;

	hook = kzalloc(sizeof(*hook), GFP_ATOMIC);
	if (!hook)
		return -ENOMEM;

	ops = &hook->ops;
	ops->pf = NFPROTO_NETDEV;
	ops->hooknum = NF_NETDEV_INGRESS;
	ops->priority = 10;
	ops->priv = &nf_flowtable;
	ops->hook = xt_flowoffload_net_hook;
	ops->dev = dev;

	hlist_add_head(&hook->list, &hooks);
	mod_delayed_work(system_power_efficient_wq, &hook_work, 0);

	return 0;
}

static struct xt_flowoffload_hook *
flow_offload_lookup_hook(struct net_device *dev)
{
	struct xt_flowoffload_hook *hook;

	hlist_for_each_entry(hook, &hooks, list) {
		if (hook->ops.dev == dev)
			return hook;
	}

	return NULL;
}

static void
xt_flowoffload_check_device(struct net_device *dev)
{
	struct xt_flowoffload_hook *hook;

	spin_lock_bh(&hooks_lock);
	hook = flow_offload_lookup_hook(dev);
	if (hook)
		hook->used = true;
	else
		xt_flowoffload_create_hook(dev);
	spin_unlock_bh(&hooks_lock);
}

static void
xt_flowoffload_register_hooks(void)
{
	struct xt_flowoffload_hook *hook;

restart:
	hlist_for_each_entry(hook, &hooks, list) {
		if (hook->registered)
			continue;

		hook->registered = true;
		spin_unlock_bh(&hooks_lock);
		nf_register_net_hook(dev_net(hook->ops.dev), &hook->ops);
		spin_lock_bh(&hooks_lock);
		goto restart;
	}

}

static void
xt_flowoffload_cleanup_hooks(void)
{
	struct xt_flowoffload_hook *hook;

restart:
	hlist_for_each_entry(hook, &hooks, list) {
		if (hook->used)
			continue;

		hlist_del(&hook->list);
		spin_unlock_bh(&hooks_lock);
		nf_unregister_net_hook(dev_net(hook->ops.dev), &hook->ops);
		kfree(hook);
		spin_lock_bh(&hooks_lock);
		goto restart;
	}

}

static void
xt_flowoffload_check_hook(struct flow_offload *flow, void *data)
{
	struct flow_offload_tuple *tuple = &flow->tuplehash[0].tuple;
	struct xt_flowoffload_hook *hook;
	bool *found = data;

	spin_lock_bh(&hooks_lock);
	hlist_for_each_entry(hook, &hooks, list) {
		if (hook->ops.dev->ifindex != tuple->iifidx &&
		    hook->ops.dev->ifindex != tuple->oifidx)
			continue;

		hook->used = true;
		*found = true;
	}
	spin_unlock_bh(&hooks_lock);
}

static void
xt_flowoffload_hook_work(struct work_struct *work)
{
	struct xt_flowoffload_hook *hook;
	bool found = false;
	int err;

	spin_lock_bh(&hooks_lock);
	xt_flowoffload_register_hooks();
	hlist_for_each_entry(hook, &hooks, list)
		hook->used = false;
	spin_unlock_bh(&hooks_lock);

	err = nf_flow_table_iterate(&nf_flowtable, xt_flowoffload_check_hook,
				    &found);
	if (err && err != -EAGAIN)
	    goto out;

	spin_lock_bh(&hooks_lock);
	xt_flowoffload_cleanup_hooks();
	spin_unlock_bh(&hooks_lock);

out:
	if (found)
		queue_delayed_work(system_power_efficient_wq, &hook_work, HZ);
}

static bool
xt_flowoffload_skip(struct sk_buff *skb)
{
	struct ip_options *opt = &(IPCB(skb)->opt);

	if (unlikely(opt->optlen))
		return true;
	if (skb_sec_path(skb))
		return true;

	return false;
}

static int
xt_flowoffload_route(struct sk_buff *skb, const struct nf_conn *ct,
		   const struct xt_action_param *par,
		   struct nf_flow_route *route, enum ip_conntrack_dir dir)
{
	struct dst_entry *this_dst = skb_dst(skb);
	struct dst_entry *other_dst = NULL;
	struct flowi fl;

	memset(&fl, 0, sizeof(fl));
	switch (xt_family(par)) {
	case NFPROTO_IPV4:
		fl.u.ip4.daddr = ct->tuplehash[!dir].tuple.dst.u3.ip;
		break;
	case NFPROTO_IPV6:
		fl.u.ip6.daddr = ct->tuplehash[!dir].tuple.dst.u3.in6;
		break;
	}

	nf_route(xt_net(par), &other_dst, &fl, false, xt_family(par));
	if (!other_dst)
		return -ENOENT;

	route->tuple[dir].dst		= this_dst;
	route->tuple[dir].ifindex	= xt_in(par)->ifindex;
	route->tuple[!dir].dst		= other_dst;
	route->tuple[!dir].ifindex	= xt_out(par)->ifindex;

	return 0;
}

static unsigned int
flowoffload_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	enum ip_conntrack_info ctinfo;
	enum ip_conntrack_dir dir;
	struct nf_flow_route route;
	struct flow_offload *flow;
	struct nf_conn *ct;

	if (xt_flowoffload_skip(skb))
		return XT_CONTINUE;

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL)
		return XT_CONTINUE;

	switch (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
		break;
	default:
		return XT_CONTINUE;
	}

	if (test_bit(IPS_HELPER_BIT, &ct->status))
		return XT_CONTINUE;

	if (ctinfo == IP_CT_NEW ||
	    ctinfo == IP_CT_RELATED)
		return XT_CONTINUE;

	if (!xt_in(par) || !xt_out(par))
		return XT_CONTINUE;

	if (test_and_set_bit(IPS_OFFLOAD_BIT, &ct->status))
		return XT_CONTINUE;

	dir = CTINFO2DIR(ctinfo);

	if (xt_flowoffload_route(skb, ct, par, &route, dir) < 0)
		goto err_flow_route;

	flow = flow_offload_alloc(ct, &route);
	if (!flow)
		goto err_flow_alloc;

	if (flow_offload_add(&nf_flowtable, flow) < 0)
		goto err_flow_add;

	xt_flowoffload_check_device(xt_in(par));
	xt_flowoffload_check_device(xt_out(par));

	return XT_CONTINUE;

err_flow_add:
	flow_offload_free(flow);
err_flow_alloc:
	dst_release(route.tuple[!dir].dst);
err_flow_route:
	clear_bit(IPS_OFFLOAD_BIT, &ct->status);
	return XT_CONTINUE;
}


static int flowoffload_chk(const struct xt_tgchk_param *par)
{
	return 0;
}

static struct xt_target offload_tg_reg __read_mostly = {
	.family		= NFPROTO_UNSPEC,
	.name		= "FLOWOFFLOAD",
	.revision	= 0,
	.checkentry	= flowoffload_chk,
	.target		= flowoffload_tg,
	.me		= THIS_MODULE,
};

static int xt_flowoffload_table_init(struct nf_flowtable *table)
{
	nf_flow_table_init(table);
	return 0;
}

static void xt_flowoffload_table_cleanup(struct nf_flowtable *table)
{
	nf_flow_table_free(table);
}

static int __init xt_flowoffload_tg_init(void)
{
	int ret;

	INIT_DELAYED_WORK(&hook_work, xt_flowoffload_hook_work);

	ret = xt_flowoffload_table_init(&nf_flowtable);
	if (ret)
		return ret;

	ret = xt_register_target(&offload_tg_reg);
	if (ret)
		xt_flowoffload_table_cleanup(&nf_flowtable);

	return ret;
}

static void __exit xt_flowoffload_tg_exit(void)
{
	xt_unregister_target(&offload_tg_reg);
	xt_flowoffload_table_cleanup(&nf_flowtable);
}

MODULE_LICENSE("GPL");
module_init(xt_flowoffload_tg_init);
module_exit(xt_flowoffload_tg_exit);
