/* This target marks packets to be enqueued to an imq device */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_IMQ.h>
#include <linux/imq.h>

static unsigned int imq_target(struct sk_buff **pskb,
			       unsigned int hooknum,
			       const struct net_device *in,
			       const struct net_device *out,
			       const void *targinfo,
			       void *userinfo)
{
	struct ipt_imq_info *mr = (struct ipt_imq_info*)targinfo;

	(*pskb)->imq_flags = mr->todev | IMQ_F_ENQUEUE;
	(*pskb)->nfcache |= NFC_ALTERED;

	return IPT_CONTINUE;
}

static int imq_checkentry(const char *tablename,
			  const struct ipt_entry *e,
			  void *targinfo,
			  unsigned int targinfosize,
			  unsigned int hook_mask)
{
	struct ipt_imq_info *mr;

	if (targinfosize != IPT_ALIGN(sizeof(struct ipt_imq_info))) {
		printk(KERN_WARNING "IMQ: invalid targinfosize\n");
		return 0;
	}
	mr = (struct ipt_imq_info*)targinfo;

	if (strcmp(tablename, "mangle") != 0) {
		printk(KERN_WARNING
		       "IMQ: IMQ can only be called from \"mangle\" table, not \"%s\"\n",
		       tablename);
		return 0;
	}
	
	if (mr->todev > IMQ_MAX_DEVS) {
		printk(KERN_WARNING
		       "IMQ: invalid device specified, highest is %u\n",
		       IMQ_MAX_DEVS);
		return 0;
	}
	
	return 1;
}

static struct ipt_target ipt_imq_reg = {
	{ NULL, NULL},
	"IMQ",
	imq_target,
	imq_checkentry,
	NULL,
	THIS_MODULE
};

static int __init init(void)
{
	if (ipt_register_target(&ipt_imq_reg))
		return -EINVAL;

	return 0;
}

static void __exit fini(void)
{
	ipt_unregister_target(&ipt_imq_reg);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
