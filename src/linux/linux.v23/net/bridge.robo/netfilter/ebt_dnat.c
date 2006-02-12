/*
 *  ebt_dnat
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
#include <net/sock.h>

static int ebt_target_dnat(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
	struct ebt_nat_info *info = (struct ebt_nat_info *)data;

	memcpy(((**pskb).mac.ethernet)->h_dest, info->mac,
	   ETH_ALEN * sizeof(unsigned char));
	return info->target;
}

static int ebt_target_dnat_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_nat_info *info = (struct ebt_nat_info *)data;

	if (BASE_CHAIN && info->target == EBT_RETURN)
		return -EINVAL;
	CLEAR_BASE_CHAIN_BIT;
	if ( (strcmp(tablename, "nat") ||
	   (hookmask & ~((1 << NF_BR_PRE_ROUTING) | (1 << NF_BR_LOCAL_OUT)))) &&
	   (strcmp(tablename, "broute") || hookmask & ~(1 << NF_BR_BROUTING)) )
		return -EINVAL;
	if (datalen != EBT_ALIGN(sizeof(struct ebt_nat_info)))
		return -EINVAL;
	if (INVALID_TARGET)
		return -EINVAL;
	return 0;
}

static struct ebt_target dnat =
{
	{NULL, NULL}, EBT_DNAT_TARGET, ebt_target_dnat, ebt_target_dnat_check,
	NULL, THIS_MODULE
};

static int __init init(void)
{
	return ebt_register_target(&dnat);
}

static void __exit fini(void)
{
	ebt_unregister_target(&dnat);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
