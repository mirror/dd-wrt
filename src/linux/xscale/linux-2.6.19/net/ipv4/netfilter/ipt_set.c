/* Copyright (C) 2000-2002 Joakim Axelsson <gozem@linux.nu>
 *                         Patrick Schaaf <bof@bof.de>
 *                         Martin Josefsson <gandalf@wlug.westbo.se>
 * Copyright (C) 2003-2004 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Kernel module to match an IP set. */

#include <linux/module.h>
#include <linux/ip.h>
#include <linux/skbuff.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_set.h>
#include <linux/netfilter_ipv4/ipt_set.h>

static inline int
match_set(const struct ipt_set_info *info,
	  const struct sk_buff *skb,
	  int inv)
{
	if (ip_set_testip_kernel(info->index, skb, info->flags))
		inv = !inv;
	return inv;
}

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const struct xt_match *match,
      const void *matchinfo,
      int offset,
      unsigned int protoff,
      int *hotdrop)
{
	const struct ipt_set_info_match *info = matchinfo;

	return match_set(&info->match_set,
			 skb,
			 info->match_set.flags[0] & IPSET_MATCH_INV);
}

static int
checkentry(const char *tablename,
	   const void *ip,
	   const struct xt_match *match,
	   void *matchinfo,
	   unsigned int hook_mask)
{
	struct ipt_set_info_match *info =
		(struct ipt_set_info_match *) matchinfo;
	ip_set_id_t index;

	index = ip_set_get_byindex(info->match_set.index);

	if (index == IP_SET_INVALID_ID) {
		ip_set_printk("Cannot find set indentified by id %u to match",
			      info->match_set.index);
		return 0;	/* error */
	}
	if (info->match_set.flags[IP_SET_MAX_BINDINGS] != 0) {
		ip_set_printk("That's nasty!");
		return 0;	/* error */
	}

	return 1;
}

static void destroy(const struct xt_match *match, void *matchinfo)
{
	struct ipt_set_info_match *info = matchinfo;

	ip_set_put(info->match_set.index);
}

static struct ipt_match set_match = {
	.name		= "set",
	.match		= &match,
	.matchsize	= sizeof(struct ipt_set_info_match),
	.checkentry	= &checkentry,
	.destroy	= &destroy,
	.me		= THIS_MODULE
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>");
MODULE_DESCRIPTION("iptables IP set match module");

static int __init init(void)
{
	return ipt_register_match(&set_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&set_match);
}

module_init(init);
module_exit(fini);
