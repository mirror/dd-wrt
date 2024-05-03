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
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 **************************************************************************
 */

/*
 * nss_tlsmgr_priv.h
 */

#ifndef __NSS_TLSMGR_PRIV_H_
#define __NSS_TLSMGR_PRIV_H_

#define NSS_TLSMGR_DEBUG_LEVEL_ERROR 1
#define NSS_TLSMGR_DEBUG_LEVEL_WARN 2
#define NSS_TLSMGR_DEBUG_LEVEL_INFO 3
#define NSS_TLSMGR_DEBUG_LEVEL_TRACE 4

#define nss_tlsmgr_info_always(s, ...) pr_info("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);

#define nss_tlsmgr_error(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_alert("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_tlsmgr_warn(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_warn("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_tlsmgr_info(s, ...) pr_debug("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);
#define nss_tlsmgr_trace(s, ...) pr_debug("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);
#else

#define nss_tlsmgr_info(s, ...) {	\
	if (NSS_TLSMGR_DEBUG_LEVEL > NSS_TLSMGR_DEBUG_LEVEL_WARN)	\
		pr_notice("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
}

#define nss_tlsmgr_trace(s, ...) {	\
	if (NSS_TLSMGR_DEBUG_LEVEL > NSS_TLSMGR_DEBUG_LEVEL_INFO)	\
		pr_info("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
}

#endif /* CONFIG_DYNAMIC_DEBUG */

#define NSS_TLSMGR_PRINT_PAGES 2
#define NSS_TLSMGR_PRINT_BYTES(bytes) ((((bytes) * BITS_PER_BYTE) / 4) + 4)
#define NSS_TLSMGR_PRINT_DWORD NSS_TLSMGR_PRINT_BYTES(8)
#define NSS_TLSMGR_PRINT_WORD NSS_TLSMGR_PRINT_BYTES(4)
#define NSS_TLSMGR_PRINT_SHORT NSS_TLSMGR_PRINT_BYTES(2)
#define NSS_TLSMGR_PRINT_BYTE NSS_TLSMGR_PRINT_BYTES(1)
#define NSS_TLSMGR_PRINT_IPADDR (NSS_TLSMGR_PRINT_WORD * 4)
#define NSS_TLSMGR_PRINT_EXTRA 64      /* Bytes */

/*
 * Statistics dump information
 */
struct nss_tlsmgr_print {
	char *str;              /* Name of variable. */
	ssize_t var_size;       /* Size of variable in bytes. */
};

/*
 * TLS manager data
 */
struct nss_tlsmgr {
	atomic_t is_configured;			/* Firmware is configured. */
	rwlock_t lock;				/* Tunnel lock. */

	ssize_t print_len;			/* Print buffer length. */
	ssize_t stats_len;			/* Statistics length. */

	struct dentry *dentry;			/* Debugfs root directory. */
	struct nss_tls_node_stats stats;	/* Node statistics. */
	struct nss_ctx_instance *nss_ctx;	/* NSS data/message handle. */
};

extern struct nss_tlsmgr *tlsmgr_drv;

#endif /* !__NSS_TLSMGR_PRIV_H_ */
