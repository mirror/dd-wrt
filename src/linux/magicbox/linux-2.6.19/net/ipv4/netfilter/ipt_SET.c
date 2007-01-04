/* Copyright (C) 2000-2002 Joakim Axelsson <gozem@linux.nu>
 *                         Patrick Schaaf <bof@bof.de>
 *                         Martin Josefsson <gandalf@wlug.westbo.se>
 * Copyright (C) 2003-2004 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* ipt_SET.c - netfilter target to manipulate IP sets */

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/protocol.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ipt_set.h>

static unsigned int
target(struct sk_buff **pskb,
       const struct net_device *in,
       const struct net_device *out,
       unsigned int hooknum,
       const struct xt_target *target,
       const void *targinfo)
{
	const struct ipt_set_info_target *info = targinfo;

	if (info->add_set.index != IP_SET_INVALID_ID)
		ip_set_addip_kernel(info->add_set.index,
				    *pskb,
				    info->add_set.flags);
	if (info->del_set.index != IP_SET_INVALID_ID)
		ip_set_delip_kernel(info->del_set.index,
				    *pskb,
				    info->del_set.flags);

	return IPT_CONTINUE;
}

static int
checkentry(const char *tablename,
	   const void *e,
	   const struct xt_target *target,
	   void *targinfo,
	   unsigned int hook_mask)
{
	struct ipt_set_info_target *info =
		(struct ipt_set_info_target *) targinfo;
	ip_set_id_t index;

	if (info->add_set.index != IP_SET_INVALID_ID) {
		index = ip_set_get_byindex(info->add_set.index);
		if (index == IP_SET_INVALID_ID) {
			ip_set_printk("cannot find add_set index %u as target",
				      info->add_set.index);
			return 0;	/* error */
		}
	}

	if (info->del_set.index != IP_SET_INVALID_ID) {
		index = ip_set_get_byindex(info->del_set.index);
		if (index == IP_SET_INVALID_ID) {
			ip_set_printk("cannot find del_set index %u as target",
				      info->del_set.index);
			return 0;	/* error */
		}
	}
	if (info->add_set.flags[IP_SET_MAX_BINDINGS] != 0
	    || info->del_set.flags[IP_SET_MAX_BINDINGS] != 0) {
		ip_set_printk("That's nasty!");
		return 0;	/* error */
	}

	return 1;
}

static void destroy(const struct xt_target *target, void *targetinfo)
{
	struct ipt_set_info_target *info = targetinfo;

	if (info->add_set.index != IP_SET_INVALID_ID)
		ip_set_put(info->add_set.index);
	if (info->del_set.index != IP_SET_INVALID_ID)
		ip_set_put(info->del_set.index);
}

static struct ipt_target SET_target = {
	.name 		= "SET",
	.target 	= target,
	.targetsize	= sizeof(struct ipt_set_info_target),
	.checkentry 	= checkentry,
	.destroy 	= destroy,
	.me 		= THIS_MODULE
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>");
MODULE_DESCRIPTION("iptables IP set target module");

static int __init init(void)
{
	return ipt_register_target(&SET_target);
}

static void __exit fini(void)
{
	ipt_unregister_target(&SET_target);
}

module_init(init);
module_exit(fini);
