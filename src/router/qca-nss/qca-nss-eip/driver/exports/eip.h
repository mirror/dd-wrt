/*
 * Copyright (c) 2022-2026, Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef __EIP_H
#define __EIP_H

#include <linux/version.h>
#include <linux/netdevice.h>
#include <crypto/aead.h>
#include <crypto/skcipher.h>
#include <crypto/hash.h>
#include <soc/qcom/socinfo.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(6, 1, 0))
#include <dt-bindings/arm/qcom,ids.h>
#include <linux/soc/qcom/smem.h>
#endif

#define EIP_TR_IPSEC_FLAG_TUNNEL   BIT(0)  /* set = Tunnel, clear = Transport */
#define EIP_TR_IPSEC_FLAG_ENC      BIT(1)  /* set = Encapsulation, clear = Decapsulation */
#define EIP_TR_IPSEC_FLAG_IPV6     BIT(2)  /* set = IPv6 tuple, clear = IPv4 tuple SA */
#define EIP_TR_IPSEC_FLAG_UDP      BIT(3)  /* set = ESP-in-UDP, clear = ESP encapsulation */
#define EIP_TR_IPSEC_FLAG_ESN      BIT(4)  /* set = Extended sequence number, clear = 32-bit sequence number */
#define EIP_TR_IPSEC_FLAG_SKIP     BIT(5)  /* set = SKIP SA, clear = RFC */
#define EIP_TR_IPSEC_FLAG_CP_TOS   BIT(6)  /* set = copy IP TOS value from inner, clear = use default TOS */
#define EIP_TR_IPSEC_FLAG_CP_DF    BIT(7)  /* set = copy IP DF value from inner, clear = use default DF */

#define EIP_DTLS_TYPE_APP_DATA	0x17U	/* DTLS header type is Application data */
#define EIP_DTLS_TYPE_CC_SPEC	0x20U	/* DTLS header type is change cipher spec */

/*
 * eip_features_t
 *	EIP HW features.
 */
typedef enum eip_hw_features {
	EIP_OFFLOAD_INNER_FLOW = BIT(0),	/* EIP supports inner flow offload */
	EIP_OFFLOAD_OUTER_FLOW = BIT(1),	/* EIP supports outer flow offload */
} eip_features_t;

struct eip_ctx;	/* HW context per sevice */
struct eip_tr;	/* Transform record per session */
struct eip_tun;	/* EIP flow context */

/*
 * Request object. Use below inline APIs to get the relavent objects.
 */
typedef void *eip_req_t;

/*
 * Tranformation completion callback.
 *
 * @datatypes
 * eip_req_t \n
 *
 * @param[in] app_data	Opaque data to receive on callback.
 * @param[in] req       Request received in callback.
 */
typedef void (*eip_tr_callback_t)(void *app_data, eip_req_t req);

/*
 * Tranformation completion with error callback.
 *
 * @datatypes
 * eip_req_t \n
 *
 * @param[in] app_data	Opaque data to receive on callback.
 * @param[in] req       Request received in callback.
 * @param[in] err	Any error for processing a request.
 */
typedef void (*eip_tr_err_callback_t)(void *app_data, eip_req_t req, int err);

/*
 * eip_dtls_hdr
 *	DTLS headers as per RFC6347.
 */
struct eip_dtls_hdr {
	uint8_t type;		/* Content type */
	__be16 version;		/* DTLS version */
	__be16 epoch;		/* Session epoch value */
	uint8_t seq[6];		/* 48 bit sequence number */
	__be16 length;		/* Payload length */
} __packed;

/*
 * eip_capwap_hdr
 *	CAPWAP header as per RFC5415.
 */
struct eip_capwap_hdr {
	uint8_t preamble;	/* CAPWAP payload type */
	uint8_t reseved[3];	/* Reserved bits */
} __packed;

/*
 * eip_svc
 *	EIP HW Services
 */
enum eip_svc {
	EIP_SVC_NONE = 0,
	EIP_SVC_SKCIPHER,
	EIP_SVC_AHASH,
	EIP_SVC_AEAD,
	EIP_SVC_IPSEC,
	EIP_SVC_HYBRID_IPSEC,
	EIP_SVC_DTLS,
	EIP_SVC_MAX
};

/*
 * eip_ipsec_replay
 *	Supported Anti-replay window size.
 */
enum eip_ipsec_replay {
	EIP_IPSEC_REPLAY_NONE = 0,
	EIP_IPSEC_REPLAY_32,
	EIP_IPSEC_REPLAY_64,
	EIP_IPSEC_REPLAY_128,
	EIP_IPSEC_REPLAY_256,
	EIP_IPSEC_REPLAY_384,
	EIP_IPSEC_REPLAY_MAX
};

/*
 * eip_dtls_replay
 *	Supported Anti-replay window size.
 */
enum eip_dtls_replay {
	EIP_DTLS_REPLAY_NONE = 0,
	EIP_DTLS_REPLAY_64,
	EIP_DTLS_REPLAY_128,
	EIP_DTLS_REPLAY_MAX
};

/*
 * eip_flow_tuple
 *      Filled by caller; Allocate on stack.
 */
struct eip_flow_tuple {
	__be32 src_ip[4];	/* Source IP address */
	__be32 dst_ip[4];	/* Destination IP address */
	__be32 spi;		/* For non-IPsec flow this must be zero */
	__be16 src_port;	/* Source transport layer(L4) port. */
	__be16 dst_port;	/* Destination transport layer(L4) port. */
	__be16 epoch;		/* For non-DTLS flow this must be zero */
	uint8_t ip_ver;		/* IP version. */
	uint8_t ip_proto;	/* IP protocol number. */
};

/*
 * eip_he_stats
 *	EIP stats
 */
struct eip_hw_stats {
	uint64_t pkt_cnt;	/* Number of packet processed */
	uint64_t byte_cnt;	/* Number of bytes processed */
};

/*
 * eip_tr_base
 *	Allocated by user; mostly on stack
 */
struct eip_tr_base {
	enum eip_svc svc;
	struct {
		const char *key_data;
		uint16_t key_len;
		uint8_t blk_len;
	} cipher, auth;

	uint32_t nonce;
	char alg_name[CRYPTO_MAX_ALG_NAME];
};

/*
 * eip_tun_update_info
 *	Tunnel update info.
 */
struct eip_tun_update_info {
#define EIP_TUN_TR_VALID BIT(0)	/* TR is valid in this struct */
	uint32_t valid_flags;	/* Valid flags */
	struct eip_tr *tr;	/* Transform record to be replaced */
	struct eip_tr *new_tr;	/* New transform record */
};

/*
 * eip_flow_info
 *	EIP flow info.
 */
struct eip_flow_info {
	struct eip_flow_tuple tuple;	/* Flow tuple */
	struct eip_tr *tr;	/* Transform record */
};

/*
 * eip_tr_info_crypto
 *	Allocated by user; mostly on stack
 */
struct eip_tr_info_crypto {
	eip_tr_callback_t enc_cb;		/* Encode request callback */
	eip_tr_callback_t dec_cb;		/* Decode request callback */
	eip_tr_callback_t auth_cb;		/* Auth request callback */
	eip_tr_err_callback_t enc_err_cb;	/* Encode request error callback */
	eip_tr_err_callback_t dec_err_cb;	/* Decode request error callback */
	eip_tr_err_callback_t auth_err_cb;	/* Auth request error callback */
	void *app_data;				/* Opaque to pass with callback */
};

/*
 * eip_tr_info_ipsec
 *	Allocated by user; mostly on stack
 */
struct eip_tr_info_ipsec {
	uint32_t sa_flags;			/* IPsec SA flags */
	enum eip_ipsec_replay replay;	/* Anti-replay window size */

	__be32 src_ip[4];	/* Source IP address */
	__be32 dst_ip[4];	/* Destination IP address */
	__be32 xlate_src_ip;	/* Transport mode translated v4 source IP */
	__be32 xlate_dst_ip;	/* Transport mode translated v4 destination IP */
	__be32 spi_idx;		/* ESP SPI Index */
	__be16 src_port;	/* Source port for ESP-over-UDP flow */
	__be16 dst_port;	/* Destination port for ESP-over-UDP flow */

	uint32_t ppe_mdata[4];	/* EIP-PPE meta/bypass information */
	uint8_t ip_ver;		/* Version 4 or 6 */
	uint8_t ip_ttl;		/* Default time-to-live */
	uint8_t ip_df;		/* Default don't fragment bit */
	uint8_t ip_dscp;	/* Default DSCP value, when copy from inner is disabled */
	uint8_t icv_len;	/* Part of digest appended/verified in payload */

	eip_tr_callback_t cb;		/* Transformation completion callback */
	eip_tr_err_callback_t err_cb;	/* Transformation error callback */
	void *app_data;			/* Opaque to pass with callback */
};

/*
 * eip_tr_info_dtls
 *	Allocated by user; mostly on stack
 */
struct eip_tr_info_dtls {
	uint32_t flags;			/* DTLS flags */
#define EIP_TR_DTLS_FLAG_ENC      BIT(0)  /* set = Encapsulation, clear = Decapsulation */
#define EIP_TR_DTLS_FLAG_IPV6     BIT(1)  /* set = IPv6 tuple, clear = IPv4 tuple SA */
#define EIP_TR_DTLS_FLAG_UDPLITE  BIT(2)  /* set = UDPlite, clear = UDP */
#define EIP_TR_DTLS_FLAG_CAPWAP  BIT(3)  /* set = CAPWAP+DTLS, clear = DTLS */
#define EIP_TR_DTLS_FLAG_CP_TOS   BIT(4)  /* set = copy IP TOS value from SKB, clear = use default */
#define EIP_TR_DTLS_FLAG_CP_DF    BIT(5)  /* set = copy IP DF value from SKB, clear = use default  */
#define EIP_TR_DTLS_FLAG_UDPLITE_CSUM  BIT(6)  /* set = Checksum UDP-Lite header, clear = Full payload */

	enum eip_dtls_replay replay;	/* Anti-replay window size */

	__be32 src_ip[4];	/* Source IP address */
	__be32 dst_ip[4];	/* Destination IP address */
	__be16 src_port;	/* Source port for ESP-over-UDP flow */
	__be16 dst_port;	/* Destination port for ESP-over-UDP flow */
	__be16 epoch;		/* DTLS epoch version */
	uint16_t version;	/* DTLS version (0xFEFF/v1.0 or 0xFEFD/v1.2) */

	uint8_t ip_ver;		/* Version 4 or 6 */
	uint8_t ip_ttl;		/* Default time-to-live */
	uint8_t ip_dscp;	/* Default DSCP value, when copy from inner is disabled */
	bool ip_df;		/* Default don't fragment bit */

	eip_tr_callback_t cb;		/* Transformation completion callback */
	eip_tr_err_callback_t err_cb;	/* Transformation error callback */
	void *app_data;			/* Opaque to pass with callback */
};

/*
 * eip_tr_info
 *	Trasform record info allocated by user; mostly on stack
 */
struct eip_tr_info {
	struct eip_tr_base base;	/* Base common information */
	union {
		struct eip_tr_info_crypto crypto;	/* Information require for crypto session */
		struct eip_tr_info_ipsec ipsec;		/* Information require for IPsec session */
		struct eip_tr_info_dtls dtls;		/* Information require for DTLS session */
	};
};

/*
 * eip_tr_algo_info
 * 	Algo specific information.
 */
struct eip_tr_algo_info {
	uint16_t iv_len;	/* IV length for configured algo */
	uint16_t blk_len;	/* Cipher block length for configured algo */
	uint16_t hmac_len;	/* HMAC length for configured algo */
};

/*
 * eip_req2aead_request
 *	eip_req_t to aead_request.
 *
 * @datatypes
 * eip_req_t
 *
 * @param[in] req	Request received in callback.
 *
 * @return
 * aead_request
 */
static inline struct aead_request *eip_req2aead_request(eip_req_t req)
{
	return (struct aead_request *)req;
}

/*
 * eip_req2skcipher_request
 *	eip_req_t to skcipher_request.
 *
 * @datatypes
 * eip_req_t
 *
 * @param[in] req	Request received in callback.
 *
 * @return
 * skcipher_request
 */
static inline struct skcipher_request *eip_req2skcipher_request(eip_req_t req)
{
	return (struct skcipher_request *)req;
}

/*
 * eip_req2ahash_request
 *	eip_req_t to ahash_request
 *
 * @datatypes
 * eip_req_t
 *
 * @param[in] req	Request received in callback.
 *
 * @return
 * ahash_request
 */
static inline struct ahash_request *eip_req2ahash_request(eip_req_t req)
{
	return (struct ahash_request *)req;
}

/*
 * eip_req2skb
 *	eip_req_t to SKB.
 *
 * @datatypes
 * eip_req_t
 *
 * @param[in] req	Request received in callback.
 *
 * @return
 * sk_buff
 */
static inline struct sk_buff *eip_req2skb(eip_req_t req)
{
	return (struct sk_buff *)req;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
/*
 * eip_is_enabled
 *	Check if EIP is enabled.
 *
 * @return
 * true if crypto capabality is available.
 */
static inline bool eip_is_enabled(void)
{
	return cpu_is_nss_crypto_enabled();
}
#else
/*
 * eip_is_enabled
 *	Check if EIP is enabled.
 *
 * @return
 * true if crypto capabality is available.
 */
static inline bool eip_is_enabled(void)
{
	uint32_t soc_id;

	if (qcom_smem_get_soc_id(&soc_id))
		return false;

	return (soc_id == QCOM_ID_IPQ5321 || soc_id == QCOM_ID_IPQ5322 || soc_id == QCOM_ID_IPQ9570 ||
			soc_id == QCOM_ID_IPQ9550 || soc_id == QCOM_ID_IPQ9574 ||
			soc_id == QCOM_ID_IPQ9554 || soc_id == QCOM_ID_IPQ5332 ||
			soc_id == QCOM_ID_IPQ5300 || soc_id == QCOM_ID_IPQ5424 ||
			soc_id == QCOM_ID_IPQ5404 || soc_id == QCOM_ID_IPQ5200 ||
			soc_id == QCOM_ID_IPQ5210 || soc_id == QCOM_ID_IPQ9650 ||
			soc_id == QCOM_ID_IPQ9620 || soc_id == QCOM_ID_QCF3200 ||
			soc_id == QCOM_ID_QCF3210 || soc_id == QCOM_ID_QCF2200);
}
#endif


/*
 * eip_ctx_alloc
 *	EIP context API(s)
 *
 * @datatypes
 * enum eip_svc
 *
 * @param[in] svc	Service for which context needs to allocate.
 * @param[out] dentry	debugfs directory to use for base module.
 *
 * @return
 * eip_ctx
 */
struct eip_ctx *eip_ctx_alloc(enum eip_svc svc, struct dentry **dentry);

/*
 * eip_ctx_free
 *	Free driver context.
 *
 * @datatypes
 * struct eip_ctx
 *
 * @param[in] ctx	Driver context to be freed.
 */
void eip_ctx_free(struct eip_ctx *ctx);

/*
 * eip_tr_alloc
 *	Allocate new hardware transformation record.
 *
 * @datatypes
 * struct eip_ctx \n
 * struct eip_tr_info
 *
 * @param[in] ctx	Driver context associated with record.
 * @param[in] info	Transformation info.
 *
 * @return
 * eip_tr
 */
struct eip_tr *eip_tr_alloc(struct eip_ctx *ctx, struct eip_tr_info *info);

/*
 * eip_tr_disable
 *	Disable transformation record.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 */
void eip_tr_disable(struct eip_tr *tr);

/*
 * eip_tr_free
 *	Free transformation record.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 */
void eip_tr_free(struct eip_tr *tr);

/*
 * eip_tr_get_hw_stats
 *	Get delta of TR packet count and byte counts processed by TR
 *
 * @datatypes
 * struct eip_tr \n
 * struct eip_hw_stats
 *
 * @param[in] tr	Transform record.
 * @param[in] stats	TR stats
 *
 * @return
 */
void eip_tr_get_hw_stats(struct eip_tr *tr, struct eip_hw_stats *stats);

/*
 * eip_tr_skcipher_enc
 *	Encrypt the data using provided TR.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] cipher	Pointer to the skcipher request.
 *
 * @return
 * Enqueue error.
 */
int eip_tr_skcipher_enc(struct eip_tr *tr, struct skcipher_request *cipher);

/*
 * eip_tr_skcipher_dec
 *	Decrypt the data using provided TR.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] cipher	Pointer to the skcipher request.
 *
 * @return
 * Enqueue error.
 */

int eip_tr_skcipher_dec(struct eip_tr *tr, struct skcipher_request *cipher);

/*
 * eip_tr_ahash_auth
 *	Generate hash for data using provided TR.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] hash	Pointer to the ahash request.
 * @param[in] res	Pointer to the result scatterlist.
 *
 * @return
 * Enqueue error.
 */

int eip_tr_ahash_auth(struct eip_tr *tr, struct ahash_request *hash, struct scatterlist *res);

/*
 * eip_tr_aead_enc
 *	Encrypt & append hash for the data using provided TR.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] aead	Pointer to the aead request.
 *
 * @return
 * Enqueue error.
 */
int eip_tr_aead_encauth(struct eip_tr *tr, struct aead_request *aead);

/*
 * eip_tr_aead_dec
 *	Verify hash and decrypt the data using provided TR.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] aead	Pointer to the aead request.
 *
 * @return
 * Enqueue error.
 */
int eip_tr_aead_authdec(struct eip_tr *tr, struct aead_request *aead);

/*
 * eip_tr_dtls_enc
 *	Encapsulate packet with DTLS.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] skb	Pointer to the SKB.
 *
 * @return
 * Enqueue error.
 */
int eip_tr_dtls_enc(struct eip_tr *tr, struct sk_buff *skb);

/*
 * eip_tr_dtls_dec
 *	Decapsulate the packet with DTLS.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] skb	Pointer to the SKB.
 *
 * @return
 * Enqueue error.
 */
int eip_tr_dtls_dec(struct eip_tr *tr, struct sk_buff *skb);

/*
 * eip_tr_ipsec_enc
 *	Encrypt & append hash for the data using provided TR.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] skb	Pointer to the SKB.
 *
 * @return
 * Enqueue error.
 */
int eip_tr_ipsec_enc(struct eip_tr *tr, struct sk_buff *skb);

/*
 * eip_tr_ipsec_dec
 *	Verify hash and decrypt the data using provided TR.
 *
 * @datatypes
 * struct eip_tr
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[in] skb	Pointer to the SKB.
 *
 * @return
 * Enqueue error.
 */
int eip_tr_ipsec_dec(struct eip_tr *tr, struct sk_buff *skb);

/*
 * eip_tr_get_algo_info
 * 	Get algo info.
 *
 * @datatypes
 * struct eip_tr
 * struct eip_tr_algo_info
 *
 * @param[in] tr	Pointer to the transformation record.
 * @param[out] algo	Pointer to the algo information.
 */
void eip_tr_get_algo_info(struct eip_tr *tr, struct eip_tr_algo_info *algo);

/*
 * eip_ctx_algo_supported
 * 	Check if the algorithm is supported by DMA.
 *
 * @datatypes
 * const char *
 *
 * @param[in] dma_ctx  Pointer to DMA context.
 * @param[in] alg_name Algorithm name.
 *
 * @return
 * True/False.
 */
bool eip_ctx_algo_supported(struct eip_ctx *dma_ctx, const char *algo_name);

/*
 * eip_is_inline_supported
 * 	Check if inline port is supported in EIP.
 *
 * @return
 * true/false
 */
bool eip_is_inline_supported(void);

/*
 * eip_tun_update
 *	eip_tun_update
 *
 * @datatypes
 * struct eip_tun
 * struct eip_tun_update_info
 *
 * @param[in] tun Tunnel.
 * @param[in] info Tunnel info
 *
 * @return
 * Return 0 on sucess, else error code.
 */
int eip_tun_update(struct eip_tun *tun, struct eip_tun_update_info *info);

/*
 * eip_tun_alloc
 *	Allocate and initialize tunnel.
 * @datatypes
 * struct eip_ctx
 *
 * @param[in] ctx EIP context
 *
 * @return
 * tun on success, NULL in error case
 */
struct eip_tun *eip_tun_alloc(struct eip_ctx *ctx);

/*
 * eip_tun_free
 *	Free tunnel.
 *
 * @datatypes
 * struct eip_tun
 *
 * @param[in] tun tunnel.
 *
 * @return
 *
 */
void eip_tun_free(struct eip_tun *tun);

/*
 * eip_flow_add
 *	Add flows to EIP HW.
 *
 * @datatypes
 * struct eip_tun
 * struct flow_flow_info
 *
 * @param[in] tun Tunnel context.
 * @param[in] info Flow info.
 *
 * @return
 * 0 on suscess, else NULL on fail.
 */
int eip_flow_add(struct eip_tun *tun, struct eip_flow_info *info);

/*
 * eip_flow_del
 *	Delete flows from EIP HW.
 *
 * @datatypes
 * struct eip_tun
 * struct flow_flow_info
 *
 * @param[in] tun Tunnel context.
 * @param[in] info Flow info.
 * @return
 */
void eip_flow_del(struct eip_tun *tun, struct eip_flow_info *info);

/*
 * eip_feature_check
 *	Check HW features.
 *
 * @datatypes
 * struct eip_ctx
 * eip_features_t
 *
 * @param[in] ctx EIP context
 * @param[in] feature
 *
 * @return
 * true if featur is supported, else false
 */
bool eip_feature_check(struct eip_ctx *ctx, eip_features_t feature);

#endif /* __EIP_H */
