/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/*
 * nss_wifi_mesh_priv.h
 *	Mesh manager header
 */
#ifndef __NSS_WIFI_MESH_PRIV_H_
#define __NSS_WIFI_MESH_PRIV_H_

#define NSS_WIFI_MESH_MAX 16

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_wifi_meshmgr_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_wifi_meshmgr_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define nss_wifi_meshmgr_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_WIFI_MESHMGR_DEBUG_LEVEL < 2)
#define nss_wifi_meshmgr_warn(s, ...)
#else
#define nss_wifi_meshmgr_warn(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_WIFI_MESHMGR_DEBUG_LEVEL < 3)
#define nss_wifi_meshmgr_info(s, ...)
#else
#define nss_wifi_meshmgr_info(s, ...)   pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_WIFI_MESHMGR_DEBUG_LEVEL < 4)
#define nss_wifi_meshmgr_trace(s, ...)
#else
#define nss_wifi_meshmgr_trace(s, ...)  pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * nss_wifi_meshmgr_mesh_ctx
 *	Mesh context per interface.
 */
struct nss_wifi_meshmgr_mesh_ctx {
	struct net_device *dev;			/* Netdevice pointer */
	atomic_t ref;				/* Atomic reference count */
	int32_t encap_ifnum;			/* Encap interface number */
	int32_t decap_ifnum;			/* Decap interface number */
	struct semaphore sem;			/* Semaphore for message synchronization */
	struct completion complete;		/* Per context complete */
	nss_tx_status_t response;		/* Response type */
};

/*
 * nss_wifi_meshmgr_ctx
 *	Global mesh context.
 */
struct nss_wifi_meshmgr_ctx {
	struct nss_ctx_instance *nss_ctx;	/* Nss context for mesh */
	uint32_t mesh_count;			/* Active mesh count */
	spinlock_t ref_lock;			/* Spinlock */
	struct nss_wifi_meshmgr_mesh_ctx *mesh_ctx[NSS_WIFI_MESH_MAX];
						/* Mesh handle table */
};
#endif /* __NSS_WIFI_MESH_PRIV_H_ */
