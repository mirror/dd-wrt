#pragma once
#include <linux/kconfig.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "compat_skbuff.h"

#define DEBUGP Use__pr_debug__instead

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
#	warning Kernels below 4.16 not supported.
#endif

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
#	define ip_route_me_harder(xnet, xsk, xskb, xaddrtype) ip_route_me_harder((xnet), (xskb), (xaddrtype))
#	define ip6_route_me_harder(xnet, xsk, xskb) ip6_route_me_harder((xnet), (xskb))
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
#	define pde_data(inode) PDE_DATA(inode)
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
#	define get_random_u32_below prandom_u32_max
#endif

extern void *HX_memmem(const void *, size_t, const void *, size_t);
