/*
 *  ebt_ip
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *
 *  April, 2002
 *
 *  Changes:
 *    added ip-sport and ip-dport
 *    Innominate Security Technologies AG <mhopf@innominate.com>
 *    September, 2002
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_ip.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/module.h>

struct tcpudphdr {
	uint16_t src;
	uint16_t dst;
};

union h_u {
	unsigned char *raw;
	struct tcpudphdr *tuh;
};

static int ebt_filter_ip(const struct sk_buff *skb, const struct net_device *in,
   const struct net_device *out, const void *data,
   unsigned int datalen)
{
	struct ebt_ip_info *info = (struct ebt_ip_info *)data;

	if (info->bitmask & EBT_IP_TOS &&
	   FWINV(info->tos != ((*skb).nh.iph)->tos, EBT_IP_TOS))
		return EBT_NOMATCH;
	if (info->bitmask & EBT_IP_PROTO) {
		if (FWINV(info->protocol != ((*skb).nh.iph)->protocol,
		          EBT_IP_PROTO))
			return EBT_NOMATCH;
		if ( info->protocol == IPPROTO_TCP ||
		     info->protocol == IPPROTO_UDP )
		{
			union h_u h;
			h.raw = skb->data + skb->nh.iph->ihl*4;
			if (info->bitmask & EBT_IP_DPORT) {
				uint16_t port = ntohs(h.tuh->dst);
				if (FWINV(port < info->dport[0] ||
				          port > info->dport[1],
				          EBT_IP_DPORT))
				return EBT_NOMATCH;
			}
			if (info->bitmask & EBT_IP_SPORT) {
				uint16_t port = ntohs(h.tuh->src);
				if (FWINV(port < info->sport[0] ||
				          port > info->sport[1],
				          EBT_IP_SPORT))
				return EBT_NOMATCH;
			}
		}
	}
	if (info->bitmask & EBT_IP_SOURCE &&
	   FWINV((((*skb).nh.iph)->saddr & info->smsk) !=
	   info->saddr, EBT_IP_SOURCE))
		return EBT_NOMATCH;
	if ((info->bitmask & EBT_IP_DEST) &&
	   FWINV((((*skb).nh.iph)->daddr & info->dmsk) !=
	   info->daddr, EBT_IP_DEST))
		return EBT_NOMATCH;
	return EBT_MATCH;
}

static int ebt_ip_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_ip_info *info = (struct ebt_ip_info *)data;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_ip_info)))
		return -EINVAL;
	if (e->ethproto != __constant_htons(ETH_P_IP) ||
	   e->invflags & EBT_IPROTO)
		return -EINVAL;
	if (info->bitmask & ~EBT_IP_MASK || info->invflags & ~EBT_IP_MASK)
		return -EINVAL;
	if (info->bitmask & (EBT_IP_DPORT | EBT_IP_SPORT)) {
		if (!info->bitmask & EBT_IPROTO)
			return -EINVAL;
		if (info->protocol != IPPROTO_TCP &&
		    info->protocol != IPPROTO_UDP)
			 return -EINVAL;
	}
	if (info->bitmask & EBT_IP_DPORT && info->dport[0] > info->dport[1])
		return -EINVAL;
	if (info->bitmask & EBT_IP_SPORT && info->sport[0] > info->sport[1])
		return -EINVAL;
	return 0;
}

static struct ebt_match filter_ip =
{
	{NULL, NULL}, EBT_IP_MATCH, ebt_filter_ip, ebt_ip_check, NULL,
	THIS_MODULE
};

static int __init init(void)
{
	return ebt_register_match(&filter_ip);
}

static void __exit fini(void)
{
	ebt_unregister_match(&filter_ip);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
