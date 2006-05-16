/*
 *  ebt_log
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *
 *  April, 2002
 *
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_log.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/if_arp.h>
#include <linux/spinlock.h>

static spinlock_t ebt_log_lock = SPIN_LOCK_UNLOCKED;

static int ebt_log_check(const char *tablename, unsigned int hookmask,
   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_log_info *info = (struct ebt_log_info *)data;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_log_info)))
		return -EINVAL;
	if (info->bitmask & ~EBT_LOG_MASK)
		return -EINVAL;
	if (info->loglevel >= 8)
		return -EINVAL;
	info->prefix[EBT_LOG_PREFIX_SIZE - 1] = '\0';
	return 0;
}

struct tcpudphdr
{
	uint16_t src;
	uint16_t dst;
};

struct arppayload
{
	unsigned char mac_src[ETH_ALEN];
	unsigned char ip_src[4];
	unsigned char mac_dst[ETH_ALEN];
	unsigned char ip_dst[4];
};

static void print_MAC(unsigned char *p)
{
	int i;

	for (i = 0; i < ETH_ALEN; i++, p++)
		printk("%02x%c", *p, i == ETH_ALEN - 1 ? ' ':':');
}

#define myNIPQUAD(a) a[0], a[1], a[2], a[3]
static void ebt_log(const struct sk_buff *skb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
	struct ebt_log_info *info = (struct ebt_log_info *)data;
	char level_string[4] = "< >";
	level_string[1] = '0' + info->loglevel;

	spin_lock_bh(&ebt_log_lock);
	printk(level_string);
	printk("%s IN=%s OUT=%s ", info->prefix, in ? in->name : "",
	   out ? out->name : "");

	printk("MAC source = ");
	print_MAC((skb->mac.ethernet)->h_source);
	printk("MAC dest = ");
	print_MAC((skb->mac.ethernet)->h_dest);

	printk("proto = 0x%04x", ntohs(((*skb).mac.ethernet)->h_proto));

	if ((info->bitmask & EBT_LOG_IP) && skb->mac.ethernet->h_proto ==
	   htons(ETH_P_IP)){
		struct iphdr *iph = skb->nh.iph;
		printk(" IP SRC=%u.%u.%u.%u IP DST=%u.%u.%u.%u,",
		   NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
		printk(" IP tos=0x%02X, IP proto=%d", iph->tos, iph->protocol);
		if (iph->protocol == IPPROTO_TCP ||
		    iph->protocol == IPPROTO_UDP) {
			struct tcpudphdr *ports = (struct tcpudphdr *)(skb->data + iph->ihl*4);

			if (skb->data + iph->ihl*4 > skb->tail) {
				printk(" INCOMPLETE TCP/UDP header");
				goto out;
			}
			printk(" SPT=%u DPT=%u", ntohs(ports->src),
			   ntohs(ports->dst));
		}
		goto out;
	}

	if ((info->bitmask & EBT_LOG_ARP) &&
	    ((skb->mac.ethernet->h_proto == __constant_htons(ETH_P_ARP)) ||
	    (skb->mac.ethernet->h_proto == __constant_htons(ETH_P_RARP)))) {
		struct arphdr * arph = skb->nh.arph;
		printk(" ARP HTYPE=%d, PTYPE=0x%04x, OPCODE=%d",
		   ntohs(arph->ar_hrd), ntohs(arph->ar_pro),
		   ntohs(arph->ar_op));
		/* If it's for Ethernet and the lengths are OK,
		 * then log the ARP payload */
		if (arph->ar_hrd == __constant_htons(1) &&
		    arph->ar_hln == ETH_ALEN &&
		    arph->ar_pln == sizeof(uint32_t)) {
			struct arppayload *arpp = (struct arppayload *)(skb->data + sizeof(*arph));

			if (skb->data + sizeof(*arph) > skb->tail) {
				printk(" INCOMPLETE ARP header");
				goto out;
			}

			printk(" ARP MAC SRC=");
			print_MAC(arpp->mac_src);
			printk(" ARP IP SRC=%u.%u.%u.%u",
			       myNIPQUAD(arpp->ip_src));
			printk(" ARP MAC DST=");
			print_MAC(arpp->mac_dst);
			printk(" ARP IP DST=%u.%u.%u.%u",
			       myNIPQUAD(arpp->ip_dst));
		}

	}
out:
	printk("\n");
	spin_unlock_bh(&ebt_log_lock);
}

static struct ebt_watcher log =
{
	{NULL, NULL}, EBT_LOG_WATCHER, ebt_log, ebt_log_check, NULL,
	THIS_MODULE
};

static int __init init(void)
{
	return ebt_register_watcher(&log);
}

static void __exit fini(void)
{
	ebt_unregister_watcher(&log);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
