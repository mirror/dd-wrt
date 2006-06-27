/*
 *  ebt_arp
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *	Tim Gardner <timg@tpi.com>
 *
 *  April, 2002
 *
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_arp.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/module.h>

static int ebt_filter_arp(const struct sk_buff *skb, const struct net_device *in,
   const struct net_device *out, const void *data, unsigned int datalen)
{
	struct ebt_arp_info *info = (struct ebt_arp_info *)data;

	if (info->bitmask & EBT_ARP_OPCODE && FWINV(info->opcode !=
	   ((*skb).nh.arph)->ar_op, EBT_ARP_OPCODE))
		return EBT_NOMATCH;
	if (info->bitmask & EBT_ARP_HTYPE && FWINV(info->htype !=
	   ((*skb).nh.arph)->ar_hrd, EBT_ARP_HTYPE))
		return EBT_NOMATCH;
	if (info->bitmask & EBT_ARP_PTYPE && FWINV(info->ptype !=
	   ((*skb).nh.arph)->ar_pro, EBT_ARP_PTYPE))
		return EBT_NOMATCH;

	if (info->bitmask & (EBT_ARP_SRC_IP | EBT_ARP_DST_IP))
	{
		uint32_t arp_len = sizeof(struct arphdr) +
		   (2 * (((*skb).nh.arph)->ar_hln)) +
		   (2 * (((*skb).nh.arph)->ar_pln));
		uint32_t dst;
		uint32_t src;

		// Make sure the packet is long enough.
		if ((((*skb).nh.raw) + arp_len) > (*skb).tail)
			return EBT_NOMATCH;
		// IPv4 addresses are always 4 bytes.
		if (((*skb).nh.arph)->ar_pln != sizeof(uint32_t))
			return EBT_NOMATCH;

		if (info->bitmask & EBT_ARP_SRC_IP) {
			memcpy(&src, ((*skb).nh.raw) + sizeof(struct arphdr) +
			   ((*skb).nh.arph)->ar_hln, sizeof(uint32_t));
			if (FWINV(info->saddr != (src & info->smsk),
			   EBT_ARP_SRC_IP))
				return EBT_NOMATCH;
		}

		if (info->bitmask & EBT_ARP_DST_IP) {
			memcpy(&dst, ((*skb).nh.raw)+sizeof(struct arphdr) +
			   (2*(((*skb).nh.arph)->ar_hln)) +
			   (((*skb).nh.arph)->ar_pln), sizeof(uint32_t));
			if (FWINV(info->daddr != (dst & info->dmsk),
			   EBT_ARP_DST_IP))
				return EBT_NOMATCH;
		}
	}

	if (info->bitmask & (EBT_ARP_SRC_MAC | EBT_ARP_DST_MAC))
	{
		uint32_t arp_len = sizeof(struct arphdr) +
		   (2 * (((*skb).nh.arph)->ar_hln)) +
		   (2 * (((*skb).nh.arph)->ar_pln));
		unsigned char dst[ETH_ALEN];
		unsigned char src[ETH_ALEN];

		// Make sure the packet is long enough.
		if ((((*skb).nh.raw) + arp_len) > (*skb).tail)
			return EBT_NOMATCH;
		// MAC addresses are 6 bytes.
		if (((*skb).nh.arph)->ar_hln != ETH_ALEN)
			return EBT_NOMATCH;
		if (info->bitmask & EBT_ARP_SRC_MAC) {
			uint8_t verdict, i;

			memcpy(&src, ((*skb).nh.raw) +
					sizeof(struct arphdr),
					ETH_ALEN);
			verdict = 0;
			for (i = 0; i < 6; i++)
				verdict |= (src[i] ^ info->smaddr[i]) &
				       info->smmsk[i];	
			if (FWINV(verdict != 0, EBT_ARP_SRC_MAC))
				return EBT_NOMATCH;
		}

		if (info->bitmask & EBT_ARP_DST_MAC) { 
			uint8_t verdict, i;

			memcpy(&dst, ((*skb).nh.raw) +
					sizeof(struct arphdr) +
			   		(((*skb).nh.arph)->ar_hln) +
			   		(((*skb).nh.arph)->ar_pln),
					ETH_ALEN);
			verdict = 0;
			for (i = 0; i < 6; i++)
				verdict |= (dst[i] ^ info->dmaddr[i]) &
					info->dmmsk[i];
			if (FWINV(verdict != 0, EBT_ARP_DST_MAC))
				return EBT_NOMATCH;
		}
	}

	return EBT_MATCH;
}

static int ebt_arp_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_arp_info *info = (struct ebt_arp_info *)data;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_arp_info)))
		return -EINVAL;
	if ((e->ethproto != __constant_htons(ETH_P_ARP) &&
	   e->ethproto != __constant_htons(ETH_P_RARP)) ||
	   e->invflags & EBT_IPROTO)
		return -EINVAL;
	if (info->bitmask & ~EBT_ARP_MASK || info->invflags & ~EBT_ARP_MASK)
		return -EINVAL;
	return 0;
}

static struct ebt_match filter_arp =
{
	{NULL, NULL}, EBT_ARP_MATCH, ebt_filter_arp, ebt_arp_check, NULL,
	THIS_MODULE
};

static int __init init(void)
{
	return ebt_register_match(&filter_arp);
}

static void __exit fini(void)
{
	ebt_unregister_match(&filter_arp);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
