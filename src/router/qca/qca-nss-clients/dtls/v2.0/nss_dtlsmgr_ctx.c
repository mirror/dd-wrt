/*
 **************************************************************************
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
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
 * nss_dtlsmgr_ctx.c
 *	NSS DTLS Manager Context
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/crypto.h>
#include <linux/debugfs.h>
#include <linux/rtnetlink.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/atomic.h>
#include <asm/cmpxchg.h>

#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/aes.h>
#include <crypto/authenc.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/skcipher.h>
#include <crypto/hash.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include <nss_cryptoapi.h>
#include <nss_dtls_cmn.h>
#include <nss_dtlsmgr.h>

#include "nss_dtlsmgr_private.h"

#define NSS_DTLSMGR_KEY_PARAM_SIZE RTA_SPACE(sizeof(struct crypto_authenc_key_param))

extern struct nss_dtlsmgr g_dtls;

static struct nss_dtlsmgr_algo_info dtlsmgr_algo_info[NSS_DTLSMGR_ALGO_MAX] = {
	{"echainiv(authenc(hmac(sha1),cbc(aes)))", NSS_DTLSMGR_KEY_PARAM_SIZE},
	{"echainiv(authenc(hmac(sha256),cbc(aes)))", NSS_DTLSMGR_KEY_PARAM_SIZE},
	{"echainiv(authenc(hmac(sha1),cbc(des3_ede)))", NSS_DTLSMGR_KEY_PARAM_SIZE},
	{"echainiv(authenc(hmac(sha256),cbc(des3_ede)))", NSS_DTLSMGR_KEY_PARAM_SIZE},
	{"rfc4106(gcm(aes))", 0}
};

/*
 * nss_dtlsmgr_ctx_alloc_crypto()
 *	Allocate a crypto session through Linux CryptoAPI framework.
 */
static int nss_dtlsmgr_ctx_alloc_crypto(struct nss_dtlsmgr_ctx *ctx, struct nss_dtlsmgr_dtls_data *dtls,
					struct nss_dtlsmgr_crypto *crypto)
{
	struct crypto_authenc_key_param *key_param;
	struct nss_dtlsmgr_algo_info *info;
	struct rtattr *rta;
	char *keys, *p;
	uint16_t keylen;

	if (crypto->algo >= ARRAY_SIZE(dtlsmgr_algo_info)) {
		nss_dtlsmgr_warn("%p: invalid crypto algorithm", ctx);
		return -EINVAL;
	}

	info = &dtlsmgr_algo_info[crypto->algo];
	dtls->aead = crypto_alloc_aead(info->name, 0, 0);
	if (IS_ERR(dtls->aead)) {
		nss_dtlsmgr_warn("%p: failed to allocate crypto aead context", ctx);
		return -ENOMEM;
	}

	nss_dtlsmgr_trace("cipher_keylen:%d auth_keylen:%d nonce_len:%d\n",
			  crypto->cipher_key.len, crypto->auth_key.len, crypto->nonce.len);

	/*
	 * Construct keys
	 */
	keylen = info->rta_key_size;
	keylen += crypto->cipher_key.len;
	keylen += crypto->auth_key.len;
	keylen += crypto->nonce.len;

	keys = vzalloc(keylen);
	if (!keys) {
		nss_dtlsmgr_warn("%p: failed to allocate key memory", ctx);
		crypto_free_aead(dtls->aead);
		return -ENOMEM;
	}

	if (crypto->algo == NSS_DTLSMGR_ALGO_AES_GCM) {
		memcpy(keys, crypto->cipher_key.data, crypto->cipher_key.len);
		/* Copy nonce after the key */
		memcpy(keys + crypto->cipher_key.len, crypto->nonce.data, crypto->nonce.len);
		goto setkey;
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
	memcpy(p, crypto->auth_key.data, crypto->auth_key.len);
	p += crypto->auth_key.len;

	/*
	 * Copy cipher Key
	 */
	key_param->enckeylen = cpu_to_be32(crypto->cipher_key.len);
	memcpy(p, crypto->cipher_key.data, crypto->cipher_key.len);

setkey:

	if (crypto_aead_setkey(dtls->aead, keys, keylen)) {
		nss_dtlsmgr_warn("%p: failed to configure keys", ctx);
		vfree(keys);
		crypto_free_aead(dtls->aead);
		return -ENOSPC;
	}

	nss_cryptoapi_aead_ctx2session(dtls->aead, &dtls->crypto_idx);
	dtls->blk_len = (uint8_t)crypto_aead_blocksize(dtls->aead);
	dtls->hash_len = (uint8_t)crypto_aead_authsize(dtls->aead);
	dtls->iv_len = (uint8_t)crypto_aead_ivsize(dtls->aead);

	vfree(keys);
	return 0;
}

/*
 * nss_dtlsmgr_ctx_alloc_dtls()
 *	Allocate a DTLS session.
 */
static struct nss_dtlsmgr_dtls_data *nss_dtlsmgr_ctx_alloc_dtls(struct nss_dtlsmgr_ctx *ctx,
								struct nss_dtlsmgr_ctx_data *data,
								struct nss_dtlsmgr_crypto *crypto)
{
	struct nss_dtlsmgr_dtls_data *dtls;
	int error;

	nss_dtlsmgr_trace("%p: allocating context data(%u)", ctx, data->di_type);

	dtls = vzalloc(sizeof(*dtls));
	if (!dtls) {
		nss_dtlsmgr_warn("%p: failed to allocate dtls data(%u) ", ctx, data->di_type);
		return NULL;
	}

	INIT_LIST_HEAD(&dtls->list);

	error = nss_dtlsmgr_ctx_alloc_crypto(ctx, dtls, crypto);
	if (error < 0) {
		nss_dtlsmgr_warn("%p: unable to allocate crypto(%u) - error(%d)", ctx, data->di_type, error);
		vfree(dtls);
		return NULL;
	}

	nss_dtlsmgr_trace("%p: crypto_aead allocated", ctx);
	return dtls;
}

/*
 * nss_dtlsmgr_ctx_free_dtls()
 *	Free the DTLS context.
 */
static void nss_dtlsmgr_ctx_free_dtls(struct nss_dtlsmgr_dtls_data *dtls)
{
	crypto_free_aead(dtls->aead);
	vfree(dtls);
}

/*
 * nss_dtlsmgr_ctx_configure_hdr()
 *	Configure the DTLS header related information.
 */
static bool nss_dtlsmgr_ctx_configure_hdr(struct nss_dtlsmgr_ctx_data *data)
{
	const uint32_t type = NSS_DTLS_CMN_MSG_TYPE_CONFIGURE_HDR;
	enum nss_dtls_cmn_error resp = NSS_DTLS_CMN_ERROR_NONE;
	struct nss_dtls_cmn_ctx_config_hdr *cfg;
	struct nss_dtls_cmn_msg ndcm = { {0} };
	nss_tx_status_t status;
	uint32_t mask = 0;

	BUG_ON(in_atomic());

	mask |= NSS_DTLS_CMN_CTX_HDR_IPV6;
	mask |= NSS_DTLS_CMN_CTX_HDR_UDPLITE;
	mask |= NSS_DTLS_CMN_CTX_HDR_CAPWAP;
	mask |= NSS_DTLS_CMN_CTX_CIPHER_MODE_GCM;
	mask |= NSS_DTLS_CMN_CTX_OUTER_UDPLITE_CSUM;
	mask |= NSS_DTLS_CMN_CTX_INNER_ACCEPT_ALL;

	cfg = &ndcm.msg.hdr_cfg;
	cfg->flags = data->flags & mask;
	cfg->dest_ifnum = data->dest_ifnum;
	cfg->src_ifnum = data->src_ifnum;

	memcpy(cfg->sip, data->flow.sip, sizeof(cfg->sip));
	memcpy(cfg->dip, data->flow.dip, sizeof(cfg->dip));

	cfg->sport = data->flow.sport;
	cfg->dport = data->flow.dport;
	cfg->hop_limit_ttl = data->flow.hop_limit_ttl;
	cfg->dscp = data->flow.dscp;
	cfg->dscp_copy = data->flow.dscp_copy;
	cfg->df = data->flow.df;

	nss_dtlsmgr_trace("flags:0x%x dest_ifnum:0x%x src_ifnum:0x%x sport:0x%x dport:0x%x sip:0x%x dip:0x%x",
			  cfg->flags, cfg->dest_ifnum, cfg->src_ifnum, cfg->sport, cfg->dport,
			  cfg->sip[0], cfg->dip[0]);

	status = nss_dtls_cmn_tx_msg_sync(data->nss_ctx, data->ifnum, type, sizeof(*cfg), &ndcm, &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_warn("%p: msg_sync failed, if_num(%u), status(%d), type(%d), resp(%d)",
				 data, data->ifnum, type, status, resp);
		return false;
	}

	return true;
}

/*
 * nss_dtlsmgr_ctx_configure_dtls()
 *	Configure the DTLS version, crypto related data, window size and epoch.
 */
static bool nss_dtlsmgr_ctx_configure_dtls(struct nss_dtlsmgr_ctx_data *data, struct nss_dtlsmgr_dtls_data *dtls)
{
	const uint32_t type = NSS_DTLS_CMN_MSG_TYPE_CONFIGURE_DTLS;
	enum nss_dtls_cmn_error resp = NSS_DTLS_CMN_ERROR_NONE;
	struct nss_dtls_cmn_ctx_config_dtls *cfg;
	struct nss_dtls_cmn_msg ndcm = {0};
	nss_tx_status_t status;

	BUG_ON(in_atomic());

	cfg = &ndcm.msg.dtls_cfg;
	cfg->ver = dtls->ver;
	cfg->crypto_idx = dtls->crypto_idx;
	cfg->epoch = dtls->epoch;
	cfg->window_size = dtls->window_size;
	cfg->iv_len = dtls->iv_len;
	cfg->hash_len = dtls->hash_len;
	cfg->blk_len = dtls->blk_len;

	status = nss_dtls_cmn_tx_msg_sync(data->nss_ctx, data->ifnum, type, sizeof(*cfg), &ndcm, &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_warn("%p: msg_sync failed, if_num(%u), status(%d), type(%d), resp(%d)",
				data, data->ifnum, type, status, resp);
		return false;
	}

	return true;
}

/*
 * nss_dtlsmgr_ctx_deconfigure()
 *	Deconfigure the DTLS context and free all the related data.
 */
static bool nss_dtlsmgr_ctx_deconfigure(struct nss_dtlsmgr_ctx *ctx, struct nss_dtlsmgr_ctx_data *data)
{
	const uint32_t type = NSS_DTLS_CMN_MSG_TYPE_DECONFIGURE;
	enum nss_dtls_cmn_error resp = NSS_DTLS_CMN_ERROR_NONE;
	struct nss_dtls_cmn_msg ndcm = {0};
	struct nss_dtlsmgr_dtls_data *cur;
	nss_tx_status_t status;

	status = nss_dtls_cmn_tx_msg_sync(data->nss_ctx, data->ifnum, type, 0, &ndcm, &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_warn("%p: msg_sync failed, if_num(%u), status(%d), type(%d), resp(%d)",
				ctx, data->ifnum, type, status, resp);
		return false;
	}

	nss_dtls_cmn_unregister_if(data->ifnum);

	for (;;) {
		write_lock(&ctx->lock);
		cur = list_first_entry_or_null(&data->dtls_active, struct nss_dtlsmgr_dtls_data, list);
		if (!cur) {
			write_unlock(&ctx->lock);
			break;
		}

		list_del(&cur->list);
		write_unlock(&ctx->lock);
		nss_dtlsmgr_ctx_free_dtls(cur);
	}

	status = nss_dynamic_interface_dealloc_node(data->ifnum, data->di_type);
	if (status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_warn("%p: fail to deallocate dynamic(%d) interface(%u)", ctx, data->di_type, data->ifnum);
		return false;
	}

	data->ifnum = -1;
	return true;
}

/*
 * nss_dtlsmgr_ctx_create_encap()
 *	Create DTLS encapsulation dynamic interface and configure the DTLS context.
 */
static int nss_dtlsmgr_ctx_create_encap(struct nss_dtlsmgr_ctx *ctx, uint32_t ifnum,
					uint32_t src_ifnum, struct nss_dtlsmgr_config *ndc)
{
	struct nss_dtlsmgr_encap_config *cfg = &ndc->encap;
	struct nss_dtlsmgr_ctx_data *data = &ctx->encap;
	struct nss_dtlsmgr_flow_data *flow = &data->flow;
	struct nss_dtlsmgr_dtls_data *dtls;
	uint32_t mask;

	dtls = nss_dtlsmgr_ctx_alloc_dtls(ctx, &ctx->encap, &ndc->encap.crypto);
	if (!dtls) {
		nss_dtlsmgr_warn("%p: unable to allocate encap context data", ctx);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&data->dtls_active);

	data->di_type = NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_INNER;
	data->ifnum = ifnum;
	data->src_ifnum = src_ifnum;
	data->flags = ndc->flags;
	data->tailroom = dtls->blk_len + dtls->hash_len;
	data->headroom = dtls->iv_len;

	memcpy(&flow->sip, cfg->sip, sizeof(flow->sip));
	memcpy(&flow->dip, cfg->dip, sizeof(flow->dip));

	flow->sport = cfg->sport;
	flow->dport = cfg->dport;
	flow->dscp = cfg->dscp;
	flow->dscp_copy = cfg->dscp_copy;
	flow->df = cfg->df;
	flow->hop_limit_ttl = cfg->ip_ttl;

	dtls->epoch = cfg->epoch;
	dtls->ver = cfg->ver;

	mask = NSS_DTLSMGR_HDR_IPV6 | NSS_DTLSMGR_HDR_CAPWAP;

	data->headroom += NSS_DTLSMGR_DTLS_HDR_SZ;

	/*
	 * We need to provide the firmware the source and
	 * destination interface number. This allows it
	 * to work with dynamically created interfaces
	 *
	 */
	switch (mask & ndc->flags) {
	case NSS_DTLSMGR_HDR_IPV6 | NSS_DTLSMGR_HDR_CAPWAP:
		data->dest_ifnum = NSS_IPV6_RX_INTERFACE;
		data->headroom += sizeof(struct ipv6hdr);
		data->headroom += NSS_DTLSMGR_CAPWAP_DTLS_HDR_SZ;
		break;
	case NSS_DTLSMGR_HDR_IPV6:
		data->dest_ifnum = NSS_IPV6_RX_INTERFACE;
		data->headroom += sizeof(struct ipv6hdr);
		break;
	case NSS_DTLSMGR_HDR_CAPWAP:
		data->dest_ifnum = NSS_IPV4_RX_INTERFACE;
		data->headroom += sizeof(struct iphdr);
		data->headroom += NSS_DTLSMGR_CAPWAP_DTLS_HDR_SZ;
		break;
	default:
		data->dest_ifnum = NSS_IPV4_RX_INTERFACE;
		data->headroom += sizeof(struct iphdr);
		break;
	}

	/*
	 * Header size is same for UDP and UDPLite
	 */
	data->headroom += sizeof(struct udphdr);

	nss_dtlsmgr_trace("%p: encap ifnum(%u), src(%u), dest(0x%x)", ctx, data->ifnum,
			  data->src_ifnum, data->dest_ifnum);

	/*
	 * Register NSS DTLS Encap I/F
	 */
	data->nss_ctx = nss_dtls_cmn_register_if(data->ifnum,
						nss_dtlsmgr_ctx_dev_rx_inner,
						nss_dtlsmgr_ctx_dev_event_inner,
						ctx->dev,
						0,
						data->di_type,
						(void *)data);
	if (!data->nss_ctx) {
		nss_dtlsmgr_warn("%p: NSS register interface(%u) failed", ctx, data->ifnum);
		nss_dtlsmgr_ctx_free_dtls(dtls);
		return -ENODEV;
	}

	if (!nss_dtlsmgr_ctx_configure_hdr(data)) {
		nss_dtlsmgr_warn("%p: unable to configure(%d) hdr", ctx, data->di_type);
		goto fail;
	}

	if (!nss_dtlsmgr_ctx_configure_dtls(data, dtls)) {
		nss_dtlsmgr_warn("%p: unable to configure(%d) dtls", ctx, data->di_type);
		goto fail;
	}

	write_lock(&ctx->lock);
	list_add(&dtls->list, &data->dtls_active);
	write_unlock(&ctx->lock);

	return 0;
fail:
	nss_dtls_cmn_unregister_if(data->ifnum);
	nss_dtlsmgr_ctx_free_dtls(dtls);
	return -EBUSY;
}

/*
 * nss_dtlsmgr_ctx_create_decap()
 *	Create DTLS decapsulation dynamic interface and configure the DTLS context.
 */
static int nss_dtlsmgr_ctx_create_decap(struct nss_dtlsmgr_ctx *ctx, uint32_t ifnum, uint32_t src_ifnum,
					struct nss_dtlsmgr_config *cfg)
{
	struct nss_dtlsmgr_ctx_data *data = &ctx->decap;
	struct nss_dtlsmgr_dtls_data *dtls;

	dtls = nss_dtlsmgr_ctx_alloc_dtls(ctx, &ctx->decap, &cfg->decap.crypto);
	if (!dtls) {
		nss_dtlsmgr_warn("%p: unable to allocate decap context data", ctx);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&data->dtls_active);

	data->di_type = NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_OUTER;
	data->ifnum = ifnum;

	/*
	 * We need to provide the firmware the source and
	 * destination interface number. This allows it
	 * to work with dynamically created interfaces
	 *
	 */
	data->src_ifnum = src_ifnum;
	data->dest_ifnum = cfg->decap.nexthop_ifnum;
	data->tailroom = data->headroom = 0;
	data->flags = cfg->flags;

	nss_dtlsmgr_trace("%p: decap ifnum(%u), src(%u), dest(%u)", ctx, data->ifnum,
			  data->src_ifnum, data->dest_ifnum);

	dtls->window_size = cfg->decap.window_size;
	dtls->ver = cfg->encap.ver;

	/*
	 * Register NSS DTLS Decap I/F
	 */
	data->nss_ctx = nss_dtls_cmn_register_if(data->ifnum,
						nss_dtlsmgr_ctx_dev_rx_outer,
						nss_dtlsmgr_ctx_dev_event_outer,
						ctx->dev,
						0,
						data->di_type,
						(void *)data);
	if (!data->nss_ctx) {
		nss_dtlsmgr_warn("%p: NSS register interface(%u) failed", ctx, data->ifnum);
		nss_dtlsmgr_ctx_free_dtls(dtls);
		return -ENODEV;
	}

	if (!nss_dtlsmgr_ctx_configure_hdr(data)) {
		nss_dtlsmgr_warn("%p: unable to configure(%d) hdr", ctx, data->di_type);
		goto fail;
	}

	if (!nss_dtlsmgr_ctx_configure_dtls(data, dtls)) {
		nss_dtlsmgr_warn("%p: unable to configure(%d) hdr", ctx, data->di_type);
		goto fail;
	}

	write_lock(&ctx->lock);
	list_add(&dtls->list, &data->dtls_active);
	write_unlock(&ctx->lock);

	return 0;
fail:
	nss_dtls_cmn_unregister_if(data->ifnum);
	nss_dtlsmgr_ctx_free_dtls(dtls);
	return -EBUSY;
}

/*
 * nss_dtlsmgr_session_switch()
 *	Send a switch message to firmware to use new cipher spec
 *
 * Note: This deletes the older cipher spec and pops the next cipher spec
 * for use.
 */
static bool nss_dtlsmgr_session_switch(struct nss_dtlsmgr_ctx *ctx, struct nss_dtlsmgr_ctx_data *data)
{
	const uint32_t type = NSS_DTLS_CMN_MSG_TYPE_SWITCH_DTLS;
	enum nss_dtls_cmn_error resp = NSS_DTLS_CMN_ERROR_NONE;
	struct nss_dtls_cmn_msg ndcm = {0};
	struct nss_dtlsmgr_dtls_data *dtls;
	nss_tx_status_t status;

	BUG_ON(in_atomic());

	/*
	 * TODO: Add retry messaging to ensure that in case of failures, due to queue
	 * full conditions we do attempt few retries before aborting.
	 */
	status = nss_dtls_cmn_tx_msg_sync(data->nss_ctx, data->ifnum, type, 0, &ndcm, &resp);
	if (status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_warn("%p: msg_sync failed, if_num(%u), status(%d), type(%d), resp(%d)",
				ctx, data->ifnum, type, status, resp);
		return false;
	}

	/*
	 * We essentially pop the head of the dtls list.
	 * It is expected that an update should have already
	 * added a new dtls entry at the tail of the list
	 */
	write_lock(&ctx->lock);
	dtls = list_first_entry_or_null(&data->dtls_active, struct nss_dtlsmgr_dtls_data, list);
	if (!dtls) {
		write_unlock(&ctx->lock);
		return false;
	}

	list_del(&dtls->list);
	write_unlock(&ctx->lock);

	nss_dtlsmgr_ctx_free_dtls(dtls);
	return true;
}

/*
 * nss_dtlsmgr_session_create()
 *	Create DTLS session and associated crypto sessions.
 */
struct net_device *nss_dtlsmgr_session_create(struct nss_dtlsmgr_config *cfg)
{
	struct nss_dtlsmgr *drv = &g_dtls;
	struct nss_dtlsmgr_ctx *ctx;
	struct net_device *dev;
	int32_t encap_ifnum;
	int32_t decap_ifnum;
	int error;

	if (!atomic_read(&drv->is_configured)) {
		nss_dtlsmgr_warn("%p: dtls firmware not ready", drv);
		return NULL;
	}

	if ((cfg->encap.ver != NSS_DTLSMGR_VERSION_1_0) && (cfg->encap.ver != NSS_DTLSMGR_VERSION_1_2)) {
		nss_dtlsmgr_warn("%p: invalid encapsulation version(%d)", drv, cfg->encap.ver);
		return NULL;
	}

	encap_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_INNER);
	if (encap_ifnum < 0) {
		nss_dtlsmgr_warn("%p: failed to allocate encap dynamic interface(%u)", drv, encap_ifnum);
		return NULL;
	}

	decap_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_OUTER);
	if (decap_ifnum < 0) {
		nss_dtlsmgr_warn("%p: failed to allocate decap dynamic interface(%u)", drv, decap_ifnum);
		goto dealloc_encap_node;
	}

	nss_dtlsmgr_trace("dynamic interfaces, encap(%u), decap(%u)", encap_ifnum, decap_ifnum);

	dev = alloc_netdev(sizeof(*ctx), "dtls%d", NET_NAME_ENUM, nss_dtlsmgr_ctx_dev_setup);
	if (!dev) {
		nss_dtlsmgr_warn("%p: unable to allocate dtls device", ctx);
		goto dealloc_decap_node;
	}

	ctx = netdev_priv(dev);
	ctx->dev = dev;
	rwlock_init(&ctx->lock);

	NSS_DTLSMGR_SET_MAGIC(ctx, NSS_DTLSMGR_CTX_MAGIC);

	error = nss_dtlsmgr_ctx_create_encap(ctx, encap_ifnum, decap_ifnum, cfg);
	if (error < 0) {
		nss_dtlsmgr_warn("%p: unable to create encap context, error(%d)", ctx, error);
		goto free_dev;
	}

	error = nss_dtlsmgr_ctx_create_decap(ctx, decap_ifnum, encap_ifnum, cfg);
	if (error < 0) {
		nss_dtlsmgr_warn("%p: unable to create decap context, error(%d)", ctx, error);
		goto destroy_encap;
	}

	/*
	 * Set the needed headroom and tailroom as a multiple of 4 bytes
	 * so that the skb data pointer remains 4 byte aligned when the
	 * headroom/tailroom is adjusted.
	 */
	dev->needed_headroom = ALIGN(ctx->encap.headroom, 4);
	dev->needed_tailroom = ALIGN(ctx->encap.tailroom, 4);

	ctx->app_data = cfg->app_data;
	ctx->notify_cb = cfg->notify;
	ctx->data_cb = cfg->data;

	/*
	 * If, the user has not provided the callback function then
	 * we will register the default callback handler
	 */
	if (!ctx->data_cb) {
		ctx->data_cb = nss_dtlsmgr_ctx_dev_data_callback;
		ctx->app_data = ctx;
	}

	error = rtnl_is_locked() ? register_netdevice(dev) : register_netdev(dev);
	if (error < 0) {
		nss_dtlsmgr_warn("%p: unable register net_device(%s)", ctx, dev->name);
		goto destroy_decap;
	}

	dev->mtu = dev->mtu - (ctx->encap.headroom + ctx->encap.tailroom);

	nss_dtlsmgr_trace("%p: dtls session(%s) created, encap(%u), decap(%u)",
			  ctx, dev->name, ctx->encap.ifnum, ctx->decap.ifnum);

	if (nss_dtlsmgr_create_debugfs(ctx)) {
		nss_dtlsmgr_warn("Failed to create debugfs for ctx(%p)", ctx);
	}

	return dev;

destroy_decap:
	nss_dtlsmgr_ctx_deconfigure(ctx, &ctx->decap);

destroy_encap:
	nss_dtlsmgr_ctx_deconfigure(ctx, &ctx->encap);

free_dev:
	free_netdev(dev);

dealloc_decap_node:
	nss_dynamic_interface_dealloc_node(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_OUTER);

dealloc_encap_node:
	nss_dynamic_interface_dealloc_node(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_INNER);
	return NULL;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_create);

/*
 * nss_dtlsmgr_session_destroy()
 *	Destroy DTLS session
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_destroy(struct net_device *dev)
{
	struct nss_dtlsmgr_ctx *ctx = netdev_priv(dev);
	NSS_DTLSMGR_VERIFY_MAGIC(ctx);

	/*
	 * Reset the callback handlers atomically
	 */
	xchg(&ctx->notify_cb, NULL);
	xchg(&ctx->data_cb, NULL);

	nss_dtlsmgr_trace("%p: destroying encap(%u) and decap(%u) sessions",
			  ctx, ctx->encap.ifnum, ctx->decap.ifnum);

	if (!nss_dtlsmgr_ctx_deconfigure(ctx, &ctx->encap)) {
		nss_dtlsmgr_warn("%p: unable to deconfigure encap", ctx);
		return NSS_DTLSMGR_FAIL;
	}

	if (!nss_dtlsmgr_ctx_deconfigure(ctx, &ctx->decap)) {
		nss_dtlsmgr_warn("%p: unable to deconfigure decap", ctx);
		return NSS_DTLSMGR_FAIL;
	}

	NSS_DTLSMGR_SET_MAGIC(ctx, 0);

	rtnl_is_locked() ? unregister_netdevice(dev) : unregister_netdev(dev);

	return NSS_DTLSMGR_OK;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_destroy);

/*
 * nss_dtlsmgr_session_update_encap()
 *	Update the encapsulation crypto keys.
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_update_encap(struct net_device *dev, struct nss_dtlsmgr_config_update *cfg)
{
	struct nss_dtlsmgr_ctx *ctx = netdev_priv(dev);
	struct nss_dtlsmgr_ctx_data *data = &ctx->encap;
	struct nss_dtlsmgr_dtls_data *dtls, *prev_dtls;

	NSS_DTLSMGR_VERIFY_MAGIC(ctx);

	dtls = nss_dtlsmgr_ctx_alloc_dtls(ctx, &ctx->encap, &cfg->crypto);
	if (!dtls) {
		nss_dtlsmgr_warn("%p: unable to update encap context data", ctx);
		return NSS_DTLSMGR_FAIL_NOMEM;
	}

	/*
	 * Get the first entry in the list to compare the crypto key lengths
	 */
	prev_dtls = list_first_entry_or_null(&data->dtls_active, struct nss_dtlsmgr_dtls_data, list);
	if (!prev_dtls) {
		nss_dtlsmgr_warn("%p: dtls list is emtpy\n", ctx);
		return NSS_DTLSMGR_FAIL_NOCRYPTO;
	}

	/*
	 * If the new keys lengths are longer, then there isn't enough headroom and tailroom.
	 */
	BUG_ON(prev_dtls->iv_len < dtls->iv_len);
	BUG_ON(prev_dtls->blk_len < dtls->blk_len);
	BUG_ON(prev_dtls->hash_len < dtls->hash_len);

	nss_dtlsmgr_trace("%p: encap context update allocated (%u)", ctx, ctx->encap.ifnum);

	dtls->epoch = cfg->epoch;
	dtls->window_size = cfg->window_size;

	if (!nss_dtlsmgr_ctx_configure_dtls(&ctx->encap, dtls)) {
		nss_dtlsmgr_warn("%p: unable to configure encap dtls", ctx);
		nss_dtlsmgr_ctx_free_dtls(dtls);
		return NSS_DTLSMGR_FAIL_MESSAGE;
	}

	write_lock(&ctx->lock);
	list_add_tail(&dtls->list, &ctx->encap.dtls_active);
	write_unlock(&ctx->lock);

	nss_dtlsmgr_trace("%p: encap context update done", ctx);
	return NSS_DTLSMGR_OK;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_update_encap);

/*
 * nss_dtlsmgr_session_update_decap()
 *	Update the decapsulation crypto keys.
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_update_decap(struct net_device *dev, struct nss_dtlsmgr_config_update *cfg)
{
	struct nss_dtlsmgr_ctx *ctx = netdev_priv(dev);
	struct nss_dtlsmgr_dtls_data *dtls;

	NSS_DTLSMGR_VERIFY_MAGIC(ctx);

	dtls = nss_dtlsmgr_ctx_alloc_dtls(ctx, &ctx->decap, &cfg->crypto);
	if (!dtls) {
		nss_dtlsmgr_warn("%p: unable to update decap context data", ctx);
		return NSS_DTLSMGR_FAIL_NOMEM;
	}

	nss_dtlsmgr_trace("%p: decap context update allocated (%u)", ctx, ctx->decap.ifnum);

	dtls->epoch = cfg->epoch;
	dtls->window_size = cfg->window_size;

	if (!nss_dtlsmgr_ctx_configure_dtls(&ctx->decap, dtls)) {
		nss_dtlsmgr_warn("%p: unable to configure decap dtls", ctx);
		nss_dtlsmgr_ctx_free_dtls(dtls);
		return NSS_DTLSMGR_FAIL_MESSAGE;
	}

	write_lock(&ctx->lock);
	list_add_tail(&dtls->list, &ctx->decap.dtls_active);
	write_unlock(&ctx->lock);

	nss_dtlsmgr_trace("%p: decap context update done", ctx);
	return NSS_DTLSMGR_OK;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_update_decap);

/*
 * nss_dtlsmgr_session_switch_encap()
 *	Send a message to encapsulation DTLS interface to switch to the new crypto keys.
 */
bool nss_dtlsmgr_session_switch_encap(struct net_device *dev)
{
	struct nss_dtlsmgr_ctx *ctx = netdev_priv(dev);
	struct nss_dtlsmgr_ctx_data *data = &ctx->encap;

	NSS_DTLSMGR_VERIFY_MAGIC(ctx);

	if (!nss_dtlsmgr_session_switch(ctx, data)) {
		nss_dtlsmgr_warn("%p: failed to send encap switch_dtls(%u)", ctx, data->ifnum);
		return false;
	}

	nss_dtlsmgr_trace("%p: encap(%u) cipher switch done", ctx, data->ifnum);
	return true;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_switch_encap);

/*
 * nss_dtlsmgr_session_switch_decap()
 *	Send a message to decapsulation DTLS interface to switch to the new crypto keys.
 */
bool nss_dtlsmgr_session_switch_decap(struct net_device *dev)
{
	struct nss_dtlsmgr_ctx *ctx = netdev_priv(dev);
	struct nss_dtlsmgr_ctx_data *data = &ctx->decap;

	NSS_DTLSMGR_VERIFY_MAGIC(ctx);

	if (!nss_dtlsmgr_session_switch(ctx, data)) {
		nss_dtlsmgr_warn("%p: failed to send decap switch_dtls(%u)", ctx, data->ifnum);
		return false;
	}

	nss_dtlsmgr_trace("%p: decap(%u) cipher switch done", ctx, data->ifnum);
	return true;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_switch_decap);

/*
 * nss_dtlsmgr_get_interface()
 *	Returns NSS DTLS interface number for encap/decap on success.
 */
int32_t nss_dtlsmgr_get_interface(struct net_device *dev, enum nss_dtlsmgr_interface_type type)
{
	int32_t ifnum;

	switch (type) {
	case NSS_DTLSMGR_INTERFACE_TYPE_INNER:
		ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_INNER);
		break;

	case NSS_DTLSMGR_INTERFACE_TYPE_OUTER:
		ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_DTLS_CMN_OUTER);
		break;

	default:
		nss_dtlsmgr_warn("%p: invalid interface type %d", dev, type);
		return -EINVAL;
	}

	if (ifnum < 0) {
		nss_dtlsmgr_warn("%p: couldn't find DTLS interface number (%d)", dev, ifnum);
		return ifnum;
	}

	return nss_dtls_cmn_get_ifnum(ifnum);
}
EXPORT_SYMBOL(nss_dtlsmgr_get_interface);
