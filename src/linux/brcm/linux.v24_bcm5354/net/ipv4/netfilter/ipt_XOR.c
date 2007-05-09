/* XOR target for IP tables
 * (C) 2000 by Tim Vandermeersch <Tim.Vandermeersch@pandora.be>
 * Based on ipt_TTL.c
 *
 * Version 1.0
 *
 * This software is distributed under the terms of GNU GPL
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_XOR.h>

MODULE_AUTHOR("Tim Vandermeersch <Tim.Vandermeersch@pandora.be>");
MODULE_DESCRIPTION("IP tables XOR module");
MODULE_LICENSE("GPL");

static unsigned int ipt_xor_target(struct sk_buff **pskb, unsigned int hooknum, 
		const struct net_device *in, const struct net_device *out, 
		const void *targinfo, void *userinfo)
{
	struct ipt_XOR_info *info = (void *) targinfo;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph;
	struct udphdr *udph;
	int i, j, k;
  
	if (iph->protocol == IPPROTO_TCP) {
		tcph = (struct tcphdr *) ((*pskb)->data + iph->ihl*4);
		for (i=0, j=0; i<(ntohs(iph->tot_len) - iph->ihl*4 - tcph->doff*4); ) {
			for (k=0; k<=info->block_size; k++) {
			        (*pskb)->data[ iph->ihl*4 + tcph->doff*4 + i ] ^= info->key[j];
				i++;
			}
			j++;
			if (info->key[j] == 0x00)
				j = 0;
		}
	} else if (iph->protocol == IPPROTO_UDP) {
		udph = (struct udphdr *) ((*pskb)->data + iph->ihl*4);
		for (i=0, j=0; i<(ntohs(udph->len)-8); ) {
			for (k=0; k<=info->block_size; k++) {
				(*pskb)->data[ iph->ihl*4 + sizeof(struct udphdr) + i ] ^= info->key[j];
				i++;
			}
			j++;
			if (info->key[j] == 0x00)
				j = 0;
		}
	}
  
	return IPT_CONTINUE;
}

static int ipt_xor_checkentry(const char *tablename, const struct ipt_entry *e,
		void *targinfo, unsigned int targinfosize, 
		unsigned int hook_mask)
{
	struct ipt_XOR_info *info = targinfo;

	if (targinfosize != IPT_ALIGN(sizeof(struct ipt_XOR_info))) {
		printk(KERN_WARNING "XOR: targinfosize %u != %Zu\n", 
				targinfosize, IPT_ALIGN(sizeof(struct ipt_XOR_info)));
	return 0;
	}	

	if (strcmp(tablename, "mangle")) {
		printk(KERN_WARNING "XOR: can only be called from"
				"\"mangle\" table, not \"%s\"\n", tablename);
		return 0; 
	}

	if (!strcmp(info->key, "")) {
		printk(KERN_WARNING "XOR: You must specify a key");
		return 0;
	}

	if (info->block_size == 0) {
		printk(KERN_WARNING "XOR: You must specify a block-size");
		return 0;
	}

	return 1;
}

static struct ipt_target ipt_XOR = { { NULL, NULL }, "XOR",
	ipt_xor_target, ipt_xor_checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_target(&ipt_XOR);
}

static void __exit fini(void)
{
	ipt_unregister_target(&ipt_XOR);
}

module_init(init);
module_exit(fini);
