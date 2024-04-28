/*
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
 */

#include <linux/skbuff.h>
#include "nss_crypto_defines.h"
#include "nss_crypto_api.h"

#ifndef __NSS_CRYPTO_HDR_H
#define __NSS_CRYPTO_HDR_H

#define NSS_CRYPTO_CMN_HDR_VERSION 0x10
#define NSS_CRYPTO_CMN_HDR_MAGIC 0x8897

#define NSS_CRYPTO_FRAG_FLAGS_FIRST 0x1
#define NSS_CRYPTO_FRAG_FLAGS_LAST 0x2

#if defined(NSS_CRYPTO_DEBUG)
#define NSS_CRYPTO_VERIFY_HDR(ch, len) BUG_ON(ch->tot_len < len)
#else
#define NSS_CRYPTO_VERIFY_HDR(ch, len)
#endif

/**
 * nss_crypto_frag
 *	structure to describe a crypto fragment
 */
struct nss_crypto_frag {
	uint32_t addr;			/**< starting address */
	uint16_t len;			/**< length of entries */
	uint16_t flags;			/**< flags specific to address */
};

/*
 * nss_crypto_hdr
 *	crypto metadata
 *
 * +-----------------------+
 * |                       |
 * |  common metadata      |
 * |                       |
 * +-----------------------+
 * |                       |
 * |  Fragment list        |
 * |                       |
 * +-----------------------+
 * |                       |
 * |  IV[0 ... 3]          |
 * |                       |
 * +-----------------------+
 * |                       |
 * |  Hash[0 ... 5]        |
 * |                       |
 * +-----------------------+
 * |                       |
 * |  private              |
 * |                       |
 * +-----------------------+
 *
 * We want to keep the size the expandable, it will allow having the size of the
 * metadata for best case scenarios optimal for usage. Having fixed size arrays
 * pose challenge in terms of potential use cases where the no. of fragments are huge
 *
 * Best case size estimate
 * - Common Metadata = 32 bytes
 * - two fragments = 32 bytes
 * - IV as address = 8 - 16 bytes
 * - Hash = 12 - 20 bytes
 * - private = N bytes
 *
 * Note: data length excludes hmac_len for encrypt, and hash will
 * be produced in 2nd fragment for host generated crypto
 */
struct nss_crypto_hdr {
	uint8_t version;			/**< Version number */
	uint8_t skip;				/**< skip bytes from start */
	uint8_t auth;				/**< authenticate only len */
	uint8_t res1;				/**< reserved */

	uint16_t index;				/**< crypto index */
	uint8_t res2[2];				/**< reserved */

	uint8_t op;				/**< operation direction */
	uint8_t error;				/**< error */
	uint8_t hmac_len;			/**< hmac/icv length */
	uint8_t iv_len;				/**< iv len */

	uint16_t data_len;			/**< total length */
	uint8_t in_frags;			/**< number of in fragments */
	uint8_t out_frags;			/**< number of out fragments */

	uint16_t in_frag_start;			/**< input frags */
	uint16_t out_frag_start;		/**< output frags */

	uint16_t iv_start;			/**< IV offset in header */
	uint16_t hmac_start;			/**< hmac offset in header */

	uint16_t priv_start;			/**< priv is at ch + priv_start> */
	uint16_t priv_len;			/**< private data len */

	uint16_t buf_start;			/**< buf specific information */
	uint16_t buf_len;			/**< buffer structure length */

	uint32_t hw_status[2];			/* hw status information in case of error */

	uint16_t tot_len;			/** fixed header length + variable payload length */
	uint16_t magic;				/**< Magic number */

	uint8_t user_data[];			/**< user data */
};

/**
 * nss_crypto_buf
 *	struture hold information in crypto rx path
 */
struct nss_crypto_buf {
	struct sk_buff *skb;		/**< Skb associated with the transform */
	struct nss_crypto_user *user;	/**< NSS crypto handle */
	nss_crypto_req_callback_t comp;	/**< request complete callback */
	void *app_data;			/**< app data */
	bool mapped;			/**< buffer is mapped */
};

/**
 * nss_crypto_hdr_alloc
 *	allocate a crypto header for submitting transforms
 *
 * @datatype
 * nss_crypto_user
 *
 * @param[in] user user handler
 * @param[in] num_frags no. of scatter/gather fragments
 * @param[in] iv_len length of the IV
 * @param[in] hmac_len length of HMAC
 * @param[in] ahash AHASH mode or not
 *
 * @return
 * Pointer to the start of the header.
 */
struct nss_crypto_hdr *nss_crypto_hdr_alloc(struct nss_crypto_user *user, uint32_t session,
				uint8_t num_frags, uint8_t iv_len, uint8_t hmac_len, bool ahash);

/**
 * nss_crypto_hdr_free
 *	free an allocated crypto header
 *
 * datatype
 * nss_crypto_user
 *
 * @param[in] user crypto user handle
 * @param[in] ch crypto header
 *
 * @return
 * None.
 */
void nss_crypto_hdr_free(struct nss_crypto_user *user, struct nss_crypto_hdr *ch);

/*
 * nss_crypto_hdr_map_sglist
 *	map an SGLIST into header fragments
 *
 * @datatype
 * nss_crypto_hdr
 * scatterlist
 *
 * @param[in] ch crypto header
 * @param[in] src SGLIST list
 * @param[in] auth_len authentication length for the packet

 * @return
 * None.
 */
void nss_crypto_hdr_map_sglist(struct nss_crypto_hdr *ch, struct scatterlist *src, uint16_t auth_len);

/*
 * nss_crypto_hdr_map_sglist_ahash
 *	map an HASH/HMAC SGLIST into header fragments
 *
 * @datatype
 * nss_crypto_hdr
 * scatterlist
 *
 * @param[in] ch crypto header
 * @param[in] src SGLIST list
 * @param[in] auth_len authentication length for the packet

 * @return
 * None.
 */
void nss_crypto_hdr_map_sglist_ahash(struct nss_crypto_hdr *ch, struct scatterlist *src, uint16_t auth_len);

/*
 * nss_crypto_hdr_map_skb
 *	map cryptop into header fragments
 *
 * @note: this api is used with OCF, where Klips
 * stack give a data pointer which needs to be
 * mapped into crypto header fragments.
 *
 * @datatype
 * nss_crypto_hdr
 * skbuff
 *
 * @param[in] ch crypto header
 * @param[in] skb skb buffer
 *
 * @return
 * None.
 */
void nss_crypto_hdr_map_skb(struct nss_crypto_hdr *ch, struct sk_buff *skb);

/**
 * nss_crypto_hdr_set_skip
 *	set cipher skip
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 * @param[in] skip cipher skip
 *
 * @return
 * None.
 */
static inline void nss_crypto_hdr_set_skip(struct nss_crypto_hdr *ch, uint8_t skip)
{
	ch->skip = skip;
	ch->auth = skip;
}

/**
 * nss_crypto_hdr_set_auth
 *	set auth only len
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 * @param[in] auth auth len
 *
 * @return
 * None.
 */
static inline void nss_crypto_hdr_set_auth(struct nss_crypto_hdr *ch, uint8_t auth)
{
	ch->auth = auth;
}

/**
 * nss_crypto_hdr_set_op
 *	set operation type
 *
 * @datatype
 * nss_crypto_hdr
 * nss_crypto_op_dir
 *
 * @param[in] ch crypto header
 * @param[in] op operation code
 *
 * @return
 * None.
 */
static inline void nss_crypto_hdr_set_op(struct nss_crypto_hdr *ch, enum nss_crypto_op_dir op)
{
	ch->op = op;
}

/**
 * nss_crypto_hdr_get_error
 *	get the crypto transform error value
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Error status of transform request.
 */
static inline enum nss_crypto_cmn_resp_error nss_crypto_hdr_get_error(struct nss_crypto_hdr *ch)
{
	return ch->error;
}

/**
 * nss_crypto_hdr_get_hmac_len
 *	get the hmac length
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Hmac length.
 */
static inline uint8_t nss_crypto_hdr_get_hmac_len(struct nss_crypto_hdr *ch)
{
	return ch->hmac_len;
}

/**
 * nss_crypto_hdr_get_in_frag
 *	get start of the input fragment
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Start of input fragment.
 */
static inline struct nss_crypto_frag *nss_crypto_hdr_get_in_frag(struct nss_crypto_hdr *ch)
{
	NSS_CRYPTO_VERIFY_HDR(ch, ch->in_frag_start + sizeof(struct nss_crypto_frag));
	return (struct nss_crypto_frag *)((uint8_t *)ch + ch->in_frag_start);
}

/**
 * nss_crypto_hdr_get_out_frag
 *	get start of out fragment
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Start of out fragment.
 */
static inline struct nss_crypto_frag *nss_crypto_hdr_get_out_frag(struct nss_crypto_hdr *ch)
{
	NSS_CRYPTO_VERIFY_HDR(ch, ch->out_frag_start + sizeof(struct nss_crypto_frag));
	return (struct nss_crypto_frag *)((uint8_t *)ch + ch->out_frag_start);
}

/**
 * nss_crypto_hdr_get_iv
 *	get start of IV
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Start of IV.
 */
static inline uint32_t *nss_crypto_hdr_get_iv(struct nss_crypto_hdr *ch)
{
	NSS_CRYPTO_VERIFY_HDR(ch, ch->iv_start + ch->iv_len);
	return (uint32_t *)((uint8_t *)ch + ch->iv_start);
}

/**
 * nss_crypto_hdr_get_hmac
 *	get start of HMAC
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Start of HMAC.
 */
static inline uint8_t *nss_crypto_hdr_get_hmac(struct nss_crypto_hdr *ch)
{
	NSS_CRYPTO_VERIFY_HDR(ch, ch->hmac_start + ch->hmac_len);
	return (uint8_t *)((uint8_t *)ch + ch->hmac_start);
}

/**
 * nss_crypto_hdr_get_buf
 *	get the start buf structure
 *
 * @datatype
 *	nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Start of buffer.
 */
static inline struct nss_crypto_buf *nss_crypto_hdr_get_buf(struct nss_crypto_hdr *ch)
{
	NSS_CRYPTO_VERIFY_HDR(ch, ch->buf_start + sizeof(struct nss_crypto_buf));
	return (struct nss_crypto_buf *)((uint8_t *)ch + ch->buf_start);
}

/**
 * nss_crypto_hdr_set_tot_len
 *	set total length of the transform
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 * @param[in] tot_len total length of data
 *
 * @return
 * None.
 */
static inline void nss_crypto_hdr_set_tot_len(struct nss_crypto_hdr *ch, uint16_t tot_len)
{
	ch->tot_len = tot_len;
}

/**
 * nss_crypto_hdr_get_tot_len
 *	get total length of data
 *
 * @datatype
 * nss_crypto_hdr
 *
 * @param[in] ch crypto header
 *
 * @return
 * Total length of data.
 */
static inline uint16_t nss_crypto_hdr_get_tot_len(struct nss_crypto_hdr *ch)
{
	return ch->tot_len;
}
#endif /* !__NSS_CRYPTO_HDR_H */
