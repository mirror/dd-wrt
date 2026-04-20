/* Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/if.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/xfrm.h>
#include <net/esp.h>
#include <net/protocol.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include <linux/netfilter.h>
#include <crypto/authenc.h>
#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>
#include <ecm_interface_ipsec.h>
#include <ecm_notifier.h>

#include "nss_ipsec_xfrm_tunnel.h"
#include "nss_ipsec_xfrm_sa.h"
#include "nss_ipsec_xfrm_flow.h"
#include "nss_ipsec_xfrm.h"

/*
 * XFRM algorithm to IPsec manager algorithm map
 */
struct nss_ipsec_xfrm_algo {
	const char *cipher_name;		/* XFRM Cipher algorithm name */
	const char *auth_name;		/* XFRM Authentication algorithm name */
	enum nss_ipsecmgr_algo algo;	/* IPsec manager algorithm number */
};

/*
 * Linux crypto algorithm names.
 */
static struct nss_ipsec_xfrm_algo xfrm_algo[] = {
	{.cipher_name = "cbc(aes)", .auth_name = "hmac(sha1)", .algo = NSS_IPSECMGR_ALGO_AES_CBC_SHA1_HMAC},
	{.cipher_name = "cbc(des3_ede)", .auth_name = "hmac(sha1)", .algo = NSS_IPSECMGR_ALGO_3DES_CBC_SHA1_HMAC},
#ifndef NSS_IPSEC_XFRM_IPQ50XX
	{.cipher_name  = "cbc(aes)", .auth_name = "hmac(md5)", .algo = NSS_IPSECMGR_ALGO_AES_CBC_MD5_HMAC},
	{.cipher_name = "cbc(des3_ede)", .auth_name = "hmac(md5)", .algo = NSS_IPSECMGR_ALGO_3DES_CBC_MD5_HMAC},
	{.cipher_name = "rfc4106(gcm(aes))", .auth_name = "rfc4106(gcm(aes))", .algo = NSS_IPSECMGR_ALGO_AES_GCM_GMAC_RFC4106},
	{.cipher_name = "ecb(cipher_null)", .auth_name = "hmac(sha1)", .algo = NSS_IPSECMGR_ALGO_NULL_CIPHER_SHA1_HMAC},
	{.cipher_name = "ecb(cipher_null)", .auth_name = "hmac(sha256)", .algo = NSS_IPSECMGR_ALGO_NULL_CIPHER_SHA256_HMAC},
#endif
	{.cipher_name = "cbc(aes)", .auth_name = "hmac(sha256)", .algo = NSS_IPSECMGR_ALGO_AES_CBC_SHA256_HMAC},
	{.cipher_name = "cbc(des3_ede)", .auth_name = "hmac(sha256)", .algo = NSS_IPSECMGR_ALGO_3DES_CBC_SHA256_HMAC},
};

/*
 * nss_ipsec_xfrm_sa_get_algo()
 *	Get the IPsec manager algorithm from XFRM cipher & authentication names
 */
static enum nss_ipsecmgr_algo nss_ipsec_xfrm_sa_get_algo(struct nss_ipsec_xfrm_sa *sa, char *cipher_name, char *auth_name)
{
	struct nss_ipsec_xfrm_algo *xalg = xfrm_algo;
	uint32_t i;

	nss_ipsec_xfrm_trace("%p: get XFRM algorithm name(%s,%s) to IPsec manager algo", sa,
			cipher_name ? cipher_name : "", auth_name ? auth_name : "");

	for (i = 0; i < ARRAY_SIZE(xfrm_algo); i++, xalg++) {
		if (cipher_name && strncmp(xalg->cipher_name, cipher_name, strlen(xalg->cipher_name))) {
			continue;
		}

		if (auth_name && strncmp(xalg->auth_name, auth_name, strlen(xalg->auth_name))) {
			continue;
		}

		return xalg->algo;
	}

	return NSS_IPSECMGR_ALGO_MAX;
}

/*
 * nss_ipsec_xfrm_sa_final()
 *	Frees the SA object
 */
static void nss_ipsec_xfrm_sa_final(struct kref *kref)
{
	struct nss_ipsec_xfrm_sa *sa = container_of(kref, struct nss_ipsec_xfrm_sa, ref);
	struct nss_ipsec_xfrm_tunnel *tun = sa->tun;
	struct nss_ipsec_xfrm_drv *drv = sa->drv;

	nss_ipsec_xfrm_info("%p: SA freed\n", sa);

	if (tun) {
		nss_ipsec_xfrm_tunnel_deref(sa->tun);
		sa->tun = NULL;
	}

	atomic64_inc(&drv->stats.sa_freed);
	kfree(sa);
}

static bool nss_ipsec_xfrm_sa_init_crypto(struct nss_ipsec_xfrm_sa *sa, struct xfrm_state *x)
{
	struct nss_ipsecmgr_crypto_keys *nick = &sa->data.cmn.keys;
	struct nss_ipsecmgr_sa_cmn *nisc = &sa->data.cmn;
	char *cipher_name, *auth_name;

	if (x->aead) { /* Combined mode cipher/authentication */
		unsigned int key_len = 0;

		key_len = ALIGN(x->aead->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;

		/* Cipher */
		cipher_name = x->aead->alg_name;
		nick->cipher_keylen = key_len - 4; /* Subtract nonce */
		nick->cipher_key = x->aead->alg_key;

		/* Authentication; No separate auth keys */
		auth_name = x->aead->alg_name;
		nisc->icv_len = x->aead->alg_icv_len / BITS_PER_BYTE;

		/* Nonce */
		nick->nonce_size = 4;
		nick->nonce = x->aead->alg_key + nick->cipher_keylen;
	} else if (x->ealg && x->aalg) { /* Authenticated encryption */
		/* Cipher */
		cipher_name = x->ealg->alg_name;
		nick->cipher_keylen = ALIGN(x->ealg->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;
		nick->cipher_key = x->ealg->alg_key;

		/* Authentication */
		auth_name = x->aalg->alg_name;
		nick->auth_keylen = ALIGN(x->aalg->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;
		nick->auth_key = x->aalg->alg_key;
		nisc->icv_len =  x->aalg->alg_trunc_len / BITS_PER_BYTE;
	} else if (x->ealg) { /* Pure encryption */
		/* Cipher */
		cipher_name = x->ealg->alg_name;
		nick->cipher_keylen = ALIGN(x->ealg->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;
		nick->cipher_key = x->ealg->alg_key;

		/* Authentication */
		auth_name = NULL;
		nick->auth_keylen = 0;
		nick->auth_key = NULL;
		nisc->icv_len =  0;
	} else { /* Bypass mode */
		cipher_name = "NULL";
		auth_name = "NULL";
		memset(nick, 0, sizeof(struct nss_ipsecmgr_crypto_keys));
		nisc->icv_len = 0;
	}

	nisc->algo = nss_ipsec_xfrm_sa_get_algo(sa, cipher_name, auth_name);
	if (nisc->algo == NSS_IPSECMGR_ALGO_MAX) {
		nss_ipsec_xfrm_err("%p: Failed to find algorithm (%s,%s)\n", sa, cipher_name ? cipher_name : "",
				auth_name ? auth_name : "");
		memset(nick, 0, sizeof(struct nss_ipsecmgr_crypto_keys));
		return false;
	}

	return true;
}

/*
 * nss_ipsec_xfrm_sa_init_tuple
 *	Initialize the SA tuple from XFRM state
 */
static void nss_ipsec_xfrm_sa_init_tuple(struct nss_ipsec_xfrm_sa *sa, struct xfrm_state *x)
{
	struct net_device *local_dev;

	sa->type = NSS_IPSECMGR_SA_TYPE_ENCAP;
	sa->tuple.spi_index = ntohl(x->id.spi);
	sa->tuple.proto_next_hdr = IPPROTO_ESP;

	/*
	* Check if this is ESP-over-UDP SA
	*/
	if (x->encap) {
		sa->tuple.sport = ntohs(x->encap->encap_sport);
		sa->tuple.dport = ntohs(x->encap->encap_dport);
		sa->tuple.proto_next_hdr = IPPROTO_UDP;
	}

	if (x->props.family == AF_INET) {
		sa->tuple.ip_version = IPVERSION;
		sa->tuple.src_ip[0] = ntohl(x->props.saddr.a4);
		sa->tuple.dest_ip[0] = ntohl(x->id.daddr.a4);

		local_dev = ip_dev_find(&init_net, x->id.daddr.a4);
	} else {
		sa->tuple.ip_version = 6;

		sa->tuple.src_ip[0] = ntohl(x->props.saddr.a6[0]);
		sa->tuple.src_ip[1] = ntohl(x->props.saddr.a6[1]);
		sa->tuple.src_ip[2] = ntohl(x->props.saddr.a6[2]);
		sa->tuple.src_ip[3] = ntohl(x->props.saddr.a6[3]);

		sa->tuple.dest_ip[0] = ntohl(x->id.daddr.a6[0]);
		sa->tuple.dest_ip[1] = ntohl(x->id.daddr.a6[1]);
		sa->tuple.dest_ip[2] = ntohl(x->id.daddr.a6[2]);
		sa->tuple.dest_ip[3] = ntohl(x->id.daddr.a6[3]);

		local_dev = ipv6_dev_find(&init_net, (struct in6_addr *)x->id.daddr.a6, 1);
	}

	/*
	 * Check if the SA is outer or inner and override the type
	 */
	if (local_dev) {
		dev_put(local_dev);
		sa->type = NSS_IPSECMGR_SA_TYPE_DECAP;
	}
}

/*
 * nss_ipsec_xfrm_sa_deinit()
 *	Deinitialize the SA.
 */
void nss_ipsec_xfrm_sa_deinit(struct xfrm_state *x)
{
	nss_ipsec_xfrm_info("%p: flags 0x%x\n", x, x->xflags);

	/*
	 * If XFRM state was programmed, then deinit
	 */
	if (x->xflags & XFRM_STATE_OFFLOAD_NSS) {
		x->xflags &= ~XFRM_STATE_OFFLOAD_NSS;
		nss_ipsec_xfrm_sa_deref(x->data);
		x->data = NULL;
	}
}

/*
 * nss_ipsec_xfrm_sa_init()
 * 	Initialize common SA parameters befor programming the IPsec manager
 */
static void nss_ipsec_xfrm_sa_init(struct nss_ipsec_xfrm_sa *sa, struct xfrm_state *x)
{
	struct nss_ipsecmgr_sa_data *sa_data = &sa->data;
	struct nss_ipsec_xfrm_tunnel *tun = sa->tun;

	nss_ipsec_xfrm_sa_init_tuple(sa, x);

	sa_data->type = sa->type;

	if (sa_data->type == NSS_IPSECMGR_SA_TYPE_ENCAP) {
		sa_data->encap.ttl_hop_limit = tun->ttl;
		sa_data->encap.copy_dscp = !(x->props.extra_flags & XFRM_SA_XFLAG_DONT_ENCAP_DSCP);
		sa_data->encap.copy_df = !(x->props.flags & XFRM_STATE_NOPMTUDISC);

		/*
		 * TODO: Handle ECN
		 */
		nss_ipsec_xfrm_trace("%p: Encap SA xfrm(%p), ttl(%d), dscp(%d), df(%d)\n", sa, x,
				sa_data->encap.ttl_hop_limit, sa_data->encap.copy_dscp, sa_data->encap.copy_df);
	} else {
		uint32_t replay_win = x->props.replay_window / BITS_PER_BYTE;

		sa_data->decap.replay_win = replay_win & (NSS_IPSEC_XFRM_SA_MAX_REPLAY_WIN_SZ - 1);
		nss_ipsec_xfrm_trace("%p: Decap SA xfrm(%p), replay_win(%u) \n", sa, x, sa_data->decap.replay_win);
	}

	sa_data->cmn.transport_mode = (x->props.mode == XFRM_MODE_TRANSPORT);
	sa_data->cmn.enable_esn = !!(x->props.flags & XFRM_STATE_ESN);
	sa_data->cmn.enable_natt = !!x->encap;
	sa_data->cmn.skip_trailer = false;
	sa_data->cmn.crypto_has_keys = true;

	/*
	 * Bind XFRM state to SA
	 */
	x->data = nss_ipsec_xfrm_sa_ref(sa);
	x->xflags |= XFRM_STATE_OFFLOAD_NSS;
}

/*
 * nss_ipsec_xfrm_sa_sp_set()
 * 	Load skb with sp.
 */
bool nss_ipsec_xfrm_sa_sp_set(struct nss_ipsec_xfrm_sa *sa, struct sk_buff *skb)
{
	struct sec_path *sp;
	struct xfrm_state *x;

	/*
	 * Lookup the XFRM state using SPI index; note that this will hold the state reference
	 * which needs to be released after the SP usage is over
	 */
	x = xfrm_state_lookup_byspi(&init_net, htonl(sa->tuple.spi_index), sa->tun->family);
	if (!x) {
		nss_ipsec_xfrm_err("%p: Failed to set SP; no XFRM state found, spi(%u)\n", sa, sa->tuple.spi_index);
		return false;
	}

	/*
	 * Allocate a new SP for the incoming SKB
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0))
	skb->sp = sp = secpath_dup(NULL);
#else
	sp = secpath_set(skb);
#endif
	if(!sp) {
		xfrm_state_put(x);
		return false;
	}

	sp->xvec[sp->len++] = x;
	return true;
}

/*
 * nss_ipsec_xfrm_sa_deref()
 *	Put SA ref.
 */
void  nss_ipsec_xfrm_sa_deref(struct nss_ipsec_xfrm_sa *sa)
{
	kref_put(&sa->ref, nss_ipsec_xfrm_sa_final);
}

/*
 * nss_ipsec_xfrm_sa_ref()
 *	Hold SA ref.
 */
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_ref(struct nss_ipsec_xfrm_sa *sa)
{
	kref_get(&sa->ref);
	return sa;
}

/*
 * nss_ipsec_xfrm_sa_ref_by_spi()
 * 	Get NSS xfrm plugin SA object from spi_index and family
 */
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_ref_by_spi(uint32_t spi_index, uint32_t family)
{
	struct nss_ipsec_xfrm_sa * sa;
	struct xfrm_state *x;

	/*
	 * Lookup XFRM state by SPI index
	 */
	x = xfrm_state_lookup_byspi(&init_net, htonl(spi_index), family);
	if (!x) {
		return NULL;
	}

	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	xfrm_state_put(x);

	return sa;
}

/*
 * nss_ipsec_xfrm_sa_ref_by_state()
 *	 Get NSS xfrm plugin SA object from xfrm state.
 */
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_ref_by_state(struct xfrm_state *x)
{
	if (x->xflags & XFRM_STATE_OFFLOAD_NSS) {
		return nss_ipsec_xfrm_sa_ref(x->data);
	}

	return NULL;
}

/*
 * nss_ipsec_xfrm_sa_dealloc()
 *	Delete an SA object. Expected to be called when the associated xfrm_state is getting deleted.
 */
void nss_ipsec_xfrm_sa_dealloc(struct nss_ipsec_xfrm_sa *sa, struct xfrm_state *x)
{
	struct nss_ipsec_xfrm_drv *drv = sa->drv;

	nss_ipsec_xfrm_info("%p: xfrm_state %p", sa, x);

	atomic64_inc(&drv->stats.sa_dealloced);

	/*
	 * SA should always have an assoicated tunnel.
	 */
	BUG_ON(!sa->tun);

	/*
	 * Delete from ipsecmgr.
	 */
	nss_ipsecmgr_sa_del(sa->tun->dev, &sa->tuple);

	/*
	 * Only switch the value if it same for the current SA
	 */
	atomic_cmpxchg(&sa->tun->default_spi, sa->tuple.spi_index, 0);

	/*
	 * Refcnt was set to 1 when the sa object was created.
	 * We are releasing the same here.
	 */
	nss_ipsec_xfrm_sa_deref(sa);
}

/*
 * nss_ipsec_xfrm_sa_alloc()
 *	Create a new SA object.
 */
struct nss_ipsec_xfrm_sa *nss_ipsec_xfrm_sa_alloc(struct nss_ipsec_xfrm_tunnel *tun, struct xfrm_state *x)
{
	struct nss_ipsec_xfrm_drv *drv = tun->drv;
	struct nss_ipsec_xfrm_sa *sa;
	nss_ipsecmgr_status_t status;
	uint32_t if_num = 0;

	sa = kzalloc(sizeof(struct nss_ipsec_xfrm_sa), GFP_KERNEL);
	if (!sa) {
		nss_ipsec_xfrm_err("%p: Failed to create v4 SA; Out of Memory\n", x);
		return NULL;
	}

	atomic64_inc(&drv->stats.sa_alloced);

	sa->drv = drv;
	sa->tun = nss_ipsec_xfrm_tunnel_ref(tun);

	kref_init(&sa->ref);
	atomic_set(&sa->ecm_accel_outer, 0);

	/*
	 * Initialize the SA with common parameters
	 */
	nss_ipsec_xfrm_sa_init(sa, x);

	/*
	 * Derive crypto parameters
	 */
	if (!nss_ipsec_xfrm_sa_init_crypto(sa, x)) {
		nss_ipsec_xfrm_err("%p: Failed to alloc SA; Invalid crypto params\n", sa);
		goto error;
	}

	/*
	 * Add it to the ipsecmgr
	 */
	status = nss_ipsecmgr_sa_add_sync(tun->dev, &sa->tuple, &sa->data, &if_num);
	if ((status != NSS_IPSECMGR_OK) && (status != NSS_IPSECMGR_DUPLICATE_SA)) {
		nss_ipsec_xfrm_err("%p: Failed to alloc SA; IPsec manager error(%d)\n", sa, status);
		goto error;
	}

	/*
	 * Override the default SPI index if the outer SA was sucessfully programmed
	 * TODO: This only works if there is only one active outer SA per tunnel. Fix
	 * the outer exception to store the SA that causes the exception.
	 */
	if (sa->type == NSS_IPSECMGR_SA_TYPE_DECAP) {
		atomic_xchg(&tun->default_spi, sa->tuple.spi_index);
	}

	/*
	 * Fetch SA related info from ipsecmgr; this should always succeed as we have
	 * added a new SA
	 */
	status = nss_ipsecmgr_sa_get_info(tun->dev, &sa->tuple, &sa->sa_info);
	BUG_ON(!status);
	x->props.header_len = sa->sa_info.hdr_len;
	x->props.trailer_len = sa->sa_info.trailer_len;

	/*
	 * Reset all key related references
	 */
	memset(&sa->data.cmn.keys, 0, sizeof(struct nss_ipsecmgr_crypto_keys));

	nss_ipsec_xfrm_info("New SA created; src %pI4h dst %pI4h sport %d dport %d proto %d mode %d spi %x type %d\n",
			sa->tuple.src_ip, sa->tuple.dest_ip, sa->tuple.sport, sa->tuple.dport, sa->tuple.proto_next_hdr,
			x->props.mode, sa->tuple.spi_index, sa->type);

	return sa;
error:
	nss_ipsec_xfrm_sa_deinit(x);
	nss_ipsec_xfrm_sa_deref(sa);
	return NULL;
}
