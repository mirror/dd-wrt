/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
 *  Authentication Copyright 2002 Arcturus Networks Inc.
 *      by Norman Shulman <norm@arcturusnetworks.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  This file was created using net/ipv4/ipip.c in Linux 2.4.18 as
 *  a template.  Without such a good start, this project would probably
 *  never have come into existance.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/if_arp.h>
#include <linux/mroute.h>
#include <linux/init.h>
#include <linux/netfilter_ipv4.h>
#include <linux/crypto.h>
#include <linux/random.h>

#include <net/sock.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/ipip.h>
#include <net/protocol.h>
#include <net/inet_ecn.h>

#include "ipsec_sa.h"

#ifndef ARPHRD_IPSEC
#define ARPHRD_IPSEC 31
#endif

#define HASH_SIZE  16
#define HASH(addr) ((addr^(addr>>4))&0xF)

#ifdef ALIGN
#undef ALIGN
#endif
#define ALIGN(x,a) (((x)+(a)-1) & ~((a)-1))

#define SIOCIPSEC_GET_TUNNEL   (SIOCDEVPRIVATE + 0)
#define SIOCIPSEC_ADD_TUNNEL   (SIOCDEVPRIVATE + 1)
#define SIOCIPSEC_DEL_TUNNEL   (SIOCDEVPRIVATE + 2)
#define SIOCIPSEC_CHG_TUNNEL   (SIOCDEVPRIVATE + 3)
#define SIOCIPSEC_GET_SA       (SIOCDEVPRIVATE + 4)
#define SIOCIPSEC_ADD_SA       (SIOCDEVPRIVATE + 5)
#define SIOCIPSEC_DEL_SA       (SIOCDEVPRIVATE + 6)
#define SIOCIPSEC_CHG_SA       (SIOCDEVPRIVATE + 7)
#define SIOCIPSEC_GET_STATS    (SIOCDEVPRIVATE + 8)

/* Keep error state on tunnel for 30 sec */
#define IPTUNNEL_ERR_TIMEO	(30*HZ)

/* Early versions of the cryptoapi did not have a version identifier,
 * so we emulate it here, and set the version to 0,0,1. */
#if !defined(CRYPTO_API_VERSION_CODE)
#define CRYPTO_API_VERSION_CODE 0x000001
#endif

/* The API was changed in version 0.1.0: */
#if CRYPTO_API_VERSION_CODE < 0x000100
#define decrypt_atomic_iv decrypt_atomic
#define encrypt_atomic_iv encrypt_atomic
#endif

struct esphdr {
	__u32 spi;
	__u32 seq;
	__u8 iv[0];
};

#define IPSEC_SA_VERSION              0
#define IPSEC_SA_CRYPTOLEN            32
#define IPSEC_STATS_VERSION           1
#define MAX_HMACLEN                   IPSEC_MAX_DIGEST_KEYLEN

struct ipsec_sa_parm
{
	u32         version;

	u32         dst;
	u32         src;
	u32         spi;
	u32         flags;

	char        cipher[IPSEC_SA_CRYPTOLEN];
	int         cipher_keylen;
	const void *cipher_key;
	char        digest[IPSEC_SA_CRYPTOLEN];
	int         digest_keylen;
	const void *digest_key;
	int         digest_hmaclen;
};

struct ipsec_stats
{
	unsigned long	version;

	unsigned long	rx_unknown_sa;
	unsigned long	rx_auth_fail;
	unsigned long	rx_padding_fail;
	unsigned long	rx_non_tunnel;
	unsigned long	rx_no_tunnel;
	unsigned long	rx_mem;
	unsigned long	rx_other;
	unsigned long	rx_ok;

	unsigned long	tx_unknown_sa;
	unsigned long	tx_recursion;
	unsigned long	tx_route;
	unsigned long	tx_mtu;
	unsigned long	tx_mem;
	unsigned long	tx_other;
	unsigned long	tx_ok;
};

struct ipsec_tunnel_parm
{
	char		name[IFNAMSIZ];
	int		link;
	struct iphdr	iph;
	int		spi;
};

struct ipsec_tunnel
{
	struct ipsec_tunnel	*next;
	struct net_device	*dev;
	struct net_device_stats	stat;

	int			recursion;	/* Depth of hard_start_xmit recursion */
	int			err_count;	/* Number of arrived ICMP errors */
	unsigned long		err_time;	/* Time when the last ICMP error arrived */

	struct ipsec_tunnel_parm parms;
};

static int ipsec_fb_tunnel_init(struct net_device *dev);
static int ipsec_tunnel_init(struct net_device *dev);

static struct ipsec_stats s_stats = { IPSEC_STATS_VERSION };

static struct net_device ipsec_fb_tunnel_dev = {
	name:	"ipsec0",
	init:	ipsec_fb_tunnel_init,
};

static struct ipsec_tunnel ipsec_fb_tunnel = {
	dev:	&ipsec_fb_tunnel_dev,
	parms:	{ name:	"ipsec0", }
};

static struct ipsec_tunnel *tunnels_r_l[HASH_SIZE];
static struct ipsec_tunnel *tunnels_r[HASH_SIZE];
static struct ipsec_tunnel *tunnels_l[HASH_SIZE];
static struct ipsec_tunnel *tunnels_wc[1];
static struct ipsec_tunnel **tunnels[4] = { tunnels_wc, tunnels_l, tunnels_r, tunnels_r_l };

static rwlock_t ipsec_lock = RW_LOCK_UNLOCKED;

static struct ipsec_tunnel * ipsec_tunnel_lookup(u32 remote, u32 local)
{
	unsigned h0 = HASH(remote);
	unsigned h1 = HASH(local);
	struct ipsec_tunnel *t;

	for (t = tunnels_r_l[h0^h1]; t; t = t->next) {
		if (local == t->parms.iph.saddr &&
		    remote == t->parms.iph.daddr && (t->dev->flags&IFF_UP))
			return t;
	}
	for (t = tunnels_r[h0]; t; t = t->next) {
		if (remote == t->parms.iph.daddr && (t->dev->flags&IFF_UP))
			return t;
	}
	for (t = tunnels_l[h1]; t; t = t->next) {
		if (local == t->parms.iph.saddr && (t->dev->flags&IFF_UP))
			return t;
	}
	if ((t = tunnels_wc[0]) != NULL && (t->dev->flags&IFF_UP))
		return t;
	return NULL;
}

static struct ipsec_tunnel **ipsec_bucket(struct ipsec_tunnel *t)
{
	u32 remote = t->parms.iph.daddr;
	u32 local = t->parms.iph.saddr;
	unsigned h = 0;
	int prio = 0;

	if (remote) {
		prio |= 2;
		h ^= HASH(remote);
	}
	if (local) {
		prio |= 1;
		h ^= HASH(local);
	}
	return &tunnels[prio][h];
}


static void ipsec_tunnel_unlink(struct ipsec_tunnel *t)
{
	struct ipsec_tunnel **tp;

	write_lock_bh(&ipsec_lock);
	for (tp = ipsec_bucket(t); *tp; tp = &(*tp)->next) {
		if (t == *tp) {
			*tp = t->next;
			break;
		}
	}
	write_unlock_bh(&ipsec_lock);
}

static void ipsec_tunnel_link(struct ipsec_tunnel *t)
{
	struct ipsec_tunnel **tp = ipsec_bucket(t);

	write_lock_bh(&ipsec_lock);
	t->next = *tp;
	*tp = t;
	write_unlock_bh(&ipsec_lock);
}

struct ipsec_tunnel *ipsec_tunnel_locate(struct ipsec_tunnel_parm *parms)
{
	u32 remote = parms->iph.daddr;
	u32 local = parms->iph.saddr;
	struct ipsec_tunnel *t, **tp;
	unsigned h = 0;
	int prio = 0;

	if (remote) {
		prio |= 2;
		h ^= HASH(remote);
	}
	if (local) {
		prio |= 1;
		h ^= HASH(local);
	}
	for (tp = &tunnels[prio][h]; (t = *tp) != NULL; tp = &t->next) {
		if (local == t->parms.iph.saddr && remote == t->parms.iph.daddr)
			return t;
	}

	return NULL;
}

struct ipsec_tunnel *ipsec_tunnel_create(struct ipsec_tunnel_parm *parms)
{
	struct ipsec_tunnel *t, *nt;
	struct net_device *dev;

	MOD_INC_USE_COUNT;
	dev = kmalloc(sizeof(*dev) + sizeof(*t), GFP_KERNEL);
	if (dev == NULL) {
		MOD_DEC_USE_COUNT;
		return NULL;
	}
	memset(dev, 0, sizeof(*dev) + sizeof(*t));
	dev->priv = (void*)(dev+1);
	nt = (struct ipsec_tunnel*)dev->priv;
	nt->dev = dev;

	dev->init = ipsec_tunnel_init;
	dev->features |= NETIF_F_DYNALLOC;
	memcpy(&nt->parms, parms, sizeof(*parms));
	nt->parms.name[IFNAMSIZ-1] = '\0';
	strcpy(dev->name, nt->parms.name);
	if (dev->name[0] == 0) {
		int i;
		for (i=1; i<100; i++) {
			sprintf(dev->name, "ipsec%d", i);
			if (__dev_get_by_name(dev->name) == NULL)
				break;
		}
		if (i==100)
			goto failed;
		memcpy(nt->parms.name, dev->name, IFNAMSIZ);
	}
	if (register_netdevice(dev) < 0)
		goto failed;

	dev_hold(dev);
	ipsec_tunnel_link(nt);
	/* Do not decrement MOD_USE_COUNT here. */
	return nt;

failed:
	kfree(dev);
	MOD_DEC_USE_COUNT;
	return NULL;
}

static void ipsec_tunnel_destructor(struct net_device *dev)
{
	if (dev != &ipsec_fb_tunnel_dev) {
		MOD_DEC_USE_COUNT;
	}
}

static void ipsec_tunnel_uninit(struct net_device *dev)
{
	if (dev == &ipsec_fb_tunnel_dev) {
		write_lock_bh(&ipsec_lock);
		tunnels_wc[0] = NULL;
		write_unlock_bh(&ipsec_lock);
	} else
		ipsec_tunnel_unlink((struct ipsec_tunnel*)dev->priv);
	dev_put(dev);
}

void ipsec_esp_err(struct sk_buff *skb, u32 info)
{
#ifndef I_WISH_WORLD_WERE_PERFECT

/* It is not :-( All the routers (except for Linux) return only
   8 bytes of packet payload. It means, that precise relaying of
   ICMP in the real Internet is absolutely infeasible.
 */
	struct iphdr *iph = (struct iphdr*)skb->data;
	int type = skb->h.icmph->type;
	int code = skb->h.icmph->code;
	struct ipsec_tunnel *t;

	switch (type) {
	default:
	case ICMP_PARAMETERPROB:
		return;

	case ICMP_DEST_UNREACH:
		switch (code) {
		case ICMP_SR_FAILED:
		case ICMP_PORT_UNREACH:
			/* Impossible event. */
			return;
		case ICMP_FRAG_NEEDED:
			/* Soft state for pmtu is maintained by IP core. */
			return;
		default:
			/* All others are translated to HOST_UNREACH.
			   rfc2003 contains "deep thoughts" about NET_UNREACH,
			   I believe they are just ether pollution. --ANK
			 */
			break;
		}
		break;
	case ICMP_TIME_EXCEEDED:
		if (code != ICMP_EXC_TTL)
			return;
		break;
	}

	read_lock(&ipsec_lock);
	t = ipsec_tunnel_lookup(iph->daddr, iph->saddr);
	if (t == NULL || t->parms.iph.daddr == 0)
		goto out;
	if (t->parms.iph.ttl == 0 && type == ICMP_TIME_EXCEEDED)
		goto out;

	if (jiffies - t->err_time < IPTUNNEL_ERR_TIMEO)
		t->err_count++;
	else
		t->err_count = 1;
	t->err_time = jiffies;
out:
	read_unlock(&ipsec_lock);
	return;
#else
	struct iphdr *iph = (struct iphdr*)dp;
	int hlen = iph->ihl<<2;
	struct iphdr *eiph;
	int type = skb->h.icmph->type;
	int code = skb->h.icmph->code;
	int rel_type = 0;
	int rel_code = 0;
	int rel_info = 0;
	struct sk_buff *skb2;
	struct rtable *rt;

	if (len < hlen + sizeof(struct iphdr))
		return;
	eiph = (struct iphdr*)(dp + hlen);

	switch (type) {
	default:
		return;
	case ICMP_PARAMETERPROB:
		if (skb->h.icmph->un.gateway < hlen)
			return;

		/* So... This guy found something strange INSIDE encapsulated
		   packet. Well, he is fool, but what can we do ?
		 */
		rel_type = ICMP_PARAMETERPROB;
		rel_info = skb->h.icmph->un.gateway - hlen;
		break;

	case ICMP_DEST_UNREACH:
		switch (code) {
		case ICMP_SR_FAILED:
		case ICMP_PORT_UNREACH:
			/* Impossible event. */
			return;
		case ICMP_FRAG_NEEDED:
			/* And it is the only really necesary thing :-) */
			rel_info = ntohs(skb->h.icmph->un.frag.mtu);
			if (rel_info < hlen+68)
				return;
			rel_info -= hlen;
			/* BSD 4.2 MORE DOES NOT EXIST IN NATURE. */
			if (rel_info > ntohs(eiph->tot_len))
				return;
			break;
		default:
			/* All others are translated to HOST_UNREACH.
			   rfc2003 contains "deep thoughts" about NET_UNREACH,
			   I believe, it is just ether pollution. --ANK
			 */
			rel_type = ICMP_DEST_UNREACH;
			rel_code = ICMP_HOST_UNREACH;
			break;
		}
		break;
	case ICMP_TIME_EXCEEDED:
		if (code != ICMP_EXC_TTL)
			return;
		break;
	}

	/* Prepare fake skb to feed it to icmp_send */
	skb2 = skb_clone(skb, GFP_ATOMIC);
	if (skb2 == NULL)
		return;
	dst_release(skb2->dst);
	skb2->dst = NULL;
	skb_pull(skb2, skb->data - (u8*)eiph);
	skb2->nh.raw = skb2->data;

	/* Try to guess incoming interface */
	if (ip_route_output(&rt, eiph->saddr, 0, RT_TOS(eiph->tos), 0)) {
		kfree_skb(skb2);
		return;
	}
	skb2->dev = rt->u.dst.dev;

	/* route "incoming" packet */
	if (rt->rt_flags&RTCF_LOCAL) {
		ip_rt_put(rt);
		rt = NULL;
		if (ip_route_output(&rt, eiph->daddr, eiph->saddr, eiph->tos, 0) ||
		    rt->u.dst.dev->type != ARPHRD_IPGRE) {
			ip_rt_put(rt);
			kfree_skb(skb2);
			return;
		}
	} else {
		ip_rt_put(rt);
		if (ip_route_input(skb2, eiph->daddr, eiph->saddr, eiph->tos, skb2->dev) ||
		    skb2->dst->dev->type != ARPHRD_IPGRE) {
			kfree_skb(skb2);
			return;
		}
	}

	/* change mtu on this route */
	if (type == ICMP_DEST_UNREACH && code == ICMP_FRAG_NEEDED) {
		if (rel_info > skb2->dst->pmtu) {
			kfree_skb(skb2);
			return;
		}
		skb2->dst->pmtu = rel_info;
		rel_info = htonl(rel_info);
	} else if (type == ICMP_TIME_EXCEEDED) {
		struct ipsec_tunnel *t = (struct ipsec_tunnel*)skb2->dev->priv;
		if (t->parms.iph.ttl) {
			rel_type = ICMP_DEST_UNREACH;
			rel_code = ICMP_HOST_UNREACH;
		}
	}

	icmp_send(skb2, rel_type, rel_code, rel_info);
	kfree_skb(skb2);
	return;
#endif
}

static inline void ipsec_ecn_decapsulate(struct iphdr *iph, struct sk_buff *skb)
{
	if (INET_ECN_is_ce(iph->tos) &&
	    INET_ECN_is_not_ce(skb->nh.iph->tos))
		IP_ECN_set_ce(iph);
}

#if 0
static void dump_packet(struct sk_buff *skb)
{
	int i;

	printk(KERN_DEBUG "IPsec: Packet:");
	for (i = 0; i < skb->len; i+=4)
	{
		printk(" %02x%02x%02x%02x",
			   skb->data[i],
			   skb->data[i+1],
			   skb->data[i+2],
			   skb->data[i+3]);
	}
	printk("\n");
}
#endif

struct sk_buff *
ipsec_esp_encapsulate(struct ipsec_sa *sa, struct sk_buff *skb)
{
	struct cipher_context *cipher = sa->cipher;
	struct digest_context *digest = sa->digest;
	int esp_len;
	int padding;
	struct sk_buff *tmp_skb = skb;
	unsigned char *tail;
	struct esphdr *esp;
	size_t alignment = 1;
	size_t headroom = sizeof(struct esphdr);
	size_t tailroom = 2;
	u8 *auth_pos;
	u8 hmac[MAX_HMACLEN];

	if (cipher) {
		alignment = cipher->ci->blocksize;
		headroom += cipher->ci->ivsize;
	}

	if (digest) {
		alignment = alignment > 4 ? alignment : 4;
		tailroom += sa->digest_hmaclen;
	}

	esp_len = ALIGN(skb->len + 2, alignment);
	padding = esp_len - skb->len - 2;
	tailroom += padding;

	skb = skb_copy_expand(
		skb,
		skb_headroom(skb) + sizeof(struct iphdr) + headroom,
		skb_tailroom(skb) + tailroom,
		GFP_ATOMIC);
	if (skb && tmp_skb->sk)
		skb_set_owner_w(skb, tmp_skb->sk);
	kfree_skb(tmp_skb);
	if (!skb) {
		s_stats.tx_mem++;
		return NULL;
	}

	/* Add padding, pad length, next header and any authenticator */
	auth_pos = skb->tail + padding + 2;
	skb_put(skb, tailroom);
	tail = auth_pos;
	*--tail = IPPROTO_IPIP;
	*--tail = padding;
	while (padding > 0)
		*--tail = padding--;

	/* Install the ESP header */
	esp = (struct esphdr *)skb_push(skb, headroom);
	esp->spi = htonl(sa->spi);
	esp->seq = htonl(++sa->tx_seq);

	if (cipher) {
		get_random_bytes(esp->iv, cipher->ci->ivsize);

		cipher->ci->encrypt_atomic_iv(
			cipher,
			esp->iv + cipher->ci->ivsize,
			esp->iv + cipher->ci->ivsize,
			esp_len,
			(const void*)esp->iv);
	}

	if (digest) {
		digest->di->hmac_atomic(
			digest,
			sa->digest_key,
			sa->digest_keylen, 
			(u8*)esp,
			auth_pos - (u8*)esp,
			hmac);
		memcpy(auth_pos, hmac, sa->digest_hmaclen);
	}

	return skb;
}

struct sk_buff *
ipsec_esp_decapsulate(struct sk_buff *skb, u8 *nextprot)
{
	struct ipsec_sa *sa;
	struct esphdr *esp;
	struct cipher_implementation *ci;
	struct digest_implementation *di;
	unsigned char *tail;
	int padding;
	u8 *auth_pos;
	u8 hmac[MAX_HMACLEN];

	if (skb_cloned(skb) || skb_shared(skb)) {
		skb = skb_unshare(skb, GFP_ATOMIC);
		if (!skb) {
			s_stats.rx_mem++;
			return NULL;
		}
	}

	if (skb_is_nonlinear(skb) && skb_linearize(skb, GFP_ATOMIC)) {
		s_stats.rx_mem++;
		goto err;
	}

	esp = (struct esphdr *)skb->data;
	if (ntohl(esp->spi) == IPSEC_SPI_ANY ||
	    (sa = ipsec_sa_get(skb->nh.iph->daddr, skb->nh.iph->saddr, ntohl(esp->spi))) == NULL) {
		s_stats.rx_unknown_sa++;
		goto err;
	}

	if (sa->digest) {
		di = sa->digest->di;

		if (skb->len < sa->digest_hmaclen) {
			s_stats.rx_other++;
			goto err;
		}

		auth_pos = skb->tail - sa->digest_hmaclen;

		di->hmac_atomic(
			sa->digest,
			sa->digest_key,
			sa->digest_keylen, 
			(u8*)esp,
			auth_pos - (u8*)esp,
			hmac);

		if (memcmp(auth_pos, hmac, sa->digest_hmaclen) != 0) {
			ipsec_sa_put(sa);
			s_stats.rx_auth_fail++;
			goto err;
		}

		skb_trim(skb, skb->len - sa->digest_hmaclen);
	}

	if (!skb_pull(skb, sizeof(struct esphdr))) {
		ipsec_sa_put(sa);
		s_stats.rx_other++;
		goto err;
	}

	if (sa->cipher) {
		ci = sa->cipher->ci;

		ci->decrypt_atomic_iv(
			sa->cipher,
			esp->iv + ci->ivsize,
			esp->iv + ci->ivsize,
			skb->len - ci->ivsize,
			(const void*)esp->iv);

		if (!skb_pull(skb, ci->ivsize)) {
			ipsec_sa_put(sa);
			s_stats.rx_other++;
			goto err;
		}
	}

	ipsec_sa_put(sa);

	/* Verify padding */
	tail = skb->tail;
	*nextprot = *--tail;
	padding = *--tail;
	if (padding > skb->len - 2) {
		s_stats.rx_padding_fail++;
		goto err;
	}
	while (padding > 0) {
		if (*--tail != padding--) {
			s_stats.rx_padding_fail++;
			goto err;
		}
	}
	skb_trim(skb, skb->len - 2 - *(skb->tail-2));

	return skb;

err:
	kfree_skb(skb);
	return NULL;
}

int ipsec_esp_rcv_tunnel(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct ipsec_tunnel *tunnel;

	/* Verify that the packet is big enough to contain an IP header */
	if (!pskb_may_pull(skb, sizeof(struct iphdr)))
		goto out;

	iph = skb->nh.iph;
	skb->mac.raw = skb->nh.raw;
	skb->nh.raw = skb->data;
	memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));
	skb->protocol = __constant_htons(ETH_P_IP);
	skb->pkt_type = PACKET_HOST;

	read_lock(&ipsec_lock);
	if ((tunnel = ipsec_tunnel_lookup(iph->saddr, iph->daddr)) != NULL) {
		s_stats.rx_ok++;
		tunnel->stat.rx_packets++;
		tunnel->stat.rx_bytes += skb->len;
		skb->dev = tunnel->dev;
		dst_release(skb->dst);
		skb->dst = NULL;
#ifdef CONFIG_NETFILTER
		nf_conntrack_put(skb->nfct);
		skb->nfct = NULL;
#ifdef CONFIG_NETFILTER_DEBUG
		skb->nf_debug = 0;
#endif
#endif
		netif_rx(skb);
		read_unlock(&ipsec_lock);
		return 0;
	}
	read_unlock(&ipsec_lock);

	s_stats.rx_no_tunnel++;

	icmp_send(skb, ICMP_DEST_UNREACH, ICMP_PROT_UNREACH, 0);

out:
	kfree_skb(skb);
	return 0;
}

int ipsec_esp_rcv(struct sk_buff *skb)
{
	u8 nextprot;

	if ((skb = ipsec_esp_decapsulate(skb, &nextprot)) == NULL)
		return 0;
		
	if (nextprot == IPPROTO_IPIP) /* Tunnel mode packet */
		return ipsec_esp_rcv_tunnel(skb);

	s_stats.rx_non_tunnel++;
	kfree_skb(skb);

	return 0;
}

/* Need this wrapper because NF_HOOK takes the function address */
static inline int do_ip_send(struct sk_buff *skb)
{
	return ip_send(skb);
}

/*
 *	This function assumes it is being called from dev_queue_xmit()
 *	and that skb is filled properly by that function.
 */

static int ipsec_tunnel_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ipsec_tunnel *tunnel = (struct ipsec_tunnel*)dev->priv;
	struct net_device_stats *stats = &tunnel->stat;
	struct iphdr  *tiph = &tunnel->parms.iph;
	u8     tos = tunnel->parms.iph.tos, old_iph_tos, old_iph_ttl;
	u16    df = tiph->frag_off;
	struct rtable *rt;     			/* Route to the other host */
	struct net_device *tdev;			/* Device to other host */
	struct iphdr  *old_iph = skb->nh.iph;
	struct iphdr  *iph;			/* Our new IP header */
	struct ipsec_sa *sa;
	u32    dst = tiph->daddr;
	int    mtu;

	if (tunnel->recursion++) {
		tunnel->stat.collisions++;
		s_stats.tx_recursion++;
		goto tx_error;
	}

	if (skb->protocol != __constant_htons(ETH_P_IP)) {
		s_stats.tx_other++;
		goto tx_error;
	}

	if (tos&1)
		tos = old_iph->tos;

	if (!dst) {
		/* NBMA tunnel */
		if ((rt = (struct rtable*)skb->dst) == NULL) {
			tunnel->stat.tx_fifo_errors++;
			s_stats.tx_other++;
			goto tx_error;
		}
		if ((dst = rt->rt_gateway) == 0) {
			s_stats.tx_other++;
			goto tx_error_icmp;
		}
	}

	if (ip_route_output(&rt, dst, tiph->saddr, RT_TOS(tos), tunnel->parms.link)) {
		tunnel->stat.tx_carrier_errors++;
		s_stats.tx_route++;
		goto tx_error_icmp;
	}
	tdev = rt->u.dst.dev;

	if (tdev == dev) {
		ip_rt_put(rt);
		tunnel->stat.collisions++;
		s_stats.tx_recursion++;
		goto tx_error;
	}

	mtu = rt->u.dst.pmtu -
		sizeof(struct iphdr) -
		sizeof(struct esphdr) -
		MAX_IV_SIZE - MAX_HMACLEN - 2;
	if (mtu < 68) {
		tunnel->stat.collisions++;
		ip_rt_put(rt);
		s_stats.tx_mtu++;
		goto tx_error;
	}
	if (skb->dst && mtu < skb->dst->pmtu)
		skb->dst->pmtu = mtu;

	df |= (old_iph->frag_off&__constant_htons(IP_DF));

	if ((old_iph->frag_off&__constant_htons(IP_DF)) && mtu < ntohs(old_iph->tot_len)) {
		icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, htonl(mtu));
		ip_rt_put(rt);
		s_stats.tx_mtu++;
		goto tx_error;
	}

	if (tunnel->err_count > 0) {
		if (jiffies - tunnel->err_time < IPTUNNEL_ERR_TIMEO) {
			tunnel->err_count--;
			dst_link_failure(skb);
		} else
			tunnel->err_count = 0;
	}

	skb->h.raw = skb->nh.raw;

	old_iph_ttl = old_iph->ttl;
	old_iph_tos = old_iph->tos;

	if ((sa = ipsec_sa_get(rt->rt_dst, rt->rt_src, tunnel->parms.spi)) == NULL) {
		ip_rt_put(rt);
		stats->tx_dropped++;
		dev_kfree_skb(skb);
		tunnel->recursion--;
		s_stats.tx_unknown_sa++;
		return 0;
	}

	skb = ipsec_esp_encapsulate(sa, skb);
	ipsec_sa_put(sa);
	if (!skb) {
		ip_rt_put(rt);
		stats->tx_dropped++;
		tunnel->recursion--;
		s_stats.tx_mem++;
		return 0;
	}

	skb->nh.raw = skb_push(skb, sizeof(struct iphdr));
	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));
	dst_release(skb->dst);
	skb->dst = &rt->u.dst;

	/*
	 *	Push down and install the IP header.
	 */

	iph 			=	skb->nh.iph;
	iph->version		=	4;
	iph->ihl		=	sizeof(struct iphdr)>>2;
	iph->frag_off		=	df;
	iph->protocol		=	IPPROTO_ESP;
	iph->tos		=	INET_ECN_encapsulate(tos, old_iph_tos);
	iph->daddr		=	rt->rt_dst;
	iph->saddr		=	rt->rt_src;

	if ((iph->ttl = tiph->ttl) == 0)
		iph->ttl	=	old_iph_ttl;

#ifdef CONFIG_NETFILTER
	nf_conntrack_put(skb->nfct);
	skb->nfct = NULL;
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif
#endif

	s_stats.tx_ok++;

	IPTUNNEL_XMIT();

	tunnel->recursion--;
	return 0;

tx_error_icmp:
	dst_link_failure(skb);
tx_error:
	stats->tx_errors++;
	dev_kfree_skb(skb);
	tunnel->recursion--;
	return 0;
}

static int
ipsec_sa_check_ioctl_version(struct ifreq *ifr)
{
	u32 version;

	if (get_user(version, ifr->ifr_ifru.ifru_data))
		return -EFAULT;

	if (version != IPSEC_SA_VERSION) {
		if (put_user(version, ifr->ifr_ifru.ifru_data))
			return -EFAULT;
		return -EINVAL;
	}

	return 0;
}

static int
ipsec_tunnel_ioctl (struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct ipsec_sa_parm sap;
	struct ipsec_tunnel_parm tp;
	struct ipsec_tunnel *t;
	struct ipsec_sa *sa;
	char cipher_key[IPSEC_SA_CRYPTOLEN];
	char digest_key[IPSEC_SA_CRYPTOLEN];
	int err = 0;

	MOD_INC_USE_COUNT;

	switch (cmd) {
	case SIOCIPSEC_GET_TUNNEL:
		t = NULL;
		if (dev == &ipsec_fb_tunnel_dev) {
			if (copy_from_user(&tp, ifr->ifr_ifru.ifru_data, sizeof(tp))) {
				err = -EFAULT;
				break;
			}
			t = ipsec_tunnel_locate(&tp);
		}
		if (t == NULL)
			t = (struct ipsec_tunnel*)dev->priv;
		memcpy(&tp, &t->parms, sizeof(tp));
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &tp, sizeof(tp)))
			err = -EFAULT;
		break;

	case SIOCIPSEC_ADD_TUNNEL:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto done;

		err = -EFAULT;
		if (copy_from_user(&tp, ifr->ifr_ifru.ifru_data, sizeof(tp)))
			goto done;

		err = -EINVAL;
		if (tp.iph.version != 4 || tp.iph.protocol != IPPROTO_ESP ||
		    tp.iph.ihl != 5 || (tp.iph.frag_off&__constant_htons(~IP_DF)))
			goto done;
		if (tp.iph.ttl)
			tp.iph.frag_off |= __constant_htons(IP_DF);

		err = -EEXIST;
		if (ipsec_tunnel_locate(&tp))
			goto done;

		err = -ENOBUFS;
		if ((t = ipsec_tunnel_create(&tp)) == NULL)
			goto done;

		err = 0;
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &t->parms, sizeof(tp)))
			err = -EFAULT;
		break;

	case SIOCIPSEC_CHG_TUNNEL:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto done;

		err = -EPERM;
		if (dev == &ipsec_fb_tunnel_dev)
			goto done;

		err = -EFAULT;
		if (copy_from_user(&tp, ifr->ifr_ifru.ifru_data, sizeof(tp)))
			goto done;

		err = -EINVAL;
		if (tp.iph.version != 4 || tp.iph.protocol != IPPROTO_ESP ||
		    tp.iph.ihl != 5 || (tp.iph.frag_off&__constant_htons(~IP_DF)))
			goto done;
		if (tp.iph.ttl)
			tp.iph.frag_off |= __constant_htons(IP_DF);

		err = -EEXIST;
		t = ipsec_tunnel_locate(&tp);
		if (t != NULL && t->dev != dev)
			goto done;

		err = -EINVAL;
		if (((dev->flags&IFF_POINTOPOINT) && !tp.iph.daddr) ||
			(!(dev->flags&IFF_POINTOPOINT) && tp.iph.daddr))
			goto done;

		t = (struct ipsec_tunnel*)dev->priv;
		ipsec_tunnel_unlink(t);
		t->parms.link = tp.link;
		t->parms.iph.saddr = tp.iph.saddr;
		t->parms.iph.daddr = tp.iph.daddr;
		t->parms.spi = tp.spi;
		memcpy(dev->dev_addr, &tp.iph.saddr, 4);
		memcpy(dev->broadcast, &tp.iph.daddr, 4);
		ipsec_tunnel_link(t);
		netdev_state_change(dev);

		t->parms.iph.ttl = tp.iph.ttl;
		t->parms.iph.tos = tp.iph.tos;
		t->parms.iph.frag_off = tp.iph.frag_off;

		err = 0;
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &t->parms, sizeof(tp)))
			err = -EFAULT;
		break;

	case SIOCIPSEC_DEL_TUNNEL:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto done;

		if (dev == &ipsec_fb_tunnel_dev) {
			err = -EFAULT;
			if (copy_from_user(&tp, ifr->ifr_ifru.ifru_data, sizeof(tp)))
				goto done;
			err = -ENOENT;
			if ((t = ipsec_tunnel_locate(&tp)) == NULL)
				goto done;
			err = -EPERM;
			if (t == &ipsec_fb_tunnel)
				goto done;
			dev = t->dev;
		}
		err = unregister_netdevice(dev);
		break;

	case SIOCIPSEC_GET_SA:
		if ((err = ipsec_sa_check_ioctl_version(ifr)) != 0)
			goto done;

		err = -EFAULT;
		if (copy_from_user(&sap, ifr->ifr_ifru.ifru_data, sizeof(sap)))
			goto done;

		err = -EINVAL;
		if (sap.version != IPSEC_SA_VERSION)
			goto done;

		err = -ENOENT;
		if (sap.dst == INADDR_ANY && sap.src == INADDR_ANY)
			sa = ipsec_sa_get_num(sap.spi);
		else
			sa = ipsec_sa_get(sap.dst, sap.src, sap.spi);
		if (!sa)
			goto done;

		memset(&sap, 0, sizeof(sap));
		sap.dst = sa->dst;
		sap.src = sa->src;
		sap.spi = sa->spi;
		sap.flags = sa->flags;
		if (sa->cipher) {
			strcpy(sap.cipher, sa->cipher->ci->trans.t_name);
			sap.cipher_keylen = sa->cipher->key_length;
		}
		if (sa->digest) {
			strcpy(sap.digest, sa->digest->di->trans.t_name);
			sap.digest_keylen = sa->digest_keylen;
			sap.digest_hmaclen = sa->digest_hmaclen;
		}

		ipsec_sa_put(sa);

		err = -EFAULT;
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &sap, sizeof(sap)))
			goto done;

		err = 0;
		break;

	case SIOCIPSEC_ADD_SA:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto done;

		if ((err = ipsec_sa_check_ioctl_version(ifr)) != 0)
			goto done;

		err = -EFAULT;
		if (copy_from_user(&sap, ifr->ifr_ifru.ifru_data, sizeof(sap)))
			goto done;

		err = -EINVAL;
		if (sap.version != IPSEC_SA_VERSION || sap.dst == INADDR_ANY ||
			sap.src == INADDR_ANY || sap.spi == IPSEC_SPI_ANY)
			goto done;

		err = -EEXIST;
		if ((sa = ipsec_sa_get(sap.dst, sap.src, sap.spi)) != NULL) {
			ipsec_sa_put(sa);
			goto done;
		}

		if (sap.cipher[0] != '\0') {
			err = -EINVAL;
			if (sap.cipher_keylen > sizeof(cipher_key))
				goto done;

			err = -EFAULT;
			if (copy_from_user(cipher_key, sap.cipher_key, sap.cipher_keylen))
				goto done;
		}

		if (sap.digest[0] != '\0') {
			err = -EINVAL;
			if (sap.digest_keylen > sizeof(digest_key))
				goto done;

			err = -EFAULT;
			if (copy_from_user(digest_key, sap.digest_key, sap.digest_keylen))
				goto done;
		}

		/* Make sure the cipher and digest names are terminated. */
		sap.cipher[sizeof(sap.cipher) - 1] = '\0';
		sap.digest[sizeof(sap.digest) - 1] = '\0';

		err = ipsec_sa_add(sap.dst,
				   sap.src,
				   sap.spi,
				   sap.flags,
				   sap.cipher,
				   cipher_key,
				   sap.cipher_keylen,
				   sap.digest,
				   digest_key,
				   sap.digest_keylen,
				   sap.digest_hmaclen);
		break;

	case SIOCIPSEC_DEL_SA:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto done;

		if ((err = ipsec_sa_check_ioctl_version(ifr)) != 0)
			goto done;

		err = -EFAULT;
		if (copy_from_user(&sap, ifr->ifr_ifru.ifru_data, sizeof(sap)))
			goto done;

		err = -ENOENT;
		if (ipsec_sa_del(sap.dst, sap.src, sap.spi) == 0)
			goto done;

		err = 0;
		break;

	case SIOCIPSEC_GET_STATS:
		err = -EFAULT;
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &s_stats, sizeof(s_stats)))
			goto done;

		err = 0;
		break;

	default:
		err = -EINVAL;
	}

done:
	MOD_DEC_USE_COUNT;
	return err;
}

static struct net_device_stats *ipsec_tunnel_get_stats(struct net_device *dev)
{
	return &(((struct ipsec_tunnel*)dev->priv)->stat);
}

static int ipsec_tunnel_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 || new_mtu > 0xFFF8 - sizeof(struct iphdr))
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

static void ipsec_tunnel_init_gen(struct net_device *dev)
{
	struct ipsec_tunnel *t = (struct ipsec_tunnel*)dev->priv;

	dev->uninit		= ipsec_tunnel_uninit;
	dev->destructor		= ipsec_tunnel_destructor;
	dev->hard_start_xmit	= ipsec_tunnel_xmit;
	dev->get_stats		= ipsec_tunnel_get_stats;
	dev->do_ioctl		= ipsec_tunnel_ioctl;
	dev->change_mtu		= ipsec_tunnel_change_mtu;

	dev->type		= ARPHRD_IPSEC;
	dev->hard_header_len 	= LL_MAX_HEADER + sizeof(struct iphdr);
	dev->mtu		= 1500 - sizeof(struct iphdr);
	dev->flags		= IFF_NOARP;
	dev->iflink		= 0;
	dev->addr_len		= 4;
	memcpy(dev->dev_addr, &t->parms.iph.saddr, 4);
	memcpy(dev->broadcast, &t->parms.iph.daddr, 4);
}

static int ipsec_tunnel_init(struct net_device *dev)
{
	struct net_device *tdev = NULL;
	struct ipsec_tunnel *tunnel;
	struct iphdr *iph;

	tunnel = (struct ipsec_tunnel*)dev->priv;
	iph = &tunnel->parms.iph;

	ipsec_tunnel_init_gen(dev);

	if (iph->daddr) {
		struct rtable *rt;
		if (!ip_route_output(&rt, iph->daddr, iph->saddr, RT_TOS(iph->tos), tunnel->parms.link)) {
			tdev = rt->u.dst.dev;
			ip_rt_put(rt);
		}
		dev->flags |= IFF_POINTOPOINT;
	}

	if (!tdev && tunnel->parms.link)
		tdev = __dev_get_by_index(tunnel->parms.link);

	dev->hard_header_len = (tdev ? tdev->hard_header_len : LL_MAX_HEADER) +
		sizeof(struct iphdr) +
		sizeof(struct esphdr) +
		MAX_IV_SIZE;

	dev->mtu = (tdev ? tdev->mtu : 1500) -
		sizeof(struct iphdr) -
		sizeof(struct esphdr) -
		MAX_IV_SIZE - MAX_HMACLEN - 2;

	dev->iflink = tunnel->parms.link;

	return 0;
}

#ifdef MODULE
static int ipsec_fb_tunnel_open(struct net_device *dev)
{
	MOD_INC_USE_COUNT;
	return 0;
}

static int ipsec_fb_tunnel_close(struct net_device *dev)
{
	MOD_DEC_USE_COUNT;
	return 0;
}
#endif

int __init ipsec_fb_tunnel_init(struct net_device *dev)
{
	struct iphdr *iph;

	ipsec_tunnel_init_gen(dev);
#ifdef MODULE
	dev->open		= ipsec_fb_tunnel_open;
	dev->stop		= ipsec_fb_tunnel_close;
#endif

	iph = &ipsec_fb_tunnel.parms.iph;
	iph->version		= 4;
	iph->protocol		= IPPROTO_ESP;
	iph->ihl		= 5;

	dev_hold(dev);
	tunnels_wc[0]		= &ipsec_fb_tunnel;
	return 0;
}

static struct inet_protocol ipsec_esp_protocol = {
	handler:	ipsec_esp_rcv,
	err_handler:	ipsec_esp_err,
	protocol:	IPPROTO_ESP,
	name:		"IPSEC_ESP"
};

static char banner[] __initdata =
	KERN_INFO "IPsec over IPv4 tunneling driver\n";

int __init ipsec_tunnel_module_init(void)
{
	int err;

	printk(banner);

	ipsec_sa_init();
	ipsec_fb_tunnel_dev.priv = (void*)&ipsec_fb_tunnel;
	err = register_netdev(&ipsec_fb_tunnel_dev);
	if (err)
		return err;
	inet_add_protocol(&ipsec_esp_protocol);

	return 0;
}

static void __exit ipsec_tunnel_module_fini(void)
{
	if ( inet_del_protocol(&ipsec_esp_protocol) < 0 )
		printk(KERN_INFO "ipsec close: can't remove protocol\n");

	unregister_netdev(&ipsec_fb_tunnel_dev);
	ipsec_sa_destroy();
}

module_init(ipsec_tunnel_module_init);
module_exit(ipsec_tunnel_module_fini);

MODULE_AUTHOR("Tobias Ringstrom <tobias@ringstrom.mine.nu>");
MODULE_DESCRIPTION("IPsec over IPv4 tunneling driver");
MODULE_LICENSE("GPL");
