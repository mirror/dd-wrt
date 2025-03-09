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
#include "compat_skbuff.h"
#if IS_ENABLED(CONFIG_IP6_NF_IPTABLES)
#	define WITH_IPV6 1
#endif

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
EXPORT_SYMBOL_GPL(HX_memmem);

MODULE_LICENSE("GPL");
