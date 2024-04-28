/*
 **************************************************************************
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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
 * nss_connnmgr_pppoe.h
 *	NSS PPPOE client definitions
 */

#ifndef _NSS_CONNMGR_PPPOE_H_
#define _NSS_CONNMGR_PPPOE_H_
/*
 * Debug macros
 */
#if (NSS_PPPOE_DEBUG_LEVEL < 1)
#define nss_connmgr_pppoe_assert(fmt, args...)
#else
#define nss_connmgr_pppoe_assert(c) BUG_ON(!(c))
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_connmgr_pppoe_warn(s, ...) pr_debug("%s[%d]:" s, __func__, \
					  __LINE__, ##__VA_ARGS__)
#define nss_connmgr_pppoe_info(s, ...) pr_debug("%s[%d]:" s, __func__, \
					  __LINE__, ##__VA_ARGS__)
#define nss_connmgr_pppoe_trace(s, ...) pr_debug("%s[%d]:" s, __func__, \
					   __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_PPPOE_DEBUG_LEVEL < 2)
#define nss_connmgr_pppoe_warn(s, ...)
#else
#define nss_connmgr_pppoe_warn(s, ...) pr_warn("%s[%d]:" s, __func__, \
					 __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPPOE_DEBUG_LEVEL < 3)
#define nss_connmgr_pppoe_info(s, ...)
#else
#define nss_connmgr_pppoe_info(s, ...) pr_notice("%s[%d]:" s, __func__, \
					   __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPPOE_DEBUG_LEVEL < 4)
#define nss_connmgr_pppoe_trace(s, ...)
#else
#define nss_connmgr_pppoe_trace(s, ...)  pr_info("%s[%d]:" s, __func__, \
					   __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * Structure for PPPOE client driver session info
 */
struct nss_connmgr_pppoe_session_info {
	uint32_t session_id;
	uint8_t server_mac[ETH_ALEN];
	uint8_t local_mac[ETH_ALEN];
};

/*
 * Structure for PPPOE session entry into HASH table
 */
struct nss_connmgr_pppoe_session_entry {
	struct nss_connmgr_pppoe_session_info info;
	struct net_device *dev;
	struct hlist_node hash_list;
};
#endif
