/*
 *	Linux NET3:	Ethernet over IP protocol decoder. 
 *
 *	Authors: Alexey Kuznetsov (kuznet@ms2.inr.ac.ru)
 *	         Sebastian Gottschall (s.gottschall@dd-wrt.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 */

/*
   This version of net/ipv4/etherip.c created by Lennert Buytenhek
   by mashing net/ipv4/ip_gre.c and net/ipv4/ipip.c together.

   For comments look at net/ipv4/ip_gre.c
 */
 
#include <linux/version.h>
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
#include <linux/in6.h>
#include <linux/inetdevice.h>
#include <linux/igmp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/etherdevice.h>

#include <net/sock.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/protocol.h>
#include <net/arp.h>
#include <net/checksum.h>
#include <net/dsfield.h>
#include <net/inet_ecn.h>
#include <net/xfrm.h>
#include <net/ip_tunnels.h>


#if IS_ENABLED(CONFIG_IPV6)
#include <net/ipv6.h>
#include <net/ip6_fib.h>
#include <net/ip6_route.h>
#endif

struct etheriphdr
{
	struct iphdr	iph;
	u16		version;
} __attribute__((packed));


#define ETHERIP_HASH_SIZE  16
#define ETHERIP_HASH(addr) ((addr^(addr>>4))&0xF)

#define ETHERIP_VERSION		0x0300

static int etherip_fb_tunnel_init(struct net_device *dev);
static int etherip_tunnel_init(struct net_device *dev);
static void etherip_tunnel_setup(struct net_device *dev);
static void etherip_tunnel_setup_fb(struct net_device *dev);

static struct net_device *etherip_fb_tunnel_dev;

static struct ip_tunnel *tunnels_r_l[ETHERIP_HASH_SIZE];
static struct ip_tunnel *tunnels_r[ETHERIP_HASH_SIZE];
static struct ip_tunnel *tunnels_l[ETHERIP_HASH_SIZE];
static struct ip_tunnel *tunnels_wc[1];

static struct ip_tunnel **tunnels[4] = { tunnels_wc, tunnels_l, tunnels_r, tunnels_r_l};

static DEFINE_RWLOCK(etherip_lock);

static struct ip_tunnel * etherip_tunnel_lookup(u32 remote, u32 local)
{
	unsigned h0 = ETHERIP_HASH(remote);
	unsigned h1 = ETHERIP_HASH(local);
	struct ip_tunnel *t;

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

static struct ip_tunnel **etherip_bucket(struct ip_tunnel *t)
{
	u32 remote = t->parms.iph.daddr;
	u32 local = t->parms.iph.saddr;
	unsigned h = 0;
	int prio = 0;

	if (remote) {
		prio |= 2;
		h ^= ETHERIP_HASH(remote);
	}
	if (local) {
		prio |= 1;
		h ^= ETHERIP_HASH(local);
	}
	return &tunnels[prio][h];
}

static void etherip_tunnel_unlink(struct ip_tunnel *t)
{
	struct ip_tunnel **tp;

	for (tp = etherip_bucket(t); *tp; tp = &(*tp)->next) {
		if (t == *tp) {
			write_lock_bh(&etherip_lock);
			*tp = t->next;
			write_unlock_bh(&etherip_lock);
			break;
		}
	}
}

static void etherip_tunnel_link(struct ip_tunnel *t)
{
	struct ip_tunnel **tp = etherip_bucket(t);

	t->next = *tp;
	write_lock_bh(&etherip_lock);
	*tp = t;
	write_unlock_bh(&etherip_lock);
}

static struct ip_tunnel * etherip_tunnel_locate(struct net_device *ndev, struct ip_tunnel_parm *parms, int create)
{
	u32 remote = parms->iph.daddr;
	u32 local = parms->iph.saddr;
	struct ip_tunnel *t, **tp, *nt;
	struct net *net = dev_net(ndev);
	struct net_device *dev;
	unsigned h = 0;
	int prio = 0;
	char name[IFNAMSIZ];

	if (remote) {
		prio |= 2;
		h ^= ETHERIP_HASH(remote);
	}
	if (local) {
		prio |= 1;
		h ^= ETHERIP_HASH(local);
	}
	for (tp = &tunnels[prio][h]; (t = *tp) != NULL; tp = &t->next) {
		if (local == t->parms.iph.saddr && remote == t->parms.iph.daddr)
			return t;
	}
	if (!create)
		return NULL;

	if (parms->name[0])
		strlcpy(name, parms->name, IFNAMSIZ);
	else {
		int i;
		for (i=1; i<100; i++) {
			sprintf(name, "etherip%d", i);
			if (__dev_get_by_name(net, name) == NULL)
				break;
		}
		if (i==100)
			goto failed;
	}

	dev = alloc_netdev(sizeof(*t), name, NET_NAME_UNKNOWN, etherip_tunnel_setup);
	if (!dev)
		return NULL;

	nt = netdev_priv(dev);
	nt->parms = *parms;

	if (register_netdevice(dev) < 0) {
		free_netdev(dev);
		goto failed;
	}

	dev_hold(dev);
	etherip_tunnel_link(nt);
	/* Do not decrement MOD_USE_COUNT here. */
	return nt;

failed:
	return NULL;
}

static void etherip_tunnel_uninit(struct net_device *dev)
{
	if (dev == etherip_fb_tunnel_dev) {
		write_lock_bh(&etherip_lock);
		tunnels_wc[0] = NULL;
		write_unlock_bh(&etherip_lock);
	} else
		etherip_tunnel_unlink((struct ip_tunnel*)netdev_priv(dev));
	dev_put(dev);
}


void etherip_err(struct sk_buff *skb, u32 info)
{
#ifndef I_WISH_WORLD_WERE_PERFECT
/* It is not :-( All the routers (except for Linux) return only
   8 bytes of packet payload. It means, that precise relaying of
   ICMP in the real Internet is absolutely infeasible.
 */

	struct iphdr *iph = (struct iphdr*)skb->data;
	const int type = icmp_hdr(skb)->type;
	const int code = icmp_hdr(skb)->code;
	struct ip_tunnel *t;

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

	read_lock(&etherip_lock);
	t = etherip_tunnel_lookup(iph->daddr, iph->saddr);
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
	read_unlock(&etherip_lock);
	return;
#endif
}

static inline void etherip_ecn_decapsulate(struct iphdr *iph, struct sk_buff*skb)
{
	if (INET_ECN_is_ce(iph->tos)) {
		if (skb->protocol == htons(ETH_P_IP)) {
			IP_ECN_set_ce(ipip_hdr(skb));
		} else if (skb->protocol == htons(ETH_P_IPV6)) {
			IP6_ECN_set_ce(skb, ipipv6_hdr(skb));
		}
	}
}

static inline u8
etherip_ecn_encapsulate(u8 tos, struct iphdr *inner_iph, struct sk_buff *skb)
{
	u8 inner = 0;
	if (skb->protocol == htons(ETH_P_IP))
		inner = inner_iph->tos;
	else if (skb->protocol == htons(ETH_P_IPV6))
		inner = ipv6_get_dsfield((struct ipv6hdr *)inner_iph);
	return INET_ECN_encapsulate(tos, inner);
}

int etherip_rcv(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct ip_tunnel *tunnel;
	struct etheriphdr *ethiph;

	if (!pskb_may_pull(skb, sizeof(struct etheriphdr)))
		goto out;

	ethiph = (struct etheriphdr *)skb_network_header(skb);
	if (ethiph->version != htons(ETHERIP_VERSION)) {
		kfree_skb(skb);
		return 0;
	}

	iph = ip_hdr(skb);

	read_lock(&etherip_lock);
	if ((tunnel = etherip_tunnel_lookup(iph->saddr, iph->daddr)) != NULL) {
		struct pcpu_sw_netstats *tstats;
		secpath_reset(skb);

		/* Pull etherip header.  */
		skb_pull(skb, 2);
		skb->protocol = eth_type_trans(skb, tunnel->dev);

		memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));

		tstats = this_cpu_ptr(tunnel->dev->tstats);
		tstats->rx_packets++;
		tstats->rx_bytes += skb->len;

		skb->dev = tunnel->dev;
		skb_dst_drop(skb);
		nf_reset(skb);
		etherip_ecn_decapsulate(iph, skb);
		netif_rx(skb);
		read_unlock(&etherip_lock);
		return 0;
	}
	read_unlock(&etherip_lock);

out:
	return -1;
}

static int etherip_tunnel_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ip_tunnel *tunnel = (struct ip_tunnel*)netdev_priv(dev);
	struct pcpu_sw_netstats *tstats;
	struct iphdr *inner_iph = ip_hdr(skb);
	struct iphdr *tiph = &tunnel->parms.iph;
	u8     tos;
	u16    df;
	struct rtable *rt;     		/* Route to the other host */
	struct net_device *tdev;	/* Device to other host */
	struct etheriphdr  *ethiph;
	struct iphdr  *iph;		/* Our new IP header */
	int    max_headroom;		/* The extra header space needed */
	int    mtu;
	struct flowi4 fl;

	/* Need valid non-multicast daddr.  */
	if (tiph->daddr == 0 || ipv4_is_multicast(tiph->daddr))
		goto tx_error;

	tos = tiph->tos;
	if (tos&1) {
		if (skb->protocol == htons(ETH_P_IP))
			tos = inner_iph->tos;
		tos &= ~1;
	}

	memset(&fl, 0, sizeof(fl));
	fl.flowi4_oif = tunnel->parms.link,
	fl.daddr = tiph->daddr,
	fl.saddr = tiph->saddr,
	fl.flowi4_tos = RT_TOS(tos),
	fl.flowi4_proto = IPPROTO_ETHERIP;
	rt = ip_route_output_key(dev_net(dev), &fl);
	if (IS_ERR(rt)) {
		dev->stats.tx_carrier_errors++;
		goto tx_error_icmp;
	}
	tdev = rt->dst.dev;

	if (tdev == dev) {
		ip_rt_put(rt);
		dev->stats.collisions++;
		goto tx_error;
	}

	df = tiph->frag_off;
	if (df)
		mtu = dst_mtu(&rt->dst) - sizeof(struct etheriphdr);
	else
		mtu = skb_dst(skb) ? dst_mtu(skb_dst(skb)) : dev->mtu;

	if (skb_dst(skb))
		skb_dst(skb)->ops->update_pmtu(skb_dst(skb), NULL, skb, mtu);



	if (skb->protocol == htons(ETH_P_IP)) {
		df |= (inner_iph->frag_off&htons(IP_DF));

		if ((inner_iph->frag_off & htons(IP_DF)) &&
		    mtu < ntohs(inner_iph->tot_len)) {
			icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, htonl(mtu));
			ip_rt_put(rt);
			goto tx_error;
		}
	}

#if IS_ENABLED(CONFIG_IPV6)
	else if (skb->protocol == htons(ETH_P_IPV6)) {
		struct rt6_info *rt6 = (struct rt6_info*)skb_dst(skb);

		if (rt6 && mtu < dst_mtu(skb_dst(skb)) && mtu >= IPV6_MIN_MTU) {
			if (tiph->daddr || rt6->rt6i_dst.plen == 128) {
				rt6->rt6i_flags |= RTF_MODIFIED;
				dst_metric_set(skb_dst(skb), RTAX_MTU, mtu);
			}
		}

		/* @@@ Is this correct?  */
		if (mtu >= IPV6_MIN_MTU && mtu < skb->len - 2 * sizeof(struct etheriphdr)){
			icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
			ip_rt_put(rt);
			goto tx_error;
		}
	}
#endif

	if (tunnel->err_count > 0) {
		if (jiffies - tunnel->err_time < IPTUNNEL_ERR_TIMEO) {
			tunnel->err_count--;
			dst_link_failure(skb);
		} else
			tunnel->err_count = 0;
	}

	max_headroom = LL_RESERVED_SPACE(tdev) + sizeof(struct etheriphdr);

	if (skb_headroom(skb) < max_headroom || skb_cloned(skb) || skb_shared(skb)){
		struct sk_buff *new_skb = skb_realloc_headroom(skb, max_headroom);
		if (!new_skb) {
			ip_rt_put(rt);
  			dev->stats.tx_dropped++;
			dev_kfree_skb(skb);
			return 0;
		}
		if (skb->sk)
			skb_set_owner_w(new_skb, skb->sk);
		dev_kfree_skb(skb);
		skb = new_skb;
		inner_iph = ip_hdr(skb);
	}


	skb->transport_header = skb->mac_header;
	skb_push(skb, sizeof(struct etheriphdr));
	skb_reset_network_header(skb);
	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));
	skb_dst_drop(skb);
	skb_dst_set(skb, &rt->dst);

	/*
	 *	Push down and install the etherip header.
	 */

	ethiph 			=	(struct etheriphdr *)ip_hdr(skb);

	iph			=	&ethiph->iph;
	iph->version		=	4;
	iph->ihl		=	sizeof(struct iphdr) >> 2;
	iph->frag_off		=	df;
	iph->protocol		=	IPPROTO_ETHERIP;
	iph->tos		=	etherip_ecn_encapsulate(tos, inner_iph, skb);
	iph->daddr = fl.daddr;
	iph->saddr = fl.saddr;

	ethiph->version		=	htons(ETHERIP_VERSION);

	if ((ethiph->iph.ttl = tiph->ttl) == 0) {
		if (skb->protocol == htons(ETH_P_IP))
			ethiph->iph.ttl = inner_iph->ttl;
#if IS_ENABLED(CONFIG_IPV6)
		else if (skb->protocol == htons(ETH_P_IPV6))
			ethiph->iph.ttl = ((struct ipv6hdr*)inner_iph)->hop_limit;
#endif
		else
			ethiph->iph.ttl = dst_metric(&rt->dst, RTAX_HOPLIMIT);
	}

	nf_reset(skb);

	tstats = this_cpu_ptr(dev->tstats);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
	__IPTUNNEL_XMIT_COMPAT(tstats, &dev->stats);
#else
	__IPTUNNEL_XMIT_COMPAT(dev_net(dev), skb->sk, tstats, &dev->stats);
#endif
	tunnel->dev->trans_start = jiffies;
	return 0;

tx_error_icmp:
	dst_link_failure(skb);

tx_error:
	dev->stats.tx_errors++;
	dev_kfree_skb(skb);
	return 0;
}

static int
etherip_tunnel_ioctl (struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int err = 0;
	struct ip_tunnel_parm p;
	struct ip_tunnel *t;

	switch (cmd) {
	case SIOCGETTUNNEL:
		t = NULL;
		if (dev == etherip_fb_tunnel_dev) {
			if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p))) {
				err = -EFAULT;
				break;
			}
			t = etherip_tunnel_locate(dev, &p, 0);
		}
		if (t == NULL)
			t = (struct ip_tunnel*)netdev_priv(dev);
		memcpy(&p, &t->parms, sizeof(p));
		if (copy_to_user(ifr->ifr_ifru.ifru_data, &p, sizeof(p)))
			err = -EFAULT;
		break;

	case SIOCADDTUNNEL:
	case SIOCCHGTUNNEL:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto done;

		err = -EFAULT;
		if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p)))
			goto done;

		err = -EINVAL;
		if (p.iph.version != 4 || p.iph.protocol != IPPROTO_ETHERIP ||
		    p.iph.ihl != 5 || (p.iph.frag_off&htons(~IP_DF)))
			goto done;
		if (p.iph.ttl)
			p.iph.frag_off |= htons(IP_DF);

		t = etherip_tunnel_locate(dev, &p, cmd == SIOCADDTUNNEL);

		if (dev != etherip_fb_tunnel_dev && cmd == SIOCCHGTUNNEL) {
			if (t != NULL) {
				if (t->dev != dev) {
					err = -EEXIST;
					break;
				}
			} else {
				if (!p.iph.daddr) {
					err = -EINVAL;
					break;
				}

				t = (struct ip_tunnel*)netdev_priv(dev);
				etherip_tunnel_unlink(t);
				t->parms.iph.saddr = p.iph.saddr;
				t->parms.iph.daddr = p.iph.daddr;
				etherip_tunnel_link(t);
				netdev_state_change(dev);
			}
		}

		if (t) {
			err = 0;
			if (cmd == SIOCCHGTUNNEL) {
				t->parms.iph.ttl = p.iph.ttl;
				t->parms.iph.tos = p.iph.tos;
				t->parms.iph.frag_off = p.iph.frag_off;
			}
			if (copy_to_user(ifr->ifr_ifru.ifru_data, &t->parms, sizeof(p)))
				err = -EFAULT;
		} else
			err = (cmd == SIOCADDTUNNEL ? -ENOBUFS : -ENOENT);
		break;

	case SIOCDELTUNNEL:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto done;

		if (dev == etherip_fb_tunnel_dev) {
			err = -EFAULT;
			if (copy_from_user(&p, ifr->ifr_ifru.ifru_data, sizeof(p)))
				goto done;
			err = -ENOENT;
			if ((t = etherip_tunnel_locate(dev, &p, 0)) == NULL)
				goto done;
			err = -EPERM;
			if (t->dev == etherip_fb_tunnel_dev)
				goto done;
			dev = t->dev;
		}
		unregister_netdevice(dev);
		break;

	default:
		err = -EINVAL;
	}

done:
	return err;
}


struct rtnl_link_stats64 *etherip_tunnel_get_stats64(struct net_device *dev,
						struct rtnl_link_stats64 *tot)
{
	int i;

	netdev_stats_to_stats64(tot, &dev->stats);

	for_each_possible_cpu(i) {
		const struct pcpu_sw_netstats *tstats =
						   per_cpu_ptr(dev->tstats, i);
		u64 rx_packets, rx_bytes, tx_packets, tx_bytes;
		unsigned int start;

		do {
			start = u64_stats_fetch_begin_irq(&tstats->syncp);
			rx_packets = tstats->rx_packets;
			tx_packets = tstats->tx_packets;
			rx_bytes = tstats->rx_bytes;
			tx_bytes = tstats->tx_bytes;
		} while (u64_stats_fetch_retry_irq(&tstats->syncp, start));

		tot->rx_packets += rx_packets;
		tot->tx_packets += tx_packets;
		tot->rx_bytes   += rx_bytes;
		tot->tx_bytes   += tx_bytes;
	}

	return tot;
}

static int etherip_tunnel_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 || new_mtu > 0xFFF8 - sizeof(struct etheriphdr))
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
int etherip_get_iflink(const struct net_device *dev)
{
	struct ip_tunnel *tunnel = netdev_priv(dev);

	return tunnel->parms.link;
}
#endif
static int etherip_tunnel_set_mac_address(struct net_device *dev, void *p) {
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	return 0;
}


static const struct net_device_ops etherip_netdev_ops_fb = {
	.ndo_init       = etherip_fb_tunnel_init,
	.ndo_uninit     = etherip_tunnel_uninit,
	.ndo_start_xmit	= etherip_tunnel_xmit,
	.ndo_do_ioctl	= etherip_tunnel_ioctl,
	.ndo_change_mtu = etherip_tunnel_change_mtu,
	.ndo_get_stats64 = etherip_tunnel_get_stats64,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
	.ndo_get_iflink = etherip_get_iflink,
#endif
};

static const struct net_device_ops etherip_netdev_ops = {
	.ndo_init       = etherip_tunnel_init,
	.ndo_uninit     = etherip_tunnel_uninit,
	.ndo_start_xmit	= etherip_tunnel_xmit,
	.ndo_do_ioctl	= etherip_tunnel_ioctl,
	.ndo_change_mtu = etherip_tunnel_change_mtu,
	.ndo_get_stats64 = etherip_tunnel_get_stats64,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
	.ndo_get_iflink = etherip_get_iflink,
#endif
	.ndo_set_mac_address = etherip_tunnel_set_mac_address,
};

static void etherip_dev_free(struct net_device *dev)
{
	free_percpu(dev->tstats);
	free_netdev(dev);
}

static void etherip_tunnel_setup_fb(struct net_device *dev)
{
	ether_setup(dev);
	
	dev->netdev_ops		= &etherip_netdev_ops_fb;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	dev->destructor		= etherip_dev_free;
#else
	dev->priv_destructor 	= etherip_dev_free;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
	dev->iflink = 0;
#endif

	dev->hard_header_len	= ETH_HLEN;	//  + sizeof(struct etheriphdr);
	dev->tx_queue_len	= 0;
	random_ether_addr(dev->dev_addr);
}


static void etherip_tunnel_setup(struct net_device *dev)
{

	etherip_tunnel_setup_fb(dev);
	dev->netdev_ops		= &etherip_netdev_ops;
}



static int etherip_tunnel_init(struct net_device *dev)
{
	struct net_device *tdev = NULL;
	struct ip_tunnel *tunnel;
	struct iphdr *iph;
	struct rtable *rt;

	tunnel = (struct ip_tunnel*)netdev_priv(dev);
	iph = &tunnel->parms.iph;

	tunnel->dev = dev;
	strcpy(tunnel->parms.name, dev->name);

	/* Guess output device to choose reasonable mtu and hard_header_len */
	if (iph->daddr) {
		struct flowi4 fl;
		fl.flowi4_oif = tunnel->parms.link,
		fl.daddr = iph->daddr,
		fl.saddr = iph->saddr,
		fl.flowi4_tos = RT_TOS(iph->tos),
		fl.flowi4_proto = IPPROTO_ETHERIP;
		if ((rt = ip_route_output_key(dev_net(dev), &fl))) {
			tdev = rt->dst.dev;
			ip_rt_put(rt);
		}
	}
	dev->tstats = alloc_percpu(struct pcpu_sw_netstats);

	if (!tdev && tunnel->parms.link)
		tdev = __dev_get_by_index(dev_net(dev), tunnel->parms.link);

	if (tdev) {
		dev->hard_header_len = tdev->hard_header_len + sizeof(struct etheriphdr);
		dev->mtu = tdev->mtu - sizeof(struct etheriphdr);
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
	dev->iflink = tunnel->parms.link;
#endif

	return 0;
}

int __init etherip_fb_tunnel_init(struct net_device *dev)
{
	struct ip_tunnel *tunnel = (struct ip_tunnel*)netdev_priv(dev);
	struct iphdr *iph = &tunnel->parms.iph;

	tunnel->dev = dev;
	dev->tstats = alloc_percpu(struct pcpu_sw_netstats);
	strcpy(tunnel->parms.name, dev->name);

	iph->version		= 4;
	iph->protocol		= IPPROTO_ETHERIP;
	iph->ihl		= 5;

	dev_hold(dev);
	tunnels_wc[0]		= tunnel;
	return 0;
}


static struct net_protocol etherip_protocol = {
	.handler	=	etherip_rcv,
	.err_handler	=	etherip_err,
	.no_policy    = 1,
	.netns_ok     = 1,
};


/*
 *	And now the modules code and kernel interface.
 */

static int __init etherip_init(void)
{
	int err;
	printk(KERN_INFO "Ethernet over IPv4 tunneling driver\n");

	if (inet_add_protocol(&etherip_protocol, IPPROTO_ETHERIP) < 0) {
		printk(KERN_INFO "etherip init: can't add protocol\n");
		return -EAGAIN;
	}

	etherip_fb_tunnel_dev = alloc_netdev(sizeof(struct ip_tunnel),
					     "etherip0", NET_NAME_UNKNOWN, etherip_tunnel_setup_fb);
	if (!etherip_fb_tunnel_dev) {
		err = -ENOMEM;
		goto err1;
	}


	if ((err = register_netdev(etherip_fb_tunnel_dev)))
		goto err2;
out:
	return err;
err2:
	free_netdev(etherip_fb_tunnel_dev);
err1:
	inet_del_protocol(&etherip_protocol, IPPROTO_ETHERIP);
	goto out;
}

void etherip_fini(void)
{
	if (inet_del_protocol(&etherip_protocol, IPPROTO_ETHERIP) < 0)
		printk(KERN_INFO "etherip close: can't remove protocol\n");

	unregister_netdev(etherip_fb_tunnel_dev);
}

module_init(etherip_init);
module_exit(etherip_fini);
MODULE_LICENSE("GPL");
