/*
 * net/atm/rt2684.c - RFC1577 Classical IP over ATM
 *
 *      shrinked version: only handles encapsulation
 *      (no atmarp handling)
 *
 *      Song Wang (songw@broadcom.com)
 */

#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <net/arp.h>
#include <linux/rtnetlink.h>
#include <linux/atmrt2684.h>
#include <linux/atmclip.h>
#include <linux/ip.h>

#include "ipcommon.h"

/*#define DEBUG*/
#ifdef DEBUG
#define DPRINTK(format, args...) printk(KERN_DEBUG "rt2684: " format, ##args)
#else
#define DPRINTK(format, args...)
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
	printk(KERN_DEBUG "rt2684: skb: %s\n", buf);
}
#else
#define skb_debug(skb)	do {} while (0)
#endif

struct rt2684_vcc {
	struct atm_vcc  *atmvcc;
	struct rt2684_dev *rtdev;

        int		xoff;		/* 1 if send buffer is full */
	unsigned long	last_use;	/* last send or receive operation */
	unsigned long	idle_timeout;	/* keep open idle for so many jiffies*/
	void (*old_push)(struct atm_vcc *vcc,struct sk_buff *skb);
					/* keep old push fn for chaining */
	void (*old_pop)(struct atm_vcc *vcc,struct sk_buff *skb);
					/* keep old pop fn for chaining */
 	unsigned char	encap;		/* 0: NULL, 1: LLC/SNAP */
        struct list_head rtvccs;


#ifdef CONFIG_ATM_rt2684_IPFILTER
	struct rt2684_filter filter;
#endif /* CONFIG_ATM_rt2684_IPFILTER */
#ifndef FASTER_VERSION
	unsigned copies_needed, copies_failed;
#endif /* FASTER_VERSION */
};

struct rt2684_dev {
	struct net_device net_dev;
	struct list_head rt2684_devs;
	int number;
	struct list_head rtvccs; /* one device <=> one vcc (before xmas) */
	struct net_device_stats stats;
	spinlock_t xoff_lock;	/* ensures that pop is atomic (SMP) */

};

/*
 * This lock should be held for writing any time the list of devices or
 * their attached vcc's could be altered.  It should be held for reading
 * any time these are being queried.  Note that we sometimes need to
 * do read-locking under interrupt context, so write locking must block
 * the current CPU's interrupts
 */
static rwlock_t devs_lock = RW_LOCK_UNLOCKED;

static LIST_HEAD(rt2684_devs);

static const unsigned char llc_oui[] = {
	0xaa,	/* DSAP: non-ISO */
	0xaa,	/* SSAP: non-ISO */
	0x03,	/* Ctrl: Unnumbered Information Command PDU */
	0x00,	/* OUI: EtherType */
	0x00,
	0x00 };

static inline struct rt2684_dev *RTPRIV(const struct net_device *net_dev)
{
	return (struct rt2684_dev *) ((char *) (net_dev) -
	    (unsigned long) (&((struct rt2684_dev *) 0)->net_dev));
}

static inline struct rt2684_dev *list_entry_rtdev(const struct list_head *le)
{
	return list_entry(le, struct rt2684_dev, rt2684_devs);
}

static inline struct rt2684_vcc *RT2684_VCC(const struct atm_vcc *atmvcc)
{
	return (struct rt2684_vcc *) (atmvcc->user_back);
}

static inline struct rt2684_vcc *list_entry_rtvcc(const struct list_head *le)
{
	return list_entry(le, struct rt2684_vcc, rtvccs);
}

/* Caller should hold read_lock(&devs_lock) */
static struct rt2684_dev *rt2684_find_dev(const struct rt2684_if_spec *s)
{
	struct list_head *lh;
	struct rt2684_dev *rtdev;
	switch (s->method) {
	case RT2684_FIND_BYNUM:
		list_for_each(lh, &rt2684_devs) {
			rtdev = list_entry_rtdev(lh);
			if (rtdev->number == s->spec.devnum)
				return rtdev;
		}
		break;
	case RT2684_FIND_BYIFNAME:
		list_for_each(lh, &rt2684_devs) {
			rtdev = list_entry_rtdev(lh);
			if (!strncmp(rtdev->net_dev.name, s->spec.ifname,
			    sizeof rtdev->net_dev.name))
				return rtdev;
		}
		break;
	}
	return NULL;
}

static inline struct rt2684_vcc *pick_outgoing_vcc(struct sk_buff *skb,
	struct rt2684_dev *rtdev)
{
	return list_empty(&rtdev->rtvccs) ? NULL :
	    list_entry_rtvcc(rtdev->rtvccs.next); /* 1 vcc/dev right now */
}

/*
 * Send a packet out a particular vcc.
 */
static int rt2684_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct rt2684_dev *rtdev = RTPRIV(dev);
	struct rt2684_vcc *rtvcc;
        struct atm_vcc *atmvcc;
	int old;
	unsigned long flags;

	DPRINTK("rt2684_start_xmit, skb->dst=%p\n", skb->dst);
	read_lock(&devs_lock);
	rtvcc = pick_outgoing_vcc(skb, rtdev);
	if (rtvcc == NULL) {
		DPRINTK("no vcc attached to dev %s\n", dev->name);
		rtdev->stats.tx_errors++;
		rtdev->stats.tx_carrier_errors++;
		/* netif_stop_queue(dev); */
		dev_kfree_skb(skb);
		read_unlock(&devs_lock);
		return -EUNATCH;
	}

	if (rtvcc->encap) {
		void *here;

		here = skb_push(skb,RFC1483LLC_LEN);
		memcpy(here,llc_oui,sizeof(llc_oui));
		((u16 *) here)[3] = skb->protocol;
	}

	skb_debug(skb);
	ATM_SKB(skb)->vcc = atmvcc = rtvcc->atmvcc;
	DPRINTK("atm_skb(%p)->vcc(%p)->dev(%p)\n", skb, atmvcc, atmvcc->dev);
	if (!atm_may_send(atmvcc, skb->truesize)) {
		/* we free this here for now, because we cannot know in a higher
		   layer whether the skb point it supplied wasn't freed yet.
		   now, it always is.
		*/
		dev_kfree_skb(skb);
		return 0;
	}
	atomic_add(skb->truesize,&sk_atm(atmvcc)->sk_wmem_alloc);
	ATM_SKB(skb)->atm_options = atmvcc->atm_options;
	rtvcc->last_use = jiffies;
	DPRINTK("atm_skb(%p)->vcc(%p)->dev(%p)\n",skb,atmvcc,atmvcc->dev);
	old = xchg(&rtvcc->xoff,1); /* assume XOFF ... */
	if (old) {
		printk(KERN_WARNING "rt2684_start_xmit: XOFF->XOFF transition\n");
		return 0;
	}
	rtdev->stats.tx_packets++;
	rtdev->stats.tx_bytes += skb->len;
	if( atmvcc->send(atmvcc, skb) != 0 )
	    rtdev->stats.tx_dropped++;

	if (atm_may_send(atmvcc,0)) {
		rtvcc->xoff = 0;
		return 0;
	}
	spin_lock_irqsave(&rtdev->xoff_lock,flags);
	netif_stop_queue(dev); /* XOFF -> throttle immediately */
	barrier();
	if (!rtvcc->xoff)
		netif_start_queue(dev);
		/* Oh, we just raced with rt2684_pop. netif_start_queue should be
		   good enough, because nothing should really be asleep because
		   of the brief netif_stop_queue. If this isn't true or if it
		   changes, use netif_wake_queue instead. */
	spin_unlock_irqrestore(&rtdev->xoff_lock,flags);

        read_unlock(&devs_lock);
	return 0;
}

static struct net_device_stats *rt2684_get_stats(struct net_device *dev)
{
	DPRINTK("rt2684_get_stats\n");
	return &RTPRIV(dev)->stats;
}

#ifdef CONFIG_ATM_rt2684_IPFILTER
/* this IOCTL is experimental. */
static int rt2684_setfilt(struct atm_vcc *atmvcc, unsigned long arg)
{
	struct rt2684_vcc *rtvcc;
	struct rt2684_filter_set fs;

	if (copy_from_user(&fs, (void *) arg, sizeof fs))
		return -EFAULT;
	if (fs.ifspec.method != RT2684_FIND_BYNOTHING) {
		/*
		 * This is really a per-vcc thing, but we can also search
		 * by device
		 */
		struct rt2684_dev *rtdev;
		read_lock(&devs_lock);
		rtdev = rt2684_find_dev(&fs.ifspec);
		if (rtdev == NULL || list_empty(&rtdev->rtvccs) ||
		    rtdev->rtvccs.next != rtdev->rtvccs.prev)  /* >1 VCC */
			rtvcc = NULL;
		else
			rtvcc = list_entry_rtvcc(rtdev->rtvccs.next);
		read_unlock(&devs_lock);
		if (rtvcc == NULL)
			return -ESRCH;
	} else
		rtvcc = rt2684_VCC(atmvcc);
	memcpy(&rtvcc->filter, &fs.filter, sizeof(rtvcc->filter));
	return 0;
}

/* Returns 1 if packet should be dropped */
static inline int
packet_fails_filter(u16 type, struct rt2684_vcc *rtvcc, struct sk_buff *skb)
{
	if (rtvcc->filter.netmask == 0)
		return 0;			/* no filter in place */
	if (type == __constant_htons(ETH_P_IP) &&
	    (((struct iphdr *) (skb->data))->daddr & rtvcc->filter.
	     netmask) == rtvcc->filter.prefix)
		return 0;
	if (type == __constant_htons(ETH_P_ARP))
		return 0;
	/* TODO: we should probably filter ARPs too.. don't want to have
	 *   them returning values that don't make sense, or is that ok?
	 */
	return 1;		/* drop */
}
#endif /* CONFIG_ATM_rt2684_IPFILTER */

static void rt2684_close_vcc(struct rt2684_vcc *rtvcc)
{
	DPRINTK("removing VCC %p from dev %p\n", rtvcc, rtvcc->rtdev);
	write_lock_irq(&devs_lock);
	list_del(&rtvcc->rtvccs);
	write_unlock_irq(&devs_lock);
	rtvcc->atmvcc->user_back = NULL;	/* what about vcc->recvq ??? */
	rtvcc->old_push(rtvcc->atmvcc, NULL);	/* pass on the bad news */
	kfree(rtvcc);
}

static void rt2684_pop(struct atm_vcc *atmvcc,struct sk_buff *skb)
{
	struct rt2684_vcc *rtvcc = RT2684_VCC(atmvcc);
        struct rt2684_dev *rtdev = rtvcc->rtdev;
	struct net_device *dev = skb->dev;
	int old;
	unsigned long flags;

	DPRINTK("rt2684_pop(vcc %p)\n",atmvcc);
	rtvcc->old_pop(atmvcc,skb);
	/* skb->dev == NULL in outbound ARP packets */
	if (!dev) return;
	spin_lock_irqsave(&rtdev->xoff_lock,flags);
	if (atm_may_send(atmvcc,0)) {
		old = xchg(&rtvcc->xoff,0);
		if (old) netif_wake_queue(dev);
	}
	spin_unlock_irqrestore(&rtdev->xoff_lock,flags);
}

/* when AAL5 PDU comes in: */
static void rt2684_push(struct atm_vcc *atmvcc, struct sk_buff *skb)
{
	struct rt2684_vcc *rtvcc = RT2684_VCC(atmvcc);
	struct rt2684_dev *rtdev = rtvcc->rtdev;

	DPRINTK("rt2684_push\n");

	if (skb == NULL) {	/* skb==NULL means VCC is being destroyed */
		rt2684_close_vcc(rtvcc);
		return;
	}

#if defined(CONFIG_MIPS_BRCM)
//	skb->__unused=FROM_WAN;
#endif	
	skb_debug(skb);
	atm_return(atmvcc, skb->truesize);
	DPRINTK("skb from rtdev %p\n", rtdev);

	skb->dev = &rtdev->net_dev;
	if (!skb->dev) {
		dev_kfree_skb_any(skb);
		return;
	}
	ATM_SKB(skb)->vcc = atmvcc;
	skb->mac.raw = skb->data;
	if (!rtvcc->encap || skb->len < RFC1483LLC_LEN || memcmp(skb->data,
	    llc_oui,sizeof(llc_oui))) skb->protocol = htons(ETH_P_IP);
	else {
		skb->protocol = ((u16 *) skb->data)[3];
		skb_pull(skb,RFC1483LLC_LEN);
	}
	DPRINTK("received packet's protocol: %x\n", ntohs(skb->protocol));
	skb_debug(skb);
	rtvcc->last_use = jiffies;
	if (!(rtdev->net_dev.flags & IFF_UP)) { /* sigh, interface is down */
		rtdev->stats.rx_dropped++;
		dev_kfree_skb(skb);
		return;
	}
	rtdev->stats.rx_packets++;
	rtdev->stats.rx_bytes += skb->len;
	memset(ATM_SKB(skb), 0, sizeof(struct atm_skb_data));
	netif_rx(skb);
}

/* assign a vcc to a dev
Note: we do not have explicit unassign, but look at _push()
*/
static int rt2684_regvcc(struct atm_vcc *atmvcc, unsigned long arg)
{

	int err;
	struct rt2684_vcc *rtvcc;
	struct sk_buff_head copy;
	struct sk_buff *skb;
	struct rt2684_dev *rtdev;
	struct atm_backend_rt2684 be;

	if (copy_from_user(&be, (void *) arg, sizeof be)) {
		return -EFAULT;
	}
	write_lock_irq(&devs_lock);
        /* Find the atmxxx device according to the interface name */
	rtdev = rt2684_find_dev(&be.ifspec);
	if (rtdev == NULL) {
		printk(KERN_ERR
		    "rt2684: tried to attach to non-existant device\n");
		err = -ENXIO;
		goto error;
	}
	if (atmvcc->push == NULL) {
		err = -EBADFD;
		goto error;
	}
	if (!list_empty(&rtdev->rtvccs)) {	/* Only 1 VCC/dev right now */
		err = -EEXIST;
		goto error;
	}
	rtvcc = kmalloc(sizeof(struct rt2684_vcc), GFP_KERNEL);
	if (!rtvcc) {
		err = -ENOMEM;
		goto error;
	}
	memset(rtvcc, 0, sizeof(struct rt2684_vcc));
	DPRINTK("rt2684_regvcc vcc=%p, encaps=%d, rtvcc=%p\n", atmvcc, be.encaps,
		rtvcc);
	list_add(&rtvcc->rtvccs, &rtdev->rtvccs);
	write_unlock_irq(&devs_lock);
	rtvcc->rtdev = rtdev;
	rtvcc->atmvcc = atmvcc;
	atmvcc->user_back = rtvcc;

        rtvcc->xoff = 0;
	//rtvcc->encap = 1;
        rtvcc->encap = be.encaps;
	rtvcc->last_use = jiffies;
	//rtvcc->idle_timeout = timeout*HZ;
	rtvcc->old_push = atmvcc->push;
	rtvcc->old_pop = atmvcc->pop;
	atmvcc->push = rt2684_push;
	atmvcc->pop = rt2684_pop;

	skb_queue_head_init(&copy);
	skb_migrate(&sk_atm(atmvcc)->sk_receive_queue,&copy);
	/* re-process everything received between connection setup and MKIP */
	while ((skb = skb_dequeue(&copy))) {
	        unsigned int len = skb->len;

		rt2684_push(atmvcc,skb);
		RTPRIV(skb->dev)->stats.rx_packets--;
		RTPRIV(skb->dev)->stats.rx_bytes -= len;
	}
	return 0;

    error:
	write_unlock_irq(&devs_lock);
	return err;
}


/* Initialize net device atmxxx */
static int rt2684_initdev(struct net_device *dev)
{
	DPRINTK("rt2684_initdev %s\n",dev->name);
	dev->hard_start_xmit = rt2684_start_xmit;
	/* sg_xmit ... */
	dev->hard_header = NULL;
	dev->rebuild_header = NULL;
	dev->set_mac_address = NULL;
	dev->hard_header_parse = NULL;
	dev->hard_header_cache = NULL;
	dev->header_cache_update = NULL;
	dev->change_mtu = NULL;
	dev->do_ioctl = NULL;
	dev->get_stats = rt2684_get_stats;
	dev->type = ARPHRD_PPP; /* We are not using atmarp, so set to PPP */
	dev->hard_header_len = RFC1483LLC_LEN;
	dev->mtu = RFC1626_MTU;
	dev->addr_len = 0;
	dev->tx_queue_len = 100; /* "normal" queue (packets) */
	    /* When using a "real" qdisc, the qdisc determines the queue */
	    /* length. tx_queue_len is only used for the default case, */
	    /* without any more elaborate queuing. 100 is a reasonable */
	    /* compromise between decent burst-tolerance and protection */
	    /* against memory hogs. */
	/* Using the same set of flags as PPP */
	dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
	return 0;
}

static int rt2684_create(unsigned long arg)
{
	int err;
	struct rt2684_dev *rtdev;
	struct atm_newif_rt2684 ni;

	DPRINTK("rt2684_create\n");
	/*
	 * We track module use by vcc's NOT the devices they're on.  We're
	 * protected here against module death by the kernel_lock, but if
	 * we need to sleep we should make sure that the module doesn't
	 * disappear under us.
	 */
	if (copy_from_user(&ni, (void *) arg, sizeof ni)) {
		return -EFAULT;
	}

        /* Create atmxxx device */
	if ((rtdev = kmalloc(sizeof(struct rt2684_dev), GFP_KERNEL)) == NULL) {
		return -ENOMEM;
	}
	memset(rtdev, 0, sizeof(struct rt2684_dev));
	INIT_LIST_HEAD(&rtdev->rtvccs);

	write_lock_irq(&devs_lock);
	rtdev->number = list_empty(&rt2684_devs) ? 1 :
	    list_entry_rtdev(rt2684_devs.prev)->number + 1;
	list_add_tail(&rtdev->rt2684_devs, &rt2684_devs);
	write_unlock_irq(&devs_lock);

 	if (ni.ifname[0] != '\0') {
		memcpy(rtdev->net_dev.name, ni.ifname,
		    sizeof(rtdev->net_dev.name));
		rtdev->net_dev.name[sizeof(rtdev->net_dev.name) - 1] = '\0';
	} else
		sprintf(rtdev->net_dev.name, "atm%d", rtdev->number);

	DPRINTK("registered netdev %s\n", rtdev->net_dev.name);

	rtdev->net_dev.init = rt2684_initdev;
	spin_lock_init(&rtdev->xoff_lock);

	/* open, stop, do_ioctl ? */
	err = register_netdev(&rtdev->net_dev);
	if (err < 0) {
		printk(KERN_ERR "rt2684_create: register_netdev failed\n");
		write_lock_irq(&devs_lock);
		list_del(&rtdev->rt2684_devs);
		write_unlock_irq(&devs_lock);
		kfree(rtdev);
		return err;
	}
	return 0;
}

/*
 * This handles ioctls actually performed on our vcc - we must return
 * -ENOIOCTLCMD for any unrecognized ioctl
 *
 * Called from common.c: atm_backend_t b is used to differentiate
 * different ioctl hooks.
 */
static int rt2684_ioctl(struct socket *sock, unsigned int cmd,
	unsigned long arg)
{
	int err;
	struct atm_vcc *atmvcc = ATM_SD(sock);

        DPRINTK("rt2684_ioctl\n");
	switch(cmd) {
	case ATM_SETBACKEND:
	case ATM_NEWBACKENDIF: {
		atm_backend_t b;
		err = get_user(b, (atm_backend_t *) arg);
		if (err)
			return -EFAULT;
		if (b != ATM_BACKEND_RT2684)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (cmd == ATM_SETBACKEND)
			return rt2684_regvcc(atmvcc, arg);
		else
			return rt2684_create(arg);
		}
#ifdef CONFIG_ATM_rt2684_IPFILTER
	case rt2684_SETFILT:
		if (atmvcc->push != rt2684_push)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		err = rt2684_setfilt(atmvcc, arg);
		return err;
#endif /* CONFIG_ATM_rt2684_IPFILTER */
	}
	return -ENOIOCTLCMD;
}

static struct atm_ioctl rt2684_ioctl_ops = {
	.owner	= THIS_MODULE,
	.ioctl	= rt2684_ioctl,
};

/* Never put more than 256 bytes in at once */
static int rt2684_proc_engine(loff_t pos, char *buf)
{
	struct list_head *lhd, *lhc;
	struct rt2684_dev *rtdev;
	struct rt2684_vcc *rtvcc;
	list_for_each(lhd, &rt2684_devs) {
		rtdev = list_entry_rtdev(lhd);
		if (pos-- == 0)
			return sprintf(buf, "dev %.16s: num=%d\n",
			    rtdev->net_dev.name,
			    rtdev->number);
		list_for_each(lhc, &rtdev->rtvccs) {
			rtvcc = list_entry_rtvcc(lhc);
			if (pos-- == 0)
				return sprintf(buf, "  vcc %d.%d.%d: encaps=%s"
#ifndef FASTER_VERSION
				    ", failed copies %u/%u"
#endif /* FASTER_VERSION */
				    "\n", rtvcc->atmvcc->dev->number,
				    rtvcc->atmvcc->vpi, rtvcc->atmvcc->vci,
				    (rtvcc->encap == 1) ? "LLC" : "NULL"
#ifndef FASTER_VERSION
				    , rtvcc->copies_failed
				    , rtvcc->copies_needed
#endif /* FASTER_VERSION */
				    );
#ifdef CONFIG_ATM_rt2684_IPFILTER
#define b1(var, byte)	((u8 *) &rtvcc->filter.var)[byte]
#define bs(var)		b1(var, 0), b1(var, 1), b1(var, 2), b1(var, 3)
			if (rtvcc->filter.netmask != 0 && pos-- == 0)
				return sprintf(buf, "    filter=%d.%d.%d.%d/"
				    "%d.%d.%d.%d\n", bs(prefix), bs(netmask));
#undef bs
#undef b1
#endif /* CONFIG_ATM_rt2684_IPFILTER */
		}
	}
	return 0;
}

static ssize_t rt2684_proc_read(struct file *file, char *buf, size_t count,
	loff_t *pos)
{
	unsigned long page;
	int len = 0, x, left;
	page = __get_free_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;
	left = PAGE_SIZE - 256;
	if (count < left)
		left = count;
	read_lock(&devs_lock);
	for (;;) {
		x = rt2684_proc_engine(*pos, &((char *) page)[len]);
		if (x == 0)
			break;
		if (x > left)
			/*
			 * This should only happen if the user passed in
			 * a "count" too small for even one line
			 */
			x = -EINVAL;
		if (x < 0) {
			len = x;
			break;
		}
		len += x;
		left -= x;
		(*pos)++;
		if (left < 256)
			break;
	}
	read_unlock(&devs_lock);
	if (len > 0 && copy_to_user(buf, (char *) page, len))
		len = -EFAULT;
	free_page(page);
	return len;
}

static struct file_operations rt2684_proc_operations = {
	.read	= rt2684_proc_read,
};

extern struct proc_dir_entry *atm_proc_root;	/* from proc.c */


static int __init rt2684_init(void)
{
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *p;
	if ((p = create_proc_entry("rt2684", 0, atm_proc_root)) == NULL)
		return -ENOMEM;
	p->proc_fops = &rt2684_proc_operations;
#endif	
	register_atm_ioctl(&rt2684_ioctl_ops);
	return 0;
}

static void __exit rt2684_exit(void)
{
	struct rt2684_dev *rtdev;
	
	deregister_atm_ioctl(&rt2684_ioctl_ops);
#ifdef CONFIG_PROC_FS	
	remove_proc_entry("rt2684", atm_proc_root);
#endif	
	while (!list_empty(&rt2684_devs)) {
		rtdev = list_entry_rtdev(rt2684_devs.next);
		unregister_netdev(&rtdev->net_dev);
		list_del(&rtdev->rt2684_devs);
		kfree(rtdev);
	}
}

module_init(rt2684_init);
module_exit(rt2684_exit);

MODULE_AUTHOR("Song Wang");
MODULE_DESCRIPTION("RFC2684 routed protocols over ATM/AAL5 (for IPoA)");
MODULE_LICENSE("GPL");
