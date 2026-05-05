/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#ifndef __EIP_TR_H
#define __EIP_TR_H

/*
 * TODO: Set timeout equal to processing time for max queue reap.
 */
#define EIP_TR_INVAL_TIMEOUT msecs_to_jiffies(1000)
#define EIP_TR_SIZE (sizeof(struct eip_tr) + EIP_HW_CTX_SIZE_LARGE_BYTES + L1_CACHE_BYTES)
#define EIP_TR_CTRL_CONTEXT_WORDS(x) (((x) & 0x3FU) << 8)
#define EIP_TR_REDIR_EN (0x1U << 11) /* Redirect enable */
#define EIP_TR_REDIR_IFACE(i) (((i) & 0xFU) << 12) /* Redirect destination interface */
#define EIP_MAX_PKT_LEN	9216 /* Bytes */

struct eip_svc_entry;
typedef bool (*eip_tr_init_t) (struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);

/*
 * IPsec specific Transform Record Field
 */
#define EIP_TR_IPSEC_SPI (1U << 27)
#define EIP_TR_IPSEC_SEQ_NUM (1U << 28)
#define EIP_TR_IPSEC_EXT_SEQ_NUM(x) ((x) << 29)
#define EIP_TR_IPSEC_CONTROL_IV(x) ((x) << 5)
#define EIP_TR_IPSEC_IV_FORMAT(x) ((x) << 10)
#define EIP_TR_IPSEC_SEQ_NUM_STORE (1U << 22)
#define EIP_TR_IPSEC_PAD_TYPE (0x1U << 16)
#define EIP_TR_IPSEC_NULL_PAD_TYPE (0x7U << 14)
#define EIP_TR_IPSEC_IPHDR_PROC (0x1U << 19)
#define EIP_TR_IPSEC_TTL(x) ((x) << 16)
#define EIP_TR_IPSEC_ENCAP_TOKEN_VERIFY 0xd0060000U
#define EIP_TR_IPSEC_ENCAP_TOKEN_INST 0xe12e0800U
#define EIP_TR_IPSEC_ENCAP_ESN_TOKEN_INST 0xe2560800U
#define EIP_TR_IPSEC_ENCAP_TOKEN_INST_LEN(x) (((x) + 1U) << 24)
#define EIP_TR_IPSEC_ENCAP_TOKEN_HDR 0x420000U
#define EIP_TR_IPSEC_ENCAP_TOKEN_HDR_IV 0x4000000U
#define EIP_TR_IPSEC_IV_SIZE(x) (x)
#define EIP_TR_IPSEC_ICV_SIZE(x) ((x) << 8)
#define EIP_TR_IPSEC_OHDR_PROTO(x) ((x) << 16)
#define EIP_TR_IPSEC_ESP_PROTO(x) ((x) << 24)
#define EIP_TR_IPSEC_NATT_SPORT IPSEC_EIP197_NATT_SPORT
#define EIP_TR_IPSEC_NATT_DPORT (IPSEC_EIP197_NATT_SPORT << 16)
#define EIP_TR_IPSEC_SEQ_NUM_OFFSET(x) ((x) << 24)
#define EIP_TR_IPSEC_SEQ_NUM_OFFSET_EN (1U << 30)
#define EIP_TR_IPSEC_DF_COPY	0
#define EIP_TR_IPSEC_DF_RESET	1U
#define EIP_TR_IPSEC_DF_SET	2U
#define EIP_TR_IPSEC_DF(x) ((x & 0x3U) << 20)
#define EIP_TR_IPSEC_DSCP(x) ((x) << 24)
#define EIP_TR_IPSEC_DSCP_COPY_EN(x) ((x) << 22)
#define EIP_TR_IPSEC_IPV6_EN (0x1U << 8)
#define EIP_TR_IPSEC_DECAP_TOKEN_VERIFY 0xd0060000U
#define EIP_TR_IPSEC_DECAP_TOKEN_VERIFY_SEQ 0x8000000U
#define EIP_TR_IPSEC_DECAP_TOKEN_VERIFY_PAD 0x5000000U
#define EIP_TR_IPSEC_DECAP_TOKEN_VERIFY_HMAC 0x10000U
#define EIP_TR_IPSEC_REPLAY_WINDOW_SZ_32 (1U << 8);
#define EIP_TR_IPSEC_REPLAY_WINDOW_SZ_64 (2U << 8);
#define EIP_TR_IPSEC_REPLAY_WINDOW_SZ_128 (4U << 8);
#define EIP_TR_IPSEC_REPLAY_WINDOW_SZ_384 (12U << 8);
#define EIP_TR_IPSEC_SEQ_NUM_MASK_32 (0x2U << 30)
#define EIP_TR_IPSEC_SEQ_NUM_MASK_64 (0x1U << 30)
#define EIP_TR_IPSEC_SEQ_NUM_MASK_128 (0x3U << 30)
#define EIP_TR_IPSEC_SEQ_NUM_MASK_384 ((0x2U << 30) | (0x1U << 15))
#define EIP_TR_IPSEC_DECAP_TOKEN_INST_SEQ_UPDATE(x) ((x) << 24)
#define EIP_TR_IPSEC_EXT_SEQ_NUM_PROC(x) ((x) << 29)
#define EIP_TR_IPSEC_DECAP_TOKEN_INST 0xe02e1800U
#define EIP_TR_IPSEC_DECAP_ESN_TOKEN_INST 0xe0561800U
#define EIP_TR_IPSEC_DECAP_TUNNEL_TOKEN_HDR 0x01020000U
#define EIP_TR_IPSEC_DECAP_TRANSPORT_TOKEN_HDR 0x01820000U

#define EIP_TR_IPSEC_OHDR_PROTO_BYPASS 0
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_ENC 2U
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_DEC 4U
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_ENC 5U
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_DEC 6U
#define EIP_TR_IPSEC_OHDR_PROTO_V6_TUNNEL_ENC 7U
#define EIP_TR_IPSEC_OHDR_PROTO_V6_TUNNEL_DEC 8U
#define EIP_TR_IPSEC_OHDR_PROTO_V6_TRANSPORT_ENC 11U
#define EIP_TR_IPSEC_OHDR_PROTO_V6_TRANSPORT_DEC 12U
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_NATT_ENC 22U
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TUNNEL_NATT_DEC 24U
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_NATT_ENC 25U
#define EIP_TR_IPSEC_OHDR_PROTO_V4_TRANSPORT_NATT_DEC 26U

#define EIP_TR_IPSEC_PROTO_NONE 0
#define EIP_TR_IPSEC_PROTO_OUT_CBC 1U
#define EIP_TR_IPSEC_PROTO_OUT_NULL_AUTH 2U
#define EIP_TR_IPSEC_PROTO_OUT_CTR 3U
#define EIP_TR_IPSEC_PROTO_OUT_CCM 4U
#define EIP_TR_IPSEC_PROTO_OUT_GCM 5U
#define EIP_TR_IPSEC_PROTO_OUT_GMAC 6U
#define EIP_TR_IPSEC_PROTO_IN_CBC 7U
#define EIP_TR_IPSEC_PROTO_IN_NULL_AUTH 8U
#define EIP_TR_IPSEC_PROTO_IN_CTR 9U
#define EIP_TR_IPSEC_PROTO_IN_CCM 10U
#define EIP_TR_IPSEC_PROTO_IN_GCM 11U
#define EIP_TR_IPSEC_PROTO_IN_GMAC 12U

#define U64_FROM_U32(hi, lo) (((u64)(hi) << 32)|(u64)(lo))

/*
 * eip_tr_stats
 *	Statistics for each transform record.
 */
struct eip_tr_stats {
	uint64_t tx_frags;	/* Total buffer fragments sent via this TR */
	uint64_t tx_pkts;	/* Total buffer sent via this TR */
	uint64_t tx_bytes;	/* Total bytes via this TR */
	uint64_t tx_error_len;	/* Total packets dropped for exceeding max jumbo limit */
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
	struct eip_tr_ops ops;		/* Transformation operation callback */
	void *app_data;			/* Opaque to pass with callback */
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
	};

	uint32_t tr_addr_type;			/* TR address with lower 2-bit representing type */
	uint32_t ctrl_words[2];			/* Initial HW control word */
	uint32_t bypass[4];			/* Hybrid DMA bypass data */
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
	struct eip_hw_stats tr_stats;		/* Last readed TR stats */

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
