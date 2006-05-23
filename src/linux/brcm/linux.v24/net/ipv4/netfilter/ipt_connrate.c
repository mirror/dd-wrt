/* Connection transfer rate match for netfilter.
 *
 * Copyright (c) 2004 Nuutti Kotivuori <naked@iki.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_connrate.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nuutti Kotivuori <naked@iki.fi>");
MODULE_DESCRIPTION("iptables connection transfer rate match module");

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      int *hotdrop)
{
	const struct ipt_connrate_info *sinfo = matchinfo;
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;
	u_int32_t rate;

	if (!(ct = ip_conntrack_get((struct sk_buff *)skb, &ctinfo)))
		return 0; /* no match */

	rate = ip_conntrack_rate_get(&ct->rate);
	if (sinfo->from > sinfo->to) /* inverted range */
		return (rate < sinfo->to || rate > sinfo->from);
	else /* normal range */
		return (rate >= sinfo->from && rate <= sinfo->to);
}

static int
check(const char *tablename,
      const struct ipt_ip *ip,
      void *matchinfo,
      unsigned int matchsize,
      unsigned int hook_mask)
{
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_connrate_info)))
		return 0;

	return 1;
}

static struct ipt_match connrate_match = {
	.name = "connrate",
	.match = &match,
	.checkentry = &check,
	.me = THIS_MODULE
};

static int __init init(void)
{
	return ipt_register_match(&connrate_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&connrate_match);
}

module_init(init);
module_exit(fini);
