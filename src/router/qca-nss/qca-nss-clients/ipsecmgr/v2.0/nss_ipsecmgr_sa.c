/*
 **************************************************************************
 * Copyright (c) 2016-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/route.h>
#include <net/ip6_route.h>
#include <asm/atomic.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>

#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/internal/hash.h>
#include <crypto/authenc.h>
#include <crypto/skcipher.h>

#include <nss_api_if.h>
#include <nss_ipsec_cmn.h>
#include <nss_ipsecmgr.h>
#include <nss_cryptoapi.h>

#include "nss_ipsecmgr_ref.h"
#include "nss_ipsecmgr_flow.h"
#include "nss_ipsecmgr_sa.h"
#include "nss_ipsecmgr_ctx.h"
#include "nss_ipsecmgr_tunnel.h"
#include "nss_ipsecmgr_priv.h"

#define NSS_IPSECMGR_DEBUGFS_NAME_SZ 128 /* bytes */

extern struct nss_ipsecmgr_drv *ipsecmgr_drv;

/*
 * Linux crypto algorithm names.
 */
static const char *ipsecmgr_algo_name[NSS_IPSECMGR_ALGO_MAX] = {
	"echainiv(authenc(hmac(sha1),cbc(aes)))",
	"echainiv(authenc(hmac(sha256),cbc(aes)))",
	"echainiv(authenc(hmac(sha1),cbc(des3_ede)))",
	"echainiv(authenc(hmac(sha256),cbc(des3_ede)))",
	"hmac(sha1)",
	"hmac(sha256)",
	"seqiv(rfc4106(gcm(aes)))",
	"echainiv(authenc(hmac(md5),cbc(aes)))",
	"echainiv(authenc(hmac(md5),cbc(des3_ede)))",
	"echainiv(authenc(hmac(sha384),cbc(aes)))",
	"echainiv(authenc(hmac(sha512),cbc(aes)))",
};

/*
 * SA tuple print info
 */
static const struct nss_ipsecmgr_print ipsecmgr_print_sa_tuple[] = {
	{"dest_ip", NSS_IPSECMGR_PRINT_IPADDR},
	{"dest_port", NSS_IPSECMGR_PRINT_WORD},
	{"src_ip", NSS_IPSECMGR_PRINT_IPADDR},
	{"src_port", NSS_IPSECMGR_PRINT_WORD},
	{"spi", NSS_IPSECMGR_PRINT_WORD},
	{"protocol", NSS_IPSECMGR_PRINT_WORD},
	{"ip_version", NSS_IPSECMGR_PRINT_WORD},
};

/*
 * SA replay print info
 */
static const struct nss_ipsecmgr_print ipsecmgr_print_sa_replay[] = {
	{"replay_win_start", NSS_IPSECMGR_PRINT_DWORD},
	{"replay_win_current", NSS_IPSECMGR_PRINT_DWORD},
	{"replay_win_size", NSS_IPSECMGR_PRINT_WORD},
};

/*
 * SA tx default print info
 */
static const struct nss_ipsecmgr_print ipsecmgr_print_sa_feature[] = {
	{"tx_default", NSS_IPSECMGR_PRINT_BYTE},
	{"flags", NSS_IPSECMGR_PRINT_WORD},
};

/*
 * SA statistics print info
 */
static const struct nss_ipsecmgr_print ipsecmgr_print_sa_stats[] = {
	{"\trx_packets", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_bytes", NSS_IPSECMGR_PRINT_DWORD},
	{"\ttx_packets", NSS_IPSECMGR_PRINT_DWORD},
	{"\ttx_bytes", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[0]", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[1]", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[2]", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[3]", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_headroom", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_tailroom", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_replay", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_replay_dup", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_replay_win", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_pbuf_crypto", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_queue", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_queue_crypto", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_queue_nexthop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_pbuf_alloc", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_pbuf_linear", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_pbuf_stats", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_pbuf_align", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_cipher", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_auth", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_seq_ovf", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_blk_len", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_hash_len", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_transform", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_crypto", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_classification", NSS_IPSECMGR_PRINT_DWORD},
};

/*
 * nss_ipsecmgr_sa_tuple_print()
 * 	Print SA tuple
 */
static ssize_t nss_ipsecmgr_sa_tuple_print(struct nss_ipsec_cmn_sa_tuple *tuple, char *buf, ssize_t max_len)
{
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_sa_tuple;
	uint32_t dest_ip[4], src_ip[4];
	ssize_t len = 0;

	len += snprintf(buf + len, max_len - len, "SA tuple: {");
	switch (tuple->ip_ver) {
	case IPVERSION:
		len += snprintf(buf + len, max_len - len, "%s: %pI4h,", prn->str, tuple->dest_ip);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->dest_port);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %pI4h,", prn->str, tuple->src_ip);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->src_port);
		prn++;

		break;

	case 6:
		nss_ipsecmgr_hton_v6addr(src_ip, tuple->src_ip);
		nss_ipsecmgr_hton_v6addr(dest_ip, tuple->dest_ip);

		len += snprintf(buf + len, max_len - len, "%s: %pI6c,", prn->str, dest_ip);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->dest_port);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %pI6c,", prn->str, src_ip);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->src_port);
		prn++;

		break;
	}

	len += snprintf(buf + len, max_len - len, "%s: 0x%x,", prn->str, tuple->spi_index);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->protocol);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u", prn->str, tuple->ip_ver);
	prn++;

	len += snprintf(buf + len, max_len - len, "}\n");
	return len;
}

/*
 * nss_ipsecmgr_sa_feature_print()
 *	Print SA tx default value
 */
static ssize_t nss_ipsecmgr_sa_feature_print(struct nss_ipsecmgr_sa *sa, char *buf, ssize_t max_len)
{
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_sa_feature;
	ssize_t len = 0;

	len += snprintf(buf + len, max_len - len, "SA feature: {");
	len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, sa->state.tx_default);
	prn++;
	len += snprintf(buf + len, max_len - len, "%s: 0x%x", prn->str, sa->state.data.flags);
	len += snprintf(buf + len, max_len - len, "}\n");

	return len;
}

/*
 * nss_ipsecmgr_sa_replay_print()
 * 	Print SA replay state
 */
static ssize_t nss_ipsecmgr_sa_replay_print(struct nss_ipsec_cmn_sa_replay *replay, char *buf, ssize_t max_len)
{
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_sa_replay;
	ssize_t len = 0;

	len += snprintf(buf + len, max_len - len, "SA replay: {");

	len += snprintf(buf + len, max_len - len, "%s: %llu,", prn->str, replay->seq_start);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %llu,", prn->str, replay->seq_cur);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u", prn->str, replay->window_size);
	prn++;

	len += snprintf(buf + len, max_len - len, "}\n");
	return len;
}

/*
 * nss_ipsecmgr_sa_stats_print()
 * 	Print SA statistics
 */
static ssize_t nss_ipsecmgr_sa_stats_print(struct nss_ipsecmgr_sa_stats_priv *stats, char *buf, ssize_t max_len)
{
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_sa_stats;
	uint64_t *stats_word = (uint64_t *)stats;
	ssize_t len = 0;
	int i;

	/*
	 * This expects a strict order as per the stats structure
	 */
	len += snprintf(buf + len, max_len - len, "SA stats: {\n");

	for (i = 0; i < ARRAY_SIZE(ipsecmgr_print_sa_stats); i++, prn++)
		len += snprintf(buf + len, max_len - len, "%s: %llu\n", prn->str, *stats_word++);

	len += snprintf(buf + len, max_len - len, "}\n");
	return len;
}

/*
 * nss_ipsecmgr_sa_print_len()
 * 	Return the total length for printing
 */
static ssize_t nss_ipsecmgr_sa_print_len(struct nss_ipsecmgr_ref *ref)
{
	ssize_t len = NSS_IPSECMGR_SA_PRINT_EXTRA;
	const struct nss_ipsecmgr_print *prn;
	int i;

	for (i = 0, prn = ipsecmgr_print_sa_tuple; i < ARRAY_SIZE(ipsecmgr_print_sa_tuple); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	for (i = 0, prn = ipsecmgr_print_sa_replay; i < ARRAY_SIZE(ipsecmgr_print_sa_replay); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	for (i = 0, prn = ipsecmgr_print_sa_stats; i < ARRAY_SIZE(ipsecmgr_print_sa_stats); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	for (i = 0, prn = ipsecmgr_print_sa_feature; i < ARRAY_SIZE(ipsecmgr_print_sa_feature); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	return len;
}

/*
 * nss_ipsecmgr_sa_print()
 *	Print SA info
 */
static ssize_t nss_ipsecmgr_sa_print(struct nss_ipsecmgr_ref *ref, char *buf)
{
	struct nss_ipsecmgr_sa *sa = container_of(ref, struct nss_ipsecmgr_sa, ref);
	ssize_t max_len = nss_ipsecmgr_sa_print_len(ref);
	ssize_t len;

	len = snprintf(buf, max_len, "---- SA(0x%x) -----\n", sa->state.tuple.spi_index);

	len += nss_ipsecmgr_sa_tuple_print(&sa->state.tuple, buf + len, max_len - len);
	len += nss_ipsecmgr_sa_replay_print(&sa->state.replay, buf + len, max_len - len);
	len += nss_ipsecmgr_sa_feature_print(sa, buf + len, max_len - len);
	len += nss_ipsecmgr_sa_stats_print(&sa->stats, buf + len, max_len - len);

	return len;
}

/*
 * nss_ipsecmgr_sa_crypto_alloc()
 *	Allocate Crypto resources
 */
static nss_ipsecmgr_status_t nss_ipsecmgr_sa_crypto_alloc(struct nss_ipsecmgr_sa *sa,
							  struct nss_ipsecmgr_sa_cmn *cmn,
							  struct nss_ipsec_cmn_sa_tuple *tuple,
						  	  struct nss_ipsec_cmn_sa_data *data)
{
	struct nss_ipsecmgr_crypto_keys *keys = &cmn->keys;
	struct crypto_authenc_key_param *key_param;
	struct rtattr *rta;
	char *rt_keys, *p;
	uint32_t index;
	uint16_t keylen;

	/*
	 * If, crypto is programmed using index(s) skip key-based programming
	 */
	if (!cmn->crypto_has_keys) {
		sa->aead = NULL;
		data->blk_len = cmn->index.blk_len;
		data->iv_len = cmn->index.iv_len;
		data->icv_len = cmn->icv_len;
		tuple->crypto_index = cmn->index.session;
		return NSS_IPSECMGR_OK;
	}

	switch (cmn->algo) {
	/*
	 * AEAD Algorithms
	 */
	case NSS_IPSECMGR_ALGO_AES_CBC_MD5_HMAC:
	case NSS_IPSECMGR_ALGO_AES_CBC_SHA1_HMAC:
	case NSS_IPSECMGR_ALGO_AES_CBC_SHA256_HMAC:
	case NSS_IPSECMGR_ALGO_3DES_CBC_MD5_HMAC:
	case NSS_IPSECMGR_ALGO_3DES_CBC_SHA1_HMAC:
	case NSS_IPSECMGR_ALGO_3DES_CBC_SHA256_HMAC:
	case NSS_IPSECMGR_ALGO_AES_CBC_SHA384_HMAC:
	case NSS_IPSECMGR_ALGO_AES_CBC_SHA512_HMAC:

		sa->aead = crypto_alloc_aead(ipsecmgr_algo_name[cmn->algo], 0, 0);
		if (IS_ERR(sa->aead)) {
			nss_ipsecmgr_warn("%px: failed to allocate crypto aead context for algo=%s\n", sa,
					  ipsecmgr_algo_name[cmn->algo]);
			return NSS_IPSECMGR_FAIL_NOCRYPTO;
		}

		nss_ipsecmgr_trace("cipher_keylen:%d auth_keylen:%d\n", keys->cipher_keylen, keys->auth_keylen);

		/*
		 * Construct keys
		 */
		keylen = RTA_SPACE(sizeof(*key_param));
		keylen += keys->cipher_keylen;
		keylen += keys->auth_keylen;
		keylen += keys->nonce_size;

		rt_keys = vzalloc(keylen);
		if (!rt_keys) {
			nss_ipsecmgr_warn("%px: failed to allocate key memory\n", sa);
			crypto_free_aead(sa->aead);
			return NSS_IPSECMGR_FAIL_NOMEM;
		}

		p = rt_keys;
		rta = (void *)p;
		rta->rta_type = CRYPTO_AUTHENC_KEYA_PARAM;
		rta->rta_len = RTA_LENGTH(sizeof(*key_param));
		key_param = RTA_DATA(rta);
		p += RTA_SPACE(sizeof(*key_param));

		/*
		 * Copy authentication key
		 */
		memcpy(p, keys->auth_key, keys->auth_keylen);
		p += keys->auth_keylen;

		/*
		 * Copy cipher Key
		 */
		key_param->enckeylen = cpu_to_be32(keys->cipher_keylen);
		memcpy(p, keys->cipher_key, keys->cipher_keylen);

		if (crypto_aead_setkey(sa->aead, rt_keys, keylen)) {
			nss_ipsecmgr_warn("%px: failed to configure keys\n", sa);
			crypto_free_aead(sa->aead);
			vfree(rt_keys);
			return NSS_IPSECMGR_INVALID_KEYLEN;
		}

		nss_cryptoapi_aead_ctx2session(sa->aead, &index);
		data->blk_len = (uint8_t)crypto_aead_blocksize(sa->aead);
		data->iv_len = (uint8_t)crypto_aead_ivsize(sa->aead);
		data->icv_len = cmn->icv_len;
		tuple->crypto_index = (uint16_t)index;
		vfree(rt_keys);
		break;

	/*
	 * AHASH Algorithms
	 */
	case NSS_IPSECMGR_ALGO_NULL_CIPHER_SHA1_HMAC:
	case NSS_IPSECMGR_ALGO_NULL_CIPHER_SHA256_HMAC:
		sa->ahash = crypto_alloc_ahash(ipsecmgr_algo_name[cmn->algo], 0, 0);
		if (IS_ERR(sa->ahash)) {
			nss_ipsecmgr_warn("%px: failed to allocate crypto ahash context\n", sa);
			return NSS_IPSECMGR_FAIL_NOCRYPTO;
		}

		if (crypto_ahash_setkey(sa->ahash, keys->auth_key, keys->auth_keylen)) {
			nss_ipsecmgr_warn("%px: failed to configure keys\n", sa);
			crypto_free_ahash(sa->ahash);
			return NSS_IPSECMGR_INVALID_KEYLEN;
		}

		nss_cryptoapi_ahash_ctx2session(sa->ahash, &index);
		data->flags |= NSS_IPSEC_CMN_FLAG_CIPHER_NULL;
		data->icv_len = cmn->icv_len;
		data->blk_len = 0;
		data->iv_len = 0;
		tuple->crypto_index = (uint16_t)index;
		break;

	/*
	 * GCM Mode
	 */
	case NSS_IPSECMGR_ALGO_AES_GCM_GMAC_RFC4106:
		sa->aead = crypto_alloc_aead(ipsecmgr_algo_name[cmn->algo], 0, 0);
		if (IS_ERR(sa->aead)) {
			nss_ipsecmgr_warn("%px: failed to allocate crypto aead context\n", sa);
			return NSS_IPSECMGR_FAIL_NOCRYPTO;
		}

		keylen = keys->cipher_keylen + keys->nonce_size;

		/*
		 * Construct key with nonce
		 */
		rt_keys = vzalloc(keylen);
		if (!rt_keys) {
			nss_ipsecmgr_warn("%px: failed to allocate key memory\n", sa);
			crypto_free_aead(sa->aead);
			return NSS_IPSECMGR_FAIL_NOMEM;
		}

		memcpy(rt_keys, keys->cipher_key, keys->cipher_keylen);
		memcpy(rt_keys + keys->cipher_keylen, (uint8_t *)keys->nonce, keys->nonce_size);

		if (crypto_aead_setkey(sa->aead, rt_keys, keylen)) {
			nss_ipsecmgr_warn("%px: failed to configure keys\n", sa);
			crypto_free_aead(sa->aead);
			vfree(rt_keys);
			return NSS_IPSECMGR_INVALID_KEYLEN;
		}

		nss_cryptoapi_aead_ctx2session(sa->aead, &index);
		data->blk_len = (uint8_t)crypto_aead_blocksize(sa->aead);
		data->iv_len = (uint8_t)crypto_aead_ivsize(sa->aead);
		data->icv_len = cmn->icv_len;
		data->flags |= NSS_IPSEC_CMN_FLAG_CIPHER_GCM;
		tuple->crypto_index = (uint16_t)index;
		vfree(rt_keys);
		break;

	default:
		nss_ipsecmgr_warn("%px: invalid crypto algorithm\n", sa);
		return NSS_IPSECMGR_INVALID_ALGO;
	}

	return NSS_IPSECMGR_OK;
}

/*
 * nss_ipsecmgr_sa_free()
 *	Free the SA entry and any associated crypto context
 */
static void nss_ipsecmgr_sa_free(struct nss_ipsecmgr_sa *sa)
{
	if (sa->aead)
		crypto_free_aead(sa->aead);

	if (sa->ahash)
		crypto_free_ahash(sa->ahash);

	kfree(sa);
}

/*
 * nss_ipsecmgr_sa_del_ref()
 *	Detach the SA entry from the list
 */
static void nss_ipsecmgr_sa_del_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_sa *sa = container_of(ref, struct nss_ipsecmgr_sa, ref);
	struct nss_ipsecmgr_tunnel *tun = NULL;
	struct net_device *dev;

	/*
	 * Linux does not provide any specific API(s) to test for RW locks. The caller
	 * being internal is assumed to hold write lock before initiating this.
	 */
	BUG_ON(write_can_lock(&ipsecmgr_drv->lock));

	list_del_init(&sa->list);

	dev = dev_get_by_index(&init_net, sa->tunnel_id);
	if (!dev) {
		nss_ipsecmgr_trace("%px: Failed to find dev for tunnel-ID(%u)", sa, sa->tunnel_id);
		return;
	}

	/*
	 * Check the default transmit SA; if it matches then we clear it
	 */
	tun = netdev_priv(dev);
	if (tun->tx_sa == sa) {
		tun->tx_sa = NULL;
	}

	dev_put(dev);
}

/*
 * nss_ipsecmgr_sa_free_ref()
 *	Detach the SA entry from the list
 */
static void nss_ipsecmgr_sa_free_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_sa *sa = container_of(ref, struct nss_ipsecmgr_sa, ref);
	enum nss_ipsec_cmn_msg_type type = NSS_IPSEC_CMN_MSG_TYPE_SA_DESTROY;
	struct nss_ipsec_cmn_msg nicm;
	nss_tx_status_t status;

	/*
	 * The free path can potentially sleep hence we detach the SA here but
	 * free it later
	 */
	memset(&nicm, 0, sizeof(nicm));
	memcpy(&nicm.msg.sa.sa_tuple, &sa->state.tuple, sizeof(nicm.msg.sa.sa_tuple));

	status = nss_ipsec_cmn_tx_msg_sync(sa->nss_ctx, sa->ifnum, type, sizeof(nicm.msg.sa), &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_warn("%px: Failed to send message(%u) to NSS(%u)", sa->nss_ctx, type, status);
	}

	nss_ipsecmgr_sa_free(sa);
}

/*
 * nss_ipsecmgr_sa_alloc()
 *	Allocate SA and initialize it
 */
static struct nss_ipsecmgr_sa *nss_ipsecmgr_sa_alloc(struct nss_ipsecmgr_ctx *ctx)
{
	struct nss_ipsecmgr_tunnel *tun = ctx->tun;
	struct nss_ipsecmgr_sa *sa;

	/*
	 * Allocate the SA entry
	 */
	sa = kzalloc(sizeof(*sa), GFP_ATOMIC);
	if (!sa) {
		nss_ipsecmgr_warn("%px: Failed to allocate SA", ctx);
		return NULL;
	}

	sa->tunnel_id = tun->dev->ifindex;
	sa->type = ctx->state.type;
	sa->nss_ctx = ctx->nss_ctx;
	sa->ifnum = ctx->ifnum;
	sa->cb = tun->cb;

	INIT_LIST_HEAD(&sa->list);
	nss_ipsecmgr_ref_init(&sa->ref, nss_ipsecmgr_sa_del_ref, nss_ipsecmgr_sa_free_ref);
	nss_ipsecmgr_ref_init_print(&sa->ref, nss_ipsecmgr_sa_print_len, nss_ipsecmgr_sa_print);

	return sa;
}

/*
 * nss_ipsecmgr_sa_update_db()
 *	Update SA database
 */
static bool nss_ipsecmgr_sa_update_db(struct nss_ipsecmgr_sa *sa)
{
	struct list_head *db = ipsecmgr_drv->sa_db;
	struct nss_ipsecmgr_tunnel *tun;
	struct nss_ipsecmgr_ctx *ctx;
	struct net_device *dev;
	uint32_t hash_idx;

	dev = dev_get_by_index(&init_net, sa->tunnel_id);
	if (!dev) {
		nss_ipsecmgr_warn("%px: Failed to find tunnel(%d) between SA creation\n", sa, sa->tunnel_id);
		return false;
	}

	tun = netdev_priv(dev);

	write_lock_bh(&ipsecmgr_drv->lock);

	ctx = nss_ipsecmgr_ctx_find(tun, sa->type);
	if (!ctx) {
		nss_ipsecmgr_warn("%px: Failed to find context (%u) between SA creation\n", sa, sa->type);
		write_unlock_bh(&ipsecmgr_drv->lock);
		dev_put(dev);
		return false;
	}

	/*
	 * Save the SA for default TX on tunnel if enabled
	 */
	if (sa->state.tx_default) {
		tun->tx_sa = sa;
	}

	/*
	 * Compute the hash index and add SA reference to the context.
	 */
	hash_idx = nss_ipsecmgr_sa_tuple2hash(&sa->state.tuple, NSS_IPSECMGR_SA_MAX);

	nss_ipsecmgr_ref_add(&sa->ref, &ctx->ref);
	list_add(&sa->list, &db[hash_idx]);
	write_unlock_bh(&ipsecmgr_drv->lock);

	dev_put(dev);
	return true;
}

/*
 * nss_ipsecmgr_sa_create_resp()
 *	SA create response callback
 */
static void nss_ipsecmgr_sa_create_resp(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_ipsecmgr_sa *sa = app_data;

	if (ncm->response != NSS_CMN_RESPONSE_ACK) {
#ifdef NSS_IPSECMGR_DEBUG
		if (ncm->error == NSS_IPSEC_CMN_MSG_ERROR_SA_DUP) {
			write_lock_bh(&ipsecmgr_drv->lock);
			if (!nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa->state.tuple)) {
				nss_ipsecmgr_trace("%px: Duplicate SA in FW not in host (%u)\n", sa, ncm->error);
			}
			write_unlock_bh(&ipsecmgr_drv->lock);
		}
#endif
		nss_ipsecmgr_trace("%px: NSS response error (%u)\n", sa, ncm->error);
		nss_ipsecmgr_sa_free(sa);
		return;
	}

	if (!nss_ipsecmgr_sa_update_db(sa)) {
		nss_ipsecmgr_warn("%px: Failed to update SA database", sa);
		nss_ipsecmgr_sa_free(sa);
		return;
	}
}

/*
 * nss_ipsecmgr_sa_init_encap()
 *	Initialize encapsulation SA
 */
static void nss_ipsecmgr_sa_init_encap(struct nss_ipsecmgr_sa *sa, struct nss_ipsecmgr_sa_data *data,
					struct nss_ipsec_cmn_msg *nicm)
{
	struct nss_ipsec_cmn_sa_tuple *sa_tuple = &nicm->msg.sa.sa_tuple;
	struct nss_ipsec_cmn_sa_data *sa_data = &nicm->msg.sa.sa_data;

	sa_data->df = data->encap.df;
	sa_data->dscp = data->encap.dscp;
	sa_data->flags |= data->cmn.enable_natt ? NSS_IPSEC_CMN_FLAG_IPV4_NATT : 0;
	sa_data->flags |= data->cmn.enable_esn ? NSS_IPSEC_CMN_FLAG_ESP_ESN : 0;
	sa_data->flags |= data->cmn.skip_trailer ? NSS_IPSEC_CMN_FLAG_ESP_SKIP : 0;
	sa_data->flags |= data->encap.copy_dscp ? NSS_IPSEC_CMN_FLAG_COPY_DSCP : 0;
	sa_data->flags |= data->encap.copy_df ? NSS_IPSEC_CMN_FLAG_COPY_DF : 0;
	sa_data->flags |= data->cmn.transport_mode ? NSS_IPSEC_CMN_FLAG_MODE_TRANS : 0;

	if (sa_tuple->ip_ver == 6) {
		sa_data->flags &= ~NSS_IPSEC_CMN_FLAG_HDR_MASK;
		sa_data->flags |= NSS_IPSEC_CMN_FLAG_IPV6;
	}

	sa_tuple->hop_limit = data->encap.ttl_hop_limit;

	/*
	 * Copy tuple information for deletion
	 */
	memcpy(&sa->state.tuple, sa_tuple, sizeof(sa->state.tuple));
	memcpy(&sa->state.data, sa_data, sizeof(sa->state.data));
	sa->state.tx_default = !!data->encap.tx_default;

	nss_ipsecmgr_trace("%px:Encapsulation SA initialized ", sa);
}

/*
 * nss_ipsecmgr_sa_init_decap()
 *	Initialize de-capsulation SA
 */
static void nss_ipsecmgr_sa_init_decap(struct nss_ipsecmgr_sa *sa, struct nss_ipsecmgr_sa_data *data,
					struct nss_ipsec_cmn_msg *nicm)
{
	struct nss_ipsec_cmn_sa_tuple *sa_tuple = &nicm->msg.sa.sa_tuple;
	struct nss_ipsec_cmn_sa_data *sa_data = &nicm->msg.sa.sa_data;

	sa_data->window_size = data->decap.replay_win;

	sa_data->flags |= data->cmn.enable_natt ? NSS_IPSEC_CMN_FLAG_IPV4_NATT : 0;
	sa_data->flags |= data->cmn.enable_esn ? NSS_IPSEC_CMN_FLAG_ESP_ESN : 0;
	sa_data->flags |= data->cmn.skip_trailer ? NSS_IPSEC_CMN_FLAG_ESP_SKIP : 0;
	sa_data->flags |= data->cmn.transport_mode ? NSS_IPSEC_CMN_FLAG_MODE_TRANS : 0;
	sa_data->flags |= data->decap.replay_win ? NSS_IPSEC_CMN_FLAG_ESP_REPLAY : 0;

	if (sa_tuple->ip_ver == 6) {
		sa_data->flags &= ~NSS_IPSEC_CMN_FLAG_HDR_MASK;
		sa_data->flags |= NSS_IPSEC_CMN_FLAG_IPV6;
	}

	/*
	 * Copy tuple information for deletion
	 */
	memcpy(&sa->state.tuple, sa_tuple, sizeof(sa->state.tuple));
	memcpy(&sa->state.data, sa_data, sizeof(sa->state.data));

	nss_ipsecmgr_trace("%px:Decapsulation SA initialized ", sa);
}

/*
 * nss_ipsecmgr_sa_sync_state()
 *	Update SA sync state
 */
void nss_ipsecmgr_sa_sync2stats(struct nss_ipsecmgr_sa *sa, struct nss_ipsec_cmn_sa_sync *sync,
					struct nss_ipsecmgr_sa_stats *stats)
{
	struct nss_ipsec_cmn_sa_stats *sa_stats = &sync->stats;
	struct nss_ipsec_cmn_sa_data *sa_data = &sa->state.data;
	uint32_t *drop_counters;
	size_t num_counters;
	int i;

	nss_ipsecmgr_sa_tuple2sa(&sync->sa_tuple, &stats->sa);

	switch (sa->type) {
	case NSS_IPSEC_CMN_CTX_TYPE_INNER:
	case NSS_IPSEC_CMN_CTX_TYPE_MDATA_INNER:
		stats->pkt_count = sa_stats->cmn_stats.tx_packets;
		stats->pkt_bytes = sa_stats->cmn_stats.tx_bytes;
		break;

	case NSS_IPSEC_CMN_CTX_TYPE_OUTER:
	case NSS_IPSEC_CMN_CTX_TYPE_MDATA_OUTER:
		stats->pkt_count = sa_stats->cmn_stats.rx_packets;
		stats->pkt_bytes = sa_stats->cmn_stats.rx_bytes;
		break;
	default:
		return;

	}

	stats->pkt_failed = 0;
	for (i = 0; i < ARRAY_SIZE(sa_stats->cmn_stats.rx_dropped); i++)
		stats->pkt_failed += sa_stats->cmn_stats.rx_dropped[i];

	/*
	 * Drop counters starts after common stats counters
	 */
	drop_counters = (uint32_t *)((uint8_t *)sa_stats + sizeof(sa_stats->cmn_stats));
	num_counters = (sizeof(*sa_stats) - sizeof(sa_stats->cmn_stats)) / sizeof(uint32_t);

	for (i = 0; i < num_counters; i++)
		stats->pkt_failed += drop_counters[i];

	if (sa_data->window_size) {
		stats->window_size = sa_data->window_size;
		stats->seq_start = sync->replay.seq_start;
		stats->seq_cur = sync->replay.seq_cur;
	}
}

/*
 * nss_ipsecmgr_sa_sync_state()
 *	Update SA sync state
 */
void nss_ipsecmgr_sa_sync_state(struct nss_ipsecmgr_sa *sa, struct nss_ipsec_cmn_sa_sync *sa_sync)
{
	uint32_t *msg_stats = (uint32_t *)&sa_sync->stats;
	uint64_t *sa_stats = (uint64_t *)&sa->stats;
	int num;

	/*
	 * DEBUG check to see if the lock is taken before accessing
	 * SA entry in the database
	 */
	BUG_ON(write_can_lock(&ipsecmgr_drv->lock));

	for (num = 0; num < sizeof(sa->stats)/sizeof(*sa_stats); num++) {
		sa_stats[num] += msg_stats[num];
	}

	memcpy(&sa->state.replay, &sa_sync->replay, sizeof(sa->state.replay));
}

/*
 * nss_ipsecmgr_sa_find()
 *	Find an SA using a SA tuple
 */
struct nss_ipsecmgr_sa *nss_ipsecmgr_sa_find(struct list_head *db, struct nss_ipsec_cmn_sa_tuple *tuple)
{
	struct nss_ipsecmgr_sa *sa;
	uint32_t hash_idx;

	hash_idx = nss_ipsecmgr_sa_tuple2hash(tuple, NSS_IPSECMGR_SA_MAX);

	list_for_each_entry(sa, &db[hash_idx], list) {
		if (nss_ipsecmgr_sa_tuple_match(&sa->state.tuple, tuple)) {
			return sa;
		}
	}

	return NULL;
}

/*
 * nss_ipsecmgr_sa_del()
 *	Delete an existing SA
 */
void nss_ipsecmgr_sa_del(struct net_device *dev, struct nss_ipsecmgr_sa_tuple *tuple)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsec_cmn_sa_tuple sa_tuple = {0};
	struct nss_ipsecmgr_sa *sa;

	nss_ipsecmgr_sa2tuple(tuple, &sa_tuple);

	/*
	 * Write lock needed here since SA and all related Flows
	 * are removed from DB. This requires write lock since SA Delete
	 * might be called in parallel from other cores as well
	 */
	write_lock_bh(&ipsecmgr_drv->lock);

	sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa_tuple);
	if (!sa) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: failed to find SA for deletion\n", tun);
		return;
	}

	/*
	 * Free the entire reference hierarchy
	 */
	nss_ipsecmgr_ref_del(&sa->ref, &tun->free_refs);
	write_unlock_bh(&ipsecmgr_drv->lock);

	schedule_work(&tun->free_work);
}
EXPORT_SYMBOL(nss_ipsecmgr_sa_del);

/*
 * nss_ipsecmgr_sa_add()
 *	Add a new SA for encapsulation or de-capsulation
 */
nss_ipsecmgr_status_t nss_ipsecmgr_sa_add(struct net_device *dev, struct nss_ipsecmgr_sa_tuple *tuple,
					struct nss_ipsecmgr_sa_data *data, uint32_t *ifnum)
{
	const enum nss_ipsec_cmn_msg_type type = NSS_IPSEC_CMN_MSG_TYPE_SA_CREATE;
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsec_cmn_sa_tuple *msg_tuple;
	struct nss_ipsec_cmn_sa_data *msg_data;
	struct nss_ipsec_cmn_msg nicm = {{0}};
	struct nss_ipsecmgr_sa *sa = NULL;
	struct nss_ipsecmgr_ctx *ctx;
	nss_tx_status_t status;

	dev_hold(dev);

	msg_tuple = &nicm.msg.sa.sa_tuple;
	msg_data =  &nicm.msg.sa.sa_data;

	nss_ipsecmgr_sa2tuple(tuple, msg_tuple);

	/*
	 * Check if the SA already exists or not
	 */
	read_lock_bh(&ipsecmgr_drv->lock);
	if (nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, msg_tuple)) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_trace("%px: Duplicate SA found", dev);
		dev_put(dev);
		return NSS_IPSECMGR_DUPLICATE_SA;
	}

	ctx = nss_ipsecmgr_ctx_find_by_sa(tun, data->type);
	if (!ctx) {
		nss_ipsecmgr_warn("%px: failed to find inner context associated with tunnel", tun);
		read_unlock_bh(&ipsecmgr_drv->lock);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL;
	}

	/*
	 * Allocate the SA entry
	 */
	sa = nss_ipsecmgr_sa_alloc(ctx);
	if (!sa) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to allocate SA for add", ctx);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL_NOMEM;
	}

	/*
	 * We are done with the net_device release the reference
	 */
	dev_put(dev);
	read_unlock_bh(&ipsecmgr_drv->lock);

	/*
	 * Allocate crypto resources
	 */
	if (nss_ipsecmgr_sa_crypto_alloc(sa, &data->cmn, msg_tuple, msg_data)) {
		nss_ipsecmgr_warn("%px: Failed to allocate crypto resource for SA add", ctx);
		nss_ipsecmgr_sa_free(sa);
		return NSS_IPSECMGR_FAIL_NOCRYPTO;
	}

	if (data->type == NSS_IPSECMGR_SA_TYPE_ENCAP) {
		nss_ipsecmgr_sa_init_encap(sa, data, &nicm);
	} else {
		nss_ipsecmgr_sa_init_decap(sa, data, &nicm);
	}

	nss_ipsec_cmn_msg_init(&nicm, sa->ifnum, type, sizeof(nicm.msg.sa), nss_ipsecmgr_sa_create_resp, sa);

	status = nss_ipsec_cmn_tx_msg(sa->nss_ctx, &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_warn("%px: Failed to send message(%u) to NSS(%u)\n", ctx, type, status);
		nss_ipsecmgr_sa_free(sa);
		return NSS_IPSECMGR_FAIL_MESSAGE;
	}

	*ifnum = nss_ipsec_cmn_get_ifnum_with_coreid(sa->ifnum);
	return NSS_IPSECMGR_OK;
}
EXPORT_SYMBOL(nss_ipsecmgr_sa_add);

/*
 * nss_ipsecmgr_sa_add_sync()
 *	Add a new SA for encapsulation or de-capsulation synchronously
 */
nss_ipsecmgr_status_t nss_ipsecmgr_sa_add_sync(struct net_device *dev, struct nss_ipsecmgr_sa_tuple *tuple,
						struct nss_ipsecmgr_sa_data *data, uint32_t *ifnum)
{
	const enum nss_ipsec_cmn_msg_type type = NSS_IPSEC_CMN_MSG_TYPE_SA_CREATE;
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsec_cmn_sa_tuple *msg_tuple;
	struct nss_ipsec_cmn_sa_data *msg_data;
	struct nss_ipsec_cmn_msg nicm = {{0}};
	struct nss_ipsecmgr_sa *sa = NULL;
	struct nss_ipsecmgr_ctx *ctx;
	nss_tx_status_t status;

	dev_hold(dev);

	msg_tuple = &nicm.msg.sa.sa_tuple;
	msg_data =  &nicm.msg.sa.sa_data;

	nss_ipsecmgr_sa2tuple(tuple, msg_tuple);

	/*
	 * Check if the SA already exists or not
	 */
	read_lock_bh(&ipsecmgr_drv->lock);
	if (nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, msg_tuple)) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_trace("%px: Duplicate SA found", dev);
		dev_put(dev);
		return NSS_IPSECMGR_DUPLICATE_SA;
	}

	ctx = nss_ipsecmgr_ctx_find_by_sa(tun, data->type);
	if (!ctx) {
		nss_ipsecmgr_warn("%px: failed to find inner context associated with tunnel", tun);
		read_unlock_bh(&ipsecmgr_drv->lock);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL;
	}

	/*
	 * Allocate the SA entry
	 */
	sa = nss_ipsecmgr_sa_alloc(ctx);
	if (!sa) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to allocate SA for add", ctx);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL_NOMEM;
	}

	/*
	 * We are done with the net_device release the reference
	 */
	dev_put(dev);
	read_unlock_bh(&ipsecmgr_drv->lock);

	/*
	 * Allocate crypto resources
	 */
	if (nss_ipsecmgr_sa_crypto_alloc(sa, &data->cmn, msg_tuple, msg_data)) {
		nss_ipsecmgr_warn("%px: Failed to allocate crypto resource for SA add", ctx);
		nss_ipsecmgr_sa_free(sa);
		return NSS_IPSECMGR_FAIL_NOCRYPTO;
	}

	if (data->type == NSS_IPSECMGR_SA_TYPE_ENCAP) {
		nss_ipsecmgr_sa_init_encap(sa, data, &nicm);
	} else {
		nss_ipsecmgr_sa_init_decap(sa, data, &nicm);
	}

	nss_ipsec_cmn_msg_init(&nicm, sa->ifnum, type, sizeof(nicm.msg.sa), NULL, NULL);

	status = nss_ipsec_cmn_tx_msg_sync(sa->nss_ctx, sa->ifnum, type, sizeof(nicm.msg.sa), &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_warn("%px: Failed to send message(%u) to NSS(%u)\n", ctx, type, status);
		nss_ipsecmgr_sa_free(sa);
		return NSS_IPSECMGR_FAIL_MESSAGE;
	}

	/*
	 * Since, this is a synchronous call add it to the database directly
	 */
	if (!nss_ipsecmgr_sa_update_db(sa)) {
		nss_ipsecmgr_warn("%px: Failed to update SA database", sa);
		nss_ipsecmgr_sa_free(sa);
		return NSS_IPSECMGR_FAIL_ADD_DB;
	}

	*ifnum = nss_ipsec_cmn_get_ifnum_with_coreid(sa->ifnum);
	return NSS_IPSECMGR_OK;
}
EXPORT_SYMBOL(nss_ipsecmgr_sa_add_sync);

/*
 * nss_ipsecmgr_sa_verify()
 * 	Confirm SA is present or not for sa tuple.
 */
bool nss_ipsecmgr_sa_verify(struct net_device *dev, struct nss_ipsecmgr_sa_tuple *tuple)
{
	struct nss_ipsec_cmn_sa_tuple sa_tuple = {0};
	struct nss_ipsecmgr_sa *sa;

	/*
	 * Look for an existing SA.
	 */
	nss_ipsecmgr_sa2tuple(tuple, &sa_tuple);

	read_lock_bh(&ipsecmgr_drv->lock);
	sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa_tuple);
	read_unlock_bh(&ipsecmgr_drv->lock);

	return !!sa;
}
EXPORT_SYMBOL(nss_ipsecmgr_sa_verify);

/*
 * nss_ipsecmgr_sa_tx_inner()
 * 	Offload given SKB to NSS for inner processing.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_sa_tx_inner(struct net_device *dev, struct nss_ipsecmgr_sa_tuple *tuple,
					struct sk_buff *skb)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	nss_ipsecmgr_status_t status = NSS_IPSECMGR_OK;
	struct nss_ipsec_cmn_sa_tuple sa_tuple = {0};
	struct nss_ipsec_cmn_mdata_encap *enc_mdata;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipsecmgr_ctx *ctx;
	struct nss_ipsecmgr_sa *sa;
	nss_tx_status_t tx_status;
	uint16_t data_len;
	uint32_t ifnum;

	BUG_ON(skb_shared(skb));
	WARN_ON(skb_is_nonlinear(skb) || skb_has_frag_list(skb));

	dev_hold(dev);

	nss_ipsecmgr_sa2tuple(tuple, &sa_tuple);
	read_lock_bh(&ipsecmgr_drv->lock);

	/*
	 * Look for an existing SA.
	 */
	sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa_tuple);
	if (unlikely(!sa)) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to find SA", tun);
		status = NSS_IPSECMGR_INVALID_SA;
		goto done;
	}

	ctx = nss_ipsecmgr_ctx_find(tun, NSS_IPSEC_CMN_CTX_TYPE_MDATA_INNER);
	if (unlikely(!ctx)) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to find context(%u)", tun, NSS_IPSEC_CMN_CTX_TYPE_MDATA_INNER);
		status = NSS_IPSECMGR_INVALID_CTX;
		goto done;
	}

	ifnum = ctx->ifnum;
	nss_ctx = ctx->nss_ctx;
	data_len = skb->len;

	/*
	 * Expand data area to cover tailroom used by NSS.
	 */
	skb_put(skb, dev->needed_tailroom);

	/*
	 * Add metadata and send SKB to IPsec meta data inner node for encapsulation.
	 */
	enc_mdata = nss_ipsecmgr_tunnel_push_mdata(skb);
	enc_mdata->sa = sa->state.tuple;
	enc_mdata->data_len = data_len;

	read_unlock_bh(&ipsecmgr_drv->lock);

	/*
	 * Send the packet to NSS
	 */
	tx_status = nss_ipsec_cmn_tx_buf(nss_ctx, skb, ifnum);
	if (unlikely(tx_status != NSS_TX_SUCCESS)) {
		nss_ipsecmgr_warn("%px: Failed to send buffer to NSS; error(%u)", tun, tx_status);
		nss_ipsecmgr_tunnel_pull_mdata(skb);
		skb_trim(skb, data_len);
		status = NSS_IPSECMGR_FAIL;
		goto done;
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_ipsecmgr_sa_tx_inner);

/*
 * nss_ipsecmgr_sa_tx_outer()
 * 	Offload given SKB to NSS for outer processing.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_sa_tx_outer(struct net_device *dev, struct nss_ipsecmgr_sa_tuple *tuple,
					struct sk_buff *skb)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	nss_ipsecmgr_status_t status = NSS_IPSECMGR_OK;
	struct nss_ipsec_cmn_sa_tuple sa_tuple = {0};
	struct nss_ipsec_cmn_mdata_decap *dec_mdata;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipsecmgr_ctx *ctx;
	struct nss_ipsecmgr_sa *sa;
	nss_tx_status_t tx_status;
	uint32_t ifnum;

	BUG_ON(skb_shared(skb));

	dev_hold(dev);

	nss_ipsecmgr_sa2tuple(tuple, &sa_tuple);
	read_lock_bh(&ipsecmgr_drv->lock);

	/*
	 * Look for an existing SA.
	 */
	sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa_tuple);
	if (unlikely(!sa)) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to find SA", tun);
		status = NSS_IPSECMGR_INVALID_SA;
		goto done;

	}

	ctx = nss_ipsecmgr_ctx_find(tun, NSS_IPSEC_CMN_CTX_TYPE_MDATA_OUTER);
	if (unlikely(!ctx)) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to find context(%u)", tun, NSS_IPSEC_CMN_CTX_TYPE_MDATA_OUTER);
		status = NSS_IPSECMGR_INVALID_CTX;
		goto done;

	}

	ifnum = ctx->ifnum;
	nss_ctx = ctx->nss_ctx;

	/*
	 * Add metadata and send SKB to IPsec meta data outer node for decapsulation.
	 */
	dec_mdata = nss_ipsecmgr_tunnel_push_mdata(skb);
	dec_mdata->sa = sa->state.tuple;

	read_unlock_bh(&ipsecmgr_drv->lock);

	/*
	 * Send the packet to NSS
	 */
	tx_status = nss_ipsec_cmn_tx_buf(nss_ctx, skb, ifnum);
	if (unlikely(tx_status != NSS_TX_SUCCESS)) {
		nss_ipsecmgr_warn("%px: Failed to send buffer to NSS; error(%u)", tun, tx_status);
		nss_ipsecmgr_tunnel_pull_mdata(skb);
		status = NSS_IPSECMGR_FAIL;
		goto done;
	}

done:
	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_ipsecmgr_sa_tx_outer);
