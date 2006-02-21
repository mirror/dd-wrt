/* This is a module which is used for setting the TOS field of a packet. */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_TOS.h>

static unsigned int
target(struct sk_buff **pskb,
       unsigned int hooknum,
       const struct net_device *in,
       const struct net_device *out,
       const void *targinfo,
       void *userinfo)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	const struct ipt_tos_target_info *tosinfo = targinfo;

	if ((iph->tos & IPTOS_TOS_MASK) != tosinfo->tos) {
		u_int16_t diffs[2];

		/* raw socket (tcpdump) may have clone of incoming
                   skb: don't disturb it --RR */
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;
			iph = (*pskb)->nh.iph;
		}

		diffs[0] = htons(iph->tos) ^ 0xFFFF;
		iph->tos = (iph->tos & IPTOS_PREC_MASK) | tosinfo->tos;
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF));
		(*pskb)->nfcache |= NFC_ALTERED;
	}
	return IPT_CONTINUE;
}

static int
checkentry(const char *tablename,
	   const struct ipt_entry *e,
           void *targinfo,
           unsigned int targinfosize,
           unsigned int hook_mask)
{
	const u_int8_t tos = ((struct ipt_tos_target_info *)targinfo)->tos;

	if (targinfosize != IPT_ALIGN(sizeof(struct ipt_tos_target_info))) {
		printk(KERN_WARNING "TOS: targinfosize %u != %Zu\n",
		       targinfosize,
		       IPT_ALIGN(sizeof(struct ipt_tos_target_info)));
		return 0;
	}

	if (strcmp(tablename, "mangle") != 0) {
		printk(KERN_WARNING "TOS: can only be called from \"mangle\" table, not \"%s\"\n", tablename);
		return 0;
	}

	if (tos != IPTOS_LOWDELAY
	    && tos != IPTOS_THROUGHPUT
	    && tos != IPTOS_RELIABILITY
	    && tos != IPTOS_MINCOST
	    && tos != IPTOS_NORMALSVC) {
		printk(KERN_WARNING "TOS: bad tos value %#x\n", tos);
		return 0;
	}

	return 1;
}

static struct ipt_target ipt_tos_reg
= { { NULL, NULL }, "TOS", target, checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	if (ipt_register_target(&ipt_tos_reg))
		return -EINVAL;

	return 0;
}

static void __exit fini(void)
{
	ipt_unregister_target(&ipt_tos_reg);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
