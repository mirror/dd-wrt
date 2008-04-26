/*
 *      $Id: ip_wccp.c,v 1.7 2005/01/07 17:26:33 hno Exp $
 *
 * Maintainer:
 *		Henrik Nordstrom <hno@squid-cache.org>
 *
 * Change log:
 *   2004-08-19 SONE Naoto
 *		Updated to support Linux 2.6.8
 *
 *   2004-02-17 Henrik Nordstrom <hno@squid-cache.org>
 *		Updated to linux-2.6.0
 *		WCCPv2 support
 *
 *   2003-10-20 Henrik Nordstrom <hno@squid-cache.org>
 *   		Dropped support for old kernels. Linux-2.4 or later required
 *   		Play well with Netfilter
 *
 *   2002-04-16 francis a. vidal <francisv@dagupan.com>
 *		Module license tag
 *
 *   2002-04-13 Henrik Nordstrom <hno@squid-cache.org>
 *   		Updated to Linux-2.4
 *		- there no longer is a len argument to ip_wccp_recv
 *		- deal with fragmented skb packets
 *		- incremental checksumming to allow detection of corrupted
 *		  packets
 *
 *   1999-09-30 Glenn Chisholm <glenn@ircache.net>
 *              Original release
 */

#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/in.h>
#include <linux/if_arp.h>
#include <linux/init.h>
#include <linux/inetdevice.h>
#include <linux/version.h>
#include <net/checksum.h>
#include <net/protocol.h>
#include <linux/netfilter_ipv4.h>
#include <net/ip.h>
#include <net/inet_ecn.h>

#define WCCP_PROTOCOL_TYPE 	0x883E
#define WCCP_GRE_LEN		sizeof(u32)
#define WCCP2_GRE_EXTRA		sizeof(u32)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,9)
/* New License scheme */
#ifdef MODULE_LICENSE
MODULE_AUTHOR("Glenn Chisholm");
MODULE_DESCRIPTION("WCCP module");
MODULE_LICENSE("GPL");
#endif
#endif

static inline void ip_wccp_ecn_decapsulate(struct iphdr *outer_iph, struct sk_buff *skb)
{
	struct iphdr *inner_iph = skb->nh.iph;

	if (INET_ECN_is_ce(outer_iph->tos))
	    IP_ECN_set_ce(inner_iph);
}


int ip_wccp_rcv(struct sk_buff *skb)
{
	u32  *gre_hdr;
	struct iphdr *iph;

	if (!pskb_may_pull(skb, 16))
		goto drop;

	iph = skb->nh.iph;
	gre_hdr = (u32 *)skb->h.raw;
	if(*gre_hdr != __constant_htonl(WCCP_PROTOCOL_TYPE)) 
		goto drop;

	skb->mac.raw = skb->nh.raw;

	/* WCCP2 puts an extra 4 octets into the header, but uses the same
	 * encapsulation type; if it looks as if the first octet of the packet
	 * isn't the beginning of an IPv4 header, assume it's WCCP2.
	 * This should be safe as these bits are reserved in the WCCPv2 header
	 * and always zero in WCCPv2.
	 */
	if ((skb->h.raw[WCCP_GRE_LEN] & 0xF0) != 0x40) {
		skb->nh.raw = pskb_pull(skb, WCCP_GRE_LEN + WCCP2_GRE_EXTRA);
	} else { 
		skb->nh.raw = pskb_pull(skb, WCCP_GRE_LEN);
	}
	if (skb->len <= 0) 
		goto drop;

	memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));
	skb->protocol = __constant_htons(ETH_P_IP);
	skb->pkt_type = PACKET_HOST;

	dst_release(skb->dst);
	skb->dst = NULL;
#ifdef CONFIG_NETFILTER
	nf_conntrack_put(skb->nfct);
	skb->nfct = NULL;
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif
#endif
	ip_wccp_ecn_decapsulate(iph, skb);
	netif_rx(skb);
	return(0);

drop:
	kfree_skb(skb);
	return(0);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,8)
static struct net_protocol ipwccp_protocol = {
#else
static struct inet_protocol ipwccp_protocol = {
#endif
    .handler	= ip_wccp_rcv
};
static inline void wccp_add_protocol(void) { inet_add_protocol(&ipwccp_protocol, IPPROTO_GRE); }
static inline int wccp_del_protocol(void) { return inet_del_protocol(&ipwccp_protocol, IPPROTO_GRE); }
#else
static struct inet_protocol ipwccp_protocol = {
  ip_wccp_rcv,     
  NULL,           
  0,            
  IPPROTO_GRE, 
  0,          
  NULL,      
  "GRE"     
};
static inline void wccp_add_protocol(void) { inet_add_protocol(&ipwccp_protocol); }
static inline int wccp_del_protocol(void) { return inet_del_protocol(&ipwccp_protocol); }
#endif

int __init ip_wccp_init(void)
{
	printk(KERN_INFO "WCCP IPv4/GRE driver\n");
	wccp_add_protocol();
	return 0;
}

static void __exit ip_wccp_fini(void)
{
	if (wccp_del_protocol() < 0)
		printk(KERN_INFO "ip_wccp: can't remove protocol\n");
	else
		printk(KERN_INFO "WCCP IPv4/GRE driver unloaded\n");
}

#ifdef MODULE
module_init(ip_wccp_init);
#endif
module_exit(ip_wccp_fini);
