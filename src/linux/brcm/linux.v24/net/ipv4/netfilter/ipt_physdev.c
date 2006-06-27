/* Kernel module to match the bridge port in and
 * out device for IP packets coming into contact with a bridge. */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ipt_physdev.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_bridge.h>
#include <linux/netdevice.h>
#define MATCH   1
#define NOMATCH 0

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      const void *hdr,
      u_int16_t datalen,
      int *hotdrop)
{
	int i;
	static const char nulldevname[IFNAMSIZ] = { 0 };
	const struct ipt_physdev_info *info = matchinfo;
	unsigned long ret;
	const char *indev, *outdev;
	struct nf_bridge_info *nf_bridge;

	/* Not a bridged IP packet or no info available yet:
	 * LOCAL_OUT/mangle and LOCAL_OUT/nat don't know if
	 * the destination device will be a bridge. */
	if (!(nf_bridge = skb->nf_bridge)) {
		/* Return MATCH if the invert flags of the used options are on */
		if ((info->bitmask & IPT_PHYSDEV_OP_BRIDGED) &&
		    !(info->invert & IPT_PHYSDEV_OP_BRIDGED))
			return NOMATCH;
		if ((info->bitmask & IPT_PHYSDEV_OP_ISIN) &&
		    !(info->invert & IPT_PHYSDEV_OP_ISIN))
			return NOMATCH;
		if ((info->bitmask & IPT_PHYSDEV_OP_ISOUT) &&
		    !(info->invert & IPT_PHYSDEV_OP_ISOUT))
			return NOMATCH;
		if ((info->bitmask & IPT_PHYSDEV_OP_IN) &&
		    !(info->invert & IPT_PHYSDEV_OP_IN))
			return NOMATCH;
		if ((info->bitmask & IPT_PHYSDEV_OP_OUT) &&
		    !(info->invert & IPT_PHYSDEV_OP_OUT))
			return NOMATCH;
		return MATCH;
	}

	/* This only makes sense in the FORWARD and POSTROUTING chains */
	if ((info->bitmask & IPT_PHYSDEV_OP_BRIDGED) &&
	    (!!(nf_bridge->mask & BRNF_BRIDGED) ^
	    !(info->invert & IPT_PHYSDEV_OP_BRIDGED)))
		return NOMATCH;

	if ((info->bitmask & IPT_PHYSDEV_OP_ISIN &&
	    (!nf_bridge->physindev ^ !!(info->invert & IPT_PHYSDEV_OP_ISIN))) ||
	    (info->bitmask & IPT_PHYSDEV_OP_ISOUT &&
	    (!nf_bridge->physoutdev ^ !!(info->invert & IPT_PHYSDEV_OP_ISOUT))))
		return NOMATCH;

	if (!(info->bitmask & IPT_PHYSDEV_OP_IN))
		goto match_outdev;
	indev = nf_bridge->physindev ? nf_bridge->physindev->name : nulldevname;
	for (i = 0, ret = 0; i < IFNAMSIZ/sizeof(unsigned long); i++) {
		ret |= (((const unsigned long *)indev)[i]
			^ ((const unsigned long *)info->physindev)[i])
			& ((const unsigned long *)info->in_mask)[i];
	}

	if ((ret == 0) ^ !(info->invert & IPT_PHYSDEV_OP_IN))
		return NOMATCH;

match_outdev:
	if (!(info->bitmask & IPT_PHYSDEV_OP_OUT))
		return MATCH;
	outdev = nf_bridge->physoutdev ?
		 nf_bridge->physoutdev->name : nulldevname;
	for (i = 0, ret = 0; i < IFNAMSIZ/sizeof(unsigned long); i++) {
		ret |= (((const unsigned long *)outdev)[i]
			^ ((const unsigned long *)info->physoutdev)[i])
			& ((const unsigned long *)info->out_mask)[i];
	}

	return (ret != 0) ^ !(info->invert & IPT_PHYSDEV_OP_OUT);
}

static int
checkentry(const char *tablename,
		       const struct ipt_ip *ip,
		       void *matchinfo,
		       unsigned int matchsize,
		       unsigned int hook_mask)
{
	const struct ipt_physdev_info *info = matchinfo;

	if (matchsize != IPT_ALIGN(sizeof(struct ipt_physdev_info)))
		return 0;
	if (!(info->bitmask & IPT_PHYSDEV_OP_MASK) ||
	    info->bitmask & ~IPT_PHYSDEV_OP_MASK)
		return 0;
	return 1;
}

static struct ipt_match physdev_match = {
	.name		= "physdev",
	.match		= &match,
	.checkentry	= &checkentry,
	.me		= THIS_MODULE,
};

static int __init init(void)
{
	return ipt_register_match(&physdev_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&physdev_match);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
EXPORT_NO_SYMBOLS;
