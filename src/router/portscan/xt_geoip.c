/* iptables kernel module for the geoip match
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Copyright (c) 2004, 2005, 2006, 2007, 2008
 * Samuel Jean & Nicolas Bouliane
 */
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rcupdate.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/netfilter/x_tables.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include "xt_geoip.h"
#include "compat_xtables.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas Bouliane");
MODULE_AUTHOR("Samuel Jean");
MODULE_DESCRIPTION("xtables module for geoip match");
MODULE_ALIAS("ip6t_geoip");
MODULE_ALIAS("ipt_geoip");

enum geoip_proto {
	GEOIPROTO_IPV6,
	GEOIPROTO_IPV4,
	__GEOIPROTO_MAX,
};

/**
 * @list:	anchor point for geoip_head
 * @subnets:	packed ordered list of ranges (either v6 or v4)
 * @count:	number of ranges
 * @cc:		country code
 */
struct geoip_country_kernel {
	struct list_head list;
	void *subnets;
	atomic_t ref;
	unsigned int count;
	unsigned short cc;
};

static struct list_head geoip_head[__GEOIPROTO_MAX];
static DEFINE_SPINLOCK(geoip_lock);

static const enum geoip_proto nfp2geo[] = {
	[NFPROTO_IPV6] = GEOIPROTO_IPV6,
	[NFPROTO_IPV4] = GEOIPROTO_IPV4,
};
static const size_t geoproto_size[] = {
	[GEOIPROTO_IPV6] = sizeof(struct geoip_subnet6),
	[GEOIPROTO_IPV4] = sizeof(struct geoip_subnet4),
};

static struct geoip_country_kernel *
geoip_add_node(const struct geoip_country_user __user *umem_ptr,
               enum geoip_proto proto)
{
	struct geoip_country_user umem;
	struct geoip_country_kernel *p;
	size_t size;
	void *subnet;
	int ret;

	if (copy_from_user(&umem, umem_ptr, sizeof(umem)) != 0)
		return ERR_PTR(-EFAULT);
	if (umem.count > SIZE_MAX / geoproto_size[proto])
		return ERR_PTR(-E2BIG);
	p = kmalloc(sizeof(struct geoip_country_kernel), GFP_KERNEL);
	if (p == NULL)
		return ERR_PTR(-ENOMEM);

	p->count   = umem.count;
	p->cc      = umem.cc;
	size = p->count * geoproto_size[proto];
	if (size == 0) {
		/*
		 * Believe it or not, vmalloc prints a warning to dmesg for
		 * zero-sized allocations :-/
		 */
		subnet = NULL;
	} else {
		subnet = vmalloc(size);
		if (subnet == NULL) {
			ret = -ENOMEM;
			goto free_p;
		}
	}
	if (copy_from_user(subnet,
	    (const void __user *)(unsigned long)umem.subnets, size) != 0) {
		ret = -EFAULT;
		goto free_s;
	}

	p->subnets = subnet;
	atomic_set(&p->ref, 1);
	INIT_LIST_HEAD(&p->list);

	spin_lock(&geoip_lock);
	list_add_tail_rcu(&p->list, &geoip_head[proto]);
	spin_unlock(&geoip_lock);

	return p;

 free_s:
	vfree(subnet);
 free_p:
	kfree(p);
	return ERR_PTR(ret);
}

static void geoip_try_remove_node(struct geoip_country_kernel *p)
{
	spin_lock(&geoip_lock);
	if (!atomic_dec_and_test(&p->ref)) {
		spin_unlock(&geoip_lock);
		return;
	}
	list_del_rcu(&p->list);
	spin_unlock(&geoip_lock);

	synchronize_rcu();
	vfree(p->subnets);
	kfree(p);
}

static struct geoip_country_kernel *find_node(unsigned short cc,
    enum geoip_proto proto)
{
	struct geoip_country_kernel *p;
	spin_lock(&geoip_lock);

	list_for_each_entry_rcu(p, &geoip_head[proto], list)
		if (p->cc == cc) {
			atomic_inc(&p->ref);
			spin_unlock(&geoip_lock);
			return p;
		}

	spin_unlock(&geoip_lock);
	return NULL;
}

static inline int
ipv6_cmp(const struct in6_addr *p, const struct in6_addr *q)
{
	unsigned int i;

	for (i = 0; i < 4; ++i) {
		if (p->s6_addr32[i] < q->s6_addr32[i])
			return -1;
		else if (p->s6_addr32[i] > q->s6_addr32[i])
			return 1;
	}

	return 0;
}

static bool geoip_bsearch6(const struct geoip_subnet6 *range,
    const struct in6_addr *addr, int lo, int hi)
{
	int mid;

	while (true) {
		if (hi <= lo)
			return false;
		mid = (lo + hi) / 2;
		if (ipv6_cmp(&range[mid].begin, addr) <= 0 &&
		    ipv6_cmp(addr, &range[mid].end) <= 0)
			return true;
		if (ipv6_cmp(&range[mid].begin, addr) > 0)
			hi = mid;
		else if (ipv6_cmp(&range[mid].end, addr) < 0)
			lo = mid + 1;
		else
			break;
	}

	WARN_ON(true);
	return false;
}

static bool
xt_geoip_mt6(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_geoip_match_info *info = par->matchinfo;
	const struct geoip_country_kernel *node;
	const struct ipv6hdr *iph = ipv6_hdr(skb);
	unsigned int i;
	struct in6_addr ip;

	memcpy(&ip, (info->flags & XT_GEOIP_SRC) ? &iph->saddr : &iph->daddr,
	       sizeof(ip));
	for (i = 0; i < 4; ++i)
		ip.s6_addr32[i] = ntohl(ip.s6_addr32[i]);

	rcu_read_lock();
	for (i = 0; i < info->count; i++) {
		if ((node = info->mem[i].kernel) == NULL) {
			printk(KERN_ERR "xt_geoip: %c%c is not loaded into memory\n",
					COUNTRY(info->cc[i]));
			continue;
		}
		if (geoip_bsearch6(node->subnets, &ip, 0, node->count)) {
			rcu_read_unlock();
			return !(info->flags & XT_GEOIP_INV);
		}
	}

	rcu_read_unlock();
	return info->flags & XT_GEOIP_INV;
}

static bool geoip_bsearch4(const struct geoip_subnet4 *range,
    uint32_t addr, int lo, int hi)
{
	int mid;

	while (true) {
		if (hi <= lo)
			return false;
		mid = (lo + hi) / 2;
		if (range[mid].begin <= addr && addr <= range[mid].end)
			return true;
		if (range[mid].begin > addr)
			hi = mid;
		else if (range[mid].end < addr)
			lo = mid + 1;
		else
			break;
	}

	WARN_ON(true);
	return false;
}

static bool
xt_geoip_mt4(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_geoip_match_info *info = par->matchinfo;
	const struct geoip_country_kernel *node;
	const struct iphdr *iph = ip_hdr(skb);
	unsigned int i;
	uint32_t ip;

	ip = ntohl((info->flags & XT_GEOIP_SRC) ? iph->saddr : iph->daddr);
	rcu_read_lock();
	for (i = 0; i < info->count; i++) {
		if ((node = info->mem[i].kernel) == NULL) {
			printk(KERN_ERR "xt_geoip: %c%c is not loaded into memory\n",
					COUNTRY(info->cc[i]));
			continue;
		}
		if (geoip_bsearch4(node->subnets, ip, 0, node->count)) {
			rcu_read_unlock();
			return !(info->flags & XT_GEOIP_INV);
		}
	}

	rcu_read_unlock();
	return info->flags & XT_GEOIP_INV;
}

static int xt_geoip_mt_checkentry(const struct xt_mtchk_param *par)
{
	struct xt_geoip_match_info *info = par->matchinfo;
	struct geoip_country_kernel *node;
	unsigned int i;

	for (i = 0; i < info->count; i++) {
		node = find_node(info->cc[i], nfp2geo[par->family]);
		if (node == NULL) {
			node = geoip_add_node((const void __user *)(unsigned long)info->mem[i].user,
			       nfp2geo[par->family]);
			if (IS_ERR(node)) {
				printk(KERN_ERR
						"xt_geoip: unable to load '%c%c' into memory: %ld\n",
						COUNTRY(info->cc[i]), PTR_ERR(node));
				return PTR_ERR(node);
			}
		}

		/* Overwrite the, now useless, pointer info->mem[i] with
		 * a pointer to the node's kernelspace structure.
		 * This avoids searching for a node in the match() and
		 * destroy() functions.
		 */
		info->mem[i].kernel = node;
	}

	return 0;
}

static void xt_geoip_mt_destroy(const struct xt_mtdtor_param *par)
{
	struct xt_geoip_match_info *info = par->matchinfo;
	struct geoip_country_kernel *node;
	unsigned int i;

	/* This entry has been removed from the table so
	 * decrease the refcount of all countries it is
	 * using.
	 */

	for (i = 0; i < info->count; i++)
		if ((node = info->mem[i].kernel) != NULL) {
			/* Free up some memory if that node isn't used
			 * anymore. */
			geoip_try_remove_node(node);
		}
		else
			/* Something strange happened. There's no memory allocated for this
			 * country.  Please send this bug to the mailing list. */
			printk(KERN_ERR "xt_geoip: no memory allocated for this number\n");
}

static struct xt_match xt_geoip_match[] __read_mostly = {
	{
		.name       = "geoip",
		.revision   = 2,
		.family     = NFPROTO_IPV6,
		.match      = xt_geoip_mt6,
		.checkentry = xt_geoip_mt_checkentry,
		.destroy    = xt_geoip_mt_destroy,
		.matchsize  = sizeof(struct xt_geoip_match_info),
		.me         = THIS_MODULE,
	},
	{
		.name       = "geoip",
		.revision   = 2,
		.family     = NFPROTO_IPV4,
		.match      = xt_geoip_mt4,
		.checkentry = xt_geoip_mt_checkentry,
		.destroy    = xt_geoip_mt_destroy,
		.matchsize  = sizeof(struct xt_geoip_match_info),
		.me         = THIS_MODULE,
	},
};

static int __init xt_geoip_mt_init(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(geoip_head); ++i)
		INIT_LIST_HEAD(&geoip_head[i]);
	return xt_register_matches(xt_geoip_match, ARRAY_SIZE(xt_geoip_match));
}

static void __exit xt_geoip_mt_fini(void)
{
	xt_unregister_matches(xt_geoip_match, ARRAY_SIZE(xt_geoip_match));
}

module_init(xt_geoip_mt_init);
module_exit(xt_geoip_mt_fini);
