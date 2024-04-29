/*
 **************************************************************************
 * Copyright (c) 2016-2017, 2020, The Linux Foundation. All rights reserved.
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
 * nss_connmgr_dtls.c
 *	NSS DTLS Manager
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/atomic.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include "nss_connmgr_dtls.h"

/*
 * Global DTLS context
 */
static struct nss_dtlsmgr_ctx g_ctx;

static struct nss_dtlsmgr_crypto_algo dtlsmgr_algo[NSS_DTLSMGR_ALGO_MAX] = {
	[NSS_DTLSMGR_ALGO_AES_CBC_SHA1_HMAC] = {
		.cipher_algo = NSS_CRYPTO_CIPHER_AES_CBC,
		.auth_algo = NSS_CRYPTO_AUTH_SHA1_HMAC,
		.hash_len = NSS_CRYPTO_MAX_HASHLEN_SHA1 ,
		.iv_len = NSS_CRYPTO_MAX_IVLEN_AES,
	},
	[NSS_DTLSMGR_ALGO_AES_CBC_SHA256_HMAC] = {
		.cipher_algo = NSS_CRYPTO_CIPHER_AES_CBC,
		.auth_algo = NSS_CRYPTO_AUTH_SHA256_HMAC,
		.hash_len = NSS_CRYPTO_MAX_HASHLEN_SHA256 ,
		.iv_len = NSS_CRYPTO_MAX_IVLEN_AES,
	},
	[NSS_DTLSMGR_ALGO_3DES_CBC_SHA1_HMAC] = {
		.cipher_algo = NSS_CRYPTO_CIPHER_DES,
		.auth_algo = NSS_CRYPTO_AUTH_SHA1_HMAC,
		.hash_len = NSS_CRYPTO_MAX_HASHLEN_SHA1 ,
		.iv_len = NSS_CRYPTO_MAX_IVLEN_DES,
	},
	[NSS_DTLSMGR_ALGO_3DES_CBC_SHA256_HMAC] = {
		.cipher_algo = NSS_CRYPTO_CIPHER_DES,
		.auth_algo = NSS_CRYPTO_AUTH_SHA256_HMAC,
		.hash_len = NSS_CRYPTO_MAX_HASHLEN_SHA256,
		.iv_len = NSS_CRYPTO_MAX_IVLEN_DES,
	}
};

/*
 * nss_dtlsmgr_session_insert()
 *	Insert a DTLS session into global list of sessions.
 *	Must be called with global context lock held.
 */
static bool nss_dtlsmgr_session_insert(struct nss_dtlsmgr_session *s)
{
	int32_t i;

	assert_spin_locked(&g_ctx.lock);

	for (i = 0; i < NSS_MAX_DTLS_SESSIONS; i++) {
		if (g_ctx.session[i] == NULL) {
			g_ctx.session[i] = s;
			return true;
		}
	}

	return false;
}

/*
 * nss_dtlsmgr_session_remove()
 *	Remove a DTLS session from list of sessions.
 *	Must be called with global context lock held.
 */
static struct nss_dtlsmgr_session *nss_dtlsmgr_session_remove(uint32_t sif)
{
	int32_t i;
	struct nss_dtlsmgr_session *s;

	assert_spin_locked(&g_ctx.lock);

	for (i = 0; i < NSS_MAX_DTLS_SESSIONS; i++) {
		s = g_ctx.session[i];
		if (!s)
			continue;

		nss_dtlsmgr_assert(s->magic == NSS_DTLSMGR_SESSION_MAGIC);
		if (s->nss_dtls_if == sif) {
			g_ctx.session[i] = NULL;
			return s;
		}
	}

	return NULL;
}

/*
 * nss_dtlsmgr_session_find()
 *	Find a DTLS session from list of sessions.
 *	Must be called with global context lock held.
 */
static struct nss_dtlsmgr_session *nss_dtlsmgr_session_find(uint32_t sif)
{
	int32_t i;
	struct nss_dtlsmgr_session *s;

	assert_spin_locked(&g_ctx.lock);

	for (i = 0; i < NSS_MAX_DTLS_SESSIONS; i++) {
		s = g_ctx.session[i];
		if (!s)
			continue;

		nss_dtlsmgr_assert(s->magic == NSS_DTLSMGR_SESSION_MAGIC);
		if (s->nss_dtls_if == sif)
			return s;
	}

	return NULL;
}

/*
 * nss_dtlsmgr_session_cleanup()
 *	Cleanup DTLS session
 */
static void nss_dtlsmgr_session_cleanup(struct nss_dtlsmgr_session *ds)
{
	nss_crypto_status_t crypto_status;

	nss_dtlsmgr_info("%px: DTLS session I/F %u cleanup\n",
			 &g_ctx, ds->nss_dtls_if);

	nss_dtls_unregister_if(ds->nss_dtls_if);

	nss_dtlsmgr_netdev_destroy(ds);

	nss_dynamic_interface_dealloc_node(ds->nss_dtls_if,
					   NSS_DYNAMIC_INTERFACE_TYPE_DTLS);

	crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl,
						ds->crypto_idx_encap);
	if (crypto_status != NSS_CRYPTO_STATUS_OK) {
		nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n",
				 &g_ctx, ds->nss_dtls_if, ds->crypto_idx_encap);
	}

	crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl,
						ds->crypto_idx_decap);
	if (crypto_status != NSS_CRYPTO_STATUS_OK) {
		nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n",
				 &g_ctx, ds->nss_dtls_if, ds->crypto_idx_decap);
	}

	if (ds->cidx_decap_pending != NSS_CRYPTO_MAX_IDXS) {
		crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl,
							ds->cidx_decap_pending);
		if (crypto_status != NSS_CRYPTO_STATUS_OK) {
			nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n", &g_ctx, ds->nss_dtls_if, ds->cidx_decap_pending);
		}
	}

	if (ds->cidx_encap_pending != NSS_CRYPTO_MAX_IDXS) {
		crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl,
							ds->cidx_encap_pending);
		if (crypto_status != NSS_CRYPTO_STATUS_OK) {
			nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n", &g_ctx, ds->nss_dtls_if, ds->cidx_encap_pending);
		}
	}

	ds->magic = 0;
	kfree(ds);
}

/*
 * nss_dtlsmgr_session_find_and_ref()
 *	Find a DTLS session from list of sessions and
 *	increments reference count. Must be called with
 *	global context lock held.
 */
static struct nss_dtlsmgr_session *nss_dtlsmgr_session_find_and_ref(uint32_t sif)
{
	struct nss_dtlsmgr_session *s;

	s = nss_dtlsmgr_session_find(sif);
	if (atomic_inc_and_test(&s->ref)) {
		nss_dtlsmgr_assert(false);
	}

	return s;
}

/*
 * nss_dtlsmgr_session_ref_dec()
 *	Decrement reference count of a DTLS session.
 *	Perform session cleanup if reference count falls to zero.
 */
static void nss_dtlsmgr_session_ref_dec(struct nss_dtlsmgr_session *s)
{
	if (atomic_dec_and_test(&s->ref)) {
		nss_dtlsmgr_session_cleanup(s);
	}
}

/*
 * nss_connmgr_dtls_data_receive()
 *	Handler to receive packets from NSS.
 */
static void nss_connmgr_dtls_data_receive(struct net_device *dev,
					  struct sk_buff *skb,
					  struct napi_struct *napi)
{
	struct nss_dtlsmgr_netdev_priv *priv;
	struct nss_dtlsmgr_session *s;
	__be32 meta;

	BUG_ON(dev == NULL);
	BUG_ON(skb == NULL);

	dev_hold(dev);
	priv = netdev_priv(dev);
	s = priv->s;

	/*
	 * Get DTLS metadata
	 */
	meta = *(__be32 *)skb->data;
	meta = ntohl(meta);
	if (NSS_DTLSMGR_METADATA_CTYPE(meta) != NSS_DTLSMGR_CTYPE_APP) {
		nss_dtlsmgr_info("%px: Dropping non app dtls pkt\n", skb);
		dev_kfree_skb_any(skb);
		dev_put(dev);
		return;
	}

	if (NSS_DTLSMGR_METADATA_ERROR(meta) != NSS_DTLSMGR_METADATA_ERROR_OK) {
		nss_dtlsmgr_info("%px: Dropping error pkt\n", skb);
		dev_kfree_skb_any(skb);
		dev_put(dev);
		return;
	}

	/*
	 * Remove four bytes at start of
	 * buffer containing the DTLS metadata.
	 */
	skb_pull(skb, NSS_DTLSMGR_METADATA_LEN);

	skb_reset_network_header(skb);
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = dev->ifindex;
	skb->dev = dev;
	if (s->flags & NSS_DTLSMGR_HDR_IPV6)
		skb->protocol = htons(ETH_P_IPV6);
	else
		skb->protocol = htons(ETH_P_IP);

	netif_receive_skb(skb);
	dev_put(dev);
}

/*
 * nss_connmgr_dtls_event_receive()
 *	Event Callback to receive events from NSS
 */
static void nss_connmgr_dtls_event_receive(void *if_ctx,
					   struct nss_dtls_msg *tnlmsg)
{
	struct nss_dtlsmgr_session *ds = (struct nss_dtlsmgr_session *)if_ctx;
	struct nss_dtlsmgr_session_stats_update stats;
	struct nss_dtls_session_stats *msg_stats;

	spin_lock(&g_ctx.lock);
	ds = nss_dtlsmgr_session_find_and_ref(tnlmsg->cm.interface);
	spin_unlock(&g_ctx.lock);

	if (!ds) {
		return;
	}

	switch (tnlmsg->cm.type) {
	case NSS_DTLS_MSG_SESSION_STATS:
		if (ds->stats_update_cb == NULL)
			break;

		memset(&stats, 0, sizeof(struct nss_dtlsmgr_session_stats_update));
		msg_stats = &tnlmsg->msg.stats;

		stats.tx_pkts = msg_stats->node_stats.tx_packets;
		stats.rx_pkts = msg_stats->node_stats.rx_packets;
		stats.rx_dropped = nss_cmn_rx_dropped_sum(&msg_stats->node_stats);
		stats.tx_auth_done = msg_stats->tx_auth_done;
		stats.rx_auth_done = msg_stats->rx_auth_done;
		stats.tx_cipher_done = msg_stats->tx_cipher_done;
		stats.rx_cipher_done = msg_stats->rx_cipher_done;
		stats.tx_cbuf_alloc_fail = msg_stats->tx_cbuf_alloc_fail;
		stats.rx_cbuf_alloc_fail = msg_stats->rx_cbuf_alloc_fail;
		stats.tx_cenqueue_fail = msg_stats->tx_cenqueue_fail;
		stats.rx_cenqueue_fail = msg_stats->rx_cenqueue_fail;
		stats.tx_dropped_hroom = msg_stats->tx_dropped_hroom;
		stats.tx_dropped_troom = msg_stats->tx_dropped_troom;
		stats.tx_forward_enqueue_fail = msg_stats->tx_forward_enqueue_fail;
		stats.rx_forward_enqueue_fail = msg_stats->rx_forward_enqueue_fail;
		stats.rx_invalid_version = msg_stats->rx_invalid_version;
		stats.rx_invalid_epoch = msg_stats->rx_invalid_epoch;
		stats.rx_malformed = msg_stats->rx_malformed;
		stats.rx_cipher_fail = msg_stats->rx_cipher_fail;
		stats.rx_auth_fail = msg_stats->rx_auth_fail;
		stats.rx_capwap_classify_fail = msg_stats->rx_capwap_classify_fail;
		stats.rx_replay_fail = msg_stats->rx_replay_fail;
		stats.rx_replay_duplicate = msg_stats->rx_replay_duplicate;
		stats.rx_replay_out_of_window = msg_stats->rx_replay_out_of_window;
		stats.outflow_queue_full = msg_stats->outflow_queue_full;
		stats.decap_queue_full = msg_stats->decap_queue_full;
		stats.pbuf_alloc_fail = msg_stats->pbuf_alloc_fail;
		stats.pbuf_copy_fail = msg_stats->pbuf_copy_fail;
		stats.epoch = msg_stats->epoch;
		stats.tx_seq_high = msg_stats->tx_seq_high;
		stats.tx_seq_low = msg_stats->tx_seq_low;

		ds->stats_update_cb(ds->nss_dtls_if, &stats);
		break;

	default:
		nss_dtlsmgr_info("%px: Unknown Event from NSS\n", &g_ctx);
		break;
	}

	nss_dtlsmgr_session_ref_dec(ds);
}

/*
 * nss_dtlsmgr_alloc_crypto()
 *	Allocate a crypto session and update encrypt/decrypt session parameters.
 */
nss_dtlsmgr_status_t nss_dtlsmgr_alloc_crypto(struct nss_dtlsmgr_crypto *crypto, uint32_t *crypto_idx, bool encrypt)
{
	struct nss_dtlsmgr_crypto_algo *algo;
	struct nss_crypto_params params;
	struct nss_crypto_key cipher;
	struct nss_crypto_key auth;

	memset(&cipher, 0, sizeof(struct nss_crypto_key));
	memset(&auth, 0, sizeof(struct nss_crypto_key));

	if (crypto->algo >= NSS_DTLSMGR_ALGO_MAX) {
		nss_dtlsmgr_info("%px: invalid algorithm type %d", &g_ctx, crypto->algo);
		return NSS_DTLSMGR_FAIL_NOCRYPTO;
	}

	algo = &dtlsmgr_algo[crypto->algo];

	cipher.algo = algo->cipher_algo;
	cipher.key = crypto->cipher_key.data;
	cipher.key_len = crypto->cipher_key.len;

	auth.algo = algo->auth_algo;
	auth.key = crypto->auth_key.data;
	auth.key_len = crypto->auth_key.len;

	if (nss_crypto_session_alloc(g_ctx.crypto_hdl, &cipher, &auth, crypto_idx) != NSS_CRYPTO_STATUS_OK) {
		nss_dtlsmgr_info("%px: DTLS crypto alloc failed\n", &g_ctx);
		return NSS_DTLSMGR_FAIL_NOCRYPTO;
	}

	/*
	 * Update crypto session
	 */
	memset(&params, 0, sizeof(struct nss_crypto_params));
	params.cipher_skip = NSS_DTLSMGR_HDR_LEN + algo->iv_len;
	params.auth_skip = 0;
	params.req_type = (encrypt ? NSS_CRYPTO_REQ_TYPE_ENCRYPT : NSS_CRYPTO_REQ_TYPE_DECRYPT);
	params.req_type |= NSS_CRYPTO_REQ_TYPE_AUTH;

	if (nss_crypto_session_update(g_ctx.crypto_hdl, *crypto_idx, &params) != NSS_CRYPTO_STATUS_OK) {
		nss_dtlsmgr_info("%px: failed to update crypto session %d", &g_ctx, *crypto_idx);
		nss_crypto_session_free(g_ctx.crypto_hdl, *crypto_idx);
		return NSS_DTLSMGR_FAIL_NOCRYPTO;
	}

	nss_dtlsmgr_info("%px: auth_skip:%d cipher_skip:%d\n", &g_ctx, params.auth_skip, params.cipher_skip);
	return NSS_DTLSMGR_OK;
}

/*
 * nss_dtlsmgr_session_create()
 *	Create DTLS session and associated crypto sessions.
 */
struct net_device *nss_dtlsmgr_session_create(struct nss_dtlsmgr_config *cfg)
{
	struct nss_dtlsmgr_session *ds;
	struct nss_dtls_msg dtlsmsg;
	int32_t i = 0;
	struct nss_dtlsmgr_crypto_algo *encap_algo, *decap_algo;
	struct nss_dtls_session_configure *scfg;
	nss_tx_status_t status;
	nss_crypto_status_t crypto_status;
	enum nss_dtlsmgr_status ret = NSS_DTLSMGR_FAIL;
	uint32_t features = 0;
	uint32_t mtu_adjust;

	if ((cfg->encap.ver != NSS_DTLSMGR_VERSION_1_0) && (cfg->encap.ver != NSS_DTLSMGR_VERSION_1_2)) {
		nss_dtlsmgr_warn("%px: Invalid DTLS version\n", &g_ctx);
		return NULL;
	}

	/*
	 * Allocate memory for new dtls session
	 */
	ds = kzalloc(sizeof(struct nss_dtlsmgr_session), GFP_KERNEL);
	if (!ds) {
		nss_dtlsmgr_info("%px: DTLS client allocation failed\n", &g_ctx);
		return NULL;
	}

	/*
	 * Create crypto session for encap
	 */
	ret = nss_dtlsmgr_alloc_crypto(&cfg->encap.crypto, &ds->crypto_idx_encap, true);
	if (ret != NSS_DTLSMGR_OK) {
		nss_dtlsmgr_info("failed to create encap session %d", ret);
		goto dtls_crypto_encap_alloc_fail;
	}

	/*
	 * Create crypto session for decap
	 */
	ret = nss_dtlsmgr_alloc_crypto(&cfg->decap.crypto, &ds->crypto_idx_decap, false);
	if (ret != NSS_DTLSMGR_OK) {
		nss_dtlsmgr_info("failed to create decap session %d", ret);
		goto dtls_crypto_decap_alloc_fail;
	}

	/*
	 * Allocate NSS dynamic interface
	 */
	ds->nss_dtls_if = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_DTLS);
	if (ds->nss_dtls_if == -1) {
		nss_dtlsmgr_info("%px: DTLS dynamic I/F alloc failed\n", &g_ctx);
		goto dtls_dynamic_if_alloc_fail;
	}

	/*
	 * Create netdevice
	 */
	if (nss_dtlsmgr_netdev_create(ds) != NSS_DTLSMGR_OK) {
		nss_dtlsmgr_info("%px: DTLS netdev creation failed\n", &g_ctx);
		goto dtls_netdev_create_fail;
	}

	/*
	 * Register NSS DTLS I/F
	 */
	ds->nss_ctx = nss_dtls_register_if(ds->nss_dtls_if,
					   nss_connmgr_dtls_data_receive,
					   nss_connmgr_dtls_event_receive,
					   ds->netdev, features,
					   (void *)ds);
	if (ds->nss_ctx == NULL) {
		nss_dtlsmgr_info("%px: DTLS dynamic I/F register failed\n", &g_ctx);
		goto dtls_dynamic_if_register_fail;
	}

	/*
	 * Initialize DTLS manager session
	 */
	ds->magic = NSS_DTLSMGR_SESSION_MAGIC;
	ds->flags = cfg->flags;
	ds->ver = cfg->encap.ver;
	ds->sport = cfg->encap.sport;
	ds->dport = cfg->encap.dport;
	ds->epoch = cfg->encap.epoch;
	ds->ip_ttl = cfg->encap.ip_ttl;
	ds->nss_app_if = cfg->decap.nexthop_ifnum;
	ds->window_size = cfg->decap.window_size;
	ds->stats_update_cb = NULL; /* TODO */
	ds->cidx_encap_pending = NSS_CRYPTO_MAX_IDXS;
	ds->cidx_decap_pending = NSS_CRYPTO_MAX_IDXS;
	atomic_set(&ds->ref, 1);

	if (ds->flags & NSS_DTLSMGR_HDR_IPV6) {
		for (i = 0; i < 4 ; i++) {
			ds->sip.ipv6[i] = cfg->encap.sip[i];
			ds->dip.ipv6[i] = cfg->encap.dip[i];
		}
	} else {
		ds->sip.ipv4 = cfg->encap.sip[0];
		ds->dip.ipv4 = cfg->encap.dip[0];
	}

	/*
	 * Insert session into DTLS manager list
	 */
	spin_lock_bh(&g_ctx.lock);
	if (!nss_dtlsmgr_session_insert(ds)) {
		spin_unlock_bh(&g_ctx.lock);
		goto dtls_session_insert_fail;
	}
	spin_unlock_bh(&g_ctx.lock);

	/*
	 * Send DTLS configure message to NSS
	 */
	memset(&dtlsmsg, 0, sizeof(struct nss_dtls_msg));
	nss_dtls_msg_init(&dtlsmsg, ds->nss_dtls_if,
			  NSS_DTLS_MSG_SESSION_CONFIGURE,
			  sizeof(struct nss_dtls_session_configure),
			  NULL, NULL);

	encap_algo = &dtlsmgr_algo[cfg->encap.crypto.algo];
	decap_algo = &dtlsmgr_algo[cfg->decap.crypto.algo];

	scfg = &dtlsmsg.msg.cfg;
	scfg->ver = ds->ver;
	scfg->flags = ds->flags;
	scfg->crypto_idx_encap = ds->crypto_idx_encap;
	scfg->crypto_idx_decap = ds->crypto_idx_decap;
	scfg->iv_len_encap = encap_algo->iv_len;
	scfg->iv_len_decap = decap_algo->iv_len;
	scfg->hash_len_encap = encap_algo->hash_len;
	scfg->hash_len_decap = decap_algo->hash_len;
	scfg->cipher_algo_encap = encap_algo->cipher_algo;
	scfg->cipher_algo_decap = decap_algo->cipher_algo;
	scfg->auth_algo_encap = encap_algo->auth_algo;
	scfg->auth_algo_decap = decap_algo->auth_algo;
	scfg->nss_app_if = ds->nss_app_if;
	scfg->sport = ds->sport;
	scfg->dport = ds->dport;
	scfg->epoch = ds->epoch;
	scfg->window_size = ds->window_size;
	scfg->oip_ttl = ds->ip_ttl;

	if (ds->flags & NSS_DTLSMGR_HDR_IPV6) {
		for (i = 0; i < 4; i++) {
			scfg->sip[i] = ds->sip.ipv6[i];
			scfg->dip[i] = ds->dip.ipv6[i];
		}
	} else {
		scfg->sip[0] = ds->sip.ipv4;
		scfg->dip[0] = ds->dip.ipv4;
	}

	status = nss_dtls_tx_msg_sync(ds->nss_ctx, &dtlsmsg);
	if (status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_info("%px: DTLS cfg msg tx failed\n", &g_ctx);
		goto dtls_msg_tx_fail;
	}

	/*
	 * Adjust MTU of netdev
	 */
	mtu_adjust = NSS_DTLSMGR_HDR_LEN;
	mtu_adjust += sizeof(struct udphdr);

	if (ds->flags & NSS_DTLSMGR_HDR_IPV6)
		mtu_adjust += sizeof(struct ipv6hdr);
	else
		mtu_adjust += sizeof(struct iphdr);

	if (ds->flags & NSS_DTLSMGR_HDR_CAPWAP)
		mtu_adjust += NSS_DTLSMGR_CAPWAPHDR_LEN;

	mtu_adjust += ((scfg->iv_len_encap * 2) + scfg->hash_len_encap);

	ds->netdev->mtu -= mtu_adjust;

	nss_dtlsmgr_info("%px: NSS DTLS session I/F:%d(%s) created\n",
			 &g_ctx, ds->nss_dtls_if, ds->netdev->name);

	return ds->netdev;

dtls_msg_tx_fail:
	spin_lock_bh(&g_ctx.lock);
	nss_dtlsmgr_session_remove(ds->nss_dtls_if);
	spin_unlock_bh(&g_ctx.lock);

dtls_session_insert_fail:
	nss_dtls_unregister_if(ds->nss_dtls_if);

dtls_dynamic_if_register_fail:
	nss_dtlsmgr_netdev_destroy(ds);

dtls_netdev_create_fail:
	nss_dynamic_interface_dealloc_node(ds->nss_dtls_if,
					   NSS_DYNAMIC_INTERFACE_TYPE_DTLS);

dtls_dynamic_if_alloc_fail:
	crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl,
						ds->crypto_idx_decap);
	if (crypto_status != NSS_CRYPTO_STATUS_OK) {
		nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n", &g_ctx, ds->nss_dtls_if, ds->crypto_idx_decap);
	}

dtls_crypto_decap_alloc_fail:
	crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl,
						ds->crypto_idx_encap);
	if (crypto_status != NSS_CRYPTO_STATUS_OK) {
		nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n", &g_ctx, ds->nss_dtls_if, ds->crypto_idx_encap);
	}

dtls_crypto_encap_alloc_fail:
	ds->magic = 0;
	kfree(ds);
	return NULL;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_create);

/*
 * nss_dtlsmgr_session_destroy()
 *	Destroy DTLS session
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_destroy(struct net_device *dev)
{
	struct nss_dtlsmgr_netdev_priv *priv;
	struct nss_dtlsmgr_session *ds;
	nss_tx_status_t nss_status;
	struct nss_dtls_msg dtlsmsg;

	priv = netdev_priv(dev);
	ds = priv->s;

	if (!ds) {
		return NSS_DTLSMGR_FAIL;
	}

	/*
	 * Send DTLS session destroy command to FW
	 */
	memset(&dtlsmsg, 0, sizeof(struct nss_dtls_msg));
	nss_dtls_msg_init(&dtlsmsg, (uint16_t)ds->nss_dtls_if,
			  NSS_DTLS_MSG_SESSION_DESTROY, 0, NULL, NULL);

	nss_status = nss_dtls_tx_msg_sync(ds->nss_ctx, &dtlsmsg);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_warn("%px: Failed to send DTLS session destroy for I/F %u", &g_ctx, ds->nss_dtls_if);

		return NSS_DTLSMGR_FAIL;
	}

	spin_lock_bh(&g_ctx.lock);
	ds = nss_dtlsmgr_session_remove(ds->nss_dtls_if);
	spin_unlock_bh(&g_ctx.lock);

	if (!ds) {
		return NSS_DTLSMGR_FAIL;
	}

	/*
	 * Decrement reference count so as to drop it to zero
	 */
	nss_dtlsmgr_session_ref_dec(ds);

	nss_dtlsmgr_info("%px: DTLS session I/F %u disabled\n", &g_ctx, ds->nss_dtls_if);

	return NSS_DTLSMGR_OK;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_destroy);

/*
 * nss_dtlsmgr_session_update_decap()
 *	Update pending decap cipher of a DTLS session.
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_update_decap(struct net_device *dev, struct nss_dtlsmgr_config_update *udata)
{
	struct nss_dtlsmgr_netdev_priv *priv;
	struct nss_dtlsmgr_session *ds;
	nss_tx_status_t nss_status;
	nss_crypto_status_t crypto_status;
	enum nss_dtlsmgr_status ret;
	struct nss_dtls_msg dtlsmsg;
	struct nss_dtls_session_cipher_update *update;
	struct nss_dtlsmgr_crypto_algo *decap_algo;

	priv = netdev_priv(dev);
	ds = priv->s;
	if (!ds) {
		return NSS_DTLSMGR_FAIL;
	}

	/*
	 * Free any crypto session, for the decap pending cipher
	 * state, allocated by a previous call to this API but
	 * were subsequently not used for packet processing.
	 */
	if (ds->cidx_decap_pending != NSS_CRYPTO_MAX_IDXS) {
		crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl, ds->cidx_decap_pending);
		if (crypto_status != NSS_CRYPTO_STATUS_OK) {
			nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n",
					 &g_ctx, ds->nss_dtls_if, ds->cidx_decap_pending);
		}

		ds->cidx_decap_pending = NSS_CRYPTO_MAX_IDXS;
	}

	/*
	 * Alloc crypto session for decap
	 */
	ret = nss_dtlsmgr_alloc_crypto(&udata->crypto, &ds->cidx_decap_pending, false);
	if (ret != NSS_DTLSMGR_OK) {
		nss_dtlsmgr_info("failed to rekey decap session %d", ret);
		ds->cidx_decap_pending = NSS_CRYPTO_MAX_IDXS;
		nss_dtlsmgr_session_ref_dec(ds);
		return NSS_DTLSMGR_FAIL_NOCRYPTO;
	}

	decap_algo = &dtlsmgr_algo[udata->crypto.algo];

	/*
	 * Initialize DTLS session Rx cipher update message
	 */
	memset(&dtlsmsg, 0, sizeof(struct nss_dtls_msg));
	update = &dtlsmsg.msg.cipher_update;
	update->crypto_idx = ds->cidx_decap_pending;
	update->epoch = udata->epoch;
	update->iv_len = decap_algo->iv_len;
	update->hash_len = decap_algo->hash_len;
	update->auth_algo = decap_algo->auth_algo;
	update->cipher_algo = decap_algo->cipher_algo;

	nss_dtls_msg_init(&dtlsmsg, (uint16_t)ds->nss_dtls_if,
			  NSS_DTLS_MSG_REKEY_DECAP_CIPHER_UPDATE,
			  sizeof(struct nss_dtls_session_cipher_update), NULL, NULL);

	/*
	 * Send DTLS session Rx cipher update command to FW
	 */
	nss_status = nss_dtls_tx_msg_sync(ds->nss_ctx, &dtlsmsg);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_crypto_session_free(g_ctx.crypto_hdl,
					ds->cidx_decap_pending);
		ds->cidx_decap_pending = NSS_CRYPTO_MAX_IDXS;
		nss_dtlsmgr_session_ref_dec(ds);
		return NSS_DTLSMGR_FAIL;
	}

	nss_dtlsmgr_session_ref_dec(ds);
	return NSS_DTLSMGR_OK;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_update_decap);

/*
 * nss_dtlsmgr_session_update_encap()
 *	Update pending encap cipher of a DTLS session.
 */
nss_dtlsmgr_status_t nss_dtlsmgr_session_update_encap(struct net_device *dev, struct nss_dtlsmgr_config_update *udata)
{
	struct nss_dtlsmgr_netdev_priv *priv;
	struct nss_dtlsmgr_session *ds;
	nss_tx_status_t nss_status;
	nss_crypto_status_t crypto_status;
	enum nss_dtlsmgr_status ret;
	struct nss_dtls_msg dtlsmsg;
	struct nss_dtls_session_cipher_update *update;
	struct nss_dtlsmgr_crypto_algo *encap_algo;

	priv = netdev_priv(dev);
	ds = priv->s;
	if (!ds) {
		return NSS_DTLSMGR_FAIL;
	}

	/*
	 * Free any crypto session, for the encap pending cipher
	 * state, allocated by a previous call to this API but
	 * were subsequently not used for packet processing.
	 */
	if (ds->cidx_encap_pending != NSS_CRYPTO_MAX_IDXS) {
		crypto_status = nss_crypto_session_free(g_ctx.crypto_hdl,
							ds->cidx_encap_pending);
		if (crypto_status != NSS_CRYPTO_STATUS_OK) {
			nss_dtlsmgr_info("%px: dtls I/F:%u, unable to free crypto session id:%d\n", &g_ctx, ds->nss_dtls_if, ds->cidx_encap_pending);
		}

		ds->cidx_encap_pending = NSS_CRYPTO_MAX_IDXS;
	}

	/*
	 * Alloc crypto session for decap
	 */
	ret = nss_dtlsmgr_alloc_crypto(&udata->crypto, &ds->cidx_encap_pending, true);
	if (ret != NSS_DTLSMGR_OK) {
		nss_dtlsmgr_info("failed to rekey encap session %d", ret);
		ds->cidx_encap_pending = NSS_CRYPTO_MAX_IDXS;
		nss_dtlsmgr_session_ref_dec(ds);
		return NSS_DTLSMGR_FAIL_NOCRYPTO;
	}

	encap_algo = &dtlsmgr_algo[udata->crypto.algo];

	/*
	 * Initialize DTLS session Tx cipher update message
	 */
	memset(&dtlsmsg, 0, sizeof(struct nss_dtls_msg));
	update = &dtlsmsg.msg.cipher_update;
	update->crypto_idx = ds->cidx_encap_pending;
	update->epoch = udata->epoch;
	update->iv_len = encap_algo->iv_len;
	update->hash_len = encap_algo->hash_len;
	update->auth_algo = encap_algo->auth_algo;
	update->cipher_algo = encap_algo->cipher_algo;

	nss_dtls_msg_init(&dtlsmsg, (uint16_t)ds->nss_dtls_if,
			  NSS_DTLS_MSG_REKEY_ENCAP_CIPHER_UPDATE,
			  sizeof(struct nss_dtls_session_cipher_update), NULL, NULL);

	/*
	 * Send DTLS session Rx cipher update command to FW
	 */
	nss_status = nss_dtls_tx_msg_sync(ds->nss_ctx, &dtlsmsg);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_crypto_session_free(g_ctx.crypto_hdl,
					ds->cidx_encap_pending);
		ds->cidx_encap_pending = NSS_CRYPTO_MAX_IDXS;
		nss_dtlsmgr_session_ref_dec(ds);
		return NSS_DTLSMGR_FAIL;
	}

	nss_dtlsmgr_session_ref_dec(ds);
	return NSS_DTLSMGR_OK;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_update_encap);

/*
 * nss_dtlsmgr_session_switch_decap()
 *	Set pending decap cipher state of a DTLS session to current.
 */
bool nss_dtlsmgr_session_switch_decap(struct net_device *dev)
{
	struct nss_dtlsmgr_netdev_priv *priv;
	struct nss_dtlsmgr_session *ds;
	nss_tx_status_t nss_status;
	struct nss_dtls_msg dtlsmsg;

	priv = netdev_priv(dev);
	ds = priv->s;
	if (!ds) {
		return false;
	}

	memset(&dtlsmsg, 0, sizeof(struct nss_dtls_msg));
	nss_dtls_msg_init(&dtlsmsg, (uint16_t)ds->nss_dtls_if,
			  NSS_DTLS_MSG_REKEY_DECAP_CIPHER_SWITCH,
			  0, NULL, NULL);

	/*
	 * Send DTLS session Rx cipher switch command to FW
	 */
	nss_status = nss_dtls_tx_msg_sync(ds->nss_ctx, &dtlsmsg);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_session_ref_dec(ds);
		return false;
	}

	ds->crypto_idx_decap = ds->cidx_decap_pending;
	ds->cidx_decap_pending = NSS_CRYPTO_MAX_IDXS;

	nss_dtlsmgr_session_ref_dec(ds);
	return true;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_switch_decap);

/*
 * nss_dtlsmgr_session_switch_encap()
 *	Set pending encap cipher state of a DTLS session to current.
 */
bool nss_dtlsmgr_session_switch_encap(struct net_device *dev)
{
	struct nss_dtlsmgr_netdev_priv *priv;
	struct nss_dtlsmgr_session *ds;
	nss_tx_status_t nss_status;
	struct nss_dtls_msg dtlsmsg;

	priv = netdev_priv(dev);
	ds = priv->s;
	if (!ds) {
		return false;
	}

	memset(&dtlsmsg, 0, sizeof(struct nss_dtls_msg));
	nss_dtls_msg_init(&dtlsmsg, (uint16_t)ds->nss_dtls_if,
			  NSS_DTLS_MSG_REKEY_ENCAP_CIPHER_SWITCH,
			  0, NULL, NULL);

	/*
	 * Send DTLS session Tx cipher switch command to FW
	 */
	nss_status = nss_dtls_tx_msg_sync(ds->nss_ctx, &dtlsmsg);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_session_ref_dec(ds);
		return false;
	}

	ds->crypto_idx_encap = ds->cidx_encap_pending;
	ds->cidx_encap_pending = NSS_CRYPTO_MAX_IDXS;

	nss_dtlsmgr_session_ref_dec(ds);
	return true;
}
EXPORT_SYMBOL(nss_dtlsmgr_session_switch_encap);

/*
 * nss_dtlsmgr_get_interface()
 *	Returns NSS DTLS interface number for encap/decap on success.
 */
int32_t nss_dtlsmgr_get_interface(struct net_device *dev, enum nss_dtlsmgr_interface_type type)
{
	int32_t ifnum;

	if (type > NSS_DTLSMGR_INTERFACE_TYPE_MAX) {
		nss_dtlsmgr_warn("%px: invalid interface type %d", dev, type);
		return -EINVAL;
	}

	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_DTLS);
	if (ifnum < 0) {
		nss_dtlsmgr_warn("%px: couldn't find DTLS interface number (%d)", dev, ifnum);
		return ifnum;
	}

	ifnum = nss_dtls_get_ifnum_with_coreid(ifnum);

	return ifnum;
}
EXPORT_SYMBOL(nss_dtlsmgr_get_interface);

/*
 * nss_dtls_crypto_attach()
 */
static nss_crypto_user_ctx_t nss_dtls_crypto_attach(nss_crypto_handle_t crypto)
{
	struct nss_dtlsmgr_ctx *sc = &g_ctx;

	sc->crypto_hdl = crypto;
	nss_dtlsmgr_info("%px: DTLS client crypto attach\n", &g_ctx);
	return (nss_crypto_user_ctx_t)sc;
}

/*
 * nss_dtls_crypto_detach()
 */
static void nss_dtls_crypto_detach(nss_crypto_user_ctx_t uctx)
{
	struct nss_dtlsmgr_ctx *sc = NULL;

	sc = (struct nss_dtlsmgr_ctx *)uctx;
	nss_dtlsmgr_assert(sc == &g_ctx);

	sc->crypto_hdl = NULL;
	nss_dtlsmgr_info("%px: DTLS client crypto detach\n", &g_ctx);
}

/*
 * nss_dtls_init_module()
 */
int __init nss_dtls_init_module(void)
{
	int32_t i;

	nss_dtlsmgr_info("%px: NSS DTLS Manager\n", &g_ctx);

	for (i = 0; i < NSS_MAX_DTLS_SESSIONS; i++) {
		g_ctx.session[i] = NULL;
	}

	spin_lock_init(&(g_ctx.lock));
	nss_crypto_register_user(nss_dtls_crypto_attach,
				 nss_dtls_crypto_detach,
				 "nss-dtls");
	return 0;
}

/*
 * nss_dtls_destroy_all_sessions()
 */
static void nss_dtls_destroy_all_sessions(void)
{
	nss_tx_status_t nss_status;
	struct nss_dtls_msg dtlsmsg;
	struct nss_dtlsmgr_session *ds;
	int32_t i;

	for (i = 0; i < NSS_MAX_DTLS_SESSIONS; i++) {
		spin_lock_bh(&g_ctx.lock);
		if (g_ctx.session[i] == NULL) {
			spin_unlock_bh(&g_ctx.lock);
			continue;
		}

		/*
		 * Remove session from list of sessions
		 */
		ds = g_ctx.session[i];
		g_ctx.session[i] = NULL;
		spin_unlock_bh(&g_ctx.lock);

		nss_dtlsmgr_assert(ds->magic == NSS_DTLSMGR_SESSION_MAGIC);

		/*
		 * Send DTLS session destroy command to FW
		 */
		memset(&dtlsmsg, 0, sizeof(struct nss_dtls_msg));
		nss_dtls_msg_init(&dtlsmsg, (uint16_t)ds->nss_dtls_if,
				  NSS_DTLS_MSG_SESSION_DESTROY, 0,
				  NULL, NULL);

		nss_status = nss_dtls_tx_msg_sync(ds->nss_ctx, &dtlsmsg);
		if (nss_status != NSS_TX_SUCCESS)
			nss_dtlsmgr_warn("%px: Failed to send DTLS session destroy for I/F %u\n", &g_ctx, ds->nss_dtls_if);

		nss_dtlsmgr_session_ref_dec(ds);
	}
}

/*
 * nss_dtls_exit_module()
 */
void __exit nss_dtls_exit_module(void)
{
	nss_dtls_destroy_all_sessions();
	nss_crypto_unregister_user(g_ctx.crypto_hdl);
}

module_init(nss_dtls_init_module);
module_exit(nss_dtls_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS DTLS manager");
