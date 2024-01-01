// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#include "crypto_aead.h"
#include "crypto.h"
#include "pktid.h"
#include "proto.h"
#include "skb.h"

#include <crypto/aead.h>
#include <linux/skbuff.h>
#include <linux/printk.h>

#define AUTH_TAG_SIZE	16
#define MAX_IV_SIZE	16

static int ovpn_aead_encap_overhead(const struct ovpn_crypto_key_slot *ks)
{
	return  OVPN_OP_SIZE_V2 +			/* OP header size */
		4 +					/* Packet ID */
		crypto_aead_authsize(ks->encrypt);	/* Auth Tag */
}

static u8 *ovpn_aead_iv_set(struct ovpn_crypto_key_slot *ks, u8 iv[MAX_IV_SIZE])
{
	u8 *iv_ptr;

	/* In AEAD mode OpenVPN creates a nonce of 12 bytes made up of 4 bytes packet ID
	 * concatenated with 8 bytes key material coming from userspace.
	 *
	 * For AES-GCM and CHACHAPOLY the IV is 12 bytes and coincides with the OpenVPN nonce.
	 * For AES-CCM the IV is 16 bytes long and is constructed following the instructions
	 * in RFC3610
	 */

	switch(ks->alg) {
	case OVPN_CIPHER_ALG_AES_GCM:
		BUILD_BUG_ON(MAX_IV_SIZE < 12);

		iv_ptr = iv;
		break;
	case OVPN_CIPHER_ALG_CHACHA20_POLY1305:
		BUILD_BUG_ON(MAX_IV_SIZE < 12);

		iv_ptr = iv;
		break;
	case OVPN_CIPHER_ALG_AES_CCM:
	case OVPN_CIPHER_ALG_AES_CBC:
		BUILD_BUG_ON(MAX_IV_SIZE < 16);

		/* For CCM and CBC, the API expects a 16-byte IV.
		 *
		 * The first octect contains L' as per RFC3610, which is the size of the length
		 * field in octects minus 1.
		 *
		 * Since we have a nonce of 12bytes, we can allocate 3 octects for the length field,
		 * that are octects 13, 14 and 15 of iv[].
		 * In octects [1..12] we copy the OpenVPN nonce that will be used by the algorithm.
		 */
		iv[0] = 2;
		iv_ptr = iv + 1;
		break;
	default:
		return NULL;
	}

	return iv_ptr;
}

int ovpn_aead_encrypt(struct ovpn_crypto_key_slot *ks, struct sk_buff *skb, u32 peer_id)
{
	const unsigned int tag_size = crypto_aead_authsize(ks->encrypt);
	const unsigned int head_size = ovpn_aead_encap_overhead(ks);
	struct scatterlist sg[MAX_SKB_FRAGS + 2];
	u8 *iv_ptr, iv[MAX_IV_SIZE] = { 0 };
	DECLARE_CRYPTO_WAIT(wait);
	struct aead_request *req;
	struct sk_buff *trailer;
	int nfrags, ret;
	u32 pktid, op;

	/* Sample AEAD header format:
	 * 48000001 00000005 7e7046bd 444a7e28 cc6387b1 64a4d6c1 380275a...
	 * [ OP32 ] [seq # ] [             auth tag            ] [ payload ... ]
	 *          [4-byte
	 *          IV head]
	 */

	/* check that there's enough headroom in the skb for packet
	 * encapsulation, after adding network header and encryption overhead
	 */
	if (unlikely(skb_cow_head(skb, OVPN_HEAD_ROOM + head_size)))
		return -ENOBUFS;

	/* get number of skb frags and ensure that packet data is writable */
	nfrags = skb_cow_data(skb, 0, &trailer);
	if (unlikely(nfrags < 0))
		return nfrags;

	if (unlikely(nfrags + 2 > ARRAY_SIZE(sg)))
		return -ENOSPC;

	req = aead_request_alloc(ks->encrypt, GFP_KERNEL);
	if (unlikely(!req))
		return -ENOMEM;

	/* sg table:
	 * 0: op, wire nonce (AD, len=OVPN_OP_SIZE_V2+NONCE_WIRE_SIZE),
	 * 1, 2, 3, ..., n: payload,
	 * n+1: auth_tag (len=tag_size)
	 */
	sg_init_table(sg, nfrags + 2);

	/* build scatterlist to encrypt packet payload */
	ret = skb_to_sgvec_nomark(skb, sg + 1, 0, skb->len);
	if (unlikely(nfrags != ret)) {
		ret = -EINVAL;
		goto free_req;
	}

	/* append auth_tag onto scatterlist */
	__skb_push(skb, tag_size);
	sg_set_buf(sg + nfrags + 1, skb->data, tag_size);

	/* obtain packet ID, which is used both as a first
	 * 4 bytes of nonce and last 4 bytes of associated data.
	 */
	ret = ovpn_pktid_xmit_next(&ks->pid_xmit, &pktid);
	if (unlikely(ret < 0))
		return ret;

	/* construct the nonce by combining pkt ID and userspace data.
	 * iv_ptr is the address where the nonce has to start.
	 */
	iv_ptr = ovpn_aead_iv_set(ks, iv);
	if (unlikely(!iv_ptr)) {
		ret = -EOPNOTSUPP;
		goto free_req;
	}
	ovpn_pktid_aead_write(pktid, &ks->nonce_tail_xmit, iv_ptr);

	/* make space for packet id and push it to the front */
	__skb_push(skb, NONCE_WIRE_SIZE);
	memcpy(skb->data, iv_ptr, NONCE_WIRE_SIZE);

	/* add packet op as head of additional data */
	op = ovpn_opcode_compose(OVPN_DATA_V2, ks->key_id, peer_id);
	__skb_push(skb, OVPN_OP_SIZE_V2);
	BUILD_BUG_ON(sizeof(op) != OVPN_OP_SIZE_V2);
	*((__force __be32 *)skb->data) = htonl(op);

	/* AEAD Additional data */
	sg_set_buf(sg, skb->data, OVPN_OP_SIZE_V2 + NONCE_WIRE_SIZE);

	/* setup async crypto operation */
	aead_request_set_tfm(req, ks->encrypt);
	aead_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG |
				       CRYPTO_TFM_REQ_MAY_SLEEP,
				  crypto_req_done, &wait);
	aead_request_set_crypt(req, sg, sg, skb->len - head_size, iv);
	aead_request_set_ad(req, OVPN_OP_SIZE_V2 + NONCE_WIRE_SIZE);

	/* encrypt it */
	ret = crypto_wait_req(crypto_aead_encrypt(req), &wait);
	if (ret < 0)
		net_err_ratelimited("%s: encrypt failed: %d\n", __func__, ret);

free_req:
	aead_request_free(req);
	return ret;
}

int ovpn_aead_decrypt(struct ovpn_crypto_key_slot *ks, struct sk_buff *skb)
{
	const unsigned int tag_size = crypto_aead_authsize(ks->decrypt);
	u8 *sg_data, *iv_ptr, iv[MAX_IV_SIZE] = { 0 };
	struct scatterlist sg[MAX_SKB_FRAGS + 2];
	int ret, payload_len, nfrags;
	unsigned int payload_offset;
	DECLARE_CRYPTO_WAIT(wait);
	struct aead_request *req;
	struct sk_buff *trailer;
	unsigned int sg_len;
	__be32 *pid;

	payload_offset = OVPN_OP_SIZE_V2 + NONCE_WIRE_SIZE + tag_size;
	payload_len = skb->len - payload_offset;

	/* sanity check on packet size, payload size must be >= 0 */
	if (unlikely(payload_len < 0))
		return -EINVAL;

	/* Prepare the skb data buffer to be accessed up until the auth tag.
	 * This is required because this area is directly mapped into the sg list.
	 */
	if (unlikely(!pskb_may_pull(skb, payload_offset)))
		return -ENODATA;

	/* get number of skb frags and ensure that packet data is writable */
	nfrags = skb_cow_data(skb, 0, &trailer);
	if (unlikely(nfrags < 0))
		return nfrags;

	if (unlikely(nfrags + 2 > ARRAY_SIZE(sg)))
		return -ENOSPC;

	req = aead_request_alloc(ks->decrypt, GFP_KERNEL);
	if (unlikely(!req))
		return -ENOMEM;

	/* sg table:
	 * 0: op, wire nonce (AD, len=OVPN_OP_SIZE_V2+NONCE_WIRE_SIZE),
	 * 1, 2, 3, ..., n: payload,
	 * n+1: auth_tag (len=tag_size)
	 */
	sg_init_table(sg, nfrags + 2);

	/* packet op is head of additional data */
	sg_data = skb->data;
	sg_len = OVPN_OP_SIZE_V2 + NONCE_WIRE_SIZE;
	sg_set_buf(sg, sg_data, sg_len);

	/* build scatterlist to decrypt packet payload */
	ret = skb_to_sgvec_nomark(skb, sg + 1, payload_offset, payload_len);
	if (unlikely(nfrags != ret)) {
		ret = -EINVAL;
		goto free_req;
	}

	/* append auth_tag onto scatterlist */
	sg_set_buf(sg + nfrags + 1, skb->data + sg_len, tag_size);

	iv_ptr = ovpn_aead_iv_set(ks, iv);
	if (unlikely(!iv_ptr)) {
		ret = -EOPNOTSUPP;
		goto free_req;
	}
	/* copy nonce into IV buffer */
	memcpy(iv_ptr, skb->data + OVPN_OP_SIZE_V2, NONCE_WIRE_SIZE);
	memcpy(iv_ptr + NONCE_WIRE_SIZE, ks->nonce_tail_recv.u8,
	       sizeof(struct ovpn_nonce_tail));

	/* setup async crypto operation */
	aead_request_set_tfm(req, ks->decrypt);
	aead_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG |
				       CRYPTO_TFM_REQ_MAY_SLEEP,
				  crypto_req_done, &wait);
	aead_request_set_crypt(req, sg, sg, payload_len + tag_size, iv);

	aead_request_set_ad(req, NONCE_WIRE_SIZE + OVPN_OP_SIZE_V2);

	/* decrypt it */
	ret = crypto_wait_req(crypto_aead_decrypt(req), &wait);
	if (ret < 0) {
		net_err_ratelimited("%s: decrypt failed: %d\n", __func__, ret);
		goto free_req;
	}

	/* PID sits after the op */
	pid = (__force __be32 *)(skb->data + OVPN_OP_SIZE_V2);
	ret = ovpn_pktid_recv(&ks->pid_recv, ntohl(*pid), 0);
	if (unlikely(ret < 0))
		goto free_req;

	/* point to encapsulated IP packet */
	__skb_pull(skb, payload_offset);

free_req:
	aead_request_free(req);
	return ret;
}

/* Initialize a struct crypto_aead object */
struct crypto_aead *ovpn_aead_init(const char *title, const char *alg_name,
				   const unsigned char *key, unsigned int keylen)
{
	struct crypto_aead *aead;
	int ret;

	aead = crypto_alloc_aead(alg_name, 0, 0);
	if (IS_ERR(aead)) {
		ret = PTR_ERR(aead);
		pr_err("%s crypto_alloc_aead failed for %s, err=%d\n", title, alg_name, ret);
		aead = NULL;
		goto error;
	}

	ret = crypto_aead_setkey(aead, key, keylen);
	if (ret) {
		pr_err("%s crypto_aead_setkey size=%u failed, err=%d\n", title, keylen, ret);
		goto error;
	}

	ret = crypto_aead_setauthsize(aead, AUTH_TAG_SIZE);
	if (ret) {
		pr_err("%s crypto_aead_setauthsize failed, err=%d\n", title, ret);
		goto error;
	}

	pr_debug("********* Cipher %s (%s)\n", alg_name, title);
	pr_debug("*** IV size=%u\n", crypto_aead_ivsize(aead));
	pr_debug("*** req size=%u\n", crypto_aead_reqsize(aead));
	pr_debug("*** block size=%u\n", crypto_aead_blocksize(aead));
	pr_debug("*** auth size=%u\n", crypto_aead_authsize(aead));
	pr_debug("*** alignmask=0x%x\n", crypto_aead_alignmask(aead));

	return aead;

error:
	crypto_free_aead(aead);
	return ERR_PTR(ret);
}

void ovpn_aead_crypto_key_slot_destroy(struct ovpn_crypto_key_slot *ks)
{
	if (!ks)
		return;

	crypto_free_aead(ks->encrypt);
	crypto_free_aead(ks->decrypt);
	kfree(ks);
}

static struct ovpn_crypto_key_slot *
ovpn_aead_crypto_key_slot_init(enum ovpn_cipher_alg alg,
			       const unsigned char *encrypt_key,
			       unsigned int encrypt_keylen,
			       const unsigned char *decrypt_key,
			       unsigned int decrypt_keylen,
			       const unsigned char *encrypt_nonce_tail,
			       unsigned int encrypt_nonce_tail_len,
			       const unsigned char *decrypt_nonce_tail,
			       unsigned int decrypt_nonce_tail_len,
			       u16 key_id)
{
	struct ovpn_crypto_key_slot *ks = NULL;
	const char *alg_name;
	int ret;

	/* validate crypto alg */
	switch (alg) {
#if IS_ENABLED(CONFIG_CRYPTO_GCM)
	case OVPN_CIPHER_ALG_AES_GCM:
		alg_name = "gcm(aes)";
		break;
#endif
#if IS_ENABLED(CONFIG_CRYPTO_CHACHA20POLY1305)
	case OVPN_CIPHER_ALG_CHACHA20_POLY1305:
		alg_name = "rfc7539(chacha20,poly1305)";
		break;
#endif
#if IS_ENABLED(CONFIG_CRYPTO_CCM)
	case OVPN_CIPHER_ALG_AES_CCM:
		alg_name = "ccm(aes)";
		break;
#endif
#if IS_ENABLED(CONFIG_CRYPTO_CBC)
	case OVPN_CIPHER_ALG_AES_CBC:
		alg_name = "cbc(aes)";
		break;
#endif
	default:
		return ERR_PTR(-EOPNOTSUPP);
	}

	/* build the key slot */
	ks = kmalloc(sizeof(*ks), GFP_KERNEL);
	if (!ks)
		return ERR_PTR(-ENOMEM);

	ks->alg = alg;
	ks->encrypt = NULL;
	ks->decrypt = NULL;
	kref_init(&ks->refcount);
	ks->key_id = key_id;

	ks->encrypt = ovpn_aead_init("encrypt", alg_name, encrypt_key,
				     encrypt_keylen);
	if (IS_ERR(ks->encrypt)) {
		ret = PTR_ERR(ks->encrypt);
		ks->encrypt = NULL;
		goto destroy_ks;
	}

	ks->decrypt = ovpn_aead_init("decrypt", alg_name, decrypt_key,
				     decrypt_keylen);
	if (IS_ERR(ks->decrypt)) {
		ret = PTR_ERR(ks->decrypt);
		ks->decrypt = NULL;
		goto destroy_ks;
	}

	if (sizeof(struct ovpn_nonce_tail) != encrypt_nonce_tail_len ||
	    sizeof(struct ovpn_nonce_tail) != decrypt_nonce_tail_len) {
		ret = -EINVAL;
		goto destroy_ks;
	}

	memcpy(ks->nonce_tail_xmit.u8, encrypt_nonce_tail,
	       sizeof(struct ovpn_nonce_tail));
	memcpy(ks->nonce_tail_recv.u8, decrypt_nonce_tail,
	       sizeof(struct ovpn_nonce_tail));

	/* init packet ID generation/validation */
	ovpn_pktid_xmit_init(&ks->pid_xmit);
	ovpn_pktid_recv_init(&ks->pid_recv);

	return ks;

destroy_ks:
	ovpn_aead_crypto_key_slot_destroy(ks);
	return ERR_PTR(ret);
}

struct ovpn_crypto_key_slot *
ovpn_aead_crypto_key_slot_new(const struct ovpn_key_config *kc)
{
	return ovpn_aead_crypto_key_slot_init(kc->cipher_alg,
					      kc->encrypt.cipher_key,
					      kc->encrypt.cipher_key_size,
					      kc->decrypt.cipher_key,
					      kc->decrypt.cipher_key_size,
					      kc->encrypt.nonce_tail,
					      kc->encrypt.nonce_tail_size,
					      kc->decrypt.nonce_tail,
					      kc->decrypt.nonce_tail_size,
					      kc->key_id);
}
