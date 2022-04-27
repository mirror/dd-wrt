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
 * $Id: ieee80211_input.c 3304 2008-01-29 11:18:15Z mentor $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 input handling.
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/if_vlan.h>
#include <net/iw_handler.h>	/* wireless_send_event(..) */
#include <linux/wireless.h>	/* SIOCGIWTHRSPY */
#include <linux/if_arp.h>	/* ARPHRD_ETHER */

#include <net80211/if_llc.h>
#include <net80211/if_ethersubr.h>
#include <net80211/if_media.h>
#include <net80211/if_athproto.h>

#include <net80211/ieee80211_var.h>

#ifdef IEEE80211_DEBUG

#define BUF_LEN 192
/*
 * Decide if a received management frame should be
 * printed when debugging is enabled.  This filters some
 * of the less interesting frames that come frequently
 * (e.g. beacons).
 */
static __inline int doprint(struct ieee80211vap *vap, int subtype)
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

static const u_int8_t *ieee80211_getbssid(struct ieee80211vap *, const struct ieee80211_frame *);
static void ieee80211_discard_frame(struct ieee80211vap *, const struct ieee80211_frame *, const char *, const char *, ...);
static void ieee80211_discard_ie(struct ieee80211vap *, const struct ieee80211_frame *, const char *, const char *, ...);
static void ieee80211_discard_mac(struct ieee80211vap *, const u_int8_t mac[IEEE80211_ADDR_LEN], const char *, const char *, ...);
#else
#define	IEEE80211_DISCARD(_vap, _m, _wh, _type, _fmt, ...)
#define	IEEE80211_DISCARD_IE(_vap, _m, _wh, _type, _fmt, ...)
#define	IEEE80211_DISCARD_MAC(_vap, _m, _mac, _type, _fmt, ...)
#endif				/* IEEE80211_DEBUG */

static struct sk_buff *ieee80211_defrag(struct ieee80211_node *, struct sk_buff *, int);
static void ieee80211_deliver_data(struct ieee80211_node *, struct sk_buff *);
static struct sk_buff *ieee80211_decap(struct ieee80211vap *, struct sk_buff *, int);
static void ieee80211_send_error(struct ieee80211_node *, const u_int8_t *, int, int);
static void ieee80211_recv_pspoll(struct ieee80211_node *, struct sk_buff *);
static int accept_data_frame(struct ieee80211vap *, struct ieee80211_node *, struct ieee80211_key *, struct sk_buff *, struct ether_header *);

#ifdef ATH_SUPERG_FF
static int athff_decap(struct sk_buff *);
#endif
#ifdef USE_HEADERLEN_RESV
static __be16 ath_eth_type_trans(struct sk_buff *, struct net_device *);
#endif

#if WIRELESS_EXT >= 16
/**
 * Given a node and the RSSI value of a just received frame from the node, this
 * function checks if to raise an iwspy event because we iwspy the node and RSSI
 * exceeds threshold (if active).
 * 
 * @param vap: VAP
 * @param ni: sender node
 * @param rssi: RSSI value of received frame
 */
static void iwspy_event(struct ieee80211vap *vap, struct ieee80211_node *ni, u_int rssi)
{
	if (vap->iv_spy.thr_low && vap->iv_spy.num && ni && (rssi < vap->iv_spy.thr_low || rssi > vap->iv_spy.thr_high)) {
		int i;
		for (i = 0; i < vap->iv_spy.num; i++) {
			if (IEEE80211_ADDR_EQ(ni->ni_macaddr, &(vap->iv_spy.mac[i * IEEE80211_ADDR_LEN]))) {

				union iwreq_data wrq;
				struct iw_thrspy thr;
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_DEBUG, "%s: we spy " MAC_FMT ", threshold is active and rssi exceeds" " it -> raise an iwspy event\n", __func__, MAC_ADDR(ni->ni_macaddr));
				memset(&wrq, 0, sizeof(wrq));
				wrq.data.length = 1;
				memset(&thr, 0, sizeof(struct iw_thrspy));
				memcpy(thr.addr.sa_data, ni->ni_macaddr, IEEE80211_ADDR_LEN);
				thr.addr.sa_family = ARPHRD_ETHER;
				set_quality(&thr.qual, rssi, vap->iv_ic->ic_channoise);
				set_quality(&thr.low, vap->iv_spy.thr_low, vap->iv_ic->ic_channoise);
				set_quality(&thr.high, vap->iv_spy.thr_high, vap->iv_ic->ic_channoise);
				wireless_send_event(vap->iv_dev, SIOCGIWTHRSPY, &wrq, (char *)&thr);
				break;
			}
		}
	}
}

#else
#define iwspy_event(_vap, _ni, _rssi)
#endif				/* WIRELESS_EXT >= 16 */

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
int ieee80211_input(struct ieee80211vap *vap, struct ieee80211_node *ni_or_null, struct sk_buff *skb, int rssi, u_int64_t rtsf)
{
#define	HAS_SEQ(type)	((type & 0x4) == 0)
	struct ieee80211_node *ni = ni_or_null;
	struct ieee80211com *ic;
	struct net_device *dev;
	struct ieee80211_node *ni_wds = NULL;
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
	u_int8_t mac_to_send[IEEE80211_ADDR_LEN];
	uint32_t ctstimeout;

	type = -1;		/* undefined */

	if (!vap || !vap->iv_bss || !vap->iv_dev || !vap->iv_ic)
		goto discard;

	ic = vap->iv_ic;
	dev = vap->iv_dev;

	if ((vap->iv_dev->flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
		goto discard;

	if ((vap->iv_opmode == IEEE80211_M_STA) && (vap->iv_dev->flags & IFF_RUNNING) && (IEEE80211_NODE_SAVEQ_QLEN(vap->iv_bss) > 0) && ((ni->ni_flags & IEEE80211_NODE_PWR_MGT) == 0))
		ieee80211_sta_pwrsave(vap, 0);

	/* initialize ni as in the previous API */
	if (ni_or_null == NULL) {
		/* This function does not 'own' vap->iv_bss, so we cannot
		 * guarantee its existence during the following call, hence
		 * briefly grab our own reference. */
		ni = ieee80211_ref_node(vap->iv_bss);
		KASSERT(ni != NULL, ("null node"));
	} else {
		ni->ni_inact = ni->ni_inact_reload;
	}

	KASSERT(skb->len >= sizeof(struct ieee80211_frame_min), ("frame length too short: %u", skb->len));

	/* XXX adjust device in sk_buff? */

	/*
	 * In monitor mode, send everything directly to bpf.
	 * Also do not process frames w/o i_addr2 any further.
	 * XXX may want to include the CRC
	 */
	if (vap->iv_opmode == IEEE80211_M_MONITOR)
		goto out;

	if (!skb->data)
		goto out;

	if (skb->len < sizeof(struct ieee80211_frame_min)) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY, ni->ni_macaddr, NULL, "too short (1): len %u", skb->len);
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

	if ((wh->i_fc[0] & IEEE80211_FC0_VERSION_MASK) != IEEE80211_FC0_VERSION_0) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY, ni->ni_macaddr, NULL, "wrong version %x", wh->i_fc[0]);
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
			if (!IEEE80211_ADDR_EQ(bssid, vap->iv_bssid)) {
				/* not interested in */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, bssid, NULL, "%s", "not to bss");
				vap->iv_stats.is_rx_wrongbss++;
				goto out;
			}
			iwspy_event(vap, ni, rssi);
			break;
		case IEEE80211_M_IBSS:
		case IEEE80211_M_AHDEMO:
			if (!IEEE80211_ADDR_EQ(wh->i_addr3, vap->iv_bssid) || (!IEEE80211_ADDR_EQ(wh->i_addr1, vap->iv_myaddr) && !IEEE80211_IS_MULTICAST(wh->i_addr1) && (subtype != IEEE80211_FC0_SUBTYPE_BEACON))) {
				if (!(vap->iv_dev->flags & IFF_PROMISC)) {
					IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, bssid, NULL, "%s", "not to bss");
					vap->iv_stats.is_rx_wrongbss++;
					goto out;
				}
			}
			if (dir != IEEE80211_FC1_DIR_NODS)
				bssid = wh->i_addr1;
			else if (type == IEEE80211_FC0_TYPE_CTL)
				bssid = wh->i_addr1;
			else {
				if (skb->len < sizeof(struct ieee80211_frame)) {
					IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY, ni->ni_macaddr, NULL, "too short (2): len %u", skb->len);
					vap->iv_stats.is_rx_tooshort++;
					goto out;
				}
				bssid = wh->i_addr3;
			}
			/* Do not try to find a node reference if the packet really did come from the BSS */
			if (type == IEEE80211_FC0_TYPE_DATA && ni == vap->iv_bss && IEEE80211_ADDR_EQ(vap->iv_bssid, wh->i_addr3)) {
				/* Try to find sender in local node table. */
				if (!ni_or_null) {
					ieee80211_unref_node(&ni);
					ni = ieee80211_find_txnode(vap, wh->i_addr2);
				}
				if (ni == NULL) {
					/* NB: stat kept for alloc failure */
					goto discard;
				}
			}
			iwspy_event(vap, ni, rssi);
			break;
		case IEEE80211_M_HOSTAP:
//                      printk(KERN_INFO "process hostapd\n");
			if (dir != IEEE80211_FC1_DIR_NODS) {
				bssid = wh->i_addr1;
			} else if (type == IEEE80211_FC0_TYPE_CTL) {
				bssid = wh->i_addr1;
			} else {
				if (skb->len < sizeof(struct ieee80211_frame)) {
					vap->iv_stats.is_rx_tooshort++;
					goto out;
				}
				bssid = wh->i_addr3;
			}
			/*
			 * Validate the bssid. Let beacons get through though for 11g protection mode.
			 */
			if (!IEEE80211_ADDR_EQ(bssid, vap->iv_bssid) && !IEEE80211_ADDR_EQ(bssid, dev->broadcast) && (subtype != IEEE80211_FC0_SUBTYPE_BEACON)) {
#ifdef ATH_SUPERG_XR
				/*
				 * allow MGT frames to vap->iv_xrvap.
				 * this will allow roaming between  XR and normal vaps
				 * without station dis associating from previous vap.
				 */
				if (!(vap->iv_xrvap && IEEE80211_ADDR_EQ(bssid, vap->iv_xrvap->iv_bssid) && type == IEEE80211_FC0_TYPE_MGT && ni != vap->iv_bss)) {
					/* not interested in */
					vap->iv_stats.is_rx_wrongbss++;
					goto out;
				}
			}
#else
				/* not interested in */
				vap->iv_stats.is_rx_wrongbss++;
				goto out;
			}
#endif
#ifdef HAVE_POLLING
			if (ic->ic_pollingmode) {

#define CN_SEND_CTS_ON_TIMER 1
				if (vap->iv_policy_glue->cn_event(vap, CN_CTS_DATA_RCVD, ni->ni_macaddr) == 0) {
#if CN_SEND_CTS_ON_TIMER
					//vap->iv_policy_glue->cn_select( vap, mac_to_send, &ctstimeout, 0 );
					vap->iv_policy_glue->cn_action(vap, NULL, mac_to_send, 0);

					if (memcmp(mac_to_send, ni->ni_macaddr, IEEE80211_ADDR_LEN) != 0)
						ieee80211_send_cts(vap, mac_to_send, ctstimeout, NULL);
					else
						ieee80211_send_cts(vap, ni->ni_macaddr, ctstimeout, ieee80211_ref_node(ni));
#endif
				}
			}
#endif
			break;
		case IEEE80211_M_WDS:
			if (skb->len < sizeof(struct ieee80211_frame_addr4)) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY, ni->ni_macaddr, NULL, "too short (3): len %u", skb->len);
				vap->iv_stats.is_rx_tooshort++;
				goto out;
			}
			bssid = wh->i_addr1;
			if (!IEEE80211_ADDR_EQ(bssid, vap->iv_bssid) && !IEEE80211_ADDR_EQ(bssid, dev->broadcast)) {
				/* not interested in */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, bssid, NULL, "%s", "not to bss");
				vap->iv_stats.is_rx_wrongbss++;
				goto out;
			}
			if (!IEEE80211_ADDR_EQ(wh->i_addr2, vap->wds_mac)) {
				/* not interested in */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, wh->i_addr2, NULL, "%s", "not from DS");
				vap->iv_stats.is_rx_wrongbss++;
				goto out;
			}
			break;
		default:
			/* XXX catch bad values */
			goto out;
		}
		/* since ieee80211_input() can be called by
		 * ieee80211_input_all(), we need to check that we are not
		 * updating for unknown nodes. FIXME : such check might be
		 * needed at other places */
		if (IEEE80211_ADDR_EQ(wh->i_addr2, ni->ni_macaddr)) {
			ni->ni_rssi = rssi;
			ni->ni_rtsf = rtsf;
			ni->ni_last_rx = jiffies;
		}
#ifdef LED2_PIN
		/* UBNT */
		if ((vap->iv_state == IEEE80211_S_RUN) && (dev->flags & IFF_RUNNING)) {
			// update momentary stats for sta mode
			if (vap->iv_opmode == IEEE80211_M_STA && ni->ni_associd) {
				vap->iv_last_rssiupdate = jiffies;
				mod_timer(&vap->iv_rssiupdate, jiffies + HZ);
			}
			// Update leds to show RSSI level
			if (ic->ic_update_rssi_leds)
				ic->ic_update_rssi_leds(ic, ieee80211_getrssi(ic));
		}
		/* END UBNT */
#endif
		if (HAS_SEQ(type)) {
			u_int8_t tid;
			if (IEEE80211_QOS_HAS_SEQ(wh)) {
				tid = ((struct ieee80211_qosframe *)wh)->i_qos[0] & IEEE80211_QOS_TID;
				if (TID_TO_WME_AC(tid) >= WME_AC_VI)
					ic->ic_wme.wme_hipri_traffic++;
				tid++;
			} else
				tid = 0;
			rxseq = le16toh(*(__le16 *)wh->i_seq);
			if ((wh->i_fc[1] & IEEE80211_FC1_RETRY) && (rxseq == ni->ni_rxseqs[tid])) {
				/* duplicate, discard */
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT,
						      bssid, "duplicate",
						      "seqno <%u,%u> fragno <%u,%u> tid %u",
						      rxseq >> IEEE80211_SEQ_SEQ_SHIFT, ni->ni_rxseqs[tid] >> IEEE80211_SEQ_SEQ_SHIFT, rxseq & IEEE80211_SEQ_FRAG_MASK, ni->ni_rxseqs[tid] & IEEE80211_SEQ_FRAG_MASK, tid);
				vap->iv_stats.is_rx_dup++;
				IEEE80211_NODE_STAT(ni, rx_dup);
				goto out;
			}
			ni->ni_rxseqs[tid] = rxseq;
		}
	}

	switch (type) {
	case IEEE80211_FC0_TYPE_DATA:
		hdrspace = ieee80211_hdrsize(wh);
		if (skb->len < hdrspace) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, "data", "too short: len %u, expecting %u", skb->len, hdrspace);
			vap->iv_stats.is_rx_tooshort++;
			goto out;	/* XXX */
		}
		switch (vap->iv_opmode) {
		case IEEE80211_M_STA:
			switch (dir) {
			case IEEE80211_FC1_DIR_FROMDS:
				break;
			case IEEE80211_FC1_DIR_DSTODS:
				if (vap->iv_flags_ext & IEEE80211_FEXT_WDS)
					break;
			default:
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}

			if (IEEE80211_IS_MULTICAST(wh->i_addr1)) {
				/* ignore 3-addr mcast if we're WDS STA */
				if (vap->iv_flags_ext & IEEE80211_FEXT_WDS)
					goto out;

				/* Discard multicast if IFF_MULTICAST not set */
				if ((0 != memcmp(wh->i_addr3, dev->broadcast, ETH_ALEN)) && (0 == (dev->flags & IFF_MULTICAST))) {
					IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, NULL, "%s", "multicast disabled.");
					printk(KERN_ERR "CONFIG ERROR: multicast flag " "cleared on radio, but multicast was used.\n");
					vap->iv_stats.is_rx_mcastdisabled++;
					goto out;
				}
				/* Discard echos of our own multicast or broadcast */
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
					IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, NULL, "%s", "multicast echo");
					vap->iv_stats.is_rx_mcastecho++;
					goto out;
				}
			} else {
				/* Same BSSID, but not meant for us to receive */
				if (!IEEE80211_ADDR_EQ(wh->i_addr1, vap->iv_myaddr))
					goto out;

				if (dir == IEEE80211_FC1_DIR_DSTODS && (vap->iv_flags_ext & IEEE80211_FEXT_WDS))
					del_timer_sync(&ic->ic_wdsprobe);
			}
			break;
		case IEEE80211_M_IBSS:
		case IEEE80211_M_AHDEMO:
			/* ignore foreign data frames */
			if (ni == vap->iv_bss)
				goto out;

			if (dir != IEEE80211_FC1_DIR_NODS) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}
			/* XXX no power-save support */
			break;
		case IEEE80211_M_HOSTAP:
			if ((dir != IEEE80211_FC1_DIR_TODS) && (dir != IEEE80211_FC1_DIR_DSTODS)) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}
			/* check if source STA is associated */
			if (ni == vap->iv_bss) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, "data", "%s", "unknown src");
				/* NB: caller deals with reference */
				if (vap->iv_state == IEEE80211_S_RUN)
					ieee80211_send_error(ni, wh->i_addr2, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_NOT_AUTHED);
				vap->iv_stats.is_rx_notassoc++;
				goto err;
			}
			if (ni->ni_associd == 0) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, "data", "%s", "unassoc src");
				IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DISASSOC, IEEE80211_REASON_NOT_ASSOCED);
				vap->iv_stats.is_rx_notassoc++;
				goto err;
			}

			/*
			 * If we're a 4 address packet, make sure we have an entry in
			 * the node table for the packet source address (addr4).
			 * If not, add one.
			 */
			/* check for wds link first */
			if ((dir == IEEE80211_FC1_DIR_DSTODS) && !ni->ni_subif) {
				if (vap->iv_flags_ext & IEEE80211_FEXT_WDSSEP) {
					ieee80211_wds_addif(ni);
					/* we must drop frames here until the interface has
					 * been fully separated, otherwise a bridge might get
					 * confused */
					goto err;
				}
			}

			/*
			 * Check for power save state change.
			 */
			if (!(ni->ni_flags & IEEE80211_NODE_UAPSD)) {
				if ((wh->i_fc[1] & IEEE80211_FC1_PWR_MGT) ^ (ni->ni_flags & IEEE80211_NODE_PWR_MGT))
					ieee80211_node_pwrsave(ni, wh->i_fc[1] & IEEE80211_FC1_PWR_MGT);
			} else if (ni->ni_flags & IEEE80211_NODE_PS_CHANGED) {
				int pwr_save_changed = 0;
				IEEE80211_LOCK_IRQ(ic);
				if ((*(__le16 *)(&wh->i_seq[0])) == ni->ni_pschangeseq) {
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
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, "data", "invalid dir 0x%x", dir);
				vap->iv_stats.is_rx_wrongdir++;
				goto out;
			}
			break;
		default:
			/* XXX here to keep compiler happy */
			goto out;
		}

		/* check if there is any data left */
		hdrspace = ieee80211_hdrspace(ic, wh);
		if (skb->len < hdrspace)
			goto out;

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
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, "WEP", "%s", "PRIVACY off");
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
		wh = NULL;	/* no longer valid, catch any uses */

		/*
		 * Next strip any MSDU crypto bits.
		 */
		if (key != NULL && !ieee80211_crypto_demic(vap, key, skb, hdrspace, 0)) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, ni->ni_macaddr, "data", "%s", "demic error");
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
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, ni->ni_macaddr, "data", "%s", "decap error");
			vap->iv_stats.is_rx_decap++;
			IEEE80211_NODE_STAT(ni, rx_decap);
			goto err;
		}
		eh = (struct ether_header *)skb->data;

		if (!accept_data_frame(vap, ni, key, skb, eh))
			goto out;

		IEEE80211_NODE_STAT(ni, rx_data);
		IEEE80211_NODE_STAT_ADD(ni, rx_bytes, skb->len);
		ic->ic_lastdata = jiffies;

#ifdef ATH_SUPERG_FF
		/* check for FF */
		llc = (struct llc *)(skb->data + sizeof(struct ether_header));
		if (ntohs(llc->llc_snap.ether_type) == (u_int16_t)ATH_ETH_TYPE) {
			struct sk_buff *skb1 = NULL;
			struct ether_header *eh_tmp;
			struct athl2p_tunnel_hdr *ath_hdr;
			unsigned int frame_len;

			/* NB: assumes linear (i.e., non-fragmented) skb */

			/* get to the tunneled headers */
			ath_hdr = (struct athl2p_tunnel_hdr *)
			    skb_pull(skb, sizeof(struct ether_header) + LLC_SNAPFRAMELEN);
			/* ignore invalid frames */
			if (ath_hdr == NULL)
				goto err;

			/* only implementing FF now. drop all others. */
			if (ath_hdr->proto != ATH_L2TUNNEL_PROTO_FF) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_SUPG | IEEE80211_MSG_INPUT, eh->ether_shost, "fast-frame", "bad atheros tunnel prot %u", ath_hdr->proto);
				vap->iv_stats.is_rx_badathtnl++;
				goto err;
			}
			vap->iv_stats.is_rx_ffcnt++;

			/* move past the tunneled header, with alignment */
			skb_pull(skb, roundup(sizeof(struct athl2p_tunnel_hdr) - 2, 4) + 2);
			eh_tmp = (struct ether_header *)skb->data;

			/* ether_type must be length as FF frames are always LLC/SNAP encap'd */
			frame_len = ntohs(eh_tmp->ether_type);

			skb1 = skb_clone(skb, GFP_ATOMIC);
			if (skb1 == NULL)
				goto err;
			ieee80211_skb_copy_noderef(skb, skb1);

			/* we now have 802.3 MAC hdr followed by 802.2 LLC/SNAP; convert to EthernetII.
			 * Note that the frame is at least IEEE80211_MIN_LEN, due to the driver code. */
			athff_decap(skb);

			/* remove second frame from end of first */
			skb_trim(skb, sizeof(struct ether_header) + frame_len - LLC_SNAPFRAMELEN);

			/* prepare second tunneled frame */
			skb_pull(skb1, roundup(sizeof(struct ether_header) + frame_len, 4));

			/* Fail if there is no space left for at least the necessary headers */
			if (athff_decap(skb1)) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, ni->ni_macaddr, "data", "%s", "Decapsulation error");
				vap->iv_stats.is_rx_decap++;
				IEEE80211_NODE_STAT(ni, rx_decap);
				ieee80211_dev_kfree_skb(&skb1);
				goto err;
			}

			/* deliver the frames */
			ieee80211_deliver_data(ni, skb);
			ieee80211_deliver_data(ni, skb1);
		} else {
			/* assume non-atheros llc type */
			ieee80211_deliver_data(ni, skb);
		}
#else				/* !ATH_SUPERG_FF */
		ieee80211_deliver_data(ni, skb);
#endif
		if (ni_or_null == NULL)
			ieee80211_unref_node(&ni);
		return IEEE80211_FC0_TYPE_DATA;

	case IEEE80211_FC0_TYPE_MGT:
		/*
		 * WDS opmode do not support management frames
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
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY, ni->ni_macaddr, "mgt", "too short: len %u", skb->len);
			vap->iv_stats.is_rx_tooshort++;
			goto out;
		}
		if (vap->iv_opmode == IEEE80211_M_STA) {
			/* Same BSSID, but not meant for us to receive */
			if (!IEEE80211_ADDR_EQ(wh->i_addr1, vap->iv_myaddr) && !IEEE80211_IS_MULTICAST(wh->i_addr1))
				goto out;
		}
#ifdef IEEE80211_DEBUG
		if ((ieee80211_msg_debug(vap) && doprint(vap, subtype)) || ieee80211_msg_dumppkts(vap)) {
			ieee80211_note(vap, "received %s from " MAC_FMT " rssi %d\n", ieee80211_mgt_subtype_name[subtype >> IEEE80211_FC0_SUBTYPE_SHIFT], MAC_ADDR(wh->i_addr2), rssi);
		}
#endif
		if (wh->i_fc[1] & IEEE80211_FC1_PROT) {
			if (subtype != IEEE80211_FC0_SUBTYPE_AUTH) {
				/*
				 * Only shared key auth frames with a challenge
				 * should be encrypted, discard all others.
				 */
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, ieee80211_mgt_subtype_name[subtype >> IEEE80211_FC0_SUBTYPE_SHIFT], "%s", "WEP set but not permitted");
				vap->iv_stats.is_rx_mgtdiscard++;	/* XXX */
				goto out;
			}
			if ((vap->iv_flags & IEEE80211_F_PRIVACY) == 0) {
				/*
				 * Discard encrypted frames when privacy is off.
				 */
				IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, "mgt", "%s", "WEP set but PRIVACY off");
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
		ic->ic_recv_mgmt(vap, ni_or_null, skb, subtype, rssi, rtsf);
		goto out;

	case IEEE80211_FC0_TYPE_CTL:
		IEEE80211_NODE_STAT(ni, rx_ctrl);
		vap->iv_stats.is_rx_ctl++;
		if (vap->iv_opmode == IEEE80211_M_HOSTAP)
			if (subtype == IEEE80211_FC0_SUBTYPE_PS_POLL)
				ieee80211_recv_pspoll(ni, skb);
		goto out;

	default:
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, NULL, "bad frame type 0x%x", type);
		/* should not come here */
		break;
	}
err:
	vap->iv_devstats.rx_errors++;
out:
	if (ni_or_null == NULL)
		ieee80211_unref_node(&ni);
discard:
	if (skb != NULL)
		ieee80211_dev_kfree_skb(&skb);
	return type;
#undef HAS_SEQ
}

EXPORT_SYMBOL(ieee80211_input);

/*
 * Determines whether a frame should be accepted, based on information
 * about the frame's origin and encryption, and policy for this vap.
 */
static int accept_data_frame(struct ieee80211vap *vap, struct ieee80211_node *ni, struct ieee80211_key *key, struct sk_buff *skb, struct ether_header *eh)
{
#define IS_EAPOL(eh) ((eh)->ether_type == __constant_htons(ETHERTYPE_PAE))
#define PAIRWISE_SET(vap) ((vap)->iv_nw_keys[0].wk_cipher != &ieee80211_cipher_none)
	if (IS_EAPOL(eh)) {
		/* encrypted eapol is always OK */
		if (key)
			return 1;
		/* cleartext eapol is OK if we don't have pairwise keys yet */
		if (!PAIRWISE_SET(vap))
			return 1;
		/* cleartext eapol is OK if configured to allow it */
		if (!IEEE80211_VAP_DROPUNENC_EAPOL(vap))
			return 1;
		/* cleartext eapol is OK if other unencrypted is OK */
		if (!(vap->iv_flags & IEEE80211_F_DROPUNENC))
			return 1;
		/* not OK */
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, eh->ether_shost, "data", "unauthorized port: ether type 0x%x len %u", ntohs(eh->ether_type), skb->len);
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
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, eh->ether_shost, "data", "unauthorized port: ether type 0x%x len %u", ntohs(eh->ether_type), skb->len);
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
int ieee80211_input_all(struct ieee80211com *ic, struct sk_buff *skb, int rssi, u_int64_t rtsf)
{
	struct ieee80211_frame_min *wh = (struct ieee80211_frame_min *)skb->data;
	struct ieee80211vap *vap;
	int type = -1;

	/* XXX locking */
	TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next) {
		struct ieee80211_node *ni = NULL;
		struct sk_buff *skb1;

		if ((vap->iv_dev->flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
			continue;

		if ((vap->iv_opmode == IEEE80211_M_HOSTAP) && !IEEE80211_IS_MULTICAST(wh->i_addr1))
			continue;

		ni = ieee80211_find_rxnode(ic, vap, wh);
		if (TAILQ_NEXT(vap, iv_next) != NULL) {
			skb1 = skb_copy(skb, GFP_ATOMIC);
			if (skb1 == NULL) {
				continue;
			}
			/* We duplicate the reference after skb_copy */
			ieee80211_skb_copy_noderef(skb, skb1);
		} else {
			skb1 = skb;
			skb = NULL;
		}
		type = ieee80211_input(vap, ni, skb1, rssi, rtsf);
		if (ni)
			ieee80211_unref_node(&ni);
	}

out:
	if (skb != NULL)	/* no vaps, reclaim skb */
		ieee80211_dev_kfree_skb(&skb);
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
static struct sk_buff *ieee80211_defrag(struct ieee80211_node *ni, struct sk_buff *skb, int hdrlen)
{
	struct ieee80211_frame *wh = (struct ieee80211_frame *)skb->data;
	u_int16_t rxseq, last_rxseq;
	u_int8_t fragno, last_fragno;
	u_int8_t more_frag = wh->i_fc[1] & IEEE80211_FC1_MORE_FRAG;

	rxseq = le16_to_cpu(*(__le16 *)wh->i_seq) >> IEEE80211_SEQ_SEQ_SHIFT;
	fragno = le16_to_cpu(*(__le16 *)wh->i_seq) & IEEE80211_SEQ_FRAG_MASK;

	/* Quick way out, if there's nothing to defragment */
	if (!more_frag && fragno == 0 && ni->ni_rxfrag == NULL)
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
		ieee80211_dev_kfree_skb(&skb);
		return NULL;
	}

	/*
	 * Update the time stamp.  As a side effect, it
	 * also makes sure that the timer will not change
	 * ni->ni_rxfrag for at least 1 second, or in
	 * other words, for the remaining of this function.
	 * XXX HUGE HORRIFIC HACK
	 */
	ni->ni_rxfragstamp = jiffies;

	/*
	 * Validate that fragment is in order and
	 * related to the previous ones.
	 */
	if (ni->ni_rxfrag) {
		struct ieee80211_frame *lwh;

		lwh = (struct ieee80211_frame *)ni->ni_rxfrag->data;
		last_rxseq = le16_to_cpu(*(__le16 *)lwh->i_seq) >> IEEE80211_SEQ_SEQ_SHIFT;
		last_fragno = le16_to_cpu(*(__le16 *)lwh->i_seq) & IEEE80211_SEQ_FRAG_MASK;
		if (rxseq != last_rxseq || fragno != last_fragno + 1 || (!IEEE80211_ADDR_EQ(wh->i_addr1, lwh->i_addr1))
		    || (!IEEE80211_ADDR_EQ(wh->i_addr2, lwh->i_addr2))
		    || (ni->ni_rxfrag->end - ni->ni_rxfrag->tail < skb->len)) {
			/*
			 * Unrelated fragment or no space for it,
			 * clear current fragments
			 */
			ieee80211_dev_kfree_skb(&ni->ni_rxfrag);
		}
	}

	/* If this is the first fragment */
	if (ni->ni_rxfrag == NULL && fragno == 0) {
		ni->ni_rxfrag = skb;
		/* If more frags are coming */
		if (more_frag) {
			if (skb_is_nonlinear(skb)) {
				/*
				 * We need a continous buffer to
				 * assemble fragments
				 */
				ni->ni_rxfrag = skb_copy(skb, GFP_ATOMIC);
				if (ni->ni_rxfrag) {
					ieee80211_skb_copy_noderef(skb, ni->ni_rxfrag);
					ieee80211_dev_kfree_skb(&skb);
				}
			}
			/*
			 * Check that we have enough space to hold
			 * incoming fragments
			 * XXX 4-address/QoS frames?
			 */
			else if ((skb_end_pointer(skb) - skb->head) < (ni->ni_vap->iv_dev->mtu + hdrlen)) {
				ni->ni_rxfrag = skb_copy_expand(skb, 0, (ni->ni_vap->iv_dev->mtu + hdrlen) - (skb_end_pointer(skb) - skb->head), GFP_ATOMIC);
				if (ni->ni_rxfrag)
					ieee80211_skb_copy_noderef(skb, ni->ni_rxfrag);
				ieee80211_dev_kfree_skb(&skb);
			}
		}
	} else {
		if (ni->ni_rxfrag) {
			struct ieee80211_frame *lwh = (struct ieee80211_frame *)
			    ni->ni_rxfrag->data;

			/*
			 * We know we have enough space to copy,
			 * we've verified that before
			 */
			/* Copy current fragment at end of previous one */
			memcpy(skb_tail_pointer(ni->ni_rxfrag), skb->data + hdrlen, skb->len - hdrlen);
			/* Update tail and length */
			skb_put(ni->ni_rxfrag, skb->len - hdrlen);
			/* Keep a copy of last sequence and fragno */
			*(__le16 *)lwh->i_seq = *(__le16 *)wh->i_seq;
		}
		/* we're done with the fragment */
		ieee80211_dev_kfree_skb(&skb);
	}

	if (more_frag) {
		/* More to come */
		skb = NULL;
	} else {
		/* Last fragment received, we're done! */
		skb = ni->ni_rxfrag;
		ni->ni_rxfrag = NULL;
	}
	return skb;
}

static void ieee80211_deliver_data(struct ieee80211_node *ni, struct sk_buff *skb)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct net_device *dev = vap->iv_dev;
	struct ether_header *eh = (struct ether_header *)skb->data;

#ifdef ATH_SUPERG_XR
	/*
	 * if it is a XR vap, send the data to associated normal net
	 * device. XR vap has a net device which is not registered with
	 * OS. 
	 */
	if (vap->iv_xrvap && vap->iv_flags & IEEE80211_F_XR)
		dev = vap->iv_xrvap->iv_dev;
#endif

	/* if the node has a wds subif, move data frames there,
	 * but keep EAP traffic on the master */
	if (ni->ni_subif && ((eh)->ether_type != __constant_htons(ETHERTYPE_PAE))) {
		if (ni->ni_vap == ni->ni_subif) {
			ieee80211_dev_kfree_skb(&skb);
			return;
		} else {
			vap = ni->ni_subif;
			dev = vap->iv_dev;
		}
	}

	/* perform as a bridge within the vap */
	/* XXX intra-vap bridging only */
	if (vap->iv_opmode == IEEE80211_M_HOSTAP && (vap->iv_flags & IEEE80211_F_NOBRIDGE) == 0) {
		struct sk_buff *skb1 = NULL;

		if (ETHER_IS_MULTICAST(eh->ether_dhost) && !netif_queue_stopped(dev)) {
			/* Create a SKB for the BSS to send out. */
			skb1 = skb_clone(skb, GFP_ATOMIC);
			if (skb1)
				SKB_CB(skb1)->ni = ieee80211_ref_node(vap->iv_bss);
		} else {
			/*
			 * Check if destination is associated with the
			 * same vap and authorized to receive traffic.
			 * Beware of traffic destined for the vap itself;
			 * sending it will not work; just let it be
			 * delivered normally.
			 */
			struct ieee80211_node *ni1 = ieee80211_find_txnode(vap, eh->ether_dhost);
			if (ni1 != NULL) {
				if (ieee80211_node_is_authorized(ni1) && !ni1->ni_subif && ni1 != vap->iv_bss) {

					/* tried to bridge to a subif, drop the packet */
					if (ni->ni_subif) {
						ieee80211_unref_node(&ni1);
						ieee80211_dev_kfree_skb(&skb);
						return;
					}

					skb1 = skb;
					skb = NULL;
				}
				/* XXX statistic? */
				ieee80211_unref_node(&ni1);
			}
		}
		if (skb1 != NULL) {
			int ret;
			skb1->dev = dev;
			skb_reset_mac_header(skb1);
			skb_set_network_header(skb1, sizeof(struct ether_header));

			skb1->protocol = __constant_htons(ETH_P_802_2);
			/* XXX insert vlan tag before queue it? */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
			ret = dev->hard_start_xmit(skb1, dev);
#else
			ret = dev->netdev_ops->ndo_start_xmit(skb1, dev);
#endif

			if (ret == NETDEV_TX_BUSY)
				ieee80211_dev_kfree_skb(&skb1);

			else if (ret != NETDEV_TX_OK) {
				/* If queue dropped the packet because device was
				 * too busy */
				vap->iv_devstats.tx_dropped++;
			}
			/* skb is no longer ours, either way after dev_queue_xmit */
			skb1 = NULL;
		}
	}

	vap->iv_ic->ic_recv++;
	if (skb != NULL) {
		skb->dev = dev;

#ifdef USE_HEADERLEN_RESV
		skb->protocol = ath_eth_type_trans(skb, dev);
#else
		skb->protocol = eth_type_trans(skb, dev);
#endif
		vap->iv_devstats.rx_packets++;
		vap->iv_devstats.rx_bytes += skb->len;
#if IEEE80211_VLAN_TAG_USED
		if (ni->ni_vlan != 0 && vap->iv_vlgrp != NULL) {
			/* attach vlan tag */
			vlan_hwaccel_receive_skb(skb, vap->iv_vlgrp, ni->ni_vlan);
			skb = NULL;	/* SKB is no longer ours */
		} else
#endif
		{
			netif_receive_skb(skb);
			skb = NULL;	/* SKB is no longer ours */
		}
		dev->last_rx = jiffies;
	}
}

/* This function removes the 802.11 header, including LLC/SNAP headers and 
 * replaces it with an Ethernet II header. */
static struct sk_buff *ieee80211_decap(struct ieee80211vap *vap, struct sk_buff *skb, int hdrlen)
{
	const struct llc snap_hdr = {.llc_dsap = LLC_SNAP_LSAP,
		.llc_ssap = LLC_SNAP_LSAP,
		.llc_snap.control = LLC_UI,
		.llc_snap.org_code = { 0x0, 0x0, 0x0 }
	};
	struct ieee80211_qosframe_addr4 wh;	/* Max size address frames */
	struct ether_header *eh;
	struct llc *llc;
	__be16 ether_type = 0;

	memcpy(&wh, skb->data, hdrlen);	/* Make a copy of the variably sized .11 header */

	llc = (struct llc *)skb_pull(skb, hdrlen);
	/* XXX: For some unknown reason some APs think they are from DEC and 
	 * use an OUI of 00-00-f8. This should be killed as soon as sanity is 
	 * restored. */
	if ((skb->len >= LLC_SNAPFRAMELEN) && (memcmp(&snap_hdr, llc, 5) == 0) && ((llc->llc_snap.org_code[2] == 0x0) || (llc->llc_snap.org_code[2] == 0xf8))) {
		ether_type = llc->llc_un.type_snap.ether_type;
		skb_pull(skb, LLC_SNAPFRAMELEN);
		llc = NULL;
	}

	eh = (struct ether_header *)skb_push(skb, sizeof(struct ether_header));
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

	if (llc != NULL)
		eh->ether_type = htons(skb->len - sizeof(*eh));
	else
		eh->ether_type = ether_type;

	if (!ALIGNED_POINTER(skb->data + sizeof(*eh), u_int32_t)) {
		memmove(skb->data - 2, skb->data, skb->len);
		skb->data -= 2;
	}
	return skb;
}

/*
 * Install received rate set information in the node's state block.
 */
int ieee80211_setup_rates(struct ieee80211_node *ni, const u_int8_t *rates, const u_int8_t *xrates, int flags)
{
	struct ieee80211_rateset *rs = &ni->ni_rates;
	struct ieee80211vap *vap = ni->ni_vap;
	int ret;

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
			IEEE80211_NOTE(vap, IEEE80211_MSG_XRATE, ni, "extended rate set too large;" " only using %u of %u rates", nxrates, xrates[1]);
			vap->iv_stats.is_rx_rstoobig++;
		}
		memcpy(rs->rs_rates + rs->rs_nrates, xrates + 2, nxrates);
		rs->rs_nrates += nxrates;
	}
	ret = ieee80211_fix_rate(ni, flags);
	if (!rs->rs_nrates && vap && vap->iv_bss && ni != vap->iv_bss)
		ni->ni_rates = vap->iv_bss->ni_rates;
	return ret;
}

static void ieee80211_auth_open(struct ieee80211_node *ni, struct ieee80211_frame *wh, int rssi, u_int64_t rtsf, u_int16_t seq, u_int16_t status)
{
	struct ieee80211vap *vap = ni->ni_vap;
	unsigned int tmpnode = 0;

	if (ni->ni_authmode == IEEE80211_AUTH_SHARED) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "open auth", "bad sta auth mode %u", ni->ni_authmode);
		vap->iv_stats.is_rx_bad_auth++;	/* XXX maybe a unique error? */
		if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
			if (ni == vap->iv_bss) {
				ni = ieee80211_dup_bss(vap, wh->i_addr2, 1);
				if (ni == NULL)
					return;
				tmpnode = 1;
			}
			IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, (seq + 1) | (IEEE80211_STATUS_ALG << 16));

			if (tmpnode)
				ieee80211_unref_node(&ni);
			return;
		}
	}
	switch (vap->iv_opmode) {
	case IEEE80211_M_IBSS:
		if (vap->iv_state != IEEE80211_S_RUN || seq != IEEE80211_AUTH_OPEN_REQUEST) {
			vap->iv_stats.is_rx_bad_auth++;
			return;
		}
		ieee80211_new_state(vap, IEEE80211_S_AUTH, wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK);
		break;

	case IEEE80211_M_AHDEMO:
	case IEEE80211_M_WDS:
		/* should not come here */
		break;

	case IEEE80211_M_HOSTAP:
		if (vap->iv_state != IEEE80211_S_RUN || seq != IEEE80211_AUTH_OPEN_REQUEST) {
			vap->iv_stats.is_rx_bad_auth++;
			return;
		}
		/* always accept open authentication requests */
		if (ni == vap->iv_bss) {
			ni = ieee80211_dup_bss(vap, wh->i_addr2, 0);
			if (ni == NULL)
				return;
			tmpnode = 1;
		}

		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, seq + 1);
		IEEE80211_NOTE(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni, "station authenticated (%s)", "open");
		/*
		 * When 802.1x is not in use mark the port
		 * authorized at this point so traffic can flow.
		 */
		if (ni->ni_authmode != IEEE80211_AUTH_8021X)
			ieee80211_node_authorize(ni);
		if (tmpnode)
			ieee80211_unref_node(&ni);
		break;

	case IEEE80211_M_STA:
		if (vap->iv_state != IEEE80211_S_AUTH || seq != IEEE80211_AUTH_OPEN_RESPONSE) {
			vap->iv_stats.is_rx_bad_auth++;
			return;
		}
		if (status != 0) {
			IEEE80211_NOTE(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni, "open auth failed (reason %d)", status);
			vap->iv_stats.is_rx_auth_fail++;
			ieee80211_new_state(vap, IEEE80211_S_SCAN, IEEE80211_SCAN_FAIL_STATUS);
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
static void ieee80211_send_error(struct ieee80211_node *ni, const u_int8_t *mac, int subtype, int arg)
{
	struct ieee80211vap *vap = ni->ni_vap;
	int istmp;

	if (ni == vap->iv_bss) {
		ni = ieee80211_dup_bss(vap, mac, 1);
		if (ni == NULL) {
			/* XXX msg */
			return;
		}
		istmp = 1;
	} else
		istmp = 0;
	IEEE80211_SEND_MGMT(ni, subtype, arg);
	if (istmp)
		ieee80211_unref_node(&ni);
}

static int alloc_challenge(struct ieee80211_node *ni)
{
	if (ni->ni_challenge == NULL)
		MALLOC(ni->ni_challenge, u_int32_t *, IEEE80211_CHALLENGE_LEN, M_DEVBUF, M_NOWAIT);
	if (ni->ni_challenge == NULL) {
		IEEE80211_NOTE(ni->ni_vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni, "%s", "shared key challenge alloc failed");
		/* XXX statistic */
	}
	return (ni->ni_challenge != NULL);
}

/* XXX TODO: add statistics */
static void ieee80211_auth_shared(struct ieee80211_node *ni, struct ieee80211_frame *wh, u_int8_t *frm, u_int8_t *efrm, int rssi, u_int64_t rtsf, u_int16_t seq, u_int16_t status)
{
	struct ieee80211vap *vap = ni->ni_vap;
	u_int8_t *challenge;
	int allocbs = 0, estatus = 0;

	/*
	 * NB: this can happen as we allow pre-shared key
	 * authentication to be enabled w/o wep being turned
	 * on so that configuration of these can be done
	 * in any order.  It may be better to enforce the
	 * ordering in which case this check would just be
	 * for sanity/consistency.
	 */
	if ((vap->iv_flags & IEEE80211_F_PRIVACY) == 0) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "%s", " PRIVACY is disabled");
		estatus = IEEE80211_STATUS_ALG;
		goto bad;
	}
	/*
	 * Pre-shared key authentication is evil; accept
	 * it only if explicitly configured (it is supported
	 * mainly for compatibility with clients like OS X).
	 */
	if (ni->ni_authmode != IEEE80211_AUTH_AUTO && ni->ni_authmode != IEEE80211_AUTH_SHARED) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "bad sta auth mode %u", ni->ni_authmode);
		vap->iv_stats.is_rx_bad_auth++;	/* XXX maybe a unique error? */
		estatus = IEEE80211_STATUS_ALG;
		goto bad;
	}

	challenge = NULL;
	if (frm + 1 < efrm) {
		if ((frm[1] + 2) > (efrm - frm)) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "ie %d/%d too long", frm[0], (frm[1] + 2) - (efrm - frm));
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
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "%s", "no challenge");
			vap->iv_stats.is_rx_bad_auth++;
			estatus = IEEE80211_STATUS_CHALLENGE;
			goto bad;
		}
		if (challenge[1] != IEEE80211_CHALLENGE_LEN) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "bad challenge len %d", challenge[1]);
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
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "bad operating mode %u", vap->iv_opmode);
		return;
	case IEEE80211_M_HOSTAP:
		if (vap->iv_state != IEEE80211_S_RUN) {
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "bad state %u", vap->iv_state);
			estatus = IEEE80211_STATUS_ALG;	/* XXX */
			goto bad;
		}
		switch (seq) {
		case IEEE80211_AUTH_SHARED_REQUEST:
			if (ni == vap->iv_bss) {
				ni = ieee80211_dup_bss(vap, wh->i_addr2, 0);
				if (ni == NULL) {
					/* NB: no way to return an error */
					return;
				}
				allocbs = 1;
			}

			ni->ni_rssi = rssi;
			ni->ni_rtsf = rtsf;
			ni->ni_last_rx = jiffies;
			if (!alloc_challenge(ni)) {
				if (allocbs)
					ieee80211_unref_node(&ni);
				/* NB: don't return error so they rexmit */
				return;
			}
			get_random_bytes(ni->ni_challenge, IEEE80211_CHALLENGE_LEN);
			IEEE80211_NOTE(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni, "shared key %sauth request", allocbs ? "" : "re");
			break;
		case IEEE80211_AUTH_SHARED_RESPONSE:
			if (ni == vap->iv_bss) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key response", "%s", "unknown station");
				/* NB: don't send a response */
				return;
			}
			if (ni->ni_challenge == NULL) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key response", "%s", "no challenge recorded");
				vap->iv_stats.is_rx_bad_auth++;
				estatus = IEEE80211_STATUS_CHALLENGE;
				goto bad;
			}
			if (memcmp(ni->ni_challenge, &challenge[2], challenge[1]) != 0) {
				IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key response", "%s", "challenge mismatch");
				vap->iv_stats.is_rx_auth_fail++;
				estatus = IEEE80211_STATUS_CHALLENGE;
				goto bad;
			}
			IEEE80211_NOTE(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ni, "station authenticated (%s)", "shared key");
			ieee80211_node_authorize(ni);
			break;
		default:
			IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_AUTH, ni->ni_macaddr, "shared key auth", "bad seq %d", seq);
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
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_AUTH, ieee80211_getbssid(vap, wh), "shared key auth failed (reason %d)", status);
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
			IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_AUTH, seq + 1);
			break;
		default:
			IEEE80211_DISCARD(vap, IEEE80211_MSG_AUTH, wh, "shared key auth", "bad seq %d", seq);
			vap->iv_stats.is_rx_bad_auth++;
			goto bad;
		}
		break;
	}
	if (allocbs)
		ieee80211_unref_node(&ni);
	return;
bad:
	/* Send an error response; but only when operating as an AP. */
	if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
		/* XXX hack to workaround calling convention */
		ieee80211_send_error(ni, wh->i_addr2, IEEE80211_FC0_SUBTYPE_AUTH, (seq + 1) | (estatus << 16));
		/* Remove node state if it exists and isn't just a 
		 * temporary copy of the bss (dereferenced later) */
		if (!allocbs && (ni != vap->iv_bss))
			ieee80211_node_leave(ni);
	} else if (vap->iv_opmode == IEEE80211_M_STA) {
		/*
		 * Kick the state machine.  This short-circuits
		 * using the mgt frame timeout to trigger the
		 * state transition.
		 */
		if (vap->iv_state == IEEE80211_S_AUTH)
			ieee80211_new_state(vap, IEEE80211_S_SCAN, 0);
	}
	if (allocbs)
		ieee80211_unref_node(&ni);
}

/* Verify the existence and length of __elem or get out. */
#define IEEE80211_VERIFY_ELEMENT(__elem, __maxlen) do {			\
	if ((__elem) == NULL) {						\
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,		\
			wh, ieee80211_mgt_subtype_name[subtype >>	\
				IEEE80211_FC0_SUBTYPE_SHIFT],		\
			"%s", "no " #__elem);				\
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
static void ieee80211_ssid_mismatch(struct ieee80211vap *vap, const char *tag, u_int8_t mac[IEEE80211_ADDR_LEN], u_int8_t *ssid)
{
	printk(KERN_ERR "[" MAC_FMT "] discard %s frame, ssid mismatch: ", MAC_ADDR(mac), tag);
	ieee80211_print_essid(ssid + 2, ssid[1]);
	printk(KERN_ERR "\n");
}

#define	IEEE80211_VERIFY_SSID(_ni, _ssid) do {				\
	if ((_ni)->ni_esslen == 0)					\
		return;							\
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
#else				/* !IEEE80211_DEBUG */
#define	IEEE80211_VERIFY_SSID(_ni, _ssid) do {				\
	if ((_ni)->ni_esslen == 0)					\
		return;							\
	if ((_ssid)[1] != 0 &&						\
	    ((_ssid)[1] != (_ni)->ni_esslen ||				\
	    memcmp((_ssid) + 2, (_ni)->ni_essid, (_ssid)[1]) != 0)) {	\
		vap->iv_stats.is_rx_ssidmismatch++;			\
		return;							\
	}								\
} while (0)
#endif				/* !IEEE80211_DEBUG */

/* unaligned little endian access */
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

static __inline int iswpaoui(const u_int8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm + 2) == ((MADWIFI_WPA_OUI_TYPE << 24) | WPA_OUI);
}

static __inline int iswmeoui(const u_int8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm + 2) == ((WME_OUI_TYPE << 24) | WME_OUI);
}

static __inline int iswmeparam(const u_int8_t *frm)
{
	return frm[1] > 5 && LE_READ_4(frm + 2) == ((WME_OUI_TYPE << 24) | WME_OUI) && frm[6] == WME_PARAM_OUI_SUBTYPE;
}

static __inline int iswmeinfo(const u_int8_t *frm)
{
	return frm[1] > 5 && LE_READ_4(frm + 2) == ((WME_OUI_TYPE << 24) | WME_OUI) && frm[6] == WME_INFO_OUI_SUBTYPE;
}

static __inline int isatherosoui(const u_int8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm + 2) == ((ATH_OUI_TYPE << 24) | ATH_OUI);
}

static __inline int ismtikoui(const u_int8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm + 2) == MTIK_OUI;
}

/*
 * Convert a WPA cipher selector OUI to an internal
 * cipher algorithm.  Where appropriate we also
 * record any key length.
 */
static int wpa_cipher(u_int8_t *sel, u_int8_t *keylen)
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
static int wpa_keymgmt(u_int8_t *sel)
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
static int ieee80211_parse_wpa(struct ieee80211vap *vap, u_int8_t *frm, struct ieee80211_rsnparms *rsn_parm, const struct ieee80211_frame *wh)
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "vap not WPA, flags 0x%x", vap->iv_flags);
		return IEEE80211_REASON_IE_INVALID;
	}

	if (len < 14) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "too short, len %u", len);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 6, len -= 4;	/* NB: len is payload only */
	/* NB: iswapoui already validated the OUI and type */
	w = LE_READ_2(frm);
	if (w != WPA_VERSION) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "bad version %u", w);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 2;
	len -= 2;

	/* multicast/group cipher */
	w = wpa_cipher(frm, &rsn_parm->rsn_mcastkeylen);
	if (w != rsn_parm->rsn_mcastcipher) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "mcast cipher mismatch; got %u, expected %u", w, rsn_parm->rsn_mcastcipher);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 4;
	len -= 4;

	/* unicast ciphers */
	n = LE_READ_2(frm);
	frm += 2;
	len -= 2;
	if (len < n * 4 + 2) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "ucast cipher data too short; len %u, n %u", len, n);
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "%s", "ucast cipher set empty");
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "key mgmt alg data too short; len %u, n %u", len, n);
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "WPA", "%s", "no acceptable key mgmt alg");
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
static int rsn_cipher(u_int8_t *sel, u_int8_t *keylen)
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
static int rsn_keymgmt(u_int8_t *sel)
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
static int ieee80211_parse_rsn(struct ieee80211vap *vap, u_int8_t *frm, struct ieee80211_rsnparms *rsn_parm, const struct ieee80211_frame *wh)
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "vap not RSN, flags 0x%x", vap->iv_flags);
		return IEEE80211_REASON_IE_INVALID;
	}

	if (len < 10) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "too short, len %u", len);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 2;
	w = LE_READ_2(frm);
	if (w != RSN_VERSION) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "bad version %u", w);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 2;
	len -= 2;

	/* multicast/group cipher */
	w = rsn_cipher(frm, &rsn_parm->rsn_mcastkeylen);
	if (w != rsn_parm->rsn_mcastcipher) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "mcast cipher mismatch; got %u, expected %u", w, rsn_parm->rsn_mcastcipher);
		return IEEE80211_REASON_IE_INVALID;
	}
	frm += 4;
	len -= 4;

	/* unicast ciphers */
	n = LE_READ_2(frm);
	frm += 2;
	len -= 2;
	if (len < n * 4 + 2) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "ucast cipher data too short; len %u, n %u", len, n);
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "%s", "ucast cipher set empty");
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "key mgmt alg data too short; len %u, n %u", len, n);
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
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WPA, wh, "RSN", "%s", "no acceptable key mgmt alg");
		return IEEE80211_REASON_IE_INVALID;
	}
	if (w & RSN_ASE_8021X_UNSPEC)
		rsn_parm->rsn_keymgmt = RSN_ASE_8021X_UNSPEC;
	else
		rsn_parm->rsn_keymgmt = RSN_ASE_8021X_PSK;

	/* optional RSN capabilities */
	if (len > 2)
		rsn_parm->rsn_caps = LE_READ_2(frm);
	/* XXX PMKID */

	return 0;
}

/* Record information element for later use. */
void ieee80211_saveie(u_int8_t **iep, const u_int8_t *ie)
{
	if ((*iep == NULL) || (ie == NULL) || ((*iep)[1] != ie[1])) {
		if (*iep != NULL)
			FREE(*iep, M_DEVBUF);
		*iep = NULL;
		if (ie != NULL)
			MALLOC(*iep, void *, ie[1] + 2, M_DEVBUF, M_NOWAIT);
	}
	if ((*iep != NULL) && (ie != NULL))
		memcpy(*iep, ie, ie[1] + 2);
}

EXPORT_SYMBOL(ieee80211_saveie);

static int ieee80211_parse_wmeie(u_int8_t *frm, const struct ieee80211_frame *wh, struct ieee80211_node *ni)
{
	u_int len = frm[1];

	if (len != 7) {
		IEEE80211_DISCARD_IE(ni->ni_vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WME, wh, "WME IE", "too short, len %u", len);
		return -1;
	}
	ni->ni_uapsd = frm[WME_CAPINFO_IE_OFFSET];
	if (ni->ni_uapsd) {
		ni->ni_flags |= IEEE80211_NODE_UAPSD;
		switch (WME_UAPSD_MAXSP(ni->ni_uapsd)) {
		case 1:
			ni->ni_uapsd_maxsp = 2;
			break;
		case 2:
			ni->ni_uapsd_maxsp = 4;
			break;
		case 3:
			ni->ni_uapsd_maxsp = 6;
			break;
		default:
			ni->ni_uapsd_maxsp = WME_UAPSD_NODE_MAXQDEPTH;
		}
	}
	IEEE80211_NOTE(ni->ni_vap, IEEE80211_MSG_POWER, ni, "UAPSD bit settings from STA: %02x", ni->ni_uapsd);

	return 1;
}

static int ieee80211_parse_wmeparams(struct ieee80211vap *vap, u_int8_t *frm, const struct ieee80211_frame *wh, u_int8_t *qosinfo)
{
#define	MS(_v, _f)	(((_v) & _f) >> _f##_S)
	struct ieee80211_wme_state *wme = &vap->iv_ic->ic_wme;
	u_int len = frm[1], qosinfo_count;
	int i;

	*qosinfo = 0;

	if (len < sizeof(struct ieee80211_wme_param) - 2) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_WME, wh, "WME", "too short, len %u", len);
		return -1;
	}
	*qosinfo = frm[__offsetof(struct ieee80211_wme_param, param_qosInfo)];
	qosinfo_count = *qosinfo & WME_QOSINFO_COUNT;
	/* XXX do proper check for wraparound */
	if (qosinfo_count == wme->wme_wmeChanParams.cap_info_count)
		return 0;
	frm += __offsetof(struct ieee80211_wme_param, params_acParams);
	for (i = 0; i < WME_NUM_AC; i++) {
		struct wmeParams *wmep = &wme->wme_wmeChanParams.cap_wmeParams[i];
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

static void ieee80211_parse_athParams(struct ieee80211_node *ni, u_int8_t *ie)
{
#ifdef ATH_SUPERG_DYNTURBO
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = ni->ni_ic;
#endif				/* ATH_SUPERG_DYNTURBO */
	struct ieee80211_ie_athAdvCap *athIe = (struct ieee80211_ie_athAdvCap *)ie;

	ni->ni_ath_flags = athIe->athAdvCap_capability;
	if (ni->ni_ath_flags & IEEE80211_ATHC_COMP)
		ni->ni_ath_defkeyindex = LE_READ_2(&athIe->athAdvCap_defKeyIndex);
#if 0
	/* NB: too noisy */
	IEEE80211_NOTE(vap, IEEE80211_MSG_SUPG, ni, "recv ath params: caps 0x%x flags 0x%x defkeyix %u", athIe->athAdvCap_capability, ni->ni_ath_flags, ni->ni_ath_defkeyindex);
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
#endif				/* ATH_SUPERG_DYNTURBO */
}

static void forward_mgmt_to_app(struct ieee80211vap *vap, int subtype, struct sk_buff *skb, struct ieee80211_frame *wh)
{
	struct net_device *dev = vap->iv_dev;
	int filter_type = 0;

	switch (subtype) {
	case IEEE80211_FC0_SUBTYPE_BEACON:
		filter_type = IEEE80211_FILTER_TYPE_BEACON;
		break;
	case IEEE80211_FC0_SUBTYPE_PROBE_REQ:
		filter_type = IEEE80211_FILTER_TYPE_PROBE_REQ;
		break;
	case IEEE80211_FC0_SUBTYPE_PROBE_RESP:
		filter_type = IEEE80211_FILTER_TYPE_PROBE_RESP;
		break;
	case IEEE80211_FC0_SUBTYPE_ASSOC_REQ:
	case IEEE80211_FC0_SUBTYPE_REASSOC_REQ:
		filter_type = IEEE80211_FILTER_TYPE_ASSOC_REQ;
		break;
	case IEEE80211_FC0_SUBTYPE_ASSOC_RESP:
	case IEEE80211_FC0_SUBTYPE_REASSOC_RESP:
		filter_type = IEEE80211_FILTER_TYPE_ASSOC_RESP;
		break;
	case IEEE80211_FC0_SUBTYPE_AUTH:
		filter_type = IEEE80211_FILTER_TYPE_AUTH;
		break;
	case IEEE80211_FC0_SUBTYPE_DEAUTH:
		filter_type = IEEE80211_FILTER_TYPE_DEAUTH;
		break;
	case IEEE80211_FC0_SUBTYPE_DISASSOC:
		filter_type = IEEE80211_FILTER_TYPE_DISASSOC;
		break;
	default:
		break;
	}

	if (filter_type && ((vap->app_filter & filter_type) == filter_type)) {
		struct sk_buff *skb1;

		skb1 = skb_copy(skb, GFP_ATOMIC);
		if (skb1 == NULL)
			return;
		/* We duplicate the reference after skb_copy */
		ieee80211_skb_copy_noderef(skb, skb1);
		skb1->dev = dev;
		skb_reset_mac_header(skb1);

		skb1->ip_summed = CHECKSUM_NONE;
		skb1->pkt_type = PACKET_OTHERHOST;
		skb1->protocol = __constant_htons(0x0019);	/* ETH_P_80211_RAW */

		netif_receive_skb(skb1);
		vap->iv_devstats.rx_packets++;
		vap->iv_devstats.rx_bytes += skb1->len;
	}
}

void ieee80211_saveath(struct ieee80211_node *ni, u_int8_t *ie)
{
	const struct ieee80211_ie_athAdvCap *athIe = (const struct ieee80211_ie_athAdvCap *)ie;

	ieee80211_saveie(&ni->ni_ath_ie, ie);
	if (athIe != NULL) {
		ni->ni_ath_flags = athIe->athAdvCap_capability;
		if (ni->ni_ath_flags & IEEE80211_ATHC_COMP)
			ni->ni_ath_defkeyindex = LE_READ_2(&athIe->athAdvCap_defKeyIndex);
	} else {
		ni->ni_ath_flags = 0;
		ni->ni_ath_defkeyindex = IEEE80211_INVAL_DEFKEY;
	}
}

/*
 * Structure to be passed through ieee80211_iterate_nodes() to count_nodes()
 */
struct count_nodes_arg {
	const int k;
	const int *subset;
	int count;
	struct ieee80211_node *new;
};

/* Count nodes which don't support at least one of arg->subset. */
static void count_nodes(void *_arg, struct ieee80211_node *ni)
{
	struct count_nodes_arg *arg = (struct count_nodes_arg *)_arg;
	int i;

	if (ni->ni_suppchans == NULL)
		return;

	if (ni == arg->new)
		return;

	for (i = 0; i < arg->k; i++)
		if (isclr(ni->ni_suppchans, arg->subset[i])) {
			arg->count++;
			return;
		}
}

/* Structure to be passed through combinations() to channel_combination() */
struct channel_combination_arg {
	struct ieee80211com *ic;
	struct ieee80211_node *new;
	int *best;
	int benefit;
};

#ifdef IEEE80211_DEBUG
/* sprintf() set[] array consisting of k integers */
static const char *ints_sprintf(const int k, const int set[])
{
	static char buf[915];	/* 0-255: 10*2 + 90*3 + 156*4 + '\0' */
	char *ptr = buf;
	int i;
	for (i = 0; i < k; i++)
		ptr += snprintf(ptr, buf + sizeof(buf) - ptr, "%d ", set[i]);
	return buf;
}
#endif

/* Action done for each combination of channels that are not supported by 
 * currently joining station. */
static void channel_combination(const int k, const int subset[], void *_arg)
{
	struct channel_combination_arg *arg = (struct channel_combination_arg *)_arg;
	struct ieee80211com *ic = arg->ic;
	struct count_nodes_arg cn_arg = { k, subset, 0, arg->new };
	int permil, allowed;
	int sta_assoc = ic->ic_sta_assoc;	/* make > 0 check consistent 
						 * with / operation */

	ieee80211_iterate_nodes(&arg->ic->ic_sta, &count_nodes, (void *)&cn_arg);

	/* The following two sanity checks can theoretically fail due to lack
	 * of locking, but since it is not fatal, we will just print a debug
	 * msg and neglect it */
	if (cn_arg.count == 0) {
		IEEE80211_NOTE(arg->new->ni_vap, IEEE80211_MSG_ANY, arg->new, "%s", "ic_chan_nodes incosistency (incorrect " "uncommon channel count)");
		return;
	}
	if (sta_assoc == 0) {
		IEEE80211_NOTE(arg->new->ni_vap, IEEE80211_MSG_ANY, arg->new, "%s", "no STAs associated, so there should be " "no \"uncommon\" channels");
		return;
	}

	permil = 1000 * cn_arg.count / sta_assoc;
	allowed = ic->ic_sc_slcg * k;
	/* clamp it to provide more sensible output */
	if (allowed > 1000)
		allowed = 1000;

	IEEE80211_NOTE(arg->new->ni_vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, arg->new, "Making channels %savailable would require " "kicking out %d stations,", ints_sprintf(k, subset), cn_arg.count);
	IEEE80211_NOTE(arg->new->ni_vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, arg->new, "what is %d permils of all associated STAs " "(slcg permits < %d).", permil, allowed);

	if (permil > allowed)
		return;
	if (allowed - permil > arg->benefit) {
		memcpy(arg->best, subset, k * sizeof(*subset));
		arg->benefit = allowed - permil;
	}
}

/* Enumerate all combinations of k-element subset of n-element set via a 
 * callback function. */
static void combinations(int n, int set[], int k, void (*callback)(const int, const int[], void *), void *arg)
{
	int subset[k], pos[k], i;
	for (i = 0; i < k; i++)
		pos[i] = 0;
	i = 0;
forward:
	if (i > 0) {
		while (set[pos[i]] < subset[i - 1] && pos[i] < n)
			pos[i]++;
		if (pos[i] == n)
			goto backward;
	}
	subset[i] = set[pos[i]];
	set[pos[i]] = set[n - 1];
	n--;

	i++;
	if (i == k) {
		callback(k, subset, arg);
	} else {
		pos[i] = 0;
		goto forward;
	}
backward:
	i--;
	if (i < 0)
		return;
	set[pos[i]] = subset[i];
	n++;

	pos[i]++;
	if (pos[i] == n)
		goto backward;
	goto forward;
}

static __inline int find_worse_nodes(struct ieee80211com *ic, struct ieee80211_node *new)
{
	int i, tmp1, tmp2;
	u_int16_t n_common, n_uncommon;
	u_int16_t cn_total = ic->ic_cn_total;
	u_int16_t to_gain;

	if (cn_total == 0)
		/* should not happen */
		return 1;

	n_common = n_uncommon = 0;

	CHANNEL_FOREACH(i, ic, tmp1, tmp2) {
		if (isset(new->ni_suppchans_new, i)) {
			if (ic->ic_chan_nodes[i] == ic->ic_cn_total) {
				n_common++;
			} else {
				n_uncommon++;
			}
		}
	}

	to_gain = ic->ic_sc_mincom - n_common + 1;
	IEEE80211_NOTE(new->ni_vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, new, "By accepting STA we would need to gain at least " "%d common channels.", to_gain);
	IEEE80211_NOTE(new->ni_vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, new, "%d channels supported by the joining STA are " "not commonly supported by others.", n_uncommon);

	if (to_gain > n_uncommon) {
		IEEE80211_NOTE(new->ni_vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, new, "%s", "Even disassociating all the nodes will not " "be enough.");
		return 0;
	}

	{
		int uncommon[n_uncommon];
		int best[to_gain];
		struct channel_combination_arg arg = { ic, new, best, -1 };
		int j = 0;

		CHANNEL_FOREACH(i, ic, tmp1, tmp2)
		    if (isset(new->ni_suppchans_new, i) && (ic->ic_chan_nodes[i] != ic->ic_cn_total)) {
			if (j == n_uncommon)
				/* silent assert */
				break;
			uncommon[j++] = i;
		}

		combinations(n_uncommon, uncommon, to_gain, &channel_combination, &arg);
		if (arg.benefit < 0) {
			IEEE80211_NOTE(new->ni_vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, new, "%s", "No combination of channels allows a " "beneficial trade-off.");
			return 0;
		}
		IEEE80211_NOTE(new->ni_vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, new, "Nodes which don't support channels %swill be " "forced to leave.", ints_sprintf(to_gain, best));

		if (new->ni_needed_chans != NULL)
			FREE(new->ni_needed_chans, M_DEVBUF);
		MALLOC(new->ni_needed_chans, void *, to_gain * sizeof(*new->ni_needed_chans), M_DEVBUF, M_NOWAIT);

		if (new->ni_needed_chans == NULL) {
			IEEE80211_NOTE(new->ni_vap, IEEE80211_MSG_DEBUG | IEEE80211_MSG_DOTH, new, "%s", "needed_chans allocation failed");
			return 0;
		}

		/* Store the list of channels to remove nodes which don't 
		 * support them. */
		for (i = 0; i < to_gain; i++)
			new->ni_needed_chans[i] = best[i];
		new->ni_n_needed_chans = to_gain;
		return 1;
	}
}

#if 0
static int ieee80211_parse_sc_ie(struct ieee80211_node *ni, u_int8_t *frm, const struct ieee80211_frame *wh)
{
	struct ieee80211_ie_sc *sc_ie = (struct ieee80211_ie_sc *)frm;
	struct ieee80211com *ic = ni->ni_ic;
#ifdef IEEE80211_DEBUG
	struct ieee80211vap *vap = ni->ni_vap;
	int reassoc = (wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_REASSOC_REQ;
#endif
	int i, tmp1, tmp2;
	int count;

	if (sc_ie == NULL) {
		if (ni->ni_ic->ic_sc_algorithm == IEEE80211_SC_STRICT) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "deny %s request, no supported " "channels IE", reassoc ? "reassoc" : "assoc");
			return IEEE80211_STATUS_SUPPCHAN_UNACCEPTABLE;
		}
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "%s request: no supported channels IE", reassoc ? "reassoc" : "assoc");
		return IEEE80211_STATUS_SUCCESS;
	}
	if (sc_ie->sc_len % 2 != 0) {
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "deny %s request, malformed supported " "channels IE (len)", reassoc ? "reassoc" : "assoc");
		/* XXX: deauth with IEEE80211_REASON_IE_INVALID? */
		return IEEE80211_STATUS_SUPPCHAN_UNACCEPTABLE;
	}
	if (ni->ni_suppchans_new == NULL) {
		MALLOC(ni->ni_suppchans_new, void *, IEEE80211_CHAN_BYTES, M_DEVBUF, M_NOWAIT);
		if (ni->ni_suppchans_new == NULL) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "deny %s request, couldn't allocate " "memory for SC IE!", reassoc ? "reassoc" : "assoc");
			return IEEE80211_STATUS_SUPPCHAN_UNACCEPTABLE;
		}
	}
	memset(ni->ni_suppchans_new, 0, IEEE80211_CHAN_BYTES);
	for (i = 0; i < (sc_ie->sc_len / 2); i++) {
		u_int8_t chan = sc_ie->sc_subband[i].sc_first;
		/* XXX: see 802.11d-2001-4-05-03-interp,
		 * but what about .11j, turbo, etc.? */
		u_int8_t step = (chan <= 14 ? 1 : 4);
		u_int16_t last = chan + step * (sc_ie->sc_subband[i].sc_number - 1);

		/* check for subband under- (sc_number == 0) or overflow */
		if ((last < chan) || ((chan <= 14) && (last > 14)) || (chan > 14 && last > 200)) {
			/* XXX: deauth with IEEE80211_REASON_IE_INVALID? */
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "deny %s request, malformed supported " "channels ie (subbands, %d, %d)", reassoc ? "reassoc" : "assoc", chan, last);
			return IEEE80211_STATUS_SUPPCHAN_UNACCEPTABLE;
		}

		for (; chan <= last; chan += step)
			setbit(ni->ni_suppchans_new, chan);
	}
	/* forbid STAs that claim they don't support the channel they are
	 * currently operating at */
	if (isclr(ni->ni_suppchans_new, ic->ic_bsschan->ic_ieee)) {
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "deny %s request, sc ie does not contain bss " "channel(subbands)", reassoc ? "reassoc" : "assoc");
		return IEEE80211_STATUS_SUPPCHAN_UNACCEPTABLE;
	}

	if ((ic->ic_sc_algorithm != IEEE80211_SC_TIGHT) && (ic->ic_sc_algorithm != IEEE80211_SC_STRICT))
		goto success;

	/* count number of channels that will be common to all STAs after the
	 * new one joins */
	count = 0;
	CHANNEL_FOREACH(i, ic, tmp1, tmp2)
	    if (isset(ni->ni_suppchans_new, i) && (ic->ic_chan_nodes[i] == ic->ic_cn_total))
		count++;

	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "%s request: %d common channels, %d " "required", reassoc ? "reassoc" : "assoc", count, ic->ic_sc_mincom);
	if (count < ic->ic_sc_mincom) {
		/* common channel count decreases below the required minimum */
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "%s request: not enough common " "channels available, tight/strict algorithm " "engaged", reassoc ? "reassoc" : "assoc");

		if (!find_worse_nodes(ic, ni)) {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DOTH, wh->i_addr2, "deny %s request, tight/strict " "criterion not met", reassoc ? "reassoc" : "assoc");
			return IEEE80211_STATUS_SUPPCHAN_UNACCEPTABLE;
		}
	}

success:
	IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_DOTH | IEEE80211_MSG_ASSOC | IEEE80211_MSG_ELEMID, wh->i_addr2, "%s", "supported channels ie parsing successful");
	return IEEE80211_STATUS_SUCCESS;
}

#endif
struct ieee80211_channel *ieee80211_doth_findchan(struct ieee80211vap *vap, u_int8_t chan)
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

static void ieee80211_doth_cancel_cs(struct ieee80211vap *vap)
{
	del_timer(&vap->iv_csa_timer);
	if (vap->iv_csa_jiffies)
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH,
				  "channel switch canceled (was: " "to %3d " "(%4d MHz) in %u TBTT, mode %u)\n", vap->iv_csa_chan->ic_ieee, vap->iv_csa_chan->ic_freq, vap->iv_csa_count, vap->iv_csa_mode);
	vap->iv_csa_jiffies = 0;
}

static void ieee80211_doth_switch_channel(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;
	unsigned long flags;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH, "%s: Channel switch to %3d (%4d MHz) NOW!\n", __func__, vap->iv_csa_chan->ic_ieee, vap->iv_csa_chan->ic_freq);
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

	vap->iv_csa_jiffies = 0;	/* supress "cancel" msg */
	ieee80211_doth_cancel_cs(vap);

	ic->ic_curchan = ic->ic_bsschan = vap->iv_csa_chan;
	ic->ic_set_channel(ic);
	spin_lock_irqsave(&channel_lock, flags);
	ieee80211_scan_set_bss_channel(ic, ic->ic_bsschan);
	spin_unlock_irqrestore(&channel_lock, flags);
}

static void ieee80211_doth_switch_channel_tmr(unsigned long arg)
{
	struct ieee80211vap *vap = (struct ieee80211vap *)arg;
	ieee80211_doth_switch_channel(vap);
}

static int ieee80211_parse_csaie(struct ieee80211_node *ni, u_int8_t *frm, const struct ieee80211_frame *wh)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_channel *c;
	struct ieee80211_ie_csa *csa_ie = (struct ieee80211_ie_csa *)frm;

	if (!frm) {
		/* we had CS underway but now we got Beacon without CSA IE */
		/* XXX abuse? */

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH, "%s: channel switch is scheduled, but we got " "Beacon without CSA IE!\n", __func__);

		ieee80211_doth_cancel_cs(vap);
		return 0;
	}

	if (csa_ie->csa_len != 3) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_DOTH, wh, "channel switch", "invalid length %u", csa_ie->csa_len);
		return -1;
	}

	if (isclr(ic->ic_chan_avail, csa_ie->csa_chan)) {
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_DOTH, wh, "channel switch", "invalid channel %u", csa_ie->csa_chan);
		return -1;
	}

	if ((c = ieee80211_doth_findchan(vap, csa_ie->csa_chan)) == NULL) {
		/* XXX something wrong */
		IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_DOTH, wh, "channel switch", "channel %u lookup failed", csa_ie->csa_chan);
		return -1;
	}

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH, "%s: channel switch to %u in %u tbtt (mode %u) announced\n", __func__, csa_ie->csa_chan, csa_ie->csa_count, csa_ie->csa_mode);

	if (vap->iv_csa_jiffies) {
		/* CSA was received recently */
		if (c != vap->iv_csa_chan) {
			/* XXX abuse? */
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH, "%s: channel switch channel " "changed from %3d (%4d MHz) to %u!\n", __func__, vap->iv_csa_chan->ic_ieee, vap->iv_csa_chan->ic_freq, csa_ie->csa_chan);

			if (vap->iv_csa_count > IEEE80211_CSA_PROTECTION_PERIOD)
				ieee80211_doth_cancel_cs(vap);
			return 0;
		}

		if (csa_ie->csa_mode != vap->iv_csa_mode) {
			/* Can be abused, but with no (to little) impact. */

			/* CS mode change has no influence on our actions since
			 * we don't respect cs modes at all (yet). Complain and
			 * forget. */
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH, "%s: channel switch mode changed from " "%u to %u!\n", __func__, vap->iv_csa_mode, csa_ie->csa_mode);
		}

		if (csa_ie->csa_count >= vap->iv_csa_count) {
			/* XXX abuse? what for? */
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH, "%s: channel switch count didn't " "decrease (%u -> %u)!\n", __func__, vap->iv_csa_count, csa_ie->csa_count);
			return 0;
		}

		{
			u_int32_t elapsed = IEEE80211_JIFFIES_TO_TU(jiffies - vap->iv_csa_jiffies);
			u_int32_t cnt_diff = vap->iv_csa_count - csa_ie->csa_count;
			u_int32_t expected = ni->ni_intval * cnt_diff;
			int32_t delta = elapsed - expected;
			if (delta < 0)
				delta = -delta;
			if (delta > IEEE80211_CSA_SANITY_THRESHOLD) {
				/* XXX abuse? for now, it's safer to cancel CS
				 * than to follow it blindly */
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_DOTH,
						  "%s: %u.%02u bintvals elapsed, "
						  "but count dropped by %u (delta" " = %u TUs)\n", __func__, elapsed / ni->ni_intval, elapsed * 100 / ni->ni_intval % 100, cnt_diff, delta);

				ieee80211_doth_cancel_cs(vap);
				return 0;
			}
		}

		vap->iv_csa_count = csa_ie->csa_count;
		mod_timer(&vap->iv_csa_timer, jiffies + IEEE80211_TU_TO_JIFFIES(vap->iv_csa_count * ni->ni_intval + 10));
	} else {
		/* CSA wasn't received recently, so this is the first one in
		 * the sequence. */

#if 0
		/* Needed for DFS / FCC ... */

		if (csa_ie->csa_count < IEEE80211_CSA_PROTECTION_PERIOD) {
			IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID | IEEE80211_MSG_DOTH, wh, "channel switch", "initial announcement: channel switch" " would occur too soon (in %u tbtt)", csa_ie->csa_count);
			return 0;
		}
#endif

		vap->iv_csa_mode = csa_ie->csa_mode;
		vap->iv_csa_count = csa_ie->csa_count;
		vap->iv_csa_chan = c;

		vap->iv_csa_timer.function = ieee80211_doth_switch_channel_tmr;
		vap->iv_csa_timer.data = (unsigned long)vap;
		vap->iv_csa_timer.expires = jiffies + IEEE80211_TU_TO_JIFFIES(vap->iv_csa_count * ni->ni_intval + 10);
		add_timer(&vap->iv_csa_timer);
	}

	vap->iv_csa_jiffies = jiffies;

	if (vap->iv_csa_count <= 1)
		ieee80211_doth_switch_channel(vap);

	return 0;
}

/* XXX. Not the right place for such a definition */
struct l2_update_frame {
	u8 da[ETH_ALEN];	/* broadcast */
	u8 sa[ETH_ALEN];	/* STA addr */
	__be16 len;		/* 6 */
	u8 dsap;		/* null DSAP address */
	u8 ssap;		/* null SSAP address, CR=Response */
	u8 control;
	u8 xid_info[3];
} __attribute__((packed));

static void ieee80211_deliver_l2_rnr(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct net_device *dev = vap->iv_dev;
	struct sk_buff *skb;
	struct l2_update_frame *l2uf;

	skb = ieee80211_dev_alloc_skb(sizeof(*l2uf));
	if (skb == NULL) {
		return;
	}
	skb_put(skb, sizeof(*l2uf));
	l2uf = (struct l2_update_frame *)(skb->data);
	/* dst: Broadcast address */
	memcpy(l2uf->da, dev->broadcast, ETH_ALEN);
	/* src: associated STA */
	memcpy(l2uf->sa, ni->ni_macaddr, ETH_ALEN);
	l2uf->len = htons(6);
	l2uf->dsap = 0;
	l2uf->ssap = 0;
	l2uf->control = 0xf5;
	l2uf->xid_info[0] = 0x81;
	l2uf->xid_info[1] = 0x80;
	l2uf->xid_info[2] = 0x00;

	skb->dev = dev;
	/* eth_trans_type modifies skb state (skb_pull(ETH_HLEN)), so use
	 * constants instead. We know the packet type anyway. */
	skb->pkt_type = PACKET_BROADCAST;
	skb->protocol = htons(ETH_P_802_2);
	skb_reset_mac_header(skb);

	ieee80211_deliver_data(ni, skb);
	return;
}

static void ieee80211_deliver_l2_xid(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct net_device *dev = vap->iv_dev;
	struct sk_buff *skb;
	struct l2_update_frame *l2uf;

	skb = ieee80211_dev_alloc_skb(sizeof(*l2uf));
	if (skb == NULL) {
		return;
	}
	/* Leak check / cleanup destructor */
	skb_put(skb, sizeof(*l2uf));
	l2uf = (struct l2_update_frame *)(skb->data);
	/* dst: Broadcast address */
	memcpy(l2uf->da, dev->broadcast, ETH_ALEN);
	/* src: associated STA */
	memcpy(l2uf->sa, ni->ni_macaddr, ETH_ALEN);
	l2uf->len = htons(6);
	l2uf->dsap = 0x00;	/* NULL DSAP address */
	l2uf->ssap = 0x01;	/* NULL SSAP address, CR Bit: Response */
	l2uf->control = 0xaf;	/* XID response lsb.1111F101.
				 * F=0 (no poll command; unsolicited frame) */
	l2uf->xid_info[0] = 0x81;	/* XID format identifier */
	l2uf->xid_info[1] = 1;	/* LLC types/classes: Type 1 LLC */
	l2uf->xid_info[2] = 1 << 1;	/* XID sender's receive window size (RW)
					 * FIX: what is correct RW with 802.11? */
	skb->dev = dev;
	/* eth_trans_type modifies skb state (skb_pull(ETH_HLEN)), so use
	 * constants instead. We know the packet type anyway. */
	skb->pkt_type = PACKET_BROADCAST;
	skb->protocol = htons(ETH_P_802_2);
	skb_reset_mac_header(skb);

	ieee80211_deliver_data(ni, skb);
	return;
}

static __inline int contbgscan(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;

	vap->iv_bgscantrintvl = (vap->iv_bgscantrintvl + 1) % 4;
	return ((ic->ic_flags_ext & IEEE80211_FEXT_BGSCAN) && (((ic->ic_flags_ext & IEEE80211_FEXT_BGSCAN_THR) && !vap->iv_bgscantrintvl) || time_after(jiffies, ic->ic_lastdata + vap->iv_bgscanidle)));
}

static __inline int startbgscan(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;

	return ((vap->iv_flags & IEEE80211_F_BGSCAN) &&
		!IEEE80211_IS_CHAN_DTURBO(ic->ic_curchan) && time_after(jiffies, ic->ic_lastscan + vap->iv_bgscanintvl) && time_after(jiffies, ic->ic_lastdata + vap->iv_bgscanidle));
}

/*
 * Context: SoftIRQ
 */
void ieee80211_recv_mgmt(struct ieee80211vap *vap, struct ieee80211_node *ni_or_null, struct sk_buff *skb, int subtype, int rssi, u_int64_t rtsf)
{
#define	ISPROBE(_st)	((_st) == IEEE80211_FC0_SUBTYPE_PROBE_RESP)
#define	ISREASSOC(_st)	((_st) == IEEE80211_FC0_SUBTYPE_REASSOC_RESP)
	struct ieee80211_node *ni = ni_or_null;
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_frame *wh;
	u_int8_t *frm, *efrm;
	u_int8_t *ssid, *rates, *xrates, *suppchan, *wpa, *rsn, *wme, *ath, *mtik;
	u_int8_t rate;
	int reassoc, resp, allocbs = 0, has_erp = 0;
	u_int8_t qosinfo;

	if (ni_or_null == NULL)
		ni = vap->iv_bss;

	wh = (struct ieee80211_frame *)skb->data;
	frm = (u_int8_t *)&wh[1];
	efrm = skb->data + skb->len;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC, "%s: vap:%p[" MAC_FMT "] ni:%p[" MAC_FMT "]\n", __func__, vap, MAC_ADDR(vap->iv_bssid), ni_or_null, MAC_ADDR(wh->i_addr2));

	/* forward management frame to application */
	if (vap->iv_opmode != IEEE80211_M_MONITOR)
		forward_mgmt_to_app(vap, subtype, skb, wh);

	switch (subtype) {
	case IEEE80211_FC0_SUBTYPE_PROBE_RESP:
	case IEEE80211_FC0_SUBTYPE_BEACON:{
			struct ieee80211_scanparams scan;

			/*
			 * We process beacon/probe response frames:
			 *    o when scanning, or
			 *    o station mode when associated (to collect state
			 *      updates such as 802.11g slot time), or
			 *    o adhoc mode (to discover neighbors)
			 *    o ap mode in protection mode (beacons only)
			 * Frames otherwise received are discarded.
			 */
			if (!((ic->ic_flags & IEEE80211_F_SCAN) ||
			      (vap->iv_opmode == IEEE80211_M_STA && ni->ni_associd) || (vap->iv_opmode == IEEE80211_M_IBSS) || ((subtype == IEEE80211_FC0_SUBTYPE_BEACON) && (vap->iv_opmode == IEEE80211_M_HOSTAP)))) {
				vap->iv_stats.is_rx_mgtdiscard++;
				return;
			}
			/*
			 * beacon/probe response frame format
			 *      [8] time stamp
			 *      [2] beacon interval
			 *      [2] capability information
			 *      [tlv] ssid
			 *      [tlv] supported rates
			 *      [tlv] country information
			 *      [tlv] parameter set (FH/DS)
			 *      [tlv] erp information
			 *      [tlv] extended supported rates
			 *      [tlv] WME
			 *      [tlv] WPA or RSN
			 *      [tlv] Atheros Advanced Capabilities
			 */
			IEEE80211_VERIFY_LENGTH(efrm - frm, 12);
			memset(&scan, 0, sizeof(scan));
			scan.isprobe = (subtype == IEEE80211_FC0_SUBTYPE_PROBE_RESP) && IEEE80211_ADDR_EQ(wh->i_addr2, vap->iv_myaddr);
			scan.tstamp = frm;
			frm += 8;
			scan.bintval = le16toh(*(__le16 *)frm);
			frm += 2;
			scan.capinfo = le16toh(*(__le16 *)frm);
			frm += 2;
			scan.bchan = (ieee80211_chan2ieee(ic, ic->ic_curchan) + vap->iv_channelshift) & 0xff;
			scan.chan = scan.bchan;

			while (frm < efrm) {
				/* Agere element in beacon */
				if ((*frm == IEEE80211_ELEMID_AGERE1) || (*frm == IEEE80211_ELEMID_AGERE2)) {
					frm = efrm;
					continue;
				}

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
						IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID, wh, "ERP", "bad len %u", frm[1]);
						vap->iv_stats.is_rx_elem_toobig++;
						break;
					}
					scan.erp = frm[2];
					has_erp = 1;
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
					else if (ismtikoui(frm))
						scan.mtik = frm;
					break;
				case IEEE80211_ELEMID_CHANSWITCHANN:
					if (ic->ic_flags & IEEE80211_F_DOTH)
						scan.csa = frm;
					break;
				default:
					IEEE80211_DISCARD_IE(vap, IEEE80211_MSG_ELEMID, wh, "unhandled", "id %u, len %u", *frm, frm[1]);
//                              printk(KERN_EMERG "id %u, len %u\n", *frm, frm[1]);
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
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID, wh, ieee80211_mgt_subtype_name[subtype >> IEEE80211_FC0_SUBTYPE_SHIFT], "invalid channel %u", scan.chan);
				vap->iv_stats.is_rx_badchan++;
				return;
			}
#endif
			if (scan.chan != scan.bchan && ic->ic_phytype != IEEE80211_T_FH) {
				/*
				 * Frame was received on a channel different from the
				 * one indicated in the DS params element id;
				 * silently discard it.
				 *
				 * NB: this can happen due to signal leakage.
				 *     But we should take it for FH PHY because
				 *     the RSSI value should be correct even for
				 *     different hop pattern in FH.
				 */
//                      printk(KERN_EMERG "for off-channel %u\n", scan.chan);
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID, wh, ieee80211_mgt_subtype_name[subtype >> IEEE80211_FC0_SUBTYPE_SHIFT], "for off-channel %u", scan.chan);
				vap->iv_stats.is_rx_chanmismatch++;
				return;
			}

			/* IEEE802.11 does not specify the allowed range for 
			 * beacon interval. We discard any beacons with a 
			 * beacon interval outside of an arbitrary range in
			 * order to protect against attack.
			 */
			if (!IEEE80211_BINTVAL_VALID(scan.bintval)) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_SCAN, wh, "beacon", "invalid beacon interval (%u)", scan.bintval);
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
			if (vap->iv_opmode == IEEE80211_M_STA && ni->ni_associd != 0 && IEEE80211_ADDR_EQ(wh->i_addr2, vap->iv_bssid)) {
				/* record tsf of last beacon */
				memcpy(ni->ni_tstamp.data, scan.tstamp, sizeof(ni->ni_tstamp));

				/* when rssi falls below the disconnection threshold, drop the connection */
				if ((vap->iv_rssi_dis_thr > 0) && (vap->iv_rssi_dis_max > 0)) {
					if ((rssi > 0) && (rssi < vap->iv_rssi_dis_thr)) {
						if (++vap->iv_rssi_dis_trig > vap->iv_rssi_dis_max) {
							vap->iv_rssi_dis_trig = 0;
							ieee80211_node_leave(ni);
							return;
						}
					} else {
						vap->iv_rssi_dis_trig = 0;
					}
				}

				/* When rssi is low, start doing bgscans more frequently to allow
				 * the supplicant to make a better switching decision */
				if (!(ic->ic_flags & IEEE80211_F_SCAN) && (rssi < vap->iv_bgscanthr) &&
				    (!vap->iv_bgscanthr_next ||
				     !time_before(jiffies, vap->iv_bgscanthr_next)) &&
				    (vap->iv_state == IEEE80211_S_RUN) && time_after(jiffies, vap->iv_lastconnect + msecs_to_jiffies(IEEE80211_BGSCAN_INTVAL_MIN * 1000))) {
					int ret;

					ic->ic_lastdata = 0;
					ic->ic_lastscan = 0;
					ic->ic_flags_ext |= IEEE80211_FEXT_BGSCAN_THR;
					ret = ieee80211_bg_scan(vap);
					if (ret)
						vap->iv_bgscanthr_next = jiffies + msecs_to_jiffies(IEEE80211_BGSCAN_TRIGGER_INTVL * 1000);
				}

				if (ni->ni_intval != scan.bintval) {
					IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "beacon interval divergence: " "was %u, now %u", ni->ni_intval, scan.bintval);
					if (!ni->ni_intval_end) {
						int msecs = 0;	/* silence compiler */
						ni->ni_intval_cnt = 0;
						ni->ni_intval_old = ni->ni_intval;
						msecs = (ni->ni_intval_old * 1024 * 10) / 1000;
						ni->ni_intval_end = jiffies + msecs_to_jiffies(msecs);
						IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "scheduling beacon " "interval measurement " "for %u msecs", msecs);
					}
					if (scan.bintval > ni->ni_intval) {
						ni->ni_intval = scan.bintval;
						vap->iv_flags_ext |= IEEE80211_FEXT_APPIE_UPDATE;
					}
					/* XXX: statistic */
				}
				if (ni->ni_intval_end) {
					if (scan.bintval == ni->ni_intval_old)
						ni->ni_intval_cnt++;
					if (!time_before(jiffies, ni->ni_intval_end)) {
						IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "beacon interval " "measurement finished, " "old value repeated: " "%u times", ni->ni_intval_cnt);
						ni->ni_intval_end = 0;
						if (ni->ni_intval_cnt == 0) {
							IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "reprogramming bmiss " "timer from %u to %u", ni->ni_intval_old, scan.bintval);
							ni->ni_intval = scan.bintval;
							vap->iv_flags_ext |= IEEE80211_FEXT_APPIE_UPDATE;
						} else {
							IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "ignoring the divergence " "(maybe someone tried to " "spoof the AP?)", 0);
						}
					}
					/* XXX statistic */
				}
				if (ni->ni_erp != scan.erp) {
					IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "erp change: was 0x%x, now 0x%x", ni->ni_erp, scan.erp);
					if (scan.erp & IEEE80211_ERP_USE_PROTECTION)
						ic->ic_flags |= IEEE80211_F_USEPROT;
					else
						ic->ic_flags &= ~IEEE80211_F_USEPROT;
					ni->ni_erp = scan.erp;
					/* XXX statistic */
				}
				if ((ni->ni_capinfo ^ scan.capinfo) & IEEE80211_CAPINFO_SHORT_SLOTTIME) {
					IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "capabilities change: was 0x%x, now 0x%x", ni->ni_capinfo, scan.capinfo);
					/*
					 * NB: we assume short preamble doesn't
					 *     change dynamically
					 */
					ieee80211_set_shortslottime(ic, IEEE80211_IS_CHAN_A(ic->ic_bsschan) || (ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME));
					ni->ni_capinfo = scan.capinfo;
					/* XXX statistic */
				}
				if (scan.wme != NULL && (ni->ni_flags & IEEE80211_NODE_QOS)) {
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
				if (scan.csa != NULL || vap->iv_csa_jiffies)
					ieee80211_parse_csaie(ni, scan.csa, wh);
				if (scan.tim != NULL) {
					/*
					 * Check the TIM. For now we drop out of
					 * power save mode for any reason.
					 */
					struct ieee80211_tim_ie *tim = (struct ieee80211_tim_ie *)scan.tim;
					int aid = IEEE80211_AID(ni->ni_associd);
					int ix = aid / NBBY;
					int min = tim->tim_bitctl & ~1;
					int max = tim->tim_len + min - 4;
					if ((tim->tim_bitctl & 1) || (min <= ix && ix <= max && isset(tim->tim_bitmap - min, aid)))
						ieee80211_sta_pwrsave(vap, 0);
					vap->iv_dtim_count = tim->tim_count;
				}

				/* WDS/Repeater: re-schedule software beacon timer for 
				 * STA. Reset consecutive bmiss counter as well */
				IEEE80211_LOCK_IRQ(ic);
				if (vap->iv_state == IEEE80211_S_RUN) {
					vap->iv_bmiss_count = 0;
					if (vap->iv_flags_ext & IEEE80211_FEXT_SWBMISS)
						mod_timer(&vap->iv_swbmiss, jiffies + vap->iv_swbmiss_period);
					else
						del_timer(&vap->iv_swbmiss);
				}
				IEEE80211_UNLOCK_IRQ(ic);

				/* If scanning, pass the info to the scan module.
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
				 * our ap. */
				if (ic->ic_flags & IEEE80211_F_SCAN)
					ieee80211_add_scan(vap, &scan, wh, subtype, rssi, rtsf);
				else if (contbgscan(vap) || startbgscan(vap))
					ieee80211_bg_scan(vap);
				return;
			}

			/* Update AP protection mode when in 11G mode */
			if ((vap->iv_opmode == IEEE80211_M_HOSTAP) && IEEE80211_IS_CHAN_ANYG(ic->ic_curchan)) {

				/* Assume no ERP IE == 11b AP */
				if ((!has_erp || (has_erp && (scan.erp & IEEE80211_ERP_NON_ERP_PRESENT))) && (rssi > ic->ic_protmode_rssi)) {
					struct ieee80211vap *tmpvap;

					if (!(ic->ic_flags & IEEE80211_F_USEPROT)) {
						ic->ic_flags |= IEEE80211_F_USEPROT;
						TAILQ_FOREACH(tmpvap, &ic->ic_vaps, iv_next) {
							tmpvap->iv_flags_ext |= IEEE80211_FEXT_ERPUPDATE;
						}
					}
					ic->ic_protmode_lasttrig = jiffies;
				}
			}

			/*
			 * If scanning, just pass information to the scan module.
			 */
			if (ic->ic_flags & IEEE80211_F_SCAN) {
				ieee80211_add_scan(vap, &scan, wh, subtype, rssi, rtsf);
			}

			/* stop processing if the bss channel is not set up yet */
			if (!ic->ic_bsschan || ic->ic_bsschan == IEEE80211_CHAN_ANYC)
				break;

			/* NB: Behavior of WDS-Link and Ad-Hoc is very similar here:
			 * When we receive a beacon that belongs to the AP that we're
			 * connected to, use it to refresh the local node info.
			 * If no node is found, go through the vap's wds link table
			 * and try to find the sub-vap that is interested in this address
			 */
			if (((vap->iv_opmode == IEEE80211_M_IBSS) &&
			     (scan.capinfo & IEEE80211_CAPINFO_IBSS)) || (((vap->iv_opmode == IEEE80211_M_HOSTAP) || (vap->iv_opmode == IEEE80211_M_WDS)) && (scan.capinfo & IEEE80211_CAPINFO_ESS))) {
				struct ieee80211_node *tni = NULL;
				struct ieee80211vap *avp = NULL;
				int found = 0;

				IEEE80211_LOCK_IRQ(vap->iv_ic);
				if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
					TAILQ_FOREACH(avp, &vap->iv_wdslinks, iv_wdsnext) {
						if (!memcmp(avp->wds_mac, wh->i_addr2, IEEE80211_ADDR_LEN)) {
							if (avp->iv_state != IEEE80211_S_RUN)
								continue;
							if (!avp->iv_wdsnode)
								continue;
							found = 1;
							break;
						}
					}
					if (found)
						tni = ieee80211_ref_node(avp->iv_wdsnode);
				} else if ((vap->iv_opmode == IEEE80211_M_WDS) && vap->iv_wdsnode) {
					found = 1;
					tni = ieee80211_ref_node(vap->iv_wdsnode);
				} else if ((vap->iv_opmode == IEEE80211_M_IBSS) && (vap->iv_state == IEEE80211_S_RUN)) {
					tni = ieee80211_find_node(&ic->ic_sta, wh->i_addr2);
					found = 1;
				}
				IEEE80211_UNLOCK_IRQ(vap->iv_ic);

				if (!found)
					break;

				memcpy(&SKB_CB(skb)->beacon_tsf, scan.tstamp, sizeof(u_int64_t));

				if (tni == NULL) {
					if (avp) {
						IEEE80211_LOCK_IRQ(ic);
						tni = ieee80211_add_neighbor(avp, wh, &scan);
						/* force assoc */
						tni->ni_associd |= 0xc000;
						avp->iv_wdsnode = ieee80211_ref_node(tni);
						IEEE80211_UNLOCK_IRQ(ic);
					} else if ((vap->iv_opmode == IEEE80211_M_IBSS) && IEEE80211_ADDR_EQ(wh->i_addr3, vap->iv_bssid)) {
						/* Create a new entry in the neighbor table. */
						tni = ieee80211_add_neighbor(vap, wh, &scan);
					}
				} else {
					/*
					 * Copy data from beacon to neighbor table.
					 * Some of this information might change after
					 * ieee80211_add_neighbor(), so we just copy
					 * everything over to be safe.
					 */
					tni->ni_esslen = scan.ssid[1];
					memcpy(tni->ni_essid, scan.ssid + 2, scan.ssid[1]);
					IEEE80211_ADDR_COPY(tni->ni_bssid, wh->i_addr3);
					memcpy(tni->ni_tstamp.data, scan.tstamp, sizeof(tni->ni_tstamp));
					tni->ni_inact = tni->ni_inact_reload;
					tni->ni_intval = IEEE80211_BINTVAL_SANITISE(scan.bintval);
					tni->ni_capinfo = scan.capinfo;
					tni->ni_chan = ic->ic_curchan;
					tni->ni_fhdwell = scan.fhdwell;
					tni->ni_fhindex = scan.fhindex;
					tni->ni_erp = scan.erp;
					tni->ni_timoff = scan.timoff;
					if (scan.wme != NULL)
						ieee80211_saveie(&tni->ni_wme_ie, scan.wme);
					if (scan.wpa != NULL)
						ieee80211_saveie(&tni->ni_wpa_ie, scan.wpa);
					if (scan.rsn != NULL)
						ieee80211_saveie(&tni->ni_rsn_ie, scan.rsn);
					if (scan.ath != NULL)
						ieee80211_saveath(tni, scan.ath);
					if (scan.mtik != NULL)
						ieee80211_saveie(&tni->ni_mtik_ie, scan.mtik);

					/* NB: must be after ni_chan is setup */
					ieee80211_setup_rates(tni, scan.rates, scan.xrates, IEEE80211_F_DOSORT);
				}
				if (tni != NULL) {
					tni->ni_rssi = rssi;
					tni->ni_rtsf = rtsf;
					tni->ni_last_rx = jiffies;
					ieee80211_unref_node(&tni);
				}
			}
			break;
		}

	case IEEE80211_FC0_SUBTYPE_PROBE_REQ:
		if (vap->iv_opmode == IEEE80211_M_STA || vap->iv_opmode == IEEE80211_M_AHDEMO || vap->iv_state != IEEE80211_S_RUN) {
			vap->iv_stats.is_rx_mgtdiscard++;
			return;
		}
		if (vap->iv_no_probereq)
			return;
		if (IEEE80211_IS_MULTICAST(wh->i_addr2)) {
			/* frame must be directed */
			vap->iv_stats.is_rx_mgtdiscard++;	/* XXX: stat */
			return;
		}

		/*
		 * XR vap does not process  probe requests.
		 */
#ifdef ATH_SUPERG_XR
		if (vap->iv_flags & IEEE80211_F_XR)
			return;
#endif
		/*
		 * prreq frame format
		 *      [tlv] ssid
		 *      [tlv] supported rates
		 *      [tlv] extended supported rates
		 *      [tlv] Atheros Advanced Capabilities
		 */
		ssid = rates = xrates = ath = mtik = NULL;
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
				else if (ismtikoui(frm))
					mtik = frm;
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
			IEEE80211_DISCARD(vap, IEEE80211_MSG_INPUT, wh, ieee80211_mgt_subtype_name[subtype >> IEEE80211_FC0_SUBTYPE_SHIFT], "%s", "no ssid with ssid suppression enabled");
			vap->iv_stats.is_rx_ssidmismatch++;
			/*XXX*/ return;
		}
		if (ni == vap->iv_bss && vap->iv_opmode != IEEE80211_M_IBSS) {
			ni = ieee80211_dup_bss(vap, wh->i_addr2, 1);
			if (ni == NULL)
				return;
			allocbs = 1;
		}

		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_INPUT, wh->i_addr2, "%s", "recv probe req");
		ni->ni_rssi = rssi;
		ni->ni_rtsf = rtsf;
		ni->ni_last_rx = jiffies;
		rate = ieee80211_setup_rates(ni, rates, xrates, IEEE80211_F_DOSORT | IEEE80211_F_DOFRATE | IEEE80211_F_DONEGO | IEEE80211_F_DODEL);
		if (rate & IEEE80211_RATE_BASIC) {
			IEEE80211_DISCARD(vap, IEEE80211_MSG_XRATE, wh, ieee80211_mgt_subtype_name[subtype >> IEEE80211_FC0_SUBTYPE_SHIFT], "%s", "recv'd rate set invalid");
		} else {
			IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_PROBE_RESP, 0);
		}
		if (allocbs) {
			/*
			 * Temporary node created just to send a
			 * response, reclaim immediately
			 */
			ieee80211_unref_node(&ni);
		} else if (ath != NULL)
			ieee80211_saveath(ni, ath);
		else if (mtik != NULL)
			ieee80211_saveie(&ni->ni_mtik_ie, mtik);
		break;

	case IEEE80211_FC0_SUBTYPE_AUTH:{
			u_int16_t algo, seq, status;
			/*
			 * auth frame format
			 *      [2] algorithm
			 *      [2] sequence
			 *      [2] status
			 *      [tlv*] challenge
			 */
			IEEE80211_VERIFY_LENGTH(efrm - frm, 6);
			algo = le16toh(*(__le16 *)frm);
			seq = le16toh(*(__le16 *)(frm + 2));
			status = le16toh(*(__le16 *)(frm + 4));
#ifdef ATH_SUPERG_XR
			if (!IEEE80211_ADDR_EQ(wh->i_addr3, vap->iv_bssid)) {
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
						/* This would be a stupid place to add 
						 * a node to the table; XR stuff needs 
						 * work anyway. */
						ieee80211_node_reset(ni, vap->iv_xrvap);
					}
					vap = vap->iv_xrvap;
				} else {
					IEEE80211_DISCARD(vap, IEEE80211_MSG_AUTH, wh, "auth", "%s", "not to pier xr bssid");
					return;
				}
			}
#endif
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_AUTH, wh->i_addr2, "recv auth frame with algorithm %d seq %d", algo, seq);
			/* Consult the ACL policy module if setup. */
			if (vap->iv_acl != NULL && !vap->iv_acl->iac_check(vap, wh->i_addr2)) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ACL, wh, "auth", "%s", "disallowed by ACL");
				vap->iv_stats.is_rx_acl++;
				return;
			}
			if (vap->iv_flags & IEEE80211_F_COUNTERM) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_AUTH | IEEE80211_MSG_CRYPTO, wh, "auth", "%s", "TKIP countermeasures enabled");
				vap->iv_stats.is_rx_auth_countermeasures++;
				if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
					ieee80211_send_error(ni, wh->i_addr2, IEEE80211_FC0_SUBTYPE_AUTH, IEEE80211_REASON_MIC_FAILURE);
				}
				return;
			}
			if (algo == IEEE80211_AUTH_ALG_SHARED)
				ieee80211_auth_shared(ni, wh, frm + 6, efrm, rssi, rtsf, seq, status);
			else if (algo == IEEE80211_AUTH_ALG_OPEN)
				ieee80211_auth_open(ni, wh, rssi, rtsf, seq, status);
			else {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, "auth", "unsupported alg %d", algo);
				vap->iv_stats.is_rx_auth_unsupported++;
				if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
					/* XXX not right */
					ieee80211_send_error(ni, wh->i_addr2, IEEE80211_FC0_SUBTYPE_AUTH, (seq + 1) | (IEEE80211_STATUS_ALG << 16));
				}
				return;
			}
			break;
		}

	case IEEE80211_FC0_SUBTYPE_ASSOC_REQ:
	case IEEE80211_FC0_SUBTYPE_REASSOC_REQ:{
			u_int16_t capinfo, bintval;
			struct ieee80211_rsnparms rsn_parm;
			u_int8_t reason;

			if (vap->iv_opmode != IEEE80211_M_HOSTAP || vap->iv_state != IEEE80211_S_RUN) {
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
			 *      [2] capability information
			 *      [2] listen interval
			 *      [6*] current AP address (reassoc only)
			 *      [tlv] ssid
			 *      [tlv] supported rates
			 *      [tlv] extended supported rates
			 *      [tlv] supported channels
			 *      [tlv] wpa or RSN
			 *      [tlv] WME
			 *      [tlv] Atheros Advanced Capabilities
			 */
			IEEE80211_VERIFY_LENGTH(efrm - frm, (reassoc ? 10 : 4));
			if (!IEEE80211_ADDR_EQ(wh->i_addr3, vap->iv_bssid)) {
				IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, ieee80211_mgt_subtype_name[subtype >> IEEE80211_FC0_SUBTYPE_SHIFT], "%s", "wrong bssid");
				vap->iv_stats.is_rx_assoc_bss++;
				return;
			}
			capinfo = le16toh(*(__le16 *)frm);
			frm += 2;
			bintval = le16toh(*(__le16 *)frm);
			frm += 2;
			if (reassoc)
				frm += 6;	/* ignore current AP info */
			ssid = rates = xrates = suppchan = wpa = rsn = wme = ath = mtik = NULL;
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
					/* XXX verify only one of RSN and WPA IEs? */
				case IEEE80211_ELEMID_RSN:
					if (vap->iv_flags & IEEE80211_F_WPA2)
						rsn = frm;
					else
						IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA, "[" MAC_FMT "] ignoring RSN IE " "in association request\n", MAC_ADDR(wh->i_addr2));
					break;
				case IEEE80211_ELEMID_VENDOR:
					/* NB: Provide all IEs for wpa_supplicant, so
					 * it can handle downgrade attacks, etc. */
					if (iswpaoui(frm) && !wpa) {
						if (vap->iv_flags & IEEE80211_F_WPA1)
							wpa = frm;
						else
							IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA, "[" MAC_FMT "] " "ignoring WPA IE in " "association request\n", MAC_ADDR(wh->i_addr2));
					} else if (iswmeinfo(frm))
						wme = frm;
					else if (isatherosoui(frm))
						ath = frm;
					else if (ismtikoui(frm))
						mtik = frm;
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
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, wh->i_addr2, "deny %s request, sta not authenticated", reassoc ? "reassoc" : "assoc");
				ieee80211_send_error(ni, wh->i_addr2, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_ASSOC_NOT_AUTHED);
				vap->iv_stats.is_rx_assoc_notauth++;
				return;
			}

			if (ni->ni_needed_chans != NULL) {
				FREE(ni->ni_needed_chans, M_DEVBUF);
				ni->ni_needed_chans = NULL;
			}
#if 0
			if (ic->ic_flags & IEEE80211_F_DOTH) {
				u_int8_t status;
				status = ieee80211_parse_sc_ie(ni, suppchan, wh);
				if (status != IEEE80211_STATUS_SUCCESS) {
					/* ieee80211_parse_sc_ie already printed dbg msg */
					IEEE80211_SEND_MGMT(ni, resp, status);
					ieee80211_node_leave(ni);	/* XXX */
					vap->iv_stats.is_rx_assoc_badscie++;	/* XXX */
					return;
				}
			}
#endif
			/* Assert right associstion security credentials */
			/* XXX Divy. Incomplete */
			if (wpa == NULL && (ic->ic_flags & IEEE80211_F_WPA)) {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA, "[" MAC_FMT "] no WPA/RSN IE in association request\n", MAC_ADDR(wh->i_addr2));
				IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_RSN_REQUIRED);
				ieee80211_node_leave(ni);
				/* XXX distinguish WPA/RSN? */
				vap->iv_stats.is_rx_assoc_badwpaie++;
				return;
			}

			if (rsn != NULL) {
				/* Initialise values to node defaults, which are then 
				 * overwritten by values in the IE. These are 
				 * installed once association is complete. */
				rsn_parm = ni->ni_rsn;
				if (rsn[0] != IEEE80211_ELEMID_RSN)
					reason = ieee80211_parse_wpa(vap, rsn, &rsn_parm, wh);
				else
					reason = ieee80211_parse_rsn(vap, rsn, &rsn_parm, wh);
				if (reason != 0) {
					IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH, reason);
					ieee80211_node_leave(ni);
					/* XXX distinguish WPA/RSN? */
					vap->iv_stats.is_rx_assoc_badwpaie++;
					return;
				}
				IEEE80211_NOTE_MAC(vap,
						   IEEE80211_MSG_ASSOC | IEEE80211_MSG_WPA,
						   wh->i_addr2,
						   "%s ie: mc %u/%u uc %u/%u key %u caps 0x%x",
						   rsn[0] != IEEE80211_ELEMID_RSN ? "WPA" : "RSN",
						   rsn_parm.rsn_mcastcipher, rsn_parm.rsn_mcastkeylen, rsn_parm.rsn_ucastcipher, rsn_parm.rsn_ucastkeylen, rsn_parm.rsn_keymgmt, rsn_parm.rsn_caps);
			}
			/* discard challenge after association */
			if (ni->ni_challenge != NULL) {
				FREE(ni->ni_challenge, M_DEVBUF);
				ni->ni_challenge = NULL;
			}
			/* 802.11 spec. says to ignore station's privacy bit */
			if ((capinfo & IEEE80211_CAPINFO_ESS) == 0) {
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, wh->i_addr2, "deny %s request, capability mismatch 0x%x", reassoc ? "reassoc" : "assoc", capinfo);
				IEEE80211_SEND_MGMT(ni, resp, IEEE80211_STATUS_CAPINFO);
				ieee80211_node_leave(ni);
				vap->iv_stats.is_rx_assoc_capmismatch++;
				return;
			}
			rate = ieee80211_setup_rates(ni, rates, xrates, IEEE80211_F_DOSORT | IEEE80211_F_DOFRATE | IEEE80211_F_DONEGO | IEEE80211_F_DODEL);
			/*
			 * If constrained to 11g-only stations reject an
			 * 11b-only station.  We cheat a bit here by looking
			 * at the max negotiated xmit rate and assuming anyone
			 * with a best rate <24Mb/s is an 11b station.
			 */
			if ((rate & IEEE80211_RATE_BASIC) || ((vap->iv_flags & IEEE80211_F_PUREG) && rate < 48)) {
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, wh->i_addr2, "deny %s request, rate set mismatch", reassoc ? "reassoc" : "assoc");
				IEEE80211_SEND_MGMT(ni, resp, IEEE80211_STATUS_BASIC_RATE);
				ieee80211_node_leave(ni);
				vap->iv_stats.is_rx_assoc_norate++;
				return;
			}
			if (vap->iv_max_nodes > 0) {
				unsigned int active_nodes = 0;
				struct ieee80211_node *tni;

				IEEE80211_NODE_TABLE_LOCK_IRQ(&ic->ic_sta);
				TAILQ_FOREACH(tni, &ic->ic_sta.nt_node, ni_list) {
					if (tni->ni_vap != vap)
						continue;
					if (tni->ni_associd == 0)
						continue;
					active_nodes++;
				}
				IEEE80211_NODE_TABLE_UNLOCK_IRQ(&ic->ic_sta);

				if (active_nodes >= vap->iv_max_nodes) {
					/* too many nodes connected */
					ieee80211_node_leave(ni);
					return;
				}
			}
			if (ni->ni_associd != 0 && IEEE80211_IS_CHAN_ANYG(ic->ic_bsschan)) {
				if ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME)
				    != (capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME)) {
					IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ANY, wh->i_addr2, "deny %s request, short slot time " "capability mismatch 0x%x", reassoc ? "reassoc" : "assoc", capinfo);
					IEEE80211_SEND_MGMT(ni, resp, IEEE80211_STATUS_CAPINFO);
					ieee80211_node_leave(ni);
					vap->iv_stats.is_rx_assoc_capmismatch++;
					return;
				}
			}

			ni->ni_rssi = rssi;
			ni->ni_rtsf = rtsf;
			ni->ni_last_rx = jiffies;
			ni->ni_intval = IEEE80211_BINTVAL_SANITISE(bintval);
			ni->ni_capinfo = capinfo;
			ni->ni_chan = ic->ic_curchan;
			ni->ni_fhdwell = vap->iv_bss->ni_fhdwell;
			ni->ni_fhindex = vap->iv_bss->ni_fhindex;
			ni->ni_assoctime = jiffies;

			/* WPA */
			ieee80211_saveie(&ni->ni_wpa_ie, wpa);
			/* RSN */
			ni->ni_rsn = rsn_parm;
			ieee80211_saveie(&ni->ni_rsn_ie, rsn);
			/* WME - including QoS flag */
			ieee80211_saveie(&ni->ni_wme_ie, wme);
			ni->ni_flags &= ~IEEE80211_NODE_QOS;
			if ((wme != NULL) && (ieee80211_parse_wmeie(wme, wh, ni) > 0))
				ni->ni_flags |= IEEE80211_NODE_QOS;

			ieee80211_saveath(ni, ath);
			ieee80211_saveie(&ni->ni_mtik_ie, mtik);

			/* Send Receiver Not Ready (RNR) followed by XID for newly 
			 * associated stations. */
			ieee80211_deliver_l2_rnr(ni);
			ieee80211_deliver_l2_xid(ni);
			ieee80211_node_join(ni, resp);
#ifdef ATH_SUPERG_XR
			if (ni->ni_prev_vap && ni->ni_vap != ni->ni_prev_vap && ni->ni_vap->iv_ath_cap & IEEE80211_ATHC_XR) {
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
	case IEEE80211_FC0_SUBTYPE_REASSOC_RESP:{
			u_int16_t capinfo, associd;
			u_int16_t status;

			if (vap->iv_opmode != IEEE80211_M_STA || vap->iv_state != IEEE80211_S_ASSOC) {
				vap->iv_stats.is_rx_mgtdiscard++;
				return;
			}

			/*
			 * asresp frame format
			 *     [2] capability information
			 *     [2] status
			 *     [2] association ID
			 *     [tlv] supported rates
			 *     [tlv] extended supported rates
			 *     [tlv] WME
			 */
			IEEE80211_VERIFY_LENGTH(efrm - frm, 6);
			ni = vap->iv_bss;
			capinfo = le16toh(*(__le16 *)frm);
			frm += 2;
			status = le16toh(*(__le16 *)frm);
			frm += 2;
			if (status != 0) {
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC, wh->i_addr2, "%sassoc failed (reason %d)", ISREASSOC(subtype) ? "re" : "", status);
				vap->iv_stats.is_rx_auth_fail++;	/* XXX */
				ieee80211_new_state(vap, IEEE80211_S_SCAN, IEEE80211_SCAN_FAIL_STATUS);
				return;
			}
			associd = le16toh(*(__le16 *)frm);
			frm += 2;

			rates = xrates = wme = NULL;
			while (frm < efrm) {
				/* 
				 * Do not discard frames containing proprietary Agere
				 * elements 128 and 129, as the reported element length
				 * is often wrong. Skip rest of the frame, since we can
				 * not rely on the given element length making it impossible
				 * to know where the next element starts.
				 */
				if ((*frm == IEEE80211_ELEMID_AGERE1) || (*frm == IEEE80211_ELEMID_AGERE2)) {
					frm = efrm;
					continue;
				}

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
			rate = ieee80211_setup_rates(ni, rates, xrates, IEEE80211_F_DOSORT | IEEE80211_F_DOFRATE | IEEE80211_F_DONEGO | IEEE80211_F_DODEL);
			if (rate & IEEE80211_RATE_BASIC) {
				IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC, wh->i_addr2, "%sassoc failed (rate set mismatch)", ISREASSOC(subtype) ? "re" : "");
				vap->iv_stats.is_rx_assoc_norate++;
				ieee80211_new_state(vap, IEEE80211_S_SCAN, IEEE80211_SCAN_FAIL_STATUS);
				return;
			}

			ni->ni_capinfo = capinfo;
			ni->ni_associd = associd;
			if (wme != NULL && ieee80211_parse_wmeparams(vap, wme, wh, &qosinfo) >= 0) {
				ni->ni_flags |= IEEE80211_NODE_QOS;
				ieee80211_wme_updateparams(vap);
			} else
				ni->ni_flags &= ~IEEE80211_NODE_QOS;
			/*
			 * Configure state now that we are associated.
			 *
			 * XXX may need different/additional driver callbacks?
			 */
			if (IEEE80211_IS_CHAN_A(ic->ic_curchan) || ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_PREAMBLE) && (ic->ic_caps & IEEE80211_C_SHPREAMBLE))) {
				ic->ic_flags |= IEEE80211_F_SHPREAMBLE;
				ic->ic_flags &= ~IEEE80211_F_USEBARKER;
			} else {
				ic->ic_flags &= ~IEEE80211_F_SHPREAMBLE;
				ic->ic_flags |= IEEE80211_F_USEBARKER;
			}
			ieee80211_set_shortslottime(ic, IEEE80211_IS_CHAN_A(ic->ic_curchan) || (ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME));
			/*
			 * Honor ERP protection.
			 *
			 * NB: ni_erp should zero for non-11g operation
			 *     but we check the channel characteristics
			 *     just in case.
			 */
			if (IEEE80211_IS_CHAN_ANYG(ic->ic_curchan) && (ni->ni_erp & IEEE80211_ERP_USE_PROTECTION))
				ic->ic_flags |= IEEE80211_F_USEPROT;
			else
				ic->ic_flags &= ~IEEE80211_F_USEPROT;
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_ASSOC, wh->i_addr2,
					   "%sassoc success: %s preamble, %s slot time%s%s%s%s%s%s%s",
					   ISREASSOC(subtype) ? "re" : "",
					   (ic->ic_flags & IEEE80211_F_SHPREAMBLE) &&
					   (ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_PREAMBLE) ? "short" : "long",
					   ic->ic_flags & IEEE80211_F_SHSLOT ? "short" : "long",
					   ic->ic_flags & IEEE80211_F_USEPROT ? ", protection" : "",
					   ni->ni_flags & IEEE80211_NODE_QOS ? ", QoS" : "",
					   IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_TURBOP) ?
					   ", turbo" : "",
					   IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_COMP) ?
					   ", compression" : "",
					   IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_FF) ?
					   ", fast-frames" : "", IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_XR) ? ", XR" : "", IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_AR) ? ", AR" : "");
			ieee80211_new_state(vap, IEEE80211_S_RUN, subtype);
			break;
		}

	case IEEE80211_FC0_SUBTYPE_DEAUTH:{
			u_int16_t reason;

			if (vap->iv_state == IEEE80211_S_SCAN) {
				vap->iv_stats.is_rx_mgtdiscard++;
				return;
			}
			/*
			 * deauth frame format
			 *        [2] reason
			 */
			IEEE80211_VERIFY_LENGTH(efrm - frm, 2);
			reason = le16toh(*(__le16 *)frm);
			vap->iv_stats.is_rx_deauth++;
			IEEE80211_NODE_STAT(ni, rx_deauth);

			IEEE80211_NOTE(vap, IEEE80211_MSG_AUTH, ni, "recv deauthenticate (reason %d)", reason);
			switch (vap->iv_opmode) {
			case IEEE80211_M_STA:
				ieee80211_new_state(vap, IEEE80211_S_AUTH, wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK);
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

	case IEEE80211_FC0_SUBTYPE_DISASSOC:{
			u_int16_t reason;

			if (vap->iv_state != IEEE80211_S_RUN && vap->iv_state != IEEE80211_S_ASSOC && vap->iv_state != IEEE80211_S_AUTH) {
				vap->iv_stats.is_rx_mgtdiscard++;
				return;
			}
			/*
			 * disassoc frame format
			 *        [2] reason
			 */
			IEEE80211_VERIFY_LENGTH(efrm - frm, 2);
			reason = le16toh(*(__le16 *)frm);
			vap->iv_stats.is_rx_disassoc++;
			IEEE80211_NODE_STAT(ni, rx_disassoc);

			IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni, "recv disassociate (reason %d)", reason);
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
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ANY, wh, "mgt", "subtype 0x%x not handled", subtype);
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
static void ieee80211_recv_pspoll(struct ieee80211_node *ni, struct sk_buff *skb0)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211_frame_min *wh;
	struct sk_buff *skb;
	u_int16_t aid;
	int qlen;

	wh = (struct ieee80211_frame_min *)skb0->data;
	if (ni->ni_associd == 0) {
		IEEE80211_DISCARD(vap, IEEE80211_MSG_POWER | IEEE80211_MSG_DEBUG, (struct ieee80211_frame *)wh, "ps-poll", "%s", "unassociated station");
		vap->iv_stats.is_ps_unassoc++;
		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_NOT_ASSOCED);
		return;
	}

	aid = le16toh(wh->i_dur);
	if (aid != ni->ni_associd) {
		IEEE80211_DISCARD(vap, IEEE80211_MSG_POWER | IEEE80211_MSG_DEBUG, (struct ieee80211_frame *)wh, "ps-poll", "aid mismatch: sta aid 0x%x poll aid 0x%x", ni->ni_associd, aid);
		vap->iv_stats.is_ps_badaid++;
		IEEE80211_SEND_MGMT(ni, IEEE80211_FC0_SUBTYPE_DEAUTH, IEEE80211_REASON_NOT_ASSOCED);
		return;
	}

	/* Okay, take the first queued packet and put it out... */
	IEEE80211_NODE_SAVEQ_LOCK_IRQ(ni);
	IEEE80211_NODE_SAVEQ_DEQUEUE(ni, skb, qlen);
	IEEE80211_NODE_SAVEQ_UNLOCK_IRQ(ni);
	if (skb == NULL) {
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_POWER, wh->i_addr2, "%s", "recv ps-poll, but queue empty");
		ieee80211_send_nulldata(ieee80211_ref_node(ni));
		vap->iv_stats.is_ps_qempty++;	/* XXX node stat */
		if (vap->iv_set_tim != NULL)
			vap->iv_set_tim(ni, 0);	/* just in case */
		return;
	}
	/* 
	 * If there are more packets, set the more packets bit
	 * in the packet dispatched to the station; otherwise
	 * turn off the TIM bit.
	 */
	if (qlen != 0) {
		IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni, "recv ps-poll, send packet, %u still queued", qlen);
		/*
		 * NB: More-data bit will be set during encap.
		 */
	} else {
		IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni, "%s", "recv ps-poll, send packet, queue empty");
		if (vap->iv_set_tim != NULL)
			vap->iv_set_tim(ni, 0);
	}
	M_PWR_SAV_SET(skb);	/* ensure MORE_DATA bit is set correctly */

	ieee80211_parent_queue_xmit(skb);	/* Submit to parent device, including updating stats */
}

#ifdef ATH_SUPERG_FF
static int athff_decap(struct sk_buff *skb)
{
	struct ether_header eh_src, *eh_dst;
	struct llc *llc;

	if (skb->len < (sizeof(struct ether_header) + LLC_SNAPFRAMELEN))
		return -1;

	memcpy(&eh_src, skb->data, sizeof(struct ether_header));
	llc = (struct llc *)skb_pull(skb, sizeof(struct ether_header));
	eh_src.ether_type = llc->llc_un.type_snap.ether_type;
	skb_pull(skb, LLC_SNAPFRAMELEN);

	eh_dst = (struct ether_header *)skb_push(skb, sizeof(struct ether_header));
	memcpy(eh_dst, &eh_src, sizeof(struct ether_header));

	return 0;
}
#endif

#ifdef USE_HEADERLEN_RESV
/*
 * The kernel version of this function alters the skb in a manner
 * inconsistent with dev->hard_header_len header reservation. This 
 * is a rewrite of the portion of eth_type_trans() that we need.
 */
static __be16 ath_eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	struct ethhdr *eth;

	skb_reset_mac_header(skb);
	skb_pull(skb, ETH_HLEN);
	eth = (struct ethhdr *)skb_mac_header(skb);

	if (*eth->h_dest & 1)
		if (memcmp(eth->h_dest, dev->broadcast, ETH_ALEN) == 0)
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	else if (memcmp(eth->h_dest, dev->dev_addr, ETH_ALEN))
		skb->pkt_type = PACKET_OTHERHOST;

	if ((ntohs(eth->h_proto) >= 1536) || (ntohs(eth->h_proto) < 38))
		return eth->h_proto;
	return htons(ETH_P_802_2);
}
#endif

/*
 * Process a frame w/ hw detected MIC failure.
 * The frame will be dropped in any case.
 */
void ieee80211_check_mic(struct ieee80211_node *ni, struct sk_buff *skb)
{
	struct ieee80211vap *vap = ni->ni_vap;

	struct ieee80211_frame *wh;
	struct ieee80211_key *key;
	int hdrspace;
	struct ieee80211com *ic = vap->iv_ic;

	if (skb->len < sizeof(struct ieee80211_frame_min)) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_ANY, ni->ni_macaddr, NULL, "too short (1): len %u", skb->len);
		vap->iv_stats.is_rx_tooshort++;
		return;
	}

	wh = (struct ieee80211_frame *)skb->data;

	hdrspace = ieee80211_hdrspace(ic, wh);
	key = ieee80211_crypto_decap(ni, skb, hdrspace);
	if (key == NULL) {
		/* NB: stats+msgs handled in crypto_decap */
		IEEE80211_NODE_STAT(ni, rx_wepfail);
		return;
	}

	if (!ieee80211_crypto_demic(vap, key, skb, hdrspace, 1)) {
		IEEE80211_DISCARD_MAC(vap, IEEE80211_MSG_INPUT, ni->ni_macaddr, "data", "%s", "demic error");
		IEEE80211_NODE_STAT(ni, rx_demicfail);
	}
	return;
}

EXPORT_SYMBOL(ieee80211_check_mic);

#ifdef IEEE80211_DEBUG
/*
 * Debugging support.
 */

/*
 * Return the bssid of a frame.
 */
static const u_int8_t *ieee80211_getbssid(struct ieee80211vap *vap, const struct ieee80211_frame *wh)
{
	if (vap->iv_opmode == IEEE80211_M_STA)
		return wh->i_addr2;
	if ((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) != IEEE80211_FC1_DIR_NODS)
		return wh->i_addr1;
	if ((wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_PS_POLL)
		return wh->i_addr1;
	return wh->i_addr3;
}

void ieee80211_note(struct ieee80211vap *vap, const char *fmt, ...)
{
	char buf[BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printk(KERN_DEBUG "%s/%s[" MAC_FMT "]: %s", vap->iv_ic->ic_dev->name, vap->iv_dev->name, MAC_ADDR(vap->iv_myaddr), buf);	/* NB: no \n */
}

EXPORT_SYMBOL(ieee80211_note);

void ieee80211_note_frame(struct ieee80211vap *vap, const struct ieee80211_frame *wh, const char *fmt, ...)
{
	char buf[BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printk(KERN_DEBUG "%s/%s[" MAC_FMT "]: " MAC_FMT " %s\n", vap->iv_ic->ic_dev->name, vap->iv_dev->name, MAC_ADDR(vap->iv_myaddr), MAC_ADDR(ieee80211_getbssid(vap, wh)), buf);
}

EXPORT_SYMBOL(ieee80211_note_frame);

void ieee80211_note_mac(struct ieee80211vap *vap, const u_int8_t mac[IEEE80211_ADDR_LEN], const char *fmt, ...)
{
	char buf[BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printk(KERN_DEBUG "%s/%s[" MAC_FMT "]: " MAC_FMT " %s\n", vap->iv_ic->ic_dev->name, vap->iv_dev->name, MAC_ADDR(vap->iv_myaddr), MAC_ADDR(mac), buf);
}

EXPORT_SYMBOL(ieee80211_note_mac);

static void ieee80211_discard_frame(struct ieee80211vap *vap, const struct ieee80211_frame *wh, const char *type, const char *fmt, ...)
{
	char buf[BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printk(KERN_DEBUG "%s/%s[" MAC_FMT "]: " MAC_FMT " discard %s%sframe, %s\n",
	       vap->iv_ic->ic_dev->name, vap->iv_dev->name, MAC_ADDR(vap->iv_myaddr), MAC_ADDR(wh->i_addr2), (type != NULL) ? type : "", (type != NULL) ? " " : "", buf);
}

static void ieee80211_discard_ie(struct ieee80211vap *vap, const struct ieee80211_frame *wh, const char *type, const char *fmt, ...)
{
	char buf[BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printk(KERN_DEBUG "%s/%s[" MAC_FMT "]: "
	       MAC_FMT " discard %s%sinformation element, %s\n",
	       vap->iv_ic->ic_dev->name, vap->iv_dev->name, MAC_ADDR(vap->iv_myaddr), MAC_ADDR(ieee80211_getbssid(vap, wh)), (type != NULL) ? type : "", (type != NULL) ? " " : "", buf);
}

static void ieee80211_discard_mac(struct ieee80211vap *vap, const u_int8_t mac[IEEE80211_ADDR_LEN], const char *type, const char *fmt, ...)
{
	char buf[BUF_LEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printk(KERN_DEBUG "%s/%s[" MAC_FMT "]: " MAC_FMT " discard %s%sframe, %s\n",
	       vap->iv_ic->ic_dev->name, vap->iv_dev->name, MAC_ADDR(vap->iv_myaddr), MAC_ADDR(mac), (type != NULL) ? type : "", (type != NULL) ? " " : "", buf);
}
#endif				/* IEEE80211_DEBUG */
