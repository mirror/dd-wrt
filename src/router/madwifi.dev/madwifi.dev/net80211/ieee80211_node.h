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
 * $Id: ieee80211_node.h 3268 2008-01-26 20:48:11Z mtaylor $
 */
#ifndef _NET80211_IEEE80211_NODE_H_
#define _NET80211_IEEE80211_NODE_H_

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_ioctl.h>	/* for ieee80211_nodestats */
#include <net80211/ieee80211_proto.h>	/* for proto macros on node */

/*
 * Each ieee80211com instance has a single timer that fires once a
 * second.  This is used to initiate various work depending on the
 * state of the instance: scanning (passive or active), ``transition''
 * (waiting for a response to a management frame when operating
 * as a station), and node inactivity processing (when operating
 * as an AP).  For inactivity processing each node has a timeout
 * set in its ni_inact field that is decremented on each timeout
 * and the node is reclaimed when the counter goes to zero.  We
 * use different inactivity timeout values depending on whether
 * the node is associated and authorized (either by 802.1x or
 * open/shared key authentication) or associated but yet to be
 * authorized.  The latter timeout is shorter to more aggressively
 * reclaim nodes that leave part way through the 802.1x exchange.
 */
#define	IEEE80211_INACT_WAIT	15	/* inactivity interval (secs) */
#define	IEEE80211_INACT_INIT	(30/IEEE80211_INACT_WAIT)	/* initial */
#define	IEEE80211_INACT_AUTH	(180/IEEE80211_INACT_WAIT)	/* associated but not authorized */
#define	IEEE80211_INACT_RUN	(300/IEEE80211_INACT_WAIT)	/* authorized */
#define	IEEE80211_INACT_PROBE	(30/IEEE80211_INACT_WAIT)	/* probe */
#define	IEEE80211_INACT_SCAN	(300/IEEE80211_INACT_WAIT)	/* scanned */

#define	IEEE80211_TRANS_WAIT	300	/* mgt frame tx timer (msecs) */

#define	IEEE80211_NODE_HASHSIZE	32
/* simple hash is enough for variation of macaddr */
#define	IEEE80211_NODE_HASH(addr)	\
	(((const u_int8_t *)(addr))[IEEE80211_ADDR_LEN - 1] % \
		IEEE80211_NODE_HASHSIZE)

struct ieee80211_rsnparms {
	u_int8_t rsn_mcastcipher;	/* mcast/group cipher */
	u_int8_t rsn_mcastkeylen;	/* mcast key length */
	u_int8_t rsn_ucastcipherset;	/* unicast cipher set */
	u_int8_t rsn_ucastcipher;	/* selected unicast cipher */
	u_int8_t rsn_ucastkeylen;	/* unicast key length */
	u_int8_t rsn_keymgmtset;	/* key management algorithms */
	u_int8_t rsn_keymgmt;	/* selected key mgmt algo */
	u_int16_t rsn_caps;	/* capabilities */
};

struct ieee80211_node_table;
struct ieee80211com;
struct ieee80211vap;
struct ath_buf;
struct ath_softc;

/*
 * Node specific information.  Note that drivers are expected
 * to derive from this structure to add device-specific per-node
 * state.  This is done by overriding the ic_node_* methods in
 * the ieee80211com structure.
 */
struct ieee80211_node {
	struct ieee80211vap *ni_vap, *ni_subif;
	struct ieee80211com *ni_ic;
	struct ieee80211_node_table *ni_table;
	 TAILQ_ENTRY(ieee80211_node) ni_list;
	 LIST_ENTRY(ieee80211_node) ni_hash;
	struct work_struct ni_create;	/* task for creating a subif */
	struct work_struct ni_destroy;	/* task for destroying a subif */
	atomic_t ni_refcnt;
	u_int ni_scangen;	/* gen# for timeout scan */
	u_int8_t ni_authmode;	/* authentication algorithm */
	u_int16_t ni_flags;	/* special-purpose state */
#define	IEEE80211_NODE_AUTH	0x0001	/* authorized for data */
#define	IEEE80211_NODE_QOS	0x0002	/* QoS enabled */
#define	IEEE80211_NODE_ERP	0x0004	/* ERP enabled */
/* NB: this must have the same value as IEEE80211_FC1_PWR_MGT */
#define	IEEE80211_NODE_PWR_MGT	0x0010	/* power save mode enabled */
#define	IEEE80211_NODE_AREF	0x0020	/* authentication ref held */
#define IEEE80211_NODE_UAPSD	0x0040	/* U-APSD power save enabled */
#define IEEE80211_NODE_UAPSD_TRIG 0x0080	/* U-APSD triggerable state */
#define IEEE80211_NODE_UAPSD_SP	0x0100	/* U-APSD SP in progress */
#define IEEE80211_NODE_PS_CHANGED	0x0200	/* PS state change */
	u_int8_t ni_ath_flags;	/* Atheros feature flags */
	/* NB: These must have the same values as IEEE80211_ATHC_* */
#define IEEE80211_NODE_TURBOP	0x0001	/* Turbo prime enable */
#define IEEE80211_NODE_COMP	0x0002	/* Compresssion enable */
#define IEEE80211_NODE_FF	0x0004	/* Fast Frame capable */
#define IEEE80211_NODE_XR	0x0008	/* Atheros WME enable */
#define IEEE80211_NODE_AR	0x0010	/* AR capable */
#define IEEE80211_NODE_BOOST	0x0080
	u_int16_t ni_ath_defkeyindex;	/* Atheros def key index */
#define IEEE80211_INVAL_DEFKEY	0x7FFF
	u_int16_t ni_associd;	/* assoc response */
	u_int16_t ni_txpower;	/* current transmit power (in 0.5 dBm) */
	u_int16_t ni_vlan;	/* vlan tag */
	u_int32_t *ni_challenge;	/* shared-key challenge */
	u_int8_t *ni_wpa_ie;	/* captured WPA ie */
	u_int8_t *ni_rsn_ie;	/* captured RSN ie */
	u_int8_t *ni_wme_ie;	/* captured WME ie */
	u_int8_t *ni_ath_ie;	/* captured Atheros ie */
#define IEEE80211_NODE_IS_MTIKWDS(_ni) ((_ni)->ni_mtik_ie && ((_ni)->ni_mtik_ie[10] & 4))	/* has mtik ie and flagged as wds */
	u_int8_t *ni_mtik_ie;	/* captured Mikrotik ie */
	u_int8_t *ni_suppchans;	/* supported channels */
	u_int8_t *ni_suppchans_new;	/* supported channels of ongoing association */
	u_int8_t *ni_needed_chans;	/* nodes which don't support these will be removed */
	u_int8_t ni_n_needed_chans;	/* size of ni_needed_chans list */
	u_int16_t ni_txseqs[17];	/* tx seq per-tid */
	u_int16_t ni_rxseqs[17];	/* rx seq previous per-tid */
	u_int32_t ni_rxfragstamp;	/* time stamp of last rx frag */
	struct sk_buff *ni_rxfrag;	/* rx frag reassembly */
	struct ieee80211_rsnparms ni_rsn;	/* RSN/WPA parameters */
	struct ieee80211_key ni_ucastkey;	/* unicast key */
	int ni_rxkeyoff;	/* Receive key offset */

	/* hardware */
	u_int64_t ni_rtsf;	/* recv timestamp */
	u_int32_t ni_last_rx;	/* recv jiffies */
	u_int8_t ni_rssi;	/* recv ssi */

	/* header */
	u_int8_t ni_macaddr[IEEE80211_ADDR_LEN];
	u_int8_t ni_bssid[IEEE80211_ADDR_LEN];

	/* beacon, probe response */
	union {
		u_int8_t data[8];
		__le64 tsf;
	} ni_tstamp;		/* from last rcv'd beacon */

	u_int16_t ni_intval;	/* beacon interval */
	u_int16_t ni_intval_old;	/* beacon interval before first change */
	u_int16_t ni_intval_cnt;	/* count of ni_intval != ni_intval_old */
	unsigned long ni_intval_end;	/* end of transition interval jiffies */

	u_int16_t ni_capinfo;	/* capabilities */
	u_int8_t ni_esslen;
	u_int8_t ni_essid[IEEE80211_NWID_LEN];
	struct ieee80211_rateset ni_rates;	/* negotiated rate set */
	struct ieee80211_channel *ni_chan;
	u_int16_t ni_fhdwell;	/* FH only */
	u_int8_t ni_fhindex;	/* FH only */
	u_int8_t ni_erp;	/* ERP from beacon/probe resp */
	u_int16_t ni_timoff;	/* byte offset to TIM ie */

	/* others */
	struct sk_buff_head ni_savedq;	/* packets queued for pspoll */
	short ni_inact;		/* inactivity mark count */
	short ni_inact_reload;	/* inactivity reload value */
	int ni_txrate;		/* index to ni_rates[] */
	int ni_rxrate;		/* index to ni_rates[] */
	struct ieee80211_nodestats ni_stats;	/* per-node statistics */
	struct ieee80211vap *ni_prev_vap;	/* previously associated vap */
	u_int8_t ni_uapsd;	/* U-APSD per-node flags matching WMM STA Qos Info field */
	u_int8_t ni_uapsd_maxsp;	/* maxsp from flags above */
	u_int16_t ni_uapsd_trigseq[WME_NUM_AC];	/* trigger suppression on retry */
	u_int32_t ni_assoctime;	/* sta association time */
	__le16 ni_pschangeseq;
};
MALLOC_DECLARE(M_80211_NODE);

#define	IEEE80211_NODE_AID(ni)			IEEE80211_AID(ni->ni_associd)

#define	IEEE80211_NODE_STAT(ni,stat)		(ni->ni_stats.ns_##stat++)
#define	IEEE80211_NODE_STAT_ADD(ni,stat,v)	(ni->ni_stats.ns_##stat += v)
#define	IEEE80211_NODE_STAT_SET(ni,stat,v)	(ni->ni_stats.ns_##stat = v)

#define WME_UAPSD_AC_CAN_TRIGGER(_ac, _ni) (				\
		((_ni)->ni_flags & IEEE80211_NODE_UAPSD_TRIG) &&	\
		WME_UAPSD_AC_ENABLED((_ac), (_ni)->ni_uapsd))
#define WME_UAPSD_NODE_MAXQDEPTH	8
#define IEEE80211_NODE_UAPSD_USETIM(_ni) (((_ni)->ni_uapsd & 0xF) == 0xF)
#define WME_UAPSD_NODE_INVALIDSEQ	0xffff
#define WME_UAPSD_NODE_TRIGSEQINIT(_ni)					\
		(memset(&(_ni)->ni_uapsd_trigseq[0],			\
		 0xff, sizeof((_ni)->ni_uapsd_trigseq)))

void ieee80211_node_attach(struct ieee80211com *);
void ieee80211_node_detach(struct ieee80211com *);
void ieee80211_node_vattach(struct ieee80211vap *);
void ieee80211_node_latevattach(struct ieee80211vap *);
void ieee80211_node_vdetach(struct ieee80211vap *);

static __inline int ieee80211_node_is_authorized(const struct ieee80211_node *ni)
{
	return (ni->ni_flags & IEEE80211_NODE_AUTH);
}

void ieee80211_node_authorize(struct ieee80211_node *);
void ieee80211_node_unauthorize(struct ieee80211_node *);

void ieee80211_create_ibss(struct ieee80211vap *, struct ieee80211_channel *);
void ieee80211_reset_bss(struct ieee80211vap *);
int ieee80211_ibss_merge(struct ieee80211_node *);
struct ieee80211_scan_entry;
int ieee80211_sta_join(struct ieee80211vap *, const struct ieee80211_scan_entry *);
void ieee80211_sta_join1_tasklet(IEEE80211_TQUEUE_ARG);
void ieee80211_sta_leave(struct ieee80211_node *);

#define WDS_AGING_TIME		600	/* 10 minutes */
#define WDS_AGING_COUNT 	2
#define WDS_AGING_STATIC 	0xffff
#define WDS_AGING_TIMER_VAL 	(WDS_AGING_TIME / 2)

/*
 * Table of ieee80211_node instances.  Each ieee80211com
 * has at least one for holding the scan candidates.
 * When operating as an access point or in ibss mode there
 * is a second table for associated stations or neighbors.
 */
struct ieee80211_node_table {
	const char *nt_name;	/* for debugging */
	struct ieee80211com *nt_ic;	/* back reference */
	ieee80211_node_table_lock_t nt_nodelock;	/* on node table */
	 TAILQ_HEAD(, ieee80211_node) nt_node;	/* information of all nodes */
	 ATH_LIST_HEAD(, ieee80211_node) nt_hash[IEEE80211_NODE_HASHSIZE];
	ieee80211_scan_lock_t nt_scanlock;	/* on nt_scangen */
	u_int nt_scangen;	/* gen# for timeout scan */
	int nt_inact_init;	/* initial node inact setting */
};

/* Allocates a new ieee80211_node* that has a reference count of one, and 
 * adds it to the node table. */
#ifdef IEEE80211_DEBUG_REFCNT
#define ieee80211_alloc_node_table(_vap, _mac) \
	ieee80211_alloc_node_table_debug(_vap, _mac, __func__, __LINE__)
struct ieee80211_node *ieee80211_alloc_node_table_debug(struct ieee80211vap *, const u_int8_t *, const char *name, int line);
#else
struct ieee80211_node *ieee80211_alloc_node_table(struct ieee80211vap *, const u_int8_t *);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

/* Allocates a new ieee80211_node* that has a reference count.  
 * If tmp is 0, it is added to the node table and the reference is used.
 * If tmp is 1, then the caller gets to use the reference. */
#ifdef IEEE80211_DEBUG_REFCNT
#define ieee80211_dup_bss(_vap, _mac, _tmp) \
	ieee80211_dup_bss_debug(_vap, _mac, _tmp, __func__, __LINE__)
struct ieee80211_node *ieee80211_dup_bss_debug(struct ieee80211vap *, const u_int8_t *, unsigned char tmp, const char *, int);
#else
struct ieee80211_node *ieee80211_dup_bss(struct ieee80211vap *, const u_int8_t *, unsigned char tmp);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

void ieee80211_node_reset(struct ieee80211_node *, struct ieee80211vap *);

/* Returns a ieee80211_node* with refcount incremented, if found */
#ifdef IEEE80211_DEBUG_REFCNT
#define	ieee80211_find_node(_nt, _mac) \
	ieee80211_find_node_debug(_nt, _mac, __func__, __LINE__)
struct ieee80211_node *ieee80211_find_node_debug(struct ieee80211_node_table *, const u_int8_t *, const char *, int);
#else
struct ieee80211_node *ieee80211_find_node(struct ieee80211_node_table *, const u_int8_t *);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

struct ieee80211vap *ieee80211_find_rxvap(struct ieee80211com *ic, const u_int8_t *mac);

/* Returns a ieee80211_node* with refcount incremented, if found */
#ifdef IEEE80211_DEBUG_REFCNT
#define	ieee80211_find_rxnode(_nt, _vap, _wh) \
	ieee80211_find_rxnode_debug(_nt, _vap, _wh, __func__, __LINE__)
struct ieee80211_node *ieee80211_find_rxnode_debug(struct ieee80211com *, struct ieee80211vap *, const struct ieee80211_frame_min *, const char *, int);
#else
struct ieee80211_node *ieee80211_find_rxnode(struct ieee80211com *, struct ieee80211vap *, const struct ieee80211_frame_min *);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

/* Returns a ieee80211_node* with refcount incremented, if found */
#ifdef IEEE80211_DEBUG_REFCNT
#define	ieee80211_find_txnode(_nt, _mac) \
	ieee80211_find_txnode_debug(_nt, _mac, __func__, __LINE__)
struct ieee80211_node *ieee80211_find_txnode_debug(struct ieee80211vap *, const u_int8_t *, const char *, int);
#else
struct ieee80211_node *ieee80211_find_txnode(struct ieee80211vap *, const u_int8_t *);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

#ifdef IEEE80211_DEBUG_REFCNT
#define ieee80211_free_node(_ni) \
	ieee80211_free_node_debug(_ni, __func__, __LINE__)
void ieee80211_free_node_debug(struct ieee80211_node *ni, const char *func, int line);
#else
void ieee80211_free_node(struct ieee80211_node *ni);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

/* Reference counting only needs to be locked out against the transitions,
 * 0->1 and 1->0 (i.e., when we do not own the reference we are getting).
 * This only happens when finding the a node reference from the node table,
 * which is locked seperately. Thus, we do not need to lock the follwoing 
 * functions. 
 * Increment the reference counter for ieee80211_node*
 */
#ifdef IEEE80211_DEBUG_REFCNT
#define ieee80211_ref_node(_ni) \
	ieee80211_ref_node_debug(_ni, __func__, __LINE__)
struct ieee80211_node *ieee80211_ref_node_debug(struct ieee80211_node *ni, const char *func, int line);
#else
struct ieee80211_node *ieee80211_ref_node(struct ieee80211_node *ni);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

#define PASS_NODE(_ni) \
	ieee80211_pass_node(&_ni)

static __inline struct ieee80211_node *ieee80211_pass_node(struct ieee80211_node **pni)
{
	struct ieee80211_node *tmp = *pni;
	*pni = NULL;
	return (tmp);
}

/* Decrement ieee80211_node* refcount, and relinquish the pointer. */
#ifdef IEEE80211_DEBUG_REFCNT
#define ieee80211_unref_node(_pni) \
	ieee80211_unref_node_debug(_pni, __func__, __LINE__)
void ieee80211_unref_node_debug(struct ieee80211_node **pni, const char *func, int line);
#else
void ieee80211_unref_node(struct ieee80211_node **pni);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */

typedef void ieee80211_iter_func(void *, struct ieee80211_node *);
void ieee80211_iterate_nodes(struct ieee80211_node_table *, ieee80211_iter_func *, void *);
void ieee80211_iterate_dev_nodes(struct net_device *, struct ieee80211_node_table *, ieee80211_iter_func *, void *);
void ieee80211_dump_node(struct ieee80211_node_table *, struct ieee80211_node *);
void ieee80211_dump_nodes(struct ieee80211_node_table *);
/* Returns a node with refcount of one.  Caller must release that reference */
#ifdef IEEE80211_DEBUG_REFCNT
#define ieee80211_fakeup_adhoc_node(_vap, _mac) \
	ieee80211_fakeup_adhoc_node_debug(_vap, _mac, __func__, __LINE__)
struct ieee80211_node *ieee80211_fakeup_adhoc_node_debug(struct ieee80211vap *, const u_int8_t macaddr[], const char *, int);
#else
struct ieee80211_node *ieee80211_fakeup_adhoc_node(struct ieee80211vap *, const u_int8_t macaddr[]);
#endif				/* #ifdef IEEE80211_DEBUG_REFCNT */
struct ieee80211_scanparams;
/* Returns a node with refcount of one.  Caller must release that reference */
struct ieee80211_node *ieee80211_add_neighbor(struct ieee80211vap *, const struct ieee80211_frame *, const struct ieee80211_scanparams *);
/* Increments reference count of node */
void ieee80211_node_join(struct ieee80211_node *, int);
/* Decrements reference count of node */
void ieee80211_node_leave(struct ieee80211_node *);
u_int8_t ieee80211_getrssi(struct ieee80211com *);
int32_t ieee80211_get_node_count(struct ieee80211com *);
void ieee80211_wds_addif(struct ieee80211_node *ni);
#endif				/* _NET80211_IEEE80211_NODE_H_ */
