/* ipt_SET.c - netfilter target to manipulate IP sets
 *
 * This target can be used almost everywhere. It acts on some specified
 * IP set, adding or deleting some IP addresses/ports in the set. 
 * The addresses/ports can be either the source, or destination
 * of the packet under inspection.
 *
 */

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/protocol.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ipt_set.h>

static unsigned int
target(struct sk_buff **pskb,
       unsigned int hooknum,
       const struct net_device *in,
       const struct net_device *out,
       const void *targinfo,
       void *userinfo)
{
	const struct ipt_set_info_target *info = targinfo;
	
	if (info->add_set.id >= 0)
		ip_set_addip_kernel(ip_set_list[info->add_set.id],
				    *pskb,
				    info->add_set.flags,
				    info->add_set.set_level,
				    info->add_set.ip_level);
	if (info->del_set.id >= 0)
		ip_set_delip_kernel(ip_set_list[info->del_set.id],
				    *pskb,
				    info->del_set.flags,
				    info->del_set.set_level,
				    info->del_set.ip_level);

	return IPT_CONTINUE;
}

static int
checkentry(const char *tablename,
	   const struct ipt_entry *e,
	   void *targinfo,
	   unsigned int targinfosize, unsigned int hook_mask)
{
	struct ipt_set_info_target *info = targinfo;

	if (targinfosize != IPT_ALIGN(sizeof(*info))) {
		DP("bad target info size %u", targinfosize);
		return 0;
	}

	if (info->add_set.id >= 0
	    && !ip_set_get_byid(info->add_set.id)) {
		ip_set_printk("cannot verify add_set id %i as target",
			      info->add_set.id);
		return 0;	/* error */
	}
	if (info->del_set.id >= 0
	    && !ip_set_get_byid(info->del_set.id)) {
		ip_set_printk("cannot verify del_set id %i as target",
			      info->del_set.id);
		return 0;	/* error */
	}
	DP("checkentry OK");

	return 1;
}

static void destroy(void *targetinfo, unsigned int targetsize)
{
	struct ipt_set_info_target *info = targetinfo;

	if (targetsize != IPT_ALIGN(sizeof(struct ipt_set_info_target))) {
		ip_set_printk("invalid targetsize %d", targetsize);
		return;
	}

	if (info->add_set.id >= 0)
		ip_set_put(ip_set_list[info->add_set.id]);
	if (info->del_set.id >= 0)
		ip_set_put(ip_set_list[info->del_set.id]);
}

static struct ipt_target SET_target = {
	.name 		= "SET",
	.target 	= target,
	.checkentry 	= checkentry,
	.destroy 	= destroy,
	.me 		= THIS_MODULE
};

static int __init init(void)
{
	return ipt_register_target(&SET_target);
}

static void __exit fini(void)
{
	ipt_unregister_target(&SET_target);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
