/*
 * This is a module which is used for setting the skb->priority field
 * of an skb for qdisc classification.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_CLASSIFY.h>

MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("iptables qdisc classification target module");

static unsigned int
target(struct sk_buff **pskb,
       unsigned int hooknum,
       const struct net_device *in,
       const struct net_device *out,
       const void *targinfo,
       void *userinfo)
{
	const struct ipt_classify_target_info *clinfo = targinfo;

	if((*pskb)->priority != clinfo->priority) {
		(*pskb)->priority = clinfo->priority;
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
	if (targinfosize != IPT_ALIGN(sizeof(struct ipt_classify_target_info))){
		printk(KERN_ERR "CLASSIFY: invalid size (%u != %u).\n",
		       targinfosize,
		       IPT_ALIGN(sizeof(struct ipt_classify_target_info)));
		return 0;
	}
	
	if (hook_mask & ~(1 << NF_IP_POST_ROUTING)) {
		printk(KERN_ERR "CLASSIFY: only valid in POST_ROUTING.\n");
		return 0;
	}

	if (strcmp(tablename, "mangle") != 0) {
		printk(KERN_WARNING "CLASSIFY: can only be called from "
		                    "\"mangle\" table, not \"%s\".\n",
		                    tablename);
		return 0;
	}

	return 1;
}

static struct ipt_target ipt_classify_reg
= { { NULL, NULL }, "CLASSIFY", target, checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	if (ipt_register_target(&ipt_classify_reg))
		return -EINVAL;

	return 0;
}

static void __exit fini(void)
{
	ipt_unregister_target(&ipt_classify_reg);
}

module_init(init);
module_exit(fini);
