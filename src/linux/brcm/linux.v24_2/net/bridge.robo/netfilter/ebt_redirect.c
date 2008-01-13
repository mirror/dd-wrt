/*
 *  ebt_redirect
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *
 *  April, 2002
 *
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_redirect.h>
#include <linux/module.h>
#include <net/sock.h>
#include "../br_private.h"

static int ebt_target_redirect(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
	struct ebt_redirect_info *info = (struct ebt_redirect_info *)data;

	if (hooknr != NF_BR_BROUTING)
		memcpy((**pskb).mac.ethernet->h_dest,
		   in->br_port->br->dev.dev_addr, ETH_ALEN);
	else {
		memcpy((**pskb).mac.ethernet->h_dest,
		   in->dev_addr, ETH_ALEN);
		(*pskb)->pkt_type = PACKET_HOST;
	}
	return info->target;
}

static int ebt_target_redirect_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_redirect_info *info = (struct ebt_redirect_info *)data;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_redirect_info)))
		return -EINVAL;
	if (BASE_CHAIN && info->target == EBT_RETURN)
		return -EINVAL;
	CLEAR_BASE_CHAIN_BIT;
	if ( (strcmp(tablename, "nat") || hookmask & ~(1 << NF_BR_PRE_ROUTING)) &&
	     (strcmp(tablename, "broute") || hookmask & ~(1 << NF_BR_BROUTING)) )
		return -EINVAL;
	if (INVALID_TARGET)
		return -EINVAL;
	return 0;
}

static struct ebt_target redirect_target =
{
	{NULL, NULL}, EBT_REDIRECT_TARGET, ebt_target_redirect,
	ebt_target_redirect_check, NULL, THIS_MODULE
};

static int __init init(void)
{
	return ebt_register_target(&redirect_target);
}

static void __exit fini(void)
{
	ebt_unregister_target(&redirect_target);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
