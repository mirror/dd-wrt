/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
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
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
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
 * $Id: ieee80211_output.c 3296 2008-01-28 23:40:44Z mentor $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 output handling.
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <linux/utsname.h>

#include <linux/ip.h>		/* XXX for TOS */

#include "if_llc.h"
#include "if_ethersubr.h"
#include "if_media.h"

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_monitor.h>
#include <net80211/if_athproto.h>

#ifdef IEEE80211_DEBUG
/*
 * Decide if an outbound management frame should be
 * printed when debugging is enabled.  This filters some
 * of the less interesting frames that come frequently
 * (e.g. beacons).
 */
static __inline int doprint(struct ieee80211vap *vap, int subtype)
{
	if (subtype == IEEE80211_FC0_SUBTYPE_PROBE_RESP)
		return (vap->iv_opmode == IEEE80211_M_IBSS);
	return 1;
}
#endif
#include <net80211/cn_policy_entry.h>
#include <net80211/cn_polling_entry.h>

static const int ieee802_1d_to_ac[8] = {
	WME_AC_BE, WME_AC_BK, WME_AC_BK, WME_AC_BE,
	WME_AC_VI, WME_AC_VI, WME_AC_VO, WME_AC_VO
};

/* Given a data frame determine the 802.1p/1d tag to use. */
static unsigned int ieee80211_classify_ip(struct sk_buff *skb)
{
	const struct ether_header *eh = (struct ether_header *)skb->data;
	const struct iphdr *ip = (struct iphdr *)
	    (skb->data + sizeof(struct ether_header));
	unsigned int dscp;

	switch (skb->protocol) {
	case __constant_htons(ETH_P_IP):
		dscp = ip->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}

/*
 * Determine the priority based on VLAN and/or IP TOS. Priority is
 * written into the skb->priority field. On success, returns 0. Failure
 * due to bad or mis-matched vlan tag is indicated by non-zero return.
 */
static int ieee80211_classify(struct ieee80211_node *ni, struct sk_buff *skb)
{
	struct ieee80211vap *vap = ni->ni_vap;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263) {
		skb->priority = ieee802_1d_to_ac[skb->priority - 256];
		return 0;
	}

	if (!(ni->ni_flags & IEEE80211_NODE_QOS)) {
		/* default priority */
		skb->priority = WME_AC_BE;
		return 0;
	}

	skb->priority = ieee802_1d_to_ac[ieee80211_classify_ip(skb)];

	/* Applying ACM policy */
	if (vap->iv_opmode == IEEE80211_M_STA) {
		struct ieee80211com *ic = ni->ni_ic;

		while (skb->priority != WME_AC_BK && ic->ic_wme.wme_wmeBssChanParams.cap_wmeParams[skb->priority].wmep_acm) {
			switch (skb->priority) {
			case WME_AC_BE:
				skb->priority = WME_AC_BK;
				break;
			case WME_AC_VI:
				skb->priority = WME_AC_BE;
				break;
			case WME_AC_VO:
				skb->priority = WME_AC_VI;
				break;
			default:
				skb->priority = WME_AC_BK;
				break;
			}
		}
	}

	return 0;
}

/*
 * Context: process context (BHs disabled)
 * It must return either NETDEV_TX_OK or NETDEV_TX_BUSY
 */
int ieee80211_hardstart(struct sk_buff *skb, struct net_device *dev)
{
	struct ieee80211vap *vap = netdev_priv(dev);
	struct ieee80211com *ic = vap->iv_ic;
	struct net_device *parent = ic->ic_dev;
	struct ieee80211_node *ni = NULL;
	struct ether_header *eh;

	/* reset the skb of new frames reaching this layer BEFORE
	 * we invoke ieee80211_skb_track. */
	memset(SKB_CB(skb), 0, sizeof(struct ieee80211_cb));

	/* If an skb is passed in directly from the kernel, 
	 * we take responsibility for the reference */
	ieee80211_skb_track(skb);

	/* NB: parent must be up and running */
	if ((parent->flags & (IFF_RUNNING | IFF_UP)) != (IFF_RUNNING | IFF_UP))
		goto bad;
	/*
	 * No data frames go out unless we're running.
	 */
	if (vap->iv_state != IEEE80211_S_RUN) {
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_OUTPUT, "%s: ignore data packet, state %u\n", __func__, vap->iv_state);
#if 0
		vap->iv_stats.ist_tx_discard++;
#endif
		goto bad;
	}

	if (vap->iv_opmode == IEEE80211_M_MONITOR) {
		ieee80211_monitor_encap(vap, skb);
		ieee80211_parent_queue_xmit(skb);
		return NETDEV_TX_OK;
	}

	/* Cancel any running BG scan */
	if (!(ic->ic_flags_ext & IEEE80211_FEXT_BGSCAN_THR) && (vap->iv_state == IEEE80211_S_RUN) && (ic->ic_flags_ext & IEEE80211_FEXT_BGSCAN))
		ieee80211_cancel_scan(vap);

	/* 
	 * Find the node for the destination so we can do
	 * things like power save.
	 */
	eh = (struct ether_header *)skb->data;
	ni = ieee80211_find_txnode(vap, eh->ether_dhost);
	if (ni == NULL) {
		/* NB: ieee80211_find_txnode does stat+msg */
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_OUTPUT, mac, "no node, discard frame (%s)", __func__);
		vap->iv_stats.is_tx_nonode++;
		goto bad;
	}

	if (ni->ni_subif && (vap != ni->ni_subif) && ((eh)->ether_type != __constant_htons(ETHERTYPE_PAE)))
		goto bad;

	/* calculate priority so drivers can find the TX queue */
	if (ieee80211_classify(ni, skb)) {
		IEEE80211_NOTE(vap, IEEE80211_MSG_OUTPUT, ni, "%s: discard, classification failure", __func__);
		goto bad;
	}

	SKB_CB(skb)->ni = ieee80211_ref_node(ni);

	/* power-save checks */
	if (WME_UAPSD_AC_CAN_TRIGGER(skb->priority, ni)) {
		/* U-APSD power save queue */
		/* XXXAPSD: assuming triggerable means deliverable */
		M_FLAG_SET(skb, M_UAPSD);
	} else if ((ni->ni_flags & IEEE80211_NODE_PWR_MGT)) {
		/* Station in power save mode; stick the frame
		 * on the STA's power save queue and continue.
		 * We'll get the frame back when the time is right. */
		ieee80211_unref_node(&ni);
		return ieee80211_pwrsave(skb);
	}

	/* save non-auth traffic when we're not authenticated yet */
	if ((vap->iv_opmode == IEEE80211_M_STA) && !(dev->flags & IFF_RUNNING) && (eh->ether_type != __constant_htons(ETHERTYPE_PAE))) {
		int ret;
		ret = ieee80211_pwrsave(skb);
		M_AGE_SET(skb, IEEE80211_INACT_WAIT - 1);
		return ret;
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,5,0)
	netif_trans_update(dev);
#else
	dev->trans_start = jiffies;
#endif

#ifdef ATH_SUPERG_XR
	/* Broadcast/multicast packets need to be sent on XR vap in addition to
	 * normal vap. */
	if (vap->iv_xrvap && (ni == vap->iv_bss) && vap->iv_xrvap->iv_sta_assoc) {
		struct sk_buff *skb1 = skb_clone(skb, GFP_ATOMIC);
		if (skb1) {
			memset(SKB_CB(skb1), 0, sizeof(struct ieee80211_cb));
#ifdef IEEE80211_DEBUG_REFCNT
			SKB_CB(skb1)->tracked = 1;
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */
			SKB_CB(skb1)->ni = ieee80211_find_txnode(vap->iv_xrvap, eh->ether_dhost);
			/* Ignore this return code. */
			ieee80211_parent_queue_xmit(skb1);
		}
	}
#endif
	ieee80211_unref_node(&ni);
	ieee80211_parent_queue_xmit(skb);
	return NETDEV_TX_OK;

bad:
	if (skb != NULL)
		ieee80211_dev_kfree_skb(&skb);
	if (ni != NULL)
		ieee80211_unref_node(&ni);
	return NETDEV_TX_OK;
}

/*
 * skb is consumed in all cases
 */

void ieee80211_parent_queue_xmit(struct sk_buff *skb)
{
	struct ieee80211vap *vap = netdev_priv(skb->dev);

	vap->iv_devstats.tx_packets++;
	vap->iv_devstats.tx_bytes += skb->len;
	vap->iv_ic->ic_lastdata = jiffies;

	/* Dispatch the packet to the parent device */
	skb->dev = vap->iv_ic->ic_dev;

	if (netif_queue_stopped(skb->dev))
		ieee80211_dev_kfree_skb(&skb);
	else if (dev_queue_xmit(skb) == NET_XMIT_DROP)
		vap->iv_devstats.tx_dropped++;
}

/*
 * Set the direction field and address fields of an outgoing
 * non-QoS frame.  Note this should be called early on in
 * constructing a frame as it sets i_fc[1]; other bits can
 * then be or'd in.
 */
static struct ieee80211_frame *ieee80211_send_setup(struct ieee80211vap *vap,
						    struct ieee80211_node *ni,
						    struct sk_buff *skb, int type, const u_int8_t sa[IEEE80211_ADDR_LEN], const u_int8_t da[IEEE80211_ADDR_LEN], const u_int8_t bssid[IEEE80211_ADDR_LEN])
{
#define	WH4(wh)	((struct ieee80211_frame_addr4 *)wh)
	struct ieee80211_frame *wh;
	int len;
	int opmode = vap->iv_opmode;

	if (type == (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_CTS))
		len = sizeof(struct ieee80211_frame_cts) + roundup(sizeof(struct ieee80211_frame_cts), 4) - sizeof(struct ieee80211_frame_cts);
	else
		len = sizeof(struct ieee80211_frame);

	if ((type & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_DATA) {
		if ((opmode == IEEE80211_M_STA) && (vap->iv_flags_ext & IEEE80211_FEXT_WDS))
			opmode = IEEE80211_M_WDS;

		if (opmode == IEEE80211_M_WDS)
			len = sizeof(struct ieee80211_frame_addr4);
	}

	wh = (struct ieee80211_frame *)skb_push(skb, len);
	wh->i_fc[0] = IEEE80211_FC0_VERSION_0 | type;
	if ((type & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_DATA) {
		switch (opmode) {
		case IEEE80211_M_STA:
			wh->i_fc[1] = IEEE80211_FC1_DIR_TODS;
			IEEE80211_ADDR_COPY(wh->i_addr1, bssid);
			IEEE80211_ADDR_COPY(wh->i_addr2, sa);
			IEEE80211_ADDR_COPY(wh->i_addr3, da);
			break;
		case IEEE80211_M_IBSS:
		case IEEE80211_M_AHDEMO:
			wh->i_fc[1] = IEEE80211_FC1_DIR_NODS;
			IEEE80211_ADDR_COPY(wh->i_addr1, da);
			IEEE80211_ADDR_COPY(wh->i_addr2, sa);
			IEEE80211_ADDR_COPY(wh->i_addr3, bssid);
			break;
		case IEEE80211_M_HOSTAP:
			wh->i_fc[1] = IEEE80211_FC1_DIR_FROMDS;
			IEEE80211_ADDR_COPY(wh->i_addr1, da);
			IEEE80211_ADDR_COPY(wh->i_addr2, bssid);
			IEEE80211_ADDR_COPY(wh->i_addr3, sa);
			break;
		case IEEE80211_M_WDS:
			wh->i_fc[1] = IEEE80211_FC1_DIR_DSTODS;
			/* XXX cheat, bssid holds RA */
			IEEE80211_ADDR_COPY(wh->i_addr1, bssid);
			IEEE80211_ADDR_COPY(wh->i_addr2, vap->iv_myaddr);
			IEEE80211_ADDR_COPY(wh->i_addr3, da);
			IEEE80211_ADDR_COPY(WH4(wh)->i_addr4, sa);
			break;
		case IEEE80211_M_MONITOR:	/* NB: to quiet compiler */
			break;
		}
	} else if ((type & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL) {
		wh->i_fc[1] = IEEE80211_FC1_DIR_NODS;
		IEEE80211_ADDR_COPY(wh->i_addr1, da);
	} else {
		wh->i_fc[1] = IEEE80211_FC1_DIR_NODS;
		IEEE80211_ADDR_COPY(wh->i_addr1, da);
		IEEE80211_ADDR_COPY(wh->i_addr2, sa);
		IEEE80211_ADDR_COPY(wh->i_addr3, bssid);
	}
	wh->i_dur = 0;
	/* NB: use non-QoS tid */
	*(__le16 *)&wh->i_seq[0] = htole16(ni->ni_txseqs[0] << IEEE80211_SEQ_SEQ_SHIFT);
	ni->ni_txseqs[0]++;

	return wh;
#undef WH4
}

/*
 * Send a management frame to the specified node.  The node pointer
 * must have a reference as the pointer will be passed to the driver
 * and potentially held for a long time.  If the frame is successfully
 * dispatched to the driver, then it is responsible for freeing the
 * reference (and potentially freeing up any associated storage).
 */
static void ieee80211_mgmt_output(struct ieee80211_node *ni, struct sk_buff *skb, int type)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct ieee80211_frame *wh;

	KASSERT(ni != NULL, ("null node"));

	SKB_CB(skb)->ni = ni;

	wh = ieee80211_send_setup(vap, ni, skb, IEEE80211_FC0_TYPE_MGT | type, vap->iv_myaddr, ni->ni_macaddr, vap->iv_bssid);
	/* XXX power management */

	if ((SKB_CB(skb)->flags & M_LINK0) != 0 && ni->ni_challenge != NULL) {
		SKB_CB(skb)->flags &= ~M_LINK0;
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_AUTH, wh->i_addr1, "encrypting frame (%s)", __func__);
		wh->i_fc[1] |= IEEE80211_FC1_PROT;
	}

	if (IEEE80211_VAP_IS_SLEEPING(ni->ni_vap))
		wh->i_fc[1] |= IEEE80211_FC1_PWR_MGT;

#ifdef IEEE80211_DEBUG
	/* avoid printing too many frames */
	if ((ieee80211_msg_debug(vap) && doprint(vap, type)) || ieee80211_msg_dumppkts(vap)) {
		printk(KERN_DEBUG "[" MAC_FMT "] send %s on channel %u\n",
		       MAC_ADDR(wh->i_addr1), ieee80211_mgt_subtype_name[(type & IEEE80211_FC0_SUBTYPE_MASK) >> IEEE80211_FC0_SUBTYPE_SHIFT], ieee80211_chan2ieee(ic, ic->ic_curchan));
	}
#endif
	IEEE80211_NODE_STAT(ni, tx_mgmt);

	(void)ic->ic_mgtstart(ic, skb);
}

/* Send a null data frame to the specified node.
 *
 * NB: the caller provides us with our own node reference this must not be 
 *     leaked; this is necessary to deal with a race condition when 
 *     probing for inactive stations. */
int ieee80211_send_nulldata(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct sk_buff *skb;
	struct ieee80211_frame *wh;
	u_int8_t *frm;

	skb = ieee80211_getmgtframe(&frm, 0);
	if (skb == NULL) {
		/* XXX debug msg */
		vap->iv_stats.is_tx_nobuf++;
		ieee80211_unref_node(&ni);
		return -ENOMEM;
	}

	wh = ieee80211_send_setup(vap, ni, skb, IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_NODATA, vap->iv_myaddr, ni->ni_macaddr, vap->iv_bssid);
	/* NB: power management bit is never sent by an AP */
	if ((IEEE80211_VAP_IS_SLEEPING(ni->ni_vap)) && vap->iv_opmode != IEEE80211_M_HOSTAP && vap->iv_opmode != IEEE80211_M_WDS)
		wh->i_fc[1] |= IEEE80211_FC1_PWR_MGT;

	IEEE80211_NODE_STAT(ni, tx_data);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_DUMPPKTS,
			  "[" MAC_FMT "] send null data frame on channel %u, pwr mgt %s\n", MAC_ADDR(ni->ni_macaddr), ieee80211_chan2ieee(ic, ic->ic_curchan), wh->i_fc[1] & IEEE80211_FC1_PWR_MGT ? "ena" : "dis");

	/* XXX assign some priority; this probably is wrong */
	skb->priority = WME_AC_BE;
	SKB_CB(skb)->ni = PASS_NODE(ni);

	(void)ic->ic_mgtstart(ic, skb);	/* cheat */

	return 0;
}

#if 1
static struct ieee80211_node *cn_alloc_node(struct ieee80211vap *vap, const u_int8_t *macaddr)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;

	/* This always allocates zeroed memoery */
	ni = ic->ic_node_alloc(vap);
	if (ni != NULL) {
//              IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE,"%s: %p<%s> refcnt %d\n", __func__, ni, MAC_ADDR(macaddr),
		atomic_set(&ni->ni_refcnt, 1);

		IEEE80211_ADDR_COPY(ni->ni_macaddr, macaddr);

		ni->ni_chan = IEEE80211_CHAN_ANYC;
		ni->ni_authmode = IEEE80211_AUTH_OPEN;
		ni->ni_txpower = ic->ic_txpowlimit;

		ieee80211_crypto_resetkey(vap, &ni->ni_ucastkey, IEEE80211_KEYIX_NONE);
		ni->ni_ath_defkeyindex = IEEE80211_INVAL_DEFKEY;

		ni->ni_vap = vap;
		ni->ni_ic = ic;
		atomic_inc(&ni->ni_ic->ic_node_counter);
	} else {
		/* XXX msg */
		vap->iv_stats.is_rx_nodealloc++;
	}
	return ni;
}

void ieee80211_ctstimeout(unsigned long arg)
{
	struct ieee80211vap *vap = (struct ieee80211vap *)arg;	/*container_of(arg, struct ieee80211vap, iv_cn_ctstimer); */
	u_int8_t mac[IEEE80211_ADDR_LEN];
	uint32_t ctstimeout;

	//vap->iv_policy_glue->cn_event( vap, CN_CTS_TIMEOUT, NULL );    
	vap->iv_policy_glue->cn_action(vap, NULL, mac, 0);
	ieee80211_send_cts(vap, mac, 0, NULL);

#if (0)
//              printk(KERN_INFO "%s: CTS sent: \n", 
//                      __func__/*, ether_sprintf( mac )*/ );
#endif
}

EXPORT_SYMBOL(ieee80211_ctstimeout);

/*
 * Send a cts frame to the specified node to allow it to transmit data.
 *
 * NB: the caller is assumed to have setup a node reference
 *     for use; this is necessary to deal with a race condition
 *     when probing for inactive stations.
 */
int ieee80211_send_cts(struct ieee80211vap *vap, const u_int8_t mac[IEEE80211_ADDR_LEN], uint32_t ctstimeout, struct ieee80211_node *ni)
{
	//struct ieee80211_node *ni;
	//struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct sk_buff *skb;
	struct ieee80211_cb *cb;
	struct ieee80211_frame *wh;
	u_int8_t *frm;

	if (ni == NULL)
		ni = ieee80211_find_node(&ic->ic_sta, mac);

	if (ni == NULL) {

//              printk(KERN_INFO "ieee80211_send_cts: %s for " MAC_FMT "\n", "New Node allocated",MAC_ADDR(mac));

		ni = cn_alloc_node(vap, mac);
		if (ni == NULL) {
			/* XXX recovery? */
			return -1;
		}
	}			/* else {
				   ieee80211_ref_node(ni);
				   } */

	skb = ieee80211_getmgtframe(&frm, 0);
	if (skb == NULL) {
		/* XXX debug msg */
		vap->iv_stats.is_tx_nobuf++;
		ieee80211_unref_node(&ni);
		return -ENOMEM;
	}
	SKB_CB(skb)->ni = ni;

//      wh = (struct ieee80211_frame *)
//              skb_push(skb, sizeof(struct ieee80211_frame_cts) + 
//              roundup(sizeof(struct ieee80211_frame_cts), 4) - sizeof(struct ieee80211_frame_cts));

	wh = ieee80211_send_setup(vap, ni, skb, IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_CTS, vap->iv_myaddr, ni->ni_macaddr, vap->iv_bssid);
	/* NB: power management bit is never sent by an AP */
	if ((IEEE80211_VAP_IS_SLEEPING(ni->ni_vap)) && vap->iv_opmode != IEEE80211_M_HOSTAP && vap->iv_opmode != IEEE80211_M_WDS)
		wh->i_fc[1] |= IEEE80211_FC1_PWR_MGT;

	IEEE80211_NODE_STAT(ni, tx_data);

//      printk(KERN_INFO "[" MAC_FMT "] send cts control frame on channel %u, pwr mgt %s\n",
//              MAC_ADDR(ni->ni_macaddr),
//              ieee80211_chan2ieee(ic, ic->ic_curchan),
//              wh->i_fc[1] & IEEE80211_FC1_PWR_MGT ? "ena" : "dis");
//
/*	IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE,
		"ieee80211_ref_node (%s:%u) %p<%s> refcnt %d\n",
		__func__, __LINE__,
		ni, MAC_ADDR(ni->ni_macaddr),
		ieee80211_node_refcnt(ni));
*/
	/* XXX assign some priority; this probably is wrong */
	skb->priority = WME_AC_BE;

	(void)ic->ic_mgtstart(ic, skb);	/* cheat */
#if (0)
	vap->iv_cn_ctstimer.expires = jiffies + msecs_to_jiffies(1 /*ctstimeout */ );
	add_timer(&vap->iv_cn_ctstimer);
#endif

	return 0;
}

EXPORT_SYMBOL(ieee80211_send_cts);
#endif

/*
 * NB: unlike ieee80211_send_nulldata(), the node refcnt is
 *     bumped within this function.
 */
int ieee80211_send_qosnulldata(struct ieee80211_node *ni, int ac)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct sk_buff *skb;
	struct ieee80211_qosframe *qwh;
	u_int8_t *frm;
	u_int8_t *i_qos;
	int tid;

	skb = ieee80211_getmgtframe(&frm, 2);
	if (skb == NULL) {
		/* XXX debug msg */
		vap->iv_stats.is_tx_nobuf++;
		return -ENOMEM;
	}
	SKB_CB(skb)->ni = ieee80211_ref_node(ni);

	skb->priority = ac;

	/* grab a pointer to QoS control and also compensate for the header length
	 * difference between QoS and non-QoS frame */
	i_qos = skb_push(skb, sizeof(struct ieee80211_qosframe) - sizeof(struct ieee80211_frame));

	qwh = (struct ieee80211_qosframe *)ieee80211_send_setup(vap, ni, skb, IEEE80211_FC0_TYPE_DATA, vap->iv_myaddr,	/* SA */
								ni->ni_macaddr,	/* DA */
								vap->iv_bssid);

	qwh->i_fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_QOS_NULL;

	if (IEEE80211_VAP_IS_SLEEPING(ni->ni_vap))
		qwh->i_fc[1] |= IEEE80211_FC1_PWR_MGT;

	/* map from access class/queue to 11e header priority value */
	tid = WME_AC_TO_TID(ac);
	i_qos[0] = tid & IEEE80211_QOS_TID;
	if (ic->ic_wme.wme_wmeChanParams.cap_wmeParams[ac].wmep_noackPolicy)
		qwh->i_qos[0] |= (1 << IEEE80211_QOS_ACKPOLICY_S) & IEEE80211_QOS_ACKPOLICY;
	i_qos[1] = 0;

	IEEE80211_NODE_STAT(ni, tx_data);

	if (WME_UAPSD_AC_CAN_TRIGGER(skb->priority, ni)) {
		/* U-APSD power save queue */
		/* XXXAPSD: assuming triggerable means deliverable */
		M_FLAG_SET(skb, M_UAPSD);
	}

	(void)ic->ic_mgtstart(ic, skb);	/* cheat */

	return 0;
}

EXPORT_SYMBOL(ieee80211_send_qosnulldata);

/*
 * Ensure there is sufficient headroom and tailroom to
 * encapsulate the 802.11 data frame.  If room isn't
 * already there, reallocate so there is enough space.
 * Drivers and cipher modules assume we have done the
 * necessary work and fail rudely if they don't find
 * the space they need.
 */
static struct sk_buff *ieee80211_skbhdr_adjust(struct ieee80211vap *vap, int hdrsize, struct ieee80211_key *key, struct sk_buff *skb, int ismulticast)
{
	/* XXX pre-calculate per node? */
	int need_headroom = LLC_SNAPFRAMELEN + hdrsize;
	int need_tailroom = 0;
#ifdef ATH_SUPERG_FF
	int isff = ATH_FF_MAGIC_PRESENT(skb);
	int inter_headroom = sizeof(struct ether_header) + LLC_SNAPFRAMELEN + ATH_FF_MAX_HDR_PAD;
	struct sk_buff *skb2 = NULL;

	if (isff) {
		need_headroom += sizeof(struct athl2p_tunnel_hdr) + ATH_FF_MAX_HDR_PAD + inter_headroom;
		skb2 = skb->next;
	}
#endif

	if (key != NULL) {
		const struct ieee80211_cipher *cip = key->wk_cipher;
		/*
		 * Adjust for crypto needs.  When hardware crypto is
		 * being used we assume the hardware/driver will deal
		 * with any padding (on the fly, without needing to
		 * expand the frame contents).  When software crypto
		 * is used we need to ensure room is available at the
		 * front and back and also for any per-MSDU additions.
		 */
		/* XXX belongs in crypto code? */
		need_headroom += cip->ic_header;
		/* XXX pre-calculate per key */
		if (key->wk_flags & IEEE80211_KEY_SWCRYPT)
			need_tailroom += cip->ic_trailer;
		/* 
		 ** If tx frag is needed and cipher is TKIP,
		 ** then allocate the additional tailroom for SW MIC computation.
		 */
		if (skb->len > vap->iv_fragthreshold && !ismulticast && cip->ic_cipher == IEEE80211_CIPHER_TKIP)
			need_tailroom += cip->ic_miclen;
		else if (key->wk_flags & IEEE80211_KEY_SWMIC)
			need_tailroom += cip->ic_miclen;
	}

	need_headroom -= skb_headroom(skb);
	if (isff)
		need_tailroom -= skb_tailroom(skb2);
	else
		need_tailroom -= skb_tailroom(skb);

	if (need_headroom < 0)
		need_headroom = 0;
	if (need_tailroom < 0)
		need_tailroom = 0;

	if (skb_cloned(skb) || (need_headroom > 0) || (!isff && (need_tailroom > 0))) {

		if (pskb_expand_head(skb, need_headroom, need_tailroom, GFP_ATOMIC)) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_OUTPUT, "%s: cannot expand storage (tail)\n", __func__);
			goto error;
		}
	}

#ifdef ATH_SUPERG_FF
	if (isff) {
		inter_headroom -= skb_headroom(skb2);
		if (inter_headroom < 0)
			inter_headroom = 0;
		if ((skb_cloned(skb2) || (inter_headroom > 0) || (need_tailroom > 0))) {

			if (pskb_expand_head(skb2, inter_headroom, need_tailroom, GFP_ATOMIC)) {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_OUTPUT, "%s: cannot expand storage (tail)\n", __func__);
				goto error;
			}
		}
		skb->next = skb2;
	}
#endif				/* ATH_SUPERG_FF */

	return skb;

error:
	vap->iv_stats.is_tx_nobuf++;
	ieee80211_dev_kfree_skb(&skb);
#ifdef ATH_SUPERG_FF
	if (skb2)
		ieee80211_dev_kfree_skb(&skb2);
#endif
	return NULL;
}

#define	KEY_UNDEFINED(k)	((k).wk_cipher == &ieee80211_cipher_none)
/*
 * Return the transmit key to use in sending a unicast frame.
 * If a unicast key is set we use that.  When no unicast key is set
 * we fall back to the default transmit key.
 */
static __inline struct ieee80211_key *ieee80211_crypto_getucastkey(struct ieee80211vap *vap, struct ieee80211_node *ni)
{
	if (KEY_UNDEFINED(ni->ni_ucastkey)) {
		if (vap->iv_def_txkey == IEEE80211_KEYIX_NONE || KEY_UNDEFINED(vap->iv_nw_keys[vap->iv_def_txkey]))
			return NULL;
		return &vap->iv_nw_keys[vap->iv_def_txkey];
	} else
		return &ni->ni_ucastkey;
}

/*
 * Return the transmit key to use in sending a multicast frame.
 * Multicast traffic always uses the group key which is installed as
 * the default tx key.
 */
static __inline struct ieee80211_key *ieee80211_crypto_getmcastkey(struct ieee80211vap *vap, struct ieee80211_node *ni)
{
	if (vap->iv_def_txkey == IEEE80211_KEYIX_NONE || KEY_UNDEFINED(vap->iv_nw_keys[vap->iv_def_txkey]))
		return NULL;
	return &vap->iv_nw_keys[vap->iv_def_txkey];
}

/*
 * Encapsulate an outbound data frame.	The mbuf chain is updated and
 * a reference to the destination node is returned.  If an error is
 * encountered NULL is returned and the node reference will also be NULL.
 *
 * NB: The caller is responsible for freeing a returned node reference.
 *     The convention is ic_bss is not reference counted; the caller must
 *     maintain that.
 */
struct sk_buff *ieee80211_encap(struct ieee80211_node *ni, struct sk_buff *skb, int *framecnt)
{
#define	WH4(wh)	((struct ieee80211_frame_addr4 *)wh)
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct ether_header eh;
	struct ieee80211_frame *wh, *twh;
	struct ieee80211_key *key;
	struct llc *llc;
	int hdrsize, datalen, addqos;
	int hdrsize_nopad;
	struct sk_buff *framelist = NULL;
	struct sk_buff *tskb;
	int fragcnt = 1;
	int pdusize = 0;
	int ismulticast = 0;
	int use4addr = 0;
#ifdef ATH_SUPERG_FF
	struct sk_buff *skb2 = NULL;
	struct ether_header eh2;
	int isff = ATH_FF_MAGIC_PRESENT(skb);

	if (isff) {
#if 0
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SUPG, "%s: handling fast-frame skb (%p)\n", __func__, skb);
#endif
		skb2 = skb->next;
		if (skb2 == NULL) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_SUPG, "%s: fast-frame error, only 1 skb\n", __func__);
			goto bad;
		}
		memcpy(&eh2, skb2->data, sizeof(struct ether_header));
		skb_pull(skb2, sizeof(struct ether_header));
	}
#endif
	memcpy(&eh, skb->data, sizeof(struct ether_header));
	skb_pull(skb, sizeof(struct ether_header));

	/*
	 * Ensure space for additional headers.  First identify
	 * transmit key to use in calculating any buffer adjustments
	 * required.  This is also used below to do privacy
	 * encapsulation work.  Then calculate the 802.11 header
	 * size and any padding required by the driver.
	 *
	 * Note key may be NULL if we fall back to the default
	 * transmit key and that is not set.  In that case the
	 * buffer may not be expanded as needed by the cipher
	 * routines, but they will/should discard it.
	 */
	if (vap->iv_flags & IEEE80211_F_PRIVACY) {
		if (vap->iv_opmode == IEEE80211_M_STA || !IEEE80211_IS_MULTICAST(eh.ether_dhost))
			key = ieee80211_crypto_getucastkey(vap, ni);
		else
			key = ieee80211_crypto_getmcastkey(vap, ni);
		if ((key == NULL) && (eh.ether_type != htons(ETHERTYPE_PAE))) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_CRYPTO, eh.ether_dhost, "no default transmit key (%s) deftxkey %u", __func__, vap->iv_def_txkey);
			vap->iv_stats.is_tx_nodefkey++;
			goto bad;
		}
	} else
		key = NULL;

	addqos = (ni->ni_flags & IEEE80211_NODE_QOS) && (eh.ether_type != htons(ETHERTYPE_PAE));
	if (addqos)
		hdrsize = sizeof(struct ieee80211_qosframe);
	else
		hdrsize = sizeof(struct ieee80211_frame);

	SKB_CB(skb)->auth_pkt = (eh.ether_type == __constant_htons(ETHERTYPE_PAE));
	if (ni->ni_subif)
		vap = ni->ni_subif;

	switch (vap->iv_opmode) {
	case IEEE80211_M_IBSS:
	case IEEE80211_M_AHDEMO:
		ismulticast = IEEE80211_IS_MULTICAST(eh.ether_dhost);
		break;
	case IEEE80211_M_WDS:
		use4addr = 1;
		ismulticast = 0;
		break;
	case IEEE80211_M_HOSTAP:
		if (!IEEE80211_IS_MULTICAST(eh.ether_dhost) && !IEEE80211_ADDR_EQ(eh.ether_dhost, ni->ni_macaddr)) {
			use4addr = 1;
			ismulticast = IEEE80211_IS_MULTICAST(ni->ni_macaddr);
		} else if (IEEE80211_NODE_IS_MTIKWDS(ni)) {
			/* mtik client wds speaks only 4 addr */
			ismulticast = 0;
			use4addr = 1;
		} else
			ismulticast = IEEE80211_IS_MULTICAST(eh.ether_dhost);
		break;
	case IEEE80211_M_STA:
		if ((vap->iv_flags_ext & IEEE80211_FEXT_WDS) || IEEE80211_NODE_IS_MTIKWDS(ni)) {
			use4addr = 1;
			ismulticast = 0;
		} else
			ismulticast = IEEE80211_IS_MULTICAST(vap->iv_bssid);
		break;
	default:
		break;
	}

	if (use4addr)
		hdrsize += IEEE80211_ADDR_LEN;

	hdrsize_nopad = hdrsize;
	if (ic->ic_flags & IEEE80211_F_DATAPAD)
		hdrsize = roundup(hdrsize, sizeof(u_int32_t));

	skb = ieee80211_skbhdr_adjust(vap, hdrsize, key, skb, ismulticast);
	if (skb == NULL) {
		/* NB: ieee80211_skbhdr_adjust handles msgs+statistics */
		goto bad;
	}

#ifdef ATH_SUPERG_FF
	if (isff) {
		struct ether_header *eh_inter;
		struct athl2p_tunnel_hdr *ffhdr;
		u_int16_t payload = skb->len + LLC_SNAPFRAMELEN;
		int padded_len = payload + LLC_SNAPFRAMELEN + sizeof(struct ether_header);

		/* in case header adjustments altered skb2 */
		skb2 = skb->next;
		if (skb2 == NULL) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_SUPG, "%s: skb (%p) hdr adjust dropped 2nd skb\n", __func__, skb);
			goto bad;
		}

		/*
		 * add first skb tunnel hdrs
		 */

		llc = (struct llc *)skb_push(skb, LLC_SNAPFRAMELEN);
		llc->llc_dsap = llc->llc_ssap = LLC_SNAP_LSAP;
		llc->llc_control = LLC_UI;
		llc->llc_snap.org_code[0] = 0;
		llc->llc_snap.org_code[1] = 0;
		llc->llc_snap.org_code[2] = 0;
		llc->llc_snap.ether_type = eh.ether_type;

		eh_inter = (struct ether_header *)skb_push(skb, sizeof(struct ether_header));
		memcpy(eh_inter, &eh, sizeof(struct ether_header) - sizeof eh.ether_type);
		eh_inter->ether_type = htons(payload);

		/* overall ff encap header */
		/* XXX: the offset of 2, below, should be computed. but... it will not
		 *      practically ever change.
		 */
		ffhdr = (struct athl2p_tunnel_hdr *)skb_push(skb, sizeof(struct athl2p_tunnel_hdr) + 2);
		memset(ffhdr, 0, sizeof(struct athl2p_tunnel_hdr) + 2);

		/*
		 * add second skb tunnel hdrs
		 */

		payload = skb2->len + LLC_SNAPFRAMELEN;

		llc = (struct llc *)skb_push(skb2, LLC_SNAPFRAMELEN);
		if (llc == NULL) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_SUPG, "%s: failed to push llc for 2nd skb (%p)\n", __func__, skb);
			return NULL;
		}
		llc->llc_dsap = llc->llc_ssap = LLC_SNAP_LSAP;
		llc->llc_control = LLC_UI;
		llc->llc_snap.org_code[0] = 0;
		llc->llc_snap.org_code[1] = 0;
		llc->llc_snap.org_code[2] = 0;
		llc->llc_snap.ether_type = eh2.ether_type;

		eh_inter = (struct ether_header *)skb_push(skb2, sizeof(struct ether_header));
		if (eh_inter == NULL) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_SUPG, "%s: failed to push eth hdr for 2nd skb (%p)\n", __func__, skb);
			return NULL;
		}

		memcpy(eh_inter, &eh2, sizeof(struct ether_header) - sizeof eh2.ether_type);
		eh_inter->ether_type = htons(payload);

		/* variable length pad */
		skb_push(skb2, roundup(padded_len, 4) - padded_len);
	}
#endif

	llc = (struct llc *)skb_push(skb, LLC_SNAPFRAMELEN);
	llc->llc_dsap = llc->llc_ssap = LLC_SNAP_LSAP;
	llc->llc_control = LLC_UI;
#ifndef ATH_SUPERG_FF
	llc->llc_snap.org_code[0] = 0;
	llc->llc_snap.org_code[1] = 0;
	llc->llc_snap.org_code[2] = 0;
	llc->llc_snap.ether_type = eh.ether_type;
#else				/* ATH_SUPERG_FF */
	if (isff) {
		llc->llc_snap.org_code[0] = ATH_SNAP_ORGCODE_0;
		llc->llc_snap.org_code[1] = ATH_SNAP_ORGCODE_1;
		llc->llc_snap.org_code[2] = ATH_SNAP_ORGCODE_2;
		llc->llc_snap.ether_type = htons(ATH_ETH_TYPE);
	} else {
		llc->llc_snap.org_code[0] = 0;
		llc->llc_snap.org_code[1] = 0;
		llc->llc_snap.org_code[2] = 0;
		llc->llc_snap.ether_type = eh.ether_type;
	}
#endif				/* ATH_SUPERG_FF */
	datalen = skb->len;	/* NB: w/o 802.11 header */

	wh = (struct ieee80211_frame *)skb_push(skb, hdrsize);
	wh->i_fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_DATA;
	wh->i_dur = 0;
	if (use4addr) {
		wh->i_fc[1] = IEEE80211_FC1_DIR_DSTODS;
		IEEE80211_ADDR_COPY(wh->i_addr1, ni->ni_macaddr);
		IEEE80211_ADDR_COPY(wh->i_addr2, vap->iv_myaddr);
		IEEE80211_ADDR_COPY(wh->i_addr3, eh.ether_dhost);
		IEEE80211_ADDR_COPY(WH4(wh)->i_addr4, eh.ether_shost);
	} else {
		switch (vap->iv_opmode) {
		case IEEE80211_M_IBSS:
		case IEEE80211_M_AHDEMO:
			wh->i_fc[1] = IEEE80211_FC1_DIR_NODS;
			IEEE80211_ADDR_COPY(wh->i_addr1, eh.ether_dhost);
			IEEE80211_ADDR_COPY(wh->i_addr2, eh.ether_shost);
			/*
			 * NB: always use the bssid from iv_bssid as the
			 *     neighbor's may be stale after an ibss merge
			 */
			IEEE80211_ADDR_COPY(wh->i_addr3, vap->iv_bssid);
			break;
		case IEEE80211_M_STA:
			wh->i_fc[1] = IEEE80211_FC1_DIR_TODS;
			IEEE80211_ADDR_COPY(wh->i_addr1, vap->iv_bssid);
			IEEE80211_ADDR_COPY(wh->i_addr2, eh.ether_shost);
			IEEE80211_ADDR_COPY(wh->i_addr3, eh.ether_dhost);
			break;
		case IEEE80211_M_HOSTAP:
			wh->i_fc[1] = IEEE80211_FC1_DIR_FROMDS;
			IEEE80211_ADDR_COPY(wh->i_addr1, eh.ether_dhost);
			IEEE80211_ADDR_COPY(wh->i_addr2, vap->iv_bssid);
			IEEE80211_ADDR_COPY(wh->i_addr3, eh.ether_shost);
			if (M_PWR_SAV_GET(skb)) {
				if (IEEE80211_NODE_SAVEQ_QLEN(ni)) {
					wh->i_fc[1] |= IEEE80211_FC1_MORE_DATA;
					M_PWR_SAV_CLR(skb);
				}
			}
			break;
		case IEEE80211_M_WDS:
			wh->i_fc[1] = IEEE80211_FC1_DIR_DSTODS;
			IEEE80211_ADDR_COPY(wh->i_addr1, vap->wds_mac);
			IEEE80211_ADDR_COPY(wh->i_addr2, vap->iv_myaddr);
			IEEE80211_ADDR_COPY(wh->i_addr3, eh.ether_dhost);
			IEEE80211_ADDR_COPY(WH4(wh)->i_addr4, eh.ether_shost);
			break;
		case IEEE80211_M_MONITOR:
			goto bad;
		}
	}
	if (IEEE80211_VAP_IS_SLEEPING(vap))
		wh->i_fc[1] |= IEEE80211_FC1_PWR_MGT;
	if (addqos) {
		struct ieee80211_qosframe *qwh = (struct ieee80211_qosframe *)wh;
		u_int8_t *qos;
		int tid;

		qos = &qwh->i_qos[0];
		if (use4addr)
			qos += IEEE80211_ADDR_LEN;
		/* map from access class/queue to 11e header priority value */
		tid = WME_AC_TO_TID(skb->priority);
		qos[0] = tid & IEEE80211_QOS_TID;
		if (ic->ic_wme.wme_wmeChanParams.cap_wmeParams[skb->priority].wmep_noackPolicy)
			qos[0] |= (1 << IEEE80211_QOS_ACKPOLICY_S) & IEEE80211_QOS_ACKPOLICY;
		qos[1] = 0;
		qwh->i_fc[0] |= IEEE80211_FC0_SUBTYPE_QOS;

		*(__le16 *)&wh->i_seq[0] = htole16(ni->ni_txseqs[tid] << IEEE80211_SEQ_SEQ_SHIFT);
		ni->ni_txseqs[tid]++;
	} else {
		*(__le16 *)wh->i_seq = htole16(ni->ni_txseqs[0] << IEEE80211_SEQ_SEQ_SHIFT);
		ni->ni_txseqs[0]++;
	}

	/* Is transmit fragmentation needed? */
	if (skb->len > vap->iv_fragthreshold && !IEEE80211_IS_MULTICAST(wh->i_addr1)) {
		int pktlen, skbcnt, tailsize, ciphdrsize;
		struct ieee80211_cipher *cip;

		pktlen = skb->len;
		ciphdrsize = 0;
		tailsize = IEEE80211_CRC_LEN;

		if (key != NULL) {
			cip = (struct ieee80211_cipher *)key->wk_cipher;
			ciphdrsize = cip->ic_header;
			tailsize += (cip->ic_trailer + cip->ic_miclen);

			/* add the 8 bytes MIC length */
			if (cip->ic_cipher == IEEE80211_CIPHER_TKIP)
				pktlen += IEEE80211_WEP_MICLEN;
		}

		pdusize = vap->iv_fragthreshold - (hdrsize_nopad + ciphdrsize);
		fragcnt = *framecnt = ((pktlen - hdrsize_nopad) / pdusize) + (((pktlen - hdrsize_nopad) % pdusize == 0) ? 0 : 1);

		/*
		 * Allocate sk_buff for each subsequent fragment; First fragment
		 * reuses input skb.
		 */
		for (skbcnt = 1; skbcnt < fragcnt; skbcnt++) {
			tskb = ieee80211_dev_alloc_skb(hdrsize + ciphdrsize + pdusize + tailsize);
			if (tskb == NULL)
				break;

			tskb->next = framelist;
			framelist = tskb;
		}

		if (skbcnt != fragcnt)
			goto bad;
	} else
		*framecnt = fragcnt;

	if (key != NULL) {
		/*
		 * IEEE 802.1X: send EAPOL frames always in the clear.
		 * WPA/WPA2: encrypt EAPOL keys when pairwise keys are set.
		 */
		if (eh.ether_type != __constant_htons(ETHERTYPE_PAE) || ((vap->iv_flags & IEEE80211_F_WPA) && (vap->iv_opmode == IEEE80211_M_STA ? !KEY_UNDEFINED(*key) : !KEY_UNDEFINED(ni->ni_ucastkey)))) {
			int force_swmic = (fragcnt > 1) ? 1 : 0;

			wh->i_fc[1] |= IEEE80211_FC1_PROT;

			if (!ieee80211_crypto_enmic(vap, key, skb, force_swmic)) {
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_OUTPUT, eh.ether_dhost, "%s", "enmic failed, discard frame");
				vap->iv_stats.is_crypto_enmicfail++;
				goto bad;
			}
		}
	}

	if (fragcnt > 1) {
		int fragnum, offset, pdulen;
		void *pdu;

		fragnum = 0;
		wh = twh = (struct ieee80211_frame *)skb->data;

		/*
		 ** Setup WLAN headers as fragment headers
		 */
		wh->i_fc[1] |= IEEE80211_FC1_MORE_FRAG;

		*(__le16 *)&wh->i_seq[0] |= htole16((fragnum & IEEE80211_SEQ_FRAG_MASK) << IEEE80211_SEQ_FRAG_SHIFT);
		fragnum++;

		offset = hdrsize + pdusize;
		datalen = (skb->len - hdrsize) - pdusize;

		IEEE80211_NODE_STAT(ni, tx_data);
		IEEE80211_NODE_STAT_ADD(ni, tx_bytes, pdusize);

		for (tskb = framelist; tskb != NULL; tskb = tskb->next) {
			/*
			 * Copy WLAN header into each frag header skb
			 */
			twh = (struct ieee80211_frame *)skb_put(tskb, hdrsize);
			memcpy((void *)twh, (void *)wh, hdrsize);

			*(__le16 *)&twh->i_seq[0] |= htole16((fragnum & IEEE80211_SEQ_FRAG_MASK) << IEEE80211_SEQ_FRAG_SHIFT);
			fragnum++;

			if (pdusize <= datalen)
				pdulen = pdusize;
			else
				pdulen = datalen;

			/*
			 * Copy fragment payload from input skb.
			 * Doing copies isn't intuitive from 
			 * a performance perspective, however,
			 * for this case, it is believed to be 
			 * more efficient than cloning skbs.
			 */
			pdu = skb_put(tskb, pdulen);
			memcpy(pdu, (void *)skb->data + offset, pdulen);

			offset += pdusize;
			datalen -= pdusize;

			IEEE80211_NODE_STAT(ni, tx_data);
			IEEE80211_NODE_STAT_ADD(ni, tx_bytes, pdulen);
		}

		twh->i_fc[1] &= ~IEEE80211_FC1_MORE_FRAG;
		skb_trim(skb, hdrsize + pdusize);
		skb->next = framelist;
	} else {
		IEEE80211_NODE_STAT(ni, tx_data);
		IEEE80211_NODE_STAT_ADD(ni, tx_bytes, datalen);

#ifdef ATH_SUPERG_FF
		/* Account for a second skb in the same packet when FF is on */
		if (skb->next) {
			datalen = skb->next->len;
			IEEE80211_NODE_STAT(ni, tx_data);
			IEEE80211_NODE_STAT_ADD(ni, tx_bytes, datalen);
		}
#endif
	}

	return skb;
bad:
	if (framelist != NULL) {
		ieee80211_dev_kfree_skb_list(&framelist);
	}

	if (skb != NULL) {
		ieee80211_dev_kfree_skb_list(&skb);
	}
	return NULL;
#undef WH4
}

EXPORT_SYMBOL(ieee80211_encap);
#undef KEY_UNDEFINED

/*
 * Add a supported rates element id to a frame.
 */
u_int8_t *ieee80211_add_rates(u_int8_t *frm, const struct ieee80211_rateset *rs)
{
	int nrates;

	*frm++ = IEEE80211_ELEMID_RATES;
	nrates = rs->rs_nrates;
	if (nrates > IEEE80211_RATE_SIZE)
		nrates = IEEE80211_RATE_SIZE;
	*frm++ = nrates;
	memcpy(frm, rs->rs_rates, nrates);
	return frm + nrates;
}

/*
 * Add an extended supported rates element id to a frame.
 */
u_int8_t *ieee80211_add_xrates(u_int8_t *frm, const struct ieee80211_rateset *rs)
{
	/*
	 * Add an extended supported rates element if operating in 11g mode.
	 */
	if (rs->rs_nrates > IEEE80211_RATE_SIZE) {
		int nrates = rs->rs_nrates - IEEE80211_RATE_SIZE;
		*frm++ = IEEE80211_ELEMID_XRATES;
		*frm++ = nrates;
		memcpy(frm, rs->rs_rates + IEEE80211_RATE_SIZE, nrates);
		frm += nrates;
	}
	return frm;
}

/*
 * Add an ssid elemet to a frame.
 */
static u_int8_t *ieee80211_add_ssid(u_int8_t *frm, const u_int8_t *ssid, u_int len)
{
	*frm++ = IEEE80211_ELEMID_SSID;
	*frm++ = len;
	memcpy(frm, ssid, len);
	return frm + len;
}

/*
 * Add an erp element to a frame.
 */
u_int8_t *ieee80211_add_erp(u_int8_t *frm, struct ieee80211com *ic)
{
	u_int8_t erp;

	*frm++ = IEEE80211_ELEMID_ERP;
	*frm++ = 1;
	erp = 0;
	if (ic->ic_nonerpsta != 0)
		erp |= IEEE80211_ERP_NON_ERP_PRESENT;
	if (ic->ic_flags & IEEE80211_F_USEPROT)
		erp |= IEEE80211_ERP_USE_PROTECTION;
	if ((ic->ic_flags & IEEE80211_F_USEBARKER) || (ic->ic_nonerpsta > 0))
		erp |= IEEE80211_ERP_LONG_PREAMBLE;
	*frm++ = erp;
	return frm;
}

/*
 * Add a country information element to a frame.
 */
u_int8_t *ieee80211_add_country(u_int8_t *frm, struct ieee80211com *ic)
{
	/* add country code */
	memcpy(frm, (u_int8_t *)&ic->ic_country_ie, ic->ic_country_ie.country_len + 2);
	frm += ic->ic_country_ie.country_len + 2;
	return frm;
}

/*
 * Add Power Constraint information element
 */
u_int8_t *ieee80211_add_pwrcnstr(u_int8_t *frm, struct ieee80211com *ic)
{
	struct ieee80211_ie_pwrcnstr *ie = (struct ieee80211_ie_pwrcnstr *)frm;
	ie->pc_id = IEEE80211_ELEMID_PWRCNSTR;
	ie->pc_len = 1;
	ie->pc_lpc = IEEE80211_PWRCONSTRAINT_VAL(ic);
	frm += sizeof(*ie);
	return frm;
}

/*
 * Add Power Capability information element
 */
#if 0
static u_int8_t *ieee80211_add_pwrcap(u_int8_t *frm, struct ieee80211com *ic)
{
	struct ieee80211_ie_pwrcap *ie = (struct ieee80211_ie_pwrcap *)frm;
	ie->pc_id = IEEE80211_ELEMID_PWRCAP;
	ie->pc_len = 2;
	ie->pc_mintxpow = ic->ic_bsschan->ic_minpower;
	ie->pc_maxtxpow = ic->ic_bsschan->ic_maxpower;
	frm += sizeof(*ie);
	return frm;
}

/*
 * Add Supported Channels information element
 */
static u_int8_t *ieee80211_add_suppchan(u_int8_t *frm, struct ieee80211com *ic)
{
	memcpy(frm, (u_int8_t *)&ic->ic_sc_ie, ic->ic_sc_ie.sc_len + 2);
	frm += ic->ic_sc_ie.sc_len + 2;
	return frm;
}
#endif
static u_int8_t *ieee80211_setup_wpa_ie(struct ieee80211vap *vap, u_int8_t *ie)
{
#define	WPA_OUI_BYTES		0x00, 0x50, 0xf2
#define	ADDSHORT(frm, v) do {			\
	frm[0] = (v) & 0xff;			\
	frm[1] = (v) >> 8;			\
	frm += 2;				\
} while (0)
#define	ADDSELECTOR(frm, sel) do {		\
	memcpy(frm, sel, 4);			\
	frm += 4;				\
} while (0)
	static const u_int8_t oui[4] = { WPA_OUI_BYTES, MADWIFI_WPA_OUI_TYPE };
	static const u_int8_t cipher_suite[][4] = {
		{ WPA_OUI_BYTES, WPA_CSE_WEP40 },	/* NB: 40-bit */
		{ WPA_OUI_BYTES, WPA_CSE_TKIP },
		{ 0x00, 0x00, 0x00, 0x00 },	/* XXX WRAP */
		{ WPA_OUI_BYTES, WPA_CSE_CCMP },
		{ 0x00, 0x00, 0x00, 0x00 },	/* XXX CKIP */
		{ WPA_OUI_BYTES, WPA_CSE_NULL },
	};
	static const u_int8_t wep104_suite[4] = { WPA_OUI_BYTES, WPA_CSE_WEP104 };
	static const u_int8_t key_mgt_unspec[4] = { WPA_OUI_BYTES, WPA_ASE_8021X_UNSPEC };
	static const u_int8_t key_mgt_psk[4] = { WPA_OUI_BYTES, WPA_ASE_8021X_PSK };
	const struct ieee80211_rsnparms *rsn = &vap->iv_bss->ni_rsn;
	u_int8_t *frm = ie;
	u_int8_t *selcnt;

	*frm++ = IEEE80211_ELEMID_VENDOR;
	*frm++ = 0;		/* length filled in below */
	memcpy(frm, oui, sizeof(oui));	/* WPA OUI */
	frm += sizeof(oui);
	ADDSHORT(frm, WPA_VERSION);

	/* XXX filter out CKIP */

	/* multicast cipher */
	if (rsn->rsn_mcastcipher == IEEE80211_CIPHER_WEP && rsn->rsn_mcastkeylen >= 13)
		ADDSELECTOR(frm, wep104_suite);
	else
		ADDSELECTOR(frm, cipher_suite[rsn->rsn_mcastcipher]);

	/* unicast cipher list */
	selcnt = frm;
	ADDSHORT(frm, 0);	/* selector count */
	if (rsn->rsn_ucastcipherset & (1 << IEEE80211_CIPHER_AES_CCM)) {
		selcnt[0]++;
		ADDSELECTOR(frm, cipher_suite[IEEE80211_CIPHER_AES_CCM]);
	}
	if (rsn->rsn_ucastcipherset & (1 << IEEE80211_CIPHER_TKIP)) {
		selcnt[0]++;
		ADDSELECTOR(frm, cipher_suite[IEEE80211_CIPHER_TKIP]);
	}

	/* authenticator selector list */
	selcnt = frm;
	ADDSHORT(frm, 0);	/* selector count */
	if (rsn->rsn_keymgmtset & WPA_ASE_8021X_UNSPEC) {
		selcnt[0]++;
		ADDSELECTOR(frm, key_mgt_unspec);
	}
	if (rsn->rsn_keymgmtset & WPA_ASE_8021X_PSK) {
		selcnt[0]++;
		ADDSELECTOR(frm, key_mgt_psk);
	}

	/* optional capabilities */
	if ((rsn->rsn_caps != 0) && (rsn->rsn_caps != RSN_CAP_PREAUTH))
		ADDSHORT(frm, rsn->rsn_caps);

	/* calculate element length */
	ie[1] = frm - ie - 2;
	KASSERT(ie[1] + 2 <= sizeof(struct ieee80211_ie_wpa), ("WPA IE too big, %u > %u", ie[1] + 2, (int)sizeof(struct ieee80211_ie_wpa)));
	return frm;
#undef ADDSHORT
#undef ADDSELECTOR
#undef WPA_OUI_BYTES
}

static u_int8_t *ieee80211_setup_rsn_ie(struct ieee80211vap *vap, u_int8_t *ie)
{
#define	RSN_OUI_BYTES		0x00, 0x0f, 0xac
#define	ADDSHORT(frm, v) do {			\
	frm[0] = (v) & 0xff;			\
	frm[1] = (v) >> 8;			\
	frm += 2;				\
} while (0)
#define	ADDSELECTOR(frm, sel) do {		\
	memcpy(frm, sel, 4);			\
	frm += 4;				\
} while (0)
	static const u_int8_t cipher_suite[][4] = {
		{ RSN_OUI_BYTES, RSN_CSE_WEP40 },	/* NB: 40-bit */
		{ RSN_OUI_BYTES, RSN_CSE_TKIP },
		{ RSN_OUI_BYTES, RSN_CSE_WRAP },
		{ RSN_OUI_BYTES, RSN_CSE_CCMP },
		{ 0x00, 0x00, 0x00, 0x00 },	/* XXX CKIP */
		{ RSN_OUI_BYTES, RSN_CSE_NULL },
	};
	static const u_int8_t wep104_suite[4] = { RSN_OUI_BYTES, RSN_CSE_WEP104 };
	static const u_int8_t key_mgt_unspec[4] = { RSN_OUI_BYTES, RSN_ASE_8021X_UNSPEC };
	static const u_int8_t key_mgt_psk[4] = { RSN_OUI_BYTES, RSN_ASE_8021X_PSK };
	const struct ieee80211_rsnparms *rsn = &vap->iv_bss->ni_rsn;
	u_int8_t *frm = ie;
	u_int8_t *selcnt;

	*frm++ = IEEE80211_ELEMID_RSN;
	*frm++ = 0;		/* length filled in below */
	ADDSHORT(frm, RSN_VERSION);

	/* XXX filter out CKIP */

	/* multicast cipher */
	if (rsn->rsn_mcastcipher == IEEE80211_CIPHER_WEP && rsn->rsn_mcastkeylen >= 13)
		ADDSELECTOR(frm, wep104_suite);
	else
		ADDSELECTOR(frm, cipher_suite[rsn->rsn_mcastcipher]);

	/* unicast cipher list */
	selcnt = frm;
	ADDSHORT(frm, 0);	/* selector count */
	if (rsn->rsn_ucastcipherset & (1 << IEEE80211_CIPHER_AES_CCM)) {
		selcnt[0]++;
		ADDSELECTOR(frm, cipher_suite[IEEE80211_CIPHER_AES_CCM]);
	}
	if (rsn->rsn_ucastcipherset & (1 << IEEE80211_CIPHER_TKIP)) {
		selcnt[0]++;
		ADDSELECTOR(frm, cipher_suite[IEEE80211_CIPHER_TKIP]);
	}

	/* authenticator selector list */
	selcnt = frm;
	ADDSHORT(frm, 0);	/* selector count */
	if (rsn->rsn_keymgmtset & WPA_ASE_8021X_UNSPEC) {
		selcnt[0]++;
		ADDSELECTOR(frm, key_mgt_unspec);
	}
	if (rsn->rsn_keymgmtset & WPA_ASE_8021X_PSK) {
		selcnt[0]++;
		ADDSELECTOR(frm, key_mgt_psk);
	}

	/* capabilities */
	ADDSHORT(frm, rsn->rsn_caps);
	/* XXX PMKID */

	/* calculate element length */
	ie[1] = frm - ie - 2;
	KASSERT(ie[1] + 2 <= sizeof(struct ieee80211_ie_wpa), ("RSN IE too big, %u > %u", ie[1] + 2, (int)sizeof(struct ieee80211_ie_wpa)));
	return frm;
#undef ADDSELECTOR
#undef ADDSHORT
#undef RSN_OUI_BYTES
}

/*
 * Add a WPA/RSN element to a frame.
 */
u_int8_t *ieee80211_add_wpa(u_int8_t *frm, struct ieee80211vap *vap)
{

	KASSERT(vap->iv_flags & IEEE80211_F_WPA, ("no WPA/RSN!"));
	if (vap->iv_flags & IEEE80211_F_WPA2)
		frm = ieee80211_setup_rsn_ie(vap, frm);
	if (vap->iv_flags & IEEE80211_F_WPA1)
		frm = ieee80211_setup_wpa_ie(vap, frm);
	return frm;
}

#define	WME_OUI_BYTES		0x00, 0x50, 0xf2
/*
 * Add a WME Info element to a frame.
 */
static u_int8_t *ieee80211_add_wme(u_int8_t *frm, struct ieee80211_node *ni)
{
	static const u_int8_t oui[4] = { WME_OUI_BYTES, WME_OUI_TYPE };
	struct ieee80211_ie_wme *ie = (struct ieee80211_ie_wme *)frm;
	struct ieee80211_wme_state *wme = &ni->ni_ic->ic_wme;
	struct ieee80211vap *vap = ni->ni_vap;

	*frm++ = IEEE80211_ELEMID_VENDOR;
	*frm++ = 0;		/* length filled in below */
	memcpy(frm, oui, sizeof(oui));	/* WME OUI */
	frm += sizeof(oui);
	*frm++ = WME_INFO_OUI_SUBTYPE;	/* OUI subtype */
	*frm++ = WME_VERSION;	/* protocol version */
	/* QoS Info field depends on operating mode */
	switch (vap->iv_opmode) {
	case IEEE80211_M_HOSTAP:
		*frm = wme->wme_bssChanParams.cap_info_count;
		if (IEEE80211_VAP_UAPSD_ENABLED(vap))
			*frm |= WME_CAPINFO_UAPSD_EN;
		frm++;
		break;
	case IEEE80211_M_STA:
		*frm++ = vap->iv_uapsdinfo;
		break;
	default:
		*frm++ = 0;
	}

	ie->wme_len = frm - &ie->wme_oui[0];

	return frm;
}

/*
 * Add a WME Parameter element to a frame.
 */
u_int8_t *ieee80211_add_wme_param(u_int8_t *frm, struct ieee80211_wme_state *wme, int uapsd_enable)
{
#define	SM(_v, _f)	(((_v) << _f##_S) & _f)
#define	ADDSHORT(frm, v) do {			\
	frm[0] = (v) & 0xff;			\
	frm[1] = (v) >> 8;			\
	frm += 2;				\
} while (0)
	static const u_int8_t oui[4] = { WME_OUI_BYTES, WME_OUI_TYPE };
	struct ieee80211_wme_param *ie = (struct ieee80211_wme_param *)frm;
	int i;

	*frm++ = IEEE80211_ELEMID_VENDOR;
	*frm++ = 0;		/* length filled in below */
	memcpy(frm, oui, sizeof(oui));	/* WME OUI */
	frm += sizeof(oui);
	*frm++ = WME_PARAM_OUI_SUBTYPE;	/* OUI subtype */
	*frm++ = WME_VERSION;	/* protocol version */
	*frm = wme->wme_bssChanParams.cap_info_count;
	if (uapsd_enable)
		*frm |= WME_CAPINFO_UAPSD_EN;
	frm++;
	*frm++ = 0;		/* reserved field */
	for (i = 0; i < WME_NUM_AC; i++) {
		const struct wmeParams *ac = &wme->wme_bssChanParams.cap_wmeParams[i];
		*frm++ = SM(i, WME_PARAM_ACI) | SM(ac->wmep_acm, WME_PARAM_ACM) | SM(ac->wmep_aifsn, WME_PARAM_AIFSN);
		*frm++ = SM(ac->wmep_logcwmax, WME_PARAM_LOGCWMAX) | SM(ac->wmep_logcwmin, WME_PARAM_LOGCWMIN);
		ADDSHORT(frm, ac->wmep_txopLimit);
	}

	ie->param_len = frm - &ie->param_oui[0];

	return frm;
#undef ADDSHORT
}

#undef WME_OUI_BYTES

/*
 * Add an Atheros Advanaced Capability element to a frame
 */
u_int8_t *ieee80211_add_athAdvCap(u_int8_t *frm, u_int8_t capability, u_int16_t defaultKey)
{
	static const u_int8_t oui[6] = { (ATH_OUI & 0xff), ((ATH_OUI >> 8) & 0xff),
		((ATH_OUI >> 16) & 0xff), ATH_OUI_TYPE,
		ATH_OUI_SUBTYPE, ATH_OUI_VERSION
	};
	struct ieee80211_ie_athAdvCap *ie = (struct ieee80211_ie_athAdvCap *)frm;

	*frm++ = IEEE80211_ELEMID_VENDOR;
	*frm++ = 0;		/* Length filled in below */
	memcpy(frm, oui, sizeof(oui));	/* Atheros OUI, type, subtype, and version for adv capabilities */
	frm += sizeof(oui);
	*frm++ = capability;

	/* Setup default key index in little endian byte order */
	*frm++ = (defaultKey & 0xff);
	*frm++ = ((defaultKey >> 8) & 0xff);
	ie->athAdvCap_len = frm - &ie->athAdvCap_oui[0];

	return frm;
}

/*
 * Add XR IE element to a frame
 */
#ifdef ATH_SUPERG_XR
u_int8_t *ieee80211_add_xr_param(u_int8_t *frm, struct ieee80211vap *vap)
{
	static const u_int8_t oui[6] = { (ATH_OUI & 0xff), ((ATH_OUI >> 8) & 0xff),
		((ATH_OUI >> 16) & 0xff), ATH_OUI_TYPE_XR,
		ATH_OUI_SUBTYPE_XR, ATH_OUI_VER_XR
	};
	struct ieee80211_xr_param *ie = (struct ieee80211_xr_param *)frm;

	*frm++ = IEEE80211_ELEMID_VENDOR;
	*frm++ = 0;		/* Length filled in below */
	memcpy(frm, oui, sizeof(oui));	/* Atheros OUI, type, subtype, and version for adv capabilities */
	frm += sizeof(oui);
	*frm++ = 0;		/* XR info */

	/* copy the BSSIDs */
	if (vap->iv_flags & IEEE80211_F_XR) {
		IEEE80211_ADDR_COPY(frm, vap->iv_xrvap->iv_bssid);	/* Base BSSID */
		frm += IEEE80211_ADDR_LEN;
		IEEE80211_ADDR_COPY(frm, vap->iv_bssid);	/* XR BSSID */
		frm += IEEE80211_ADDR_LEN;
		*(__le16 *)frm = htole16(vap->iv_bss->ni_intval);	/* XR beacon interval */
		frm += 2;
		*frm++ = vap->iv_xrvap->iv_ath_cap;	/* Base mode capability */
		*frm++ = vap->iv_ath_cap;	/* XR mode capability */
	} else {
		IEEE80211_ADDR_COPY(frm, vap->iv_bssid);
		frm += IEEE80211_ADDR_LEN;
		IEEE80211_ADDR_COPY(frm, vap->iv_xrvap->iv_bssid);
		frm += IEEE80211_ADDR_LEN;
		*(__le16 *)frm = htole16(vap->iv_bss->ni_intval);
		frm += 2;
		*frm++ = vap->iv_ath_cap;
		*frm++ = vap->iv_xrvap->iv_ath_cap;
	}
	ie->param_len = frm - &ie->param_oui[0];
	return frm;
}

#endif

u_int8_t *ieee80211_add_mtik_ie(u_int8_t *frm, struct ieee80211vap *vap, int legacy)
{
	static const u_int8_t mtikoui[3] = { 0x00, 0x0c, 0x42 };
	static const u_int8_t mtikver[4] = { 0x0, 0x66, 0x2, 0x3 };	//{ 0x1f, 0x66, 0x09, 0x10 } - 31, 102, 9, 2 aka 2.9.31
	struct ieee80211_mtik_ie_data *mtikie;

	if (legacy) {
		/* obsolete mtikie, for backward compatibility */
		*frm++ = 0xdf;
		*frm++ = sizeof(struct ieee80211_mtik_ie_data);
	} else {
		*frm++ = IEEE80211_ELEMID_VENDOR;
		*frm++ = sizeof(struct ieee80211_mtik_ie_data) + 6;	/* mtik ie + oui + {version, type, subtype} */
		memcpy(frm, mtikoui, 3);	/* mtik OUI, type, subtype, and version for adv capabilities */
		frm += 3;
		*frm++ = 0;
		*frm++ = 0;
		*frm++ = 0;
	}

	mtikie = (struct ieee80211_mtik_ie_data *)frm;
	memset(mtikie, 0, sizeof(struct ieee80211_mtik_ie_data));
	mtikie->data1[0] = 0x01;
	mtikie->data1[1] = 0x1e;
	mtikie->flags = 0;
	if (vap->iv_flags_ext & IEEE80211_FEXT_WDS)
		mtikie->flags |= (1 << 2);	//4
	memcpy(mtikie->version, mtikver, 4);
	mtikie->pad1 = 0xff;
	mtikie->namelen = 0x0f;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
	strncpy(mtikie->radioname, system_utsname.nodename, mtikie->namelen - 1);
#else
	strncpy(mtikie->radioname, init_utsname()->nodename, mtikie->namelen - 1);
#endif
	frm += sizeof(struct ieee80211_mtik_ie_data);

	return frm;
}

/*
 * Send a probe request frame with the specified ssid
 * and any optional information element data.
 */
int
ieee80211_send_probereq(struct ieee80211_node *ni,
			const u_int8_t sa[IEEE80211_ADDR_LEN], const u_int8_t da[IEEE80211_ADDR_LEN], const u_int8_t bssid[IEEE80211_ADDR_LEN], const u_int8_t *ssid, size_t ssidlen, const void *optie, size_t optielen)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	enum ieee80211_phymode mode;
	struct ieee80211_frame *wh;
	struct sk_buff *skb;
	u_int8_t *frm;

	/*
	 * prreq frame format
	 *      [tlv] ssid
	 *      [tlv] supported rates
	 *      [tlv] extended supported rates
	 *      [tlv] user-specified IEs
	 */
	skb = ieee80211_getmgtframe(&frm, 2 + IEEE80211_NWID_LEN +
				    2 + IEEE80211_RATE_SIZE + 2 + (IEEE80211_RATE_MAXSIZE - IEEE80211_RATE_SIZE) + (optie != NULL ? optielen : 0) + vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_REQ].length);
	if (skb == NULL) {
		vap->iv_stats.is_tx_nobuf++;
		return -ENOMEM;
	}

	frm = ieee80211_add_ssid(frm, ssid, ssidlen);
	mode = ieee80211_chan2mode(ic->ic_curchan);
	frm = ieee80211_add_rates(frm, &ic->ic_sup_rates[ieee80211_chan2ratemode(ic->ic_curchan, mode)]);
	frm = ieee80211_add_xrates(frm, &ic->ic_sup_rates[ieee80211_chan2ratemode(ic->ic_curchan, mode)]);

	if (optie != NULL) {
		memcpy(frm, optie, optielen);
		frm += optielen;
	}

	if (vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_REQ].ie) {
		memcpy(frm, vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_REQ].ie, vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_REQ].length);
		frm += vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_REQ].length;
	}

	skb_trim(skb, frm - skb->data);

	SKB_CB(skb)->ni = ieee80211_ref_node(ni);

	wh = ieee80211_send_setup(vap, ni, skb, IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_PROBE_REQ, sa, da, bssid);
	/* XXX power management? */

	IEEE80211_NODE_STAT(ni, tx_probereq);
	IEEE80211_NODE_STAT(ni, tx_mgmt);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_DUMPPKTS, "[" MAC_FMT "] send probe req on channel %u\n", MAC_ADDR(wh->i_addr1), ieee80211_chan2ieee(ic, ic->ic_curchan));

	(void)ic->ic_mgtstart(ic, skb);
	return 0;
}

/*
 * Send a management frame.  The node is for the destination (or ic_bss
 * when in station mode).  Nodes other than ic_bss have their reference
 * count bumped to reflect our use for an indeterminate time.
 */
int ieee80211_send_mgmt(struct ieee80211_node *ni, int type, int arg)
{
#define	senderr(_x, _v)	do { vap->iv_stats._v++; ret = _x; goto bad; } while (0)
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct sk_buff *skb;
	u_int8_t *frm;
	int frm_len;
	u_int16_t capinfo;
	ieee80211_keyix_t def_keyindex;
	int has_challenge, is_shared_key, ret, timer, status;

	KASSERT(ni != NULL, ("null node"));

	timer = 0;
	switch (type) {
	case IEEE80211_FC0_SUBTYPE_PROBE_RESP:
		/*
		 * probe response frame format
		 *      [8] time stamp
		 *      [2] beacon interval
		 *      [2] capability information
		 *      [tlv] ssid
		 *      [tlv] supported rates
		 *      [7] FH/DS parameter set
		 *      [tlv] IBSS parameter set
		 *      [tlv] country code 
		 *      [3] power constraint
		 *      [3] extended rate phy (ERP)
		 *      [tlv] extended supported rates
		 *      [tlv] WME parameters
		 *      [tlv] WPA/RSN parameters
		 *      [tlv] Atheros Advanced Capabilities
		 *      [tlv] AtherosXR parameters
		 */
		frm_len = 8 + sizeof(u_int16_t)
		    + sizeof(u_int16_t)
		    + 2 + IEEE80211_NWID_LEN + 2 + IEEE80211_RATE_SIZE + 7	/* max(7,3) */
		    /* XXX allocate max size */
		    + 2 + ic->ic_country_ie.country_len + 3 + 3 + 2 + (IEEE80211_RATE_MAXSIZE - IEEE80211_RATE_SIZE)
		    + sizeof(struct ieee80211_wme_param)
		    /* XXX !WPA1+WPA2 fits w/o a cluster */
		    + (vap->iv_flags & IEEE80211_F_WPA ? 2 * sizeof(struct ieee80211_ie_wpa) : 0)
		    + sizeof(struct ieee80211_ie_athAdvCap)
		    + sizeof(struct ieee80211_mtik_ie)
		    + vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_RESP].length;
#ifdef ATH_SUPERG_XR
		if (vap->iv_ath_cap & IEEE80211_ATHC_XR)
			frm_len += sizeof(struct ieee80211_xr_param);
#endif
		skb = ieee80211_getmgtframe(&frm, frm_len);
		if (skb == NULL)
			senderr(ENOMEM, is_tx_nobuf);

		/* timestamp should be filled later */
		memset(frm, 0, 8);
		frm += 8;

		/* beacon interval */
		*(__le16 *)frm = htole16(vap->iv_bss->ni_intval);
		frm += 2;

		/* cap. info */
		if (vap->iv_opmode == IEEE80211_M_IBSS)
			capinfo = IEEE80211_CAPINFO_IBSS;
		else
			capinfo = IEEE80211_CAPINFO_ESS;
		if (vap->iv_flags & IEEE80211_F_PRIVACY)
			capinfo |= IEEE80211_CAPINFO_PRIVACY;
		if ((ic->ic_flags & IEEE80211_F_SHPREAMBLE) && IEEE80211_IS_CHAN_2GHZ(ic->ic_curchan))
			capinfo |= IEEE80211_CAPINFO_SHORT_PREAMBLE;
		if (ic->ic_flags & IEEE80211_F_SHSLOT)
			capinfo |= IEEE80211_CAPINFO_SHORT_SLOTTIME;
		*(__le16 *)frm = htole16(capinfo);
		frm += 2;

		/* ssid */
		frm = ieee80211_add_ssid(frm, vap->iv_bss->ni_essid, vap->iv_bss->ni_esslen);

		/* supported rates */
		frm = ieee80211_add_rates(frm, &ni->ni_rates);

		/* XXX: FH/DS parameter set, correct ? */
		if (ic->ic_phytype == IEEE80211_T_FH) {
			*frm++ = IEEE80211_ELEMID_FHPARMS;
			*frm++ = 5;
			*frm++ = ni->ni_fhdwell & 0x00ff;
			*frm++ = (ni->ni_fhdwell >> 8) & 0x00ff;
			*frm++ = IEEE80211_FH_CHANSET((ieee80211_chan2ieee(ic, ic->ic_curchan) + vap->iv_channelshift) & 0xff);
			*frm++ = IEEE80211_FH_CHANPAT((ieee80211_chan2ieee(ic, ic->ic_curchan) + vap->iv_channelshift) & 0xff);
			*frm++ = ni->ni_fhindex;
		} else {
			*frm++ = IEEE80211_ELEMID_DSPARMS;
			*frm++ = 1;
			*frm++ = (ieee80211_chan2ieee(ic, ic->ic_curchan) + vap->iv_channelshift) & 0xff;
		}

		if (vap->iv_opmode == IEEE80211_M_IBSS) {
			*frm++ = IEEE80211_ELEMID_IBSSPARMS;
			*frm++ = 2;
			*frm++ = 0;
			*frm++ = 0;	/* TODO: ATIM window */
		}

#ifdef COUNTRY_LIST
		/* country code */
		if ((ic->ic_flags & IEEE80211_F_DOTH) || (ic->ic_flags_ext & IEEE80211_FEXT_COUNTRYIE))
			frm = ieee80211_add_country(frm, ic);
#endif

		/* power constraint */
		if (ic->ic_flags & IEEE80211_F_DOTH)
			frm = ieee80211_add_pwrcnstr(frm, ic);

		/* ERP */
		if (IEEE80211_IS_CHAN_ANYG(ic->ic_curchan))
			frm = ieee80211_add_erp(frm, ic);

		/* Ext. Supp. Rates */
		frm = ieee80211_add_xrates(frm, &ni->ni_rates);

		/* WME */
		if (vap->iv_flags & IEEE80211_F_WME)
			frm = ieee80211_add_wme_param(frm, &ic->ic_wme, IEEE80211_VAP_UAPSD_ENABLED(vap));

		/* WPA */
		if (vap->iv_flags & IEEE80211_F_WPA)
			frm = ieee80211_add_wpa(frm, vap);

		/* AthAdvCaps */
		if (vap->iv_bss && vap->iv_bss->ni_ath_flags)
			frm = ieee80211_add_athAdvCap(frm, vap->iv_bss->ni_ath_flags, vap->iv_bss->ni_ath_defkeyindex);
#ifdef ATH_SUPERG_XR
		/* XR params */
		if (vap->iv_xrvap && vap->iv_ath_cap & IEEE80211_ATHC_XR)
			frm = ieee80211_add_xr_param(frm, vap);
#endif
		if (vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_RESP].ie) {
			memcpy(frm, vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_RESP].ie, vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_RESP].length);
			frm += vap->app_ie[IEEE80211_APPIE_FRAME_PROBE_RESP].length;
		}

		if (vap->iv_flags_ext & IEEE80211_FEXT_ADDMTIKIE)
			frm = ieee80211_add_mtik_ie(frm, vap, 0);

		skb_trim(skb, frm - skb->data);
		break;

	case IEEE80211_FC0_SUBTYPE_AUTH:
		status = arg >> 16;
		arg &= 0xffff;
		has_challenge = ((arg == IEEE80211_AUTH_SHARED_CHALLENGE || arg == IEEE80211_AUTH_SHARED_RESPONSE) && ni->ni_challenge != NULL);

		/*
		 * Deduce whether we're doing open authentication or
		 * shared key authentication.  We do the latter if
		 * we're in the middle of a shared key authentication
		 * handshake or if we're initiating an authentication
		 * request and configured to use shared key.
		 */
		is_shared_key = has_challenge || arg >= IEEE80211_AUTH_SHARED_RESPONSE || (arg == IEEE80211_AUTH_SHARED_REQUEST && vap->iv_bss->ni_authmode == IEEE80211_AUTH_SHARED);

		skb = ieee80211_getmgtframe(&frm, 3 * sizeof(u_int16_t)
					    + (has_challenge && status == IEEE80211_STATUS_SUCCESS ? sizeof(u_int16_t) + IEEE80211_CHALLENGE_LEN : 0));
		if (skb == NULL)
			senderr(ENOMEM, is_tx_nobuf);
		SKB_CB(skb)->auth_pkt = 1;

		((__le16 *)frm)[0] = (is_shared_key) ? htole16(IEEE80211_AUTH_ALG_SHARED)
		    : htole16(IEEE80211_AUTH_ALG_OPEN);
		((__le16 *)frm)[1] = htole16(arg);	/* sequence number */
		((__le16 *)frm)[2] = htole16(status);	/* status */

		if (has_challenge && status == IEEE80211_STATUS_SUCCESS) {
			((__le16 *)frm)[3] = htole16((IEEE80211_CHALLENGE_LEN << 8) | IEEE80211_ELEMID_CHALLENGE);
			memcpy(&((__le16 *)frm)[4], ni->ni_challenge, IEEE80211_CHALLENGE_LEN);
			if (arg == IEEE80211_AUTH_SHARED_RESPONSE) {
				IEEE80211_NOTE(vap, IEEE80211_MSG_AUTH, ni, "request encrypt frame (%s)", __func__);
				SKB_CB(skb)->flags |= M_LINK0;	/* WEP-encrypt, please */
			}
		}

		/* XXX not right for shared key */
		if (status == IEEE80211_STATUS_SUCCESS)
			IEEE80211_NODE_STAT(ni, tx_auth);
		else
			IEEE80211_NODE_STAT(ni, tx_auth_fail);

		if (vap->iv_opmode == IEEE80211_M_STA)
			timer = IEEE80211_TRANS_WAIT;
		break;

	case IEEE80211_FC0_SUBTYPE_DEAUTH:
		IEEE80211_NOTE(vap, IEEE80211_MSG_AUTH, ni, "send station deauthenticate (reason %d)", arg);
		skb = ieee80211_getmgtframe(&frm, sizeof(u_int16_t));
		if (skb == NULL)
			senderr(ENOMEM, is_tx_nobuf);
		*(__le16 *)frm = htole16(arg);	/* reason */

		IEEE80211_NODE_STAT(ni, tx_deauth);
		IEEE80211_NODE_STAT_SET(ni, tx_deauth_code, arg);

		ieee80211_node_unauthorize(ni);	/* port closed */
		break;

	case IEEE80211_FC0_SUBTYPE_ASSOC_REQ:
	case IEEE80211_FC0_SUBTYPE_REASSOC_REQ:
		/*
		 * asreq frame format
		 *      [2] capability information
		 *      [2] listen interval
		 *      [6*] current AP address (reassoc only)
		 *      [tlv] ssid
		 *      [tlv] supported rates
		 *      [4] power capability (802.11h)
		 *      [tlv] supported channels element (802.11h)
		 *      [tlv] extended supported rates
		 *      [tlv] WME [if enabled and AP capable]
		 *      [tlv] Atheros advanced capabilities
		 *      [tlv] user-specified IEs
		 */
		skb = ieee80211_getmgtframe(&frm, sizeof(u_int16_t) + sizeof(u_int16_t) + IEEE80211_ADDR_LEN + 2 + IEEE80211_NWID_LEN + 2 + IEEE80211_RATE_SIZE +
//                      4 + (2 + ic->ic_sc_ie.sc_len) +
					    2 + (IEEE80211_RATE_MAXSIZE - IEEE80211_RATE_SIZE) +
					    sizeof(struct ieee80211_ie_wme) +
					    sizeof(struct ieee80211_ie_athAdvCap) +
					    ((vap->iv_flags_ext & IEEE80211_FEXT_ADDMTIKIE) ?
					     sizeof(struct ieee80211_mtik_ie) : 0) + (vap->iv_opt_ie != NULL ? vap->iv_opt_ie_len : 0) + vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_REQ].length);
		if (skb == NULL)
			senderr(ENOMEM, is_tx_nobuf);
		SKB_CB(skb)->auth_pkt = 1;

		capinfo = 0;
		if (vap->iv_opmode == IEEE80211_M_IBSS)
			capinfo |= IEEE80211_CAPINFO_IBSS;
		else		/* IEEE80211_M_STA */
			capinfo |= IEEE80211_CAPINFO_ESS;
		if (vap->iv_flags & IEEE80211_F_PRIVACY)
			capinfo |= IEEE80211_CAPINFO_PRIVACY;
		/*
		 * NB: Some 11a APs reject the request when
		 *     short premable is set.
		 */
		/* Capability information */
		if ((ic->ic_flags & IEEE80211_F_SHPREAMBLE) && IEEE80211_IS_CHAN_2GHZ(ic->ic_curchan))
			capinfo |= IEEE80211_CAPINFO_SHORT_PREAMBLE;
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME) && (ic->ic_caps & IEEE80211_C_SHSLOT))
			capinfo |= IEEE80211_CAPINFO_SHORT_SLOTTIME;
		*(__le16 *)frm = htole16(capinfo);
		frm += 2;

		/* listen interval */
		*(__le16 *)frm = htole16(ic->ic_lintval);
		frm += 2;

		/* Current AP address */
		if (type == IEEE80211_FC0_SUBTYPE_REASSOC_REQ) {
			IEEE80211_ADDR_COPY(frm, vap->iv_bssid);
			frm += IEEE80211_ADDR_LEN;
		}
		/* ssid */
		frm = ieee80211_add_ssid(frm, ni->ni_essid, ni->ni_esslen);
		/* supported rates */
		frm = ieee80211_add_rates(frm, &ni->ni_rates);
		/* power capability/supported channels */
#if 0
		if (ic->ic_flags & IEEE80211_F_DOTH) {
			frm = ieee80211_add_pwrcap(frm, ic);
			frm = ieee80211_add_suppchan(frm, ic);
		}
#endif
		/* ext. supp. rates */
		frm = ieee80211_add_xrates(frm, &ni->ni_rates);

		/* wme */
		if ((vap->iv_flags & IEEE80211_F_WME) && ni->ni_wme_ie != NULL)
			frm = ieee80211_add_wme(frm, ni);
		/* ath adv. cap */
		if (ni->ni_ath_flags & vap->iv_ath_cap) {
			IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "Adding ath adv cap ie: ni_ath_flags = %02x, " "iv_ath_cap = %02x", ni->ni_ath_flags, vap->iv_ath_cap);

			/* Setup default key index for static wep case */
			def_keyindex = IEEE80211_INVAL_DEFKEY;
			if (((vap->iv_flags & IEEE80211_F_WPA) == 0) && (ni->ni_authmode != IEEE80211_AUTH_8021X) && (vap->iv_def_txkey != IEEE80211_KEYIX_NONE))
				def_keyindex = vap->iv_def_txkey;

			frm = ieee80211_add_athAdvCap(frm, ni->ni_ath_flags & vap->iv_ath_cap, def_keyindex);
		}

		/* User-spec */
		if (vap->iv_opt_ie != NULL) {
			memcpy(frm, vap->iv_opt_ie, vap->iv_opt_ie_len);
			frm += vap->iv_opt_ie_len;
		}

		if (vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_REQ].ie) {
			memcpy(frm, vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_REQ].ie, vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_REQ].length);
			frm += vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_REQ].length;
		}
		if ((vap->iv_flags_ext & IEEE80211_FEXT_ADDMTIKIE) && ni->ni_mtik_ie)
			frm = ieee80211_add_mtik_ie(frm, vap, 0);

		skb_trim(skb, frm - skb->data);

		timer = IEEE80211_TRANS_WAIT;
		break;

	case IEEE80211_FC0_SUBTYPE_ASSOC_RESP:
	case IEEE80211_FC0_SUBTYPE_REASSOC_RESP:
		/*
		 * asreq frame format
		 *      [2] capability information
		 *      [2] status
		 *      [2] association ID
		 *      [tlv] supported rates
		 *      [tlv] extended supported rates
		 *      [tlv] WME (if enabled and STA enabled)
		 *      [tlv] Atheros Advanced Capabilities 
		 */
		skb = ieee80211_getmgtframe(&frm,
					    3 * sizeof(u_int16_t) +
					    2 + IEEE80211_RATE_SIZE +
					    2 + (IEEE80211_RATE_MAXSIZE - IEEE80211_RATE_SIZE) +
					    sizeof(struct ieee80211_wme_param) +
					    (vap->iv_ath_cap ? sizeof(struct ieee80211_ie_athAdvCap) : 0) +
					    ((vap->iv_flags_ext & IEEE80211_FEXT_ADDMTIKIE) ? sizeof(struct ieee80211_mtik_ie) : 0) + vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_RESP].length);
		if (skb == NULL)
			senderr(ENOMEM, is_tx_nobuf);

		/* Capability Information */
		capinfo = IEEE80211_CAPINFO_ESS;
		if (vap->iv_flags & IEEE80211_F_PRIVACY)
			capinfo |= IEEE80211_CAPINFO_PRIVACY;
		if ((ic->ic_flags & IEEE80211_F_SHPREAMBLE) && IEEE80211_IS_CHAN_2GHZ(ic->ic_curchan))
			capinfo |= IEEE80211_CAPINFO_SHORT_PREAMBLE;
		if (ic->ic_flags & IEEE80211_F_SHSLOT)
			capinfo |= IEEE80211_CAPINFO_SHORT_SLOTTIME;
		*(__le16 *)frm = htole16(capinfo);
		frm += 2;

		/* status */
		*(__le16 *)frm = htole16(arg);
		frm += 2;

		/* Assoc ID */
		if (arg == IEEE80211_STATUS_SUCCESS) {
			*(__le16 *)frm = htole16(ni->ni_associd);
			IEEE80211_NODE_STAT(ni, tx_assoc);
		} else
			IEEE80211_NODE_STAT(ni, tx_assoc_fail);
		frm += 2;

		/* supported rates */
		frm = ieee80211_add_rates(frm, &ni->ni_rates);

		/* ext. suppo. rates */
		frm = ieee80211_add_xrates(frm, &ni->ni_rates);

		/* wme */
		if ((vap->iv_flags & IEEE80211_F_WME) && ni->ni_wme_ie != NULL)
			frm = ieee80211_add_wme_param(frm, &ic->ic_wme, IEEE80211_VAP_UAPSD_ENABLED(vap));

		/* athAdvCap */
		if (vap->iv_ath_cap)
			frm = ieee80211_add_athAdvCap(frm, vap->iv_ath_cap & ni->ni_ath_flags, ni->ni_ath_defkeyindex);

		if (vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_RESP].ie) {
			memcpy(frm, vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_RESP].ie, vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_RESP].length);
			frm += vap->app_ie[IEEE80211_APPIE_FRAME_ASSOC_RESP].length;
		}

		if ((vap->iv_flags_ext & IEEE80211_FEXT_ADDMTIKIE) && ni->ni_mtik_ie)
			frm = ieee80211_add_mtik_ie(frm, vap, 0);

		skb_trim(skb, frm - skb->data);
		break;

	case IEEE80211_FC0_SUBTYPE_DISASSOC:
		IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "send station disassociate (reason %d)", arg);
		skb = ieee80211_getmgtframe(&frm, sizeof(u_int16_t));
		if (skb == NULL)
			senderr(ENOMEM, is_tx_nobuf);
		*(__le16 *)frm = htole16(arg);	/* reason */

		IEEE80211_NODE_STAT(ni, tx_disassoc);
		IEEE80211_NODE_STAT_SET(ni, tx_disassoc_code, arg);
		break;

	default:
		IEEE80211_NOTE(vap, IEEE80211_MSG_ANY, ni, "invalid mgmt frame type %u", type);
		senderr(EINVAL, is_tx_unknownmgt);
		/* NOTREACHED */
	}

	ieee80211_mgmt_output(ieee80211_ref_node(ni), skb, type);
	if (timer)
		mod_timer(&vap->iv_mgtsend, jiffies + msecs_to_jiffies(timer));
	return 0;
bad:
	return ret;
#undef senderr
}

/*
 * Send PS-POLL from to bss. Should only be called when as STA.
 */
void ieee80211_send_pspoll(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct sk_buff *skb;
	struct ieee80211_ctlframe_addr2 *wh;

	skb = ieee80211_dev_alloc_skb(sizeof(struct ieee80211_ctlframe_addr2));
	if (skb == NULL)
		return;

	SKB_CB(skb)->ni = ieee80211_ref_node(ni);
	skb->priority = WME_AC_VO;

	wh = (struct ieee80211_ctlframe_addr2 *)skb_put(skb, sizeof(struct ieee80211_ctlframe_addr2));

	wh->i_aidordur = htole16(0xc000 | IEEE80211_NODE_AID(ni));
	IEEE80211_ADDR_COPY(wh->i_addr1, vap->iv_bssid);
	IEEE80211_ADDR_COPY(wh->i_addr2, vap->iv_myaddr);
	wh->i_fc[0] = 0;
	wh->i_fc[1] = 0;
	wh->i_fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_PS_POLL;
	if (IEEE80211_VAP_IS_SLEEPING(ni->ni_vap))
		wh->i_fc[1] |= IEEE80211_FC1_PWR_MGT;

	(void)ic->ic_mgtstart(ic, skb);	/* cheat */
}

#ifdef ATH_SUPERG_XR
/*
 * constructs and returns a contention free frames.
 * currently used for Group poll in XR mode.
 */
struct sk_buff *ieee80211_getcfframe(struct ieee80211vap *vap, int type)
{
	u_int8_t *frm;
	struct sk_buff *skb;
	struct ieee80211_frame *wh;
	struct ieee80211com *ic = vap->iv_ic;

	skb = ieee80211_getmgtframe(&frm, 0);
	if (skb == NULL)
		return NULL;
	wh = (struct ieee80211_frame *)
	    skb_push(skb, sizeof(struct ieee80211_frame));
	if (type == IEEE80211_FC0_SUBTYPE_CFPOLL) {
		wh->i_fc[1] = IEEE80211_FC1_DIR_FROMDS;
		wh->i_fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_DATA | type;
		wh->i_dur = htole16(0x8000);
	} else if (type == IEEE80211_FC0_SUBTYPE_CF_END) {
		wh->i_fc[1] = IEEE80211_FC1_DIR_NODS;
		wh->i_fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL | type;
		wh->i_dur = 0;
	}
	IEEE80211_ADDR_COPY(wh->i_addr1, ic->ic_dev->broadcast);
	IEEE80211_ADDR_COPY(wh->i_addr2, vap->iv_myaddr);
	IEEE80211_ADDR_COPY(wh->i_addr3, vap->iv_bssid);
	return skb;
}

EXPORT_SYMBOL(ieee80211_getcfframe);
#endif
