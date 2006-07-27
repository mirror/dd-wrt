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
 * $Id: ieee80211_input.c 1526 2006-04-23 23:54:40Z dyqith $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 input handling.
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/if_vlan.h>
#include <net/iw_handler.h> /* wireless_send_event(..) */
#include <linux/wireless.h> /* SIOCGIWTHRSPY */
#include <linux/if_arp.h> /* ARPHRD_ETHER */

#include "if_llc.h"
#include "if_ethersubr.h"
#include "if_media.h"
#include "if_athproto.h"

#include <net80211/ieee80211_var.h>

#ifdef IEEE80211_DEBUG
/*
 * Decide if a received management frame should be
 * printed when debugging is enabled.  This filters some
 * of the less interesting frames that come frequently
 * (e.g. beacons).
 */
static __inline int
doprint(struct ieee80211vap *vap, int subtype)
{
	switch (subtype) {
	case IEEE80211_FC0_SUBTYPE_BEACON:
		return (vap->iv_ic->ic_flags & IEEE80211_F_SCAN);
	case IEEE80211_FC0_SUBTYPE_PROBE_REQ:
		return (vap->iv_opmode == IEEE80211_M_IBSS);
	}
	return 1;
}

/*
 * Emit a debug message about discarding a frame or information
 * element.  One format is for extracting the mac address from
 * the frame header; the other is for when a header is not
 * available or otherwise appropriate.
 */
#define	IEEE80211_DISCARD(_vap, _m, _wh, _type, _fmt, ...) do {		\
	if ((_vap)->iv_debug & (_m))					\
		ieee80211_discard_frame(_vap, _wh, _type, _fmt, __VA_ARGS__);\
} while (0)
#define	IEEE80211_DISCARD_IE(_vap, _m, _wh, _type, _fmt, ...) do {	\
	if ((_vap)->iv_debug & (_m))					\
		ieee80211_discard_ie(_vap, _wh, _type, _fmt, __VA_ARGS__);\
} while (0)
#define	IEEE80211_DISCARD_MAC(_vap, _m, _mac, _type, _fmt, ...) do {	\
	if ((_vap)->iv_debug & (_m))					\
		ieee80211_discard_mac(_vap, _mac, _type, _fmt, __VA_ARGS__);\
} while (0)

static const u_int8_t *ieee80211_getbssid(struct ieee80211vap *,
	const struct ieee80211_frame *);
static void ieee80211_discard_frame(struct ieee80211vap *,
	const struct ieee80211_frame *, const char *, const char *, ...);
static void ieee80211_discard_ie(struct ieee80211vap *,
	const struct ieee80211_frame *, const char *, const char *, ...);
static void ieee80211_discard_mac(struct ieee80211vap *,
	const u_int8_t mac[IEEE80211_ADDR_LEN], const char *,
	const char *, ...);
#else
#define	IEEE80211_DISCARD(_vap, _m, _wh, _type, _fmt, ...)
#define	IEEE80211_DISCARD_IE(_vap, _m, _wh, _type, _fmt, ...)
#define	IEEE80211_DISCARD_MAC(_vap, _m, _mac, _type, _fmt, ...)
#endif /* IEEE80211_DEBUG */

static struct sk_buff *ieee80211_defrag(struct ieee80211_node *,
	struct sk_buff *, int);
static void ieee80211_deliver_data(struct ieee80211_node *, struct sk_buff *);
static struct sk_buff *ieee80211_decap(struct ieee80211vap *,
	struct sk_buff *, int);
static void ieee80211_send_error(struct ieee80211_node *, const u_int8_t *,
	int, int);
static void ieee80211_recv_pspoll(struct ieee80211_node *, struct sk_buff *);
static int accept_data_frame(struct ieee80211vap *, struct ieee80211_node *,
	struct ieee80211_key *, struct sk_buff *, struct ether_header *);


#ifdef ATH_SUPERG_FF
static void athff_decap(struct sk_buff *);
#endif
#ifdef USE_HEADERLEN_RESV
static unsigned short ath_eth_type_trans(struct sk_buff *, struct net_device *);
#endif

/* Enhanced iwspy support */
#ifdef CONFIG_NET_WIRELESS
#if WIRELESS_EXT >= 16

#ifndef IW_QUAL_QUAL_UPDATED
#define IW_QUAL_QUAL_UPDATED	0x01
#define IW_QUAL_LEVEL_UPDATED	0x02
#define IW_QUAL_NOISE_UPDATED	0x04
#endif /* IW_QUAL_QUAL_UPDATED */

/**
 * This function is a clone of set_quality(..) in ieee80211_wireless.c
 */
static void
set_quality(struct iw_quality *iq, u_int rssi)
{
	iq->qual = rssi;
	/* NB: max is 94 because noise is hardcoded to 161 */
	if (iq->qual > 94)
		iq->qual = 94;

	iq->noise = 161;		/* -95dBm */
	iq->level = iq->noise + iq->qual;
	iq->updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_UPDATED |
		IW_QUAL_NOISE_UPDATED;
}

/**
 * Given a node and the rssi value of a just received frame from the node, this
 * function checks if to raise an iwspy event because we iwspy the node and rssi
 * exceeds threshold (if active).
 * 
 * @param vap: vap
 * @param ni: sender node
 * @param rssi: rssi value of received frame
 */
static void
iwspy_event(struct ieee80211vap *vap, struct ieee80211_node *ni, u_int rssi)
{
	if (vap->iv_spy.thr_low && vap->iv_spy.num && ni && (rssi <
		vap->iv_spy.thr_low || rssi > vap->iv_spy.thr_high)) {
		int i;
		for (i = 0; i < vap->iv_spy.num; i++) {
			if (IEEE80211_ADDR_EQ(ni->ni_macaddr,
				&(vap->iv_spy.mac[i * IEEE80211_ADDR_LEN]))) {
					
				union iwreq_data wrq;
				struct iw_thrspy thr;
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_DEBUG,
					"%s: we spy %s, threshold is active "
					"and rssi exceeds it -> raise an iwspy"
					" event\n", __func__, ether_sprintf(
					 ni->ni_macaddr));
				memset(&wrq, 0, sizeof(wrq));
				wrq.data.length = 1;
				memset(&thr, 0, sizeof(struct iw_thrspy));
				memcpy(thr.addr.sa_data, ni->ni_macaddr,
					IEEE80211_ADDR_LEN);
				thr.addr.sa_family = ARPHRD_ETHER;
				set_quality(&thr.qual, rssi);
				set_quality(&thr.low, vap->iv_spy.thr_low);
				set_quality(&thr.high, vap->iv_spy.thr_high);
				wireless_send_event(vap->iv_dev,
					SIOCGIWTHRSPY, &wrq, (char*) &thr);
				break;
			}
		}
	}
}

#else
#define iwspy_event(_vap, _ni, _rssi)
#endif /* WIRELESS_EXT >= 16 */
#else
#define iwspy_event(_vap, _ni, _rssi)
#endif /* CONFIG_NET_WIRELESS */

/*
 * Process a received frame.  The node associated with the sender
 * should be supplied.  If nothing was found in the node table then
 * the caller is assumed to supply a reference to ic_bss instead.
 * The RSSI and a timestamp are also supplied.  The RSSI data is used
 * during AP scanning to select a AP to associate with; it can have
 * any units so long as values have consistent units and higher values
 * mean ``better signal''.  The receive timestamp is currently not used
 * by the 802.11 layer.
 *
 * Context: softIRQ (tasklet)
 */
int
ieee80211_input(struct ieee80211_node *ni,
	struct sk_buff *skb, int rssi, u_int32_t rstamp)
{
#define	HAS_SEQ(type)	((type & 0x4) == 0)
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni_wds = NULL;
	struct net_device *dev = vap->iv_dev;
	struct ieee80211_frame *wh;
	struct ieee80211_key *key;
	struct ether_header *eh;
#ifdef ATH_SUPERG_FF
	struct llc *llc;
#endif
	int hdrspace;
	u_int8_t dir, type, subtype;
	u_int8_t *bssid;
	u_int16_t rxseq;

	KASSERT(ni != NULL, ("null node"));
	ni->ni_inact = ni->ni_inact_reload;

	KASSERT(skb->len >= sizeof(struct ieee80211_frame_min),
		("frame length too short: %u", skb->len));

	/* XXX adjust device in sk_buff? */

	type = -1;			/* undefined */
	/*
	 * In monitor mode, send everything directly to bpf.
	 * Also do not process frames w/o i_addr2 any further.
	 * XXX may want to include the CRC
	 */
	if (vap->iv_opmode == IEEE80211_M_MONITOR)
		goto out;

	if (skb->len < sizeof(struct ieee80211_frame_min)) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY,
			ni->ni_macaddr, NULL,
			"too short (1): len %u", skb->len);
		vap->iv_stats.is_rx_tooshort++;
		goto out;
	}
	/*
	 * Bit of a cheat here, we use a pointer for a 3-address
	 * frame format but don't reference fields past outside
	 * ieee80211_frame_min w/o first validating the data is
	 * present.
	 */
	wh = (struct ieee80211_frame *)skb->data;

	if ((wh->i_fc[0] & IEEE80211_FC0_VERSION_MASK) !=
	    IEEE80211_FC0_VERSION_0) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY,
			ni->ni_macaddr, NULL, "wrong version %x", wh->i_fc[0]);
		vap->iv_stats.is_rx_badversion++;
		goto err;
	}

	dir = wh->i_fc[1] & IEEE80211_FC1_DIR_MASK;
	type = wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK;
	subtype = wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK;
	if ((ic->ic_flags & IEEE80211_F_SCAN) == 0) {
		switch (vap->iv_opmode) {
		case IEEE80211_M_STA:
			bssid = wh->i_addr2;
			if (!IEEE80211_ADDR_EQ(bssid, ni->ni_bssid)) {
				/* not interested in */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
					bssid, NULL, "%s", "not to bss");
				vap->iv_stats.is_rx_wrongbss++;
				goto out;
			}

			/* WAR for excessive beacon miss on SoC.
			 * Reset bmiss counter when we receive a
			 * non probe request frame */
			if (vap->iv_state == IEEE80211_S_RUN &&
					(!(type == IEEE80211_FC0_TYPE_MGT &&
					subtype == IEEE80211_FC0_SUBTYPE_PROBE_REQ)))
				vap->iv_bmiss_count = 0;

			break;
		case IEEE80211_M_IBSS:
		case IEEE80211_M_AHDEMO:
			if (dir != IEEE80211_FC1_DIR_NODS)
				bssid = wh->i_addr1;
			else if (type == IEEE80211_FC0_TYPE_CTL)
				bssid = wh->i_addr1;
			else {
				if (skb->len < sizeof(struct ieee80211_frame)) {
					IEEE80211_DISCARD_MAC(vap,
						IEEE80211_MSG_ANY, ni->ni_macaddr,
						NULL, "too short (2): len %u",
						skb->len);
					vap->iv_stats.is_rx_tooshort++;
					goto out;
				}
				bssid = wh->i_addr3;
			}
			if (type == IEEE80211_FC0_TYPE_DATA &&
			    ni == vap->iv_bss) {
				/*
				 * Try to find sender in local node table.
				 */
				ni = ieee80211_find_node(ni->ni_table, wh->i_addr2);
				if (ni == NULL) {
					/*
					 * Fake up a node for this newly discovered
					 * member of the IBSS.  This should probably
					 * done after an ACL check.
					 */
					ni = ieee80211_fakeup_adhoc_node(vap,
							wh->i_addr2);
					if (ni == NULL) {
						/* NB: stat kept for alloc failure */
						goto err;
					}
				}
			}
			iwspy_event(vap, ni, rssi);
			iwspy_event(vap, ni, rssi);
			break;
		case IEEE80211_M_HOSTAP:
			if (dir != IEEE80211_FC1_DIR_NODS)
				bssid = wh->i_addr1;
			else if (type == IEEE80211_FC0_TYPE_CTL)
				bssid = wh->i_addr1;
			else {
				if (skb->len < sizeof(struct ieee80211_frame)) {
					IEEE80211_DISCARD_MAC(vap,
						IEEE80211_MSG_ANY, ni->ni_macaddr,
						NULL, "too short (2): len %u",
						skb->len);
					vap->iv_stats.is_rx_tooshort++;
					goto out;
				}
				bssid = wh->i_addr3;
			}
			/*
			 * Validate the bssid.
			 */
#ifdef ATH_SUPERG_XR
			if (!IEEE80211_ADDR_EQ(bssid, vap->iv_bss->ni_bssid) &&
			    !IEEE80211_ADDR_EQ(bssid, dev->broadcast)) {
				/*
				 * allow MGT frames to vap->iv_xrvap.
				 * this will allow roaming between  XR and normal vaps
				 * without station dis associating from previous vap.
				 */
				if (!(vap->iv_xrvap && 
				    IEEE80211_ADDR_EQ(bssid, vap->iv_xrvap->iv_bss->ni_bssid) &&
				    type == IEEE80211_FC0_TYPE_MGT && 
				    ni != vap->iv_bss)) {
					/* not interested in */
					IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
						bssid, NULL, "%s", "not to bss or xrbss");
					vap->iv_stats.is_rx_wrongbss++;
					goto out;
				}
			}
#else
			if (!IEEE80211_ADDR_EQ(bssid, vap->iv_bss->ni_bssid) &&
			    !IEEE80211_ADDR_EQ(bssid, dev->broadcast)) {
				/* not interested in */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
					bssid, NULL, "%s", "not to bss");
				vap->iv_stats.is_rx_wrongbss++;
				goto out;
			}

#endif
			break;
		case IEEE80211_M_WDS:
			if (skb->len < sizeof(struct ieee80211_frame_addr4)) {
				IEEE80211_DISCARD_MAC(vap,
					IEEE80211_MSG_ANY, ni->ni_macaddr,
					NULL, "too short (3): len %u",
					skb->len);
				vap->iv_stats.is_rx_tooshort++;
				goto out;
			}
			bssid = wh->i_addr1;
			if (!IEEE80211_ADDR_EQ(bssid, vap->iv_bss->ni_bssid) &&
			    !IEEE80211_ADDR_EQ(bssid, dev->broadcast)) {
				/* not interested in */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
					bssid, NULL, "%s", "not to bss");
				vap->iv_stats.is_rx_wrongbss++;
				goto out;
			}
			if (!IEEE80211_ADDR_EQ(wh->i_addr2, vap->wds_mac)) {
				/* not interested in */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
					wh->i_addr2, NULL, "%s", "not from DS");
				vap->iv_stats.is_rx_wrongbss++;
				goto out;	
			}
			break;
		default:
			/* XXX catch bad values */
			goto out;
		}
		ni->ni_rssi = rssi;
		ni->ni_rstamp = rstamp;
		ni->ni_last_rx = jiffies;
		if (HAS_SEQ(type)) {
			u_int8_t tid;
			if (IEEE80211_QOS_HAS_SEQ(wh)) {
				tid = ((struct ieee80211_qosframe *)wh)->
					i_qos[0] & IEEE80211_QOS_TID;
				if (TID_TO_WME_AC(tid) >= WME_AC_VI)
					ic->ic_wme.wme_hipri_traffic++;
				tid++;
			} else
				tid = 0;
			rxseq = le16toh(*(u_int16_t *)wh->i_seq);
			if ((wh->i_fc[1] & IEEE80211_FC1_RETRY) &&
			    IEEE80211_SEQ_LEQ(rxseq, ni->ni_rxseqs[tid])) {
				/* duplicate, discard */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
					bssid, "duplicate",
					"seqno <%u,%u> fragno <%u,%u> tid %u",
					rxseq >> IEEE80211_SEQ_SEQ_SHIFT,
					ni->ni_rxseqs[tid] >>
						IEEE80211_SEQ_SEQ_SHIFT,
					rxseq & IEEE80211_SEQ_FRAG_MASK,
					ni->ni_rxseqs[tid] &
						IEEE80211_SEQ_FRAG_MASK,
					tid);
				vap->iv_stats.is_rx_dup++;
				IEEE80211_NODE_STAT(ni, rx_dup);
				goto out;
			}
			ni->ni_rxseqs[tid] = rxseq;
		}
	}

	switch (type) {
	case IEEE80211_FC0_TYPE_DATA:
		hdrspace = ieee80211_hdrspace(ic, wh);
		if (skb->len < hdrspace) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
				wh, "data", "too short: len %u, expecting %u",
			 	skb->len, hdrspace);
			vap->iv_stats.is_rx_tooshort++;
			goto out;		/* XXX */
		}
		switch (vap->iv_opmode) {
		case IEEE80211_M_STA:
			if ((dir != IEEE80211_FC1_DIR_FROMDS) &&
			    (!((vap->iv_flags_ext & IEEE80211_FEXT_WDS) &&
			    (dir == IEEE80211_FC1_DIR_DSTODS)))) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
					wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}
			if ((dev->flags & IFF_MULTICAST) &&
			    IEEE80211_IS_MULTICAST(wh->i_addr1)) {
				if (IEEE80211_ADDR_EQ(wh->i_addr3, vap->iv_myaddr)) {
					/*
					 * In IEEE802.11 network, multicast packet
					 * sent from me is broadcasted from AP.
					 * It should be silently discarded for
					 * SIMPLEX interface.
					 *
					 * NB: Linux has no IFF_ flag to indicate
					 *     if an interface is SIMPLEX or not;
					 *     so we always assume it to be true.
					 */
					IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
						wh, NULL, "%s", "multicast echo");
					vap->iv_stats.is_rx_mcastecho++;
					goto out;
				}
				/* 
				 * if it is brodcasted by me on behalf of
				 * a station behind me, drop it.
				 */
				if (vap->iv_flags_ext & IEEE80211_FEXT_WDS) {
					struct ieee80211_node_table *nt;
					struct ieee80211_node *ni_wds = NULL;
					nt = &ic->ic_sta;
					ni_wds = ieee80211_find_wds_node(nt, wh->i_addr3);
					if (ni_wds) {
						ieee80211_free_node(ni_wds); /* Decr ref count */
						IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
							wh, NULL, "%s",
							"multicast echo originated from node behind me");
						vap->iv_stats.is_rx_mcastecho++;
						goto out;
					} 
				}
			}
			break;
		case IEEE80211_M_IBSS:
		case IEEE80211_M_AHDEMO:
			if (dir != IEEE80211_FC1_DIR_NODS) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
					wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}
			/* XXX no power-save support */
			break;
		case IEEE80211_M_HOSTAP:
			if ((dir != IEEE80211_FC1_DIR_TODS) &&
			    (dir != IEEE80211_FC1_DIR_DSTODS)) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
					wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}
			/* check if source STA is associated */
			if (ni == vap->iv_bss) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
					wh, "data", "%s", "unknown src");
				/* NB: caller deals with reference */
    				if (vap->iv_state == IEEE80211_S_RUN)
					ieee80211_send_error(ni, wh->i_addr2,
						IEEE80211_FC0_SUBTYPE_DEAUTH,
						IEEE80211_REASON_NOT_AUTHED);
				vap->iv_stats.is_rx_notassoc++;
				goto err;
			}
			if (ni->ni_associd == 0) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
					wh, "data", "%s", "unassoc src");
				IEEE80211_SEND_MGMT(ni,
					IEEE80211_FC0_SUBTYPE_DISASSOC,
					IEEE80211_REASON_NOT_ASSOCED);
				vap->iv_stats.is_rx_notassoc++;
				goto err;
			}
			/*
			 * If we're a 4 address packet, make sure we have an entry in
			 * the node table for the packet source address (addr4).
			 * If not, add one.
			 */
			if (dir == IEEE80211_FC1_DIR_DSTODS) {
				struct ieee80211_node_table *nt;
				struct ieee80211_frame_addr4 *wh4;
				if (!(vap->iv_flags_ext & IEEE80211_FEXT_WDS)) {
					IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
						wh, "data", "%s", "4 addr not allowed");
					goto err;
				}
				wh4 = (struct ieee80211_frame_addr4 *)skb->data;
				nt = &ic->ic_sta;
				ni_wds = ieee80211_find_wds_node(nt, wh4->i_addr4);
				/* Last call increments ref count if !NULL */
				if ((ni_wds != NULL) && (ni_wds != ni)) {
					/*
					 * node with source address (addr4) moved
					 * to another WDS capable station. remove the
					 * reference to the previous station and add 
					 * reference to the new one
					 */
					 (void) ieee80211_remove_wds_addr(nt, wh4->i_addr4);
					 ieee80211_add_wds_addr(nt, ni, wh4->i_addr4, 0);
				}
				if (ni_wds == NULL)
					ieee80211_add_wds_addr(nt, ni, wh4->i_addr4, 0);
				else
					ieee80211_free_node(ni_wds); /* Decr ref count */
			}
			
			/*
			 * Check for power save state change.
			 */
			if (!(ni->ni_flags & IEEE80211_NODE_UAPSD)) {
				if ((wh->i_fc[1] & IEEE80211_FC1_PWR_MGT) ^
				    (ni->ni_flags & IEEE80211_NODE_PWR_MGT))
					ieee80211_node_pwrsave(ni, wh->i_fc[1] & IEEE80211_FC1_PWR_MGT);
			} else if (ni->ni_flags & IEEE80211_NODE_PS_CHANGED) {
				int pwr_save_changed = 0;
				IEEE80211_LOCK_IRQ(ic);
				if ((*(u_int16_t *)(&wh->i_seq[0])) == ni->ni_pschangeseq) {
					ni->ni_flags &= ~IEEE80211_NODE_PS_CHANGED;
					pwr_save_changed = 1;
				}
				IEEE80211_UNLOCK_IRQ(ic);
				if (pwr_save_changed)
					ieee80211_node_pwrsave(ni, wh->i_fc[1] & IEEE80211_FC1_PWR_MGT);
			}
			break;
		case IEEE80211_M_WDS:
			if (dir != IEEE80211_FC1_DIR_DSTODS) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
					wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}
			break;
		default:
			/* XXX here to keep compiler happy */
			goto out;
		}

		/*
		 * Handle privacy requirements.  Note that we
		 * must not be preempted from here until after
		 * we (potentially) call ieee80211_crypto_demic;
		 * otherwise we may violate assumptions in the
		 * crypto cipher modules used to do delayed update
		 * of replay sequence numbers.
		 */
		if (wh->i_fc[1] & IEEE80211_FC1_PROT) {
			if ((vap->iv_flags & IEEE80211_F_PRIVACY) == 0) {
				/*
				 * Discard encrypted frames when privacy is off.
				 */
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
					wh, "WEP", "%s", "PRIVACY off");
				vap->iv_stats.is_rx_noprivacy++;
				IEEE80211_NODE_STAT(ni, rx_noprivacy);
				goto out;
			}
			key = ieee80211_crypto_decap(ni, skb, hdrspace);
			if (key == NULL) {
				/* NB: stats+msgs handled in crypto_decap */
				IEEE80211_NODE_STAT(ni, rx_wepfail);
				goto out;
			}
			wh = (struct ieee80211_frame *)skb->data;
			wh->i_fc[1] &= ~IEEE80211_FC1_PROT;
		} else
			key = NULL;

		/*
		 * Next up, any fragmentation.
		 */
		if (!IEEE80211_IS_MULTICAST(wh->i_addr1)) {
			skb = ieee80211_defrag(ni, skb, hdrspace);
			if (skb == NULL) {
				/* Fragment dropped or frame not complete yet */
				goto out;
			}
		}
		wh = NULL;		/* no longer valid, catch any uses */

		/*
		 * Next strip any MSDU crypto bits.
		 */
		if (key != NULL &&
		    !ieee80211_crypto_demic(vap, key, skb, hdrspace)) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
				ni->ni_macaddr, "data", "%s", "demic error");
			IEEE80211_NODE_STAT(ni, rx_demicfail);
			goto out;
		}

		/*
		 * Finally, strip the 802.11 header.
		 */
		skb = ieee80211_decap(vap, skb, hdrspace);
		if (skb == NULL) {
			/* don't count Null data frames as errors */
			if (subtype == IEEE80211_FC0_SUBTYPE_NODATA)
				goto out;
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
				ni->ni_macaddr, "data", "%s", "decap error");
			vap->iv_stats.is_rx_decap++;
			IEEE80211_NODE_STAT(ni, rx_decap);
			goto err;
		}
		eh = (struct ether_header *) skb->data;

		if (! accept_data_frame(vap, ni, key, skb, eh))
			goto out;

		vap->iv_devstats.rx_packets++;
		vap->iv_devstats.rx_bytes += skb->len;
		IEEE80211_NODE_STAT(ni, rx_data);
		IEEE80211_NODE_STAT_ADD(ni, rx_bytes, skb->len);
		ic->ic_lastdata = jiffies;

#ifdef ATH_SUPERG_FF
        	/* check for FF */
		llc = (struct llc *) (skb->data + sizeof(struct ether_header));
		if (ntohs(llc->llc_snap.ether_type) == (u_int16_t)ATH_ETH_TYPE) {
			struct sk_buff *skb1 = NULL;
			struct ether_header *eh_tmp;
			struct athl2p_tunnel_hdr *ath_hdr;
			int frame_len;

			/* NB: assumes linear (i.e., non-fragmented) skb */

			/* get to the tunneled headers */
			ath_hdr = (struct athl2p_tunnel_hdr *)
				skb_pull(skb, sizeof(struct ether_header) + LLC_SNAPFRAMELEN);
 			/* ignore invalid frames */
			if(ath_hdr == NULL)
				goto err;
			
			/* only implementing FF now. drop all others. */
			if (ath_hdr->proto != ATH_L2TUNNEL_PROTO_FF) {
				IEEE80211_DISCARD_MAC(vap,
					IEEE80211_MSG_SUPG | IEEE80211_MSG_INPUT,
					eh->ether_shost, "fast-frame",
					"bad atheros tunnel prot %u",
					ath_hdr->proto);
				vap->iv_stats.is_rx_badathtnl++;
				goto err;
			}
			vap->iv_stats.is_rx_ffcnt++;
			
			/* move past the tunneled header, with alignment */
			skb_pull(skb, roundup(sizeof(struct athl2p_tunnel_hdr) - 2, 4) + 2);

			skb1 = skb_clone(skb, GFP_ATOMIC); /* XXX: GFP_ATOMIC is overkill? */
			eh_tmp = (struct ether_header *)skb->data;

			/* ether_type must be length*/
			frame_len = ntohs(eh_tmp->ether_type);

			/* we now have 802.3 MAC hdr followed by 802.2 LLC/SNAP. convert to DIX */
			athff_decap(skb);

			/* remove second frame from end of first */
			skb_trim(skb, sizeof(struct ether_header) + frame_len - LLC_SNAPFRAMELEN);

			/* prepare second tunneled frame */
			skb_pull(skb1, roundup(sizeof(struct ether_header) + frame_len, 4));
			eh_tmp = (struct ether_header *)skb1->data;
			frame_len = ntohs(eh_tmp->ether_type);
			athff_decap(skb1);

			/* deliver the frames */
			ieee80211_deliver_data(ni, skb);
			ieee80211_deliver_data(ni, skb1);
		} else {
			/* assume non-atheros llc type */
			ieee80211_deliver_data(ni, skb);
		}
#else /* !ATH_SUPERG_FF */
		ieee80211_deliver_data(ni, skb);
#endif
		return IEEE80211_FC0_TYPE_DATA;

	case IEEE80211_FC0_TYPE_MGT:
		/*
		 * WDS opmode do not support managment frames
		 */
		if (vap->iv_opmode == IEEE80211_M_WDS) {
			vap->iv_stats.is_rx_mgtdiscard++;
			goto out;
		}
		IEEE80211_NODE_STAT(ni, rx_mgmt);
		if (dir != IEEE80211_FC1_DIR_NODS) {
			vap->iv_stats.is_rx_wrongdir++;
			goto err;
		}
		if (skb->len < sizeof(struct ieee80211_frame)) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY,
				ni->ni_macaddr, "mgt", "too short: len %u",
				skb->len);
			vap->iv_stats.is_rx_tooshort++;
			goto out;
		}
#ifdef IEEE80211_DEBUG
		if ((ieee80211_msg_debug(vap) && doprint(vap, subtype)) ||
		    ieee80211_msg_dumppkts(vap)) {
			ieee80211_note(vap, "received %s from %s rssi %d\n",
				ieee80211_mgt_subtype_name[subtype >>
				IEEE80211_FC0_SUBTYPE_SHIFT],
				ether_sprintf(wh->i_addr2), rssi);
		}
#endif
		if (wh->i_fc[1] & IEEE80211_FC1_PROT) {
			if (subtype != IEEE80211_FC0_SUBTYPE_AUTH) {
				/*
				 * Only shared key auth frames with a challenge
				 * should be encrypted, discard all others.
				 */
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
					wh, ieee80211_mgt_subtype_name[subtype >>
					IEEE80211_FC0_SUBTYPE_SHIFT],
					"%s", "WEP set but not permitted");
				vap->iv_stats.is_rx_mgtdiscard++; /* XXX */
				goto out;
			}
			if ((vap->iv_flags & IEEE80211_F_PRIVACY) == 0) {
				/*
				 * Discard encrypted frames when privacy is off.
				 */
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
					wh, "mgt", "%s", "WEP set but PRIVACY off");
				vap->iv_stats.is_rx_noprivacy++;
				goto out;
			}
			hdrspace = ieee80211_hdrspace(ic, wh);
			key = ieee80211_crypto_decap(ni, skb, hdrspace);
			if (key == NULL) {
				/* NB: stats+msgs handled in crypto_decap */
				goto out;
			}
			wh = (struct ieee80211_frame *)skb->data;
			wh->i_fc[1] &= ~IEEE80211_FC1_PROT;
		}
		ic->ic_recv_mgmt(ni, skb, subtype, rssi, rstamp);
		goto out;

	case IEEE80211_FC0_TYPE_CTL:
		IEEE80211_NODE_STAT(ni, rx_ctrl);
		vap->iv_stats.is_rx_ctl++;
		if (vap->iv_opmode == IEEE80211_M_HOSTAP)
			if (subtype == IEEE80211_FC0_SUBTYPE_PS_POLL)
				ieee80211_recv_pspoll(ni, skb);
		goto out;

	default:
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
			wh, NULL, "bad frame type 0x%x", type);
		/* should not come here */
		break;
	}
err:
	vap->iv_devstats.rx_errors++;
out:
	if (skb != NULL)
		dev_kfree_skb(skb);
	return type;
#undef HAS_SEQ
}
EXPORT_SYMBOL(ieee80211_input);


/*
 * Determines whether a frame should be accepted, based on information
 * about the frame's origin and encryption, and policy for this vap.
 */
static int accept_data_frame(struct ieee80211vap *vap,
       struct ieee80211_node *ni, struct ieee80211_key *key,
       struct sk_buff *skb, struct ether_header *eh)
{
#define IS_EAPOL(eh) ((eh)->ether_type == __constant_htons(ETHERTYPE_PAE))
#define PAIRWISE_SET(vap) ((vap)->iv_nw_keys[0].wk_cipher != &ieee80211_cipher_none)
       if (IS_EAPOL(eh)) {
               /* encrypted eapol is always OK */
               if (key)
                       return 1;
               /* cleartext eapol is OK if we don't have pairwise keys yet */
               if (! PAIRWISE_SET(vap))
                       return 1;
               /* cleartext eapol is OK if configured to allow it */
               if (! IEEE80211_VAP_DROPUNENC_EAPOL(vap))
                       return 1;
               /* cleartext eapol is OK if other unencrypted is OK */
               if (! (vap->iv_flags & IEEE80211_F_DROPUNENC))
                       return 1;
               /* not OK */
               IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
                       eh->ether_shost, "data",
                       "unauthorized port: ether type 0x%x len %u",
                       eh->ether_type, skb->len);
               vap->iv_stats.is_rx_unauth++;
               vap->iv_devstats.rx_errors++;
               IEEE80211_NODE_STAT(ni, rx_unauth);
               return 0;
       }

       if (!ieee80211_node_is_authorized(ni)) {
               /*
                * Deny any non-PAE frames received prior to
                * authorization.  For open/shared-key
                * authentication the port is mark authorized
                * after authentication completes.  For 802.1x
                * the port is not marked authorized by the
                * authenticator until the handshake has completed.
                */
               IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
                       eh->ether_shost, "data",
                       "unauthorized port: ether type 0x%x len %u",
                       eh->ether_type, skb->len);
               vap->iv_stats.is_rx_unauth++;
               vap->iv_devstats.rx_errors++;
               IEEE80211_NODE_STAT(ni, rx_unauth);
               return 0;
       } else {
               /*
                * When denying unencrypted frames, discard
                * any non-PAE frames received without encryption.
                */
               if ((vap->iv_flags & IEEE80211_F_DROPUNENC) && key == NULL) {
                       IEEE80211_NODE_STAT(ni, rx_unencrypted);
                       return 0;
               }
       }
       return 1;

#undef IS_EAPOL
#undef PAIRWISE_SET
}


/*
 * Context: softIRQ (tasklet)
 */
int
ieee80211_input_all(struct ieee80211com *ic,
	struct sk_buff *skb, int rssi, u_int32_t rstamp)
{
	struct ieee80211vap *vap;
	int type = -1;

	/* XXX locking */
	TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
		struct ieee80211_node *ni;
		struct sk_buff *skb1;

		if (TAILQ_NEXT(vap, iv_next) != NULL) {
			skb1 = skb_copy(skb, GFP_ATOMIC);
			if (skb1 == NULL) {
				/* XXX stat+msg */
				continue;
			}
		} else {
			skb1 = skb;
			skb = NULL;
		}
		ni = ieee80211_ref_node(vap->iv_bss);
		type = ieee80211_input(ni, skb1, rssi, rstamp);
		ieee80211_free_node(ni);
	}
	if (skb != NULL)		/* no vaps, reclaim skb */
		dev_kfree_skb(skb);
	return type;
}
EXPORT_SYMBOL(ieee80211_input_all);

/*
 * This function reassemble fragments using the skb of the 1st fragment,
 * if large enough. If not, a new skb is allocated to hold incoming
 * fragments.
 *
 * Fragments are copied at the end of the previous fragment.  A different
 * strategy could have been used, where a non-linear skb is allocated and
 * fragments attached to that skb.
 */
static struct sk_buff *
ieee80211_defrag(struct ieee80211_node *ni, struct sk_buff *skb, int hdrlen)
{
	struct ieee80211_frame *wh = (struct ieee80211_frame *) skb->data;
	u_int16_t rxseq, last_rxseq;
	u_int8_t fragno, last_fragno;
	u_int8_t more_frag = wh->i_fc[1] & IEEE80211_FC1_MORE_FRAG;

	rxseq = le16_to_cpu(*(u_int16_t *)wh->i_seq) >> IEEE80211_SEQ_SEQ_SHIFT;
	fragno = le16_to_cpu(*(u_int16_t *)wh->i_seq) & IEEE80211_SEQ_FRAG_MASK;

	/* Quick way out, if there's nothing to defragment */
	if (!more_frag && fragno == 0 && ni->ni_rxfrag[0] == NULL)
		return skb;

	/*
	 * Remove frag to ensure it doesn't get reaped by timer.
	 */
	if (ni->ni_table == NULL) {
		/*
		 * Should never happen.  If the node is orphaned (not in
		 * the table) then input packets should not reach here.
		 * Otherwise, a concurrent request that yanks the table
		 * should be blocked by other interlocking and/or by first
		 * shutting the driver down.  Regardless, be defensive
		 * here and just bail
		 */
		/* XXX need msg+stat */
		dev_kfree_skb(skb);
		return NULL;
	}

	/*
	 * Use this lock to make sure ni->ni_rxfrag[0] is
	 * not freed by the timer process while we use it.
	 * XXX bogus
	 */
	IEEE80211_NODE_LOCK_IRQ(ni->ni_table);

	/*
	 * Update the time stamp.  As a side effect, it
	 * also makes sure that the timer will not change
	 * ni->ni_rxfrag[0] for at least 1 second, or in
	 * other words, for the remaining of this function.
	 */
	ni->ni_rxfragstamp = jiffies;

	IEEE80211_NODE_UNLOCK_IRQ(ni->ni_table);

	/*
	 * Validate that fragment is in order and
	 * related to the previous ones.
	 */
	if (ni->ni_rxfrag[0]) {
		struct ieee80211_frame *lwh;

		lwh = (struct ieee80211_frame *) ni->ni_rxfrag[0]->data;
		last_rxseq = le16_to_cpu(*(u_int16_t *)lwh->i_seq) >>
			IEEE80211_SEQ_SEQ_SHIFT;
		last_fragno = le16_to_cpu(*(u_int16_t *)lwh->i_seq) &
			IEEE80211_SEQ_FRAG_MASK;
		if (rxseq != last_rxseq
		    || fragno != last_fragno + 1
		    || (!IEEE80211_ADDR_EQ(wh->i_addr1, lwh->i_addr1))
		    || (!IEEE80211_ADDR_EQ(wh->i_addr2, lwh->i_addr2))
		    || (ni->ni_rxfrag[0]->end - ni->ni_rxfrag[0]->tail <
			skb->len)) {
			/*
			 * Unrelated fragment or no space for it,
			 * clear current fragments
			 */
			dev_kfree_skb(ni->ni_rxfrag[0]);
			ni->ni_rxfrag[0] = NULL;
		}
	}

	/* If this is the first fragment */
 	if (ni->ni_rxfrag[0] == NULL && fragno == 0) {
		ni->ni_rxfrag[0] = skb;
		/* If more frags are coming */
		if (more_frag) {
			if (skb_is_nonlinear(skb)) {
				/*
				 * We need a continous buffer to
				 * assemble fragments
				 */
				ni->ni_rxfrag[0] = skb_copy(skb, GFP_ATOMIC);
				dev_kfree_skb(skb);
			}
			/*
			 * Check that we have enough space to hold
			 * incoming fragments
			 * XXX 4-address/QoS frames?
			 */
			else if (skb->end - skb->head < ni->ni_vap->iv_dev->mtu +
				 hdrlen) {
				ni->ni_rxfrag[0] = skb_copy_expand(skb, 0,
					(ni->ni_vap->iv_dev->mtu + hdrlen) -
					(skb->end - skb->head), GFP_ATOMIC);
				dev_kfree_skb(skb);
			}
		}
	} else {
		if (ni->ni_rxfrag[0]) {
			struct ieee80211_frame *lwh = (struct ieee80211_frame *)
				ni->ni_rxfrag[0]->data;

			/*
			 * We know we have enough space to copy,
			 * we've verified that before
			 */
			/* Copy current fragment at end of previous one */
			memcpy(ni->ni_rxfrag[0]->tail,
			       skb->data + hdrlen, skb->len - hdrlen);
			/* Update tail and length */
			skb_put(ni->ni_rxfrag[0], skb->len - hdrlen);
			/* Keep a copy of last sequence and fragno */
			*(u_int16_t *) lwh->i_seq = *(u_int16_t *) wh->i_seq;
		}
		/* we're done with the fragment */
		dev_kfree_skb(skb);
	}
		
	if (more_frag) {
		/* More to come */
		skb = NULL;
	} else {
		/* Last fragment received, we're done! */
		skb = ni->ni_rxfrag[0];
		ni->ni_rxfrag[0] = NULL;
	}
	return skb;
}

static void 
ieee80211_deliver_data(struct ieee80211_node *ni, struct sk_buff *skb)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct net_device *dev = vap->iv_dev;
	struct ether_header *eh = (struct ether_header *) skb->data;

#ifdef ATH_SUPERG_XR 
	/*
	 * if it is a XR vap, send the data to associated normal net
	 * device. XR vap has a net device which is not registered with
	 * OS. 
	 */
	if (vap->iv_xrvap && vap->iv_flags & IEEE80211_F_XR)
		dev = vap->iv_xrvap->iv_dev;
#endif

	/* perform as a bridge within the vap */
	/* XXX intra-vap bridging only */
	if (vap->iv_opmode == IEEE80211_M_HOSTAP &&
	    (vap->iv_flags & IEEE80211_F_NOBRIDGE) == 0) {
		struct sk_buff *skb1 = NULL;
		
		if (ETHER_IS_MULTICAST(eh->ether_dhost))
			skb1 = skb_copy(skb, GFP_ATOMIC);
		else {
			/*
			 * Check if destination is associated with the
			 * same vap and authorized to receive traffic.
			 * Beware of traffic destined for the vap itself;
			 * sending it will not work; just let it be
			 * delivered normally.
			 */
			struct ieee80211_node *ni1 = ieee80211_find_node(
				&vap->iv_ic->ic_sta, eh->ether_dhost);
			if (ni1 != NULL) {
				if (ni1->ni_vap == vap &&
				    ieee80211_node_is_authorized(ni1) &&
				    ni1 != vap->iv_bss) {
					skb1 = skb;
					skb = NULL;
				}
				/* XXX statistic? */
				ieee80211_free_node(ni1);
			}
		}
		if (skb1 != NULL) {
			skb1->dev = dev;
			skb1->mac.raw = skb1->data;
			skb1->nh.raw = skb1->data + sizeof(struct ether_header);
			skb1->protocol = __constant_htons(ETH_P_802_2);
			/* XXX insert vlan tag before queue it? */
			dev_queue_xmit(skb1);
		}
	}

	if (skb != NULL) {
		skb->dev = dev;
		
#ifdef USE_HEADERLEN_RESV
		skb->protocol = ath_eth_type_trans(skb, dev);
#else
		skb->protocol = eth_type_trans(skb, dev);
#endif
		if (ni->ni_vlan != 0 && vap->iv_vlgrp != NULL) {
			/* attach vlan tag */
			vlan_hwaccel_receive_skb(skb, vap->iv_vlgrp, ni->ni_vlan);
		} else
			netif_rx(skb);
		dev->last_rx = jiffies;
	}
}

static struct sk_buff *
ieee80211_decap(struct ieee80211vap *vap, struct sk_buff *skb, int hdrlen)
{
	struct ieee80211_qosframe_addr4 wh;	/* Max size address frames */
	struct ether_header *eh;
	struct llc *llc;
	u_short ether_type = 0;

	memcpy(&wh, skb->data, hdrlen);	/* Only copy hdrlen over */
	llc = (struct llc *) skb_pull(skb, hdrlen);
	if (skb->len >= LLC_SNAPFRAMELEN &&
	    llc->llc_dsap == LLC_SNAP_LSAP && llc->llc_ssap == LLC_SNAP_LSAP &&
	    llc->llc_control == LLC_UI && llc->llc_snap.org_code[0] == 0 &&
	    llc->llc_snap.org_code[1] == 0 && llc->llc_snap.org_code[2] == 0) {
		ether_type = llc->llc_un.type_snap.ether_type;
		skb_pull(skb, LLC_SNAPFRAMELEN);
		llc = NULL;
	}
	eh = (struct ether_header *) skb_push(skb, sizeof(struct ether_header));
	switch (wh.i_fc[1] & IEEE80211_FC1_DIR_MASK) {
	case IEEE80211_FC1_DIR_NODS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr1);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr2);
		break;
	case IEEE80211_FC1_DIR_TODS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr3);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr2);
		break;
	case IEEE80211_FC1_DIR_FROMDS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr1);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr3);
		break;
	case IEEE80211_FC1_DIR_DSTODS:
		IEEE80211_ADDR_COPY(eh->ether_dhost, wh.i_addr3);
		IEEE80211_ADDR_COPY(eh->ether_shost, wh.i_addr4);
		break;
	}
	if (!ALIGNED_POINTER(skb->data + sizeof(*eh), u_int32_t)) {
		struct sk_buff *n;

		/* XXX does this always work? */
		n = skb_copy(skb, GFP_ATOMIC);
		dev_kfree_skb(skb);
		if (n == NULL)
			return NULL;
		skb = n;
		eh = (struct ether_header *) skb->data;
	}
	if (llc != NULL)
		eh->ether_type = htons(skb->len - sizeof(*eh));
	else
		eh->ether_type = ether_type;
	return skb;
}

/*
 * Install received rate set information in the node's state block.
 */
int
ieee80211_setup_rates(struct ieee80211_node *ni,
	const u_int8_t *rates, const u_int8_t *xrates, int flags)
{
	struct ieee80211_rateset *rs = &ni->ni_rates;

	memset(rs, 0, sizeof(*rs));
	rs->rs_nrates = rates[1];
	memcpy(rs->rs_rates, rates + 2, rs->rs_nrates);
	if (xrates != NULL) {
		u_int8_t nxrates;
		/*
		 * Tack on 11g extended supported rate element.
		 */
		nxrates = xrates[1];
		if (rs->rs_nrates + nxrates > IEEE80211_RATE_MAXSIZE) {
			struct ieee80211vap *vap = ni->ni_vap;

			nxrates = IEEE80211_RATE_MAXSIZE - rs->rs_nrates;
			IEEE80211_NOTE(vap, IEEE80211_MSG_XRATE, ni,
				"extended rate set too large;"
				" only using %u of %u rates",
				nxrates, xrates[1]);
			vap->iv_stats.is_rx_rstoobig++;
		}
		memcpy(rs->rs_rates + rs->rs_nrates, xrates+2, nxrates);
		rs->rs_nrates += nxrates;
	}
	return ieee80211_fix_rate(ni, flags);
}

static void
ieee80211_auth_open(struct ieee80211_node *ni, struct ieee80211_frame *wh,
	int rssi, u_int32_t rstamp, u_int16_t seq, u_int16_t status)
{
	struct ieee80211vap *vap = ni->ni_vap;

	if (ni->ni_authmode == IEEE80211_AUTH_SHARED) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
			ni->ni_macaddr, "open auth",
			"bad sta auth mode %u", ni->ni_authmode);
		vap->iv_stats.is_rx_bad_auth++;	/* XXX maybe a unique error? */
		if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
			/* XXX hack to workaround calling convention */

			/* XXX To send the frame to the requesting STA, we have to
			 * create a node for the station that we're going to reject.
			 * The node will be freed automatically */
			if (ni == vap->iv_bss) {
				ni = ieee80211_dup_bss(vap, wh->i_addr2);
				if (ni == NULL)
					return;

				IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, 
				"%s: %p<%s> refcnt %d\n", __func__, ni, ether_sprintf(ni->ni_macaddr), 
				ieee80211_node_refcnt(ni));
			}
			IEEE80211_SEND_MGMT(ni,	IEEE80211_FC0_SUBTYPE_AUTH,
				(seq + 1) | (IEEE80211_STATUS_ALG<<16));
			return;
		}
	}
	switch (vap->iv_opmode) {
	case IEEE80211_M_IBSS:
		if (vap->iv_state != IEEE80211_S_RUN ||
		    seq != IEEE80211_AUTH_OPEN_REQUEST) {
			vap->iv_stats.is_rx_bad_auth++;
			return;
		}
		ieee80211_new_state(vap, IEEE80211_S_AUTH,
			wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK);
		break;

	case IEEE80211_M_AHDEMO:
	case IEEE80211_M_WDS:
		/* should not come here */
		break;

	case IEEE80211_M_HOSTAP:
		if (vap->iv_state != IEEE80211_S_RUN ||
		    seq != IEEE80211_AUTH_OPEN_REQUEST) {
			vap->iv_stats.is_rx_bad_auth++;
			return;
		}
		/* always accept open authentication requests */
		if (ni == vap->iv_bss) {
			ni = ieee80211_dup_bss(vap, wh->i_addr2); 
			if (ni == NULL)
				return;

			IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, 
			"%s: %p<%s> refcnt %d\n", __func__, ni, ether_sprintf(ni->ni_macaddr), 
			ieee80211_node_refcnt(ni));

		} else if ((ni->ni_flags & IEEE80211_NODE_AREF) == 0)
			(void) ieee80211_ref_node(ni);
		/*
		 * Mark the node as referenced to reflect that it's
		 * reference count has been bumped to ensure it remains
		 * after the transaction completes.
		 */
		ni->ni_flags |= IEEE80211_NODE_AREF;

		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, seq + 1);
		IEEE80211_NOTE(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH,
			ni, "station authenticated (%s)", "open");
		/*
		 * When 802.1x is not in use mark the port
		 * authorized at this point so traffic can flow.
		 */
		if (ni->ni_authmode != IEEE80211_AUTH_8021X)
			ieee80211_node_authorize(ni);
		break;

	case IEEE80211_M_STA:
		if (vap->iv_state != IEEE80211_S_AUTH ||
		    seq != IEEE80211_AUTH_OPEN_RESPONSE) {
			vap->iv_stats.is_rx_bad_auth++;
			return;
		}
		if (status != 0) {
			IEEE80211_NOTE(vap,
				IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni,
				"open auth failed (reason %d)", status);
			vap->iv_stats.is_rx_auth_fail++;
			ieee80211_new_state(vap, IEEE80211_S_SCAN,
				IEEE80211_SCAN_FAIL_STATUS);
		} else
			ieee80211_new_state(vap, IEEE80211_S_ASSOC, 0);
		break;
	case IEEE80211_M_MONITOR:
		break;
	}
}

/*
 * Send a management frame error response to the specified
 * station.  If ni is associated with the station then use
 * it; otherwise allocate a temporary node suitable for
 * transmitting the frame and then free the reference so
 * it will go away as soon as the frame has been transmitted.
 */
static void
ieee80211_send_error(struct ieee80211_node *ni,
	const u_int8_t *mac, int subtype, int arg)
{
	struct ieee80211vap *vap = ni->ni_vap;
	int istmp;

	if (ni == vap->iv_bss) {
		ni = ieee80211_tmp_node(vap, mac);
		if (ni == NULL) {
			/* XXX msg */
			return;
		}
		istmp = 1;
	} else
		istmp = 0;
	IEEE80211_SEND_MGMT(ni, subtype, arg);
	if (istmp)
		ieee80211_free_node(ni);
}

static int
alloc_challenge(struct ieee80211_node *ni)
{
	if (ni->ni_challenge == NULL)
		MALLOC(ni->ni_challenge, u_int32_t*, IEEE80211_CHALLENGE_LEN,
			M_DEVBUF, M_NOWAIT);
	if (ni->ni_challenge == NULL) {
		IEEE80211_NOTE(ni->ni_vap,
			IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni,
			"%s", "shared key challenge alloc failed");
		/* XXX statistic */
	}
	return (ni->ni_challenge != NULL);
}

/* XXX TODO: add statistics */
static void
ieee80211_auth_shared(struct ieee80211_node *ni, struct ieee80211_frame *wh,
	u_int8_t *frm, u_int8_t *efrm, int rssi, u_int32_t rstamp,
	u_int16_t seq, u_int16_t status)
{
	struct ieee80211vap *vap = ni->ni_vap;
	u_int8_t *challenge;
	int allocbs, estatus;

	/*
	 * NB: this can happen as we allow pre-shared key
	 * authentication to be enabled w/o wep being turned
	 * on so that configuration of these can be done
	 * in any order.  It may be better to enforce the
	 * ordering in which case this check would just be
	 * for sanity/consistency.
	 */
	estatus = 0;			/* NB: silence compiler */
	if ((vap->iv_flags & IEEE80211_F_PRIVACY) == 0) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
			ni->ni_macaddr, "shared key auth",
			"%s", " PRIVACY is disabled");
		estatus = IEEE80211_STATUS_ALG;
		goto bad;
	}
	/*
	 * Pre-shared key authentication is evil; accept
	 * it only if explicitly configured (it is supported
	 * mainly for compatibility with clients like OS X).
	 */
	if (ni->ni_authmode != IEEE80211_AUTH_AUTO &&
	    ni->ni_authmode != IEEE80211_AUTH_SHARED) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
			ni->ni_macaddr, "shared key auth",
			"bad sta auth mode %u", ni->ni_authmode);
		vap->iv_stats.is_rx_bad_auth++;	/* XXX maybe a unique error? */
		estatus = IEEE80211_STATUS_ALG;
		goto bad;
	}

	challenge = NULL;
	if (frm + 1 < efrm) {
		if ((frm[1] + 2) > (efrm - frm)) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
				ni->ni_macaddr, "shared key auth",
				"ie %d/%d too long",
				frm[0], (frm[1] + 2) - (efrm - frm));
			vap->iv_stats.is_rx_bad_auth++;
			estatus = IEEE80211_STATUS_CHALLENGE;
			goto bad;
		}
		if (*frm == IEEE80211_ELEMID_CHALLENGE)
			challenge = frm;
		frm += frm[1] + 2;
	}
	switch (seq) {
	case IEEE80211_AUTH_SHARED_CHALLENGE:
	case IEEE80211_AUTH_SHARED_RESPONSE:
		if (challenge == NULL) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
				ni->ni_macaddr, "shared key auth",
				"%s", "no challenge");
			vap->iv_stats.is_rx_bad_auth++;
			estatus = IEEE80211_STATUS_CHALLENGE;
			goto bad;
		}
		if (challenge[1] != IEEE80211_CHALLENGE_LEN) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
				ni->ni_macaddr, "shared key auth",
				"bad challenge len %d", challenge[1]);
			vap->iv_stats.is_rx_bad_auth++;
			estatus = IEEE80211_STATUS_CHALLENGE;
			goto bad;
		}
	default:
		break;
	}
	switch (vap->iv_opmode) {
	case IEEE80211_M_MONITOR:
	case IEEE80211_M_AHDEMO:
	case IEEE80211_M_IBSS:
	case IEEE80211_M_WDS:
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
			ni->ni_macaddr, "shared key auth",
			"bad operating mode %u", vap->iv_opmode);
		return;
	case IEEE80211_M_HOSTAP:
		if (vap->iv_state != IEEE80211_S_RUN) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
				ni->ni_macaddr, "shared key auth",
				"bad state %u", vap->iv_state);
			estatus = IEEE80211_STATUS_ALG;	/* XXX */
			goto bad;
		}
		switch (seq) {
		case IEEE80211_AUTH_SHARED_REQUEST:
			if (ni == vap->iv_bss) {
				ni = ieee80211_dup_bss(vap, wh->i_addr2);
				if (ni == NULL) {
					/* NB: no way to return an error */
					return;
				}

				IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, 
				"%s: %p<%s> refcnt %d\n", __func__, ni, ether_sprintf(ni->ni_macaddr), 
				ieee80211_node_refcnt(ni));

				allocbs = 1;
			} else {
				if ((ni->ni_flags & IEEE80211_NODE_AREF) == 0)
					(void) ieee80211_ref_node(ni);
				allocbs = 0;
			}
			/*
			 * Mark the node as referenced to reflect that it's
			 * reference count has been bumped to ensure it remains
			 * after the transaction completes.
			 */
			ni->ni_flags |= IEEE80211_NODE_AREF;
			ni->ni_rssi = rssi;
			ni->ni_rstamp = rstamp;
			ni->ni_last_rx = jiffies;
			if (!alloc_challenge(ni)) {
				/* NB: don't return error so they rexmit */
				return;
			}
			get_random_bytes(ni->ni_challenge,
				IEEE80211_CHALLENGE_LEN);
			IEEE80211_NOTE(vap,
				IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni,
				"shared key %sauth request", allocbs ? "" : "re");
			break;
		case IEEE80211_AUTH_SHARED_RESPONSE:
			if (ni == vap->iv_bss) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
					ni->ni_macaddr, "shared key response",
					"%s", "unknown station");
				/* NB: don't send a response */
				return;
			}
			if (ni->ni_challenge == NULL) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
					ni->ni_macaddr, "shared key response",
					"%s", "no challenge recorded");
				vap->iv_stats.is_rx_bad_auth++;
				estatus = IEEE80211_STATUS_CHALLENGE;
				goto bad;
			}
			if (memcmp(ni->ni_challenge, &challenge[2],
			    challenge[1]) != 0) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
					ni->ni_macaddr, "shared key response",
					"%s", "challenge mismatch");
				vap->iv_stats.is_rx_auth_fail++;
				estatus = IEEE80211_STATUS_CHALLENGE;
				goto bad;
			}
			IEEE80211_NOTE(vap,
				IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni,
				"station authenticated (%s)", "shared key");
			ieee80211_node_authorize(ni);
			break;
		default:
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH,
				ni->ni_macaddr, "shared key auth",
				"bad seq %d", seq);
			vap->iv_stats.is_rx_bad_auth++;
			estatus = IEEE80211_STATUS_SEQUENCE;
			goto bad;
		}
		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, seq + 1);
		break;

	case IEEE80211_M_STA:
		if (vap->iv_state != IEEE80211_S_AUTH)
			return;
		switch (seq) {
		case IEEE80211_AUTH_SHARED_PASS:
			if (ni->ni_challenge != NULL) {
				FREE(ni->ni_challenge, M_DEVBUF);
				ni->ni_challenge = NULL;
			}
			if (status != 0) {
				IEEE80211_NOTE_MAC(vap,
					IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH,
					ieee80211_getbssid(vap, wh),
					"shared key auth failed (reason %d)",
					status);
				vap->iv_stats.is_rx_auth_fail++;
				/* XXX IEEE80211_SCAN_FAIL_STATUS */
				goto bad;
			}
			ieee80211_new_state(vap, IEEE80211_S_ASSOC, 0);
			break;
		case IEEE80211_AUTH_SHARED_CHALLENGE:
			if (!alloc_challenge(ni))
				goto bad;
			/* XXX could optimize by passing recvd challenge */
			memcpy(ni->ni_challenge, &challenge[2], challenge[1]);
			IEEE80211_SEND_MGMT(ni,
				IEEE80211_FC0_SUBTYPE_AUTH, seq + 1);
			break;
		default:
			IEEE80211_DISCARD(vap, IEEE80211_MSG_AUTH,
				wh, "shared key auth", "bad seq %d", seq);
			vap->iv_stats.is_rx_bad_auth++;
			goto bad;
		}
		break;
	}
	return;
bad:
	/*
	 * Send an error response; but only when operating as an AP.
	 */
	if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
		/* XXX hack to workaround calling convention */
		ieee80211_send_error(ni, wh->i_addr2, 
			IEEE80211_FC0_SUBTYPE_AUTH,
			(seq + 1) | (estatus<<16));
	} else if (vap->iv_opmode == IEEE80211_M_STA) {
		/*
		 * Kick the state machine.  This short-circuits
		 * using the mgt frame timeout to trigger the
		 * state transition.
		 */
		if (vap->iv_state == IEEE80211_S_AUTH)
			ieee80211_new_state(vap, IEEE80211_S_SCAN, 0);
	}
}

/* Verify the existence and length of __elem or get out. */
#define IEEE80211_VERIFY_ELEMENT(__elem, __maxlen) do {			\
	if ((__elem) == NULL) {						\
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,		\
			wh, ieee80211_mgt_subtype_name[subtype >>	\
				IEEE80211_FC0_SUBTYPE_SHIFT],		\
			"%s", "no " #__elem );				\
		vap->iv_stats.is_rx_elem_missing++;			\
		return;							\
	}								\
	if ((__elem)[1] > (__maxlen)) {					\
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,		\
			wh, ieee80211_mgt_subtype_name[subtype >>	\
				IEEE80211_FC0_SUBTYPE_SHIFT],		\
			"bad " #__elem " len %d", (__elem)[1]);		\
		vap->iv_stats.is_rx_elem_toobig++;			\
		return;							\
	}								\
} while (0)

#define	IEEE80211_VERIFY_LENGTH(_len, _minlen) do {			\
	if ((_len) < (_minlen)) {					\
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,		\
			wh, ieee80211_mgt_subtype_name[subtype >>	\
				IEEE80211_FC0_SUBTYPE_SHIFT],		\
			"%s", "ie too short");				\
		vap->iv_stats.is_rx_elem_toosmall++;			\
		return;							\
	}								\
} while (0)

#ifdef IEEE80211_DEBUG
static void
ieee80211_ssid_mismatch(struct ieee80211vap *vap, const char *tag,
	u_int8_t mac[IEEE80211_ADDR_LEN], u_int8_t *ssid)
{
	printf("[%s] discard %s frame, ssid mismatch: ",
		ether_sprintf(mac), tag);
	ieee80211_print_essid(ssid + 2, ssid[1]);
	printf("\n");
}

#define	IEEE80211_VERIFY_SSID(_ni, _ssid) do {				\
	if ((_ssid)[1] != 0 &&						\
	    ((_ssid)[1] != (_ni)->ni_esslen ||				\
	    memcmp((_ssid) + 2, (_ni)->ni_essid, (_ssid)[1]) != 0)) {	\
		if (ieee80211_msg_input(vap))				\
			ieee80211_ssid_mismatch(vap, 			\
			    	ieee80211_mgt_subtype_name[subtype >>	\
					IEEE80211_FC0_SUBTYPE_SHIFT],	\
				wh->i_addr2, _ssid);			\
		vap->iv_stats.is_rx_ssidmismatch++;			\
		return;							\
	}								\
} while (0)
#else /* !IEEE80211_DEBUG */
#define	IEEE80211_VERIFY_SSID(_ni, _ssid) do {				\
	if ((_ssid)[1] != 0 &&						\
	    ((_ssid)[1] != (_ni)->ni_esslen ||				\
	    memcmp((_ssid) + 2, (_ni)->ni_essid, (_ssid)[1]) != 0)) {	\
		vap->iv_stats.is_rx_ssidmismatch++;			\
		return;							\
	}								\
} while (0)
#endif /* !IEEE80211_DEBUG */

/* unalligned little endian access */     
#define LE_READ_2(p)					\
	((u_int16_t)					\
	 ((((const u_int8_t *)(p))[0]      ) |		\
	  (((const u_int8_t *)(p))[1] <<  8)))
#define LE_READ_4(p)					\
	((u_int32_t)					\
	 ((((const u_int8_t *)(p))[0]      ) |		\
	  (((const u_int8_t *)(p))[1] <<  8) |		\
	  (((const u_int8_t *)(p))[2] << 16) |		\
	  (((const u_int8_t *)(p))[3] << 24)))

static __inline int
iswpaoui(const u_int8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((WPA_OUI_TYPE<<24)|WPA_OUI);
}

static __inline int
iswmeoui(const u_int8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((WME_OUI_TYPE<<24)|WME_OUI);
}

static __inline int
iswmeparam(const u_int8_t *frm)
{
	return frm[1] > 5 && LE_READ_4(frm+2) == ((WME_OUI_TYPE<<24)|WME_OUI) &&
		frm[6] == WME_PARAM_OUI_SUBTYPE;
}

static __inline int
iswmeinfo(const u_int8_t *frm)
{
	return frm[1] > 5 && LE_READ_4(frm+2) == ((WME_OUI_TYPE<<24)|WME_OUI) &&
		frm[6] == WME_INFO_OUI_SUBTYPE;
}

static __inline int
isatherosoui(const u_int8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((ATH_OUI_TYPE<<24)|ATH_OUI);
}

/*
 * Convert a WPA cipher selector OUI to an internal
 * cipher algorithm.  Where appropriate we also
 * record any key length.
 */
static int
wpa_cipher(u_int8_t *sel, u_int8_t *keylen)
{
#define	WPA_SEL(x)	(((x) << 24) | WPA_OUI)
	u_int32_t w = LE_READ_4(sel);

	switch (w) {
	case WPA_SEL(WPA_CSE_NULL):
		return IEEE80211_CIPHER_NONE;
	case WPA_SEL(WPA_CSE_WEP40):
		if (keylen)
			*keylen = 40 / NBBY;
		return IEEE80211_CIPHER_WEP;
	case WPA_SEL(WPA_CSE_WEP104):
		if (keylen)
			*keylen = 104 / NBBY;
		return IEEE80211_CIPHER_WEP;
	case WPA_SEL(WPA_CSE_TKIP):
		return IEEE80211_CIPHER_TKIP;
	case WPA_SEL(WPA_CSE_CCMP):
		return IEEE80211_CIPHER_AES_CCM;
	}
	return 32;		/* NB: so 1<< is discarded */
#undef WPA_SEL
}

/*
 * Convert a WPA key management/authentication algorithm
 * to an internal code.
 */
static int
wpa_keymgmt(u_int8_t *sel)
{
#define	WPA_SEL(x)	(((x)<<24)|WPA_OUI)
	u_int32_t w = LE_READ_4(sel);

	switch (w) {
	case WPA_SEL(WPA_ASE_8021X_UNSPEC):
		return WPA_ASE_8021X_UNSPEC;
	case WPA_SEL(WPA_ASE_8021X_PSK):
		return WPA_ASE_8021X_PSK;
	case WPA_SEL(WPA_ASE_NONE):
		return WPA_ASE_NONE;
	}
	return 0;		/* NB: so is discarded */
#undef WPA_SEL
}

/*
 * Parse a WPA information element to collect parameters
 * and validate the parameters against what has been
 * configured for the system.
 */
static int
ieee80211_parse_wpa(struct ieee80211vap *vap, u_int8_t *frm,
	struct ieee80211_rsnparms *rsn_parm, const struct ieee80211_frame *wh)
{
	u_int8_t len = frm[1];
	u_int32_t w;
	int n;

	/*
	 * Check the length once for fixed parts: OUI, type,
	 * version, mcast cipher, and 2 selector counts.
	 * Other, variable-length data, must be checked separately.
	 */
	if (!(vap->iv_flags & IEEE80211_F_WPA1)) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "vap not WPA, flags 0x%x", vap->iv_flags);
		return IEEE80211_REASON_IE_INVALID;
	}
	
	if (len < 14) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "too short, len %u", len);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 6, len -= 4;		/* NB: len is payload only */
	/* NB: iswapoui already validated the OUI and type */
	w = LE_READ_2(frm);
	if (w != WPA_VERSION) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "bad version %u", w);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 2;
	len -= 2;

	/* multicast/group cipher */
	w = wpa_cipher(frm, &rsn_parm->rsn_mcastkeylen);
	if (w != rsn_parm->rsn_mcastcipher) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "mcast cipher mismatch; got %u, expected %u",
			w, rsn_parm->rsn_mcastcipher);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 4;
	len -= 4;

	/* unicast ciphers */
	n = LE_READ_2(frm);
	frm += 2;
	len -= 2;
	if (len < n*4+2) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "ucast cipher data too short; len %u, n %u",
			len, n);
		return IEEE80211_REASON_IE_INVALID;
	}
	w = 0;
	for (; n > 0; n--) {
		w |= 1 << wpa_cipher(frm, &rsn_parm->rsn_ucastkeylen);
		frm += 4;
		len -= 4;
	}
	w &= rsn_parm->rsn_ucastcipherset;
	if (w == 0) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "%s", "ucast cipher set empty");
		return IEEE80211_REASON_IE_INVALID;
	}
	if (w & (1 << IEEE80211_CIPHER_TKIP))
		rsn_parm->rsn_ucastcipher = IEEE80211_CIPHER_TKIP;
	else
		rsn_parm->rsn_ucastcipher = IEEE80211_CIPHER_AES_CCM;

	/* key management algorithms */
	n = LE_READ_2(frm);
	frm += 2;
	len -= 2;
	if (len < n * 4) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "key mgmt alg data too short; len %u, n %u",
			len, n);
		return IEEE80211_REASON_IE_INVALID;
	}
	w = 0;
	for (; n > 0; n--) {
		w |= wpa_keymgmt(frm);
		frm += 4;
		len -= 4;
	}
	w &= rsn_parm->rsn_keymgmtset;
	if (w == 0) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "WPA", "%s", "no acceptable key mgmt alg");
		return IEEE80211_REASON_IE_INVALID;
	}
	if (w & WPA_ASE_8021X_UNSPEC)
		rsn_parm->rsn_keymgmt = WPA_ASE_8021X_UNSPEC;
	else
		rsn_parm->rsn_keymgmt = WPA_ASE_8021X_PSK;

	if (len > 2)		/* optional capabilities */
		rsn_parm->rsn_caps = LE_READ_2(frm);

	return 0;
}

/*
 * Convert an RSN cipher selector OUI to an internal
 * cipher algorithm.  Where appropriate we also
 * record any key length.
 */
static int
rsn_cipher(u_int8_t *sel, u_int8_t *keylen)
{
#define	RSN_SEL(x)	(((x) << 24) | RSN_OUI)
	u_int32_t w = LE_READ_4(sel);

	switch (w) {
	case RSN_SEL(RSN_CSE_NULL):
		return IEEE80211_CIPHER_NONE;
	case RSN_SEL(RSN_CSE_WEP40):
		if (keylen)
			*keylen = 40 / NBBY;
		return IEEE80211_CIPHER_WEP;
	case RSN_SEL(RSN_CSE_WEP104):
		if (keylen)
			*keylen = 104 / NBBY;
		return IEEE80211_CIPHER_WEP;
	case RSN_SEL(RSN_CSE_TKIP):
		return IEEE80211_CIPHER_TKIP;
	case RSN_SEL(RSN_CSE_CCMP):
		return IEEE80211_CIPHER_AES_CCM;
	case RSN_SEL(RSN_CSE_WRAP):
		return IEEE80211_CIPHER_AES_OCB;
	}
	return 32;		/* NB: so 1<< is discarded */
#undef RSN_SEL
}

/*
 * Convert an RSN key management/authentication algorithm
 * to an internal code.
 */
static int
rsn_keymgmt(u_int8_t *sel)
{
#define	RSN_SEL(x)	(((x) << 24) | RSN_OUI)
	u_int32_t w = LE_READ_4(sel);

	switch (w) {
	case RSN_SEL(RSN_ASE_8021X_UNSPEC):
		return RSN_ASE_8021X_UNSPEC;
	case RSN_SEL(RSN_ASE_8021X_PSK):
		return RSN_ASE_8021X_PSK;
	case RSN_SEL(RSN_ASE_NONE):
		return RSN_ASE_NONE;
	}
	return 0;		/* NB: so is discarded */
#undef RSN_SEL
}

/*
 * Parse a WPA/RSN information element to collect parameters
 * and validate the parameters against what has been
 * configured for the system.
 */
static int
ieee80211_parse_rsn(struct ieee80211vap *vap, u_int8_t *frm,
	struct ieee80211_rsnparms *rsn_parm, const struct ieee80211_frame *wh)
{
	u_int8_t len = frm[1];
	u_int32_t w;
	int n;

	/*
	 * Check the length once for fixed parts: 
	 * version, mcast cipher, and 2 selector counts.
	 * Other, variable-length data, must be checked separately.
	 */
	if (!(vap->iv_flags & IEEE80211_F_WPA2)) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "vap not RSN, flags 0x%x", vap->iv_flags);
		return IEEE80211_REASON_IE_INVALID;
	}
	
	if (len < 10) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "too short, len %u", len);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 2;
	w = LE_READ_2(frm);
	if (w != RSN_VERSION) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "bad version %u", w);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 2;
	len -= 2;

	/* multicast/group cipher */
	w = rsn_cipher(frm, &rsn_parm->rsn_mcastkeylen);
	if (w != rsn_parm->rsn_mcastcipher) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "mcast cipher mismatch; got %u, expected %u",
			w, rsn_parm->rsn_mcastcipher);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 4;
	len -= 4;

	/* unicast ciphers */
	n = LE_READ_2(frm);
	frm += 2;
	len -= 2;
	if (len < n * 4 + 2) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "ucast cipher data too short; len %u, n %u",
			len, n);
		return IEEE80211_REASON_IE_INVALID;
	}
	w = 0;
	for (; n > 0; n--) {
		w |= 1 << rsn_cipher(frm, &rsn_parm->rsn_ucastkeylen);
		frm += 4;
		len -= 4;
	}
	w &= rsn_parm->rsn_ucastcipherset;
	if (w == 0) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "%s", "ucast cipher set empty");
		return IEEE80211_REASON_IE_INVALID;
	}
	if (w & (1<<IEEE80211_CIPHER_TKIP))
		rsn_parm->rsn_ucastcipher = IEEE80211_CIPHER_TKIP;
	else
		rsn_parm->rsn_ucastcipher = IEEE80211_CIPHER_AES_CCM;

	/* key management algorithms */
	n = LE_READ_2(frm);
	frm += 2;
	len -= 2;
	if (len < n * 4) {
		IEEE80211_DISCARD_IE(vap, 
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "key mgmt alg data too short; len %u, n %u",
			len, n);
		return IEEE80211_REASON_IE_INVALID;
	}
	w = 0;
	for (; n > 0; n--) {
		w |= rsn_keymgmt(frm);
		frm += 4;
		len -= 4;
	}
	w &= rsn_parm->rsn_keymgmtset;
	if (w == 0) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA,
			wh, "RSN", "%s", "no acceptable key mgmt alg");
		return IEEE80211_REASON_IE_INVALID;
	}
	if (w & RSN_ASE_8021X_UNSPEC)
		rsn_parm->rsn_keymgmt = RSN_ASE_8021X_UNSPEC;
	else
		rsn_parm->rsn_keymgmt = RSN_ASE_8021X_PSK;

	/* optional RSN capabilities */
	if (len > 2)
		rsn_parm->rsn_caps = LE_READ_2(frm);
	/* XXXPMKID */

	return 0;
}

void
ieee80211_saveie(u_int8_t **iep, const u_int8_t *ie)
{
	u_int ielen = ie[1] + 2;
	/*
	 * Record information element for later use.
	 */
	if (*iep == NULL || (*iep)[1] != ie[1]) {
		if (*iep != NULL)
			FREE(*iep, M_DEVBUF);
		MALLOC(*iep, void*, ielen, M_DEVBUF, M_NOWAIT);
	}
	if (*iep != NULL)
		memcpy(*iep, ie, ielen);
}
EXPORT_SYMBOL(ieee80211_saveie);

static int
ieee80211_parse_wmeie(u_int8_t *frm, const struct ieee80211_frame *wh, 
					  struct ieee80211_node *ni)
{
	u_int len = frm[1];

	if (len != 7) {
		IEEE80211_DISCARD_IE(ni->ni_vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WME,
			wh, "WME IE", "too short, len %u", len);
		return -1;
	}
	ni->ni_uapsd = frm[WME_CAPINFO_IE_OFFSET];
	if (ni->ni_uapsd) {
		ni->ni_flags |= IEEE80211_NODE_UAPSD;
		switch (WME_UAPSD_MAXSP(ni->ni_uapsd)) {
		case 1: 
			ni->ni_uapsd_maxsp = 2; break;
		case 2: 
			ni->ni_uapsd_maxsp = 4; break;
		case 3: 
			ni->ni_uapsd_maxsp = 6; break;
		default:  
			ni->ni_uapsd_maxsp = WME_UAPSD_NODE_MAXQDEPTH;
		}
	}
	IEEE80211_NOTE(ni->ni_vap, IEEE80211_MSG_POWER, ni,
		"UAPSD bit settings from STA: %02x", ni->ni_uapsd);

	return 1;
}

static int
ieee80211_parse_wmeparams(struct ieee80211vap *vap, u_int8_t *frm,
	const struct ieee80211_frame *wh, u_int8_t *qosinfo)
{
#define	MS(_v, _f)	(((_v) & _f) >> _f##_S)
	struct ieee80211_wme_state *wme = &vap->iv_ic->ic_wme;
	u_int len = frm[1], qosinfo_count;
	int i;

	*qosinfo = 0;

	if (len < sizeof(struct ieee80211_wme_param)-2) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_WME,
			wh, "WME", "too short, len %u", len);
		return -1;
	}
	*qosinfo = frm[__offsetof(struct ieee80211_wme_param, param_qosInfo)];
	qosinfo_count = *qosinfo & WME_QOSINFO_COUNT;
	/* XXX do proper check for wraparound */
	if (qosinfo_count == wme->wme_wmeChanParams.cap_info_count)
		return 0;
	frm += __offsetof(struct ieee80211_wme_param, params_acParams);
	for (i = 0; i < WME_NUM_AC; i++) {
		struct wmeParams *wmep =
			&wme->wme_wmeChanParams.cap_wmeParams[i];
		/* NB: ACI not used */
		wmep->wmep_acm = MS(frm[0], WME_PARAM_ACM);
		wmep->wmep_aifsn = MS(frm[0], WME_PARAM_AIFSN);
		wmep->wmep_logcwmin = MS(frm[1], WME_PARAM_LOGCWMIN);
		wmep->wmep_logcwmax = MS(frm[1], WME_PARAM_LOGCWMAX);
		wmep->wmep_txopLimit = LE_READ_2(frm + 2);
		frm += 4;
	}
	wme->wme_wmeChanParams.cap_info_count = qosinfo_count;
	return 1;
#undef MS
}

static void
ieee80211_parse_athParams(struct ieee80211_node *ni, u_int8_t *ie)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
	struct ieee80211_ie_athAdvCap *athIe =
		(struct ieee80211_ie_athAdvCap *) ie;

	ni->ni_ath_flags = athIe->athAdvCap_capability;
	if (ni->ni_ath_flags & IEEE80211_ATHC_COMP)
		ni->ni_ath_defkeyindex = LE_READ_2(&athIe->athAdvCap_defKeyIndex);
#if 0
	/* NB: too noisy */
	IEEE80211_NOTE(vap, IEEE80211_MSG_SUPG, ni,
		"recv ath params: caps 0x%x flags 0x%x defkeyix %u",
		athIe->athAdvCap_capability, ni->ni_ath_flags,
		ni->ni_ath_defkeyindex);
#endif
#ifdef ATH_SUPERG_DYNTURBO
	if (IEEE80211_ATH_CAP(vap, ni, IEEE80211_ATHC_TURBOP)) {
		u_int16_t curflags, newflags;

		/*
		 * Check for turbo mode switch.  Calculate flags
		 * for the new mode and effect the switch.
		 */
		newflags = curflags = ic->ic_bsschan->ic_flags;
		/* NB: ATHC_BOOST is not in ic_ath_cap, so get it from the ie */
		if (athIe->athAdvCap_capability & IEEE80211_ATHC_BOOST) 
			newflags |= IEEE80211_CHAN_TURBO;
		else
			newflags &= ~IEEE80211_CHAN_TURBO;
		if (newflags != curflags)
			ieee80211_dturbo_switch(ic, newflags);
	}
#endif /* ATH_SUPERG_DYNTURBO */
}

void
ieee80211_saveath(struct ieee80211_node *ni, u_int8_t *ie)
{
	const struct ieee80211_ie_athAdvCap *athIe =
		(const struct ieee80211_ie_athAdvCap *) ie;

	ni->ni_ath_flags = athIe->athAdvCap_capability;
	if (ni->ni_ath_flags & IEEE80211_ATHC_COMP)
		ni->ni_ath_defkeyindex = LE_READ_2(&athIe->athAdvCap_defKeyIndex);
	ieee80211_saveie(&ni->ni_ath_ie, ie);
}

struct ieee80211_channel *
ieee80211_doth_findchan(struct ieee80211vap *vap, u_int8_t chan)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_channel *c;
	int flags, freq;

	/* NB: try first to preserve turbo */
	flags = ic->ic_bsschan->ic_flags & IEEE80211_CHAN_ALL;
	freq = ieee80211_ieee2mhz(chan, 0);
	c = ieee80211_find_channel(ic, freq, flags);
	if (c == NULL)
		c = ieee80211_find_channel(ic, freq, 0);
	return c;
}

static int
ieee80211_parse_dothparams(struct ieee80211vap *vap, u_int8_t *frm,
	const struct ieee80211_frame *wh)
{
	struct ieee80211com *ic = vap->iv_ic;
	u_int len = frm[1];
	u_int8_t chan, tbtt;

	if (len < 4 - 2) {		/* XXX ie struct definition */
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_DOTH,
			wh, "channel switch", "too short, len %u", len);
		return -1;
	}
	chan = frm[3];
	if (isclr(ic->ic_chan_avail, chan)) {
		IEEE80211_DISCARD_IE(vap,
			IEEE80211_MSG_ELEMID | IEEE80211_MSG_DOTH,
			wh, "channel switch", "invalid channel %u", chan);
		return -1;
	}
	tbtt = frm[4];
	IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH,
		"%s: channel switch to %d in %d tbtt\n", __func__, chan, tbtt);
	if (tbtt <= 1) {
		struct ieee80211_channel *c;

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH,
			"%s: Channel switch to %d NOW!\n", __func__, chan);
#if 0
		/* XXX does not belong here? */
		/* XXX doesn't stop management frames */
		/* XXX who restarts the queue? */
		/* NB: for now, error here is non-catastrophic.
		 *     in the future we may need to ensure we
		 *     stop xmit on this channel.
		 */
		netif_stop_queue(ic->ic_dev);
#endif
		if ((c = ieee80211_doth_findchan(vap, chan)) == NULL) {
			/* XXX something wrong */
			IEEE80211_DISCARD_IE(vap,
				IEEE80211_MSG_ELEMID | IEEE80211_MSG_DOTH,
				wh, "channel switch",
				"channel %u lookup failed", chan);
			return 0;
		}
		ic->ic_prevchan = ic->ic_curchan;
		ic->ic_curchan = ic->ic_bsschan = c;
		ic->ic_set_channel(ic);
		return 1;
	}
	return 0;
}

/* XXX. Not the right place for such a definition */
struct l2_update_frame {
	struct ether_header eh;
	u8 dsap;
	u8 ssap;
	u8 control;
	u8 xid[3];
}  __packed;

static void
ieee80211_deliver_l2uf(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct net_device *dev = vap->iv_dev;
	struct sk_buff *skb;
	struct l2_update_frame *l2uf;
	struct ether_header *eh;
	
	skb = dev_alloc_skb(sizeof(*l2uf));
	if (!skb) {
		printk("ieee80211_deliver_l2uf: no buf available\n");
		return;
	}
	skb_put(skb, sizeof(*l2uf));	
	l2uf = (struct l2_update_frame *)(skb->data);
	eh = &l2uf->eh;
	/* dst: Broadcast address */
	IEEE80211_ADDR_COPY(eh->ether_dhost, dev->broadcast);
	/* src: associated STA */
	IEEE80211_ADDR_COPY(eh->ether_shost, ni->ni_macaddr);
	eh->ether_type = htons(skb->len - sizeof(*eh));
	
	l2uf->dsap = 0;
	l2uf->ssap = 0;
	l2uf->control = 0xf5;
	l2uf->xid[0] = 0x81;
	l2uf->xid[1] = 0x80;
	l2uf->xid[2] = 0x00;
	
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->mac.raw = skb->data;
	ieee80211_deliver_data(ni, skb);
	return;
}

static __inline int
contbgscan(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;

	return ((ic->ic_flags_ext & IEEE80211_FEXT_BGSCAN) &&
		time_after(jiffies, ic->ic_lastdata + vap->iv_bgscanidle));
}

static __inline int
startbgscan(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;

	return ((vap->iv_flags & IEEE80211_F_BGSCAN) &&
		!IEEE80211_IS_CHAN_DTURBO(ic->ic_curchan) &&
		time_after(jiffies, ic->ic_lastscan + vap->iv_bgscanintvl) &&
		time_after(jiffies, ic->ic_lastdata + vap->iv_bgscanidle));
}


/*
 * Context: SoftIRQ
 */
void
ieee80211_recv_mgmt(struct ieee80211_node *ni, struct sk_buff *skb,
	int subtype, int rssi, u_int32_t rstamp)
{
#define	ISPROBE(_st)	((_st) == IEEE80211_FC0_SUBTYPE_PROBE_RESP)
#define	ISREASSOC(_st)	((_st) == IEEE80211_FC0_SUBTYPE_REASSOC_RESP)
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_frame *wh;
	u_int8_t *frm, *efrm;
	u_int8_t *ssid, *rates, *xrates, *wpa, *rsn, *wme, *ath;
	u_int8_t rate;
	int reassoc, resp, allocbs;
	u_int8_t qosinfo;

	wh = (struct ieee80211_frame *) skb->data;
	frm = (u_int8_t *)&wh[1];
	efrm = skb->data + skb->len;
	switch (subtype) {
	case IEEE80211_FC0_SUBTYPE_PROBE_RESP:
	case IEEE80211_FC0_SUBTYPE_BEACON: {
		struct ieee80211_scanparams scan;

		/*
		 * We process beacon/probe response frames:
		 *    o when scanning, or
		 *    o station mode when associated (to collect state
		 *      updates such as 802.11g slot time), or
		 *    o adhoc mode (to discover neighbors)
		 * Frames otherwise received are discarded.
		 */ 
		if (!((ic->ic_flags & IEEE80211_F_SCAN) ||
		    (vap->iv_opmode == IEEE80211_M_STA && ni->ni_associd) ||
		    vap->iv_opmode == IEEE80211_M_IBSS)) {
			vap->iv_stats.is_rx_mgtdiscard++;
			return;
		}
		/*
		 * beacon/probe response frame format
		 *	[8] time stamp
		 *	[2] beacon interval
		 *	[2] capability information
		 *	[tlv] ssid
		 *	[tlv] supported rates
		 *	[tlv] country information
		 *	[tlv] parameter set (FH/DS)
		 *	[tlv] erp information
		 *	[tlv] extended supported rates
		 *	[tlv] WME
		 *	[tlv] WPA or RSN
                 *      [tlv] Atheros Advanced Capabilities
		 */
		IEEE80211_VERIFY_LENGTH(efrm - frm, 12);
		memset(&scan, 0, sizeof(scan));
		scan.tstamp  = frm;
		frm += 8;
		scan.bintval = le16toh(*(u_int16_t *)frm);
		frm += 2;
		scan.capinfo = le16toh(*(u_int16_t *)frm);
		frm += 2;
		scan.bchan = ieee80211_chan2ieee(ic, ic->ic_curchan);
		scan.chan = scan.bchan;

		while (frm < efrm) {
			IEEE80211_VERIFY_LENGTH(efrm - frm, frm[1]);
			switch (*frm) {
			case IEEE80211_ELEMID_SSID:
				scan.ssid = frm;
				break;
			case IEEE80211_ELEMID_RATES:
				scan.rates = frm;
				break;
			case IEEE80211_ELEMID_COUNTRY:
				scan.country = frm;
				break;
			case IEEE80211_ELEMID_FHPARMS:
				if (ic->ic_phytype == IEEE80211_T_FH) {
					scan.fhdwell = LE_READ_2(&frm[2]);
					scan.chan = IEEE80211_FH_CHAN(frm[4], frm[5]);
					scan.fhindex = frm[6];
				}
				break;
			case IEEE80211_ELEMID_DSPARMS:
				/*
				 * XXX hack this since depending on phytype
				 * is problematic for multi-mode devices.
				 */
				if (ic->ic_phytype != IEEE80211_T_FH)
					scan.chan = frm[2];
				break;
			case IEEE80211_ELEMID_TIM:
				/* XXX ATIM? */
				scan.tim = frm;
				scan.timoff = frm - skb->data;
				break;
			case IEEE80211_ELEMID_IBSSPARMS:
				break;
			case IEEE80211_ELEMID_XRATES:
				scan.xrates = frm;
				break;
			case IEEE80211_ELEMID_ERP:
				if (frm[1] != 1) {
					IEEE80211_DISCARD_IE(vap,
						IEEE80211_MSG_ELEMID, wh, "ERP",
						"bad len %u", frm[1]);
					vap->iv_stats.is_rx_elem_toobig++;
					break;
				}
				scan.erp = frm[2];
				break;
			case IEEE80211_ELEMID_RSN:
				scan.rsn = frm;
				break;
			case IEEE80211_ELEMID_VENDOR:
				if (iswpaoui(frm))
					scan.wpa = frm;
				else if (iswmeparam(frm) || iswmeinfo(frm))
					scan.wme = frm;
				else if (isatherosoui(frm))
					scan.ath = frm;
				break;
			case IEEE80211_ELEMID_CHANSWITCHANN:
				if (ic->ic_flags & IEEE80211_F_DOTH)
					scan.doth = frm;
				break;
			default:
				IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID,
					wh, "unhandled",
					"id %u, len %u", *frm, frm[1]);
				vap->iv_stats.is_rx_elem_unknown++;
				break;
			}
			frm += frm[1] + 2;
		}
		if (frm > efrm)
			return;
		IEEE80211_VERIFY_ELEMENT(scan.rates, IEEE80211_RATE_MAXSIZE);
		IEEE80211_VERIFY_ELEMENT(scan.ssid, IEEE80211_NWID_LEN);
#if IEEE80211_CHAN_MAX < 255
		if (scan.chan > IEEE80211_CHAN_MAX) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,
				wh, ieee80211_mgt_subtype_name[subtype >>
					IEEE80211_FC0_SUBTYPE_SHIFT],
				"invalid channel %u", scan.chan);
			vap->iv_stats.is_rx_badchan++;
			return;
		}
#endif
		if (scan.chan != scan.bchan &&
		    ic->ic_phytype != IEEE80211_T_FH) {
			/*
			 * Frame was received on a channel different from the
			 * one indicated in the DS params element id;
			 * silently discard it.
			 *
			 * NB: this can happen due to signal leakage.
			 *     But we should take it for FH phy because
			 *     the rssi value should be correct even for
			 *     different hop pattern in FH.
			 */
			IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,
				wh, ieee80211_mgt_subtype_name[subtype >>
					IEEE80211_FC0_SUBTYPE_SHIFT],
				"for off-channel %u", scan.chan);
			vap->iv_stats.is_rx_chanmismatch++;
			return;
		}

		/*
		 * Count frame now that we know it's to be processed.
		 */
		if (subtype == IEEE80211_FC0_SUBTYPE_BEACON)
			IEEE80211_NODE_STAT(ni, rx_beacons);
		else
			IEEE80211_NODE_STAT(ni, rx_proberesp);

		/*
		 * When operating in station mode, check for state updates.
		 * Be careful to ignore beacons received while doing a
		 * background scan.  We consider only 11g/WMM stuff right now.
		 */
		if (vap->iv_opmode == IEEE80211_M_STA &&
		    ni->ni_associd != 0 &&
		    IEEE80211_ADDR_EQ(wh->i_addr2, ni->ni_bssid)) {
			/* record tsf of last beacon */
			memcpy(ni->ni_tstamp.data, scan.tstamp,
				sizeof(ni->ni_tstamp));
			if (ni->ni_erp != scan.erp) {
				IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
					"erp change: was 0x%x, now 0x%x",
					ni->ni_erp, scan.erp);
				if (scan.erp & IEEE80211_ERP_USE_PROTECTION)
					ic->ic_flags |= IEEE80211_F_USEPROT;
				else
					ic->ic_flags &= ~IEEE80211_F_USEPROT;
				ni->ni_erp = scan.erp;
				/* XXX statistic */
			}
			if ((ni->ni_capinfo ^ scan.capinfo) & IEEE80211_CAPINFO_SHORT_SLOTTIME) {
				IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
					"capabilities change: was 0x%x, now 0x%x",
					ni->ni_capinfo, scan.capinfo);
				/*
				 * NB: we assume short preamble doesn't
				 *     change dynamically
				 */
				ieee80211_set_shortslottime(ic,
					IEEE80211_IS_CHAN_A(ic->ic_bsschan) ||
					(ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME));
				ni->ni_capinfo = scan.capinfo;
				/* XXX statistic */
			}
			if (scan.wme != NULL &&
			    (ni->ni_flags & IEEE80211_NODE_QOS)) {
				int _retval;
				if ((_retval = ieee80211_parse_wmeparams(vap, scan.wme, wh, &qosinfo)) >= 0) {
					if (qosinfo & WME_CAPINFO_UAPSD_EN)
						ni->ni_flags |= IEEE80211_NODE_UAPSD;
					if (_retval > 0)
						ieee80211_wme_updateparams(vap);
				}
			} else
				ni->ni_flags &= ~IEEE80211_NODE_UAPSD;
			if (scan.ath != NULL)
				ieee80211_parse_athParams(ni, scan.ath);
			if (scan.doth != NULL)
				ieee80211_parse_dothparams(vap, scan.doth, wh);
			if (scan.tim != NULL) {
				/*
				 * Check the TIM. For now we drop out of
				 * power save mode for any reason.
				 */
				struct ieee80211_tim_ie *tim =
				    (struct ieee80211_tim_ie *) scan.tim;
				int aid = IEEE80211_AID(ni->ni_associd);
				int ix = aid / NBBY;
				int min = tim->tim_bitctl &~ 1;
				int max = tim->tim_len + min - 4;
				if ((tim->tim_bitctl&1) ||
				    (min <= ix && ix <= max &&
				    isset(tim->tim_bitmap - min, aid)))
					ieee80211_sta_pwrsave(vap, 0);
				vap->iv_dtim_count = tim->tim_count;
			}

			/* WDS/Repeater: re-schedule software beacon timer for STA */
			if (vap->iv_state == IEEE80211_S_RUN &&
			    vap->iv_flags_ext & IEEE80211_FEXT_SWBMISS) {
				mod_timer(&vap->iv_swbmiss, jiffies + vap->iv_swbmiss_period);
			}
			
			/*
			 * If scanning, pass the info to the scan module.
			 * Otherwise, check if it's the right time to do
			 * a background scan.  Background scanning must
			 * be enabled and we must not be operating in the
			 * turbo phase of dynamic turbo mode.  Then,
			 * it's been a while since the last background
			 * scan and if no data frames have come through
			 * recently, kick off a scan.  Note that this
			 * is the mechanism by which a background scan
			 * is started _and_ continued each time we
			 * return on-channel to receive a beacon from
			 * our ap.
			 */
			if (ic->ic_flags & IEEE80211_F_SCAN)
				ieee80211_add_scan(vap, &scan, wh,
					subtype, rssi, rstamp);
			else if (contbgscan(vap) || startbgscan(vap))
				ieee80211_bg_scan(vap);
			return;
		}
		/*
		 * If scanning, just pass information to the scan module.
		 */
		if (ic->ic_flags & IEEE80211_F_SCAN) {
			ieee80211_add_scan(vap, &scan, wh, subtype, rssi, rstamp);
			return;
		}
		if (scan.capinfo & IEEE80211_CAPINFO_IBSS) {
			if (!IEEE80211_ADDR_EQ(wh->i_addr2, ni->ni_macaddr)) {
				/*
				 * Create a new entry in the neighbor table.
				 */
				ni = ieee80211_add_neighbor(vap, wh, &scan);
			} else {
				/*
				 * Copy data from beacon to neighbor table.
				 * Some of this information might change after
				 * ieee80211_add_neighbor(), so we just copy
				 * everything over to be safe.
				 */
				ni->ni_esslen = scan.ssid[1];
				memcpy(ni->ni_essid, scan.ssid + 2, scan.ssid[1]);
				IEEE80211_ADDR_COPY(ni->ni_bssid, wh->i_addr3);
				memcpy(ni->ni_tstamp.data, scan.tstamp,
					sizeof(ni->ni_tstamp));
				ni->ni_intval = scan.bintval;
				ni->ni_capinfo = scan.capinfo;
				ni->ni_chan = ic->ic_curchan;
				ni->ni_fhdwell = scan.fhdwell;
				ni->ni_fhindex = scan.fhindex;
				ni->ni_erp = scan.erp;
				ni->ni_timoff = scan.timoff;
				if (scan.wme != NULL)
					ieee80211_saveie(&ni->ni_wme_ie, scan.wme);
				if (scan.wpa != NULL)
					ieee80211_saveie(&ni->ni_wpa_ie, scan.wpa);
				if (scan.rsn != NULL)
					ieee80211_saveie(&ni->ni_rsn_ie, scan.rsn);
				if (scan.ath != NULL)
					ieee80211_saveath(ni, scan.ath);

				/* NB: must be after ni_chan is setup */
				ieee80211_setup_rates(ni, scan.rates,
					scan.xrates, IEEE80211_F_DOSORT);
			}
			if (ni != NULL) {
				ni->ni_rssi = rssi;
				ni->ni_rstamp = rstamp;
				ni->ni_last_rx = jiffies;
			}
		}
		break;
	}

	case IEEE80211_FC0_SUBTYPE_PROBE_REQ:
		if (vap->iv_opmode == IEEE80211_M_STA ||
		    vap->iv_opmode == IEEE80211_M_AHDEMO ||
		    vap->iv_state != IEEE80211_S_RUN) {
			vap->iv_stats.is_rx_mgtdiscard++;
			return;
		}
		if (IEEE80211_IS_MULTICAST(wh->i_addr2)) {
			/* frame must be directed */
			vap->iv_stats.is_rx_mgtdiscard++;	/* XXX stat */
			return;
		}

/*
 * XR vap does not process  probe requests.
 */
#ifdef ATH_SUPERG_XR
	if(vap->iv_flags & IEEE80211_F_XR ) 
		return;
#endif
		/*
		 * prreq frame format
		 *	[tlv] ssid
		 *	[tlv] supported rates
		 *	[tlv] extended supported rates
                 *      [tlv] Atheros Advanced Capabilities
		 */
		ssid = rates = xrates = ath = NULL;
		while (frm < efrm) {
			IEEE80211_VERIFY_LENGTH(efrm - frm, frm[1]);
			switch (*frm) {
			case IEEE80211_ELEMID_SSID:
				ssid = frm;
				break;
			case IEEE80211_ELEMID_RATES:
				rates = frm;
				break;
			case IEEE80211_ELEMID_XRATES:
				xrates = frm;
				break;
			case IEEE80211_ELEMID_VENDOR:
				if (isatherosoui(frm))
					ath = frm;
				/* XXX Atheros OUI support */
				break;
			}
			frm += frm[1] + 2;
		}
		if (frm > efrm)
			return;
		IEEE80211_VERIFY_ELEMENT(rates, IEEE80211_RATE_MAXSIZE);
		IEEE80211_VERIFY_ELEMENT(ssid, IEEE80211_NWID_LEN);
		IEEE80211_VERIFY_SSID(vap->iv_bss, ssid);
		if ((vap->iv_flags & IEEE80211_F_HIDESSID) && ssid[1] == 0) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT,
				wh, ieee80211_mgt_subtype_name[subtype >>
					IEEE80211_FC0_SUBTYPE_SHIFT],
			    	"%s", "no ssid with ssid suppression enabled");
			vap->iv_stats.is_rx_ssidmismatch++; /*XXX*/
			return;
		}
		if (ni == vap->iv_bss) {
			if (vap->iv_opmode == IEEE80211_M_IBSS) {
				/*
				 * XXX Cannot tell if the sender is operating
				 * in ibss mode.  But we need a new node to
				 * send the response so blindly add them to the
				 * neighbor table.
				 */
				ni = ieee80211_fakeup_adhoc_node(vap,
					wh->i_addr2);
			} else
				ni = ieee80211_tmp_node(vap, wh->i_addr2);
			if (ni == NULL)
				return;
			allocbs = 1;
		} else
			allocbs = 0;
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_INPUT, wh->i_addr2,
			"%s", "recv probe req");
		ni->ni_rssi = rssi;
		ni->ni_rstamp = rstamp;
		ni->ni_last_rx = jiffies;
		rate = ieee80211_setup_rates(ni, rates, xrates,
			IEEE80211_F_DOSORT | IEEE80211_F_DOFRATE |
			IEEE80211_F_DONEGO | IEEE80211_F_DODEL);
		if (rate & IEEE80211_RATE_BASIC) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_XRATE,
				wh, ieee80211_mgt_subtype_name[subtype >>
					IEEE80211_FC0_SUBTYPE_SHIFT],
				"%s", "recv'd rate set invalid");
		} else {
			IEEE80211_SEND_MGMT(ni,
				IEEE80211_FC0_SUBTYPE_PROBE_RESP, 0);
		}
		if (allocbs && vap->iv_opmode != IEEE80211_M_IBSS) {
			/*
			 * Temporary node created just to send a
			 * response, reclaim immediately
			 */
			ieee80211_free_node(ni);
		} else if (ath != NULL)
			ieee80211_saveath(ni, ath);
		break;

	case IEEE80211_FC0_SUBTYPE_AUTH: {
		u_int16_t algo, seq, status;
		/*
		 * auth frame format
		 *	[2] algorithm
		 *	[2] sequence
		 *	[2] status
		 *	[tlv*] challenge
		 */
		IEEE80211_VERIFY_LENGTH(efrm - frm, 6);
		algo   = le16toh(*(u_int16_t *)frm);
		seq    = le16toh(*(u_int16_t *)(frm + 2));
		status = le16toh(*(u_int16_t *)(frm + 4));
#ifdef ATH_SUPERG_XR
		if (!IEEE80211_ADDR_EQ(wh->i_addr3, vap->iv_bss->ni_bssid)) {
			/*
			 * node roaming between XR and normal vaps. 
			 * this can only happen in AP mode. disaccociate from
			 * previous vap first.
			 */
			if (vap->iv_xrvap) {
				if (ni == vap->iv_bss)
					ni = vap->iv_xrvap->iv_bss;
				else {
					ieee80211_node_leave(ni);
					ieee80211_node_reset(ni, vap->iv_xrvap);
				}
				vap = vap->iv_xrvap;
			} else {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_AUTH,
					wh, "auth", "%s", "not to pier xr bssid");
				return;
			}
		}
#endif
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_AUTH, wh->i_addr2,
			"recv auth frame with algorithm %d seq %d", algo, seq);
		/*
		 * Consult the ACL policy module if setup.
		 */
		if (vap->iv_acl != NULL &&
		    !vap->iv_acl->iac_check(vap, wh->i_addr2)) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_ACL,
				wh, "auth", "%s", "disallowed by ACL");
			vap->iv_stats.is_rx_acl++;
			return;
		}
		if (vap->iv_flags & IEEE80211_F_COUNTERM) {
			IEEE80211_DISCARD(vap,
				IEEE80211_MSG_AUTH | IEEE80211_MSG_CRYPTO,
				wh, "auth", "%s", "TKIP countermeasures enabled");
			vap->iv_stats.is_rx_auth_countermeasures++;
			if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
				ieee80211_send_error(ni, wh->i_addr2,
					IEEE80211_FC0_SUBTYPE_AUTH,
					IEEE80211_REASON_MIC_FAILURE);
			}
			return;
		}
		if (algo == IEEE80211_AUTH_ALG_SHARED)
			ieee80211_auth_shared(ni, wh, frm + 6, efrm, rssi,
				rstamp, seq, status);
		else if (algo == IEEE80211_AUTH_ALG_OPEN)
			ieee80211_auth_open(ni, wh, rssi, rstamp, seq, status);
		else {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
				wh, "auth", "unsupported alg %d", algo);
			vap->iv_stats.is_rx_auth_unsupported++;
			if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
				/* XXX not right */
				ieee80211_send_error(ni, wh->i_addr2,
					IEEE80211_FC0_SUBTYPE_AUTH,
					(seq+1) | (IEEE80211_STATUS_ALG << 16));
			}
			return;
		} 
		break;
	}

	case IEEE80211_FC0_SUBTYPE_ASSOC_REQ:
	case IEEE80211_FC0_SUBTYPE_REASSOC_REQ: {
		u_int16_t capinfo, bintval;
		struct ieee80211_rsnparms rsn_parm;
		u_int8_t reason;

		if (vap->iv_opmode != IEEE80211_M_HOSTAP ||
		    vap->iv_state != IEEE80211_S_RUN) {
			vap->iv_stats.is_rx_mgtdiscard++;
			return;
		}

		if (subtype == IEEE80211_FC0_SUBTYPE_REASSOC_REQ) {
			reassoc = 1;
			resp = IEEE80211_FC0_SUBTYPE_REASSOC_RESP;
		} else {
			reassoc = 0;
			resp = IEEE80211_FC0_SUBTYPE_ASSOC_RESP;
		}

		/*
		 * asreq frame format
		 *	[2] capability information
		 *	[2] listen interval
		 *	[6*] current AP address (reassoc only)
		 *	[tlv] ssid
		 *	[tlv] supported rates
		 *	[tlv] extended supported rates
		 *	[tlv] wpa or RSN
		 *      [tlv] WME
		 *	[tlv] Atheros Advanced Capabilities
		 */
		IEEE80211_VERIFY_LENGTH(efrm - frm, (reassoc ? 10 : 4));
		if (!IEEE80211_ADDR_EQ(wh->i_addr3, vap->iv_bss->ni_bssid)) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
				wh, ieee80211_mgt_subtype_name[subtype >>
					IEEE80211_FC0_SUBTYPE_SHIFT],
				"%s", "wrong bssid");
			vap->iv_stats.is_rx_assoc_bss++;
			return;
		}
		capinfo = le16toh(*(u_int16_t *)frm);
		frm += 2;
		bintval = le16toh(*(u_int16_t *)frm);
		frm += 2;
		if (reassoc)
			frm += 6;	/* ignore current AP info */
		ssid = rates = xrates = wpa = rsn = wme = ath = NULL;
		while (frm < efrm) {
			IEEE80211_VERIFY_LENGTH(efrm - frm, frm[1]);
			switch (*frm) {
			case IEEE80211_ELEMID_SSID:
				ssid = frm;
				break;
			case IEEE80211_ELEMID_RATES:
				rates = frm;
				break;
			case IEEE80211_ELEMID_XRATES:
				xrates = frm;
				break;
			/* XXX verify only one of RSN and WPA ie's? */
			case IEEE80211_ELEMID_RSN:
				if (vap->iv_flags & IEEE80211_F_WPA2)
					rsn = frm;
				else
					IEEE80211_DPRINTF(vap,
						IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA,
						"[%s] ignoring RSN IE in association request\n",
						ether_sprintf(wh->i_addr2));
				break;
			case IEEE80211_ELEMID_VENDOR:
				/* don't override RSN element
				 * XXX: actually the driver should report both WPA versions,
				 * so wpa_supplicant can choose and also detect downgrade attacks
                                 */
				if (iswpaoui(frm) && !wpa) {
					if (vap->iv_flags & IEEE80211_F_WPA1)
						wpa = frm;
					else
						IEEE80211_DPRINTF(vap,
							IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA,
							"[%s] ignoring WPA IE in association request\n",
							ether_sprintf(wh->i_addr2));
				} else if (iswmeinfo(frm))
					wme = frm;
				else if (isatherosoui(frm))
					ath = frm;
				break;
			}
			frm += frm[1] + 2;
		}
		if (frm > efrm)
			return;
		IEEE80211_VERIFY_ELEMENT(rates, IEEE80211_RATE_MAXSIZE);
		IEEE80211_VERIFY_ELEMENT(ssid, IEEE80211_NWID_LEN);
		IEEE80211_VERIFY_SSID(vap->iv_bss, ssid);

		if (ni == vap->iv_bss) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, wh->i_addr2,
				"deny %s request, sta not authenticated",
				reassoc ? "reassoc" : "assoc");
			ieee80211_send_error(ni, wh->i_addr2,
				IEEE80211_FC0_SUBTYPE_DEAUTH,
				IEEE80211_REASON_ASSOC_NOT_AUTHED);
			vap->iv_stats.is_rx_assoc_notauth++;
			return;
		}
		
		/* Assert right associstion security credentials */
		/* XXX Divy. Incomplete */
		if (wpa == NULL && (ic->ic_flags & IEEE80211_F_WPA)) {
			IEEE80211_DPRINTF(vap,
				IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA,
				"[%s] no WPA/RSN IE in association request\n",
				ether_sprintf(wh->i_addr2));
			IEEE80211_SEND_MGMT(ni,
				IEEE80211_FC0_SUBTYPE_DEAUTH,
				IEEE80211_REASON_RSN_REQUIRED);
			ieee80211_node_leave(ni);
			/* XXX distinguish WPA/RSN? */
			vap->iv_stats.is_rx_assoc_badwpaie++;
			return;	
		}
				
		if (rsn != NULL) {
			/*
			 * Parse WPA information element.  Note that
			 * we initialize the param block from the node
			 * state so that information in the IE overrides
			 * our defaults.  The resulting parameters are
			 * installed below after the association is assured.
			 */
			rsn_parm = ni->ni_rsn;
			if (rsn[0] != IEEE80211_ELEMID_RSN)
				reason = ieee80211_parse_wpa(vap, rsn, &rsn_parm, wh);
			else
				reason = ieee80211_parse_rsn(vap, rsn, &rsn_parm, wh);
			if (reason != 0) {
				IEEE80211_SEND_MGMT(ni,
					IEEE80211_FC0_SUBTYPE_DEAUTH, reason);
				ieee80211_node_leave(ni);
				/* XXX distinguish WPA/RSN? */
				vap->iv_stats.is_rx_assoc_badwpaie++;
				return;
			}
			IEEE80211_NOTE_MAC(vap,
				IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA,
				wh->i_addr2,
				"%s ie: mc %u/%u uc %u/%u key %u caps 0x%x",
				rsn[0] != IEEE80211_ELEMID_RSN ?  "WPA" : "RSN",
				rsn_parm.rsn_mcastcipher, rsn_parm.rsn_mcastkeylen,
				rsn_parm.rsn_ucastcipher, rsn_parm.rsn_ucastkeylen,
				rsn_parm.rsn_keymgmt, rsn_parm.rsn_caps);
		}
		/* discard challenge after association */
		if (ni->ni_challenge != NULL) {
			FREE(ni->ni_challenge, M_DEVBUF);
			ni->ni_challenge = NULL;
		}
		/* 802.11 spec says to ignore station's privacy bit */
		if ((capinfo & IEEE80211_CAPINFO_ESS) == 0) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, wh->i_addr2,
				"deny %s request, capability mismatch 0x%x",
				reassoc ? "reassoc" : "assoc", capinfo);
			IEEE80211_SEND_MGMT(ni, resp, IEEE80211_STATUS_CAPINFO);
			ieee80211_node_leave(ni);
			vap->iv_stats.is_rx_assoc_capmismatch++;
			return;
		}
		rate = ieee80211_setup_rates(ni, rates, xrates,
			IEEE80211_F_DOSORT | IEEE80211_F_DOFRATE |
			IEEE80211_F_DONEGO | IEEE80211_F_DODEL);
		/*
		 * If constrained to 11g-only stations reject an
		 * 11b-only station.  We cheat a bit here by looking
		 * at the max negotiated xmit rate and assuming anyone
		 * with a best rate <24Mb/s is an 11b station.
		 */
		if ((rate & IEEE80211_RATE_BASIC) ||
		    ((vap->iv_flags & IEEE80211_F_PUREG) && rate < 48)) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, wh->i_addr2,
				"deny %s request, rate set mismatch",
				reassoc ? "reassoc" : "assoc");
			IEEE80211_SEND_MGMT(ni, resp,
				IEEE80211_STATUS_BASIC_RATE);
			ieee80211_node_leave(ni);
			vap->iv_stats.is_rx_assoc_norate++;
			return;
		}

		if (ni->ni_associd != 0 && 
		    IEEE80211_IS_CHAN_ANYG(ic->ic_bsschan)) {
			if ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME)
			    != (capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME)) {
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, 
					wh->i_addr2,
				    	"deny %s request, short slot time "
					"capability mismatch 0x%x",
				    	reassoc ? "reassoc" : "assoc", capinfo);
				IEEE80211_SEND_MGMT(ni, resp, 
					IEEE80211_STATUS_CAPINFO);
				ieee80211_node_leave(ni);
				vap->iv_stats.is_rx_assoc_capmismatch++;
				return;
			}
		}

		ni->ni_rssi = rssi;
		ni->ni_rstamp = rstamp;
		ni->ni_last_rx = jiffies;
		ni->ni_intval = bintval;
		ni->ni_capinfo = capinfo;
		ni->ni_chan = ic->ic_curchan;
		ni->ni_fhdwell = vap->iv_bss->ni_fhdwell;
		ni->ni_fhindex = vap->iv_bss->ni_fhindex;
		if (wpa != NULL) {
			/*
			 * Record WPA/RSN parameters for station, mark
			 * node as using WPA and record information element
			 * for applications that require it.
			 */
			ieee80211_saveie(&ni->ni_wpa_ie, wpa);
		} else if (ni->ni_wpa_ie != NULL) {
			/*
			 * Flush any state from a previous association.
			 */
			FREE(ni->ni_wpa_ie, M_DEVBUF);
			ni->ni_wpa_ie = NULL;
		}
		if (rsn != NULL) {
			/*
			 * Record WPA/RSN parameters for station, mark
			 * node as using WPA and record information element
			 * for applications that require it.
			 */
			ni->ni_rsn = rsn_parm;
			ieee80211_saveie(&ni->ni_rsn_ie, rsn);
		} else if (ni->ni_rsn_ie != NULL) {
			/*
			 * Flush any state from a previous association.
			 */
			FREE(ni->ni_rsn_ie, M_DEVBUF);
			ni->ni_rsn_ie = NULL;
		}
		if (wme != NULL) {
			/*
			 * Record WME parameters for station, mark node
			 * as capable of QoS and record information
			 * element for applications that require it.
			 */
			ieee80211_saveie(&ni->ni_wme_ie, wme);
			if (ieee80211_parse_wmeie(wme, wh, ni) > 0)
				ni->ni_flags |= IEEE80211_NODE_QOS;
		} else if (ni->ni_wme_ie != NULL) {
			/*
			 * Flush any state from a previous association.
			 */
			FREE(ni->ni_wme_ie, M_DEVBUF);
			ni->ni_wme_ie = NULL;
			ni->ni_flags &= ~IEEE80211_NODE_QOS;
		}
		if (ath != NULL)
			ieee80211_saveath(ni, ath);
		else if (ni->ni_ath_ie != NULL) {
			/*
			 * Flush any state from a previous association.
			 */
			FREE(ni->ni_ath_ie, M_DEVBUF);
			ni->ni_ath_ie = NULL;
			ni->ni_ath_flags = 0;
		}

		/* Send TGf L2UF frame on behalf of newly associated station */
		ieee80211_deliver_l2uf(ni);
		ieee80211_node_join(ni, resp);
#ifdef ATH_SUPERG_XR
		if (ni->ni_prev_vap &&
		    ni->ni_vap != ni->ni_prev_vap &&
		    ni->ni_vap->iv_ath_cap & IEEE80211_ATHC_XR) {
			/* 
			 * node moved between XR and normal vap.
			 * move the data between  XR and normal vap.
			 */
			ic->ic_node_move_data(ni);
			ni->ni_prev_vap = ni->ni_vap;
		}
#endif
		break;
	}

	case IEEE80211_FC0_SUBTYPE_ASSOC_RESP:
	case IEEE80211_FC0_SUBTYPE_REASSOC_RESP: {
		u_int16_t capinfo, associd;
		u_int16_t status;

		if (vap->iv_opmode != IEEE80211_M_STA ||
		    vap->iv_state != IEEE80211_S_ASSOC) {
			vap->iv_stats.is_rx_mgtdiscard++;
			return;
		}

		/*
		 * asresp frame format
		 *	[2] capability information
		 *	[2] status
		 *	[2] association ID
		 *	[tlv] supported rates
		 *	[tlv] extended supported rates
		 *	[tlv] WME
		 */
		IEEE80211_VERIFY_LENGTH(efrm - frm, 6);
		ni = vap->iv_bss;
		capinfo = le16toh(*(u_int16_t *)frm);
		frm += 2;
		status = le16toh(*(u_int16_t *)frm);
		frm += 2;
		if (status != 0) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC,
				wh->i_addr2,
				"%sassoc failed (reason %d)",
				ISREASSOC(subtype) ?  "re" : "", status);
			vap->iv_stats.is_rx_auth_fail++;	/* XXX */
			ieee80211_new_state(vap, IEEE80211_S_SCAN,
				IEEE80211_SCAN_FAIL_STATUS);
			return;
		}
		associd = le16toh(*(u_int16_t *)frm);
		frm += 2;

		rates = xrates = wme = NULL;
		while (frm < efrm) {
			IEEE80211_VERIFY_LENGTH(efrm - frm, frm[1]);
			switch (*frm) {
			case IEEE80211_ELEMID_RATES:
				rates = frm;
				break;
			case IEEE80211_ELEMID_XRATES:
				xrates = frm;
				break;
			case IEEE80211_ELEMID_VENDOR:
				if (iswmeoui(frm))
					wme = frm;
				break;
			}
			frm += frm[1] + 2;
		}
		if (frm > efrm)
			return;
		IEEE80211_VERIFY_ELEMENT(rates, IEEE80211_RATE_MAXSIZE);
		rate = ieee80211_setup_rates(ni, rates, xrates,
			IEEE80211_F_DOSORT | IEEE80211_F_DOFRATE |
			IEEE80211_F_DONEGO | IEEE80211_F_DODEL);
		if (rate & IEEE80211_RATE_BASIC) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC,
				wh->i_addr2,
			 	"%sassoc failed (rate set mismatch)",
				ISREASSOC(subtype) ?  "re" : "");
			vap->iv_stats.is_rx_assoc_norate++;
			ieee80211_new_state(vap, IEEE80211_S_SCAN,
				IEEE80211_SCAN_FAIL_STATUS);
			return;
		}

		ni->ni_capinfo = capinfo;
		ni->ni_associd = associd;
		if (wme != NULL &&
		    ieee80211_parse_wmeparams(vap, wme, wh, &qosinfo) >= 0) {
			ni->ni_flags |= IEEE80211_NODE_QOS;
			ieee80211_wme_updateparams(vap);
		} else
			ni->ni_flags &= ~IEEE80211_NODE_QOS;
		/*
		 * Configure state now that we are associated.
		 *
		 * XXX may need different/additional driver callbacks?
		 */
		if (IEEE80211_IS_CHAN_A(ic->ic_curchan) ||
		    (ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_PREAMBLE)) {
			ic->ic_flags |= IEEE80211_F_SHPREAMBLE;
			ic->ic_flags &= ~IEEE80211_F_USEBARKER;
		} else {
			ic->ic_flags &= ~IEEE80211_F_SHPREAMBLE;
			ic->ic_flags |= IEEE80211_F_USEBARKER;
		}
		ieee80211_set_shortslottime(ic,
			IEEE80211_IS_CHAN_A(ic->ic_curchan) ||
				(ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME));
		/*
		 * Honor ERP protection.
		 *
		 * NB: ni_erp should zero for non-11g operation
		 *     but we check the channel characteristics
		 *     just in case.
		 */
		if (IEEE80211_IS_CHAN_ANYG(ic->ic_curchan) &&
		    (ni->ni_erp & IEEE80211_ERP_USE_PROTECTION))
			ic->ic_flags |= IEEE80211_F_USEPROT;
		else
			ic->ic_flags &= ~IEEE80211_F_USEPROT;
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC, wh->i_addr2,
			"%sassoc success: %s preamble, %s slot time%s%s%s%s%s%s%s",
			ISREASSOC(subtype) ? "re" : "",
		 	ic->ic_flags&IEEE80211_F_SHPREAMBLE ? "short" : "long",
			ic->ic_flags&IEEE80211_F_SHSLOT ? "short" : "long",
			ic->ic_flags&IEEE80211_F_USEPROT ? ", protection" : "",
			ni->ni_flags & IEEE80211_NODE_QOS ? ", QoS" : "",
			IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_TURBOP) ?
				", turbo" : "",
			IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_COMP) ?
				", compression" : "",
			IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_FF) ?
				", fast-frames" : "",
			IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_XR) ?
				", XR" : "",
			IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_AR) ?
				", AR" : ""
		);
		ieee80211_new_state(vap, IEEE80211_S_RUN, subtype);
		break;
	}

	case IEEE80211_FC0_SUBTYPE_DEAUTH: {
		u_int16_t reason;

		if (vap->iv_state == IEEE80211_S_SCAN) {
			vap->iv_stats.is_rx_mgtdiscard++;
			return;
		}
		/*
		 * deauth frame format
		 *	[2] reason
		 */
		IEEE80211_VERIFY_LENGTH(efrm - frm, 2);
		reason = le16toh(*(u_int16_t *)frm);
		vap->iv_stats.is_rx_deauth++;
		IEEE80211_NODE_STAT(ni, rx_deauth);

		IEEE80211_NOTE(vap, IEEE80211_MSG_AUTH, ni,
			"recv deauthenticate (reason %d)", reason);
		switch (vap->iv_opmode) {
		case IEEE80211_M_STA:
			ieee80211_new_state(vap, IEEE80211_S_AUTH,
				wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK);
			break;
		case IEEE80211_M_HOSTAP:
			if (ni != vap->iv_bss)
				ieee80211_node_leave(ni);
			break;
		default:
			vap->iv_stats.is_rx_mgtdiscard++;
			break;
		}
		break;
	}

	case IEEE80211_FC0_SUBTYPE_DISASSOC: {
		u_int16_t reason;

		if (vap->iv_state != IEEE80211_S_RUN &&
		    vap->iv_state != IEEE80211_S_ASSOC &&
		    vap->iv_state != IEEE80211_S_AUTH) {
			vap->iv_stats.is_rx_mgtdiscard++;
			return;
		}
		/*
		 * disassoc frame format
		 *	[2] reason
		 */
		IEEE80211_VERIFY_LENGTH(efrm - frm, 2);
		reason = le16toh(*(u_int16_t *)frm);
		vap->iv_stats.is_rx_disassoc++;
		IEEE80211_NODE_STAT(ni, rx_disassoc);

		IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
			"recv disassociate (reason %d)", reason);
		switch (vap->iv_opmode) {
		case IEEE80211_M_STA:
			ieee80211_new_state(vap, IEEE80211_S_ASSOC, 0);
			break;
		case IEEE80211_M_HOSTAP:
			if (ni != vap->iv_bss)
				ieee80211_node_leave(ni);
			break;
		default:
			vap->iv_stats.is_rx_mgtdiscard++;
			break;
		}
		break;
	}
	default:
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY,
			wh, "mgt", "subtype 0x%x not handled", subtype);
		vap->iv_stats.is_rx_badsubtype++;
		break;
	}
#undef ISREASSOC
#undef ISPROBE
}
#undef IEEE80211_VERIFY_LENGTH
#undef IEEE80211_VERIFY_ELEMENT

/*
 * Process a received ps-poll frame.
 */
static void
ieee80211_recv_pspoll(struct ieee80211_node *ni, struct sk_buff *skb0)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211_frame_min *wh;
	struct sk_buff *skb;
	u_int16_t aid;
	int qlen;

	wh = (struct ieee80211_frame_min *)skb0->data;
	if (ni->ni_associd == 0) {
		IEEE80211_DISCARD(vap,
			IEEE80211_MSG_POWER | IEEE80211_MSG_DEBUG,
			(struct ieee80211_frame *) wh, "ps-poll",
			"%s", "unassociated station");
		vap->iv_stats.is_ps_unassoc++;
		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH,
			IEEE80211_REASON_NOT_ASSOCED);
		return;
	}

	aid = le16toh(*(u_int16_t *)wh->i_dur);
	if (aid != ni->ni_associd) {
		IEEE80211_DISCARD(vap,
			IEEE80211_MSG_POWER | IEEE80211_MSG_DEBUG,
			(struct ieee80211_frame *) wh, "ps-poll",
			"aid mismatch: sta aid 0x%x poll aid 0x%x",
			ni->ni_associd, aid);
		vap->iv_stats.is_ps_badaid++;
		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH,
			IEEE80211_REASON_NOT_ASSOCED);
		return;
	}

	/* Okay, take the first queued packet and put it out... */
	IEEE80211_NODE_SAVEQ_LOCK(ni);
	IEEE80211_NODE_SAVEQ_DEQUEUE(ni, skb, qlen);
	IEEE80211_NODE_SAVEQ_UNLOCK(ni);
	if (skb == NULL) {
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_POWER, wh->i_addr2,
			"%s", "recv ps-poll, but queue empty");
		ieee80211_send_nulldata(ieee80211_ref_node(ni));
		vap->iv_stats.is_ps_qempty++;	/* XXX node stat */
		if (vap->iv_set_tim != NULL)
			vap->iv_set_tim(ni, 0);		/* just in case */
		return;
	}
	/* 
	 * If there are more packets, set the more packets bit
	 * in the packet dispatched to the station; otherwise
	 * turn off the TIM bit.
	 */
	if (qlen != 0) {
		IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
			"recv ps-poll, send packet, %u still queued", qlen);
		/*
		 * NB: More-data bit will be set during encap.
		 */
	} else {
		IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
			"%s", "recv ps-poll, send packet, queue empty");
		if (vap->iv_set_tim != NULL)
			vap->iv_set_tim(ni, 0);
	}
	M_PWR_SAV_SET(skb);		/* ensure MORE_DATA bit is set correctly */

 	ieee80211_parent_queue_xmit(skb);	/* Submit to parent device, including updating stats */
}

#ifdef ATH_SUPERG_FF
static void
athff_decap(struct sk_buff *skb)
{
	struct ether_header eh_src, *eh_dst;
	struct llc *llc;

	memcpy(&eh_src, skb->data, sizeof(struct ether_header));
	llc = (struct llc *) skb_pull(skb, sizeof(struct ether_header));
	eh_src.ether_type = llc->llc_un.type_snap.ether_type;
	skb_pull(skb, LLC_SNAPFRAMELEN);

	eh_dst = (struct ether_header *) skb_push(skb, sizeof(struct ether_header));
	memcpy(eh_dst, &eh_src, sizeof(struct ether_header));
}
#endif

#ifdef USE_HEADERLEN_RESV
/*
 * The kernel version of this function alters the skb in a manner
 * inconsistent with dev->hard_header_len header reservation. This 
 * is a rewrite of the portion of eth_type_trans() that we need.
 */
static unsigned short 
ath_eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	struct ethhdr *eth;
	
	skb->mac.raw=skb->data;
	skb_pull(skb, ETH_HLEN);
	/*
	 * NB: mac.ethernet is replaced in 2.6.9 by eth_hdr but
	 *     since that's an inline and not a define there's
	 *     no easy way to do this cleanly.
	 */
	eth = (struct ethhdr *)skb->mac.raw;
	
	if (*eth->h_dest & 1)
		if (memcmp(eth->h_dest, dev->broadcast, ETH_ALEN) == 0)
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	else
		if (memcmp(eth->h_dest, dev->dev_addr, ETH_ALEN))
			skb->pkt_type = PACKET_OTHERHOST;

	return eth->h_proto;
}
#endif

#ifdef IEEE80211_DEBUG
/*
 * Debugging support.
 */

/*
 * Return the bssid of a frame.
 */
static const u_int8_t *
ieee80211_getbssid(struct ieee80211vap *vap, const struct ieee80211_frame *wh)
{
	if (vap->iv_opmode == IEEE80211_M_STA)
		return wh->i_addr2;
	if ((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) != IEEE80211_FC1_DIR_NODS)
		return wh->i_addr1;
	if ((wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_PS_POLL)
		return wh->i_addr1;
	return wh->i_addr3;
}

void
ieee80211_note(struct ieee80211vap *vap, const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printk("%s: %s", vap->iv_dev->name, buf);	/* NB: no \n */
}
EXPORT_SYMBOL(ieee80211_note);

void
ieee80211_note_frame(struct ieee80211vap *vap, const struct ieee80211_frame *wh,
	const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printk("%s: [%s] %s\n", vap->iv_dev->name,
		ether_sprintf(ieee80211_getbssid(vap, wh)), buf);
}
EXPORT_SYMBOL(ieee80211_note_frame);

void
ieee80211_note_mac(struct ieee80211vap *vap, const u_int8_t mac[IEEE80211_ADDR_LEN],
	const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printk("%s: [%s] %s\n", vap->iv_dev->name, ether_sprintf(mac), buf);
}
EXPORT_SYMBOL(ieee80211_note_mac);

static void
ieee80211_discard_frame(struct ieee80211vap *vap, const struct ieee80211_frame *wh,
	const char *type, const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (type != NULL)
		printk("[%s:%s] discard %s frame, %s\n", vap->iv_dev->name,
			ether_sprintf(ieee80211_getbssid(vap, wh)), type, buf);
	else
		printk("[%s:%s] discard frame, %s\n", vap->iv_dev->name,
			ether_sprintf(ieee80211_getbssid(vap, wh)), buf);
}

static void
ieee80211_discard_ie(struct ieee80211vap *vap, const struct ieee80211_frame *wh,
	const char *type, const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (type != NULL)
		printk("[%s:%s] discard %s information element, %s\n",
			vap->iv_dev->name,
			ether_sprintf(ieee80211_getbssid(vap, wh)), type, buf);
	else
		printk("[%s:%s] discard information element, %s\n",
			vap->iv_dev->name,
			ether_sprintf(ieee80211_getbssid(vap, wh)), buf);
}

static void
ieee80211_discard_mac(struct ieee80211vap *vap, const u_int8_t mac[IEEE80211_ADDR_LEN],
	const char *type, const char *fmt, ...)
{
	char buf[128];		/* XXX */
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (type != NULL)
		printk("[%s:%s] discard %s frame, %s\n", vap->iv_dev->name,
			ether_sprintf(mac), type, buf);
	else
		printk("[%s:%s] discard frame, %s\n", vap->iv_dev->name,
			ether_sprintf(mac), buf);
}
#endif /* IEEE80211_DEBUG */
