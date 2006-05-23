/*
 *  ebt_snat
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *
 *  June, 2002
 *
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_nat.h>
#include <linux/module.h>

static int ebt_target_snat(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
	struct ebt_nat_info *info = (struct ebt_nat_info *) data;

	memcpy(((**pskb).mac.ethernet)->h_source, info->mac,
	   ETH_ALEN * sizeof(unsigned char));
	return info->target;
}

static int ebt_target_snat_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_nat_info *info = (struct ebt_nat_info *) data;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_nat_info)))
		return -EINVAL;
	if (BASE_CHAIN && info->target == EBT_RETURN)
		return -EINVAL;
	CLEAR_BASE_CHAIN_BIT;
	if (strcmp(tablename, "nat"))
		return -EINVAL;
	if (hookmask & ~(1 << NF_BR_POST_ROUTING))
		return -EINVAL;
	if (INVALID_TARGET)
		return -EINVAL;
	return 0;
}

static struct ebt_target snat =
{
	{NULL, NULL}, EBT_SNAT_TARGET, ebt_target_snat, ebt_target_snat_check,
	NULL, THIS_MODULE
};

static int __init init(void)
{
	return ebt_register_target(&snat);
}

static void __exit fini(void)
{
	ebt_unregister_target(&snat);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
