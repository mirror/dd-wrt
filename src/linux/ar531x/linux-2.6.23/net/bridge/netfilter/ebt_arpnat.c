/*
 *  ebt_arpnat
 *
 *	Authors:
 *      Kestutis Barkauskas <gpl@wilibox.com>
 *
 *  November, 2005
 *
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_nat.h>
#include <linux/module.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/rtnetlink.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/inetdevice.h>
#include <net/arp.h>
#include <net/ip.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <net/checksum.h>

#include "../br_private.h"

#define STRMAC "%02x:%02x:%02x:%02x:%02x:%02x"
#define STRIP "%d.%d.%d.%d"
#define MAC2STR(x) (x)[0],(x)[1],(x)[2],(x)[3],(x)[4],(x)[5]
#define IP2STR(x) (x)>>24&0xff,(x)>>16&0xff,(x)>>8&0xff,(x)&0xff

#define FLUSHTIMEOUT (60*10) /* 10 minutes to expire */
#define GIADDR_OFFSET (24)

#ifdef DEBUG
#define __STATIC
static uint8_t debug = 1;
#else
#define __STATIC static
static uint8_t debug = 0;
#endif

#ifndef __packed
#define __packed __attribute__((__packed__))
#endif

struct arpnat_dat {
    uint32_t ip;
    uint32_t expires;
    uint8_t mac[ETH_ALEN];
} __packed;

struct mac2ip {
    struct hlist_node node;
    struct arpnat_dat data;
};

static HLIST_HEAD(arpnat_table);
static spinlock_t arpnat_lock = SPIN_LOCK_UNLOCKED;
static uint8_t bootpnat = 1;
static uint32_t expires = FLUSHTIMEOUT;

__STATIC struct mac2ip* find_mac_nat(struct hlist_head* head, const uint8_t* mac)
{
    struct mac2ip* tpos;
    struct mac2ip* result = NULL;
    struct hlist_node* pos;
    struct hlist_node* n;
    hlist_for_each_entry_safe(tpos, pos, n, head, node)
    {
	if (memcmp(tpos->data.mac, mac, ETH_ALEN) == 0)
	{
            result = tpos;
	    break;
	}
	if (tpos->data.expires < jiffies)
	{
	    hlist_del(pos);
	    kfree(tpos);
	}
    }
    return result;
}

__STATIC struct mac2ip* find_ip_nat(struct hlist_head* head, uint32_t ip)
{
    struct mac2ip* tpos;
    struct mac2ip* result = NULL;
    struct hlist_node* pos;
    struct hlist_node* n;

    hlist_for_each_entry_safe(tpos, pos, n, head, node)
    {
	if (tpos->data.ip == ip)
	{
            result = tpos;
	    break;
	}
	if (tpos->data.expires < jiffies)
	{
	    hlist_del(pos);
	    kfree(tpos);
	}
    }
    return result;
}

__STATIC void free_arp_nat(struct hlist_head* head)
{
    struct mac2ip* tpos;
    struct hlist_node* pos;
    struct hlist_node* n;
    hlist_for_each_entry_safe(tpos, pos, n, head, node)
    {
        hlist_del(pos);
	kfree(tpos);
    }
}

__STATIC struct mac2ip* update_arp_nat(struct hlist_head* head, const uint8_t* mac, uint32_t ip)
{
    struct mac2ip* entry;

    entry = find_mac_nat(head, mac);
    if (entry)
	goto done;

    entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
    if (!entry)
	return NULL;
    INIT_HLIST_NODE(&entry->node);
    hlist_add_head(&entry->node, head);
done:
    memcpy(entry->data.mac, mac, ETH_ALEN);
    entry->data.ip = ip;
    entry->data.expires = jiffies + expires * HZ;

    return entry;
}

#ifdef CONFIG_PROC_FS
__STATIC int arpnat_cache_stat_get_info(char *buffer, char **start, off_t offset, int length)
{
    	int len = 0;
	struct mac2ip* tpos;
	struct hlist_node* pos;
	struct hlist_node* n;
	unsigned long flags;
        uint32_t exp;

	spin_lock_irqsave(&arpnat_lock, flags);
	hlist_for_each_entry_safe(tpos, pos, n, &arpnat_table, node)
	{
	    if (tpos->data.expires < jiffies)
	    {
		hlist_del(pos);
		kfree(tpos);
                continue;
	    }
	    exp = tpos->data.expires - jiffies;
	    len += sprintf(buffer + len, STRMAC" %8u.%02u "STRIP"\n", MAC2STR(tpos->data.mac), exp / HZ, exp % HZ, IP2STR(tpos->data.ip));
	}
        spin_unlock_irqrestore(&arpnat_lock, flags);
	len -= offset;
	if (len > length)
		len = length;
	if (len < 0)
		len = 0;
	*start = buffer + offset;
  	return len;
}

__STATIC int arpnat_write(struct file *file, const char *buffer,
			   unsigned long count, void *data)
{
    	/** arpnat entry expiration time in seconds
	 *  debug enabled/disabled
	 *  bootpnat enabled/disabled
	 **/
    	char buf[80];
	int d, b;

    	if (copy_from_user(buf, buffer, count < sizeof(buf) ? count : sizeof(buf)))
		return -EFAULT;
        if (sscanf(buf, "%u %d %d", &expires, &d, &b) != 3)
		return -EINVAL;
	debug = d;
	bootpnat = b;

  	return count;
}

__STATIC int arpnat_get_info(char *buffer, char **start, off_t offset, int length)
{
    	int len = 0;
	len += sprintf(buffer + len, "ARPNAT Expiration: %u\nDebug: %d\nBOOTPNAT: %d\n", expires, debug, bootpnat);
	len -= offset;
	if (len > length)
		len = length;
	if (len < 0)
		len = 0;
	*start = buffer + offset;
  	return len;
}
#endif


__STATIC int ebt_target_arpnat(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)
{
    struct arphdr *ah = NULL;
    struct arphdr _arph;
    //used for target only
    struct ebt_nat_info *info = (struct ebt_nat_info *) data;
    uint8_t* eth_smac = eth_hdr(*pskb)->h_source;
    uint8_t* eth_dmac = eth_hdr(*pskb)->h_dest;
    uint32_t* arp_sip = NULL;
    uint8_t* arp_smac = NULL;
    uint32_t* arp_dip = NULL;
    uint8_t* arp_dmac = NULL;
    struct mac2ip* entry = NULL;
    unsigned long flags;

    if (eth_hdr(*pskb)->h_proto == __constant_htons(ETH_P_ARP))
    {
	ah = skb_header_pointer(*pskb, 0, sizeof(_arph), &_arph);
	if (ah->ar_hln == ETH_ALEN && ah->ar_pro == htons(ETH_P_IP) &&
	    ah->ar_pln == 4)
	{
	    unsigned char *raw = skb_network_header(*pskb);
	    arp_sip = (uint32_t*)(raw + sizeof(struct arphdr) + (arp_hdr(*pskb)->ar_hln));
	    arp_smac = raw + sizeof(struct arphdr);
	    arp_dip = (uint32_t*)(raw + sizeof(struct arphdr) + (2*(arp_hdr(*pskb)->ar_hln)) + arp_hdr(*pskb)->ar_pln);
	    arp_dmac = raw + sizeof(struct arphdr) + arp_hdr(*pskb)->ar_hln + arp_hdr(*pskb)->ar_pln;
	}
	else
	{
            ah = NULL;
	}
    }
    if (in)
    {
	if (ah)
	{
	    spin_lock_irqsave(&arpnat_lock, flags);
	    entry = find_ip_nat(&arpnat_table, *arp_dip);
	    switch (ah->ar_op)
	    {
	    case __constant_htons(ARPOP_REPLY):
	    case __constant_htons(ARPOP_REQUEST):
		if (entry)
		{
		    uint32_t dip = *arp_dip;
		    uint32_t sip = inet_select_addr(in->br_port->br->dev, dip, RT_SCOPE_LINK);
		    if (! (eth_dmac[0] & 1))
		    {
			if (debug)
			    printk("IN ARPNAT: "STRMAC" -> "STRMAC"\n", MAC2STR(eth_dmac), MAC2STR(entry->data.mac));
			memcpy(arp_dmac, entry->data.mac, ETH_ALEN);
			memcpy(eth_dmac, entry->data.mac, ETH_ALEN);
			(*pskb)->pkt_type = (dip != sip) ? PACKET_OTHERHOST : (*pskb)->pkt_type;
		    }
		    spin_unlock_irqrestore(&arpnat_lock, flags);
		    /*if (dip != sip)
		    {
                        if (debug)
			    printk("SEND ARP REQUEST: "STRIP" -> "STRIP"\n", IP2STR(sip), IP2STR(dip));
			arp_send(ARPOP_REQUEST, ETH_P_ARP, dip, &in->br_port->br->dev, sip, NULL, in->br_port->br->dev.dev_addr, NULL);
		    }*/
		    return info->target;
		}
		break;
	    }
            spin_unlock_irqrestore(&arpnat_lock, flags);
	}                                                                     
	else if (eth_hdr(*pskb)->h_proto == __constant_htons(ETH_P_IP))
	{
	    struct iphdr *iph = ip_hdr(*pskb);
	    struct udphdr *uh = NULL;
	    if (bootpnat && iph->protocol == htons(IPPROTO_UDP) && !(iph->frag_off & htons(IP_OFFSET)))
	    {
		uh = (struct udphdr*)((u_int32_t *)iph + iph->ihl);
		if (uh->dest == htonl(67))
		{
		    //do something illegal for BOOTP
		    uint32_t* giaddrp = (uint32_t*)(((uint8_t*)uh) + sizeof(*uh) + GIADDR_OFFSET);
		    uint8_t* mac = (uint8_t*)(giaddrp + 1);
		    uint32_t ihl = iph->ihl << 2;
		    uint32_t size = (*pskb)->len - ihl;
                    uint32_t orig_daddr = iph->daddr;

		    spin_lock_irqsave(&arpnat_lock, flags);
		    entry = find_mac_nat(&arpnat_table, mac);
		    if (entry)
			iph->daddr = entry->data.ip;
		    else
			iph->daddr = 0xffffffff;
		    spin_unlock_irqrestore(&arpnat_lock, flags);
                    if (debug)
			printk("IN BOOTPRELAY: "STRMAC"["STRIP"] -> "STRMAC"["STRIP"]\n",
			       MAC2STR(eth_dmac), IP2STR(orig_daddr), MAC2STR(mac), IP2STR(iph->daddr));
                    memcpy(eth_dmac, mac, ETH_ALEN);
		    *giaddrp = 0;
                    uh->dest = htonl(68);
                    iph->check = 0;
		    uh->check = 0;
                    iph->check = ip_fast_csum((uint8_t*)iph, iph->ihl);
		    (*pskb)->csum = csum_partial((uint8_t*)iph + ihl, size, 0);
		    uh->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
						  size, iph->protocol,
						  (*pskb)->csum);
		    if (uh->check == 0)
			uh->check = 0xFFFF;
		    return info->target;
		}
		else
                    goto HANDLE_IP_PKT;
	    }
	    else
	    {
	    HANDLE_IP_PKT:
		spin_lock_irqsave(&arpnat_lock, flags);
		entry = find_ip_nat(&arpnat_table, iph->daddr);
		if (entry)
		{
                    //to me
		    if (inet_confirm_addr(in->br_port->br->dev, 0, entry->data.ip, RT_SCOPE_HOST))
		    {
			if (debug)
			    printk("IP PKT TO ME: "STRMAC"["STRIP"] -> "STRMAC"[type: %d]\n",
				   MAC2STR(eth_dmac), IP2STR(iph->daddr), MAC2STR(in->br_port->br->dev->dev_addr), (*pskb)->pkt_type);
			memcpy(eth_dmac, in->br_port->br->dev->dev_addr, ETH_ALEN);
		    }
		    else
		    {
			if (debug)
			    printk("IP PKT TO OTHER: "STRMAC"["STRIP"] -> "STRMAC"[type: %d]\n",
				   MAC2STR(eth_dmac), IP2STR(iph->daddr), MAC2STR(entry->data.mac), (*pskb)->pkt_type);
			memcpy(eth_dmac, entry->data.mac, ETH_ALEN);
			(*pskb)->pkt_type = PACKET_OTHERHOST;
		    }
		    spin_unlock_irqrestore(&arpnat_lock, flags);
		    return info->target;
		}
		spin_unlock_irqrestore(&arpnat_lock, flags);
	    }
	}
	if (! (eth_dmac[0] & 1))
	{
	    if (memcmp(in->br_port->br->dev->dev_addr, eth_dmac, ETH_ALEN) &&
		memcmp(in->dev_addr, eth_dmac, ETH_ALEN))
                return EBT_DROP;
	    spin_lock_irqsave(&arpnat_lock, flags);
	    entry = find_mac_nat(&arpnat_table, eth_dmac);
	    if (entry)
		memcpy(eth_dmac, entry->data.mac, ETH_ALEN);
	    else
                memcpy(eth_dmac, in->br_port->br->dev->dev_addr, ETH_ALEN);
	    spin_unlock_irqrestore(&arpnat_lock, flags);
	}
    }
    else if (out)
    {
	if (ah)
	{
	    switch (ah->ar_op)
	    {
	    case __constant_htons(ARPOP_REQUEST):
	    case __constant_htons(ARPOP_REPLY):
                spin_lock_irqsave(&arpnat_lock, flags);
		update_arp_nat(&arpnat_table, arp_smac, *arp_sip);
                spin_unlock_irqrestore(&arpnat_lock, flags);
		/* do BR ip lookup */
		if (inet_confirm_addr(out->br_port->br->dev, 0, *arp_dip, RT_SCOPE_HOST))
		{
                    return info->target;
		}
                *pskb = skb_unshare(*pskb, GFP_ATOMIC);
		eth_smac = eth_hdr(*pskb)->h_source;
		arp_smac = skb_network_header(*pskb) + sizeof(struct arphdr);
                if (debug)
		    printk("OUT ARPNAT: "STRMAC" -> "STRMAC"\n", MAC2STR(eth_smac), MAC2STR(out->dev_addr));
		memcpy(arp_smac, out->dev_addr, ETH_ALEN);
		memcpy(eth_smac, out->dev_addr, ETH_ALEN);
                return info->target;
		break;
	    }
	}
	else if (bootpnat && eth_hdr(*pskb)->h_proto == __constant_htons(ETH_P_IP) &&
		memcmp(out->br_port->br->dev->dev_addr, eth_smac, ETH_ALEN))
	{
	    struct iphdr *iph = ip_hdr(*pskb);
	    struct udphdr *uh = NULL;
	    if (iph->protocol == htons(IPPROTO_UDP) && !(iph->frag_off & htons(IP_OFFSET)))
	    {
		uh = (struct udphdr*)((u_int32_t *)iph + iph->ihl);
		if (uh->dest == htonl(67))
		{
		    //do something illegal for BOOTP
		    uint32_t giaddr = inet_select_addr(out->br_port->br->dev, iph->daddr, RT_SCOPE_LINK);
		    uint32_t* giaddrp = (uint32_t*)(((uint8_t*)uh) + sizeof(*uh) + GIADDR_OFFSET);
		    uint32_t ihl = iph->ihl << 2;
		    uint32_t size = (*pskb)->len - ihl;
                    if (debug)
			printk("OUT BOOTPRELAY: "STRIP" -> "STRIP"\n",
			       IP2STR(*giaddrp), IP2STR(giaddr));
		    *giaddrp = giaddr;
		    uh->check = 0;
		    (*pskb)->csum = csum_partial((uint8_t*)iph + ihl, size, 0);
		    uh->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
						    size, iph->protocol,
						    (*pskb)->csum);
		    if (uh->check == 0)
			uh->check = 0xFFFF;
		}
	    }
	}
	memcpy(eth_smac, out->dev_addr, ETH_ALEN);
    }
    return info->target;
}

static int ebt_target_nat_arpcheck(const char *tablename, unsigned int hookmask,
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
	if (hookmask & ~(1 << NF_BR_PRE_ROUTING) &&
	    hookmask & ~(1 << NF_BR_POST_ROUTING))
		return -EINVAL;
	if (INVALID_TARGET)
		return -EINVAL;
	return 0;
}

static struct ebt_target arpnat =
{
	.name	= 	EBT_ARPNAT_TARGET, 
	.target	=	ebt_target_arpnat, 
	.check	= 	ebt_target_nat_arpcheck,
	.me	=	THIS_MODULE
};

static int __init init(void)
{
#ifdef CONFIG_PROC_FS
    	struct proc_dir_entry *proc_arpnat = proc_net_create("arpnat", 0, arpnat_get_info);
	if (proc_arpnat)
                proc_arpnat->write_proc = arpnat_write;
    	proc_net_create("arpnat_cache", 0, arpnat_cache_stat_get_info);
#endif
	return ebt_register_target(&arpnat);
}

static void __exit fini(void)
{
    ebt_unregister_target(&arpnat);
#ifdef CONFIG_PROC_FS
    proc_net_remove("arpnat");
    proc_net_remove("arpnat_cache");
#endif
    free_arp_nat(&arpnat_table);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
