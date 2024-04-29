/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef __NSS_CONNMGR_TUNIPIP6_PRIV_H_
#define __NSS_CONNMGR_TUNIPIP6_PRIV_H_

#include "nss_connmgr_tunipip6_stats.h"
#include <linux/debugfs.h>

/*
 * tunipip6 context
 */
extern struct nss_tunipip6_context tunipip6_ctx;

/*
 * NSS tunipip6 debug macros
 */
#if (NSS_TUNIPIP6_DEBUG_LEVEL < 1)
#define nss_tunipip6_assert(fmt, args...)
#else
#define nss_tunipip6_assert(c) if (!(c)) { BUG_ON(!(c)); }
#endif

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_tunipip6_warning(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_tunipip6_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_tunipip6_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_TUNIPIP6_DEBUG_LEVEL < 2)
#define nss_tunipip6_warning(s, ...)
#else
#define nss_tunipip6_warning(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_TUNIPIP6_DEBUG_LEVEL < 3)
#define nss_tunipip6_info(s, ...)
#else
#define nss_tunipip6_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_TUNIPIP6_DEBUG_LEVEL < 4)
#define nss_tunipip6_trace(s, ...)
#else
#define nss_tunipip6_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

extern struct nss_tunipip6_context tunipip6_ctx;

/*
 * tunipip6 global context structure.
 */
struct nss_tunipip6_context {
	struct list_head dev_list;		/* List of tunipip6 interface instances */
	struct dentry *tunipip6_dentry_dir;	/* tunipip6 debugfs directory entry */
	spinlock_t lock;			/* Lock to protect list. */
};

/*
 * tunipip6 interface instance structure.
 */
struct nss_tunipip6_instance {
	struct list_head list;			/* List of tunipip6 interface instance */
	struct net_device *dev;			/* tunipip6 netdevice */
	struct dentry *dentry;			/* debugfs entry for this tunnel device */
	struct nss_tunipip6_stats stats;	/* tunipip6 statistics */
	uint32_t inner_ifnum;			/* tunipip6 inner dynamic interface */
	uint32_t outer_ifnum;			/* tunipip6 outer dynamic interface */
};

struct nss_tunipip6_instance *nss_tunipip6_find_instance(struct net_device *dev);

#endif /* __NSS_CONNMGR_TUNIPIP6_PRIV_H_ */
