/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
 */

#ifndef __EIP_TR_H
#define __EIP_TR_H

/*
 * TODO: Set timeout equal to processing time for max queue reap.
 */
#define EIP_TR_INVAL_TIMEOUT msecs_to_jiffies(1000)
#define EIP_TR_SIZE (sizeof(struct eip_tr) + EIP_HW_CTX_SIZE_LARGE_BYTES + L1_CACHE_BYTES)
#define EIP_TR_PRE_PROCESS(tr, ops, skb) ((ops)->pre(tr, skb)) /* Call pre processing function */
#define EIP_TR_POST_PROCESS(tr, ops, skb) ((ops)->post(tr, skb)) /* Call post processing function */

struct eip_svc_entry;
typedef bool (*eip_tr_init_t) (struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
typedef void (*eip_tr_proc_t)(struct eip_tr *tr, struct sk_buff *skb);

/*
 * Context control words fields
 */
#define EIP_TR_CTRL_CONTEXT_WORDS(x) (((x) & 0x3F) << 8)
#define EIP_TR_CTRL_SEQ_NUM_STORE (1U << 22)
#define EIP_TR_CTRL_SEQ_NUM_OFFSET(x) ((x) << 24)
#define EIP_TR_CTRL_SEQ_NUM_OFFSET_EN (1U << 30)

/*
 * IPsec specific Transform Record Field
 */
#define EIP_TR_IPSEC_SPI (1U << 27)
#define EIP_TR_IPSEC_SEQ_NUM (1U << 28)
#define EIP_TR_IPSEC_EXT_SEQ_NUM(x) ((x) << 29)
#define EIP_TR_IPSEC_CONTROL_IV(x) ((x) << 5)
#define EIP_TR_IPSEC_IV_FORMAT(x) ((x) << 10)
#define EIP_TR_IPSEC_PAD_TYPE (0x1U << 16)
#define EIP_TR_IPSEC_NULL_PAD_TYPE (0x7U << 14)
#define EIP_TR_IPSEC_SEQ_NUM_MASK_32 (0x2U << 30)
#define EIP_TR_IPSEC_SEQ_NUM_MASK_64 (0x1U << 30)
#define EIP_TR_IPSEC_SEQ_NUM_MASK_128 (0x3U << 30)
#define EIP_TR_IPSEC_SEQ_NUM_MASK_384 ((0x2U << 30) | (0x1U << 15))

/*
 * DTLS specific fields.
 * TODO: Shall we generalize those macro across IPsec & DTLS?
 */
#define EIP_TR_DTLS_CTX0_SEQNO (0x2U << 28) /* dtls 48-bit sequence number */
#define EIP_TR_DTLS_CTX0_SEQNO_MASK64 (0x1U << 30)	/* 64bit replay mask */
#define EIP_TR_DTLS_CTX0_SEQNO_MASK128 (0x3U << 30)	/* 128bit replay mask */
#define EIP_TR_DTLS_CTX1_PAD_TLS (0x5U << 14) /* cipher padding */
#define EIP_TR_DTLS_CTX1_HASH_STORE (0x1U << 19) /* store hash */
#define EIP_TR_DTLS_CTX1_PRE_CRYPTO_DECAP (0x1 << 13) /* decap pre-crypto operation */
#define EIP_TR_DTLS_CONTROL_IV(x) ((x) << 5)	/* Control IV source */
#define EIP_TR_DTLS_IV_FORMAT(x) ((x) << 10)	/* Full IV mode or Counter mode */

#define EIP_TR_ERR_SHIFT 16U

#define EIP_TR_IPSEC_CTX_WORDS_ESN(esn) ((esn) ? 2 : 1)
#define EIP_TR_IPSEC_SKB_CB(__skb) ((struct eip_tr_ipsec_skb_cb *)&((__skb)->cb[0]))

/*
 * eip_tr_stats
 *	Statistics for each transform record.
 */
struct eip_tr_stats {
	uint64_t tx_frags;	/* Total buffer fragments sent via this TR */
	uint64_t tx_pkts;	/* Total buffer sent via this TR */
	uint64_t tx_bytes;	/* Total bytes via this TR */
	uint64_t rx_frags;	/* Total buffer fragments received via this TR */
	uint64_t rx_pkts;	/* Total buffer received via this TR */
	uint64_t rx_bytes;	/* Total bytes recieved via this TR */
	uint64_t rx_error;	/* Total buffer received with HW error */
};

/*
 * eip_tr_ops
 *	Template to hold callbacks.
 */
struct eip_tr_ops {
	eip_tk_proc_t tk_fill;		/* cached Token fill method */
	eip_tr_proc_t pre, post;	/* Pre & Post processing methods */

	void *app_data;			/* App data passed for callback */
	eip_tr_callback_t cb;		/* completion callback to client */
	eip_tr_err_callback_t err_cb;	/* completion with error callback to client */
};

/*
 * eip_tr_crypto
 *	Crypto specific information.
 */
struct eip_tr_crypto {
	struct eip_tr_ops enc;		/* Encode operation */
	struct eip_tr_ops dec;		/* Decode operation */
	struct eip_tr_ops auth;		/* Auth operation */
};

/*
 * eip_tr_ipsec
 *	IPsec specific information.
 */
struct eip_tr_ipsec {
	struct eip_tr_ops ops;  		/* Transformation operation callback */
	__be32 spi_idx;				/* SPI index */
	__be32 src_ip[4];			/* Source IP address */
	__be32 dst_ip[4];			/* Destination IP address */
	__be16 src_port;			/* Source UDP port */
	__be16 dst_port;			/* Destination UDP port */
	uint8_t ip_version;			/* v4 or v6 */
	uint8_t protocol;			/* ESP or UDP */
	uint8_t df;				/* Don't fragment configuration */
	uint8_t tos;				/* DSCP for transform record */
	uint8_t ttl;				/* Time-to-live */
	uint8_t seq_offset;			/* Sequence no offset in context record */
	uint8_t ctx_num_words;			/* No of context dword to be updated */
	uint8_t remove_len;			/* Length of headers to be removed */
	uint8_t bypass_len; 			/* Length of headers to be bypassed (IP/IP+UDP) */
	uint8_t fixed_len; 			/* Per packet fixed length */
};

/*
 * eip_tr_dtls
 *	DTLS specific information.
 */
struct eip_tr_dtls {
	struct eip_tr_ops ops;			/* Transformation operation callback */
	__be32 src_ip[4];			/* Source IP address */
	__be32 dst_ip[4];			/* Destination IP address */
	__be16 src_port;			/* Source UDP port */
	__be16 dst_port;			/* Destination UDP port */
	__be16 version;				/* version */
	__be16 ip_df;				/* htons(IP_DF) or 0 */
	uint8_t ip_version;			/* v4 or v6 */
	uint8_t protocol;			/* UDP or UDPlite */
	uint8_t tos;				/* DSCP for transform record */
	uint8_t ttl;				/* Time-to-live */
	uint8_t seq_offset;			/* Sequence no offset in context record */
	uint8_t remove_len;			/* Length of headers to be removed */
	uint8_t bypass_len;			/* Length of headers to be bypassed (IP/IP+UDP) */
	uint8_t fixed_len;			/* Per packet fixed length */
};

/*
 * eip_tr_ipsec_skb_cb
 * 	IPsec sepcific information in skb control block
 */
struct eip_tr_ipsec_skb_cb {
	uint8_t ip_proto;	/* Protocol for IPv4, NH for IPv6. */
	uint8_t pad_len;	/* Padding length required for encryption */
};

/*
 * eip_tr
 *	Transformation record allocated by HW for each session.
 */
struct eip_tr {
	struct list_head node;			/* Node under per context TR list */
	atomic_t active;			/* TR active flag. */
	struct eip_ctx *ctx;			/* Parent context */

	union {
		struct eip_tr_crypto crypto;	/* Crypto information */
		struct eip_tr_ipsec ipsec;	/* IPsec information */
		struct eip_tr_dtls dtls;	/* DTLS information */
	};

	uint32_t tr_addr_type;			/* TR address with lower 2-bit representing type */
	uint32_t ctrl_words[2];			/* Initial HW control word */
	uint32_t nonce;				/* nonce passed during allocation */
	uint32_t tr_flags;			/* Flags for TR */
	uint16_t iv_len;			/* IV length for configured algo FIXME: NA for Crypto. Move to ipsec */
	uint16_t digest_len;			/* HMAC length for configured algo */
	uint16_t blk_len;			/* Cipher block length for configured algo */

	struct eip_tr_stats __percpu *stats_pcpu;	/* Statistisc */
	struct kref ref;			/* Reference incremented per packet */

	enum eip_svc svc;			/* Service number */
	uint32_t inval_dummy_buf;		/* Dummy 4 byte buffer for Invalidation. */
	struct delayed_work inval_work;		/* Workqueue for Invalidation schedule */

	uint32_t hw_words[] __attribute__((aligned (L1_CACHE_BYTES)));
						/* Hardware record words. Aligned it to exact L1 cache line boundary */
};

const struct eip_svc_entry *eip_tr_skcipher_get_svc(void);
size_t eip_tr_skcipher_get_svc_len(void);
const struct eip_svc_entry *eip_tr_ahash_get_svc(void);
size_t eip_tr_ahash_get_svc_len(void);
const struct eip_svc_entry *eip_tr_aead_get_svc(void);
size_t eip_tr_aead_get_svc_len(void);
const struct eip_svc_entry *eip_tr_ipsec_get_svc(void);
size_t eip_tr_ipsec_get_svc_len(void);

bool eip_tr_ahash_key2digest(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
int eip_tr_genkey(struct eip_tr *tr, struct eip_tr_info *info, uint8_t *key, uint16_t len);
void eip_tr_reset_stats(struct eip_tr *tr);
void eip_tr_get_stats(struct eip_tr *tr, struct eip_tr_stats *stats);
void eip_tr_final(struct kref *kref);
int eip_tr_classify_err(uint16_t cle_err, uint16_t tr_err);

/*
 * eip_tr_ref()
 *	Increment transform record reference.
 */
static inline struct eip_tr *eip_tr_ref(struct eip_tr *tr)
{
	kref_get(&tr->ref);
	return tr;
}

/*
 * eip_tr_ref()
 *	Increment transform record reference only if nonzero.
 */
static inline struct eip_tr *eip_tr_ref_unless_zero(struct eip_tr *tr)
{
	if (!kref_get_unless_zero(&tr->ref))
		return NULL;

	return tr;
}

/*
 * eip_tr_deref()
 *	Decrement transform record reference.
 */
static inline void eip_tr_deref(struct eip_tr *tr)
{
	kref_put(&tr->ref, eip_tr_final);
}

/*
 * eip_tr_fill_token()
 *	Fill Tokens from transform record by calling token fill callback.
 */
static inline uint8_t eip_tr_fill_token(struct eip_tk *tk, struct eip_tr_ops *ops,
		struct eip_tk_params *tk_params)
{
	return ops->tk_fill(tk, tk_params);
}

#endif /* __EIP_TR_H */
