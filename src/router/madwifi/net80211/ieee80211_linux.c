/*-
 * Copyright (c) 2003-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ieee80211_linux.c 1590 2006-05-22 04:39:55Z mrenzmann $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 support (Linux-specific code)
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/sysctl.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/vmalloc.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#include <linux/proc_fs.h>
#endif

#include <net/iw_handler.h>
#include <linux/wireless.h>
#include <linux/if_arp.h>		/* XXX for ARPHRD_* */

#include <asm/uaccess.h>

#include "if_media.h"
#include "if_ethersubr.h"

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_monitor.h>

/*
 * Print a console message with the device name prepended.
 */
void
if_printf(struct net_device *dev, const char *fmt, ...)
{
	va_list ap;
	char buf[512];		/* XXX */

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printk("%s: %s", dev->name, buf);
}

/*
 * Allocate and setup a management frame of the specified
 * size.  We return the sk_buff and a pointer to the start
 * of the contiguous data area that's been reserved based
 * on the packet length.  The data area is forced to 32-bit
 * alignment and the buffer length to a multiple of 4 bytes.
 * This is done mainly so beacon frames (that require this)
 * can use this interface too.
 */
struct sk_buff *
ieee80211_getmgtframe(u_int8_t **frm, u_int pktlen)
{
	const u_int align = sizeof(u_int32_t);
	struct ieee80211_cb *cb;
	struct sk_buff *skb;
	u_int len;

	len = roundup(sizeof(struct ieee80211_frame) + pktlen, 4);
	skb = dev_alloc_skb(len + align-1);
	if (skb != NULL) {
		u_int off = ((unsigned long) skb->data) % align;
		if (off != 0)
			skb_reserve(skb, align - off);

		cb = (struct ieee80211_cb *)skb->cb;
		cb->ni = NULL;
		cb->flags = 0;
		cb->next = NULL;

		skb_reserve(skb, sizeof(struct ieee80211_frame));
		*frm = skb_put(skb, pktlen);
	}
	return skb;
}

#if 0
/*
 * Drain a queue of sk_buff's.
 */
void
__skb_queue_drain(struct sk_buff_head *q)
{
	struct sk_buff *skb;

	while ((skb = __skb_dequeue(q)) != NULL)
		dev_kfree_skb(skb);
}
#endif

#if IEEE80211_VLAN_TAG_USED
/*
 * VLAN support.
 */

/*
 * Register a vlan group.
 */
static void
ieee80211_vlan_register(struct net_device *dev, struct vlan_group *grp)
{
	struct ieee80211vap *vap = dev->priv;

	vap->iv_vlgrp = grp;
}

/*
 * Add an rx vlan identifier
 */
static void
ieee80211_vlan_add_vid(struct net_device *dev, unsigned short vid)
{
	struct ieee80211vap *vap = dev->priv;

	if (vap->iv_vlgrp != NULL)
		vap->iv_bss->ni_vlan = vid;
}

/*
 * Kill (i.e. delete) a vlan identifier.
 */
static void
ieee80211_vlan_kill_vid(struct net_device *dev, unsigned short vid)
{
	struct ieee80211vap *vap = dev->priv;

	if (vap->iv_vlgrp != NULL)
		vap->iv_vlgrp->vlan_devices[vid] = NULL;
}
#endif /* IEEE80211_VLAN_TAG_USED */

void
ieee80211_vlan_vattach(struct ieee80211vap *vap)
{
#if IEEE80211_VLAN_TAG_USED
	struct net_device *dev = vap->iv_dev;

	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX |
			 NETIF_F_HW_VLAN_FILTER;
	dev->vlan_rx_register = ieee80211_vlan_register;
	dev->vlan_rx_add_vid = ieee80211_vlan_add_vid;
	dev->vlan_rx_kill_vid = ieee80211_vlan_kill_vid;
#endif /* IEEE80211_VLAN_TAG_USED */
}

void
ieee80211_vlan_vdetach(struct ieee80211vap *vap)
{
}

void
ieee80211_notify_node_join(struct ieee80211_node *ni, int newassoc)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct net_device *dev = vap->iv_dev;
	union iwreq_data wreq;

	if (ni == vap->iv_bss) { 
		if (newassoc) 
			netif_carrier_on(dev);
		memset(&wreq, 0, sizeof(wreq));
		IEEE80211_ADDR_COPY(wreq.addr.sa_data, ni->ni_bssid);
		wreq.addr.sa_family = ARPHRD_ETHER;
#ifdef ATH_SUPERG_XR
		if (vap->iv_xrvap && vap->iv_flags & IEEE80211_F_XR)
			dev = vap->iv_xrvap->iv_dev;
#endif
		wireless_send_event(dev, SIOCGIWAP, &wreq, NULL);
	} else {
		memset(&wreq, 0, sizeof(wreq));
		IEEE80211_ADDR_COPY(wreq.addr.sa_data, ni->ni_macaddr);
		wreq.addr.sa_family = ARPHRD_ETHER;
#ifdef ATH_SUPERG_XR
		if (vap->iv_xrvap && vap->iv_flags & IEEE80211_F_XR)
			dev = vap->iv_xrvap->iv_dev;
#endif
		wireless_send_event(dev, IWEVREGISTERED, &wreq, NULL);
	}
}

void
ieee80211_notify_node_leave(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct net_device *dev = vap->iv_dev;
	union iwreq_data wreq;

	if (ni == vap->iv_bss) {
		netif_carrier_off(dev);
		memset(wreq.ap_addr.sa_data, 0, ETHER_ADDR_LEN);
		wreq.ap_addr.sa_family = ARPHRD_ETHER;
		wireless_send_event(dev, SIOCGIWAP, &wreq, NULL);
	} else {
		/* fire off wireless event station leaving */
		memset(&wreq, 0, sizeof(wreq));
		IEEE80211_ADDR_COPY(wreq.addr.sa_data, ni->ni_macaddr);
		wreq.addr.sa_family = ARPHRD_ETHER;
		wireless_send_event(dev, IWEVEXPIRED, &wreq, NULL);
	}
}

void
ieee80211_notify_scan_done(struct ieee80211vap *vap)
{
	struct net_device *dev = vap->iv_dev;
	union iwreq_data wreq;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN, "%s\n", "notify scan done");

	/* dispatch wireless event indicating scan completed */
	wreq.data.length = 0;
	wreq.data.flags = 0;
	wireless_send_event(dev, SIOCGIWSCAN, &wreq, NULL);
}

void
ieee80211_notify_replay_failure(struct ieee80211vap *vap,
	const struct ieee80211_frame *wh, const struct ieee80211_key *k,
	u_int64_t rsc)
{
	static const char *tag = "MLME-REPLAYFAILURE.indication";
	struct net_device *dev = vap->iv_dev;
	union iwreq_data wrqu;
	char buf[128];		/* XXX */

	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, wh->i_addr2,
		"%s replay detected <keyix %d, rsc %llu >",
		k->wk_cipher->ic_name, k->wk_keyix, rsc );

	/* TODO: needed parameters: count, keyid, key type, src address, TSC */
	snprintf(buf, sizeof(buf), "%s(keyid=%d %scast addr=%s)", tag,
		k->wk_keyix,
		IEEE80211_IS_MULTICAST(wh->i_addr1) ?  "broad" : "uni",
		ether_sprintf(wh->i_addr1));
	memset(&wrqu, 0, sizeof(wrqu));
	wrqu.data.length = strlen(buf);
	wireless_send_event(dev, IWEVCUSTOM, &wrqu, buf);
}
EXPORT_SYMBOL(ieee80211_notify_replay_failure);

void
ieee80211_notify_michael_failure(struct ieee80211vap *vap,
	const struct ieee80211_frame *wh, u_int keyix)
{
	static const char *tag = "MLME-MICHAELMICFAILURE.indication";
	struct net_device *dev = vap->iv_dev;
	union iwreq_data wrqu;
	char buf[128];		/* XXX */

	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, wh->i_addr2,
		"Michael MIC verification failed <keyix %d>", keyix);
	vap->iv_stats.is_rx_tkipmic++;

	/* TODO: needed parameters: count, keyid, key type, src address, TSC */
	snprintf(buf, sizeof(buf), "%s(keyid=%d %scast addr=%s)", tag,
		keyix, IEEE80211_IS_MULTICAST(wh->i_addr1) ?  "broad" : "uni",
		ether_sprintf(wh->i_addr1));
	memset(&wrqu, 0, sizeof(wrqu));
	wrqu.data.length = strlen(buf);
	wireless_send_event(dev, IWEVCUSTOM, &wrqu, buf);
}
EXPORT_SYMBOL(ieee80211_notify_michael_failure);

int
ieee80211_load_module(const char *modname)
{
	int rv;
	
	rv = request_module(modname);
	
	if (rv < 0)
		printk(KERN_ERR "couldn't load module '%s' (%d)\n",
			modname, rv);
	/*
	 * XXX Further checking needed if module has been loaded successfully:
	 *
	 * "Note that a successful module load does not mean the module did not
	 * then unload and exit on an error of its own. Callers must check that
	 * the service they requested is now available not blindly invoke it."
	 * http://kernelnewbies.org/documents/kdoc/kernel-api/r7338.html
	 */
	return rv;
}

#ifdef CONFIG_SYSCTL

static struct proc_dir_entry *proc_madwifi;
static int proc_madwifi_count = 0;

static int
proc_read_nodes(struct ieee80211vap *vap, char *buf, int space)
{
        char *p = buf;
        struct ieee80211_node *ni;
        struct ieee80211_node_table *nt = (struct ieee80211_node_table *) &vap->iv_ic->ic_sta;
	int8_t noise;
        //IEEE80211_NODE_LOCK(nt);                                                                               
        TAILQ_FOREACH(ni, &nt->nt_node, ni_list) {
                /* Assume each node needs 500 bytes */
                if (buf + space < p + 500)
                        break;

		if (ni->ni_vap == vap &&
		    0 != memcmp(vap->iv_myaddr, ni->ni_macaddr, IEEE80211_ADDR_LEN)) {
			struct timespec t;
			jiffies_to_timespec(jiffies - ni->ni_last_rx, &t);
			p += sprintf(p, "macaddr: <%s>\n", ether_sprintf(ni->ni_macaddr));
			p += sprintf(p, " rssi %d\n", ni->ni_rssi);
			noise = -95;
			if (ni->ni_ic->ic_getchannelnoise)
			    noise = (int8_t) ni->ni_ic->ic_getchannelnoise(ni->ni_ic,ni->ni_chan);
			p += sprintf(p, " noise %d\n", noise);
			
			p += sprintf(p, " last_rx %ld.%06ld\n", 
				     t.tv_sec, t.tv_nsec / 1000);

		}
        }
        //IEEE80211_NODE_UNLOCK(nt);                                                                             
        return (p - buf);
}

static ssize_t
proc_ieee80211_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	struct proc_ieee80211_priv *pv = (struct proc_ieee80211_priv *) file->private_data;

	if (!pv->rbuf)
		return -EINVAL;
	if (pos < 0)
		return -EINVAL;
	if (pos > pv->rlen)
		return -EFAULT;
	if (len > pv->rlen - pos)
		len = pv->rlen - pos;
	if (copy_to_user(buf, pv->rbuf + pos, len))
		return -EFAULT;
	*offset = pos + len;
	return len;
}

static int
proc_ieee80211_open(struct inode *inode, struct file *file)
{
	struct proc_ieee80211_priv *pv = NULL;
	struct proc_dir_entry *dp = PDE(inode);
	struct ieee80211vap *vap = dp->data;

	if (!(file->private_data = kmalloc(sizeof(struct proc_ieee80211_priv), GFP_KERNEL)))
		return -ENOMEM;
	/* intially allocate both read and write buffers */
	pv = (struct proc_ieee80211_priv *) file->private_data;
	memset(pv, 0, sizeof(struct proc_ieee80211_priv));
	pv->rbuf = vmalloc(MAX_PROC_IEEE80211_SIZE);
	if (!pv->rbuf) {
		kfree(pv);
		return -ENOMEM;
	}
	pv->wbuf = vmalloc(MAX_PROC_IEEE80211_SIZE);
	if (!pv->wbuf) {
		vfree(pv->rbuf);
		kfree(pv);
		return -ENOMEM;
	}
	memset(pv->wbuf, 0, MAX_PROC_IEEE80211_SIZE);
	memset(pv->rbuf, 0, MAX_PROC_IEEE80211_SIZE);
	pv->max_wlen = MAX_PROC_IEEE80211_SIZE;
	pv->max_rlen = MAX_PROC_IEEE80211_SIZE;
	/* now read the data into the buffer */
	pv->rlen = proc_read_nodes(vap, pv->rbuf, MAX_PROC_IEEE80211_SIZE);
	return 0;
}

static ssize_t
proc_ieee80211_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	loff_t pos = *offset;
	struct proc_ieee80211_priv *pv =
		(struct proc_ieee80211_priv *) file->private_data;

	if (!pv->wbuf)
		return -EINVAL;
	if (pos < 0)
		return -EINVAL;
	if (pos >= pv->max_wlen)
		return 0;
	if (len > pv->max_wlen - pos)
		len = pv->max_wlen - pos;
	if (copy_from_user(pv->wbuf + pos, buf, len))
		return -EFAULT;
	if (pos + len > pv->wlen)
		pv->wlen = pos + len;
	*offset = pos + len;

	return len;
}

static int
proc_ieee80211_close(struct inode *inode, struct file *file)
{
	struct proc_ieee80211_priv *pv =
		(struct proc_ieee80211_priv *) file->private_data;
	if (pv->rbuf)
		vfree(pv->rbuf);
	if (pv->wbuf)
		vfree(pv->wbuf);
	kfree(pv);
	return 0;
}

static struct file_operations proc_ieee80211_ops = {
        .read = proc_ieee80211_read,
        .write = proc_ieee80211_write,
        .open = proc_ieee80211_open,
        .release = proc_ieee80211_close,
};

#ifdef IEEE80211_DEBUG
static int
IEEE80211_SYSCTL_DECL(ieee80211_sysctl_debug, ctl, write, filp, buffer,
	lenp, ppos)
{
	struct ieee80211vap *vap = ctl->extra1;
	u_int val;
	int ret;

	ctl->data = &val;
	ctl->maxlen = sizeof(val);
	if (write) {
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
		if (ret == 0)
			vap->iv_debug = val;
	} else {
		val = vap->iv_debug;
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
	}
	return ret;
}
#endif /* IEEE80211_DEBUG */

static int
IEEE80211_SYSCTL_DECL(ieee80211_sysctl_dev_type, ctl, write, filp, buffer,
	lenp, ppos)
{
	struct ieee80211vap *vap = ctl->extra1;
	u_int val;
	int ret;

	ctl->data = &val;
	ctl->maxlen = sizeof(val);
	if (write) {
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
		if (ret == 0 && vap->iv_opmode == IEEE80211_M_MONITOR) {
			if (val == ARPHRD_IEEE80211_RADIOTAP ||
			    val == ARPHRD_IEEE80211 ||
			    val == ARPHRD_IEEE80211_PRISM ||
			    val == ARPHRD_IEEE80211_ATHDESC) {
				vap->iv_dev->type = val;
			}
		}
	} else {
		val = vap->iv_dev->type;
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
	}
	return ret;
}
static int
IEEE80211_SYSCTL_DECL(ieee80211_sysctl_monitor_nods_only, ctl, write, filp, buffer,
	lenp, ppos)
{
	struct ieee80211vap *vap = ctl->extra1;
	u_int val;
	int ret;

	ctl->data = &val;
	ctl->maxlen = sizeof(val);
	if (write) {
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
		if (ret == 0)
			vap->iv_monitor_nods_only = val;
	} else {
		val = vap->iv_monitor_nods_only;
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
	}
	return ret;
}
static int
IEEE80211_SYSCTL_DECL(ieee80211_sysctl_monitor_txf_len, ctl, write, filp, buffer,
	lenp, ppos)
{
	struct ieee80211vap *vap = ctl->extra1;
	u_int val;
	int ret;

	ctl->data = &val;
	ctl->maxlen = sizeof(val);
	if (write) {
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
		if (ret == 0)
			vap->iv_monitor_txf_len = val;
	} else {
		val = vap->iv_monitor_txf_len;
		ret = IEEE80211_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
			lenp, ppos);
	}
	return ret;
}

#define	CTL_AUTO	-2	/* cannot be CTL_ANY or CTL_NONE */

static const ctl_table ieee80211_sysctl_template[] = {
#ifdef IEEE80211_DEBUG
	{ .ctl_name	= CTL_AUTO,
	  .procname	= "debug",
	  .mode		= 0644,
	  .proc_handler	= ieee80211_sysctl_debug
	},
#endif
	{ .ctl_name	= CTL_AUTO,
	  .procname	= "dev_type",
	  .mode		= 0644,
	  .proc_handler	= ieee80211_sysctl_dev_type
	},
	{ .ctl_name	= CTL_AUTO,
	  .procname	= "monitor_nods_only",
	  .mode		= 0644,
	  .proc_handler	= ieee80211_sysctl_monitor_nods_only
	},
	{ .ctl_name	= CTL_AUTO,
	  .procname	= "monitor_txf_len",
	  .mode		= 0644,
	  .proc_handler	= ieee80211_sysctl_monitor_txf_len
	},
	/* NB: must be last entry before NULL */
	{ .ctl_name	= CTL_AUTO,
	  .procname	= "%parent",
	  .maxlen	= IFNAMSIZ,
	  .mode		= 0444,
	  .proc_handler	= proc_dostring
	},
	{ 0 }
};

void
ieee80211_sysctl_vattach(struct ieee80211vap *vap)
{
	int i, space;
	char *devname = NULL;
	struct ieee80211_proc_entry *tmp=NULL;
	
	space = 5 * sizeof(struct ctl_table) + sizeof(ieee80211_sysctl_template);
	vap->iv_sysctls = kmalloc(space, GFP_KERNEL);
	if (vap->iv_sysctls == NULL) {
		printk("%s: no memory for sysctl table!\n", __func__);
		return;
	}

	/*
	 * Reserve space for the device name outside the net_device structure
	 * so that if the name changes we know what it used to be. 
	 */
	devname = kmalloc((strlen(vap->iv_dev->name) + 1) * sizeof(char), GFP_KERNEL);
	if (devname == NULL) {
		printk("%s: no memory for VAP name!\n", __func__);
		return;
	}
	strncpy(devname, vap->iv_dev->name, strlen(vap->iv_dev->name) + 1);

	/* setup the table */
	memset(vap->iv_sysctls, 0, space);
	vap->iv_sysctls[0].ctl_name = CTL_NET;
	vap->iv_sysctls[0].procname = "net";
	vap->iv_sysctls[0].mode = 0555;
	vap->iv_sysctls[0].child = &vap->iv_sysctls[2];
	/* [1] is NULL terminator */
	vap->iv_sysctls[2].ctl_name = CTL_AUTO;
	vap->iv_sysctls[2].procname = devname; /* XXX bad idea? */
	vap->iv_sysctls[2].mode = 0555;
	vap->iv_sysctls[2].child = &vap->iv_sysctls[4];
	/* [3] is NULL terminator */
	/* copy in pre-defined data */
	memcpy(&vap->iv_sysctls[4], ieee80211_sysctl_template,
		sizeof(ieee80211_sysctl_template));

	/* add in dynamic data references */
	for (i = 4; vap->iv_sysctls[i].ctl_name; i++)
		if (vap->iv_sysctls[i].extra1 == NULL)
			vap->iv_sysctls[i].extra1 = vap;

	/* tack on back-pointer to parent device */
	vap->iv_sysctls[i-1].data = vap->iv_ic->ic_dev->name;	/* XXX? */

	/* and register everything */
	vap->iv_sysctl_header = register_sysctl_table(vap->iv_sysctls, 1);
	if (!vap->iv_sysctl_header) {
		printk("%s: failed to register sysctls!\n", vap->iv_dev->name);
		kfree(vap->iv_sysctls);
		vap->iv_sysctls = NULL;
	}

	/* Ensure the base madwifi directory exists */
	if (!proc_madwifi && proc_net != NULL) {
		proc_madwifi = proc_mkdir("madwifi", proc_net);
		if (!proc_madwifi)
			printk(KERN_WARNING "Failed to mkdir /proc/net/madwifi\n");
	}

	/* Create a proc directory named after the VAP */
	if (proc_madwifi) {
		proc_madwifi_count++;
		vap->iv_proc = proc_mkdir(vap->iv_dev->name, proc_madwifi);
	}
	
	/* Create a proc entry listing the associated stations */
	ieee80211_proc_vcreate(vap, &proc_ieee80211_ops, "associated_sta");

	/* Recreate any other proc entries that have been registered */
		if (vap->iv_proc) {
		tmp = vap->iv_proc_entries;
		while (tmp) {
			if (!tmp->entry) {
				tmp->entry = create_proc_entry(tmp->name, 
				PROC_IEEE80211_PERM, vap->iv_proc);
				tmp->entry->data = vap;
				tmp->entry->proc_fops = tmp->fileops;
			}
			tmp = tmp->next;
		}
	}
}

/* Frees all memory used for the list of proc entries */
void 
ieee80211_proc_cleanup(struct ieee80211vap *vap)
{
	struct ieee80211_proc_entry *tmp=vap->iv_proc_entries;
	struct ieee80211_proc_entry *next = NULL;
	while (tmp) {
		next = tmp->next;
		kfree(tmp);
		tmp = next;
	}
}

/* Called by other modules to register a proc entry under the vap directory */
int 
ieee80211_proc_vcreate(struct ieee80211vap *vap, 
		struct file_operations *fileops, char *name)
{
	struct ieee80211_proc_entry *entry;
	struct ieee80211_proc_entry *tmp=NULL;

	/* Ignore if already in the list */
	if (vap->iv_proc_entries) {
		tmp = vap->iv_proc_entries;
		do {
			if (strcmp(tmp->name, name)==0)
				return -1;
			/* Check for end of list */
			if (!tmp->next)
				break;
			/* Otherwise move on */
			tmp = tmp->next;
		} while (1);
	}
	
	/* Create an item in our list for the new entry */
	entry = kmalloc(sizeof(struct ieee80211_proc_entry), GFP_KERNEL);
	if (entry == NULL) {
		printk("%s: no memory for new proc entry (%s)!\n", __func__, 
				name);
		return -1;
	}

	/* Replace null fileops pointers with our standard functions */
	if (!fileops->open)
		fileops->open = proc_ieee80211_open;
	if (!fileops->release)
		fileops->release = proc_ieee80211_close;
	if (!fileops->read)
		fileops->read = proc_ieee80211_read;
	if (!fileops->write)
		fileops->write = proc_ieee80211_write;
	
	/* Create the entry record */
	entry->name = name;
	entry->fileops = fileops;
	entry->next = NULL;
	entry->entry = NULL;

	/* Create the actual proc entry */
	if (vap->iv_proc) {
		entry->entry = create_proc_entry(entry->name, 
				PROC_IEEE80211_PERM, vap->iv_proc);
		entry->entry->data = vap;
		entry->entry->proc_fops = entry->fileops;
	}

	/* Add it to the list */
	if (!tmp) {
		/* Add to the start */
		vap->iv_proc_entries = entry;
	} else {
		/* Add to the end */
		tmp->next = entry;
	}

	return 0;
}
EXPORT_SYMBOL(ieee80211_proc_vcreate);

void
ieee80211_sysctl_vdetach(struct ieee80211vap *vap)
{
	struct ieee80211_proc_entry *tmp=NULL;

	if (vap->iv_sysctl_header) {
		unregister_sysctl_table(vap->iv_sysctl_header);
		vap->iv_sysctl_header = NULL;
	}

	if (vap->iv_proc) {
		/* Remove child proc entries but leave them in the list */
		tmp = vap->iv_proc_entries;
		while (tmp) {
			if (tmp->entry) {
				remove_proc_entry(tmp->name, vap->iv_proc);
				tmp->entry = NULL;
			}
			tmp = tmp->next;
		}
		remove_proc_entry(vap->iv_proc->name, proc_madwifi);
		if (proc_madwifi_count == 1) {
			remove_proc_entry("madwifi", proc_net);
			proc_madwifi = NULL;
		}
		proc_madwifi_count--;
	}

	if (vap->iv_sysctls[2].procname) {
		kfree(vap->iv_sysctls[2].procname);
		vap->iv_sysctls[2].procname = NULL;
	}
	
	if (vap->iv_sysctls) {
		kfree(vap->iv_sysctls);
		vap->iv_sysctls = NULL;
	}
}
#endif /* CONFIG_SYSCTL */

/*
 * Format an Ethernet MAC for printing.
 */
const char*
ether_sprintf(const u_int8_t *mac)
{
	static char etherbuf[18]; 	/* XXX */
	snprintf(etherbuf, sizeof(etherbuf), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return etherbuf;
}
EXPORT_SYMBOL(ether_sprintf);		/* XXX */

/* Function to handle the device event notifications.
 * If the event is a NETDEV_CHANGENAME, and is for an interface
 * we are taking care of, then we want to remove its existing 
 * proc entries (which now have the wrong names) and add
 * new, correct, entries.
 */
static int
ieee80211_rcv_dev_event(struct notifier_block *this, unsigned long event,
	void *ptr)
{
#ifdef CONFIG_SYSCTL
	struct net_device *dev = (struct net_device *) ptr;
	if (!dev || dev->open != &ieee80211_open)
		return 0;

        switch (event) {
        case NETDEV_CHANGENAME:
		ieee80211_sysctl_vdetach(dev->priv);
		ieee80211_sysctl_vattach(dev->priv);
		return NOTIFY_DONE;
	default:
		break;
        }
#endif /* CONFIG_SYSCTL */
        return 0;
}

static struct notifier_block ieee80211_event_block = {
        .notifier_call = ieee80211_rcv_dev_event
};

/*
 * Module glue.
 */
#include "version.h"
#include "release.h"
static char *version = WLAN_VERSION " (" RELEASE_VERSION ")";
static char *dev_info = "wlan";

MODULE_AUTHOR("Errno Consulting, Sam Leffler");
MODULE_DESCRIPTION("802.11 wireless LAN protocol support");
#ifdef MODULE_VERSION
MODULE_VERSION(RELEASE_VERSION);
#endif
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

extern	void ieee80211_auth_setup(void);

static int __init
init_wlan(void)
{
  	register_netdevice_notifier(&ieee80211_event_block);
	printk(KERN_INFO "%s: %s\n", dev_info, version);
	return 0;
}
module_init(init_wlan);

static void __exit
exit_wlan(void)
{
  	unregister_netdevice_notifier(&ieee80211_event_block);
	printk(KERN_INFO "%s: driver unloaded\n", dev_info);
}
module_exit(exit_wlan);
