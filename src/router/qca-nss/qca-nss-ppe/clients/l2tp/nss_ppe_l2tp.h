/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 */

#ifndef __NSS_PPE_L2TP_PRIV_H_
#define __NSS_PPE_L2TP_PRIV_H_
#include <linux/l2tp.h>
#include <linux/socket.h>
#include <linux/debugfs.h>

/*
 * NSS L2TP interface debug macros
 */
#if (NSS_PPE_L2TP_DEBUG_LEVEL < 1)
#define nss_ppe_l2tp_assert(fmt, args...)
#else
#define nss_ppe_l2tp_assert(c) if (!(c)) { BUG_ON(!(c)); }
#endif

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_ppe_l2tp_warning(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_l2tp_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_ppe_l2tp_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */

/*
 * Statically compile messages at different levels
 */
#if (NSS_PPE_L2TP_DEBUG_LEVEL < 2)
#define nss_ppe_l2tp_warning(s, ...)
#else
#define nss_ppe_l2tp_warning(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_L2TP_DEBUG_LEVEL < 3)
#define nss_ppe_l2tp_info(s, ...)
#else
#define nss_ppe_l2tp_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_L2TP_DEBUG_LEVEL < 4)
#define nss_ppe_l2tp_trace(s, ...)
#else
#define nss_ppe_l2tp_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

#define NSS_PPE_L2TP_CHANNEL_MAX	1
#define NSS_PPE_L2TP_VER_2		2
#define NSS_PPE_L2TP_DEFAULT_TTL	64

/*
 * nss_ppe_l2tp
 * 	L2TP global structure
 */
struct nss_ppe_l2tp {
	struct dentry *l2tp_dentry;
	uint8_t outer_ttl;
};

static bool nss_l2tp_stats_dentry_create(struct net_device *dev);
static bool nss_l2tp_stats_dentry_free(struct net_device *dev);
#endif /* __NSS_PPE_L2TP_PRIV_H_ */