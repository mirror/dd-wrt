/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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
 * nss_ovpnmgr_priv.h
 */
#ifndef __NSS_OVPNMGR_PRIV__H
#define __NSS_OVPNMGR_PRIV__H

#define NSS_OVPNMGR_DEBUG_LVL_ERROR 1		/* Turn on debug for an error. */
#define NSS_OVPNMGR_DEBUG_LVL_WARN 2		/* Turn on debug for a warning. */
#define NSS_OVPNMGR_DEBUG_LVL_INFO 3		/* Turn on debug for information. */
#define NSS_OVPNMGR_DEBUG_LVL_TRACE 4		/* Turn on debug for trace. */

/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ovpnmgr_error(s, ...) do { \
	if (net_ratelimit()) {  \
		pr_alert("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);  \
	} \
} while (0)

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ovpnmgr_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ovpnmgr_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ovpnmgr_warn(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#define nss_ovpnmgr_info(s, ...) {     \
	if (NSS_OVPNMGR_DEBUG_LEVEL > NSS_OVPNMGR_DEBUG_LVL_INFO) {   \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__); \
	} \
}
#define nss_ovpnmgr_trace(s, ...) {    \
	if (NSS_OVPNMGR_DEBUG_LEVEL > NSS_OVPNMGR_DEBUG_LVL_TRACE) {  \
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);   \
	} \
}
#define nss_ovpnmgr_warn(s, ...) {    \
	if (NSS_OVPNMGR_DEBUG_LEVEL > NSS_OVPNMGR_DEBUG_LVL_WARN) {  \
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__);   \
	} \
}
#endif /* !CONFIG_DYNAMIC_DEBUG */

/*
 * OVPN manager context structure
 */
extern struct nss_ovpnmgr_context ovpnmgr_ctx;

/*
 * nss_ovpnmgr_context
 *	OVPN manager context structure
 */
struct nss_ovpnmgr_context {
	struct dentry *dentry;		/* Debugfs entry for OVPN Tunnels. */
	struct list_head app_list;	/* List of OVPN application instances */
	rwlock_t lock;			/* Lock to protect Application List */
};

#endif /* __NSS_OVPNMGR_PRIV__H */
