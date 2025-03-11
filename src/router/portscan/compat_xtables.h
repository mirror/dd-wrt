#pragma once
#include <linux/kconfig.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "compat_skbuff.h"
#include "compat_xtnu.h"

#define DEBUGP Use__pr_debug__instead

#if IS_ENABLED(CONFIG_NF_CONNTRACK)
#	if !defined(CONFIG_NF_CONNTRACK_MARK)
#		warning You have CONFIG_NF_CONNTRACK enabled, but CONFIG_NF_CONNTRACK_MARK is not (please enable).
#	endif
#	include <net/netfilter/nf_conntrack.h>
#else
#	warning You need CONFIG_NF_CONNTRACK.
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) || \
    LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 9) && LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0) || \
    LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 78) && LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0) || \
    LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 158) && LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0)
#else

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
#	define ip6_local_out(xnet, xsk, xskb) ip6_local_out(xskb)
#	define ip6_route_me_harder(xnet, xsk, xskb) ip6_route_me_harder(xskb)
#	define ip_local_out(xnet, xsk, xskb) ip_local_out(xskb)
#	define ip_route_me_harder(xnet, xsk, xskb, xaddrtype) ip_route_me_harder((xskb), (xaddrtype))
#else
#	define ip_route_me_harder(xnet, xsk, xskb, xaddrtype) ip_route_me_harder((xnet), (xskb), (xaddrtype))
#	define ip6_route_me_harder(xnet, xsk, xskb) ip6_route_me_harder((xnet), (xskb))
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
#	define pde_data(inode) PDE_DATA(inode)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
#	define nf_nat_ipv4_multi_range_compat nf_nat_multi_range_compat
#	define nf_nat_ipv4_range nf_nat_range
#	define NF_NAT_RANGE_MAP_IPS IP_NAT_RANGE_MAP_IPS
#	define ipv6_skip_exthdr xtnu_ipv6_skip_exthdr
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
#	define ipv6_find_hdr xtnu_ipv6_find_hdr
#endif

#ifndef NF_CT_ASSERT
#	define NF_CT_ASSERT(x)	WARN_ON(!(x))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
#	define proc_ops file_operations
#	define proc_open open
#	define proc_read read
#	define proc_write write
#	define proc_lseek llseek
#	define proc_release release
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
#	define prandom_u32() random32()
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
static inline u32 prandom_u32_max(u32 ep_ro)
{
	return (u32)(((u64) prandom_u32() * ep_ro) >> 32);
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
#	define get_random_u32_below prandom_u32_max
#endif

extern void *HX_memmem(const void *, size_t, const void *, size_t);

static inline struct net *par_net(const struct xt_action_param *par)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
	return par->state->net;
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
	return par->net;
#else
	return dev_net((par->in != NULL) ? par->in : par->out);
#endif
#endif
}
