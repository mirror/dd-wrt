/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
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
 * nss_ovpnmgr_crypto.c
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/if.h>
#include <linux/crypto.h>
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/skcipher.h>
#include <crypto/aes.h>
#include <crypto/authenc.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/hash.h>

#include <nss_api_if.h>
#include <nss_cryptoapi.h>
#include <nss_qvpn.h>
#include "nss_ovpnmgr.h"
#include "nss_ovpnmgr_crypto.h"
#include "nss_ovpnmgr_tun.h"
#include "nss_ovpnmgr_app.h"
#include "nss_ovpnmgr_debugfs.h"
#include "nss_ovpnmgr_priv.h"
#include "nss_ovpnmgr_route.h"

/*
 * Linux aead crypto algorithm names.
 */
static const char *nss_ovpnmgr_crypto_algo_name[NSS_OVPNMGR_ALGO_MAX] = {
	"authenc(hmac(sha1),cbc(aes))",
	"authenc(hmac(sha256),cbc(aes))",
	"authenc(hmac(sha1),cbc(des3_ede))",
	"authenc(hmac(sha256),cbc(des3_ede))",
	"hmac(sha1)",
	"hmac(sha256)",
	"cbc(aes)",
	"cbc(des3_ede)",
};

/*
 * nss_ovpnmgr_config_crypto_key_add()
 *	Add crypto key.
 */
static int nss_ovpnmgr_config_crypto_key_add(struct nss_ovpnmgr_tun_ctx *tun_ctx, struct nss_ovpnmgr_tun *tun)
{
	enum nss_qvpn_error_type resp;
	struct nss_qvpn_msg nqm;
	struct nss_qvpn_crypto_key_add_msg *key = &nqm.msg.key_add;
	struct nss_qvpn_crypto_key_activate_msg *key_activate = &nqm.msg.key_activate;
	nss_tx_status_t status;
	uint32_t session_id;

	key->crypto_idx = tun_ctx->active.crypto_idx;

	if (tun->tun_cfg.flags & NSS_OVPNMGR_HDR_FLAG_DATA_V2) {
		session_id = NSS_OVPNMGR_TUN_DATA_V2 << NSS_OVPNMGR_TUN_OPCODE_SHIFT;
		session_id |= tun_ctx->active.key_id << NSS_OVPNMGR_TUN_KEY_ID_SHIFT;
		session_id |= tun->tun_cfg.peer_id & NSS_OVPNMGR_TUN_PEER_ID_MASK;
		session_id = htonl(session_id);
	} else {
		session_id = NSS_OVPNMGR_TUN_DATA_V1 << NSS_OVPNMGR_TUN_OPCODE_SHIFT;
		session_id |= tun_ctx->active.key_id;
	}

	memcpy(key->session_id, &session_id, sizeof(session_id));

	nss_ovpnmgr_info("%px: Active key: session=%d, key_id=%d, crypto_blk_size = %d, hmac_len = %d, iv_len = %d\n",
			tun_ctx, tun_ctx->active.crypto_idx, tun_ctx->active.key_id,
			tun_ctx->active.blk_len, tun_ctx->active.hash_len, tun_ctx->active.iv_len);

	status = nss_qvpn_tx_msg_sync(tun_ctx->nss_ctx,  &nqm, tun_ctx->ifnum, NSS_QVPN_MSG_TYPE_CRYPTO_KEY_ADD,
					sizeof(*key), &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%px: failed to add crypto key, resp = %d, status = %d\n", tun_ctx, resp, status);
		return status == NSS_TX_FAILURE_QUEUE ? -EBUSY : -EINVAL;
	}

	/* Send key activate message. */
	memset(&nqm, 0, sizeof(nqm));
	key_activate->crypto_idx = tun_ctx->active.crypto_idx;
	memcpy(key_activate->vpn_hdr_head, &session_id, sizeof(session_id));

	status = nss_qvpn_tx_msg_sync(tun_ctx->nss_ctx,  &nqm, tun_ctx->ifnum, NSS_QVPN_MSG_TYPE_CRYPTO_KEY_ACTIVATE,
					sizeof(*key_activate), &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%px: failed to activate crypto key, resp = %d, status = %d\n", tun_ctx, resp, status);
		return status == NSS_TX_FAILURE_QUEUE ? -EBUSY : -EINVAL;
	}

	return 0;
}

/*
 * nss_ovpnmgr_config_crypto_key_del()
 *	Delete crypto key.
 */
static int nss_ovpnmgr_config_crypto_key_del(struct nss_ovpnmgr_tun_ctx *tun_ctx)
{
	enum nss_qvpn_error_type resp;
	struct nss_qvpn_msg nqm;
	nss_tx_status_t status;

	/* Send key deactivate message. */
	nqm.msg.key_del.crypto_idx = tun_ctx->expiring.crypto_idx;

	nss_ovpnmgr_info("%px: Expiring key: session=%d, key_id=%d, crypto_blk_size = %d, hmac_len = %d, iv_len = %d\n",
			tun_ctx, tun_ctx->expiring.crypto_idx, tun_ctx->expiring.key_id,
			tun_ctx->expiring.blk_len, tun_ctx->expiring.hash_len, tun_ctx->expiring.iv_len);

	status = nss_qvpn_tx_msg_sync(tun_ctx->nss_ctx,  &nqm, tun_ctx->ifnum, NSS_QVPN_MSG_TYPE_CRYPTO_KEY_DEACTIVATE,
			sizeof(nqm.msg.key_del), &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%px: failed to deactivate crypto key, resp = %d, status = %d\n", tun_ctx, resp, status);
		return status == NSS_TX_FAILURE_QUEUE ? -EBUSY : -EINVAL;
	}

	status = nss_qvpn_tx_msg_sync(tun_ctx->nss_ctx,  &nqm, tun_ctx->ifnum, NSS_QVPN_MSG_TYPE_CRYPTO_KEY_DEL,
			sizeof(nqm.msg.key_del), &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_ovpnmgr_warn("%px: failed to delete crypto key, resp = %d, status = %d\n", tun_ctx, resp, status);
		return status == NSS_TX_FAILURE_QUEUE ? -EBUSY : -EINVAL;
	}
	return 0;
}

/*
 * nss_ovpnmgr_crypto_ctx_alloc_aead()
 *	Creates AEAD crypto context.
 */
static int nss_ovpnmgr_crypto_ctx_alloc_aead(struct nss_ovpnmgr_crypto_ctx *ctx,
					struct nss_ovpnmgr_crypto_config *params,
					struct nss_ovpnmgr_crypto_key *key)
{
	struct crypto_authenc_key_param *key_param;
	enum nss_ovpnmgr_algo algo = params->algo;
	struct rtattr *rta;
	uint32_t session, keylen;
	char *keys, *p;
	int res;

	nss_ovpnmgr_info("%px: Registering AEAD alg %s\n", ctx, nss_ovpnmgr_crypto_algo_name[algo]);

	ctx->tfm.aead = crypto_alloc_aead(nss_ovpnmgr_crypto_algo_name[algo], 0, 0);
	if (IS_ERR(ctx->tfm.aead)) {
		res = PTR_ERR(ctx->tfm.aead);
		nss_ovpnmgr_warn("%px: failed to allocate crypto aead context, res=%d\n", ctx, res);
		return -ENOENT;
	}

	/*
	 * Construct keys
	 */
	keylen = RTA_SPACE(sizeof(*key_param));
	keylen += key->cipher_keylen;
	keylen += key->hmac_keylen;

	keys = vzalloc(keylen);
	if (!keys) {
		nss_ovpnmgr_warn("%px: failed to allocate key memory", ctx);
		crypto_free_aead(ctx->tfm.aead);
		return -ENOMEM;
	}

	p = keys;
	rta = (void *)p;
	rta->rta_type = CRYPTO_AUTHENC_KEYA_PARAM;
	rta->rta_len = RTA_LENGTH(sizeof(*key_param));
	key_param = RTA_DATA(rta);
	p += RTA_SPACE(sizeof(*key_param));

	/*
	 * Copy authentication key
	 */
	memcpy(p, key->hmac_key, key->hmac_keylen);
	p += key->hmac_keylen;

	/*
	 * Copy cipher Key
	 */
	key_param->enckeylen = cpu_to_be32(key->cipher_keylen);
	memcpy(p, key->cipher_key, key->cipher_keylen);

	res = crypto_aead_setkey(ctx->tfm.aead, keys, keylen);
	if (res) {
		nss_ovpnmgr_warn("%px: failed to configure keys, res=%d\n", ctx, res);
		vfree(keys);
		crypto_free_aead(ctx->tfm.aead);
		return -EKEYREJECTED;
	}

	nss_cryptoapi_aead_ctx2session(ctx->tfm.aead, &session);

	ctx->crypto_type = NSS_OVPNMGR_CRYPTO_TYPE_AEAD;
	ctx->blk_len = (uint8_t)crypto_aead_blocksize(ctx->tfm.aead);
	ctx->hash_len = (uint8_t)crypto_aead_authsize(ctx->tfm.aead);
	ctx->iv_len = (uint8_t)crypto_aead_ivsize(ctx->tfm.aead);
	ctx->crypto_idx = (uint16_t)session;

	nss_ovpnmgr_info("%px: blk_len=%u, hash_len=%u, iv_len=%u, session=%d\n",
			ctx, ctx->blk_len, ctx->hash_len, ctx->iv_len, session);

	vfree(keys);
	return 0;
}

/*
 * nss_ovpnmgr_crypto_ctx_alloc_ablk()
 *	Creates ABLK crypto context.
 */
static int nss_ovpnmgr_crypto_ctx_alloc_ablk(struct nss_ovpnmgr_crypto_ctx *ctx,
		struct nss_ovpnmgr_crypto_config *params,
		struct nss_ovpnmgr_crypto_key *key)
{
	struct crypto_skcipher *skcipher;
	enum nss_ovpnmgr_algo algo = params->algo;
	uint32_t session;
	int res;

	nss_ovpnmgr_info("%px: Registering ABLK alg %s\n", ctx, nss_ovpnmgr_crypto_algo_name[algo]);

	skcipher = crypto_alloc_skcipher(nss_ovpnmgr_crypto_algo_name[algo], 0, 0);
	if (IS_ERR(skcipher)) {
		res = PTR_ERR(skcipher);
		nss_ovpnmgr_warn("%px: failed to allocate crypto aead context, res = %d\n", ctx, res);
		return -ENOENT;
	}

	res = crypto_skcipher_setkey(skcipher, key->cipher_key, key->cipher_keylen);
	if (res) {
		nss_ovpnmgr_warn("%px: failed to set skcipher key, res=%d", ctx, res);
		crypto_free_skcipher(skcipher);
		return -EKEYREJECTED;
	}

	nss_cryptoapi_skcipher_ctx2session(skcipher, &session);

	ctx->crypto_type = NSS_OVPNMGR_CRYPTO_TYPE_ABLK;
	ctx->blk_len = (uint8_t)crypto_skcipher_blocksize(skcipher);
	ctx->iv_len = (uint8_t)crypto_skcipher_ivsize(skcipher);
	ctx->tfm.skcipher = skcipher;
	ctx->crypto_idx = (uint16_t)session;

	nss_ovpnmgr_info("%px: skcipher Registered: blk_len=%u, iv_len=%u, session=%u\n",
			ctx, ctx->blk_len, ctx->iv_len, session);
	return 0;
}

/*
 * nss_ovpnmgr_crypto_ctx_alloc_ahash()
 *	Creates ABLK crypto context.
 */
static int nss_ovpnmgr_crypto_ctx_alloc_ahash(struct nss_ovpnmgr_crypto_ctx *ctx,
		struct nss_ovpnmgr_crypto_config *params,
		struct nss_ovpnmgr_crypto_key *key)
{
	enum nss_ovpnmgr_algo algo = params->algo;
	struct crypto_ahash *ahash;
	uint32_t session;
	int res;

	nss_ovpnmgr_info("%px: Registering AHASH alg %s\n", ctx, nss_ovpnmgr_crypto_algo_name[algo]);

	ahash = crypto_alloc_ahash(nss_ovpnmgr_crypto_algo_name[algo], 0, 0);
	if (IS_ERR(ahash)) {
		res = PTR_ERR(ahash);
		nss_ovpnmgr_warn("%px: failed to allocate ahash context, res = %d", ctx, res);
		return -ENOENT;
	}

	res = crypto_ahash_setkey(ahash, key->hmac_key, key->hmac_keylen);
	if (res) {
		nss_ovpnmgr_warn("%px: failed to set ahash key, res=%d", ctx, res);
		crypto_free_ahash(ahash);
		return -EKEYREJECTED;
	}

	nss_cryptoapi_ahash_ctx2session(ahash, &session);

	ctx->crypto_type = NSS_OVPNMGR_CRYPTO_TYPE_AHASH;
	ctx->tfm.ahash = ahash;
	ctx->hash_len = crypto_ahash_digestsize(ahash);
	ctx->crypto_idx = (uint16_t)session;

	nss_ovpnmgr_info("%px: AHASH Registered: hash_len=%u, session=%u\n", ctx, ctx->hash_len, session);
	return 0;
}

/*
 * nss_ovpnmgr_crypto_ctx_free()
 *	Unregisters crypto context.
 */
void nss_ovpnmgr_crypto_ctx_free(struct nss_ovpnmgr_crypto_ctx *ctx)
{
	switch (ctx->crypto_type) {
	case NSS_OVPNMGR_CRYPTO_TYPE_AEAD:
		crypto_free_aead(ctx->tfm.aead);
		ctx->tfm.aead = NULL;
		break;
	case NSS_OVPNMGR_CRYPTO_TYPE_ABLK:
		crypto_free_skcipher(ctx->tfm.skcipher);
		ctx->tfm.skcipher = NULL;
		break;
	case NSS_OVPNMGR_CRYPTO_TYPE_AHASH:
		crypto_free_ahash(ctx->tfm.ahash);
		ctx->tfm.ahash = NULL;
		break;
	}
}

/*
 * nss_ovpnmgr_crypto_ctx_alloc()
 *	Registers crypto context based on crypto_type.
 */
int nss_ovpnmgr_crypto_ctx_alloc(struct nss_ovpnmgr_crypto_ctx *ctx,
				struct nss_ovpnmgr_crypto_config *params,
				struct nss_ovpnmgr_crypto_key *key)
{
	switch (params->algo) {
	/* AEAD */
	case NSS_OVPNMGR_ALGO_AES_CBC_SHA1_HMAC:
	case NSS_OVPNMGR_ALGO_AES_CBC_SHA256_HMAC:
	case NSS_OVPNMGR_ALGO_3DES_CBC_SHA1_HMAC:
	case NSS_OVPNMGR_ALGO_3DES_CBC_SHA256_HMAC:
		return nss_ovpnmgr_crypto_ctx_alloc_aead(ctx, params, key);
	/* AHASH */
	case NSS_OVPNMGR_ALGO_NULL_CIPHER_SHA1_HMAC:
	case NSS_OVPNMGR_ALGO_NULL_CIPHER_SHA256_HMAC:
		return nss_ovpnmgr_crypto_ctx_alloc_ahash(ctx, params, key);
	/* ABLK */
	case NSS_OVPNMGR_ALGO_AES_CBC_NULL_AUTH:
	case NSS_OVPNMGR_ALGO_3DES_CBC_NULL_AUTH:
		return nss_ovpnmgr_crypto_ctx_alloc_ablk(ctx, params, key);
	case NSS_OVPNMGR_ALGO_NULL_CIPHER_NULL_AUTH:
		nss_ovpnmgr_info("%px: Cipher is none. Auth is none\n", ctx);
		ctx->tfm.aead = NULL;
		ctx->crypto_idx = U16_MAX;
		break;
	default:
		nss_ovpnmgr_info("%px: Crypto algorithm is invalid. algo=%d\n", ctx, params->algo);
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_ovpnmgr_crypto_key_del()
 *	Delete crypto keys.
 */
int nss_ovpnmgr_crypto_key_del(uint32_t tunnel_id)
{
	struct nss_ovpnmgr_crypto_ctx *ctx_inner;
	struct nss_ovpnmgr_crypto_ctx *ctx_outer;
	struct nss_ovpnmgr_tun *tun;
	struct net_device *tun_dev;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel: tunnel_id = %u\n\n", tunnel_id);
		return -EINVAL;
	}

	tun = netdev_priv(tun_dev);

	write_lock_bh(&ovpnmgr_ctx.lock);
	ctx_inner = &tun->inner.expiring;
	if (!ctx_inner->tfm.aead) {
		/*
		 * Expiring key is not valid, return.
		 */
		write_unlock_bh(&ovpnmgr_ctx.lock);
		dev_put(tun_dev);
		return -EINVAL;
	}

	ctx_outer = &tun->outer.expiring;
	if (unlikely(!ctx_outer->tfm.aead)) {
		/*
		 * Expiring key is not valid, return.
		 */
		write_unlock_bh(&ovpnmgr_ctx.lock);
		dev_put(tun_dev);
		return -EINVAL;
	}

	write_unlock_bh(&ovpnmgr_ctx.lock);

	/*
	 * When crypto key delete request is received, it is always
	 * expiring key that is deleted.  Active key is deleted only during
	 * tunnel delete request.
	 */

	/*
	 * Deregister Encryption
	 */
	nss_ovpnmgr_crypto_ctx_free(ctx_inner);

	/* Send Crypto Key delete message to NSS firmware. */
	if (nss_ovpnmgr_config_crypto_key_del(&tun->inner)) {
		nss_ovpnmgr_warn("%px: Failed to update crypto key\n", ctx_inner);
	}

	/* Deregister Decryption */
	nss_ovpnmgr_crypto_ctx_free(ctx_outer);

	/* Send Crypto Key delete message to NSS firmware. */
	if (nss_ovpnmgr_config_crypto_key_del(&tun->outer)) {
		nss_ovpnmgr_warn("%px: Failed to update crypto key\n", ctx_outer);
	}

	dev_put(tun_dev);
	return 0;
}
EXPORT_SYMBOL(nss_ovpnmgr_crypto_key_del);

/*
 * nss_ovpnmgr_crypto_key_add()
 *	Add crypto key.
 */
int nss_ovpnmgr_crypto_key_add(uint32_t tunnel_id, uint8_t key_id, struct nss_ovpnmgr_crypto_config *cfg)
{
	struct nss_ovpnmgr_crypto_ctx encrypt;
	struct nss_ovpnmgr_crypto_ctx decrypt;
	struct nss_ovpnmgr_tun *tun;
	struct net_device *tun_dev;
	int ret;

	tun_dev = dev_get_by_index(&init_net, tunnel_id);
	if (!tun_dev) {
		nss_ovpnmgr_warn("Couldn't find tunnel: tunnel_id = %u\n\n", tunnel_id);
		return -EINVAL;
	}

	tun = netdev_priv(tun_dev);

	/*
	 * Check if crypto keys are negotiated during rekey.
	 * Otherwise crypto context is allocated when tunnel is added
	 */
	if (key_id) {
		/* Register Encryption */
		ret = nss_ovpnmgr_crypto_ctx_alloc(&encrypt, cfg, &cfg->encrypt);
		if (ret) {
			nss_ovpnmgr_warn("%px: Failed to register Encryption session\n", tun);
			dev_put(tun_dev);
			return ret;
		}

		/* Register Decryption */
		ret = nss_ovpnmgr_crypto_ctx_alloc(&decrypt, cfg, &cfg->decrypt);
		if (ret) {
			/* Deregister Encryption here */
			nss_ovpnmgr_warn("%px: Failed to register Decryption session\n", tun);
			goto free_encrypt;
		}

		encrypt.key_id = key_id;
		decrypt.key_id = key_id;

		read_lock_bh(&ovpnmgr_ctx.lock);
		/* copy old active key into expiring key */
		memcpy(&tun->inner.expiring, &tun->inner.active, sizeof(tun->inner.active));
		memcpy(&tun->outer.expiring, &tun->outer.active, sizeof(tun->outer.active));

		/* copy the new key as active key */
		memcpy(&tun->inner.active, &encrypt, sizeof(encrypt));
		memcpy(&tun->outer.active, &decrypt, sizeof(decrypt));
		read_unlock_bh(&ovpnmgr_ctx.lock);
	} else {
		/*
		 * Initial crypto context is created during tunnel configuration.
		 * copy crypto context for processing.
		 * We will explicitly reset expiring key to 0.
		 */
		read_lock_bh(&ovpnmgr_ctx.lock);
		memset(&tun->inner.expiring, 0, sizeof(tun->inner.expiring));
		memset(&tun->outer.expiring, 0, sizeof(tun->outer.expiring));

		memcpy(&encrypt, &tun->inner.active, sizeof(encrypt));
		memcpy(&decrypt, &tun->outer.active, sizeof(decrypt));
		read_unlock_bh(&ovpnmgr_ctx.lock);
	}

	/*
	 * Send crypto key addition command to inner node.
	 */
	if (nss_ovpnmgr_config_crypto_key_add(&tun->inner, tun)) {
		nss_ovpnmgr_warn("%px: Failed to update inner crypto key\n", tun);
		ret = -EIO;
		goto free_decrypt;
	}

	/*
	 * Send crypto key addition command to outer node.
	 */
	if (nss_ovpnmgr_config_crypto_key_add(&tun->outer, tun)) {
		nss_ovpnmgr_warn("%px: Failed to update outer crypto key\n", tun);
		ret = -EIO;
		goto free_decrypt;
	}

	dev_put(tun_dev);
	return 0;

free_decrypt:
	nss_ovpnmgr_crypto_ctx_free(&decrypt);

free_encrypt:
	nss_ovpnmgr_crypto_ctx_free(&encrypt);

	dev_put(tun_dev);
	return ret;
}
EXPORT_SYMBOL(nss_ovpnmgr_crypto_key_add);
