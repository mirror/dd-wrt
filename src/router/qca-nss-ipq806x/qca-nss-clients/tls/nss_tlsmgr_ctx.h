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
 * nss_tlsmgr_ctx.h
 */

#ifndef __NSS_TLSMGR_CTX_H_
#define __NSS_TLSMGR_CTX_H_

#define NSS_TLSMGR_CTX_PRINT_EXTRA 64	/* Bytes */

/*
 * Context statistics
 */
struct nss_tlsmgr_ctx_fw_stats {
	/* Packet counters */
	uint64_t rx_packets;                    /* Number of packets received. */
	uint64_t rx_bytes;                      /* Number of bytes received. */
	uint64_t tx_packets;                    /* Number of packets transmitted. */
	uint64_t tx_bytes;                      /* Number of bytes transmitted. */
	uint64_t rx_dropped[NSS_MAX_NUM_PRI];   /* Packets dropped on receive due to queue full. */

	uint64_t single_rec;			/**< Number of Tx single record datagrams. */
	uint64_t multi_rec;			/**< Number of multiple Tx record datagrams. */
	uint64_t tx_inval_reqs;			/**< Number of Tx invalidation successfully requested. */
	uint64_t rx_ccs_rec;			/**< Number of change cipher spec records received. */
	uint64_t fail_ccs;			/**< Failed to switch to new crypto. */
	uint64_t eth_node_deactive;		/**< Ethernet node deactivated as no crypto available. */
	uint64_t crypto_alloc_success;		/**< Number of crypto allocation. */
	uint64_t crypto_free_req;		/**< Number of crypto free request. */
	uint64_t crypto_free_success;		/**< Number of crypto free success. */
	uint64_t fail_crypto_alloc;		/**< Number of crypto allocation failed. */
	uint64_t fail_crypto_lookup;		/**< Failed to find acive crypto session. */
	uint64_t fail_req_alloc;		/**< Failuer to allocate request memory pool.  */
	uint64_t fail_pbuf_stats;		/**< Failure in pbuf allocation for statistics. */
	uint64_t fail_ctx_active;		/**< Failure in enqueue due to inactive context. */

	/*
	 * Dont change the order below
	 */
	uint64_t hw_len_error;			/**< Length error. */
	uint64_t hw_token_error;		/**< Token error, unknown token command/instruction. */
	uint64_t hw_bypass_error;		/**< Token contains too much bypass data. */
	uint64_t hw_crypto_error;		/**< Cryptograhic block size error. */
	uint64_t hw_hash_error;			/**< Hash block size error. */
	uint64_t hw_config_error;		/**< Invalid command/algorithm/mode/combination. */
	uint64_t hw_algo_error;			/**< Unsupported algorithm. */
	uint64_t hw_hash_ovf_error;		/**< Hash input overflow. */
	uint64_t hw_auth_error;         	/**< Hash input overflow. */
	uint64_t hw_pad_verify_error;		/**< Pad verification error. */
	uint64_t hw_timeout_error;              /**< Data timed-out. */

	/*
	 * Performance statistics
	 */
	uint64_t no_desc_in;			/**< Ingress DMA descriptor not available. */
	uint64_t no_desc_out;			/**< Egress DMA descriptor not available. */
	uint64_t no_reqs;			/**< Not enough requests available for records. */
};

/*
 * TLS context
 */
struct nss_tlsmgr_ctx {
	struct net_device *dev;				/* Tunnel device. */
	struct device *nss_dev;				/* NSS device. */
	struct dentry *dentry;  			/* Debugfs directory for ctx statistics. */
	struct nss_ctx_instance *nss_ctx;		/* NSS context handle. */
	struct list_head crypto_active;			/* List of active TLS record(s). */

	struct nss_tlsmgr_ctx_fw_stats fw_stats;	/* Context FW Statistics. */
	struct nss_tlsmgr_pkt_stats host_stats; 	/* Host statistics. */
	ssize_t print_len;				/* Print buffer lenght. */

	uint32_t ifnum;         			/* NSS interface number. */
	uint32_t di_type;       			/* Dynamic interface type. */
};

nss_tlsmgr_status_t nss_tlsmgr_ctx_tx(struct nss_tlsmgr_ctx *ctx, struct sk_buff *skb, struct nss_tlsmgr_rec *rec);
extern int nss_tlsmgr_ctx_config_inner(struct nss_tlsmgr_ctx *ctx, struct net_device *dev);
extern int nss_tlsmgr_ctx_config_outer(struct nss_tlsmgr_ctx *ctx, struct net_device *dev);
extern void nss_tlsmgr_ctx_deconfig(struct nss_tlsmgr_ctx *ctx);
extern void nss_tlsmgr_ctx_stats_copy(struct nss_tlsmgr_ctx *ctx, struct rtnl_link_stats64 *dev_stats);

ssize_t nss_tlsmgr_ctx_read_stats(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos);
#endif /* !__NSS_TLSMGR_CTX_H_ */
