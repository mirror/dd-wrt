/*
 * ********************************************************************************
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_IPSECMGR_CTX_H
#define __NSS_IPSECMGR_CTX_H

#define NSS_IPSECMGR_CTX_PRINT_EXTRA 64
#define NSS_IPSECMGR_CTX_PRINT_LEN (PAGE_SIZE * NSS_IPSECMGR_PRINT_PAGES)
#define NSS_IPSECMGR_CTX_FREE_TIMEOUT msecs_to_jiffies(100) /* msecs */

/*
 * Context host statistics
 */
struct nss_ipsecmgr_ctx_host_stats {
	uint64_t v4_notify;		/* Host processed Exceptioned IPv4 inner packet */
	uint64_t v4_notify_drop;	/* Host processed and dropped IPv4 inner packet */
	uint64_t v4_route;		/* Host processed IPv4 outer packet */
	uint64_t v4_route_drop;		/* Host processed and dropped IPv4 outer packet */
	uint64_t v6_notify;		/* Host processed Exceptioned IPv6 inner packet */
	uint64_t v6_notify_drop;	/* Host processed and dropped IPv6 inner packet */
	uint64_t v6_route;		/* Host processed IPv6 outer packet */
	uint64_t v6_route_drop;		/* Host processed and dropped IPv6 outer packet */
	uint64_t inner_exp;		/* Host processed inner IPv4 exceptioned packet */
	uint64_t inner_exp_drop;	/* Host processed and dropped inner exceptioned packet */
	uint64_t inner_cb;		/* Number of times data call back called for inner packet */
	uint64_t inner_fail_dev;	/* Failed to find netdevice for inner packet */
	uint64_t inner_fail_sa;		/* Failed to find SA for inner packet */
	uint64_t inner_fail_flow;	/* Failed to find flow for inner packet */
	uint64_t outer_exp;		/* Host processed inner IPv6 exceptioned packet */
	uint64_t outer_exp_drop;	/* Host processed and dropped inner IPv6 exceptioned packet */
	uint64_t outer_cb;		/* Number of times exception call back called for outer packet */
	uint64_t outer_fail_dev;	/* Failed to find netdevice for inner packet */
	uint64_t outer_fail_sa;		/* Failed to find SA for outer packet */
	uint64_t outer_fail_flow;	/* Failed to find flow for outer packet */
	uint64_t redir_exp;		/* Redir exceptioned packet */
	uint64_t redir_exp_drop;	/* Redir exceptioned and dropped */
	uint64_t redir_cb;		/* Redir callback called */
	uint64_t redir_fail_dev;	/* Failed to find netdevice */
	uint64_t redir_fail_sa;		/* Failed to find SA */
	uint64_t redir_fail_flow;	/* Failed to find flow */
};

/*
 * Context statistics
 */
struct nss_ipsecmgr_ctx_stats_priv {
	/* Packet counters */
	uint64_t rx_packets;			/* Number of packets received. */
	uint64_t rx_bytes;			/* Number of bytes received. */
	uint64_t tx_packets;			/* Number of packets transmitted. */
	uint64_t tx_bytes;			/* Number of bytes transmitted. */
	uint64_t rx_dropped[NSS_MAX_NUM_PRI];	/* Packets dropped on receive due to queue full. */

	/* Drop counters */
	uint64_t exceptioned;		/* Exceptioned to host */
	uint64_t linearized;		/* Linearized packets */
	uint64_t redirected;		/* Redirected from inline */
	uint64_t dropped;		/* Total dropped packets */
	uint64_t fail_sa;		/* Failed to find SA */
	uint64_t fail_flow;		/* Failed to find flow */
	uint64_t fail_stats;		/* Failed to send statistics */
	uint64_t fail_exception;	/* Failed to exception */
	uint64_t fail_transform;	/* Failed to transform */
	uint64_t fail_linearized;	/* Failed to linearized */
	uint64_t fail_mdata_ver;	/* Invalid meta data version */
	uint64_t fail_ctx_active;	/* Failed to queue as ctx is not active. */
	uint64_t fail_pbuf_crypto;	/* Failed to allocate pbuf for crypto operation */
	uint64_t fail_queue_crypto;	/* Failed to queue pbuf to crypto pnode */
};

/*
 * Per context state
 */
struct nss_ipsecmgr_ctx_state {
	ssize_t print_len;				/* Print buffer length */
	ssize_t stats_len;				/* Total stats length */
	uint32_t except_ifnum;				/* Exception interface number */
	uint32_t sibling_ifnum;				/* Sibling interface number */
	enum nss_ipsec_cmn_ctx_type type;		/* Type */
	enum nss_dynamic_interface_type di_type;	/* Dynamic interface type */
};

/*
 * IPsec manager Context (encap/decap/bounce)
 */
struct nss_ipsecmgr_ctx {
	struct list_head list;				/* List node */
	struct nss_ipsecmgr_ref ref;			/* Reference node */
	struct nss_ipsecmgr_tunnel *tun;		/* IPsec tunnel */

	uint32_t ifnum;					/* Interface number */
	struct nss_ctx_instance *nss_ctx;		/* NSS context instance */

	struct nss_ipsecmgr_ctx_state state;		/* Per context state */
	struct nss_ipsecmgr_ctx_stats_priv stats;	/* Statistics */
	struct nss_ipsecmgr_ctx_host_stats hstats;	/* Host statistics */
};

/*
 * Set the exception interface number for context
 */
static inline void nss_ipsecmgr_ctx_set_except(struct nss_ipsecmgr_ctx *ctx, uint32_t except_ifnum)
{
	ctx->state.except_ifnum = except_ifnum;
}

/*
 * Set the sibling interface number for context
 */
static inline void nss_ipsecmgr_ctx_set_sibling(struct nss_ipsecmgr_ctx *ctx, uint32_t sibling_ifnum)
{
	ctx->state.sibling_ifnum = sibling_ifnum;
}

extern const struct file_operations ipsecmgr_ctx_file_ops;

/* API(s) for context specific operations */
extern void nss_ipsecmgr_ctx_rx_stats(void *app_data, struct nss_cmn_msg *ncm);
extern void nss_ipsecmgr_ctx_rx_redir(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi);
extern void nss_ipsecmgr_ctx_rx_outer(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi);
extern void nss_ipsecmgr_ctx_rx_inner(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi);

extern void nss_ipsecmgr_ctx_attach(struct list_head *db, struct nss_ipsecmgr_ctx *ctx);
extern bool nss_ipsecmgr_ctx_config(struct nss_ipsecmgr_ctx *ctx);
extern void nss_ipsecmgr_ctx_free(struct nss_ipsecmgr_ctx *ctx);
extern struct nss_ipsecmgr_ctx *nss_ipsecmgr_ctx_alloc(struct nss_ipsecmgr_tunnel *tun,
							enum nss_ipsec_cmn_ctx_type ctx_type,
							enum nss_dynamic_interface_type di_type,
							nss_ipsec_cmn_data_callback_t rx_data,
							nss_ipsec_cmn_msg_callback_t rx_stats,
							uint32_t features);
extern void nss_ipsecmgr_ctx_stats_read(struct nss_ipsecmgr_ctx *ctx, struct rtnl_link_stats64 *dev_stats);
extern struct nss_ipsecmgr_ctx *nss_ipsecmgr_ctx_find(struct nss_ipsecmgr_tunnel *tun,
							enum nss_ipsec_cmn_ctx_type type);
extern struct nss_ipsecmgr_ctx *nss_ipsecmgr_ctx_find_by_sa(struct nss_ipsecmgr_tunnel *tun,
							enum nss_ipsecmgr_sa_type sa_type);

#endif /* !__NSS_IPSECMGR_CTX_H */
