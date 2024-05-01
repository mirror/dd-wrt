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
 * nss_tlsmgr_crypto.c
 *	NSS TLS Manager crypto object
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/tlshdr.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/crypto.h>
#include <linux/debugfs.h>
#include <linux/rtnetlink.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/atomic.h>

#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/aes.h>
#include <crypto/authenc.h>
#include <crypto/des.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
#include <crypto/sha.h>
#else
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#endif
#include <crypto/skcipher.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include <nss_cryptoapi.h>
#include <nss_tls.h>
#include <nss_tlsmgr.h>

#include "nss_tlsmgr_ctx.h"
#include "nss_tlsmgr_crypto.h"
#include "nss_tlsmgr_tun.h"
#include "nss_tlsmgr_priv.h"

#define NSS_TLSMGR_KEY_PARAM_SIZE RTA_SPACE(sizeof(struct crypto_authenc_key_param))

static struct nss_tlsmgr_algo_info tlsmgr_algo_info[NSS_TLSMGR_ALGO_MAX] = {
	{"cipher_null", NSS_TLSMGR_KEY_PARAM_SIZE},
	{"hmac(sha1)", NSS_TLSMGR_KEY_PARAM_SIZE},
	{"hmac(sha256)", NSS_TLSMGR_KEY_PARAM_SIZE},
	{"authenc(hmac(sha1),cbc(aes))", NSS_TLSMGR_KEY_PARAM_SIZE},
	{"authenc(hmac(sha256),cbc(aes))", NSS_TLSMGR_KEY_PARAM_SIZE},
	{"authenc(hmac(sha1),cbc(des3_ede))", NSS_TLSMGR_KEY_PARAM_SIZE},
	{"authenc(hmac(sha256),cbc(des3_ede))", NSS_TLSMGR_KEY_PARAM_SIZE},
};

/*
 * nss_tlsmgr_crypto_free()
 *	Free the Crypto context.
 */
void nss_tlsmgr_crypto_free(struct nss_tlsmgr_crypto *ntc)
{
	if (ntc->aead)
		crypto_free_aead(ntc->aead);

	if (ntc->ahash)
		crypto_free_ahash(ntc->ahash);

	vfree(ntc);
}

/*
 * nss_tlsmgr_crypto_alloc()
 *	Allocate crypto node and configure crypto
 */
struct nss_tlsmgr_crypto *nss_tlsmgr_crypto_alloc(struct nss_tlsmgr_config *cfg)
{
	struct crypto_authenc_key_param *key_param;
	struct nss_tlsmgr_algo_info *info;
	struct nss_tlsmgr_crypto *ntc;
	struct rtattr *rta;
	char *keys, *p;
	uint16_t keylen;

	ntc = vzalloc(sizeof(*ntc));
	if (!ntc) {
		nss_tlsmgr_warn("failed to allocate crypto data\n");
		return NULL;
	}

	INIT_LIST_HEAD(&ntc->list);

	nss_tlsmgr_trace("algo:%d cipher_keylen:%d auth_keylen:%d nonce_len:%d\n",
				cfg->algo, cfg->cipher_key.len, cfg->auth_key.len, cfg->nonce.len);

	info = &tlsmgr_algo_info[cfg->algo];

	switch (cfg->algo) {
	case NSS_TLSMGR_ALGO_AES_CBC_SHA1_HMAC:
	case NSS_TLSMGR_ALGO_AES_CBC_SHA256_HMAC:
	case NSS_TLSMGR_ALGO_3DES_CBC_SHA1_HMAC:
	case NSS_TLSMGR_ALGO_3DES_CBC_SHA256_HMAC:
		ntc->aead = crypto_alloc_aead(info->name, 0, 0);
		if (IS_ERR(ntc->aead)) {
			nss_tlsmgr_warn("failed to allocate crypto aead context for algorithm(%d)\n", cfg->algo);
			vfree(ntc);
			return NULL;
		}

		/*
		 * Construct keys
		 */
		keylen = info->rta_key_sz;
		keylen += cfg->cipher_key.len;
		keylen += cfg->auth_key.len;
		keylen += cfg->nonce.len;

		keys = vzalloc(keylen);
		if (!keys) {
			nss_tlsmgr_warn("failed to allocate key memory\n");
			crypto_free_aead(ntc->aead);
			vfree(ntc);
			return NULL;
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
		memcpy(p, cfg->auth_key.data, cfg->auth_key.len);
		p += cfg->auth_key.len;

		/*
		 * Copy cipher Key
		 */
		key_param->enckeylen = cpu_to_be32(cfg->cipher_key.len);
		memcpy(p, cfg->cipher_key.data, cfg->cipher_key.len);

		if (crypto_aead_setkey(ntc->aead, keys, keylen)) {
			nss_tlsmgr_warn("failed to configure keys");
			crypto_free_aead(ntc->aead);
			vfree(keys);
			vfree(ntc);
			return NULL;
		}

		nss_cryptoapi_aead_ctx2session(ntc->aead, &ntc->crypto_idx);
		ntc->aead = NULL;
		vfree(keys);
		break;

	case NSS_TLSMGR_ALGO_NULL_SHA1_HMAC:
	case NSS_TLSMGR_ALGO_NULL_SHA256_HMAC:
		ntc->ahash = crypto_alloc_ahash(info->name, 0, 0);
		if (IS_ERR(ntc->ahash)) {
			nss_tlsmgr_warn("%px: failed to allocate crypto ahash context\n", ntc);
			vfree(ntc);
			return NULL;
		}

		if (crypto_ahash_setkey(ntc->ahash, cfg->auth_key.data, cfg->auth_key.len)) {
			nss_tlsmgr_warn("%px: failed to configure keys\n", ntc);
			crypto_free_ahash(ntc->ahash);
			vfree(ntc);
			return NULL;
		}

		nss_cryptoapi_ahash_ctx2session(ntc->ahash, &ntc->crypto_idx);
		ntc->ahash = NULL;
		break;

	default:
		nss_tlsmgr_warn("%px: invalid crypto algorithm(%d)\n", ntc, cfg->algo);
		vfree(ntc);
		return NULL;
	}

	nss_tlsmgr_trace("crypto_aead allocated\n");
	return ntc;
}

/*
 * nss_tlsmgr_crypto_update_null()
 *	Update the NULL crypto keys.
 *
 * This is used to create a dummy crypto session when TLS tunnel is created.
 * This session will be in the active list until user do a crypto update/CCS.
 * Once CCS is triggered by the user, then this dummy session will be removed
 * from the list and this new crypto updates will become the new active session.
 */
nss_tlsmgr_status_t nss_tlsmgr_crypto_update_null(struct net_device *dev, struct nss_tlsmgr_ctx *ctx)
{
        enum nss_tls_msg_type msg_type = NSS_TLS_MSG_TYPE_CIPHER_UPDATE;
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);
	struct nss_tls_cipher_update *ntcu;
	struct nss_tlsmgr_crypto *crypto;
	struct nss_tlsmgr_config cfg;
	struct nss_tls_msg ntm;
	nss_tx_status_t status;

	memset(&cfg, 0, sizeof(struct nss_tlsmgr_config));
	cfg.algo = NSS_TLSMGR_ALGO_NULL_SHA1_HMAC;
	cfg.hdr_ver = TLSHDR_VERSION_1_2;

	/*
	 * Create a dummy crypto session for the first CCS operation
	 */
	crypto = nss_tlsmgr_crypto_alloc(&cfg);
	if (!crypto) {
		nss_tlsmgr_crypto_free(crypto);
		nss_tlsmgr_warn("%px: unable to update encap context data", ctx);
		return NSS_TLSMGR_FAIL_NOMEM;
	}

	memset(&ntm, 0, sizeof(struct nss_tls_msg));

	ntcu = &ntm.msg.cipher_update;
	ntcu->crypto_idx = crypto->crypto_idx;
	ntcu->ver = cfg.hdr_ver;
	ntcu->skip = 1;

	status = nss_tls_tx_msg_sync(ctx->nss_ctx, ctx->ifnum, msg_type, sizeof(*ntcu), &ntm);
	if (status != NSS_TX_SUCCESS) {
		nss_tlsmgr_warn("%px: Failed to configure decap, status:%d, error:%d", ctx, status, ntm.cm.error);
		return false;
	}

	/*
	 * Add crypto to context active list
	 */
	write_lock(&tun->lock);
	list_add_tail(&crypto->list, &ctx->crypto_active);
	write_unlock(&tun->lock);

	nss_tlsmgr_trace("%px: NULL context(0x%x) update done", ctx, ctx->ifnum & ((1 << NSS_CORE_ID_SHIFT) - 1));

	return NSS_TLSMGR_OK;
}

/*
 * nss_tlsmgr_crypto_update_encap()
 *	Update the encapsulation crypto keys.
 */
nss_tlsmgr_status_t nss_tlsmgr_crypto_update_encap(struct net_device *dev, struct nss_tlsmgr_config *cfg)
{
        enum nss_tls_msg_type msg_type = NSS_TLS_MSG_TYPE_CIPHER_UPDATE;
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);
	struct nss_tlsmgr_ctx *ctx = &tun->ctx_enc;
	struct nss_tls_cipher_update *ntcu;
	struct nss_tlsmgr_crypto *crypto;
	struct nss_tls_msg ntm;
	nss_tx_status_t status;

	if ((cfg->hdr_ver != TLSHDR_VERSION_1_1) && (cfg->hdr_ver != TLSHDR_VERSION_1_2)) {
		nss_tlsmgr_warn("%px: Invalid TLS header version: %d", ctx, cfg->hdr_ver);
		return NSS_TLSMGR_FAIL_REC_VERSION;
	}

	if (cfg->algo >= ARRAY_SIZE(tlsmgr_algo_info)) {
		nss_tlsmgr_warn("invalid crypto algorithm\n");
		return -EINVAL;
	}

	crypto = nss_tlsmgr_crypto_alloc(cfg);
	if (!crypto) {
		nss_tlsmgr_warn("%px: unable to update encap context data", ctx);
		return NSS_TLSMGR_FAIL_NOMEM;
	}

	memset(&ntm, 0, sizeof(struct nss_tls_msg));

	ntcu = &ntm.msg.cipher_update;
	ntcu->crypto_idx = crypto->crypto_idx;
	ntcu->ver = cfg->hdr_ver;

	status = nss_tls_tx_msg_sync(ctx->nss_ctx, ctx->ifnum, msg_type, sizeof(*ntcu), &ntm);
	if (status != NSS_TX_SUCCESS) {
		nss_tlsmgr_crypto_free(crypto);
		nss_tlsmgr_warn("%px: Failed to configure encap, status:%d, error:%d", ctx, status, ntm.cm.error);
		return NSS_TLSMGR_FAIL_MESSAGE;
	}

	/*
	 * Add crypto to context active list
	 */
	write_lock(&tun->lock);
	list_add_tail(&crypto->list, &ctx->crypto_active);
	write_unlock(&tun->lock);

	nss_tlsmgr_trace("%px: encap context(0x%x) update done", ctx, ctx->ifnum & ((1 << NSS_CORE_ID_SHIFT) - 1));

	return NSS_TLSMGR_OK;
}
EXPORT_SYMBOL(nss_tlsmgr_crypto_update_encap);

/*
 * nss_tlsmgr_crypto_update_decap()
 *	Update the decapsulation crypto keys.
 */
nss_tlsmgr_status_t nss_tlsmgr_crypto_update_decap(struct net_device *dev, struct nss_tlsmgr_config *cfg)
{
        enum nss_tls_msg_type msg_type = NSS_TLS_MSG_TYPE_CIPHER_UPDATE;
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);
	struct nss_tlsmgr_ctx *ctx = &tun->ctx_dec;
	struct nss_tls_cipher_update *ntcu;
	struct nss_tlsmgr_crypto *crypto;
	struct nss_tls_msg ntm;
	nss_tx_status_t status;

	if ((cfg->hdr_ver != TLSHDR_VERSION_1_1) && (cfg->hdr_ver != TLSHDR_VERSION_1_2)) {
		nss_tlsmgr_warn("%px: Invalid TLS header version: %d", ctx, cfg->hdr_ver);
		return NSS_TLSMGR_FAIL_REC_VERSION;
	}

	if (cfg->algo >= ARRAY_SIZE(tlsmgr_algo_info)) {
		nss_tlsmgr_warn("invalid crypto algorithm\n");
		return -EINVAL;
	}

	crypto = nss_tlsmgr_crypto_alloc(cfg);
	if (!crypto) {
		nss_tlsmgr_warn("%px: unable to update decap context data", ctx);
		return NSS_TLSMGR_FAIL_NOMEM;
	}

	memset(&ntm, 0, sizeof(struct nss_tls_msg));

	ntcu = &ntm.msg.cipher_update;
	ntcu->crypto_idx = crypto->crypto_idx;
	ntcu->ver = cfg->hdr_ver;

	status = nss_tls_tx_msg_sync(ctx->nss_ctx, ctx->ifnum, msg_type, sizeof(*ntcu), &ntm);
	if (status != NSS_TX_SUCCESS) {
		nss_tlsmgr_crypto_free(crypto);
		nss_tlsmgr_warn("%px: Failed to configure decap, status:%d, error:%d", ctx, status, ntm.cm.error);
		return false;
	}

	/*
	 * Add crypto to context active list
	 */
	write_lock(&tun->lock);
	list_add_tail(&crypto->list, &ctx->crypto_active);
	write_unlock(&tun->lock);

	nss_tlsmgr_trace("%px: decap context(0x%x) update done", ctx, ctx->ifnum & ((1 << NSS_CORE_ID_SHIFT) - 1));

	return NSS_TLSMGR_OK;
}
EXPORT_SYMBOL(nss_tlsmgr_crypto_update_decap);
