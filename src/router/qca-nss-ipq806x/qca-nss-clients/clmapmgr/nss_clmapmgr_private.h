/*
 **************************************************************************
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_clmapmgr_private.h
 *
 * Private header file for NSS clmapmgr
 */

#ifndef _NSS_CLMAPMGR_PRIVATE_H_
#define _NSS_CLMAPMGR_PRIVATE_H_

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include "nss_clmapmgr.h"

/*
 * clmap debug macros
 */
#if (NSS_CLMAPMGR_DEBUG_LEVEL < 1)
#define nss_clmapmgr_assert(fmt, args...)
#else
#define nss_clmapmgr_assert(c)  BUG_ON(!(c));
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_clmapmgr_warning(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_clmapmgr_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_clmapmgr_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_CLMAPMGR_DEBUG_LEVEL < 2)
#define nss_clmapmgr_warning(s, ...)
#else
#define nss_clmapmgr_warning(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_CLMAPMGR_DEBUG_LEVEL < 3)
#define nss_clmapmgr_info(s, ...)
#else
#define nss_clmapmgr_info(s, ...)   pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_CLMAPMGR_DEBUG_LEVEL < 4)
#define nss_clmapmgr_trace(s, ...)
#else
#define nss_clmapmgr_trace(s, ...)  pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif
#define nss_clmapmgr_error(s, ...) pr_err("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

/*
 * nss_clmapmgr_priv_t
 * 	Private structure for NSS clmapmgr.
 */
struct nss_clmapmgr_priv_t {
	bool clmap_enabled;			/* Clmap status */
	int nss_if_number_us;			/* Clmapmgr upstream NSS interface number */
	int nss_if_number_ds;			/* Clmapmgr downstream NSS interface number */
	struct rtnl_link_stats64 stats;		/* Netdev stats */
};

#endif
