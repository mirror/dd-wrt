/*
Experimental ethernet netdevice using ATM AAL5 as underlying carrier
(RFC1483 obsoleted by RFC2684) for Linux 2.4
Author: Marcell GAL, 2000, XDSL Ltd, Hungary
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <asm/uaccess.h>
#include <net/arp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/capability.h>
#include <linux/seq_file.h>

#include <linux/atmbr2684.h>

#include "common.h"
#include "ipcommon.h"

#if defined(CONFIG_MIPS_BRCM)
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned long   UINT32;
#include <bcmatmapi.h>
#include <portMirror.h>
#endif

/*
 * Define this to use a version of the code which interacts with the higher
 * layers in a more intellegent way, by always reserving enough space for
 * our header at the begining of the packet.  However, there may still be
 * some problems with programs like tcpdump.  In 2.5 we'll sort out what
 * we need to do to get this perfect.  For now we just will copy the packet
 * if we need space for the header
 */
/* #define FASTER_VERSION */
//#define VLAN_DEBUG
//#define SKB_DEBUG

//#define PTM_ETH_FCS

#ifdef DEBUG
#define DPRINTK(format, args...) printk(KERN_DEBUG "br2684: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif

#ifdef PTM_ETH_FCS
typedef unsigned long   uint32;
typedef unsigned char   uint8;

static uint32 crc32( uint8 *pdata, uint nbytes, uint32 crc);
#endif

#if defined(CONFIG_BCM_VDSL)
extern int g_nPtmTraffic;
#endif
 
#ifdef SKB_DEBUG
static void skb_debug(const struct sk_buff *skb)
{
#define NUM2PRINT 50
	char buf[NUM2PRINT * 3 + 1];	/* 3 chars per byte */
	int i = 0;
	for (i = 0; i < skb->len && i < NUM2PRINT; i++) {
		sprintf(buf + i * 3, "%2.2x ", 0xff & skb->data[i]);
	}
	printk(KERN_DEBUG "br2684: skb: %s\n", buf);
}
#else
#define skb_debug(skb)	do {} while (0)
#endif

static unsigned char llc_oui_pid_pad[] =
    { 0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00 };
#define PADLEN	(2)

enum br2684_encaps {
	e_vc  = BR2684_ENCAPS_VC,
	e_llc = BR2684_ENCAPS_LLC,
};

struct br2684_vcc {
	struct atm_vcc  *atmvcc;
	struct net_device *device;
	/* keep old push,pop functions for chaining */
	void (*old_push)(struct atm_vcc *vcc,struct sk_buff *skb);
	/* void (*old_pop)(struct atm_vcc *vcc,struct sk_buff *skb); */
	enum br2684_encaps encaps;
	struct list_head brvccs;
#ifdef CONFIG_ATM_BR2684_IPFILTER
	struct br2684_filter filter;
#endif /* CONFIG_ATM_BR2684_IPFILTER */
#ifndef FASTER_VERSION
	unsigned copies_needed, copies_failed;
#endif /* FASTER_VERSION */
#if defined(CONFIG_MIPS_BRCM)
	/* Protocol filter flag, currently only PPPoE.
	   When turned on, all non-PPPoE traffic will be dropped
	   on this PVC */  
        int proto_filter;
#ifdef SUPPORT_VLAN
        unsigned short vlan_id;        /* vlan id (0-4096) */
#endif  // SUPPORT_VLAN
 
	unsigned char *atm_tx_mem;
	int atm_num_tx_dps;
	struct list_head atm_list_tx_dps;
	unsigned long atm_attach_handle;


#endif
};

#if defined(CONFIG_MIPS_BRCM)
struct net_devext {
    struct list_head list;
    struct net_device net_dev;
    struct net_device_stats stats;
    int proto_filter;
};
#endif

struct br2684_dev {
	struct net_device *net_dev;
#if defined(CONFIG_MIPS_BRCM)
	struct list_head net_devexts;
#endif
	struct list_head br2684_devs;
	int number;
	struct list_head brvccs; /* one device <=> one vcc (before xmas) */
	struct net_device_stats stats;
	int mac_was_set;
};

#if defined(CONFIG_MIPS_BRCM)
static int br2684_attachvcc(struct atm_vcc *atmvcc, void __user *arg);
static int br2684_start_xmit_bcm(struct sk_buff *skb, struct net_device *dev);
static void br2684_free_dp( PATM_VCC_DATA_PARMS pDp );
static void br2684_rx_cb( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDp, struct br2684_vcc *brvcc );
static void br2684_free_skb_or_data( PATM_VCC_DATA_PARMS pDp, void *pObj,
    int nFlag );
static void br2684_push_bcm(struct atm_vcc *atmvcc, struct sk_buff *skb);
static int bcm_set_proto_filter(struct atm_vcc *atmvcc, void __user *arg);
static int bcm_is_pppoe_passthrough_pkt(struct br2684_dev *brdev, 
     struct sk_buff *skb, struct net_device **net_dev);
#endif

#if defined(CONFIG_MIPS_BRCM) || defined(CONFIG_BCM_VDSL)
#define MIN_PKT_SIZE 60
#endif

/*
 * This lock should be held for writing any time the list of devices or
 * their attached vcc's could be altered.  It should be held for reading
 * any time these are being queried.  Note that we sometimes need to
 * do read-locking under interrupt context, so write locking must block
 * the current CPU's interrupts
 */
static DEFINE_RWLOCK(devs_lock);

static LIST_HEAD(br2684_devs);

static inline struct br2684_dev *BRPRIV(const struct net_device *net_dev)
{
	return (struct br2684_dev *) net_dev->priv;
}

#if defined(CONFIG_MIPS_BRCM)
static inline struct net_devext *EXTPRIV(const struct net_device *net_dev)
{
	return (struct net_devext *) net_dev->atalk_ptr;
}

static inline struct net_device *list_entry_devext(const struct list_head *le)
{
	return &(list_entry(le, struct net_devext, list)->net_dev);
}
#endif

static inline struct net_device *list_entry_brdev(const struct list_head *le)
{
	return list_entry(le, struct br2684_dev, br2684_devs)->net_dev;
}

static inline struct br2684_vcc *BR2684_VCC(const struct atm_vcc *atmvcc)
{
	return (struct br2684_vcc *) (atmvcc->user_back);
}

static inline struct br2684_vcc *list_entry_brvcc(const struct list_head *le)
{
	return list_entry(le, struct br2684_vcc, brvccs);
}

/* Caller should hold read_lock(&devs_lock) */
static struct net_device *br2684_find_dev(const struct br2684_if_spec *s)
{
	struct list_head *lh;
	struct net_device *net_dev;
	switch (s->method) {
	case BR2684_FIND_BYNUM:
		list_for_each(lh, &br2684_devs) {
			net_dev = list_entry_brdev(lh);
			if (BRPRIV(net_dev)->number == s->spec.devnum)
				return net_dev;
		}
		break;
	case BR2684_FIND_BYIFNAME:
		list_for_each(lh, &br2684_devs) {
			net_dev = list_entry_brdev(lh);
			if (!strncmp(net_dev->name, s->spec.ifname, IFNAMSIZ))
				return net_dev;
		}
		break;
	}
	return NULL;
}

#ifdef SUPPORT_VLAN
#include <linux/if_vlan.h>
/**
 * vlan_tag_insert - regular VLAN tag inserting
 * @skb: skbuff to tag
 * @tag: VLAN tag to insert
 *
 * Inserts the VLAN tag into @skb as part of the payload
 * Returns a VLAN tagged skb. If a new skb is created, @skb is freed.
 * 
 * Following the skb_unshare() example, in case of error, the calling function
 * doesn't have to worry about freeing the original skb.
 */
#define ATM_HEADER_LEN     10
#define ATM_AND_MAC_LEN    ((ATM_HEADER_LEN) + (2 * VLAN_ETH_ALEN))
#define MAC_LEN            (2 * VLAN_ETH_ALEN)

struct atm_vlan_ethhdr {
   unsigned char        h_atm_stuff[ATM_HEADER_LEN]; /* atm stuff 10 bytes */
   unsigned char	h_dest[ETH_ALEN];	   /* destination eth addr	*/
   unsigned char	h_source[ETH_ALEN];	   /* source ether addr	*/
   unsigned short       h_vlan_proto;              /* Should always be 0x8100 */
   unsigned short       h_vlan_TCI;                /* Encapsulates priority and VLAN ID */
   unsigned short	h_vlan_encapsulated_proto; /* packet type ID field (or len) */
};

static inline struct sk_buff *vlan_tag_insert(struct sk_buff *skb, unsigned short tag)
{
	struct atm_vlan_ethhdr *veth;

	if (skb_headroom(skb) < VLAN_HLEN) {
		struct sk_buff *sk_tmp = skb;
		skb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
		kfree_skb(sk_tmp);
		if (!skb) {
			printk(KERN_ERR "vlan: failed to realloc headroom\n");
			return NULL;
		}
	} else {
		skb = skb_unshare(skb, GFP_ATOMIC);
		if (!skb) {
			printk(KERN_ERR "vlan: failed to unshare skbuff\n");
			return NULL;
		}
	}

	veth = (struct atm_vlan_ethhdr *)skb_push(skb, VLAN_HLEN);

#ifdef CONFIG_BCM_VDSL
   if (!g_nPtmTraffic) {
	/* Move the mac addresses to the beginning of the new header. */
	memmove(skb->data, skb->data + VLAN_HLEN, (2 * VLAN_ETH_ALEN) + ATM_HEADER_LEN);
   }
   else {
      /* Move the mac addresses to the beginning of the new header. */
      memmove(skb->data, skb->data + VLAN_HLEN, (2 * VLAN_ETH_ALEN)) ;
   }
#else
      memmove(skb->data, skb->data + VLAN_HLEN, (2 * VLAN_ETH_ALEN)) ;
#endif

	/* first, the ethernet type */
	veth->h_vlan_proto = __constant_htons(ETH_P_8021Q);

	/* now, the tag */
	veth->h_vlan_TCI = htons(tag);

	skb->protocol = __constant_htons(ETH_P_8021Q);
	skb->mac.raw -= VLAN_HLEN;
	skb->nh.raw -= VLAN_HLEN;

	return skb;
}
#endif  // SUPPORT_VLAN

/*
 * Send a packet out a particular vcc.  Not to useful right now, but paves
 * the way for multiple vcc's per itf.  Returns true if we can send,
 * otherwise false
 */
static int br2684_xmit_vcc(struct sk_buff *skb, struct br2684_dev *brdev,
#if defined(CONFIG_MIPS_BRCM)
	struct br2684_vcc *brvcc, struct net_device *dev)
#else
	struct br2684_vcc *brvcc)
#endif
{
	struct atm_vcc *atmvcc;

#if defined(CONFIG_MIPS_BRCM)
	struct net_devext *devext = EXTPRIV(dev);
  
   /* We pad upto 60 bytes for the data if it is smaller in size */
    if (skb->len < MIN_PKT_SIZE)
    {
       struct sk_buff *skb2=skb_copy_expand(skb, 0, MIN_PKT_SIZE - skb->len, GFP_ATOMIC);
       dev_kfree_skb(skb);
       if (skb2 == NULL) {
          brvcc->copies_failed++;
          return 0;
       }
       skb = skb2;		
       memset(skb->tail, 0, MIN_PKT_SIZE - skb->len);		
       skb_put(skb, MIN_PKT_SIZE - skb->len);
    }
#endif
  
#ifdef FASTER_VERSION
		if (brvcc->encaps == e_llc)
			memcpy(skb_push(skb, 8), llc_oui_pid_pad, 8);
			/* last 2 bytes of llc_oui_pid_pad are managed by header routines;
			   yes, you got it: 8 + 2 = sizeof(llc_oui_pid_pad)
			 */
#else
		int minheadroom = (brvcc->encaps == e_llc) ? 10 : 2;
		if (skb_headroom(skb) < minheadroom) {
			struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);
			brvcc->copies_needed++;
			dev_kfree_skb(skb);
			if (skb2 == NULL) {
				brvcc->copies_failed++;
				return 0;
			}
			skb = skb2;
		}
		skb_push(skb, minheadroom);
		if (brvcc->encaps == e_llc)
			memcpy(skb->data, llc_oui_pid_pad, 10);
		else
			memset(skb->data, 0, 2);
#endif /* FASTER_VERSION */

#ifdef SUPPORT_VLAN
#ifdef VLAN_DEBUG
		printk("=====> br2684_xmit_vcc  bef add vlan tag, skb->len=0x%04x\n", skb->len);
		skb_debug(skb);
#endif // VLAN_DEBUG

		/* Construct the second two bytes. This field looks something
	 	 * like:
		 * usr_priority: 3 bits	 (high bits)
		 * CFI		 1 bit
		 * VLAN ID	 12 bits (low bits)
		 */
		//brvcc->vlan_id |= vlan_dev_get_egress_qos_mask(dev, skb);
		/* bit 3-0 of the 32-bit nfmark is the atm priority, set by iptables
		 * bit 7-4 is the Ethernet switch physical port number, set by lan port drivers.
		 * bit 8-11 is the wanVlan priority bits
		*/
		if (brvcc->vlan_id != 0xffff) {
			brvcc->vlan_id &= 0xffff0fff;   // clear the priority bits first
			// if bit 8-11 is set (none zeros), add in the priority bits
			if (skb->nfmark & 0x0000f000) {
				brvcc->vlan_id |= (skb->nfmark & 0x0000f000);
			}
			skb = vlan_tag_insert(skb, brvcc->vlan_id);
			if (!skb) {
			    brdev->stats.tx_dropped++;
			    return 1;
			}
#ifdef VLAN_DEBUG
			printk("=====> br2684_xmit_vcc  aft add vlan tag, skb->len=%d\n", skb->len);
			skb_debug(skb);
#endif // VLAN_DEBUG
		}
#endif  // SUPPORT_VLAN

#if defined(CONFIG_BCM_VDSL)
#ifdef PTM_ETH_FCS
    if( g_nPtmTraffic ) {
  	  union crc_32_tag
  {
        int   i;
        char  ch[4];
      } crc_32;
    
      crc_32.i = ~crc32(skb->data, skb->len, 0xffffffff);
      /* crc bytes put in the memory in reverse order */
     skb->data[skb->len]     = crc_32.ch[3];
     skb->data[skb->len + 1] = crc_32.ch[2];
     skb->data[skb->len + 2] = crc_32.ch[1];
     skb->data[skb->len + 3] = crc_32.ch[0];

  	skb->len += 4;	// extra four bytes for ethernet checksum
  }
#endif
#endif

	ATM_SKB(skb)->vcc = atmvcc = brvcc->atmvcc;
	DPRINTK("atm_skb(%p)->vcc(%p)->dev(%p)\n", skb, atmvcc, atmvcc->dev);
	if (!atm_may_send(atmvcc, skb->truesize)) {
		/* we free this here for now, because we cannot know in a higher 
			layer whether the skb point it supplied wasn't freed yet.
			now, it always is.
		*/
		dev_kfree_skb(skb);
		return 0;
	}
	atomic_add(skb->truesize, &sk_atm(atmvcc)->sk_wmem_alloc);
	ATM_SKB(skb)->atm_options = atmvcc->atm_options;
#if defined(CONFIG_MIPS_BRCM)
 	if (devext) {
     	    devext->stats.tx_packets++;
 	    devext->stats.tx_bytes += skb->len;	    
 	}
 	else {
 	    brdev->stats.tx_packets++;
 	    brdev->stats.tx_bytes += skb->len;
 	}
#else
	brdev->stats.tx_packets++;
	brdev->stats.tx_bytes += skb->len;
#endif
#if defined(CONFIG_MIPS_BRCM)
	if (atmvcc->send(atmvcc, skb) != 0)
 	{
 	    if (devext)
 		devext->stats.tx_dropped++;
 	    else
 		brdev->stats.tx_dropped++;
 	}
#else
	atmvcc->send(atmvcc, skb);
#endif
	return 1;
}

static inline struct br2684_vcc *pick_outgoing_vcc(struct sk_buff *skb,
	struct br2684_dev *brdev)
{
	return list_empty(&brdev->brvccs) ? NULL :
	    list_entry_brvcc(brdev->brvccs.next); /* 1 vcc/dev right now */
}

static int br2684_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct br2684_dev *brdev = BRPRIV(dev);
	struct br2684_vcc *brvcc;
#if defined(CONFIG_MIPS_BRCM)
 	struct net_devext *devext = EXTPRIV(dev);
#endif

	DPRINTK("br2684_start_xmit, skb->dst=%p\n", skb->dst);
	read_lock(&devs_lock);
	brvcc = pick_outgoing_vcc(skb, brdev);
	if (brvcc == NULL) {
		DPRINTK("no vcc attached to dev %s\n", dev->name);
#if defined(CONFIG_MIPS_BRCM)
 		if (devext) {
 		    devext->stats.tx_errors++;
 		    devext->stats.tx_carrier_errors++;
 		}
 		else {
 		    brdev->stats.tx_errors++;
 		    brdev->stats.tx_carrier_errors++;
 		}
#else
		brdev->stats.tx_errors++;
		brdev->stats.tx_carrier_errors++;
#endif
		/* netif_stop_queue(dev); */
		dev_kfree_skb(skb);
		read_unlock(&devs_lock);
		return 0;
	}
 	
#if defined(CONFIG_MIPS_BRCM)
/*  trmp fix
	if (brvcc->proto_filter & FILTER_PPPOE) {
		if ((skb->protocol != htons(ETH_P_PPP_DISC)) && (skb->protocol != htons(ETH_P_PPP_SES))) {
			DPRINTK("non-PPPOE packet dropped on TX dev %s\n", dev->name);
			dev_kfree_skb(skb);
			read_unlock(&devs_lock);
			return 0;
		}
	}
*/
#endif	

#ifdef VLAN_DEBUG
     if (brvcc->vlan_id != 0xffff)
             printk("=====> br2684_start_xmit  vlan_id=0x%04x\n", brvcc->vlan_id);
#endif // VLAN_DEBUG

#if defined(CONFIG_MIPS_BRCM)
	if (!br2684_xmit_vcc(skb, brdev, brvcc, dev)) {
#else
  	if (!br2684_xmit_vcc(skb, brdev, brvcc)) {
#endif
		/*
		 * We should probably use netif_*_queue() here, but that
		 * involves added complication.  We need to walk before
		 * we can run
		 */
		/* don't free here! this pointer might be no longer valid!
		dev_kfree_skb(skb);
		*/
#if defined(CONFIG_MIPS_BRCM)
		if (devext) {
		    devext->stats.tx_errors++;
		    devext->stats.tx_fifo_errors++;
		}
		else {
		    brdev->stats.tx_errors++;
		    brdev->stats.tx_fifo_errors++;
		}
#else
		brdev->stats.tx_errors++;
		brdev->stats.tx_fifo_errors++;
#endif
	}
	read_unlock(&devs_lock);
	return 0;
}

static struct net_device_stats *br2684_get_stats(struct net_device *dev)
{
	DPRINTK("br2684_get_stats\n");
#if defined(CONFIG_MIPS_BRCM)
	if (dev->atalk_ptr)
	    return &EXTPRIV(dev)->stats;
	else
	    return &BRPRIV(dev)->stats;
#else
	return &BRPRIV(dev)->stats;
#endif
}

#ifdef FASTER_VERSION
/*
 * These mirror eth_header and eth_header_cache.  They are not usually
 * exported for use in modules, so we grab them from net_device
 * after ether_setup() is done with it.  Bit of a hack.
 */
static int (*my_eth_header)(struct sk_buff *, struct net_device *,
	unsigned short, void *, void *, unsigned);
static int (*my_eth_header_cache)(struct neighbour *, struct hh_cache *);

static int
br2684_header(struct sk_buff *skb, struct net_device *dev,
	      unsigned short type, void *daddr, void *saddr, unsigned len)
{
	u16 *pad_before_eth;
	int t = my_eth_header(skb, dev, type, daddr, saddr, len);
	if (t > 0) {
		pad_before_eth = (u16 *) skb_push(skb, 2);
		*pad_before_eth = 0;
		return dev->hard_header_len;	/* or return 16; ? */
	} else
		return t;
}

static int
br2684_header_cache(struct neighbour *neigh, struct hh_cache *hh)
{
/* hh_data is 16 bytes long. if encaps is ether-llc we need 24, so
xmit will add the additional header part in that case */
	u16 *pad_before_eth = (u16 *)(hh->hh_data);
	int t = my_eth_header_cache(neigh, hh);
	DPRINTK("br2684_header_cache, neigh=%p, hh_cache=%p\n", neigh, hh);
	if (t < 0)
		return t;
	else {
		*pad_before_eth = 0;
		hh->hh_len = PADLEN + ETH_HLEN;
	}
	return 0;
}

/*
 * This is similar to eth_type_trans, which cannot be used because of
 * our dev->hard_header_len
 */
static inline __be16 br_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	struct ethhdr *eth;
	unsigned char *rawp;
	eth = eth_hdr(skb);

	if (is_multicast_ether_addr(eth->h_dest)) {
		if (!compare_ether_addr(eth->h_dest, dev->broadcast))
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	}

	else if (compare_ether_addr(eth->h_dest, dev->dev_addr))
		skb->pkt_type = PACKET_OTHERHOST;

	if (ntohs(eth->h_proto) >= 1536)
		return eth->h_proto;

	rawp = skb->data;

	/*
	 * This is a magic hack to spot IPX packets. Older Novell breaks
	 * the protocol design and runs IPX over 802.3 without an 802.2 LLC
	 * layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
	 * won't work for fault tolerant netware but does for the rest.
	 */
	if (*(unsigned short *) rawp == 0xFFFF)
		return htons(ETH_P_802_3);

	/*
	 * Real 802.2 LLC
	 */
	return htons(ETH_P_802_2);
}
#endif /* FASTER_VERSION */

/*
 * We remember when the MAC gets set, so we don't override it later with
 * the ESI of the ATM card of the first VC
 */
static int (*my_eth_mac_addr)(struct net_device *, void *);
static int br2684_mac_addr(struct net_device *dev, void *p)
{
	int err = my_eth_mac_addr(dev, p);
#if defined(CONFIG_MIPS_BRCM)
	return err;
#else
	if (!err)
		BRPRIV(dev)->mac_was_set = 1;
	return err;
#endif
}

#ifdef CONFIG_ATM_BR2684_IPFILTER
/* this IOCTL is experimental. */
static int br2684_setfilt(struct atm_vcc *atmvcc, void __user *arg)
{
	struct br2684_vcc *brvcc;
	struct br2684_filter_set fs;

	if (copy_from_user(&fs, arg, sizeof fs))
		return -EFAULT;
	if (fs.ifspec.method != BR2684_FIND_BYNOTHING) {
		/*
		 * This is really a per-vcc thing, but we can also search
		 * by device
		 */
		struct br2684_dev *brdev;
		read_lock(&devs_lock);
		brdev = BRPRIV(br2684_find_dev(&fs.ifspec));
		if (brdev == NULL || list_empty(&brdev->brvccs) ||
		    brdev->brvccs.next != brdev->brvccs.prev)  /* >1 VCC */
			brvcc = NULL;
		else
			brvcc = list_entry_brvcc(brdev->brvccs.next);
		read_unlock(&devs_lock);
		if (brvcc == NULL)
			return -ESRCH;
	} else
		brvcc = BR2684_VCC(atmvcc);
	memcpy(&brvcc->filter, &fs.filter, sizeof(brvcc->filter));
	return 0;
}

/* Returns 1 if packet should be dropped */
static inline int
packet_fails_filter(u16 type, struct br2684_vcc *brvcc, struct sk_buff *skb)
{
	if (brvcc->filter.netmask == 0)
		return 0;			/* no filter in place */
	if (type == __constant_htons(ETH_P_IP) &&
	    (((struct iphdr *) (skb->data))->daddr & brvcc->filter.
	     netmask) == brvcc->filter.prefix)
		return 0;
	if (type == __constant_htons(ETH_P_ARP))
		return 0;
	/* TODO: we should probably filter ARPs too.. don't want to have
	 *   them returning values that don't make sense, or is that ok?
	 */
	return 1;		/* drop */
}
#endif /* CONFIG_ATM_BR2684_IPFILTER */

static void br2684_close_vcc(struct br2684_vcc *brvcc)
{
	DPRINTK("removing VCC %p from dev %p\n", brvcc, brvcc->device);
	write_lock_irq(&devs_lock);
	list_del(&brvcc->brvccs);
	write_unlock_irq(&devs_lock);
	brvcc->atmvcc->user_back = NULL;	/* what about vcc->recvq ??? */
	if( brvcc->old_push)
		brvcc->old_push(brvcc->atmvcc, NULL);	/* pass on the bad news */
	kfree(brvcc);
	module_put(THIS_MODULE);
}

/* when AAL5 PDU comes in: */
static void br2684_push(struct atm_vcc *atmvcc, struct sk_buff *skb)
{
	struct br2684_vcc *brvcc = BR2684_VCC(atmvcc);
	struct net_device *net_dev = brvcc->device;
	struct br2684_dev *brdev = BRPRIV(net_dev);
#if !defined(CONFIG_BCM_VDSL)
	int plen = sizeof(llc_oui_pid_pad) + ETH_HLEN;
#else
  /* Ethernet packets from PTM traffic will not have an RFC header in place */
	int plen = ( !g_nPtmTraffic ) ? sizeof(llc_oui_pid_pad) + ETH_HLEN : ETH_HLEN; 
#endif
#if defined(CONFIG_MIPS_BRCM)
	struct list_head *lh;
	struct net_device * net_dev_ext;
	unsigned char *dst;
	struct sk_buff * skb2;
#endif

	DPRINTK("br2684_push\n");

	if (unlikely(skb == NULL)) {
		/* skb==NULL means VCC is being destroyed */
		br2684_close_vcc(brvcc);
		if (list_empty(&brdev->brvccs)) {
			read_lock(&devs_lock);
			list_del(&brdev->br2684_devs);
			read_unlock(&devs_lock);
			unregister_netdev(net_dev);
			free_netdev(net_dev);
		}
		return;
	}
#if defined(CONFIG_MIPS_BRCM)
	//skb->__unused=FROM_WAN;
#endif	

	atm_return(atmvcc, skb->truesize);
	DPRINTK("skb from brdev %p\n", brdev);
#if defined(CONFIG_BCM_VDSL)
    if ( !g_nPtmTraffic ) {
#endif
		if (brvcc->encaps == e_llc) {
			/* let us waste some time for checking the encapsulation.
			   Note, that only 7 char is checked so frames with a valid FCS
			   are also accepted (but FCS is not checked of course) */
			if (memcmp(skb->data, llc_oui_pid_pad, 7)) {
				brdev->stats.rx_errors++;
				dev_kfree_skb(skb);
				return;
			}

			/* Strip FCS if present */
			if (skb->len > 7 && skb->data[7] == 0x01)
				__skb_trim(skb, skb->len - 4);
		} else {
			plen = PADLEN + ETH_HLEN;	/* pad, dstmac,srcmac, ethtype */
			/* first 2 chars should be 0 */
			if (*((u16 *) (skb->data)) != 0) {
				brdev->stats.rx_errors++;
				dev_kfree_skb(skb);
				return;
			}
		}
		if (skb->len < plen) {
			brdev->stats.rx_errors++;
			dev_kfree_skb(skb);	/* dev_ not needed? */
			return;
		}
#if defined(CONFIG_BCM_VDSL)
  } // !g_nPtmTraffic
#endif

#ifdef SUPPORT_VLAN
	   if (brvcc->vlan_id != 0xffff) { /*  Vcc was configured to be vlan tagged*/
			unsigned tmp[ATM_AND_MAC_LEN];
#ifdef VLAN_DEBUG
			printk("=====> before removing vlan id\n");
			skb_debug(skb);
#endif
    /* 
    ** There exist a situation where we tag vlan id upstream. But DSLAM sends untagged frame downstream.  So we need to check this situation before we move data around
    */
			//eddie added if {}
#if defined(CONFIG_BCM_VDSL)
    if (!g_nPtmTraffic) {
#endif
		    if ( skb->data[22] == 0x81 && skb->data[23] == 0x0) {
				memcpy(tmp, skb->data, ATM_AND_MAC_LEN);
				skb_pull(skb, VLAN_HLEN);
				memcpy(skb->data, tmp, ATM_AND_MAC_LEN);
		    }
#if defined(CONFIG_BCM_VDSL)
    }
    else {
       if ( skb->data[MAC_LEN] == 0x81 && skb->data[MAC_LEN+1] == 0x0) {
          memcpy(tmp, skb->data, MAC_LEN);
          skb_pull(skb, VLAN_HLEN);
          memcpy(skb->data, tmp, MAC_LEN);
       }
    }
#endif
#ifdef VLAN_DEBUG
			printk("=====> after removing vlan id\n");
			skb_debug(skb);
#endif
		}
#endif // SUPPORT_VLAN

#ifdef FASTER_VERSION
	/* FIXME: tcpdump shows that pointer to mac header is 2 bytes earlier,
	   than should be. What else should I set? */
	skb_pull(skb, plen);
	skb->mac.raw = ((char *) (skb->data)) - ETH_HLEN;
	skb->pkt_type = PACKET_HOST;
#ifdef CONFIG_BR2684_FAST_TRANS
	skb->protocol = ((u16 *) skb->data)[-1];
#else				/* some protocols might require this: */
	skb->protocol = br_type_trans(skb, net_dev);
#endif /* CONFIG_BR2684_FAST_TRANS */
#else
	skb_pull(skb, plen - ETH_HLEN);
	skb->protocol = eth_type_trans(skb, net_dev);
#endif /* FASTER_VERSION */
#ifdef CONFIG_ATM_BR2684_IPFILTER
	if (unlikely(packet_fails_filter(skb->protocol, brvcc, skb))) {
		brdev->stats.rx_dropped++;
		dev_kfree_skb(skb);
		return;
	}
#endif /* CONFIG_ATM_BR2684_IPFILTER */
#if defined(CONFIG_MIPS_BRCM)
/*  temp fix
	if (brvcc->proto_filter & FILTER_PPPOE) {
		if ((skb->protocol != htons(ETH_P_PPP_DISC)) && (skb->protocol != htons(ETH_P_PPP_SES))) {
			DPRINTK("non-PPPOE packet dropped on RX dev %s\n", net_dev->name);
			dev_kfree_skb(skb);
			return;
		}
	}
*/
#endif

        skb->dev = net_dev;
	ATM_SKB(skb)->vcc = atmvcc;	/* needed ? */
	DPRINTK("received packet's protocol: %x\n", ntohs(skb->protocol));

	if (unlikely(!(net_dev->flags & IFF_UP))) {
		/* sigh, interface is down */
		brdev->stats.rx_dropped++;
		dev_kfree_skb(skb);
		return;
	}
  
#ifdef PTM_ETH_FCS
#if defined(CONFIG_BCM_VDSL)
	skb->len = (g_nPtmTraffic) ? (skb->len - 4) : skb->len ;
#endif
#endif

	brdev->stats.rx_packets++;
	brdev->stats.rx_bytes += skb->len; 
	memset(ATM_SKB(skb), 0, sizeof(struct atm_skb_data));

	list_for_each(lh, &brdev->net_devexts) {
		net_dev_ext = list_entry_devext(lh);

		dst = eth_hdr(skb)->h_dest;
		if ((dst[0] & 1) || (!memcmp(dst, net_dev_ext->dev_addr, ETH_ALEN)) || (net_dev_ext->br_port)) {

		    skb2 = skb_copy(skb, GFP_ATOMIC);
		    skb2->dev = net_dev_ext;
		    skb2->pkt_type = PACKET_HOST;

		    EXTPRIV(net_dev_ext)->stats.rx_packets++;
		    EXTPRIV(net_dev_ext)->stats.rx_bytes += skb->len; 
		    
    		    netif_rx(skb2);		    
		}
		else 
		    EXTPRIV(net_dev_ext)->stats.rx_dropped++;
	}

	netif_rx(skb);
}

static int br2684_regvcc(struct atm_vcc *atmvcc, void __user *arg)
{
/* assign a vcc to a dev
Note: we do not have explicit unassign, but look at _push()
*/
	int err;
	struct br2684_vcc *brvcc;
	struct sk_buff_head copy;
	struct sk_buff *skb;
	struct br2684_dev *brdev;
	struct net_device *net_dev;
	struct atm_backend_br2684 be;

	if (copy_from_user(&be, arg, sizeof be))
		return -EFAULT;
	brvcc = kzalloc(sizeof(struct br2684_vcc), GFP_KERNEL);
	if (!brvcc)
		return -ENOMEM;
	write_lock_irq(&devs_lock);
	net_dev = br2684_find_dev(&be.ifspec);
	if (net_dev == NULL) {
		printk(KERN_ERR
		    "br2684: tried to attach to non-existant device\n");
		err = -ENXIO;
		goto error;
	}
	brdev = BRPRIV(net_dev);
	if (atmvcc->push == NULL) {
		err = -EBADFD;
		goto error;
	}
	if (!list_empty(&brdev->brvccs)) {
		/* Only 1 VCC/dev right now */
		err = -EEXIST;
		goto error;
	}
	if (be.fcs_in != BR2684_FCSIN_NO || be.fcs_out != BR2684_FCSOUT_NO ||
	    be.fcs_auto || be.has_vpiid || be.send_padding || (be.encaps !=
	    BR2684_ENCAPS_VC && be.encaps != BR2684_ENCAPS_LLC) ||
	    be.min_size != 0) {
		err = -EINVAL;
		goto error;
	}
	DPRINTK("br2684_regvcc vcc=%p, encaps=%d, brvcc=%p\n", atmvcc, be.encaps,
		brvcc);
	if (list_empty(&brdev->brvccs) && !brdev->mac_was_set) {
		unsigned char *esi = atmvcc->dev->esi;
		if (esi[0] | esi[1] | esi[2] | esi[3] | esi[4] | esi[5])
			memcpy(net_dev->dev_addr, esi, net_dev->addr_len);
		else
			net_dev->dev_addr[2] = 1;
	}
	list_add(&brvcc->brvccs, &brdev->brvccs);
	write_unlock_irq(&devs_lock);
	brvcc->device = net_dev;
	brvcc->atmvcc = atmvcc;
	atmvcc->user_back = brvcc;
	brvcc->encaps = (enum br2684_encaps) be.encaps;
	brvcc->old_push = atmvcc->push;
#if defined(CONFIG_MIPS_BRCM)
	brvcc->proto_filter |= be.proto_filter;
#ifdef SUPPORT_VLAN
        brvcc->vlan_id = be.vlan_id;
#endif // SUPPORT_VLAN
#endif	
	barrier();
	atmvcc->push = br2684_push;
	skb_queue_head_init(&copy);
	skb_migrate(&sk_atm(atmvcc)->sk_receive_queue, &copy);
	while ((skb = skb_dequeue(&copy)) != NULL) {
		BRPRIV(skb->dev)->stats.rx_bytes -= skb->len;
		BRPRIV(skb->dev)->stats.rx_packets--;
		br2684_push(atmvcc, skb);
	}
	__module_get(THIS_MODULE);
	return 0;
    error:
	write_unlock_irq(&devs_lock);
	kfree(brvcc);
	return err;
}

static void br2684_setup(struct net_device *netdev)
{
	struct br2684_dev *brdev = BRPRIV(netdev);

	ether_setup(netdev);
	brdev->net_dev = netdev;

#ifdef FASTER_VERSION
	my_eth_header = netdev->hard_header;
	netdev->hard_header = br2684_header;
	my_eth_header_cache = netdev->hard_header_cache;
	netdev->hard_header_cache = br2684_header_cache;
	netdev->hard_header_len = sizeof(llc_oui_pid_pad) + ETH_HLEN;	/* 10 + 14 */
#endif
	my_eth_mac_addr = netdev->set_mac_address;
	netdev->set_mac_address = br2684_mac_addr;
	netdev->hard_start_xmit = br2684_start_xmit;
	netdev->get_stats = br2684_get_stats;

	INIT_LIST_HEAD(&brdev->brvccs);
}

static int br2684_create(void __user *arg, atm_backend_t b)
{
	int err;
	struct net_device *netdev;
	struct br2684_dev *brdev;
	struct atm_newif_br2684 ni;

	DPRINTK("br2684_create\n");

	if (copy_from_user(&ni, arg, sizeof ni)) {
		return -EFAULT;
	}
	if (ni.media != BR2684_MEDIA_ETHERNET || ni.mtu != 1500) {
		return -EINVAL;
	}

	netdev = alloc_netdev(sizeof(struct br2684_dev),
			      ni.ifname[0] ? ni.ifname : "nas%d",
			      br2684_setup);
	if (!netdev)
		return -ENOMEM;

	if (b == ATM_BACKEND_BR2684_BCM)
		netdev->hard_start_xmit = br2684_start_xmit_bcm;

	brdev = BRPRIV(netdev);

#if defined(CONFIG_MIPS_BRCM)
	brdev->net_dev->priv = brdev;
	INIT_LIST_HEAD(&brdev->net_devexts);
#endif

	DPRINTK("registered netdev %s\n", netdev->name);
	/* open, stop, do_ioctl ? */
	err = register_netdev(netdev);
	if (err < 0) {
		printk(KERN_ERR "br2684_create: register_netdev failed\n");
		free_netdev(netdev);
		return err;
	}

	write_lock_irq(&devs_lock);
	brdev->number = list_empty(&br2684_devs) ? 1 :
	    BRPRIV(list_entry_brdev(br2684_devs.prev))->number + 1;
	list_add_tail(&brdev->br2684_devs, &br2684_devs);
	write_unlock_irq(&devs_lock);
	return 0;
}

#if defined(CONFIG_MIPS_BRCM)
static int br2684_ext(void __user *arg, atm_backend_t b)
{
	int err;
	struct br2684_dev *brdev;
	struct atm_newif_br2684 ni;
	struct net_device *net_dev;
	struct net_devext * net_dev_ext;
	struct br2684_if_spec s;

	DPRINTK("br2684_ext\n");

	if (copy_from_user(&ni, (void *) arg, sizeof ni)) {
		return -EFAULT;
	}

	s.method = BR2684_FIND_BYIFNAME;
	sprintf(s.spec.ifname, "%s%s", "", ni.ifname); // To pass the link error;
	net_dev = br2684_find_dev(&s);
	if (net_dev == NULL) {
		printk(KERN_ERR
		    "br2684: tried to attach to non-existant device\n");
		return -ENXIO;
	}
	brdev = BRPRIV(net_dev);
	brdev->net_dev->priv = brdev;
	if ((net_dev_ext = kmalloc(sizeof(struct net_devext), GFP_KERNEL)) == NULL)
		return -ENOMEM;
	memset(net_dev_ext, 0, sizeof(struct net_devext));
	ether_setup(&net_dev_ext->net_dev);
	sprintf(net_dev_ext->net_dev.name, "%s_%d", brdev->net_dev->name, ni.media);
	net_dev_ext->net_dev.atalk_ptr = net_dev_ext;
	net_dev_ext->net_dev.priv = brdev;
	net_dev_ext->net_dev.set_mac_address = br2684_mac_addr;
	if (b == ATM_BACKEND_BR2684_BCM)
		net_dev_ext->net_dev.hard_start_xmit = br2684_start_xmit_bcm;
	else
		net_dev_ext->net_dev.hard_start_xmit = br2684_start_xmit;
	net_dev_ext->net_dev.get_stats = br2684_get_stats;
	err = register_netdev(&net_dev_ext->net_dev);

	write_lock_irq(&devs_lock);
	list_add(&net_dev_ext->list, &brdev->net_devexts);
	write_unlock_irq(&devs_lock);

	return 0;
}
#endif

/*
 * This handles ioctls actually performed on our vcc - we must return
 * -ENOIOCTLCMD for any unrecognized ioctl
 */
static int br2684_ioctl(struct socket *sock, unsigned int cmd,
	unsigned long arg)
{
	struct atm_vcc *atmvcc = ATM_SD(sock);
	void __user *argp = (void __user *)arg;

	int err;
	switch(cmd) {
#if defined(CONFIG_MIPS_BRCM)
	case ATM_EXTBACKENDIF:
        case ATM_SETEXTFILT:
#endif
	case ATM_SETBACKEND:
	case ATM_NEWBACKENDIF: {
		atm_backend_t b;
		err = get_user(b, (atm_backend_t __user *) argp);
		if (err)
			return -EFAULT;
		if (b != ATM_BACKEND_BR2684 && b != ATM_BACKEND_BR2684_BCM)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
#if defined(CONFIG_MIPS_BRCM)
		if (cmd == ATM_SETBACKEND) {
			if( b == ATM_BACKEND_BR2684_BCM)
				return br2684_attachvcc(atmvcc, argp);
			else
				return br2684_regvcc(atmvcc, argp);
		}
		else if (cmd == ATM_NEWBACKENDIF)
			return br2684_create(argp, b);
		else if (cmd == ATM_EXTBACKENDIF)
			return br2684_ext(argp, b);
                else
                        return bcm_set_proto_filter(atmvcc, argp);
#else
		if (cmd == ATM_SETBACKEND)
			return br2684_regvcc(atmvcc, argp);
		else
			return br2684_create(argp, b);
#endif
		}
#ifdef CONFIG_ATM_BR2684_IPFILTER
	case BR2684_SETFILT:
		if (atmvcc->push != br2684_push)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		err = br2684_setfilt(atmvcc, argp);
		return err;
#endif /* CONFIG_ATM_BR2684_IPFILTER */
	}
	return -ENOIOCTLCMD;
}

static struct atm_ioctl br2684_ioctl_ops = {
	.owner	= THIS_MODULE,
	.ioctl	= br2684_ioctl,
};


#ifdef CONFIG_PROC_FS
static void *br2684_seq_start(struct seq_file *seq, loff_t *pos)
{
	loff_t offs = 0;
	struct br2684_dev *brd;

	read_lock(&devs_lock);

	list_for_each_entry(brd, &br2684_devs, br2684_devs) {
		if (offs == *pos)
			return brd;
		++offs;
	}
	return NULL;
}

static void *br2684_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct br2684_dev *brd = v;

	++*pos;

	brd = list_entry(brd->br2684_devs.next, 
			 struct br2684_dev, br2684_devs);
	return (&brd->br2684_devs != &br2684_devs) ? brd : NULL;
}

static void br2684_seq_stop(struct seq_file *seq, void *v)
{
	read_unlock(&devs_lock);
}

static int br2684_seq_show(struct seq_file *seq, void *v)
{
	const struct br2684_dev *brdev = v;
	const struct net_device *net_dev = brdev->net_dev;
	const struct br2684_vcc *brvcc;

	seq_printf(seq, "dev %.16s: num=%d, mac=%02X:%02X:"
		       "%02X:%02X:%02X:%02X (%s)\n", net_dev->name,
		       brdev->number,
		       net_dev->dev_addr[0],
		       net_dev->dev_addr[1],
		       net_dev->dev_addr[2],
		       net_dev->dev_addr[3],
		       net_dev->dev_addr[4],
		       net_dev->dev_addr[5],
		       brdev->mac_was_set ? "set" : "auto");

	list_for_each_entry(brvcc, &brdev->brvccs, brvccs) {
		seq_printf(seq, "  vcc %d.%d.%d: encaps=%s"
#ifndef FASTER_VERSION
				    ", failed copies %u/%u"
#endif /* FASTER_VERSION */
				    "\n", (brvcc->atmvcc->dev) ? brvcc->atmvcc->dev->number : 0,
				    brvcc->atmvcc->vpi, brvcc->atmvcc->vci,
				    (brvcc->encaps == e_llc) ? "LLC" : "VC"
#ifndef FASTER_VERSION
				    , brvcc->copies_failed
				    , brvcc->copies_needed
#endif /* FASTER_VERSION */
				    );
#ifdef CONFIG_ATM_BR2684_IPFILTER
#define b1(var, byte)	((u8 *) &brvcc->filter.var)[byte]
#define bs(var)		b1(var, 0), b1(var, 1), b1(var, 2), b1(var, 3)
			if (brvcc->filter.netmask != 0)
				seq_printf(seq, "    filter=%d.%d.%d.%d/"
						"%d.%d.%d.%d\n",
						bs(prefix), bs(netmask));
#undef bs
#undef b1
#endif /* CONFIG_ATM_BR2684_IPFILTER */
	}
	return 0;
}

static struct seq_operations br2684_seq_ops = {
	.start = br2684_seq_start,
	.next  = br2684_seq_next,
	.stop  = br2684_seq_stop,
	.show  = br2684_seq_show,
};

static int br2684_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &br2684_seq_ops);
}

static struct file_operations br2684_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = br2684_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

extern struct proc_dir_entry *atm_proc_root;	/* from proc.c */
#endif

static int __init br2684_init(void)
{
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *p;
	if ((p = create_proc_entry("br2684", 0, atm_proc_root)) == NULL)
		return -ENOMEM;
	p->proc_fops = &br2684_proc_ops;
#endif
	register_atm_ioctl(&br2684_ioctl_ops);
	return 0;
}

static void __exit br2684_exit(void)
{
	struct net_device *net_dev;
	struct br2684_dev *brdev;
	struct br2684_vcc *brvcc;
	deregister_atm_ioctl(&br2684_ioctl_ops);

#ifdef CONFIG_PROC_FS
	remove_proc_entry("br2684", atm_proc_root);
#endif

	while (!list_empty(&br2684_devs)) {
		net_dev = list_entry_brdev(br2684_devs.next);
		brdev = BRPRIV(net_dev);
		while (!list_empty(&brdev->brvccs)) {
			brvcc = list_entry_brvcc(brdev->brvccs.next);
			br2684_close_vcc(brvcc);
		}

		list_del(&brdev->br2684_devs);
		unregister_netdev(net_dev);
		free_netdev(net_dev);
	}
}


#if defined(CONFIG_MIPS_BRCM)
#define ATM_NUM_RX_SKBS				200
#define SKB_ALIGNED_SIZE            ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)

extern int rfc2684InMirrorStatus;
extern int rfc2684OutMirrorStatus;
extern char mirrorInPort[];
extern char mirrorOutPort[];

static rwlock_t devs_lock_tx = RW_LOCK_UNLOCKED;
static unsigned char g_atm_rx_skbs[(ATM_NUM_RX_SKBS * SKB_ALIGNED_SIZE) + 0x10];
struct sk_buff *g_free_atm_rx_skbs = (struct sk_buff *) -1;

void br2684_mirror_packet( struct sk_buff *pSockBuf, UINT32 dir, char *dev );

/***************************************************************************
 * Function Name: br2684_attachvcc
 * Description  : Called by the Linux pvc2684 user mode application to use
 *                assign a PVC to a network interface.  It initializes data
 *                structures used by the Broadcom ATM API and calls the
 *                the Broadcom ATM driver function BcmAtm_AttachVcc to
 *                reserve use of the PVC.
 * Returns      : Status.
 ***************************************************************************/
static int br2684_attachvcc(struct atm_vcc *atmvcc, void __user *arg)
{
    int ret = 0;
    int i;
    unsigned char *p;
    struct br2684_vcc *brvcc;
    struct sk_buff *skb;
    struct br2684_dev *brdev;
    struct net_device *net_dev;
    struct atm_backend_br2684 be;
    ATM_VCC_ADDR vcc_addr;
    ATM_VCC_CFG vcc_cfg;
    ATM_VCC_ATTACH_PARMS vcc_ap;
    int atm_tx_mem_size;

    if( copy_from_user(&be, arg, sizeof be) == 0 )
    {
        if( (brvcc = kmalloc(sizeof(struct br2684_vcc), GFP_KERNEL)) != NULL )
        {
            memset(brvcc, 0, sizeof(struct br2684_vcc));
            write_lock_irq(&devs_lock);
            net_dev = br2684_find_dev(&be.ifspec);
            if( net_dev )
            {
                brdev = BRPRIV(net_dev);

                DPRINTK("br2684_attachvcc 1. vcc=%p, encaps=%d, brvcc=%p, "
                    "brdev=%p\n", atmvcc, be.encaps, brvcc, brdev);

                if (list_empty(&brdev->brvccs) && !brdev->mac_was_set)
                {
                    /* TBD. How to set MAC address. */
                    memcpy(net_dev->dev_addr, "\x02\x10\x18\x10\x10\x01", 6);
                }

                list_add(&brvcc->brvccs, &brdev->brvccs);
                write_unlock_irq(&devs_lock);

                brvcc->device = net_dev;
                brvcc->atmvcc = atmvcc;
                brvcc->encaps = (enum br2684_encaps) be.encaps;
                brvcc->proto_filter |= be.proto_filter;
                brvcc->old_push = atmvcc->push;
                barrier();
                atmvcc->push = br2684_push_bcm;
                atmvcc->dev = atm_dev_lookup(0);
#ifdef SUPPORT_VLAN
                brvcc->vlan_id = be.vlan_id;
#endif // SUPPORT_VLAN
                atmvcc->user_back = brvcc;

                if( g_free_atm_rx_skbs == (struct sk_buff *) -1 )
                {
                    /* Initialize Linux receive socket buffers. */
                    g_free_atm_rx_skbs = NULL;
                    for( i = 0, p = (unsigned char *)
                        (((unsigned long) g_atm_rx_skbs + 0x0f) & ~0x0f);
                        i < ATM_NUM_RX_SKBS; i++, p += SKB_ALIGNED_SIZE )
                    {
                        skb = (struct sk_buff *) p;
                        skb->retfreeq_context = g_free_atm_rx_skbs;
                        g_free_atm_rx_skbs = skb;
                    }
                }

                /* Allocate and initialize Broadcom ATM API transmit structs. */
                BcmAtm_GetInterfaceId( 0, &vcc_addr.ulInterfaceId );
                vcc_addr.usVpi = (unsigned short) (be.has_vpiid >> 16);
                vcc_addr.usVci = (unsigned short) (be.has_vpiid & 0xffff);

                brvcc->atm_num_tx_dps = 0;
                if( BcmAtm_GetVccCfg(&vcc_addr, &vcc_cfg) == STS_SUCCESS )
                {
                    if( vcc_cfg.ulTransmitQParmsSize == 0 )
                        brvcc->atm_num_tx_dps = 80;
                    else
                    {
                        for( i = 0; i < vcc_cfg.ulTransmitQParmsSize; i++ )
                        {
                            brvcc->atm_num_tx_dps +=
                                vcc_cfg.TransmitQParms[i].ulSize;
                        }
                    }
                }
                else
                    brvcc->atm_num_tx_dps = 80;

                atm_tx_mem_size =  brvcc->atm_num_tx_dps *
                    (sizeof(ATM_VCC_DATA_PARMS) + sizeof(ATM_BUFFER));
                brvcc->atm_tx_mem = kmalloc(atm_tx_mem_size, GFP_KERNEL);
                if( brvcc->atm_tx_mem != NULL )
                {
                    PATM_VCC_DATA_PARMS pDp;
                    PATM_BUFFER pAb;

                    memset(brvcc->atm_tx_mem, 0x00, atm_tx_mem_size); 
                    INIT_LIST_HEAD(&brvcc->atm_list_tx_dps);

                    for(i = 0, p = brvcc->atm_tx_mem; i < brvcc->atm_num_tx_dps;
                        i++, p += sizeof(ATM_VCC_DATA_PARMS)+sizeof(ATM_BUFFER))
                    {
                        pDp = (PATM_VCC_DATA_PARMS) p;
                        pAb = (PATM_BUFFER) (pDp + 1);

                        pDp->ulStructureId = ID_ATM_VCC_DATA_PARMS;
                        pDp->ucCircuitType = CT_AAL5;
                        pDp->ucSendPriority = ANY_PRIORITY;
                        pDp->pAtmBuffer = pAb;
                        pDp->pFnFreeDataParms = br2684_free_dp;
                        pDp->ulParmFreeDataParms = (UINT32) brvcc;

                        list_add_tail((struct list_head *)
                            pDp->ulApplicationDefined, &brvcc->atm_list_tx_dps);
                    }

                    /* Attach to VCC. */
                    memset(&vcc_ap, 0x00, sizeof(vcc_ap));
                    vcc_ap.ulStructureId = ID_ATM_VCC_ATTACH_PARMS;
                    vcc_ap.pFnReceiveDataCb = (FN_RECEIVE_CB) br2684_rx_cb;
                    vcc_ap.ulParmReceiveData = (unsigned long) brvcc;
                    if( BcmAtm_AttachVcc( &vcc_addr, &vcc_ap ) == STS_SUCCESS )
                        brvcc->atm_attach_handle = vcc_ap.ulHandle;
                    else
                    {
                        kfree(brvcc->atm_tx_mem);
                        brvcc->atm_tx_mem = NULL;

                        ret = -EIO;
                    }
                }
                else
                    ret = -ENOMEM;
            }
            else
            {
                write_unlock_irq(&devs_lock);
                printk(KERN_ERR
                    "br2684: tried to attach to non-existant device\n");
                ret = -ENXIO;
            }
        }
        else
            ret = -ENOMEM;
    }
    else
        ret = -EFAULT;

    DPRINTK("br2684_attachvcc retruns %d\n",ret);
    return( ret );
}

/***************************************************************************
 * Function Name: br2684_start_xmit
 * Description  : Called by the Linux network stack to send a packet on an
 *                ATM connection.  This function adds the RFC2684 header and
 *                calls the Broadcom ATM driver function to send the packet.
 * Returns      : Status.
 ***************************************************************************/
static int br2684_start_xmit_bcm(struct sk_buff *skb, struct net_device *dev)
{
    struct br2684_dev *brdev = BRPRIV(dev);
    struct br2684_vcc *brvcc;
#if defined(CONFIG_MIPS_BRCM)
	struct net_devext *devext = EXTPRIV(dev);
#endif

    read_lock(&devs_lock_tx);
    if( !list_empty(&brdev->brvccs) )
    {
        brvcc = list_entry_brvcc(brdev->brvccs.next);
        
        /* 
         * Avoid transmission of non-pppoe packets on pppoe inteface
         * when MSP enabled
         */
        if(((((brvcc->proto_filter & FILTER_PPPOE) == FILTER_PPPOE) && 
             (NULL == devext)) ||
           ((NULL != devext) && 
            ((devext->proto_filter & FILTER_PPPOE) == FILTER_PPPOE))) &&
           (skb->protocol != htons(ETH_P_PPP_DISC)) &&
           (skb->protocol != htons(ETH_P_PPP_SES)))
        {
            DPRINTK("non-PPPOE packet dropped on TX dev %s\n", dev->name);
            dev_kfree_skb(skb);
            read_unlock(&devs_lock);
            return 0;
        }

        if( rfc2684OutMirrorStatus == MIRROR_ENABLED )
            br2684_mirror_packet( skb, DIR_OUT, mirrorOutPort );

#if defined(CONFIG_BCM_VDSL)
        if( g_nPtmTraffic )
        {
            if (skb->len < MIN_PKT_SIZE)
            {
                struct sk_buff *skb2=skb_copy_expand(skb, 0, MIN_PKT_SIZE -
                    skb->len, GFP_ATOMIC);
                dev_kfree_skb(skb);
                if (skb2 == NULL)
                {
                    brvcc->copies_failed++;
                    skb = NULL;
                }
                else
                {
                    skb = skb2;
                    memset(skb->tail, 0, MIN_PKT_SIZE - skb->len);
                    skb_put(skb, MIN_PKT_SIZE - skb->len);
                }
            }
        }
        else
#endif
        {
            int minheadroom = (brvcc->encaps == e_llc) ?
                sizeof(llc_oui_pid_pad) : PADLEN;
            int headroom = skb_headroom(skb);

            if (headroom < minheadroom)
            {
                struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);

                brvcc->copies_needed++;
                DPRINTK("br2684_start_xmit_bcm realloc headroom %d < %d\n", \
                    headroom, minheadroom);
                dev_kfree_skb(skb);
                if (skb2 == NULL)
                {
                    brvcc->copies_failed++;
                    skb = NULL;
                }
                else
                    skb = skb2;
            }
            if( skb )
            {
                int skblen;

                skb_push(skb, minheadroom);
                if( minheadroom == sizeof(llc_oui_pid_pad) )
                    memcpy(skb->data, llc_oui_pid_pad, sizeof(llc_oui_pid_pad));
                else
                    skb->data[0] = skb->data[1] = 0x00;
                skblen = skb->len;
                if (skblen < MIN_PKT_SIZE)
                {
                    struct sk_buff *skb2=skb_copy_expand(skb, 0, MIN_PKT_SIZE -
                        skb->len, GFP_ATOMIC);

                    DPRINTK("br2684_start_xmit_bcm copy expand, %d < %d\n", \
                        skblen, MIN_PKT_SIZE);
                    dev_kfree_skb(skb);
                    if (skb2 == NULL)
                    {
                        brvcc->copies_failed++;
                        skb = NULL;
                    }
                    else
                    {
                        skb = skb2;
                        memset(skb->tail, 0, MIN_PKT_SIZE - skb->len);
                        skb_put(skb, MIN_PKT_SIZE - skb->len);
                    }
                }
            }
        }

        if( skb )
        {
            int atmPriority = 0;
            PATM_VCC_DATA_PARMS pDp;
            PATM_BUFFER pAb;

            if( !list_empty(&brvcc->atm_list_tx_dps) )
            {
                pDp = list_entry((void *) brvcc->atm_list_tx_dps.next, \
                    ATM_VCC_DATA_PARMS, ulApplicationDefined);
                list_del((struct list_head *) &pDp->ulApplicationDefined[0]);

                pDp->ulApplicationDefined[0] = (UINT32) skb;
                pDp->ulParmFreeDataParms = (UINT32) brvcc;
                pAb = pDp->pAtmBuffer;
                pAb->pDataBuf = skb->data;
                pAb->ulDataLen = (UINT32) skb->len;

#ifdef CONFIG_NETFILTER
                atmPriority = skb->nfmark & 0x0F;
                /* bit 3-0 of the 32-bit nfmark is the atm priority,
                 *    set by iptables
                 * bit 7-4 is the Ethernet switch physical port number,
                 *    set by lan port drivers.
                 * bit 8-10 is the wanVlan priority bits
                 */
                if (atmPriority <= 4 && atmPriority >= 1)
                    pDp->ucSendPriority = atmPriority;
                else if (skb->priority <= 4 && skb->priority >= 1)
                    pDp->ucSendPriority = skb->priority;
                else
                    pDp->ucSendPriority = ANY_PRIORITY;
#else
                if (skb->priority <= 4 && skb->priority >= 1)
                    pDp->ucSendPriority = skb->priority;
                else
                    pDp->ucSendPriority = ANY_PRIORITY;
#endif

                /* Send the data using the Broadcom ATM API. */
                if( BcmAtm_SendVccData( brvcc->atm_attach_handle, pDp ) ==
                    STS_SUCCESS )
                {
#if defined(CONFIG_MIPS_BRCM)
                    if (devext) {
    	                devext->stats.tx_packets++;
	                devext->stats.tx_bytes += skb->len;	    
	            }
	            else {
	                brdev->stats.tx_packets++;
	                brdev->stats.tx_bytes += skb->len;
	            }
#else
	            brdev->stats.tx_packets++;
	            brdev->stats.tx_bytes += skb->len;
#endif
                }
                else
                {
                    brdev->stats.tx_dropped++;
                    dev_kfree_skb(skb);
                    list_add_tail((struct list_head *)pDp->ulApplicationDefined,
                        &brvcc->atm_list_tx_dps);
                }
            }
            else
            {
                brdev->stats.tx_dropped++;
                dev_kfree_skb(skb);
            }
        }
    }
    else
    {
        DPRINTK("no vcc attached to dev %s\n", dev->name);
#if defined(CONFIG_MIPS_BRCM)
        if (devext) {
            devext->stats.tx_errors++;
            devext->stats.tx_carrier_errors++;
        } else {
            brdev->stats.tx_errors++;
            brdev->stats.tx_carrier_errors++;
        }
#else
        brdev->stats.tx_errors++;
        brdev->stats.tx_carrier_errors++;
#endif
        dev_kfree_skb(skb);
    }

    read_unlock(&devs_lock_tx);

    return( 0 );
}

/***************************************************************************
 * Function Name: br2684_free_dp
 * Description  : Called by the Broadcom ATM driver after it has sent a
 *                packet on an ATM PVC connection.
 * Returns      : None.
 ***************************************************************************/
static void br2684_free_dp( PATM_VCC_DATA_PARMS pDp )
{
    struct sk_buff *skb = (struct sk_buff *) pDp->ulApplicationDefined[0];
    struct br2684_vcc *brvcc = (struct br2684_vcc *) pDp->ulParmFreeDataParms;

    if( skb )
        dev_kfree_skb_any( skb );

    if( brvcc )
    {
        read_lock(&devs_lock_tx);
        list_add_tail((struct list_head *) pDp->ulApplicationDefined,
            &brvcc->atm_list_tx_dps);
        read_unlock(&devs_lock_tx);
    }
}

/***************************************************************************
 * Function Name: br2684_rx_cb
 * Description  : Called by the Broadcom ATM driver after it has received a
 *                packet on an ATM PVC connection.
 * Returns      : None.
 ***************************************************************************/
static void br2684_rx_cb( UINT32 ulHandle, PATM_VCC_ADDR pVccAddr,
    PATM_VCC_DATA_PARMS pDp, struct br2684_vcc *brvcc )
{
    struct br2684_dev *brdev = BRPRIV(brvcc->device);
    struct sk_buff *skb;
    PATM_BUFFER pAb = pDp->pAtmBuffer;
#if defined(CONFIG_MIPS_BRCM)
    struct list_head *lh;
    struct net_device * net_dev_ext;
    unsigned char *dstAddr;
    struct sk_buff * skb2;
    int isPktTxed = 0;
    struct net_devext *devext;
    struct net_device * net_dev_match = NULL;
#endif
    

    /* Remove RFC2684 header. */
    unsigned long padlen = (brvcc->encaps == e_llc)
        ? sizeof(llc_oui_pid_pad) : PADLEN;
#if !defined(CONFIG_BCM_VDSL)
    unsigned char *databuf = pAb->pDataBuf + padlen;
    unsigned long datalen = pAb->ulDataLen - padlen;
    unsigned long dataoffset = pAb->usDataOffset + padlen;
    if( padlen == sizeof(llc_oui_pid_pad) && pAb->pDataBuf[7] == 0x01)
        datalen -= 4; /* strip FCS */
#else
    unsigned char *databuf = pAb->pDataBuf;
    unsigned long datalen = pAb->ulDataLen;
    unsigned long dataoffset = pAb->usDataOffset;
    if( !g_nPtmTraffic )
    {
        databuf += padlen;
        datalen -= padlen;
        dataoffset += padlen;
        if( padlen == sizeof(llc_oui_pid_pad) && pAb->pDataBuf[7] == 0x01)
            datalen -= 4; /* strip FCS */
    }
#endif

    read_lock(&devs_lock);

    skb = g_free_atm_rx_skbs;
    if( skb )
    {
        /* Get a socket buffer from the linked list of available skbs.*/
        g_free_atm_rx_skbs = skb->retfreeq_context;
        read_unlock(&devs_lock);

        skb_hdrinit(dataoffset, ETH_FRAME_LEN, skb, databuf,
            (void *) br2684_free_skb_or_data, pDp, FROM_WAN);
        __skb_trim(skb, datalen);

        if( rfc2684InMirrorStatus == MIRROR_ENABLED )
            br2684_mirror_packet( skb, DIR_IN, mirrorInPort );

        skb->dev = brvcc->device;
        skb->protocol = eth_type_trans(skb, brvcc->device);

#if defined(SUPPORT_VLAN)
        if (brvcc->vlan_id != 0xffff)
        {
            /* There exist a situation where we tag vlan id upstream. But DSLAM
             * sends untagged frame downstream.  So we need to check this
             * situation before we move data around.
             */
            if( skb->data[MAC_LEN] == 0x81 && skb->data[MAC_LEN+1] == 0x0 )
            {
                int i;
                UINT8 *src = skb->data + MAC_LEN;
                UINT8 *dst = skb->data + MAC_LEN + VLAN_HLEN;

                for( i = 0; i < MAC_LEN; i++ )
                    *--dst = *--src;
                __skb_pull(skb, VLAN_HLEN);
            }
        }
#endif

        /* if MSP is not configured follow the old code path */
        if(((brvcc->proto_filter & FILTER_PPPOE) == FILTER_PPPOE) &&
           (list_empty(&brdev->net_devexts)) &&
           (skb->protocol != htons(ETH_P_PPP_DISC)) &&
           (skb->protocol != htons(ETH_P_PPP_SES)))
        {
            DPRINTK("non-PPPOE packet dropped on RX dev %s\n",
                                                   brvcc->device->name);
            dev_kfree_skb(skb);
        }
        else
        {

#if defined(CONFIG_MIPS_BRCM)
            /* no MSP here */
            if(list_empty(&brdev->net_devexts)) {
                brdev->stats.rx_packets++;
                brdev->stats.rx_bytes += skb->len;
                netif_rx(skb);
            } 
            else {
                isPktTxed = 0;
                dstAddr = eth_hdr(skb)->h_dest;
            
                if((dstAddr[0] & 1) ||
                   (0 == bcm_is_pppoe_passthrough_pkt(brdev, skb, &net_dev_match))) {
                    list_for_each(lh, &brdev->net_devexts) {
                        net_dev_ext = list_entry_devext(lh);
                        devext = EXTPRIV(net_dev_ext);
                        skb2 = skb_copy(skb, GFP_ATOMIC);
                        skb2->dev = net_dev_ext;
                        skb2->pkt_type = PACKET_HOST;

                        EXTPRIV(net_dev_ext)->stats.rx_packets++;
                        EXTPRIV(net_dev_ext)->stats.rx_bytes += skb->len; 
                        netif_rx(skb2);
                    }
                    brdev->stats.rx_packets++;
                    brdev->stats.rx_bytes += skb->len;
                    netif_rx(skb);
                    isPktTxed = 1;
                } 
                else if(net_dev_match) {
                    devext = EXTPRIV(net_dev_match);
                    if(((((brvcc->proto_filter & FILTER_PPPOE) == FILTER_PPPOE) && 
                         (NULL == devext)) ||
                        ((NULL != devext) && 
                         ((devext->proto_filter & FILTER_PPPOE) == FILTER_PPPOE))) &&
                         (skb->protocol != htons(ETH_P_PPP_DISC)) &&
                         (skb->protocol != htons(ETH_P_PPP_SES))) {
                        DPRINTK("non-PPPOE packet dropped on RX dev %s\n", net_dev_match->name);
                        dev_kfree_skb(skb);
                    }
                    else {
                        if(skb->dev != net_dev_match) {
                            skb->dev = net_dev_match;
                            skb->pkt_type = PACKET_HOST;
                            EXTPRIV(net_dev_match)->stats.rx_packets++;
                            EXTPRIV(net_dev_match)->stats.rx_bytes += skb->len; 
                        } 
                        else {
                            brdev->stats.rx_packets++;
                            brdev->stats.rx_bytes += skb->len;
                        }
                        netif_rx(skb);
                    }
                    isPktTxed = 1;
                }
                else if(0 == isPktTxed) {
                    if((skb->dev->br_port) && (brvcc->proto_filter != FILTER_PPPOE)) {
                        brdev->stats.rx_packets++;
                        brdev->stats.rx_bytes += skb->len;
                        netif_rx(skb);
                        isPktTxed = 1; 
                    }
                    else {
                        list_for_each(lh, &brdev->net_devexts) {
                            net_dev_ext = list_entry_devext(lh);
                            devext = EXTPRIV(net_dev_ext);
                            if((devext->proto_filter != FILTER_PPPOE) &&
                               (net_dev_ext->br_port)) {
                                skb->dev =net_dev_ext; 
                                skb->pkt_type = PACKET_HOST;
                                EXTPRIV(net_dev_ext)->stats.rx_packets++;
                                EXTPRIV(net_dev_ext)->stats.rx_bytes += skb->len; 
                                netif_rx(skb);
                                isPktTxed = 1; 
                                break;
                            }
                        }
                    }
                }
                else if(0 == isPktTxed) {
                    DPRINTK("dropping packet that has wrong dest. on RX dev %s\n",
                                                                brvcc->device->name);
                    dev_kfree_skb(skb);
                    brdev->stats.rx_dropped++;
                }
            }
        }
#else
            brdev->stats.rx_packets++;
            brdev->stats.rx_bytes += skb->len;
            netif_rx(skb);
        }
#endif
    }
    else
    {
        read_unlock(&devs_lock);
        DPRINTK("br2684_rx_cb skb == NULL\n");
        (*pDp->pFnFreeDataParms) (pDp);
    }
}

/***************************************************************************
 * Function Name: br2684_free_skb_or_data
 * Description  : This callback function returns the socket buffer header or
 *                socket buffer data that is no longer being used by the upper
 *                network layers.
 * Returns      : None.
 ***************************************************************************/
static void br2684_free_skb_or_data( PATM_VCC_DATA_PARMS pDp, void *pObj,
    int nFlag )
{
    if( nFlag == FREE_SKB )
    {
        struct sk_buff *skb = (struct sk_buff *) pObj;

        read_lock(&devs_lock);
        skb->retfreeq_context = g_free_atm_rx_skbs;
        g_free_atm_rx_skbs = skb;
        read_unlock(&devs_lock);
    }
    else
        (*pDp->pFnFreeDataParms)(pDp);
}

/***************************************************************************
 * Function Name: br2684_push_bcm
 * Description  : This function should only be called when an ATM socket is
 *                being closed. When this occurs, the socket buffer, skb,
 *                will be NULL. It should never be called with a non-NULL skb.
 * Returns      : None.
 ***************************************************************************/
static void br2684_push_bcm(struct atm_vcc *atmvcc, struct sk_buff *skb)
{
    struct br2684_vcc *brvcc = BR2684_VCC(atmvcc);
    struct net_device *net_dev = brvcc->device;
    struct br2684_dev *brdev = BRPRIV(net_dev);

    if (likely(skb == NULL))
    {
        /* skb==NULL means VCC is being destroyed */
        if( brvcc->atm_attach_handle )
        {
            BcmAtm_Detach( brvcc->atm_attach_handle );
            brvcc->atm_attach_handle = 0;
        }

        br2684_close_vcc(brvcc);

        if (list_empty(&brdev->brvccs))
        {
            read_lock(&devs_lock);
            list_del(&brdev->br2684_devs);
            read_unlock(&devs_lock);
            unregister_netdev(net_dev);
            free_netdev(net_dev);
        }
    }
    else
    {
        /* This should not happen. */
        printk("br2684: br2684_push_bcm called with non-NULL skb\n");
        dev_kfree_skb(skb);
    }
}

/***************************************************************************
 * Function Name: br2684_mirror_packet
 * Description  : This function sends a sent or received packet to a LAN port.
 *                The purpose is to allow packets sent and received on the WAN
 *                to be captured by a protocol analyzer on the Lan for debugging
 *                purposes.
 * Returns      : None.
 ***************************************************************************/
void br2684_mirror_packet( struct sk_buff *pSockBuf, UINT32 dir, char *dev )
{
    struct sk_buff *pCloneBuf;
    struct net_device *netDev;

    if( (pCloneBuf = skb_clone(pSockBuf, GFP_ATOMIC)) != NULL )
    {
        if( (netDev = __dev_get_by_name(dev)) != NULL )
        {
            unsigned long flags;

            pCloneBuf->dev = netDev;
            pCloneBuf->protocol = htons(ETH_P_802_3);
            local_irq_save(flags);
            local_irq_enable();
            dev_queue_xmit(pCloneBuf) ;
            local_irq_restore(flags);
        }
        else
            dev_kfree_skb(pCloneBuf);
    }
}
/***************************************************************************
 * Function Name: bcm_set_proto_filter
 * Description  : set the pppoe filter for non-primary service
 *               
 * Returns      : Status.
 ***************************************************************************/
static int bcm_set_proto_filter(struct atm_vcc *atmvcc, void __user *arg)
{
    int ret = 0;
    struct br2684_dev *brdev;
    struct net_device *net_dev;
    struct atm_backend_br2684 be;
    struct net_devext *devext;
    struct net_device * net_dev_ext;
    struct list_head *lh;

    if( copy_from_user(&be, arg, sizeof be) == 0 )
    {
	read_lock(&devs_lock);
        net_dev = br2684_find_dev(&be.ifspec);
        if( net_dev )
        {
            brdev = BRPRIV(net_dev);
            list_for_each(lh, &brdev->net_devexts) {
                net_dev_ext = list_entry_devext(lh);
                devext = EXTPRIV(net_dev_ext);
                devext->proto_filter |= be.proto_filter;
           }
         }
         else
         {
                printk(KERN_ERR
                    "br2684: tried to configure on non-existant device\n");
                ret = -ENXIO;
         }
         read_unlock(&devs_lock);
    }
    else
        ret = -EFAULT;

    DPRINTK("bcm_set_proto_filter %d\n",ret);
    return( ret );
}

/***************************************************************************
 * Function Name: bcm_is_pppoe_passthrough_pkt
 * Description  : checks if the packet is pppoe passthrough by checking 
 *                all MAC addresses
 *               
 * Returns      : 0 if it is pppoe passthrough
 ***************************************************************************/
static int bcm_is_pppoe_passthrough_pkt(struct br2684_dev *brdev, 
     struct sk_buff *skb, struct net_device **net_dev)
{
    int ret = -1;
    unsigned char *dstAddr;
    struct net_devext *devext;
    struct net_device *net_dev_ext;
    struct list_head *lh;
    
    *net_dev = NULL;
    dstAddr = eth_hdr(skb)->h_dest;
    
    if(!memcmp(dstAddr, skb->dev->dev_addr, ETH_ALEN)) {
        *net_dev = skb->dev;
    } 
    else {
        list_for_each(lh, &brdev->net_devexts) {
            net_dev_ext = list_entry_devext(lh);
            devext = EXTPRIV(net_dev_ext);
            if(!memcmp(dstAddr, net_dev_ext->dev_addr, ETH_ALEN)) {
                *net_dev = net_dev_ext;
                 break;
            }
        }
    }
    
    if((NULL == *net_dev) && ((ETH_P_PPP_DISC == skb->protocol) ||
                              (ETH_P_PPP_SES == skb->protocol))) {
        ret = 0;
    }

    return( ret );
}
#endif


#ifdef PTM_ETH_FCS
/*******************************************************************************
 * crc32
 *
 * Computes a crc32 over the input data using the polynomial:
 *
 *       x^32+x^26+x^23+x^22+x^16+x^12+x^11+X^10+x^8+x^7+x^5+x^4+x^2+x+1
 *
 * The caller provides the initial value (either CRC32_INIT_VALUE
 * or the previous returned value) to allow for processing of 
 * discontiguous blocks of data.  When generating the CRC the
 * caller is responsible for complementing the final return value
 * and inserting it into the byte stream.  When checking, a final
 * return value of CRC32_GOOD_VALUE indicates a valid CRC.
 *
 * Reference: Dallas Semiconductor Application Note 27
 *   Williams, Ross N., "A Painless Guide to CRC Error Detection Algorithms", 
 *     ver 3, Aug 1993, ross@guest.adelaide.edu.au, Rocksoft Pty Ltd.,
 *     ftp://ftp.rocksoft.com/clients/rocksoft/papers/crc_v3.txt
 *
 ******************************************************************************/

static uint32 crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

#define CRC_INNER_LOOP(n, c, x) \
    (c) = ((c) >> 8) ^ crc##n##_table[((c) ^ (x)) & 0xff]

uint32 crc32( uint8 *pdata, uint nbytes, uint32 crc)
{
    while (nbytes-- > 0)
        CRC_INNER_LOOP(32, crc, *pdata++);
    return crc;
}
#endif

module_init(br2684_init);
module_exit(br2684_exit);

MODULE_AUTHOR("Marcell GAL");
MODULE_DESCRIPTION("RFC2684 bridged protocols over ATM/AAL5");
MODULE_LICENSE("GPL");
