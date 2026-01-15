/*
 *	API compat layer
 *	written by Jan Engelhardt, 2008 - 2010
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License, either
 *	version 2 of the License, or any later version.
 */
#include <linux/ip.h>
#include <linux/kconfig.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_arp.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/route.h>
#include <linux/export.h>
//#if IS_ENABLED(CONFIG_IP6_NF_IPTABLES)
//#	define WITH_IPV6 1
//#endif

#if 0
extern void *HX_memmem(const void *space, size_t spacesize, const void *point, size_t pointsize);
void *HX_memmem(const void *space, size_t spacesize,
    const void *point, size_t pointsize)
{
	size_t i;

	if (pointsize > spacesize)
		return NULL;
	for (i = 0; i <= spacesize - pointsize; ++i)
		if (memcmp(space + i, point, pointsize) == 0)
			return (void *)space + i;
	return NULL;
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0) && defined(WITH_IPV6)
static int xtnu_ipv6_skip_exthdr(const struct sk_buff *skb, int start,
    uint8_t *nexthdrp, __be16 *fragoffp)
{
	return ipv6_skip_exthdr(skb, start, nexthdrp);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0) && defined(WITH_IPV6)
static int xtnu_ipv6_find_hdr(const struct sk_buff *skb, unsigned int *offset,
    int target, unsigned short *fragoff, int *fragflg)
{
	return ipv6_find_hdr(skb, offset, target, fragoff);
}
#endif

MODULE_LICENSE("GPL");
