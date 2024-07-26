/*
 **************************************************************************
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
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
 * nss_ppe_pppoe_mgr.h
 *	NSS PPPoE client definitions
 */

#ifndef _NSS_PPE_PPPOE_MGR_H_
#define _NSS_PPE_PPPOE_MGR_H_

/*
 * Debug macros
 */
#if (NSS_PPE_PPPOE_DEBUG_LEVEL < 1)
#define nss_ppe_pppoe_mgr_assert(fmt, args...)
#else
#define nss_ppe_pppoe_mgr_assert(c) BUG_ON(!(c))
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ppe_pppoe_mgr_warn(s, ...) pr_debug("%s[%d]:" s, __func__, \
					  __LINE__, ##__VA_ARGS__)
#define nss_ppe_pppoe_mgr_info(s, ...) pr_debug("%s[%d]:" s, __func__, \
					  __LINE__, ##__VA_ARGS__)
#define nss_ppe_pppoe_mgr_trace(s, ...) pr_debug("%s[%d]:" s, __func__, \
					   __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (NSS_PPE_PPPOE_DEBUG_LEVEL < 2)
#define nss_ppe_pppoe_mgr_warn(s, ...)
#else
#define nss_ppe_pppoe_mgr_warn(s, ...) pr_warn("%s[%d]:" s, __func__, \
					 __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_PPPOE_DEBUG_LEVEL < 3)
#define nss_ppe_pppoe_mgr_info(s, ...)
#else
#define nss_ppe_pppoe_mgr_info(s, ...) pr_notice("%s[%d]:" s, __func__, \
					   __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_PPE_PPPOE_DEBUG_LEVEL < 4)
#define nss_ppe_pppoe_mgr_trace(s, ...)
#else
#define nss_ppe_pppoe_mgr_trace(s, ...)  pr_info("%s[%d]:" s, __func__, \
					   __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * struct nss_ppe_pppoe_mgr_session_info
 *	Structure for PPPoE client driver session info
 */
struct nss_ppe_pppoe_mgr_session_info {
	uint32_t session_id;		/* PPPoE Session ID */
	uint8_t server_mac[ETH_ALEN];
	uint8_t local_mac[ETH_ALEN];
};

/*
 * struct nss_ppe_pppoe_mgr_session_entry
 *	Structure for PPPoE session entry into HASH table
 */
struct nss_ppe_pppoe_mgr_session_entry {
	struct nss_ppe_pppoe_mgr_session_info info;
					/* Session information */
	struct net_device *dev;		/* Net device */
	struct hlist_node hash_list;	/* Hash list for sessions */
	struct ppe_drv_iface *iface;	/* PPE PPPoE iface */
	uint16_t mtu;			/* MTU */
};
#endif
