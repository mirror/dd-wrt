/*
 * This implements the ROUTE target, which enables you to setup unusual
 * routes not supported by the standard kernel routing table.
 *
 * Copyright (C) 2002 Cedric de Launois <delaunois@info.ucl.ac.be>
 *
 * v 1.11 2004/11/23
 *
 * This software is distributed under GNU GPL v2, 1991
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ipt_ROUTE.h>
#include <linux/netdevice.h>
#include <linux/route.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/icmp.h>
#include <net/checksum.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif


/* Try to route the packet according to the routing keys specified in
 * route_info. Keys are :
 *  - ifindex : 
 *      0 if no oif preferred, 
 *      otherwise set to the index of the desired oif
 *  - route_info->gw :
 *      0 if no gateway specified,
 *      otherwise set to the next host to which the pkt must be routed
 * If success, skb->dev is the output device to which the packet must 
 * be sent and skb->dst is not NULL
 *
 * RETURN: -1 if an error occured
 *          1 if the packet was succesfully routed to the 
 *            destination desired
 *          0 if the kernel routing table could not route the packet
 *            according to the keys specified
 */
static int route(struct sk_buff *skb,
		 unsigned int ifindex,
		 const struct ipt_route_target_info *route_info)
{
	int err;
	struct rtable *rt;
	struct iphdr *iph = skb->nh.iph;
	struct rt_key key = { 
		dst:iph->daddr,
		src:0,
		oif:ifindex, 
		tos:RT_TOS(iph->tos) 
	};
	
	/* The destination address may be overloaded by the target */
	if (route_info->gw)
		key.dst = route_info->gw;
	
	/* Trying to route the packet using the standard routing table. */
	if ((err = ip_route_output_key(&rt, &key))) {
		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: couldn't route pkt (err: %i)",err);
		return -1;
	}
	
	/* Drop old route. */
	dst_release(skb->dst);
	skb->dst = NULL;

	/* Success if no oif specified or if the oif correspond to the 
	 * one desired */
	if (!ifindex || rt->u.dst.dev->ifindex == ifindex) {
		skb->dst = &rt->u.dst;
		skb->dev = skb->dst->dev;
		return 1;
	}
	
	/* The interface selected by the routing table is not the one
	 * specified by the user. This may happen because the dst address
	 * is one of our own addresses.
	 */
	if (net_ratelimit()) 
		DEBUGP("ipt_ROUTE: failed to route as desired gw=%u.%u.%u.%u oif=%i (got oif=%i)\n", 
		       NIPQUAD(route_info->gw), ifindex, rt->u.dst.dev->ifindex);
	
	return 0;
}


/* Stolen from ip_finish_output2
 * PRE : skb->dev is set to the device we are leaving by
 *       skb->dst is not NULL
 * POST: the packet is sent with the link layer header pushed
 *       the packet is destroyed
 */
static void ip_direct_send(struct sk_buff *skb)
{
	struct dst_entry *dst = skb->dst;
	struct hh_cache *hh = dst->hh;

	if (hh) {
		read_lock_bh(&hh->hh_lock);
		memcpy(skb->data - 16, hh->hh_data, 16);
		read_unlock_bh(&hh->hh_lock);
		skb_push(skb, hh->hh_len);
		hh->hh_output(skb);
	} else if (dst->neighbour)
		dst->neighbour->output(skb);
	else {
		if (net_ratelimit())
			DEBUGP(KERN_DEBUG "ipt_ROUTE: no hdr & no neighbour cache!\n");
		kfree_skb(skb);
	}
}


/* PRE : skb->dev is set to the device we are leaving by
 * POST: - the packet is directly sent to the skb->dev device, without 
 *         pushing the link layer header.
 *       - the packet is destroyed
 */
static inline int dev_direct_send(struct sk_buff *skb)
{
	return dev_queue_xmit(skb);
}


static unsigned int route_oif(const struct ipt_route_target_info *route_info,
			      struct sk_buff *skb) 
{
	unsigned int ifindex = 0;
	struct net_device *dev_out = NULL;

	/* The user set the interface name to use.
	 * Getting the current interface index.
	 */
	if ((dev_out = dev_get_by_name(route_info->oif))) {
		ifindex = dev_out->ifindex;
	} else {
		/* Unknown interface name : packet dropped */
		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: oif interface %s not found\n", route_info->oif);
		return NF_DROP;
	}

	/* Trying the standard way of routing packets */
	switch (route(skb, ifindex, route_info)) {
	case 1:
		dev_put(dev_out);
		if (route_info->flags & IPT_ROUTE_CONTINUE)
			return IPT_CONTINUE;

		ip_direct_send(skb);
		return NF_STOLEN;

	case 0:
		/* Failed to send to oif. Trying the hard way */
		if (route_info->flags & IPT_ROUTE_CONTINUE)
			return NF_DROP;

		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: forcing the use of %i\n",
			       ifindex);

		/* We have to force the use of an interface.
		 * This interface must be a tunnel interface since
		 * otherwise we can't guess the hw address for
		 * the packet. For a tunnel interface, no hw address
		 * is needed.
		 */
		if ((dev_out->type != ARPHRD_TUNNEL)
		    && (dev_out->type != ARPHRD_IPGRE)) {
			if (net_ratelimit()) 
				DEBUGP("ipt_ROUTE: can't guess the hw addr !\n");
			dev_put(dev_out);
			return NF_DROP;
		}
	
		/* Send the packet. This will also free skb
		 * Do not go through the POST_ROUTING hook because 
		 * skb->dst is not set and because it will probably
		 * get confused by the destination IP address.
		 */
		skb->dev = dev_out;
		dev_direct_send(skb);
		dev_put(dev_out);
		return NF_STOLEN;
		
	default:
		/* Unexpected error */
		dev_put(dev_out);
		return NF_DROP;
	}
}


static unsigned int route_iif(const struct ipt_route_target_info *route_info,
			      struct sk_buff *skb) 
{
	struct net_device *dev_in = NULL;

	/* Getting the current interface index. */
	if (!(dev_in = dev_get_by_name(route_info->iif))) {
		if (net_ratelimit()) 
			DEBUGP("ipt_ROUTE: iif interface %s not found\n", route_info->iif);
		return NF_DROP;
	}

	skb->dev = dev_in;
	dst_release(skb->dst);
	skb->dst = NULL;
		
	netif_rx(skb);
	dev_put(dev_in);
	return NF_STOLEN;
}


static unsigned int route_gw(const struct ipt_route_target_info *route_info,
			     struct sk_buff *skb) 
{
	if (route(skb, 0, route_info)!=1)
		return NF_DROP;

	if (route_info->flags & IPT_ROUTE_CONTINUE)
		return IPT_CONTINUE;

	ip_direct_send(skb);
	return NF_STOLEN;
}

/* To detect and deter routed packet loopback when using the --tee option,
 * we take a page out of the raw.patch book: on the copied skb, we set up
 * a fake ->nfct entry, pointing to the local &route_tee_track. We skip
 * routing packets when we see they already have that ->nfct.
 */

static struct ip_conntrack route_tee_track;

static unsigned int ipt_route_target(struct sk_buff **pskb,
				     unsigned int hooknum,
				     const struct net_device *in,
				     const struct net_device *out,
				     const void *targinfo,
				     void *userinfo)
{
	const struct ipt_route_target_info *route_info = targinfo;
	struct sk_buff *skb = *pskb;
	unsigned int res;

	/* If we are at PREROUTING or INPUT hook
	 * the TTL isn't decreased by the IP stack
	 */
	if (hooknum == NF_IP_PRE_ROUTING ||
	    hooknum == NF_IP_LOCAL_IN) {

		struct iphdr *iph = skb->nh.iph;

		if (iph->ttl <= 1) {
			struct rtable *rt;

			if (ip_route_output(&rt, iph->saddr, iph->daddr,
					    RT_TOS(iph->tos) | RTO_CONN,
					    0)) {
				return NF_DROP;
			}

			if (skb->dev == rt->u.dst.dev) {
				/* Drop old route. */
				dst_release(skb->dst);
				skb->dst = &rt->u.dst;

				/* this will traverse normal stack, and 
				 * thus call conntrack on the icmp packet */
				icmp_send(skb, ICMP_TIME_EXCEEDED, 
					  ICMP_EXC_TTL, 0);
			}

			return NF_DROP;
		}

		/*
		 * If we are at INPUT the checksum must be recalculated since
		 * the length could change as the result of a defragmentation.
		 * -- Rickard Molin
		 */
		if(hooknum == NF_IP_LOCAL_IN) {
			iph->ttl = iph->ttl - 1;
			iph->check = 0;
			iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
		} else {
			ip_decrease_ttl(iph);
		}
	}

	if ((route_info->flags & IPT_ROUTE_TEE)) {
		/*
		 * Copy the *pskb, and route the copy. Will later return
		 * IPT_CONTINUE for the original skb, which should continue
		 * on its way as if nothing happened. The copy should be
		 * independantly delivered to the ROUTE --gw.
		 */
		skb = skb_copy(*pskb, GFP_ATOMIC);
		if (!skb) {
			if (net_ratelimit()) 
				DEBUGP(KERN_DEBUG "ipt_ROUTE: copy failed!\n");
			return IPT_CONTINUE;
		}
	}

	/* Tell conntrack to forget this packet since it may get confused 
	 * when a packet is leaving with dst address == our address.
	 * Good idea ? Dunno. Need advice.
	 *
	 * NEW: mark the skb with our &route_tee_track, so we avoid looping
	 * on any already routed packet.
	 */
	if (!(route_info->flags & IPT_ROUTE_CONTINUE)) {
		nf_conntrack_put(skb->nfct);
		skb->nfct = &route_tee_track.infos[IP_CT_NEW];
		nf_conntrack_get(skb->nfct);
		skb->nfcache = 0;
#ifdef CONFIG_NETFILTER_DEBUG
		skb->nf_debug = 0;
#endif
	}

	if (route_info->oif[0]) {
		res = route_oif(route_info, skb);
	} else if (route_info->iif[0]) {
		res = route_iif(route_info, skb);
	} else if (route_info->gw) {
		res = route_gw(route_info, skb);
	} else {
		if (net_ratelimit()) 
			DEBUGP(KERN_DEBUG "ipt_ROUTE: no parameter !\n");
		res = IPT_CONTINUE;
	}

	if ((route_info->flags & IPT_ROUTE_TEE))
		res = IPT_CONTINUE;

	return res;
}


static int ipt_route_checkentry(const char *tablename,
				const struct ipt_entry *e,
				void *targinfo,
				unsigned int targinfosize,
				unsigned int hook_mask)
{
	if (strcmp(tablename, "mangle") != 0) {
		printk("ipt_ROUTE: bad table `%s', use the `mangle' table.\n",
		       tablename);
		return 0;
	}

	if (hook_mask & ~(  (1 << NF_IP_PRE_ROUTING)
			    | (1 << NF_IP_LOCAL_IN)
			    | (1 << NF_IP_FORWARD)
			    | (1 << NF_IP_LOCAL_OUT)
			    | (1 << NF_IP_POST_ROUTING))) {
		printk("ipt_ROUTE: bad hook\n");
		return 0;
	}

	if (targinfosize != IPT_ALIGN(sizeof(struct ipt_route_target_info))) {
		printk(KERN_WARNING "ipt_ROUTE: targinfosize %u != %Zu\n",
		       targinfosize,
		       IPT_ALIGN(sizeof(struct ipt_route_target_info)));
		return 0;
	}

	return 1;
}


static struct ipt_target ipt_route_reg
= { { NULL, NULL }, "ROUTE", ipt_route_target, ipt_route_checkentry, NULL,
    THIS_MODULE };


static int __init init(void)
{
	/* Set up fake conntrack (stolen from raw.patch):
	    - to never be deleted, not in any hashes */
	atomic_set(&route_tee_track.ct_general.use, 1);
	/*  - and look it like as a confirmed connection */
	set_bit(IPS_CONFIRMED_BIT, &route_tee_track.status);
	/*  - and prepare the ctinfo field for REJECT/NAT. */
	route_tee_track.infos[IP_CT_NEW].master = 
	route_tee_track.infos[IP_CT_RELATED].master = 
	route_tee_track.infos[IP_CT_RELATED + IP_CT_IS_REPLY].master = 
		&route_tee_track.ct_general;
	/* Initialize fake conntrack so that NAT will skip it */
	route_tee_track.nat.info.initialized |= 
		(1 << IP_NAT_MANIP_SRC) | (1 << IP_NAT_MANIP_DST);

	if (ipt_register_target(&ipt_route_reg))
		return -EINVAL;

	return 0;
}


static void __exit fini(void)
{
	ipt_unregister_target(&ipt_route_reg);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
