/*
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/completion.h>
#include <net/xfrm.h>
#include <net/protocol.h>
#include <net/ip6_route.h>
#include <linux/inetdevice.h>
#include <linux/icmp.h>
#include <net/addrconf.h>
#include <linux/netfilter.h>
#include <ecm_interface_ipsec.h>

#include "eip_ipsec_xfrm.h"
#include "eip_ipsec_priv.h"

/*
 * Structure to map crypto algo-name to driver-name
 */
struct eip_ipsec_xfrm_algo {
	const char *algo_name;	/*Crypto algorithm name */
	char *driver_name;	/* driver name */
};

static struct eip_ipsec_xfrm_algo xfrm_algo[] = {
	{.algo_name = "echainiv(authenc(hmac(sha1),cbc(aes)))", .driver_name = "eip-aes-cbc-sha1-hmac"},
	{.algo_name = "echainiv(authenc(hmac(sha1),cbc(des3_ede)))", .driver_name = "eip-3des-cbc-sha1-hmac"},
	{.algo_name = "echainiv(authenc(hmac(md5),cbc(aes)))", .driver_name = "eip-aes-cbc-md5-hmac"},
	{.algo_name = "echainiv(authenc(hmac(md5),cbc(des3_ede)))", .driver_name = "eip-3des-cbc-md5-hmac"},
	{.algo_name = "echainiv(authenc(hmac(sha1),ecb(cipher_null)))", .driver_name = "eip-cipher_null-ecb-sha1-hmac"},
	{.algo_name = "echainiv(authenc(hmac(sha256),cbc(aes)))", .driver_name = "eip-aes-cbc-sha256-hmac"},
	{.algo_name = "echainiv(authenc(hmac(sha256),cbc(des3_ede)))", .driver_name = "eip-3des-cbc-sha256-hmac"},
	{.algo_name = "seqiv(rfc4106(gcm(aes)))", .driver_name = "eip-aes-gcm-rfc4106"},
	{.algo_name = "echainiv(authenc(hmac(sha256),ecb(cipher_null)))", .driver_name = "eip-cipher_null-ecb-sha256-hmac"},
};

static struct xfrm_state_afinfo *fallback_v4_afinfo;
static struct xfrm_state_afinfo *fallback_v6_afinfo;

/*
 * eip_ipsec_xfrm_check_state()
 *	check if the xfrm state is already offloaded or not.
 */
static inline bool eip_ipsec_xfrm_check_state(struct xfrm_state *xs)
{
	if (unlikely(!(xs->xflags & XFRM_STATE_OFFLOAD_NSS))) {
		return false;
	}

	return true;
}

/*
 * eip_ipsec_xfrm_xmit()
 *	This is called for IPv4/v6 pakcets that are to be transformed.
 */
static int eip_ipsec_xfrm_xmit(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	return dev_queue_xmit(skb);
}

/*
 * eip_ipsec_xfrm_v4_output()
 *	Called for IPv4 Plain text packets submitted for IPSec transformation.
 */
static int eip_ipsec_xfrm_v4_output(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct xfrm_state *xs = skb_dst(skb)->xfrm;
	struct net_device *dev;
	int ret = 0;

	/*
	 * No xfrm_state associated; Drop
	 */
	if (!xs) {
		pr_warn("%px: Failed to offload; No xfrm_state associated: drop\n", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTNOSTATES);
		goto drop;
	}

	if (!eip_ipsec_xfrm_check_state(xs)) {
		pr_warn("%px: state is not offloaded; xfrm_state %p :drop\n", skb, xs);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEINVALID);
		goto drop;
	}

	dev = xs->data;
	BUG_ON(!dev);

	skb->dev = dev;

	/*
	 * Call the Post routing hooks.
	 */
	ret = NF_HOOK_COND(NFPROTO_IPV4, NF_INET_POST_ROUTING, net, sk, skb, NULL, skb_dst(skb)->dev,
				eip_ipsec_xfrm_xmit, !(IPCB(skb)->flags & IPSKB_REROUTED));
	return ret;

drop:
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

/*
 * eip_ipsec_xfrm_esp_get_mtu()
 *	Get mtu for inner packet.
 */
static uint32_t eip_ipsec_xfrm_esp_get_mtu(struct xfrm_state *xs, int mtu)
{
	struct net_device *dev;

	/*
	 * WARN_ON if the xfrm_state is not offloaded.
	 */
	WARN_ON(!eip_ipsec_xfrm_check_state(xs));

	/*
	 * since we are tracking each encap SA using a unique
	 * netdevice, hence net_device mtu is the same as SA mtu.
	 */
	dev = xs->data;
	BUG_ON(!dev);
	mtu = dev->mtu;

	return mtu;
}

/*
 * eip_ipsec_xfrm_v6_output()
 *	Called for IPv6 Plain text packets submitted for IPSec transformation.
 */
static int eip_ipsec_xfrm_v6_output(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct xfrm_state *xs = skb_dst(skb)->xfrm;
	struct net_device *dev;
	int mtu, ret = 0;

	/*
	 * No xfrm_state associated; Drop
	 */
	if (!xs) {
		pr_warn("%px: Failed to offload; No xfrm_state associated: drop\n", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTNOSTATES);
		goto drop;
	}

	if (!eip_ipsec_xfrm_check_state(xs)) {
		pr_warn("%px: state is not offloaded; xfrm_state %p :drop\n", skb, xs);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEINVALID);
		goto drop;
	}

	dev = xs->data;
	BUG_ON(!dev);

	skb->dev = dev;

	/*
         * Reject the packets if greater than MTU.
         */
        mtu = READ_ONCE(dev->mtu);
	if (unlikely(!skb->ignore_df && skb->len > mtu)) {
		pr_debug("%px: V6 inner packet size(%d) greater than MTU(%d\n)", skb, skb->len, mtu);
		icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
		goto drop;
        }

	/*
	 * Call the Post routing hooks.
	 */
	ret = NF_HOOK_COND(NFPROTO_IPV6, NF_INET_POST_ROUTING, net, sk, skb, NULL, skb_dst(skb)->dev,
				eip_ipsec_xfrm_xmit, !(IP6CB(skb)->flags & IP6SKB_REROUTED));
	return ret;

drop:
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

/*
 * eip_ipsec_xfrm_verify_offload()
 *	Verify if the XFRM state can be offloaded or not.
 */
static bool eip_ipsec_xfrm_verify_offload(struct xfrm_state *xs)
{
	/*
	 * Tunnel and Transport Mode ESP only supported.
	 */
	if ((xs->props.mode != XFRM_MODE_TUNNEL) && (xs->props.mode != XFRM_MODE_TRANSPORT)) {
		pr_warn("%p: xfrm_state offload not allowed: Non-Transport and Non-Tunnel\n", xs);
		return false;
	}

	/*
	 * Unsupported ESP-over-UDP encap type
	 */
	if (xs->encap && (xs->encap->encap_type != UDP_ENCAP_ESPINUDP)) {
		pr_warn("%p: xfrm_state offload not allowed: Non ESP-over-UDP encap\n", xs);
		return false;
	}

	return true;
}

/*
 * eip_ipsec_xfrm_get_dev()
 *	fetch the netdevice.
 */
static struct net_device *eip_ipsec_xfrm_get_dev(struct net_device *kdev, struct sk_buff *skb, int32_t *type)
{
	uint8_t ip_ver = ip_hdr(skb)->version;
	struct net_device *dev = NULL;
	struct xfrm_state *xs = NULL;

	BUG_ON((ip_ver != IPVERSION) && (ip_ver != 6));

	/*
	 * This is a plain text packet undergoing encap.
	 */
	*type = 0;

	xs = skb_dst(skb)->xfrm;
	if (xs) {
		/*
		 * check if the xfrm state is offloaded or not.
		 */
		if (!eip_ipsec_xfrm_check_state(xs)) {
			pr_debug("%p: Outbound Plain text packet(%px); not offloaded\n", xs, skb);
			return NULL;
		}

		dev = xs->data;
		BUG_ON(!dev);
		dev_hold(dev);

		return dev;
	}

	/*
	 * dst is not xfrm. This means packet could be:-
	 * 1. Outbound encapsulated (skb->iif = ipsecX)
	 * 2. Inbound decapsulated (skb->iif = ipsecX)
	 * 3. Inbound encapsulated (ECM never sees such a packet)
	 * 4. None of the above(don't care for us)
	 */
	dev = dev_get_by_index(&init_net, skb->skb_iif);
	if (!dev) {
		return NULL;
	}

	if (!eip_ipsec_dev_is_nss(dev)) {
		pr_debug("%p: Transform packet not managed by NSS\n", xs);
		dev_put(dev);
		return NULL;
	}

	return dev;
}

/*
 * eip_ipsec_xfrm_get_algo()
 *	Fetch the driver name for the given crypto algorithm
 */
const char *eip_ipsec_xfrm_get_algo(const char *algo_name)
{
	struct eip_ipsec_xfrm_algo *xalg = xfrm_algo;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(xfrm_algo); i++, xalg++) {
		if (algo_name && strncmp(xalg->algo_name, algo_name, strlen(xalg->algo_name))) {
			continue;
		}

		return xalg->driver_name;
	}

	pr_warn("Db entry not found for the given algorithm <%s>\n", algo_name);
	return NULL;
}

/*
 * eip_ipsec_xfrm_crypto_init()
 *	Initialise the crypto parameters in sa_data.
 */
static int eip_ipsec_xfrm_crypto_init(struct xfrm_state *xs, struct eip_ipsec_data *sa_data)
{
	char algo_name[CRYPTO_MAX_ALG_NAME];

	if (xs->aead) { /* Combined mode cipher/authentication */
		unsigned int key_len = 0;

		key_len = ALIGN(xs->aead->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;

		/* Cipher */
		sa_data->base.cipher.data = xs->aead->alg_key;
		sa_data->base.cipher.len = key_len - 4; /* Subtract nonce */
		sa_data->base.icv_len = xs->aead->alg_icv_len / BITS_PER_BYTE;

		/* Nonce */
		sa_data->base.nonce = *(uint32_t *)(xs->aead->alg_key + sa_data->base.cipher.len);

		/* obtain crypto algo name */
		if (snprintf(algo_name, CRYPTO_MAX_ALG_NAME, "%s(%s)",
				xs->geniv, xs->aead->alg_name) >= CRYPTO_MAX_ALG_NAME) {
			return -ENAMETOOLONG;
		}
	} else if (xs->ealg && xs->aalg) { /* Authenticated encryption */
		/* Cipher */
		sa_data->base.cipher.data = xs->ealg->alg_key;
		sa_data->base.cipher.len = ALIGN(xs->ealg->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;

		/* Authentication */
		sa_data->base.auth.data = xs->aalg->alg_key;
		sa_data->base.auth.len = ALIGN(xs->aalg->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;
		sa_data->base.icv_len = xs->aalg->alg_trunc_len / BITS_PER_BYTE;

		/* obtain crypto algo name */
		if (snprintf(algo_name, CRYPTO_MAX_ALG_NAME, "%s%sauthenc(%s,%s)%s",
				xs->geniv ?: "", xs->geniv ? "(" : "", xs->aalg ? xs->aalg->alg_name : "digest_null",
				xs->ealg->alg_name, xs->geniv ? ")" : "") >= CRYPTO_MAX_ALG_NAME) {
			return -ENAMETOOLONG;
		}
	} else if (xs->ealg) { /* Pure encryption */
		/* Cipher */
		sa_data->base.cipher.data = xs->ealg->alg_key;
		sa_data->base.cipher.len = ALIGN(xs->ealg->alg_key_len, BITS_PER_BYTE) / BITS_PER_BYTE;

		/* Authentication */
		sa_data->base.auth.data = NULL;
		sa_data->base.auth.len = 0;
		sa_data->base.icv_len = 0;

		/* obtain crypto algo name */
		if (snprintf(algo_name, CRYPTO_MAX_ALG_NAME, "%s%sauthenc(%s,%s)%s",
				xs->geniv ?: "", xs->geniv ? "(" : "", xs->aalg ? xs->aalg->alg_name : "digest_null",
				xs->ealg->alg_name, xs->geniv ? ")" : "") >= CRYPTO_MAX_ALG_NAME) {
			return -ENAMETOOLONG;
		}
	} else { /* Bypass mode */
		sa_data->base.auth.data = NULL;
		sa_data->base.auth.len = 0;
		sa_data->base.cipher.data = NULL;
		sa_data->base.cipher.len = 0;
		sa_data->base.icv_len = 0;
		return 0;
	}

	/*
	 * fetch the driver name for the given crypto algorithm.
	 */
	sa_data->base.algo_name = eip_ipsec_xfrm_get_algo(algo_name);
	if (sa_data->base.algo_name == NULL) {
		pr_warn("Failed to find driver name for the crypto algorithm: %s\n", algo_name);
		return -1;
	}

	return 0;
}

/*
 * eip_ipsec_xfrm_sa_data_init()
 *	initialise the SA data parameters.
 */
static int eip_ipsec_xfrm_sa_data_init(struct xfrm_state *xs, struct eip_ipsec_data *sa_data)
{
	struct net_device *local_dev;
	xfrm_address_t remote = {0};
	xfrm_address_t local = {0};
	size_t ip_addr_len;
	uint8_t ttl_hop_limit;
	int ret;

	if (xs->props.family == AF_INET) {
		struct rtable *rt;

		local_dev = ip_dev_find(&init_net, xs->id.daddr.a4);

		/*
		 * if the local_dev is not a valid pointer, then
		 * it is an encap direction packet.
		 */
		if (!local_dev) {
			sa_data->base.flags |= EIP_IPSEC_FLAG_ENC;
			remote.a4 = xs->id.daddr.a4;
		} else {
			dev_put(local_dev);
			remote.a4 = xs->props.saddr.a4;
		}

		rt = ip_route_output(&init_net, remote.a4, 0, 0, 0);
		if (IS_ERR(rt)) {
			pr_warn("No IPv4 dst found\n");
			return -1;
		}

		ttl_hop_limit = ip4_dst_hoplimit(&rt->dst);
		sa_data->mtu = READ_ONCE(rt->dst.dev->mtu);
		ip_rt_put(rt);
	} else {
		struct rt6_info *rt6;

		sa_data->base.flags |= EIP_IPSEC_FLAG_IPV6;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		local_dev = ipv6_dev_find(&init_net, &xs->id.daddr.in6, 1);
#else
		rcu_read_lock();
		local_dev = ipv6_dev_find(&init_net, &xs->id.daddr.in6, NULL);
		if (local_dev)
			dev_hold(local_dev);

		rcu_read_unlock();
#endif

		ip_addr_len = sizeof(remote.in6);

		/*
		 * if the local_dev is not a valid pointer, then
		 * it is an encap direction packet.
		 */
		if (!local_dev) {
			sa_data->base.flags |= EIP_IPSEC_FLAG_ENC;
			memcpy(&local.in6, &xs->props.saddr.in6, ip_addr_len);
			memcpy(&remote.in6, &xs->id.daddr.in6, ip_addr_len);
		} else {
			dev_put(local_dev);
			memcpy(&local.in6, &xs->id.daddr.in6, ip_addr_len);
			memcpy(&remote.in6, &xs->props.saddr.in6, ip_addr_len);
		}

		rt6 = rt6_lookup(&init_net, &remote.in6, NULL, 0, 0, 0);
		if (!rt6) {
			rt6 = rt6_lookup(&init_net, &remote.in6, &local.in6, 0, 0, 0);
			if (!rt6) {
				pr_warn("No IPv6 dst found\n");
				return -1;
			}
		}

		ttl_hop_limit = ip6_dst_hoplimit(&rt6->dst);
		sa_data->mtu = READ_ONCE(rt6->dst.dev->mtu);
		ip6_rt_put(rt6);
	}

	if (xs->props.mode == XFRM_MODE_TUNNEL) {
		sa_data->base.flags |= EIP_IPSEC_FLAG_TUNNEL;
	}

	if (xs->encap) {
		sa_data->base.flags |= EIP_IPSEC_FLAG_UDP;
	}

	ret = eip_ipsec_xfrm_crypto_init(xs, sa_data);
	if (ret < 0) {
		pr_warn("Crypto Initialisation failed\n");
		return ret;
	}

	sa_data->xs = xs;

	if (sa_data->base.flags & EIP_IPSEC_FLAG_ENC) {
		sa_data->base.flags |= EIP_IPSEC_FLAG_CP_DF;
		sa_data->base.flags |= EIP_IPSEC_FLAG_CP_TOS;
		sa_data->hop_limit = ttl_hop_limit;
		sa_data->df = 0;
		return 0;
	}

	/*
	 * TODO: what is the correct value to be set for df, dscp, icv_len fields ?
	 * TODO: XFRM only support 32bit. Check if there is any seperate option.
	 * replay_win is in bits.
	 */
	sa_data->replay_win = xs->props.replay_window;
	return 0;
}

/*
 * eip_ipsec_xfrm_sa_tuple_init()
 *	initialise the SA tuple parameters.
 */
static void eip_ipsec_xfrm_sa_tuple_init(struct xfrm_state *xs, struct eip_ipsec_tuple *sa_tuple)
{
	sa_tuple->esp_spi = ntohl(xs->id.spi);
	sa_tuple->protocol = IPPROTO_ESP;
	sa_tuple->sport = 0;
	sa_tuple->dport = 0;

	/*
	 * Check if this is ESP-over-UDP SA
	 */
	if (xs->encap) {
		sa_tuple->sport = ntohs(xs->encap->encap_sport);
		sa_tuple->dport = ntohs(xs->encap->encap_dport);
		sa_tuple->protocol = IPPROTO_UDP;
	}

	if (xs->props.family == AF_INET) {
		sa_tuple->ip_version = IPVERSION;
		sa_tuple->src_ip[0] = ntohl(xs->props.saddr.a4);
		sa_tuple->dest_ip[0] = ntohl(xs->id.daddr.a4);
	} else {
		sa_tuple->ip_version = 6;

		sa_tuple->src_ip[0] = ntohl(xs->props.saddr.a6[0]);
		sa_tuple->src_ip[1] = ntohl(xs->props.saddr.a6[1]);
		sa_tuple->src_ip[2] = ntohl(xs->props.saddr.a6[2]);
		sa_tuple->src_ip[3] = ntohl(xs->props.saddr.a6[3]);

		sa_tuple->dest_ip[0] = ntohl(xs->id.daddr.a6[0]);
		sa_tuple->dest_ip[1] = ntohl(xs->id.daddr.a6[1]);
		sa_tuple->dest_ip[2] = ntohl(xs->id.daddr.a6[2]);
		sa_tuple->dest_ip[3] = ntohl(xs->id.daddr.a6[3]);
	}
}

/*
 * eip_ipsec_xfrm_state_add()
 *	xfrm_state add notification handler.
 */
static int eip_ipsec_xfrm_state_add(struct xfrm_state *xs)
{
	struct eip_ctx *ctx = eip_ipsec_drv_g.ctx;
	struct eip_ipsec_tuple sa_tuple = {0};
	struct eip_ipsec_data sa_data = {0};
	struct net_device *dev;
	int ret;

	/*
	 * check if the xfrm state is already offloaded or not.
	 */
	if (eip_ipsec_xfrm_check_state(xs)) {
		pr_warn("%p: xfrm_state is already offloaded\n", xs);
		WARN_ON(1);
		return -EEXIST;
	}

	/*
	 * verify whether the xfrm state can be offloaded or not.
	 */
	if (!eip_ipsec_xfrm_verify_offload(xs)) {
		pr_warn("%p: XFRM state offloaded is not allowed\n", xs);
		return -ENOTSUPP;
	}

	/*
	 * before adding a new SA object, run through the db and find out
	 * if there is a net device already created for the given (id).
	 * If not present, then create one.
	 *
	 * TODO: How do we handle race-condition ?
	 */
	dev = eip_ipsec_dev_get_by_id(xs->props.reqid);
	if (!dev) {
		pr_debug("%p: Netdevice not created for reqid(%d); Add device\n", xs, xs->props.reqid);
		dev = eip_ipsec_dev_add();
		if (!dev) {
			pr_err("%p: Netdevice creation failed for reqid(%d)\n", xs, xs->props.reqid);
			return -ENODEV;
		}

		eip_ipsec_dev_link_id(dev, xs->props.reqid);
	}

	/*
	 * initialize the sa_data and sa_tuple objects.
	 */
	eip_ipsec_xfrm_sa_tuple_init(xs, &sa_tuple);

	ret = eip_ipsec_xfrm_sa_data_init(xs, &sa_data);
	if (ret < 0) {
		pr_warn("%p: unable to initialize xfrm_state\n", xs);
		if (!eip_ipsec_dev_sa_exist(dev)) {
			eip_ipsec_dev_unlink_id(dev, xs->props.reqid);
			eip_ipsec_dev_del(dev);
		}

		return ret;
	}

	ret = eip_ipsec_sa_add(dev, &sa_data, &sa_tuple);
	if (ret < 0) {
		pr_warn("%p: unable to offload xfrm_state\n", xs);
		if (!eip_ipsec_dev_sa_exist(dev)) {
			eip_ipsec_dev_unlink_id(dev, xs->props.reqid);
			eip_ipsec_dev_del(dev);
		}

		return ret;
	}

	/*
	 * dereference: eip_ipsec_xfrm_state_delete()
	 */
	dev_hold(dev);
	xs->data = dev;
	xs->xflags |= XFRM_STATE_OFFLOAD_NSS;

	/*
	 * Set XFRM_STATE_OFFLOAD_NSS_HW in xfrm state.
	 */
	if (eip_feature_check(ctx, EIP_OFFLOAD_INNER_FLOW)) {
		xs->xflags |= XFRM_STATE_OFFLOAD_HW_INNER;
	}

	return 0;
}

/*
 * eip_ipsec_xfrm_state_delete()
 *	xfrm_state delete notification handler.
 */
static void eip_ipsec_xfrm_state_delete(struct xfrm_state *xs)
{
	struct eip_ipsec_tuple sa_tuple;
	struct net_device *dev;

	/*
	 * check if the xfrm state is already offloaded or not.
	 */
	if (!eip_ipsec_xfrm_check_state(xs)) {
		pr_warn("%p: xfrm_state is not offloaded\n", xs);
		return;
	}

	/*
	 * SA tuple initialise.
	 */
	eip_ipsec_xfrm_sa_tuple_init(xs, &sa_tuple);

	/*
	 * fetch the net_deivce from the xfrm state.
	 * reference: eip_ipsec_xfrm_state_add()
	 */
	dev = xs->data;
	dev_put(dev);

	eip_ipsec_sa_del(dev, &sa_tuple);

	if (eip_ipsec_dev_sa_exist(dev)) {
		pr_debug("%p: Encap & Decap SA's are still linked to the given net_device %p\n", xs, dev);
		return;
	}

	eip_ipsec_dev_unlink_id(dev, xs->props.reqid);
	eip_ipsec_dev_del(dev);
}

/*
 * eip_ipsec_xfrm_esp_init_state()
 *	Initialize IPsec xfrm state of type ESP.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
static int eip_ipsec_xfrm_esp_init_state(struct xfrm_state *xs)
#else
static int eip_ipsec_xfrm_esp_init_state(struct xfrm_state *xs, struct netlink_ext_ack *extack)
#endif
{
	if (eip_ipsec_xfrm_state_add(xs)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
		NL_SET_ERR_MSG(extack, "Kernel is unable to initialize xfrm state");
#endif
		pr_err("%p: Failed to offload xfrm state\n", xs);
		return -1;
	}

	return 0;
}

/*
 * eip_ipsec_xfrm_esp_deinit_state()
 *	Destroy IPsec xfrm state of type ESP.
 */
static void eip_ipsec_xfrm_esp_deinit_state(struct xfrm_state *xs)
{
	eip_ipsec_xfrm_state_delete(xs);
	return;
}

static struct ecm_interface_ipsec_callback xfrm_ecm_ipsec_cb = {
	.tunnel_get_and_hold = eip_ipsec_xfrm_get_dev,
};

/*
 * Trapping ipv4 packets to be sent for ipsec encapsulation.
 */
static struct xfrm_state_afinfo xfrm_v4_afinfo = {
	.family = AF_INET,
	.proto = IPPROTO_IPIP,
	.output = eip_ipsec_xfrm_v4_output,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	.output_finish = NULL,
	.extract_input = NULL,
	.extract_output = NULL,
#endif
	.transport_finish = NULL,
	.local_error = NULL,
};

/*
 * ESP proto specific init/de-init handlers for ipv4.
 */
static const struct xfrm_type xfrm_v4_type = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	.description = "NSS ESP4",
#endif
	.owner = THIS_MODULE,
	.proto = IPPROTO_ESP,
	.flags = XFRM_TYPE_REPLAY_PROT,
	.init_state = eip_ipsec_xfrm_esp_init_state,
	.destructor = eip_ipsec_xfrm_esp_deinit_state,
	.get_mtu = eip_ipsec_xfrm_esp_get_mtu,
	.input = NULL,
	.output = NULL,
};

/*
 * Trapping ipv6 packets to be sent for ipsec encapsulation.
 */
static struct xfrm_state_afinfo xfrm_v6_afinfo = {
	.family = AF_INET6,
	.proto = IPPROTO_IPV6,
	.output = eip_ipsec_xfrm_v6_output,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	.output_finish = NULL,
	.extract_input = NULL,
	.extract_output = NULL,
#endif
	.transport_finish = NULL,
	.local_error = NULL,
};

/*
 * ESP proto specific init/de-init handlers for ipv6.
 */
static const struct xfrm_type xfrm_v6_type = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	.description = "NSS ESP6",
#endif
	.owner = THIS_MODULE,
	.proto = IPPROTO_ESP,
	.flags = XFRM_TYPE_REPLAY_PROT,
	.init_state = eip_ipsec_xfrm_esp_init_state,
	.destructor = eip_ipsec_xfrm_esp_deinit_state,
	.get_mtu = eip_ipsec_xfrm_esp_get_mtu,
	.input = NULL,
	.output = NULL,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	.hdr_offset = NULL,
#endif
};

/*
 * eip_ipsec_xfrm_restore_afinfo()
 *	Restore the native linux afinfo object.
 */
static void eip_ipsec_xfrm_restore_afinfo(uint16_t family)
{
	const struct xfrm_type *type_dstopts, *type_routing;
	const struct xfrm_type *type_ipip, *type_ipv6;
	const struct xfrm_type *type_ah, *type_comp;
	struct xfrm_state_afinfo *afinfo;
	const struct xfrm_type *base;

	/*
	 * unregister the xfrm types
	 */
	if (family == AF_INET) {
		base = &xfrm_v4_type;
		afinfo = fallback_v4_afinfo;
	} else {
		base = &xfrm_v6_type;
		afinfo = fallback_v6_afinfo;
	}

	BUG_ON(!afinfo);

	type_ah = afinfo->type_ah;
	type_comp = afinfo->type_comp;
	type_ipip = afinfo->type_ipip;
	type_ipv6 = afinfo->type_ipip6;
	type_dstopts = afinfo->type_dstopts;
	type_routing = afinfo->type_routing;

	/*
	 * Unregister types.
	 */
	if (type_routing) {
		xfrm_unregister_type(type_routing, family);
	}

	if (type_dstopts) {
		xfrm_unregister_type(type_dstopts, family);
	}

	if (type_ipv6) {
		xfrm_unregister_type(type_ipv6, family);
	}

	if (type_ipip) {
		xfrm_unregister_type(type_ipip, family);
	}

	if (type_comp) {
		xfrm_unregister_type(type_comp, family);
	}

	if (type_ah) {
		xfrm_unregister_type(type_ah, family);
	}

	xfrm_unregister_type(base, family);
	xfrm_state_update_afinfo(family, afinfo);
}

/*
 * eip_ipsec_xfrm_override_afinfo()
 *	Override the native linux afinfo object.
 */
static void eip_ipsec_xfrm_override_afinfo(uint16_t family)
{
	const struct xfrm_type *type_dstopts, *type_routing;
	const struct xfrm_type *type_ipip, *type_ipv6;
	const struct xfrm_type *type_ah, *type_comp;
	struct xfrm_state_afinfo *afinfo;
	const struct xfrm_type *base;

	/*
	 * Override ESP type.
	 */
	if (family == AF_INET) {
		base = &xfrm_v4_type;
		afinfo = fallback_v4_afinfo = xfrm_state_update_afinfo(AF_INET, &xfrm_v4_afinfo);
	} else {
		base = &xfrm_v6_type;
		afinfo = fallback_v6_afinfo = xfrm_state_update_afinfo(AF_INET6, &xfrm_v6_afinfo);
	}

	BUG_ON(!afinfo);

	xfrm_register_type(base, family);

	type_ah = afinfo->type_ah;
	type_comp = afinfo->type_comp;
	type_ipip = afinfo->type_ipip;
	type_ipv6 = afinfo->type_ipip6;
	type_dstopts = afinfo->type_dstopts;
	type_routing = afinfo->type_routing;

	/*
	 * Register types
	 *
	 * Propagating the registered xfrm_type from
	 * old afinfo object into new object.
	 */
	if (type_ah) {
		xfrm_register_type(type_ah, family);
	}

	if (type_comp) {
		xfrm_register_type(type_comp, family);
	}

	if (type_ipip) {
		xfrm_register_type(type_ipip, family);
	}

	if (type_ipv6) {
		xfrm_register_type(type_ipv6, family);
	}

	if (type_dstopts) {
		xfrm_register_type(type_dstopts, family);
	}

	if (type_routing) {
		xfrm_register_type(type_routing, family);
	}
}

/*
 * eip_ipsec_xfrm_init()
 *	initialization function
 */
int eip_ipsec_xfrm_init(void)
{

	/*
	 * overide the xfrm_state afinfo.
	 */
	if (!disable_v4_offload)
		eip_ipsec_xfrm_override_afinfo(AF_INET);

	eip_ipsec_xfrm_override_afinfo(AF_INET6);

	ecm_interface_ipsec_register_callbacks(&xfrm_ecm_ipsec_cb);

	pr_info("eip xfrm module loaded\n");
	return 0;
}

/*
 * eip_ipsec_xfrm_exit()
 *	exit function
 */
void eip_ipsec_xfrm_exit(void)
{
	/*
	 * Restore the xfrm_state afinfo.
	 */
	if (!disable_v4_offload)
		eip_ipsec_xfrm_restore_afinfo(AF_INET);

	eip_ipsec_xfrm_restore_afinfo(AF_INET6);

	ecm_interface_ipsec_unregister_callbacks();

	pr_info("eip xfrm module unloaded\n");
}
