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
 * nss_dtlsmgr_private.h
 */

#ifndef __NSS_DTLSMGR_PRIVATE_H_
#define __NSS_DTLSMGR_PRIVATE_H_

#define NSS_DTLSMGR_DEBUG_LEVEL_ERROR 1
#define NSS_DTLSMGR_DEBUG_LEVEL_WARN 2
#define NSS_DTLSMGR_DEBUG_LEVEL_INFO 3
#define NSS_DTLSMGR_DEBUG_LEVEL_TRACE 4

#define nss_dtlsmgr_error(s, ...) do {	\
	if (net_ratelimit()) {	\
		pr_alert("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__);	\
	}	\
} while (0)

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_dtlsmgr_warn(s, ...) pr_debug("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)
#define nss_dtlsmgr_info(s, ...) pr_debug("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)
#define nss_dtlsmgr_trace(s, ...) pr_debug("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#define nss_dtlsmgr_warn(s, ...) {	\
	if (NSS_DTLSMGR_DEBUG_LEVEL > NSS_DTLSMGR_DEBUG_LEVEL_ERROR)	\
		pr_warn("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)	\
}

#define nss_dtlsmgr_info(s, ...) {	\
	if (NSS_DTLSMGR_DEBUG_LEVEL > NSS_DTLSMGR_DEBUG_LEVEL_WARN)	\
		pr_notice("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)	\
}

#define nss_dtlsmgr_trace(s, ...) {	\
	if (NSS_DTLSMGR_DEBUG_LEVEL > NSS_DTLSMGR_DEBUG_LEVEL_INFO)	\
		pr_info("%s[%d]:" s "\n", __func__, __LINE__, ##__VA_ARGS__)	\
}

#endif /* CONFIG_DYNAMIC_DEBUG */

#define NSS_DTLSMGR_DTLS_HDR_SZ 13			/* DTLS header length */
#define NSS_DTLSMGR_CAPWAP_DTLS_HDR_SZ 4		/* CAPWAP-DTLS header length */
#define NSS_DTLSMGR_CTX_MAGIC 0x5d7eb219		/* DTLS context magic value */


#if defined (NSS_DTLSMGR_DEBUG)
#define NSS_DTLSMGR_VERIFY_MAGIC(ctx) do { \
	struct nss_dtlsmgr_ctx *__ctx = (ctx);	\
	BUG_ON(__ctx->magic != NSS_DTLSMGR_CTX_MAGIC);	\
} while(0)

#define NSS_DTLSMGR_SET_MAGIC(ctx, magic) do {	\
	struct nss_dtlsmgr_ctx *__ctx = (ctx);	\
	__ctx->magic = (magic);	\
} while(0)
#else
#define NSS_DTLSMGR_VERIFY_MAGIC(ctx)
#define NSS_DTLSMGR_SET_MAGIC(ctx, magic)
#endif

/*
 * DTLS algorithm information
 */
struct nss_dtlsmgr_algo_info {
	char *name;				/* Linux crypto algorithm string. */
	uint32_t rta_key_size;			/* RTA key attribute size. */
};

/*
 * DTLS flow information
 */
struct nss_dtlsmgr_flow_data {
	uint32_t sip[4];			/* Source IPv4/v6 address. */
	uint32_t dip[4];			/* Destination IPv4/v6 address. */
	uint32_t flags;				/* Transformation flags. */

	uint16_t sport;				/* Source UDP/UPDLite port. */
	uint16_t dport;				/* Destination UDP/UDPLite port. */

	uint8_t dscp;				/* Dscp value incase of static. */
	uint8_t hop_limit_ttl;			/* Hop limit or time to live. */
	bool dscp_copy;				/* Copy dscp. */
	bool df;				/* Do not fragment settings. */
};

/*
 * DTLS configuration data
 */
struct nss_dtlsmgr_dtls_data {
	struct list_head list;			/* List of crypto data. */
	struct crypto_aead *aead;		/* Linux AEAD context. */
	uint32_t crypto_idx;
	uint32_t ver;				/* DTLS version. */

	uint16_t window_size;			/* DTLS anti-replay window. */
	uint16_t epoch;				/* Current epoch. */

	uint8_t blk_len;			/* Cipher block length. */
	uint8_t hash_len;			/* Hash length. */
	uint8_t iv_len;				/* IV length. */
	uint8_t res1;
};

/*
 * DTLS context data
 */
struct nss_dtlsmgr_ctx_data {
	struct nss_dtlsmgr_stats stats;		/* Statistics. */
	struct nss_dtlsmgr_flow_data flow;	/* Flow data information. */
	struct nss_ctx_instance *nss_ctx;	/* NSS context handle. */
	struct list_head dtls_active;		/* List of active DTLS record(s). */

	uint32_t headroom;			/* Headroom needed. */
	uint32_t tailroom;			/* Tailroom needed. */
	uint32_t ifnum;				/* NSS interface number. */
	uint32_t src_ifnum;			/* Source interface number for NSS. */
	uint32_t dest_ifnum;			/* Destination interface number for NSS. */
	uint32_t flags;				/* DTLS flags. */
	uint32_t di_type;			/* Dynamic interface type. */
};

/*
 * DTLS manager context
 */
struct nss_dtlsmgr_ctx {
	rwlock_t lock;				/* Context lock. */
	struct net_device *dev;			/* Session netdevice. */
	struct dentry *dentry;			/* Debugfs directory for ctx statistics. */

	struct nss_dtlsmgr_ctx_data encap;	/* Encapsulation data. */
	struct nss_dtlsmgr_ctx_data decap;	/* Decapsulation data. */

	void *app_data;				/* Opaque data for callback */
	nss_dtlsmgr_notify_callback_t notify_cb;/* Statistics notification callback. */
	nss_dtlsmgr_data_callback_t data_cb;	/* Data callback. */

#if defined (NSS_DTLSMGR_DEBUG)
	uint32_t magic;				/* Magic check. */
#endif
};

/*
 * DTLS manager data
 */
struct nss_dtlsmgr {
	atomic_t is_configured;			/* Firmware is configured. */
	struct dentry *root_dir;		/* Debugfs root directory. */
	struct nss_ctx_instance *nss_ctx;	/* NSS data/message handle. */
};

extern struct nss_dtlsmgr g_dtls;

extern void nss_dtlsmgr_ctx_dev_event_inner(void *if_ctx, struct nss_cmn_msg *ndcm);
extern void nss_dtlsmgr_ctx_dev_event_outer(void *if_ctx, struct nss_cmn_msg *ndcm);
extern void nss_dtlsmgr_ctx_dev_data_callback(void *app_data, struct sk_buff *skb);
extern void nss_dtlsmgr_ctx_dev_rx_inner(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi);
extern void nss_dtlsmgr_ctx_dev_rx_outer(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi);
extern void nss_dtlsmgr_ctx_dev_setup(struct net_device *dev);
extern int nss_dtlsmgr_create_debugfs(struct nss_dtlsmgr_ctx *ctx);
#endif /* !__NSS_DTLSMGR_PRIVATE_H_ */
