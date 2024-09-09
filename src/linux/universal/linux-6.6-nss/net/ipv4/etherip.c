/*
 * etherip.c: Ethernet over IPv4 tunnel driver (according to RFC3378)
 *
 * This driver could be used to tunnel Ethernet packets through IPv4
 * networks. This is especially usefull together with the bridging
 * code in Linux.
 *
 * This code was written with an eye on the IPIP driver in linux from
 * Sam Lantinga. Thanks for the great work.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      version 2 (no later version) as published by the
 *      Free Software Foundation.
 *
 */

#include <linux/capability.h>                           
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/if_tunnel.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/netfilter_ipv4.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/ip_tunnels.h>
#include <net/xfrm.h>
#include <net/inet_ecn.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joerg Roedel <joro@zlug.org>");
MODULE_DESCRIPTION("Ethernet over IPv4 tunnel driver");

#ifndef IPPROTO_ETHERIP
#define IPPROTO_ETHERIP 97
#endif

/*
 * These 2 defines are taken from ipip.c - if it's good enough for them
 * it's good enough for me.
 */
#define E_HASH_SIZE        16
#define HASH(addr)       ((addr^(addr>>4))&0xF)

#define ETHERIP_HEADER   ((u16)0x0300)
#define ETHERIP_HLEN     2

#define BANNER1 "etherip: Ethernet over IPv4 tunneling driver\n"

struct etherip_tunnel {
	struct list_head list;
	struct net_device *dev;
	struct ip_tunnel_parm parms;
	unsigned int recursion;
};


/*struct pcpu_tstats {
	unsigned long	rx_packets;
	unsigned long	rx_bytes;
	unsigned long	tx_packets;
	unsigned long	tx_bytes;
};*/

static void etherip_dev_free(struct net_device *dev)
{
//	free_percpu(dev->tstats);
	free_netdev(dev);
}
/*
static struct net_device_stats *etherip_get_stats(struct net_device *dev)
{
	struct pcpu_tstats sum = { 0 };
	int i;

	for_each_possible_cpu(i) {
		const struct pcpu_tstats *tstats = per_cpu_ptr(dev->tstats, i);

		sum.rx_packets += tstats->rx_packets;
		sum.rx_bytes   += tstats->rx_bytes;
		sum.tx_packets += tstats->tx_packets;
		sum.tx_bytes   += tstats->tx_bytes;
	}
	dev->stats.rx_packets = sum.rx_packets;
	dev->stats.rx_bytes   = sum.rx_bytes;
	dev->stats.tx_packets = sum.tx_packets;
	dev->stats.tx_bytes   = sum.tx_bytes;
	return &dev->stats;
}
*/
static struct net_device *etherip_tunnel_dev;
static struct list_head tunnels[E_HASH_SIZE];

static DEFINE_RWLOCK(etherip_lock);

static void etherip_tunnel_setup(struct net_device *dev);

/* add a tunnel to the hash */
static void etherip_tunnel_add(struct etherip_tunnel *tun)
{
	unsigned h = HASH(tun->parms.iph.daddr);
	list_add_tail(&tun->list, &tunnels[h]);
}

/* delete a tunnel from the hash*/
static void etherip_tunnel_del(struct etherip_tunnel *tun)
{
	list_del(&tun->list);
}

/* find a tunnel in the hash by parameters from userspace */
static struct etherip_tunnel* etherip_tunnel_find(struct ip_tunnel_parm *p)
{
	struct etherip_tunnel *ret;
	unsigned h = HASH(p->iph.daddr);

	list_for_each_entry(ret, &tunnels[h], list)
		if (ret->parms.iph.daddr == p->iph.daddr)
			return ret;

	return NULL;
}

/* find a tunnel by its destination address */
static struct etherip_tunnel* etherip_tunnel_locate(u32 remote)
{
	struct etherip_tunnel *ret;
	unsigned h = HASH(remote);

	list_for_each_entry(ret, &tunnels[h], list)
		if (ret->parms.iph.daddr == remote)
			return ret;

	return NULL;
}

/* netdevice start function */
static int etherip_tunnel_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

/* netdevice stop function */
static int etherip_tunnel_stop(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

/* netdevice hard_start_xmit function
 * it gets an Ethernet packet in skb and encapsulates it in another IP
 * packet */
static int etherip_tunnel_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct etherip_tunnel *tunnel = netdev_priv(dev);
//	struct pcpu_tstats *tstats;
	struct iphdr *iph;
	struct flowi4 fl;
	struct net_device *tdev;
	struct rtable *rt;
	int max_headroom;
	int err;
	int pkt_len;
	struct net_device_stats *stats = &tunnel->dev->stats;


	if (tunnel->recursion++) {
		stats->collisions++;
		goto tx_error;
	}

	if (skb->protocol == htons(ETH_P_IPV6)) {
		goto tx_error;
	}

	memset(&fl, 0, sizeof(fl));
	fl.flowi4_oif               = tunnel->parms.link;
	fl.flowi4_proto             = IPPROTO_ETHERIP;
	fl.daddr  = tunnel->parms.iph.daddr;
	fl.saddr  = tunnel->parms.iph.saddr;
	rt = ip_route_output_key(dev_net(dev), &fl);
	if (IS_ERR(rt)) {
		stats->tx_carrier_errors++;
		goto tx_error_icmp;
	}

	tdev = rt->dst.dev;
	if (tdev == dev) {
		ip_rt_put(rt);
		stats->collisions++;
		goto tx_error;
	}

	max_headroom = (LL_RESERVED_SPACE(tdev)+sizeof(struct iphdr)
			+ ETHERIP_HLEN);

	if (skb_headroom(skb) < max_headroom || skb_cloned(skb)
			|| skb_shared(skb)) {
		struct sk_buff *skn = skb_realloc_headroom(skb, max_headroom);
		if (!skn) {
			ip_rt_put(rt);
			dev_kfree_skb(skb);
			dev->stats.tx_dropped++;			
			return 0;
		}
		if (skb->sk)
			skb_set_owner_w(skn, skb->sk);
		dev_kfree_skb(skb);
		skb = skn;
	}

	skb->transport_header = skb->mac_header;
	skb_clear_hash_if_not_l4(skb);

	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));
	IPCB(skb)->flags &= ~(IPSKB_XFRM_TUNNEL_SIZE | IPSKB_XFRM_TRANSFORMED |
			IPSKB_REROUTED);
	skb_push(skb, sizeof(struct iphdr)+ETHERIP_HLEN);
	skb_reset_network_header(skb);
        /* XXX
	 * This code leads to kernel panic if the etherip device is used
	 * in a bridge, the bridge device has no IP, and VLAN packets are
	 * transmitted over the bridge device. don't know why yet as the
	 * pointers seem to be correct
	 * Needs more investigation
	
	 if (skb->dst)
		skb->dst->ops->update_pmtu(skb->dst, dev->mtu);
	*/

	skb_dst_drop(skb);
	skb_dst_set(skb, &rt->dst);

	/* Build the IP header for the outgoing packet
	 *
	 * Note: This driver never sets the DF flag on outgoing packets
	 *       to ensure that the tunnel provides the full Ethernet MTU.
	 *       This behavior guarantees that protocols can be
	 *       encapsulated within the Ethernet packet which do not
	 *       know the concept of a path MTU
	 */
	iph = ip_hdr(skb);
	iph->version = 4;
	iph->ihl = sizeof(struct iphdr)>>2;
	iph->frag_off = 0;
	iph->protocol = IPPROTO_ETHERIP;
	iph->tos = tunnel->parms.iph.tos & INET_ECN_MASK;
	iph->daddr = fl.daddr;
	iph->saddr = fl.saddr;

	iph->ttl = tunnel->parms.iph.ttl;
	if (iph->ttl == 0)
		iph->ttl = ip4_dst_hoplimit(&rt->dst);

	/* add the 16bit etherip header after the ip header */
	((u16*)(iph+1))[0]=htons(ETHERIP_HEADER);
	nf_reset_ct(skb);

	skb->ip_summed = CHECKSUM_NONE;					\
	__ip_select_ident(dev_net(dev), iph, skb_shinfo(skb)->gso_segs ?: 1);

	err = ip_local_out(dev_net(dev), skb->sk, skb);

	if (dev) {
		if (unlikely(net_xmit_eval(err)))
			pkt_len = 0;
		iptunnel_xmit_stats(dev, pkt_len);
	}

	netif_trans_update(tunnel->dev);
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


/* checks parameters the driver gets from userspace */
static int etherip_param_check(struct ip_tunnel_parm *p)
{
	if (p->iph.version != 4 ||
	    p->iph.protocol != IPPROTO_ETHERIP ||
	    p->iph.ihl != 5 ||
	    p->iph.daddr == INADDR_ANY ||
	    ipv4_is_multicast(p->iph.daddr))
		return -EINVAL;

	return 0;
}

/* central ioctl function for all netdevices this driver manages
 * it allows to create, delete, modify a tunnel and fetch tunnel
 * information */
static int etherip_tunnel_siocdevprivate(struct net_device *dev, struct ifreq *ifr,
		       void __user *data, int cmd)
{
	int err = 0;
	struct ip_tunnel_parm p;
	struct net_device *tmp_dev;
	char *dev_name;
	struct etherip_tunnel *t;


	switch (cmd) {
	case SIOCGETTUNNEL:
		t = netdev_priv(dev);
		if (copy_to_user(data, &t->parms,
				sizeof(t->parms)))
			err = -EFAULT;
		break;
	case SIOCADDTUNNEL:
		err = -EINVAL;
		if (dev != etherip_tunnel_dev)
			goto out;

	case SIOCCHGTUNNEL:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto out;

		err = -EFAULT;
		if (copy_from_user(&p, data,
					sizeof(p)))
			goto out;
		p.i_flags = p.o_flags = 0;

		if ((err = etherip_param_check(&p)) < 0)
			goto out;

		t = etherip_tunnel_find(&p);

		err = -EEXIST;
		if (t != NULL && t->dev != dev)
			goto out;

		if (cmd == SIOCADDTUNNEL) {

			p.name[IFNAMSIZ-1] = 0;
			dev_name = p.name;
			if (dev_name[0] == 0)
				dev_name = "ethip%d";

			err = -ENOMEM;
			tmp_dev = alloc_netdev(
					sizeof(struct etherip_tunnel),
					dev_name,NET_NAME_UNKNOWN,
					etherip_tunnel_setup);

			if (tmp_dev == NULL)
				goto out;

			if (strchr(tmp_dev->name, '%')) {
				err = dev_alloc_name(tmp_dev, tmp_dev->name);
				if (err < 0)
					goto add_err;
			}

			t = netdev_priv(tmp_dev);
			t->dev = tmp_dev;
//			tmp_dev->tstats = alloc_percpu(struct pcpu_tstats);
//			if (!tmp_dev->tstats)
//			    goto add_err;

			strncpy(p.name, tmp_dev->name, IFNAMSIZ);
			memcpy(&(t->parms), &p, sizeof(p));

			err = -EFAULT;
			if (copy_to_user(data, &p,
						sizeof(p)))
				goto add_err;
			
			err = register_netdevice(tmp_dev);
			if (err < 0)
				goto add_err;

			write_lock_bh(&etherip_lock);
			etherip_tunnel_add(t);
			write_unlock_bh(&etherip_lock);

		} else {
			err = -EINVAL;
			if ((t = netdev_priv(dev)) == NULL)
				goto out;
			if (dev == etherip_tunnel_dev)
				goto out;
			write_lock_bh(&etherip_lock);
			memcpy(&(t->parms), &p, sizeof(p));
			write_unlock_bh(&etherip_lock);
		}

		err = 0;
		break;
add_err:
		etherip_dev_free(tmp_dev);
		goto out;

	case SIOCDELTUNNEL:
		err = -EPERM;
		if (!capable(CAP_NET_ADMIN))
			goto out;

		err = -EFAULT;
		if (copy_from_user(&p, data,
					sizeof(p)))
			goto out;

		err = -EINVAL;
		if (dev == etherip_tunnel_dev) {
			t = etherip_tunnel_find(&p);
			if (t == NULL) {
				goto out;
			}
		} else
			t = netdev_priv(dev);

		write_lock_bh(&etherip_lock);
		etherip_tunnel_del(t);
		write_unlock_bh(&etherip_lock);

		unregister_netdevice(t->dev);
		err = 0;

		break;
	default:
		err = -EINVAL;
	}

out:
	return err;
}


static const struct net_device_ops etherip_netdev_ops = {
	.ndo_open		= etherip_tunnel_open,
	.ndo_stop		= etherip_tunnel_stop,
	.ndo_start_xmit		= etherip_tunnel_xmit,
	.ndo_siocdevprivate 	= etherip_tunnel_siocdevprivate,
//	.ndo_do_ioctl		= etherip_tunnel_ioctl,
//	.ndo_get_stats  	= etherip_get_stats,
};

/* device init function - called via register_netdevice
 * The tunnel is registered as an Ethernet device. This allows
 * the tunnel to be added to a bridge */
static void etherip_tunnel_setup(struct net_device *dev)
{
	dev->netdev_ops		= &etherip_netdev_ops;
	dev->priv_destructor      = etherip_dev_free;
	dev->features		|= NETIF_F_NETNS_LOCAL;
	dev->priv_flags		&= ~IFF_XMIT_DST_RELEASE;

	ether_setup(dev);
	dev->tx_queue_len = 0;
	eth_hw_addr_random(dev);
}

/* receive function for EtherIP packets
 * Does some basic checks on the MAC addresses and
 * interface modes */
static int etherip_rcv(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct etherip_tunnel *tunnel;
	struct net_device *dev;

	iph = ip_hdr(skb);

	read_lock_bh(&etherip_lock);
	tunnel = etherip_tunnel_locate(iph->saddr);
	if (tunnel == NULL)
		goto drop;
//	struct pcpu_tstats *tstats;

	dev = tunnel->dev;
	secpath_reset(skb);
	skb_pull(skb, ETHERIP_HLEN);
	skb->dev = dev;
	skb->pkt_type = PACKET_HOST;
	skb->protocol = eth_type_trans(skb, tunnel->dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb_dst_drop(skb);

	/* do some checks */
	if (skb->pkt_type == PACKET_HOST || skb->pkt_type == PACKET_BROADCAST)
		goto accept;

	if (skb->pkt_type == PACKET_MULTICAST && (netdev_mc_count(dev) > 0 || dev->flags & IFF_ALLMULTI))
		goto accept;

	if (skb->pkt_type == PACKET_OTHERHOST && dev->flags & IFF_PROMISC)
		goto accept;

drop:
	read_unlock_bh(&etherip_lock);
	kfree_skb(skb);
	return 0;

accept:
//	tstats = this_cpu_ptr(tunnel->dev->tstats);
//	tstats->rx_packets++;
//	tstats->rx_bytes += skb->len;
	
//	tunnel->dev->last_rx = jiffies;
	tunnel->dev->stats.rx_packets++;
	tunnel->dev->stats.rx_bytes += skb->len;
	nf_reset_ct(skb);
	netif_rx(skb);
	read_unlock_bh(&etherip_lock);
	return 0;

}

static struct net_protocol etherip_protocol = {
	.handler      = etherip_rcv,
	.err_handler  = 0,
	.no_policy    = 0,
};

/* module init function
 * initializes the EtherIP protocol (97) and registers the initial
 * device */
static int __init etherip_init(void)
{
	int err, i;
	struct etherip_tunnel *p;

	printk(KERN_INFO BANNER1);

	for (i = 0; i < E_HASH_SIZE; ++i)
		INIT_LIST_HEAD(&tunnels[i]);

	if (inet_add_protocol(&etherip_protocol, IPPROTO_ETHERIP)) {
		printk(KERN_ERR "etherip: can't add protocol\n");
		return -EBUSY;
	}

	etherip_tunnel_dev = alloc_netdev(sizeof(struct etherip_tunnel),
			"etherip0",NET_NAME_UNKNOWN,
			etherip_tunnel_setup);

	if (!etherip_tunnel_dev) {
		err = -ENOMEM;
		goto err2;
	}

	p = netdev_priv(etherip_tunnel_dev);
	p->dev = etherip_tunnel_dev;
	/* set some params for iproute2 */
	strcpy(p->parms.name, "etherip0");
	p->parms.iph.protocol = IPPROTO_ETHERIP;

	if ((err = register_netdev(etherip_tunnel_dev)))
		goto err1;

//	etherip_tunnel_dev->tstats = alloc_percpu(struct pcpu_tstats);
//	if (!etherip_tunnel_dev->tstats)
//		return -ENOMEM;

out:
	return err;
err1:
	etherip_dev_free(etherip_tunnel_dev);
err2:
	inet_del_protocol(&etherip_protocol, IPPROTO_ETHERIP);
	goto out;
}

/* destroy all tunnels */
static void __exit etherip_destroy_tunnels(void)
{
	int i;
	struct list_head *ptr;
	struct etherip_tunnel *tun;

	for (i = 0; i < E_HASH_SIZE; ++i) {
		list_for_each(ptr, &tunnels[i]) {
			tun = list_entry(ptr, struct etherip_tunnel, list);
			ptr = ptr->prev;
			etherip_tunnel_del(tun);
			unregister_netdevice(tun->dev);
		}
	}
}

/* module cleanup function */
static void __exit etherip_exit(void)
{
	rtnl_lock();
	etherip_destroy_tunnels();
	unregister_netdevice(etherip_tunnel_dev);
	rtnl_unlock();
	if (inet_del_protocol(&etherip_protocol, IPPROTO_ETHERIP))
		printk(KERN_ERR "etherip: can't remove protocol\n");
}

module_init(etherip_init);
module_exit(etherip_exit);
