/*
 *  ebt_arpnat
 *
 *	Authors:
 *      Kestutis Barkauskas <gpl@wilibox.com>
 *
 *  November, 2005
 *
 *	Rewritten by:
 *      Kestutis Barkauskas and Kestutis Kupciunas <gpl@ubnt.com>
 *
 *  November, 2006
 *
 *      spin_lock_irqsave/restore() changed to spin_lock_bh()/spin_unlock_bh()
 *	Thanks Zilvinas Valinskas <gpl@wilibox.com>
 *
 *  October, 2007
 *      VLAN (802.1Q) support added.
 *	Note: PPPOE is not supported with VLAN yet.
 *      Kestutis Barkauskas <gpl@ubnt.com>
 *
 *  November, 2007
 *      IP conflict issue on ethernet side clients after device reboot fix.
 *	Thanks Zilvinas Valinskas <gpl@wilibox.com>
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_nat.h>
#include <linux/module.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
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

#define FLUSHTIMEOUT (3600*7) /* 7 hours to expire */
#define GIADDR_OFFSET (24)
#define FLAGS_OFFSET (10)

/* DHCP workaround modes */
#define BOOTP_RELAY 2
#define BOOTP_BCAST 1

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

struct arpnat_dat
{
	uint32_t ip;
	uint32_t expires;
	uint8_t mac[ETH_ALEN];
	uint16_t pppoe_id;
} __packed;

struct mac2ip
{
	struct hlist_node node;
	struct arpnat_dat data;
};

static HLIST_HEAD(arpnat_table);
static spinlock_t arpnat_lock = SPIN_LOCK_UNLOCKED;
static uint8_t bootpnat = 1;
static uint8_t pppoenat = 1;
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

__STATIC struct mac2ip* find_pppoe_nat(struct hlist_head* head, uint16_t id)
{
	struct mac2ip* tpos;
	struct mac2ip* result = NULL;
	struct hlist_node* pos;
	struct hlist_node* n;

	hlist_for_each_entry_safe(tpos, pos, n, head, node)
	{
		if (id && (tpos->data.pppoe_id == id))
		{
			result = tpos;
                        tpos->data.expires = jiffies + expires * HZ;
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
	entry->data.pppoe_id = 0;
	done:
	memcpy(entry->data.mac, mac, ETH_ALEN);
	entry->data.ip = ip;
	entry->data.expires = jiffies + expires * HZ;

	return entry;
}

__STATIC struct mac2ip* update_pppoe_nat(struct hlist_head* head, const uint8_t* mac, uint16_t id)
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
	entry->data.ip = 0;
	done:
	memcpy(entry->data.mac, mac, ETH_ALEN);
	entry->data.pppoe_id = id;
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
	uint32_t exp;

	spin_lock_bh(&arpnat_lock);
	hlist_for_each_entry_safe(tpos, pos, n, &arpnat_table, node)
	{
		if (tpos->data.expires < jiffies)
		{
			hlist_del(pos);
			kfree(tpos);
			continue;
		}
		exp = tpos->data.expires - jiffies;
		len += sprintf(buffer + len, STRMAC" %8u.%02u "STRIP" %x\n",
					   MAC2STR(tpos->data.mac), exp / HZ, exp % HZ, IP2STR(tpos->data.ip), tpos->data.pppoe_id);
	}
	spin_unlock_bh(&arpnat_lock);
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
	 *  bootpnat disabled[0]/enable[1]/releay[2]
	 *  pppoenat enabled/disabled
	 **/
	char buf[80];
	int d, b, p;

	if (copy_from_user(buf, buffer, count < sizeof(buf) ? count : sizeof(buf)))
		return -EFAULT;
	if (sscanf(buf, "%u %d %d %d", &expires, &d, &b, &p) != 4)
		return -EINVAL;
	debug = d;
	bootpnat = b;
	pppoenat = p;

	return count;
}

__STATIC int arpnat_get_info(char *buffer, char **start, off_t offset, int length)
{
	int len = 0;
	len += sprintf(buffer + len, "ARPNAT Expiration: %u\nDebug: %d\nBOOTPNAT: %d\nPPPOE: %d\n", expires, debug, bootpnat, pppoenat);
	len -= offset;
	if (len > length)
		len = length;
	if (len < 0)
		len = 0;
	*start = buffer + offset;
	return len;
}
#endif

/**
 * Do ARP NAT on input chain
 **/
__STATIC int do_arp_in(struct sk_buff **pskb, const struct net_device *in, int *target, int vlan)
{
	struct arphdr *ah = (*pskb)->nh.arph;
	uint8_t* eth_dmac = ((*pskb)->mac.ethernet)->h_dest;
	uint32_t* arp_sip;
	uint32_t* arp_dip;
	uint8_t* arp_dmac;
	struct mac2ip* entry;
	uint8_t* nh_raw = (*pskb)->nh.raw;

	ah = (struct arphdr*)((uint8_t*)ah + vlan);
	nh_raw += vlan;


	if (ah->ar_hln == ETH_ALEN && ah->ar_pro == htons(ETH_P_IP) &&
		ah->ar_pln == 4)
	{
		arp_sip = (uint32_t*)(nh_raw + sizeof(struct arphdr) + ah->ar_hln);
		arp_dip = (uint32_t*)(nh_raw + sizeof(struct arphdr) + (2* ah->ar_hln) + ah->ar_pln);
		arp_dmac = nh_raw + sizeof(struct arphdr) + ah->ar_hln + ah->ar_pln;
	}
	else
		return 1;

	spin_lock_bh(&arpnat_lock);
	entry = find_ip_nat(&arpnat_table, *arp_dip);
	switch (ah->ar_op)
	{
	case __constant_htons(ARPOP_REPLY):
	case __constant_htons(ARPOP_REQUEST):
		if (entry)
		{
			uint32_t dip = *arp_dip;
			uint32_t sip = inet_select_addr(&in->br_port->br->dev, dip, RT_SCOPE_LINK);
			if (! (eth_dmac[0] & 1))
			{
				if (debug)
					printk("IN ARPNAT: "STRMAC" -> "STRMAC"\n", MAC2STR(eth_dmac), MAC2STR(entry->data.mac));
				memcpy(arp_dmac, entry->data.mac, ETH_ALEN);
				memcpy(eth_dmac, entry->data.mac, ETH_ALEN);
				(*pskb)->pkt_type = (dip != sip) ? PACKET_OTHERHOST : (*pskb)->pkt_type;
			}
			spin_unlock_bh(&arpnat_lock);
			/*if (dip != sip)
			 {
			 if (debug)
			 printk("SEND ARP REQUEST: "STRIP" -> "STRIP"\n", IP2STR(sip), IP2STR(dip));
			 arp_send(ARPOP_REQUEST, ETH_P_ARP, dip, &in->br_port->br->dev, sip, NULL, in->br_port->br->dev.dev_addr, NULL);
			 }*/
			return 0;
		}
		else
		{
			if (arp_sip == arp_dip)
				*target = EBT_DROP;
		}

		break;
	}
	spin_unlock_bh(&arpnat_lock);
	return 1;
}

/**
 * Send arp request to all bridge ports, except the specified one.
 */
__STATIC int forge_arp(const struct net_device *in, uint32_t sip, uint32_t dip, uint8_t *smac)
{
	struct net_bridge_port *p;

	p = in->br_port->br->port_list;
	while (p != NULL) {
		if (p != in->br_port)
		{
			if (debug)
				printk("SEND ARP REQUEST on PORT %d: "STRIP"["STRMAC"] -> "STRIP"\n",
						p->port_no, IP2STR(sip), MAC2STR(smac), IP2STR(dip));
			arp_send(ARPOP_REQUEST, ETH_P_ARP, dip, p->dev, sip, NULL, smac, NULL);
		}
		p = p->next;
	}

	return 0;
}

/**
 * Do IP NAT on input chain
 **/
__STATIC int do_ip_in(struct sk_buff **pskb, const struct net_device *in, int *target, int vlan)
{
	uint8_t* eth_dmac = ((*pskb)->mac.ethernet)->h_dest;
	struct mac2ip* entry;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *uh;

    iph = (struct iphdr *)((uint8_t*)iph + vlan);

	if ((*pskb)->len < sizeof(struct ethhdr) + sizeof(struct iphdr))
		return 0;

	if (bootpnat && iph->protocol == htons(IPPROTO_UDP) && !(iph->frag_off & htons(IP_OFFSET)))
	{
		uh = (struct udphdr*)((u_int32_t *)iph + iph->ihl);
		if (uh->source == htons(67))
		{
			//do something illegal for BOOTP
			uint32_t* giaddrp = (uint32_t*)(((uint8_t*)uh) + sizeof(*uh) + GIADDR_OFFSET);
			uint8_t* mac = (uint8_t*)(giaddrp + 1);
			uint32_t ihl = iph->ihl << 2;
			uint32_t size = (*pskb)->len - ihl;
			uint32_t orig_daddr = iph->daddr;

			iph->daddr = 0xffffffff;
			if (debug)
				printk("IN BOOTPRELAY: "STRMAC"["STRIP"] -> "STRMAC"["STRIP"]\n",
					   MAC2STR(eth_dmac), IP2STR(orig_daddr), MAC2STR(mac), IP2STR(iph->daddr));
			memcpy(eth_dmac, mac, ETH_ALEN);
			*giaddrp = 0;
			uh->dest = htons(68);
			iph->check = 0;
			uh->check = 0;
			iph->check = ip_fast_csum((uint8_t*)iph, iph->ihl);
			(*pskb)->csum = csum_partial((uint8_t*)iph + ihl, size, 0);
			uh->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
										  size, iph->protocol,
										  (*pskb)->csum);
			if (uh->check == 0)
				uh->check = 0xFFFF;
			return 0;
		}
		else
			goto HANDLE_IP_PKT;
	}
	else
	{
		HANDLE_IP_PKT:
		spin_lock_bh(&arpnat_lock);
		entry = find_ip_nat(&arpnat_table, iph->daddr);
		if (entry)
		{
			//to me
			if (inet_confirm_addr(&in->br_port->br->dev, 0, entry->data.ip, RT_SCOPE_HOST))
			{
				if (debug)
					printk("IP PKT TO ME: "STRMAC"["STRIP"] -> "STRMAC"[type: %d]\n",
						   MAC2STR(eth_dmac), IP2STR(iph->daddr), MAC2STR(in->br_port->br->dev.dev_addr), (*pskb)->pkt_type);
				memcpy(eth_dmac, in->br_port->br->dev.dev_addr, ETH_ALEN);
			}
			else
			{
				if (debug)
					printk("IP PKT TO OTHER: "STRMAC"["STRIP"] -> "STRMAC"[type: %d]\n",
						   MAC2STR(eth_dmac), IP2STR(iph->daddr), MAC2STR(entry->data.mac), (*pskb)->pkt_type);
				memcpy(eth_dmac, entry->data.mac, ETH_ALEN);
				(*pskb)->pkt_type = PACKET_OTHERHOST;
			}
			spin_unlock_bh(&arpnat_lock);
			return 0;
		}
		spin_unlock_bh(&arpnat_lock);

		if (!entry && !(iph->frag_off & htons(IP_OFFSET)) && !(eth_dmac[0] & 1) &&
		    !inet_confirm_addr(&in->br_port->br->dev, 0, iph->daddr, RT_SCOPE_HOST))
		{
			uint32_t dip = iph->daddr;
			uint32_t sip = iph->saddr;
			if (sip && sip != dip && dip != 0xffffffff)
			{
				uint8_t* eth_smac = ((*pskb)->mac.ethernet)->h_source;
				struct net_bridge_fdb_entry* fdb_entry = br_fdb_get(in->br_port->br, eth_smac);
				if (!fdb_entry)
					forge_arp(in, sip, dip, eth_smac);
				/* FIXME: 
				* if entry is not found we should attempt to find our 
				* own gateway hwaddr and use it as destination MAC.
				* if gateway hwaddr is not present, ARP for it and 
				* broadcast the packet, which is not a good idea, but is
				* probably better than discarding it into the void. Hmm, maybe 
				* some TCP retry emulation could be done here?
			    */
			}
		}
	}
	return 1;
}

/**
 * Do PPPOE DISCOVERY NAT on input chain
 **/
__STATIC int do_pppoed_in(struct sk_buff **pskb, const struct net_device *in, int *target, int vlan)
{
	uint8_t* eth_dmac = ((*pskb)->mac.ethernet)->h_dest;
	struct pppoe_hdr* pppoe = (struct pppoe_hdr*)((*pskb)->nh.raw);
	struct pppoe_tag* tag = (struct pppoe_tag*)(pppoe + 1);

	while (tag && ntohs(tag->tag_type) != PTT_EOL && (uint8_t*)tag < (uint8_t*)((*pskb)->nh.raw) + sizeof(*pppoe) + ntohs(pppoe->length))
	{
		if (ntohs(tag->tag_type) == PTT_RELAY_SID)
		{
                        int16_t size;
			//update relay_sid for mac
			if (pppoe->code == PADS_CODE || pppoe->code == PADT_CODE)
			{
				spin_lock_bh(&arpnat_lock);
				update_pppoe_nat(&arpnat_table, tag->tag_data, pppoe->code == PADT_CODE ? 0 : pppoe->sid);
				spin_unlock_bh(&arpnat_lock);
			}
			memcpy(eth_dmac, tag->tag_data, ETH_ALEN);
#ifdef ARPNAT_STRIP_PPPOE_PTT_RELAY_SID
			//move rest tags
                        size = ntohs(pppoe->length) - ((uint8_t*)(tag->tag_data + 12) - (uint8_t*)(pppoe + 1));
                        if (size > 0) memmove(tag, tag->tag_data + 12, size);
			pppoe->length = htons(ntohs(pppoe->length) - sizeof(*tag) - 12); //drop PTT_RELAY_SID
#endif
			return 0;
		}
		tag = (struct pppoe_tag*)(tag->tag_data + htons(tag->tag_len));
	}
	return 1;
}

/**
 * Do PPPOE SESSION NAT on input chain
 **/
__STATIC int do_pppoes_in(struct sk_buff **pskb, const struct net_device *in, int *target, int vlan)
{
	struct pppoe_hdr* pppoe = (struct pppoe_hdr*)((*pskb)->nh.raw);
	uint8_t* eth_dmac = ((*pskb)->mac.ethernet)->h_dest;
	struct mac2ip* entry;

	spin_lock_bh(&arpnat_lock);
	entry = find_pppoe_nat(&arpnat_table, pppoe->sid);
	if (entry)
		memcpy(eth_dmac, entry->data.mac, ETH_ALEN);
	else
		memcpy(eth_dmac, in->br_port->br->dev.dev_addr, ETH_ALEN);
	spin_unlock_bh(&arpnat_lock);
	return 0;
}

__STATIC int do_in(struct sk_buff **pskb, const struct net_device *in, int target)
{
	int nat = 1;
	uint8_t* eth_dmac = ((*pskb)->mac.ethernet)->h_dest;
	uint16_t proto = ((*pskb)->mac.ethernet)->h_proto;
	int vlan = 0;

	if (proto == __constant_htons(ETH_P_8021Q))
	{
	    	struct vlan_ethhdr *hdr = (struct vlan_ethhdr *)
			((*pskb)->mac.ethernet);
                proto = hdr->h_vlan_encapsulated_proto;
                vlan = 4;
	}

	switch (proto)
	{
	case __constant_htons(ETH_P_ARP):
		nat = do_arp_in(pskb, in, &target, vlan);
		break;
	case __constant_htons(ETH_P_IP):
		nat = do_ip_in(pskb, in, &target, vlan);
		break;
	case __constant_htons(ETH_P_PPP_DISC):
		if (pppoenat)
			nat = do_pppoed_in(pskb, in, &target, vlan);
		break;
	case __constant_htons(ETH_P_PPP_SES):
		if (pppoenat)
			nat = do_pppoes_in(pskb, in, &target, vlan);
		break;
	}
	if (nat && ! (eth_dmac[0] & 1))
	{
#ifdef DROP_PACKETS_NOT_FOR_ME
		if (memcmp(in->br_port->br->dev.dev_addr, eth_dmac, ETH_ALEN) &&
			memcmp(in->dev_addr, eth_dmac, ETH_ALEN))
			return EBT_DROP;
#endif
		if (debug)
		{
			printk("DMAC["STRMAC"]->BRMAC["STRMAC"]\n", MAC2STR(eth_dmac), MAC2STR(in->br_port->br->dev.dev_addr));
		}
       	memcpy(eth_dmac, in->br_port->br->dev.dev_addr, ETH_ALEN);
	}
	return target;
}


/**
 * Do ARP NAT on output chain
 **/
__STATIC int do_arp_out(struct sk_buff **pskb, const struct net_device *out, int *target, int vlan)
{
	struct arphdr *ah = (*pskb)->nh.arph;
	uint32_t* arp_sip;
	uint8_t* arp_smac;
	uint32_t* arp_dip;
	uint8_t* nh_raw = (*pskb)->nh.raw;

	ah = (struct arphdr*)((uint8_t*)ah + vlan);
	nh_raw += vlan;

	if (ah->ar_hln == ETH_ALEN && ah->ar_pro == htons(ETH_P_IP) &&
		ah->ar_pln == 4)
	{
		arp_sip = (uint32_t*)(nh_raw + sizeof(*ah) + ah->ar_hln);
		arp_smac = nh_raw + sizeof(*ah);
		arp_dip = (uint32_t*)(nh_raw + sizeof(*ah) + (2 * ah->ar_hln) + ah->ar_pln);
	}
	else
		//Not IP ARP just NAT it
		return 1;

	switch (ah->ar_op)
	{
	case __constant_htons(ARPOP_REQUEST):
	case __constant_htons(ARPOP_REPLY):
		spin_lock_bh(&arpnat_lock);
		update_arp_nat(&arpnat_table, arp_smac, *arp_sip);
		spin_unlock_bh(&arpnat_lock);
		/* do BR ip lookup */
		if (inet_confirm_addr(&out->br_port->br->dev, 0, *arp_dip, RT_SCOPE_HOST))
			//It is to us do not NAT
			return 0;

		*pskb = skb_unshare(*pskb, GFP_ATOMIC);
		if (debug)
			printk("OUT ARPNAT: "STRMAC" -> "STRMAC"\n", MAC2STR(((*pskb)->mac.ethernet)->h_source), MAC2STR(out->dev_addr));
		//We have new skb so remap nh_raw
		nh_raw = (*pskb)->nh.raw;
		nh_raw += vlan;
		arp_smac = nh_raw + sizeof(*ah);
		memcpy(arp_smac, out->dev_addr, ETH_ALEN);
		break;
	}
	return 1;
}

/**
 * Do BOOTP NAT on output chain
 **/
__STATIC int do_bootp_out(struct sk_buff **pskb, const struct net_device *out, int *target, int vlan)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *uh = (struct udphdr*)((u_int32_t *)iph + iph->ihl);
	uint32_t ihl;
	uint32_t size;

	iph = (struct iphdr *)((uint8_t*)iph + vlan);
	uh = (struct udphdr *)((uint8_t*)uh + vlan);

	if (!memcmp(out->br_port->br->dev.dev_addr, ((*pskb)->mac.ethernet)->h_source, ETH_ALEN) ||
		iph->protocol != htons(IPPROTO_UDP) ||
		(iph->frag_off & htons(IP_OFFSET)))
		//to us or not UDP or fragment then just NAT
		return 1;

	if (uh->dest != htons(67))
		//not bootp then just NAT
		return 1;

	if (bootpnat == BOOTP_RELAY)
	{
		//do something illegal for BOOTP
		uint32_t giaddr = inet_select_addr(&out->br_port->br->dev, iph->daddr, RT_SCOPE_LINK);
		uint32_t* giaddrp = (uint32_t*)(((uint8_t*)uh) + sizeof(*uh) + GIADDR_OFFSET);
		if (debug)
			printk("OUT BOOTPRELAY: "STRIP" -> "STRIP"\n",
				   IP2STR(*giaddrp), IP2STR(giaddr));
		*giaddrp = giaddr;
	}
	else
	{
		uint8_t* flagp = (uint8_t*)(((uint8_t*)uh) + sizeof(*uh) + FLAGS_OFFSET);
		*flagp |= 0x80;
	}

	ihl = iph->ihl << 2;
	size = (*pskb)->len - ihl;
	uh->check = 0;
	(*pskb)->csum = csum_partial((uint8_t*)iph + ihl, size, 0);
	uh->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
								  size, iph->protocol,
								  (*pskb)->csum);
	if (uh->check == 0)
		uh->check = 0xFFFF;

	return 1;
}

/**
 * Do PPPOE NAT on output chain
 **/
__STATIC int do_pppoe_out(struct sk_buff **pskb, const struct net_device *out, int *target, int vlan)
{
	struct pppoe_hdr* pppoe = (struct pppoe_hdr*)((*pskb)->nh.raw);
	struct pppoe_tag* tag = (struct pppoe_tag*)(pppoe + 1);

	while (tag && ntohs(tag->tag_type) != PTT_EOL && (uint8_t*)tag < (uint8_t*)((*pskb)->nh.raw) + sizeof(*pppoe) + ntohs(pppoe->length) )
	{
		if (ntohs(tag->tag_type) == PTT_RELAY_SID)
		{
			//update relay_sid for mac
			if (pppoe->code == PADS_CODE || pppoe->code == PADT_CODE)
			{
				spin_lock_bh(&arpnat_lock);
				update_pppoe_nat(&arpnat_table, ((*pskb)->mac.ethernet)->h_source, pppoe->code == PADT_CODE ? 0 : pppoe->sid);
				spin_unlock_bh(&arpnat_lock);
			}
			// In this place we will eat original PTT_RELAY_SID.
                        // We are sorry but we have to do that to make Wireless STA in Bridge mode working.
			memset(tag->tag_data, 0, 12);
			memcpy(tag->tag_data, ((*pskb)->mac.ethernet)->h_source, ETH_ALEN); 
			return 1;
		}
		tag = (struct pppoe_tag*)(tag->tag_data + ntohs(tag->tag_len));
	}
	if (tag)
	{
                int8_t has_ptt_eol = (ntohs(tag->tag_type) == PTT_EOL);
		if ((*pskb)->len <= 1484)
			//make some room for PTT_RELAY_SID
			skb_put(*pskb, sizeof(*tag) + 12);
		//else XXX: set Generic Error to packet and send it back

		//add relay_sid to the packet
		tag->tag_type = htons(PTT_RELAY_SID);
		tag->tag_len = htons(12);
                memset(tag->tag_data, 0, 12);
		memcpy(tag->tag_data, ((*pskb)->mac.ethernet)->h_source, ETH_ALEN);
		tag = (struct pppoe_tag*)(tag->tag_data + 12);
                if (has_ptt_eol)
		    //Set PTTL_EOL tag
		    memset(tag, 0, sizeof(struct pppoe_tag));
		pppoe->length = htons(ntohs(pppoe->length) + sizeof(*tag) + 12);
	}
	return 1;
}

/**
 * Handle packets on output chain
 **/
__STATIC int do_out(struct sk_buff **pskb, const struct net_device *out, int target)
{
	int nat = 1;
	uint16_t proto = ((*pskb)->mac.ethernet)->h_proto;
	int vlan = 0;

	if (proto == __constant_htons(ETH_P_8021Q))
	{
	    	struct vlan_ethhdr *hdr = (struct vlan_ethhdr *)
			((*pskb)->mac.ethernet);
                proto = hdr->h_vlan_encapsulated_proto;
                vlan = 4;
	}

	switch (proto)
	{
	case __constant_htons(ETH_P_ARP):
		nat = do_arp_out(pskb, out, &target, vlan);
		break;
	case __constant_htons(ETH_P_IP):
		if (bootpnat)
			nat = do_bootp_out(pskb, out, &target, vlan);
		break;
	case __constant_htons(ETH_P_PPP_DISC):
		if (pppoenat)
			nat = do_pppoe_out(pskb, out, &target, vlan);
		break;
	}
	if (nat)
		memcpy(((*pskb)->mac.ethernet)->h_source, out->dev_addr, ETH_ALEN);
	return target;
}

/**
 * Handle ARPNAT
 **/
__STATIC int ebt_target_arpnat(struct sk_buff **pskb, unsigned int hooknr,
							   const struct net_device *in, const struct net_device *out,
							   const void *data, unsigned int datalen)
{
	int target = ((struct ebt_nat_info *)data)->target;

	if (in)
		return do_in(pskb, in, target);
	else if (out)
	{
#ifdef NOT_IN_THIS_RELEASE_HAVE_SET_MAC_ADDR  		 
		uint8_t* smac = ((*pskb)->mac.ethernet)->h_source;
		uint8_t* mac = ((struct ebt_nat_info *)data)->mac; 
		if (mac[0] == 0xff && !(smac[0] & 1))
		{	
			memcpy(mac, smac, ETH_ALEN);
			printk("CLONE TO FIRST MAC["STRMAC"]\n", MAC2STR(mac));
			//It hangs with kernel bug there
			out->stop((struct net_device *)out);
			out->set_mac_address((struct net_device *)out, mac);
			out->open((struct net_device *)out);
		}
#endif
		return do_out(pskb, out, target);
	}
	return target;
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
	{NULL, NULL}, EBT_ARPNAT_TARGET, ebt_target_arpnat, ebt_target_nat_arpcheck,
	NULL, THIS_MODULE
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
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
