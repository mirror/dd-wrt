/**
 * Strip all IP options in the IP packet header.
 *
 * (C) 2001 by Fabrice MARIE <fabrice@netfilter.org>
 * This software is distributed under GNU GPL v2, 1991
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>

#include <linux/netfilter_ipv4/ip_tables.h>

MODULE_AUTHOR("Fabrice MARIE <fabrice@netfilter.org>");
MODULE_DESCRIPTION("Strip all options in IPv4 packets");
MODULE_LICENSE("GPL");

static unsigned int
target(struct sk_buff **pskb,
       unsigned int hooknum,
       const struct net_device *in,
       const struct net_device *out,
       const void *targinfo,
       void *userinfo)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct sk_buff *skb = (*pskb);
	struct ip_options * opt;
	unsigned char * optiph = skb->nh.raw;
	int l = ((struct ip_options *)(&(IPCB(skb)->opt)))->optlen;
	

	/* if no options in packet then nothing to clear. */
	if (iph->ihl * 4 == sizeof(struct iphdr))
		return IPT_CONTINUE;

	/* else clear all options */
	memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));
	memset(optiph+sizeof(struct iphdr), IPOPT_NOOP, l);
	opt = &(IPCB(skb)->opt);
	opt->is_data = 0;
	opt->optlen = l;

	skb->nfcache |= NFC_ALTERED;

        return IPT_CONTINUE;
}

static int
checkentry(const char *tablename,
	   const struct ipt_entry *e,
           void *targinfo,
           unsigned int targinfosize,
           unsigned int hook_mask)
{
	if (strcmp(tablename, "mangle")) {
		printk(KERN_WARNING "IPV4OPTSSTRIP: can only be called from \"mangle\" table, not \"%s\"\n", tablename);
		return 0;
	}
	/* nothing else to check because no parameters */
	return 1;
}

static struct ipt_target ipt_ipv4optsstrip_reg
= { { NULL, NULL }, "IPV4OPTSSTRIP", target, checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	if (ipt_register_target(&ipt_ipv4optsstrip_reg))
		return -EINVAL;
	printk("ipt_IPV4OPTSSTRIP loaded\n");

	return 0;
}

static void __exit fini(void)
{
	ipt_unregister_target(&ipt_ipv4optsstrip_reg);
	printk("ipt_IPV4OPTSSTRIP unloaded\n");
}

module_init(init);
module_exit(fini);
