/* iptables module for the IPv4 and TCP ECN bits, Version 1.2
 *
 * (C) 2002 by Harald Welte <laforge@gnumonks.org>
 * 
 * This software is distributed under GNU GPL v2, 1991
 * 
 * ipt_ECN.c,v 1.7 2003/12/15 15:18:06 laforge Exp 
*/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_ECN.h>

MODULE_LICENSE("GPL");

/* set ECT codepoint from IP header.
 * 	return 0 in case there was no ECT codepoint
 * 	return 1 in case ECT codepoint has been overwritten
 * 	return < 0 in case there was error */
static int inline
set_ect_ip(struct sk_buff **pskb, struct iphdr *iph,
	   const struct ipt_ECN_info *einfo)
{
	if ((iph->tos & IPT_ECN_IP_MASK)
	    != (einfo->ip_ect & IPT_ECN_IP_MASK)) {
		u_int16_t diffs[2];

		/* raw socket (tcpdump) may have clone of incoming
		 * skb: don't disturb it --RR */
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;
			iph = (*pskb)->nh.iph;
		}

		diffs[0] = htons(iph->tos) ^ 0xFFFF;
		iph->tos = iph->tos & ~IPT_ECN_IP_MASK;
		iph->tos = iph->tos | (einfo->ip_ect & IPT_ECN_IP_MASK);
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF));
		(*pskb)->nfcache |= NFC_ALTERED;

		return 1;
	} 
	return 0;
}

static int inline
set_ect_tcp(struct sk_buff **pskb, struct iphdr *iph,
	    const struct ipt_ECN_info *einfo)
{

	struct tcphdr *tcph;
	u_int16_t *tcpflags;
	u_int16_t diffs[2];

	/* raw socket (tcpdump) may have clone of incoming
	 * skb: don't disturb it --RR */
	if (skb_cloned(*pskb) && !(*pskb)->sk) {
		struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
		if (!nskb)
			return NF_DROP;
		kfree_skb(*pskb);
		*pskb = nskb;
	}

	iph = (*pskb)->nh.iph;
	tcph = (void *) iph + iph->ihl * 4;
	tcpflags = (u_int16_t *)tcph + 6;

	diffs[0] = *tcpflags;

	if (einfo->operation & IPT_ECN_OP_SET_ECE
	    && tcph->ece != einfo->proto.tcp.ece) {
		tcph->ece = einfo->proto.tcp.ece;
	}

	if (einfo->operation & IPT_ECN_OP_SET_CWR
	    && tcph->cwr != einfo->proto.tcp.cwr) {
		tcph->cwr = einfo->proto.tcp.cwr;
	}
	
	if (diffs[0] != *tcpflags) {
		diffs[0] = diffs[0] ^ 0xFFFF;
		diffs[1] = *tcpflags;
		tcph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    tcph->check^0xFFFF));
		(*pskb)->nfcache |= NFC_ALTERED;

		return 1;
	}

	return 0;
}

static unsigned int
target(struct sk_buff **pskb,
       unsigned int hooknum,
       const struct net_device *in,
       const struct net_device *out,
       const void *targinfo,
       void *userinfo)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	const struct ipt_ECN_info *einfo = targinfo;

	if (einfo->operation & IPT_ECN_OP_SET_IP)
		set_ect_ip(pskb, iph, einfo);

	if (einfo->operation & (IPT_ECN_OP_SET_ECE | IPT_ECN_OP_SET_CWR)
	    && iph->protocol == IPPROTO_TCP)
		set_ect_tcp(pskb, iph, einfo);

	return IPT_CONTINUE;
}

static int
checkentry(const char *tablename,
	   const struct ipt_entry *e,
           void *targinfo,
           unsigned int targinfosize,
           unsigned int hook_mask)
{
	const struct ipt_ECN_info *einfo = (struct ipt_ECN_info *)targinfo;

	if (targinfosize != IPT_ALIGN(sizeof(struct ipt_ECN_info))) {
		printk(KERN_WARNING "ECN: targinfosize %u != %Zu\n",
		       targinfosize,
		       IPT_ALIGN(sizeof(struct ipt_ECN_info)));
		return 0;
	}

	if (strcmp(tablename, "mangle") != 0) {
		printk(KERN_WARNING "ECN: can only be called from \"mangle\" table, not \"%s\"\n", tablename);
		return 0;
	}

	if (einfo->operation & IPT_ECN_OP_MASK) {
		printk(KERN_WARNING "ECN: unsupported ECN operation %x\n",
			einfo->operation);
		return 0;
	}
	if (einfo->ip_ect & ~IPT_ECN_IP_MASK) {
		printk(KERN_WARNING "ECN: new ECT codepoint %x out of mask\n",
			einfo->ip_ect);
		return 0;
	}

	if ((einfo->operation & (IPT_ECN_OP_SET_ECE|IPT_ECN_OP_SET_CWR))
	    && e->ip.proto != IPPROTO_TCP) {
		printk(KERN_WARNING "ECN: cannot use TCP operations on a "
		       "non-tcp rule\n");
		return 0;
	}

	return 1;
}

static struct ipt_target ipt_ecn_reg
= { { NULL, NULL }, "ECN", target, checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	if (ipt_register_target(&ipt_ecn_reg))
		return -EINVAL;

	return 0;
}

static void __exit fini(void)
{
	ipt_unregister_target(&ipt_ecn_reg);
}

module_init(init);
module_exit(fini);
