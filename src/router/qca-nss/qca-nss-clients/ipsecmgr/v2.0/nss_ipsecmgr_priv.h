/*
 * ********************************************************************************
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **********************************************************************************
 */

#ifndef __NSS_IPSECMGR_PRIV_H
#define __NSS_IPSECMGR_PRIV_H

#include <net/ipv6.h>

#define NSS_IPSECMGR_DEBUG_LVL_ERROR 1		/**< Turn on debug for an error. */
#define NSS_IPSECMGR_DEBUG_LVL_WARN 2		/**< Turn on debug for a warning. */
#define NSS_IPSECMGR_DEBUG_LVL_INFO 3		/**< Turn on debug for information. */
#define NSS_IPSECMGR_DEBUG_LVL_TRACE 4		/**< Turn on debug for trace. */

#define nss_ipsecmgr_info_always(s, ...) pr_info("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)

#define nss_ipsecmgr_error(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_alert("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#define nss_ipsecmgr_warn(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_warn("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_ipsecmgr_info(s, ...) pr_debug("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)
#define nss_ipsecmgr_trace(s, ...) pr_debug("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#define nss_ipsecmgr_info(s, ...) {	\
	if (NSS_IPSECMGR_DEBUG_LEVEL > NSS_IPSECMGR_DEBUG_LVL_INFO) {	\
		pr_notice("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}
#define nss_ipsecmgr_trace(s, ...) {	\
	if (NSS_IPSECMGR_DEBUG_LEVEL > NSS_IPSECMGR_DEBUG_LVL_TRACE) {	\
		pr_info("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
}

#endif /* !CONFIG_DYNAMIC_DEBUG */
#define NSS_IPSECMGR_CHK_POW2(x) (__builtin_constant_p(x) && !(~(x - 1) & (x >> 1)))

#define NSS_IPSECMGR_DEFAULT_TUN_NAME "ipsecdummy"
#define NSS_IPSECMGR_ESP_TRAIL_SZ 2 /* esp trailer size. */
#define NSS_IPSECMGR_ESP_PAD_SZ 14 /* maximum amount of padding. */

#define NSS_IPSECMGR_PRINT_PAGES 2
#define NSS_IPSECMGR_PRINT_BYTES(bytes) ((((bytes) * BITS_PER_BYTE) / 4) + 4)
#define NSS_IPSECMGR_PRINT_DWORD NSS_IPSECMGR_PRINT_BYTES(8)
#define NSS_IPSECMGR_PRINT_WORD NSS_IPSECMGR_PRINT_BYTES(4)
#define NSS_IPSECMGR_PRINT_SHORT NSS_IPSECMGR_PRINT_BYTES(2)
#define NSS_IPSECMGR_PRINT_BYTE NSS_IPSECMGR_PRINT_BYTES(1)
#define NSS_IPSECMGR_PRINT_IPADDR (NSS_IPSECMGR_PRINT_WORD * 4)

/*
 * Statistics dump information
 */
struct nss_ipsecmgr_print {
	char *str;		/* Name of variable. */
	ssize_t var_size;	/* Size of variable in bytes. */
};

/*
 * IPsec manager drv instance
 */
struct nss_ipsecmgr_drv {
	struct dentry *dentry;			/* Debugfs entry per ipsecmgr module. */
	struct net_device *dev;			/* IPsec dummy net device. */

	rwlock_t lock;					/* lock for all DB operations. */
	struct list_head sa_db[NSS_IPSECMGR_SA_MAX];	/* SA database. */
	struct list_head flow_db[NSS_IPSECMGR_FLOW_MAX];/* Flow database. */
	struct list_head tun_db;			/* Tunnel database. */

	struct nss_ctx_instance *nss_ctx;	/* NSS context. */
	struct delayed_work cfg_work;		/* Configure node work. */
	uint32_t ifnum;				/* NSS IPsec base interface. */
	uint16_t max_mtu;			/* Maximum MTU supported. */
	bool ipsec_inline;			/* IPsec inline mode. */
};

/*
 * nss_ipsecmgr_hton_v6addr()
 *	Host to network order and swap
 */
static inline void nss_ipsecmgr_hton_v6addr(uint32_t *dest, uint32_t *src)
{
	dest[3] = htonl(src[3]);
	dest[2] = htonl(src[2]);
	dest[1] = htonl(src[1]);
	dest[0] = htonl(src[0]);
}

/*
 * nss_ipsecmgr_db_init_entries()
 *	Initialize the entries databases
 */
static inline void nss_ipsecmgr_db_init_entries(struct list_head *db, uint32_t max)
{
	struct list_head *head = db;
	int i;

	/*
	 * initialize the database
	 */
	for (i = 0; i < max; i++, head++)
		INIT_LIST_HEAD(head);
}

/*
 * nss_ipsecmgr_db_init()
 *	Initialize the single entry database
 */
static inline void nss_ipsecmgr_db_init(struct list_head *db)
{
	INIT_LIST_HEAD(db);
}

extern struct nss_ipsecmgr_drv *ipsecmgr_drv;
#endif
