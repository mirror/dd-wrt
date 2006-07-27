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
 * $Id: ieee80211_var.h 1512 2006-04-18 17:43:09Z dyqith $
 */
#ifndef _NET80211_IEEE80211_VAR_H_
#define _NET80211_IEEE80211_VAR_H_

/*
 * Definitions for IEEE 802.11 drivers.
 */
#define	IEEE80211_DEBUG
#undef	IEEE80211_DEBUG_REFCNT			/* node refcnt stuff */

#include <net80211/ieee80211_linux.h>

#include <sys/queue.h>

#include <net80211/_ieee80211.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>		/* for ieee80211_stats */
#include <net80211/ieee80211_node.h>
#include <net80211/ieee80211_power.h>
#include <net80211/ieee80211_proto.h>
#include <net80211/ieee80211_scan.h>

#define	IEEE80211_TXPOWER_MAX	100	/* .5 dBm units */
#define	IEEE80211_TXPOWER_MIN	0	/* kill radio */

#define	IEEE80211_DTIM_MAX	15	/* max DTIM period */
#define	IEEE80211_DTIM_MIN	1	/* min DTIM period */
#define	IEEE80211_DTIM_DEFAULT	1	/* default DTIM period */

#define	IEEE80211_BINTVAL_MAX	500	/* max beacon interval (TU's) */
#define	IEEE80211_BINTVAL_MIN	25	/* min beacon interval (TU's) */
#define	IEEE80211_BINTVAL_DEFAULT 100	/* default beacon interval (TU's) */

#define	IEEE80211_BGSCAN_INTVAL_MIN	15	/* min bg scan intvl (secs) */
#define	IEEE80211_BGSCAN_INTVAL_DEFAULT	(5*60)	/* default bg scan intvl */

#define	IEEE80211_BGSCAN_IDLE_MIN	100	/* min idle time (ms) */
#define	IEEE80211_BGSCAN_IDLE_DEFAULT	250	/* default idle time (ms) */

#define IEEE80211_COVERAGE_CLASS_MAX	31	/* max coverage class */
#define IEEE80211_REGCLASSIDS_MAX	10	/* max regclass id list */

#define	IEEE80211_PS_SLEEP	0x1	/* STA is in power saving mode */
#define	IEEE80211_PS_MAX_QUEUE	50	/* maximum saved packets */

#define	IEEE80211_XR_BEACON_FACTOR	3	/* factor between xr Beacon interval and normal beacon interval */
#define	IEEE80211_XR_DEFAULT_RATE_INDEX	0	
#define	IEEE80211_XR_FRAG_THRESHOLD	540	

#define	IEEE80211_FIXED_RATE_NONE	-1

#define IEEE80211_SWBMISS_THRESHOLD	10 /* software beacon miss threshold, in TU's */
#define IEEE80211_BMISS_COUNT_MAX  2 /* maximum consecutive bmiss allowed */

#define	IEEE80211_MS_TO_TU(x)	(((x) * 1000) / 1024)
#define	IEEE80211_TU_TO_MS(x)	(((x) * 1024) / 1000)
#define	IEEE80211_TU_TO_JIFFIES(x) ((IEEE80211_TU_TO_MS(x) * HZ) / 1000)

#define IEEE80211_PWRCONSTRAINT_VAL(ic) \
	(((ic)->ic_bsschan->ic_maxregpower > (ic)->ic_curchanmaxpwr) ? \
	    (ic)->ic_bsschan->ic_maxregpower - (ic)->ic_curchanmaxpwr : 0)

/*
 * 802.11 control state is split into a common portion that maps
 * 1-1 to a physical device and one or more "Virtual AP's" (VAP)
 * that are bound to an ieee80211com instance and share a single
 * underlying device.  Each VAP has a corresponding OS device
 * entity through which traffic flows and that applications use
 * for issuing ioctls, etc.
 */

/*
 * Data common to one or more virtual AP's.  State shared by
 * the underlying device and the net80211 layer is exposed here;
 * e.g. device-specific callbacks.
 */
struct ieee80211vap;

struct ieee80211com {
	struct net_device *ic_dev;		/* associated device */
	ieee80211com_lock_t ic_comlock;		/* state update lock */
	ieee80211com_lock_t ic_vapslock; 	/* vap state machine lock */
	TAILQ_HEAD(, ieee80211vap) ic_vaps;	/* list of vap instances */
	enum ieee80211_phytype ic_phytype;	/* XXX wrong for multi-mode */
	enum ieee80211_opmode ic_opmode;	/* operation mode */
	struct ifmedia ic_media;			/* interface media config */
	u_int8_t ic_myaddr[IEEE80211_ADDR_LEN];
	struct timer_list ic_inact;		/* mgmt/inactivity timer */

	u_int32_t ic_flags;			/* state flags */
	u_int32_t ic_flags_ext;			/* extension of state flags */
	u_int32_t ic_caps;			/* capabilities */
	u_int8_t ic_ath_cap;			/* Atheros adv. capablities */
	u_int8_t ic_promisc;			/* vap's needing promisc mode */
	u_int8_t ic_allmulti;			/* vap's needing all multicast*/
	u_int8_t ic_nopened;			/* vap's been opened */
	struct ieee80211_rateset ic_sup_rates[IEEE80211_MODE_MAX];
	struct ieee80211_rateset ic_sup_xr_rates;
	struct ieee80211_rateset ic_sup_half_rates;
	struct ieee80211_rateset ic_sup_quarter_rates;
	u_int16_t ic_modecaps;			/* set of mode capabilities */
	u_int16_t ic_curmode;			/* current mode */
	u_int16_t ic_lintval;			/* beacon interval */
	u_int16_t ic_holdover;			/* PM hold over duration */
	u_int16_t ic_bmisstimeout;		/* beacon miss threshold (ms) */
	u_int16_t ic_txpowlimit; 		/* global tx power limit (in 0.5 dBm) */
	u_int16_t ic_newtxpowlimit; 		/* tx power limit to change to (in 0.5 dBm) */
	u_int16_t ic_uapsdmaxtriggers; 		/* max triggers that could arrive */
	u_int8_t ic_coverageclass; 		/* coverage class */

	/*
	 * Channel state:
	 *
	 * ic_channels is the set of available channels for the device;
	 *    it is setup by the driver
	 * ic_nchans is the number of valid entries in ic_channels
	 * ic_chan_avail is a bit vector of these channels used to check
	 *    whether a channel is available w/o searching the channel table.
	 * ic_chan_active is a (potentially) constrained subset of
	 *    ic_chan_avail that reflects any mode setting or user-specified
	 *    limit on the set of channels to use/scan
	 * ic_curchan is the current channel the device is set to; it may
	 *    be different from ic_bsschan when we are off-channel scanning
	 *    or otherwise doing background work
	 * ic_bsschan is the channel selected for operation; it may
	 *    be undefined (IEEE80211_CHAN_ANYC)
	 * ic_prevchan is a cached ``previous channel'' used to optimize
	 *    lookups when switching back+forth between two channels
	 *    (e.g. for dynamic turbo)
	 */
	int ic_nchans;				/* # entries in ic_channels */
	struct ieee80211_channel ic_channels[IEEE80211_CHAN_MAX+1];
	u_int8_t ic_chan_avail[IEEE80211_CHAN_BYTES];
	u_int8_t ic_chan_active[IEEE80211_CHAN_BYTES];
	struct ieee80211_channel *ic_curchan;	/* current channel */
	struct ieee80211_channel *ic_bsschan;	/* bss channel */
	struct ieee80211_channel *ic_prevchan;	/* previous channel */

	/* regulatory class ids */
	u_int ic_nregclass;			/* # entries in ic_regclassids */
	u_int8_t ic_regclassids[IEEE80211_REGCLASSIDS_MAX];

	u_int8_t		ic_isdfsregdomain; /* is this DFS regdomain? */

	/* scan-related state */
	struct ieee80211_scan_state *ic_scan;	/* scan state */
	enum ieee80211_roamingmode ic_roaming;	/* roaming mode */
	unsigned long ic_lastdata;		/* time of last data frame */
	unsigned long ic_lastscan;		/* time last scan completed */

	/* NB: this is the union of all vap stations/neighbors */
	struct ieee80211_node_table ic_sta;	/* stations/neighbors */

	/* XXX multi-bss: split out common/vap parts? */
	struct ieee80211_wme_state ic_wme;	/* WME/WMM state */

	/* XXX multi-bss: can per-vap be done/make sense? */
	enum ieee80211_protmode	ic_protmode;	/* 802.11g protection mode */
	u_int16_t ic_nonerpsta;			/* # non-ERP stations */
	u_int16_t ic_longslotsta;		/* # long slot time stations */
	u_int16_t ic_sta_assoc;			/* stations associated */
	u_int16_t ic_dt_sta_assoc;		/* dturbo capable stations */
	u_int16_t ic_xr_sta_assoc;		/* XR stations associated */
	
	/*
	 * Spectrum Management.
	 */
	u_int16_t ic_country_code;
	int ic_country_outdoor;
	struct ieee80211_ie_country ic_country_ie; /* country info element */
	/* 
	 *  current channel max power, used to compute Power Constraint IE.
	 *
	 *  NB: local power constraint depends on the channel, but assuming it must
	 *     be detected dynamically, we cannot maintain a table (i.e., will not
	 *     know value until change to channel and detect).
	 */
	u_int8_t ic_curchanmaxpwr;
	u_int8_t ic_chanchange_tbtt;
	u_int8_t ic_chanchange_chan;

	/* virtual ap create/delete */
	struct ieee80211vap *(*ic_vap_create)(struct ieee80211com *,
		const char *, int, int, int, struct net_device *);
	void (*ic_vap_delete)(struct ieee80211vap *);
	/* send/recv 802.11 management frame */
	int (*ic_send_mgmt)(struct ieee80211_node *, int, int);
	void (*ic_recv_mgmt)(struct ieee80211_node *, struct sk_buff *, int,
		int, u_int32_t);
	/* send management frame to driver, like hardstart */
	int (*ic_mgtstart)(struct ieee80211com *, struct sk_buff *);
	/* reset device state after 802.11 parameter/state change */
	int (*ic_init)(struct net_device *);
	int (*ic_reset)(struct net_device *);
	/* update device state for 802.11 slot time change */
	void (*ic_updateslot)(struct net_device *);
	/* new station association callback/notification */
	void (*ic_newassoc)(struct ieee80211_node *, int);
	/* node state management */
	struct ieee80211_node *(*ic_node_alloc)(struct ieee80211_node_table *,
		struct ieee80211vap *);
	void (*ic_node_free)(struct ieee80211_node *);
	void (*ic_node_cleanup)(struct ieee80211_node *);
	u_int8_t (*ic_node_getrssi)(const struct ieee80211_node *);
	u_int8_t (*ic_node_move_data)(const struct ieee80211_node *);

	/* scanning support */
	void (*ic_scan_start)(struct ieee80211com *);
	void (*ic_scan_end)(struct ieee80211com *);
	void (*ic_set_channel)(struct ieee80211com *);

	int16_t (*ic_getnoise)(struct ieee80211com *);

	int16_t (*ic_getchannelnoise)(struct ieee80211com *,struct ieee80211_channel *);

	/* u-apsd support */
	void (*ic_uapsd_flush)(struct ieee80211_node *);

	/* set coverage class */
	void (*ic_set_coverageclass)(struct ieee80211com *);

	/* mhz to ieee conversion */
	u_int (*ic_mhz2ieee)(struct ieee80211com *, u_int, u_int);
};

struct vlan_group;
struct eapolcom;
struct ieee80211_aclator;

struct ieee80211_nsparams {
	enum ieee80211_state newstate;
	int arg;
	int result;
};

#define IW_MAX_SPY 8
struct ieee80211_spy {
        u_int8_t mac[IW_MAX_SPY * IEEE80211_ADDR_LEN];
        u_int8_t thr_low;	/* 1 byte rssi value, 0 = threshold is off */
        u_int8_t thr_high;	/* 1 byte rssi value */   
        u_int8_t num;
};

#ifdef CONFIG_SYSCTL
#define MAX_PROC_IEEE80211_SIZE 16383
#define PROC_IEEE80211_PERM 0644

struct proc_ieee80211_priv {
     int rlen;
     int max_rlen;
     char *rbuf;

     int wlen;
     int max_wlen;
     char *wbuf;
};

struct ieee80211_proc_entry {
	char *name;
	struct file_operations *fileops;
	struct proc_dir_entry *entry;
	struct ieee80211_proc_entry *next;
};
#endif

struct ieee80211vap {
	struct net_device *iv_dev;		/* associated device */
	struct net_device_stats	iv_devstats;	/* interface statistics */
	struct ifmedia iv_media;			/* interface media config */
#ifdef CONFIG_NET_WIRELESS
	struct iw_statistics iv_iwstats;		/* wireless statistics block */
#endif
#ifdef CONFIG_SYSCTL
	struct ctl_table_header	*iv_sysctl_header;
	struct ctl_table *iv_sysctls;
	struct proc_dir_entry *iv_proc;
	struct ieee80211_proc_entry *iv_proc_entries;
#endif
	struct vlan_group *iv_vlgrp;		/* vlan group state */

	TAILQ_ENTRY(ieee80211vap) iv_next;	/* list of vap instances */
	u_int iv_unit;				/* virtual AP unit */
	struct ieee80211com *iv_ic;		/* back ptr to common state */
	u_int32_t iv_debug;			/* debug msg flags */
	struct ieee80211_stats iv_stats;		/* statistics */

	int iv_monitor_nods_only;		/* in monitor mode only nods traffic */
	int iv_monitor_txf_len;			/* in monitor mode, truncate tx packets */

	int (*iv_newstate)(struct ieee80211vap *, enum ieee80211_state, int);
	u_int8_t iv_myaddr[IEEE80211_ADDR_LEN];
	u_int32_t iv_flags;			/* state flags */
	u_int32_t iv_flags_ext;			/* extension of state flags */
	u_int32_t iv_caps;			/* capabilities */
	u_int8_t iv_ath_cap;			/* Atheros adv. capablities */
	enum ieee80211_opmode iv_opmode;	/* operation mode */
	enum ieee80211_state iv_state;		/* state machine state */
	struct timer_list iv_mgtsend;		/* mgmt frame response timer */
						/* inactivity timer settings */
	int iv_inact_init;			/* setting for new station */
	int iv_inact_auth;			/* auth but not assoc setting */
	int iv_inact_run;			/* authorized setting */
	int iv_inact_probe;			/* inactive probe time */

	int iv_des_nssid;			/* # desired ssids */
	struct ieee80211_scan_ssid iv_des_ssid[1];/* desired ssid table */
	u_int8_t iv_des_bssid[IEEE80211_ADDR_LEN];
	struct ieee80211_channel *iv_des_chan;	/* desired channel */
	u_int16_t iv_des_mode;			/* desired mode */
	int iv_nicknamelen;			/* XXX junk */
	u_int8_t	 iv_nickname[IEEE80211_NWID_LEN];
	u_int iv_bgscanidle;			/* bg scan idle threshold */
	u_int iv_bgscanintvl;			/* bg scan min interval */
	u_int iv_scanvalid;			/* scan cache valid threshold */
	struct ieee80211_roam iv_roam;		/* sta-mode roaming state */

	u_int32_t *iv_aid_bitmap;		/* association id map */
	u_int16_t iv_max_aid;
	u_int16_t iv_sta_assoc;			/* stations associated */
	u_int16_t iv_ps_sta;			/* stations in power save */
	u_int16_t iv_ps_pending;			/* ps sta's w/ pending frames */
	u_int8_t *iv_tim_bitmap;			/* power-save stations w/ data*/
	u_int16_t iv_tim_len;			/* ic_tim_bitmap size (bytes) */
	u_int8_t iv_dtim_period;			/* DTIM period */
	u_int8_t iv_dtim_count;			/* DTIM count from last bcn */
						/* set/unset aid pwrsav state */
	void (*iv_set_tim)(struct ieee80211_node *, int);
	u_int8_t iv_uapsdinfo;			/* sta mode QoS Info flags */
	struct ieee80211_node *iv_bss;		/* information for this node */
	int iv_fixed_rate;			/* 802.11 rate or -1 */
	u_int16_t iv_rtsthreshold;
	u_int16_t iv_fragthreshold;
	u_int16_t iv_txmin;			/* min tx retry count */
	u_int16_t iv_txmax;			/* max tx retry count */
	u_int16_t iv_txlifetime;			/* tx lifetime */
	int iv_inact_timer;			/* inactivity timer wait */
	void *iv_opt_ie;			/* user-specified IE's */
	u_int16_t iv_opt_ie_len;			/* length of ni_opt_ie */
	u_int16_t iv_def_txkey;			/* default/group tx key index */
	struct ieee80211_key iv_nw_keys[IEEE80211_WEP_NKID];
	int (*iv_key_alloc)(struct ieee80211vap *, const struct ieee80211_key *);
	int (*iv_key_delete)(struct ieee80211vap *, const struct ieee80211_key *,
		struct ieee80211_node *);
	int (*iv_key_set)(struct ieee80211vap *, const struct ieee80211_key *,
		const u_int8_t mac[IEEE80211_ADDR_LEN]);
	void (*iv_key_update_begin)(struct ieee80211vap *);
	void (*iv_key_update_end)(struct ieee80211vap *);
#ifdef ATH_SUPERG_COMP
	void (*iv_comp_set)(struct ieee80211vap *, struct ieee80211_node *, int);
#endif
	const struct ieee80211_authenticator *iv_auth;/* authenticator glue */
	void *iv_ec;				/* private auth state */
	struct ieee80211vap *iv_xrvap;		/* pointer to XR VAP , if XR is enabled */
	u_int16_t iv_xrbcnwait;			/* SWBA count incremented until it reaches XR_BECON_FACTOR */
	struct timer_list iv_xrvapstart;	/* timer to start xr */
	u_int8_t iv_chanchange_count; 		/* 11h counter for channel change */
	int iv_mcast_rate; 			/* Multicast rate (Kbps) */

	const struct ieee80211_aclator *iv_acl;	/* aclator glue */
	void *iv_as;				/* private aclator state */

	struct timer_list iv_swbmiss;		/* software beacon miss timer */
	u_int16_t iv_swbmiss_period; 		/* software beacon miss timer period */
	u_int8_t iv_bmiss_count;
	struct ieee80211_nsparams iv_nsparams;	/* new state parameters for tasklet for stajoin1 */
	struct IEEE80211_TQ_STRUCT iv_stajoin1tq; /* tasklet for newstate action called from stajoin1tq */
	unsigned int iv_nsdone;			/* Done with scheduled newstate tasklet */
	uint8_t	wds_mac[IEEE80211_ADDR_LEN];
	struct ieee80211_spy iv_spy;         	/* IWSPY support */
};
MALLOC_DECLARE(M_80211_VAP);

#define	IEEE80211_ADDR_NULL(a1)	(memcmp(a1, "\x00\x00\x00\x00\x00\x00", \
	IEEE80211_ADDR_LEN) == 0)
#define	IEEE80211_ADDR_EQ(a1,a2)	(memcmp(a1,a2,IEEE80211_ADDR_LEN) == 0)
#define	IEEE80211_ADDR_COPY(dst,src)	memcpy(dst,src,IEEE80211_ADDR_LEN)

/* ic_flags */
#define	IEEE80211_F_FF		0x00000001	/* CONF: ATH FF enabled */
#define	IEEE80211_F_TURBOP	0x00000002	/* CONF: ATH Turbo enabled*/
#define	IEEE80211_F_PROMISC	0x00000004	/* STATUS: promiscuous mode */
#define	IEEE80211_F_ALLMULTI	0x00000008	/* STATUS: all multicast mode */
/* NB: this is intentionally setup to be IEEE80211_CAPINFO_PRIVACY */
#define	IEEE80211_F_PRIVACY	0x00000010	/* CONF: privacy enabled */
#define	IEEE80211_F_PUREG	0x00000020	/* CONF: 11g w/o 11b sta's */
#define	IEEE80211_F_XRUPDATE	0x00000040	/* CONF: update beacon XR element*/
#define	IEEE80211_F_SCAN	0x00000080	/* STATUS: scanning */
#define	IEEE80211_F_XR		0x00000100	/* CONF: operate in XR mode */
#define	IEEE80211_F_SIBSS	0x00000200	/* STATUS: start IBSS */
/* NB: this is intentionally setup to be IEEE80211_CAPINFO_SHORT_SLOTTIME */
#define	IEEE80211_F_SHSLOT	0x00000400	/* STATUS: use short slot time*/
#define	IEEE80211_F_PMGTON	0x00000800	/* CONF: Power mgmt enable */
#define	IEEE80211_F_DESBSSID	0x00001000	/* CONF: des_bssid is set */
#define	IEEE80211_F_WME		0x00002000	/* CONF: enable WME use */
#define	IEEE80211_F_BGSCAN	0x00004000	/* CONF: bg scan enabled */
#define	IEEE80211_F_SWRETRY	0x00008000	/* CONF: sw tx retry enabled */
#define IEEE80211_F_TXPOW_FIXED	0x00010000	/* TX Power: fixed rate */
#define	IEEE80211_F_IBSSON	0x00020000	/* CONF: IBSS creation enable */
#define	IEEE80211_F_SHPREAMBLE	0x00040000	/* STATUS: use short preamble */
#define	IEEE80211_F_DATAPAD	0x00080000	/* CONF: do alignment pad */
#define	IEEE80211_F_USEPROT	0x00100000	/* STATUS: protection enabled */
#define	IEEE80211_F_USEBARKER	0x00200000	/* STATUS: use barker preamble*/
#define	IEEE80211_F_TIMUPDATE	0x00400000	/* STATUS: update beacon tim */
#define	IEEE80211_F_WPA1	0x00800000	/* CONF: WPA enabled */
#define	IEEE80211_F_WPA2	0x01000000	/* CONF: WPA2 enabled */
#define	IEEE80211_F_WPA		0x01800000	/* CONF: WPA/WPA2 enabled */
#define	IEEE80211_F_DROPUNENC	0x02000000	/* CONF: drop unencrypted */
#define	IEEE80211_F_COUNTERM	0x04000000	/* CONF: TKIP countermeasures */
#define	IEEE80211_F_HIDESSID	0x08000000	/* CONF: hide SSID in beacon */
#define IEEE80211_F_NOBRIDGE    0x10000000	/* CONF: disable internal bridge */

#define	IEEE80211_F_WMEUPDATE	0x20000000	/* STATUS: update beacon wme */
#define IEEE80211_F_DOTH	0x40000000	/* enable 11.h */
#define IEEE80211_F_CHANSWITCH	0x80000000	/* force chanswitch */

/* ic_flags_ext */
#define	IEEE80211_FEXT_WDS	0x00000001	/* CONF: 4 addr allowed */
#define IEEE80211_FEXT_COUNTRYIE 0x00000002	/* CONF: enable country IE */
#define IEEE80211_FEXT_SCAN_PENDING 0x00000004	/* STATE: scan pending */
#define	IEEE80211_FEXT_BGSCAN	0x00000008	/* STATE: enable full bgscan completion */
#define IEEE80211_FEXT_UAPSD	0x00000010	/* CONF: enable U-APSD */
#define IEEE80211_FEXT_SLEEP	0x00000020	/* STATUS: sleeping */
#define IEEE80211_FEXT_EOSPDROP	0x00000040	/* drop uapsd EOSP frames for test */
#define	IEEE80211_FEXT_MARKDFS	0x00000080	/* Enable marking of dfs interfernce */
#define IEEE80211_FEXT_REGCLASS	0x00000100	/* CONF: send regclassids in country ie */
#define IEEE80211_FEXT_ERPUPDATE 0x00000200	/* STATUS: update ERP element */
#define IEEE80211_FEXT_SWBMISS 0x00000400	/* CONF: use software beacon timer */
#define IEEE80211_FEXT_DROPUNENC_EAPOL 0x00000800      /* CONF: drop unencrypted eapol frames */

#define IEEE80211_COM_UAPSD_ENABLE(_ic)		((_ic)->ic_flags_ext |= IEEE80211_FEXT_UAPSD)
#define IEEE80211_COM_UAPSD_DISABLE(_ic)	((_ic)->ic_flags_ext &= ~IEEE80211_FEXT_UAPSD)
#define IEEE80211_COM_UAPSD_ENABLED(_ic)	((_ic)->ic_flags_ext & IEEE80211_FEXT_UAPSD)
#define IEEE80211_COM_GOTOSLEEP(_ic)		((_ic)->ic_flags_ext |= IEEE80211_FEXT_GOTOSLEEP)
#define IEEE80211_COM_WAKEUP(_ic)		((_ic)->ic_flags_ext &= ~IEEE80211_FEXT_SLEEP)
#define IEEE80211_COM_IS_SLEEPING(_ic)		((_ic)->ic_flags_ext & IEEE80211_FEXT_SLEEP)

#define IEEE80211_VAP_UAPSD_ENABLE(_v)	((_v)->iv_flags_ext |= IEEE80211_FEXT_UAPSD)
#define IEEE80211_VAP_UAPSD_DISABLE(_v)	((_v)->iv_flags_ext &= ~IEEE80211_FEXT_UAPSD)
#define IEEE80211_VAP_UAPSD_ENABLED(_v)	((_v)->iv_flags_ext & IEEE80211_FEXT_UAPSD)
#define IEEE80211_VAP_GOTOSLEEP(_v)	((_v)->iv_flags_ext |= IEEE80211_FEXT_SLEEP)
#define IEEE80211_VAP_WAKEUP(_v)	((_v)->iv_flags_ext &= ~IEEE80211_FEXT_SLEEP)
#define IEEE80211_VAP_IS_SLEEPING(_v)	((_v)->iv_flags_ext & IEEE80211_FEXT_SLEEP)
#define IEEE80211_VAP_EOSPDROP_ENABLE(_v)  ((_v)->iv_flags_ext |= IEEE80211_FEXT_EOSPDROP)
#define IEEE80211_VAP_EOSPDROP_DISABLE(_v) ((_v)->iv_flags_ext &= ~IEEE80211_FEXT_EOSPDROP)
#define IEEE80211_VAP_EOSPDROP_ENABLED(_v) ((_v)->iv_flags_ext & IEEE80211_FEXT_EOSPDROP)
#define IEEE80211_VAP_DROPUNENC_EAPOL_ENABLE(_v)  ((_v)->iv_flags_ext |= IEEE80211_FEXT_DROPUNENC_EAPOL)
#define IEEE80211_VAP_DROPUNENC_EAPOL_DISABLE(_v) ((_v)->iv_flags_ext &= ~IEEE80211_FEXT_DROPUNENC_EAPOL)
#define IEEE80211_VAP_DROPUNENC_EAPOL(_v) ((_v)->iv_flags_ext & IEEE80211_FEXT_DROPUNENC_EAPOL)


/* ic_caps */
#define	IEEE80211_C_WEP		0x00000001	/* CAPABILITY: WEP available */
#define	IEEE80211_C_TKIP	0x00000002	/* CAPABILITY: TKIP available */
#define	IEEE80211_C_AES		0x00000004	/* CAPABILITY: AES OCB avail */
#define	IEEE80211_C_AES_CCM	0x00000008	/* CAPABILITY: AES CCM avail */
#define	IEEE80211_C_CKIP	0x00000020	/* CAPABILITY: CKIP available */
#define	IEEE80211_C_FF		0x00000040	/* CAPABILITY: ATH FF avail */
#define	IEEE80211_C_TURBOP	0x00000080	/* CAPABILITY: ATH Turbo avail*/
#define	IEEE80211_C_IBSS	0x00000100	/* CAPABILITY: IBSS available */
#define	IEEE80211_C_PMGT	0x00000200	/* CAPABILITY: Power mgmt */
#define	IEEE80211_C_HOSTAP	0x00000400	/* CAPABILITY: HOSTAP avail */
#define	IEEE80211_C_AHDEMO	0x00000800	/* CAPABILITY: Old Adhoc Demo */
#define	IEEE80211_C_SWRETRY	0x00001000	/* CAPABILITY: sw tx retry */
#define	IEEE80211_C_TXPMGT	0x00002000	/* CAPABILITY: tx power mgmt */
#define	IEEE80211_C_SHSLOT	0x00004000	/* CAPABILITY: short slottime */
#define	IEEE80211_C_SHPREAMBLE	0x00008000	/* CAPABILITY: short preamble */
#define	IEEE80211_C_MONITOR	0x00010000	/* CAPABILITY: monitor mode */
#define	IEEE80211_C_TKIPMIC	0x00020000	/* CAPABILITY: TKIP MIC avail */
#define	IEEE80211_C_WPA1	0x00800000	/* CAPABILITY: WPA1 avail */
#define	IEEE80211_C_WPA2	0x01000000	/* CAPABILITY: WPA2 avail */
#define	IEEE80211_C_WPA		0x01800000	/* CAPABILITY: WPA1+WPA2 avail*/
#define	IEEE80211_C_BURST	0x02000000	/* CAPABILITY: frame bursting */
#define	IEEE80211_C_WME		0x04000000	/* CAPABILITY: WME avail */
#define	IEEE80211_C_WDS		0x08000000	/* CAPABILITY: 4-addr support */
#define IEEE80211_C_WME_TKIPMIC	0x10000000	/* CAPABILITY: TKIP MIC for QoS frame */
#define	IEEE80211_C_BGSCAN	0x20000000	/* CAPABILITY: bg scanning */
#define	IEEE80211_C_UAPSD	0x40000000	/* CAPABILITY: UAPSD */
/* XXX protection/barker? */

#define	IEEE80211_C_CRYPTO	0x0000002f	/* CAPABILITY: crypto alg's */

/* Atheros ABOLT definitions */
#define IEEE80211_ABOLT_TURBO_G		0x01	/* Legacy Turbo G */
#define IEEE80211_ABOLT_TURBO_PRIME	0x02	/* Turbo Prime */
#define IEEE80211_ABOLT_COMPRESSION	0x04	/* Compression */
#define IEEE80211_ABOLT_FAST_FRAME	0x08	/* Fast Frames */
#define IEEE80211_ABOLT_BURST		0x10	/* Bursting */
#define IEEE80211_ABOLT_WME_ELE		0x20	/* WME based cwmin/max/burst tuning */
#define IEEE80211_ABOLT_XR		0x40	/* XR */
#define IEEE80211_ABOLT_AR		0x80	/* AR switches out based on adjaced non-turbo traffic */

/* Atheros Advanced Capabilities ABOLT definition */
#define IEEE80211_ABOLT_ADVCAP	(IEEE80211_ABOLT_TURBO_PRIME | \
				 IEEE80211_ABOLT_COMPRESSION | \
				 IEEE80211_ABOLT_FAST_FRAME | \
				 IEEE80211_ABOLT_XR | \
				 IEEE80211_ABOLT_AR | \
				 IEEE80211_ABOLT_BURST | \
				 IEEE80211_ABOLT_WME_ELE)

/* check if a capability was negotiated for use */
#define	IEEE80211_ATH_CAP(vap, ni, bit) \
	((ni)->ni_ath_flags & (vap)->iv_ath_cap & (bit))

/* flags to VAP create function */
#define IEEE80211_VAP_XR		0x10000	/* create a XR VAP without registering net device with OS */

int ieee80211_ifattach(struct ieee80211com *);
void ieee80211_ifdetach(struct ieee80211com *);
int ieee80211_vap_setup(struct ieee80211com *, struct net_device *,
	const char *, int, int, int);
int ieee80211_vap_attach(struct ieee80211vap *, ifm_change_cb_t, ifm_stat_cb_t);
void ieee80211_vap_detach(struct ieee80211vap *);
void ieee80211_mark_dfs(struct ieee80211com *, struct ieee80211_channel *);
void ieee80211_dfs_test_return(struct ieee80211com *, u_int8_t);
void ieee80211_announce(struct ieee80211com *);
void ieee80211_announce_channels(struct ieee80211com *);
int ieee80211_media_change(struct net_device *);
void ieee80211_media_status(struct net_device *, struct ifmediareq *);
int ieee80211_rate2media(struct ieee80211com*, int, enum ieee80211_phymode);
int ieee80211_media2rate(int);
u_int ieee80211_mhz2ieee(u_int, u_int);
u_int ieee80211_chan2ieee(struct ieee80211com *,	const struct ieee80211_channel *);
u_int ieee80211_ieee2mhz(u_int, u_int);
struct ieee80211_channel *ieee80211_find_channel(struct ieee80211com *, int, int);
int ieee80211_setmode(struct ieee80211com *, enum ieee80211_phymode);
void ieee80211_reset_erp(struct ieee80211com *, enum ieee80211_phymode);
enum ieee80211_phymode ieee80211_chan2mode(const struct ieee80211_channel *);
void ieee80211_build_countryie(struct ieee80211com *);
int ieee80211_media_setup(struct ieee80211com *, struct ifmedia *, u_int32_t,
	ifm_change_cb_t, ifm_stat_cb_t);

/* 
 * Key update synchronization methods.  XXX should not be visible.
 */
static __inline void
ieee80211_key_update_begin(struct ieee80211vap *vap)
{
	vap->iv_key_update_begin(vap);
}
static __inline void
ieee80211_key_update_end(struct ieee80211vap *vap)
{
	vap->iv_key_update_end(vap);
}

/*
 * XXX these need to be here for IEEE80211_F_DATAPAD
 */

/*
 * Return the space occupied by the 802.11 header and any
 * padding required by the driver.  This works for a
 * management or data frame.
 */
static __inline int
ieee80211_hdrspace(struct ieee80211com *ic, const void *data)
{
	int size = ieee80211_hdrsize(data);
	if (ic->ic_flags & IEEE80211_F_DATAPAD)
		size = roundup(size, sizeof(u_int32_t));
	return size;
}

/*
 * Like ieee80211_hdrspace, but handles any type of frame.
 */
static __inline int
ieee80211_anyhdrspace(struct ieee80211com *ic, const void *data)
{
	int size = ieee80211_anyhdrsize(data);
	if (ic->ic_flags & IEEE80211_F_DATAPAD)
		size = roundup(size, sizeof(u_int32_t));
	return size;
}

#define	IEEE80211_MSG_DEBUG	0x40000000	/* IFF_DEBUG equivalent */
#define	IEEE80211_MSG_DUMPPKTS	0x20000000	/* IFF_LINK2 equivalant */
#define	IEEE80211_MSG_CRYPTO	0x10000000	/* crypto work */
#define	IEEE80211_MSG_INPUT	0x08000000	/* input handling */
#define	IEEE80211_MSG_XRATE	0x04000000	/* rate set handling */
#define	IEEE80211_MSG_ELEMID	0x02000000	/* element id parsing */
#define	IEEE80211_MSG_NODE	0x01000000	/* node handling */
#define	IEEE80211_MSG_ASSOC	0x00800000	/* association handling */
#define	IEEE80211_MSG_AUTH	0x00400000	/* authentication handling */
#define	IEEE80211_MSG_SCAN	0x00200000	/* scanning */
#define	IEEE80211_MSG_OUTPUT	0x00100000	/* output handling */
#define	IEEE80211_MSG_STATE	0x00080000	/* state machine */
#define	IEEE80211_MSG_POWER	0x00040000	/* power save handling */
#define	IEEE80211_MSG_DOT1X	0x00020000	/* 802.1x authenticator */
#define	IEEE80211_MSG_DOT1XSM	0x00010000	/* 802.1x state machine */
#define	IEEE80211_MSG_RADIUS	0x00008000	/* 802.1x radius client */
#define	IEEE80211_MSG_RADDUMP	0x00004000	/* dump 802.1x radius packets */
#define	IEEE80211_MSG_RADKEYS	0x00002000	/* dump 802.1x keys */
#define	IEEE80211_MSG_WPA	0x00001000	/* WPA/RSN protocol */
#define	IEEE80211_MSG_ACL	0x00000800	/* ACL handling */
#define	IEEE80211_MSG_WME	0x00000400	/* WME protocol */
#define	IEEE80211_MSG_SUPG	0x00000200	/* SUPERG */
#define	IEEE80211_MSG_DOTH	0x00000100	/* 11.h */
#define	IEEE80211_MSG_INACT	0x00000080	/* inactivity handling */
#define	IEEE80211_MSG_ROAM	0x00000040	/* sta-mode roaming */

#define	IEEE80211_MSG_ANY	0xffffffff	/* anything */

#ifdef IEEE80211_DEBUG
#define	ieee80211_msg(_vap, _m)	((_vap)->iv_debug & (_m))
#define	IEEE80211_DPRINTF(_vap, _m, _fmt, ...) do {			\
	if (ieee80211_msg(_vap, _m))					\
		ieee80211_note(_vap, _fmt, __VA_ARGS__);		\
} while (0)
#define	IEEE80211_NOTE(_vap, _m, _ni, _fmt, ...) do {			\
	if (ieee80211_msg(_vap, _m))					\
		ieee80211_note_mac(_vap, (_ni)->ni_macaddr, _fmt, __VA_ARGS__);\
} while (0)
#define	IEEE80211_NOTE_MAC(_vap, _m, _mac, _fmt, ...) do {		\
	if (ieee80211_msg(_vap, _m))					\
		ieee80211_note_mac(_vap, _mac, _fmt, __VA_ARGS__);	\
} while (0)
#define	IEEE80211_NOTE_FRAME(_vap, _m, _wh, _fmt, ...) do {		\
	if (ieee80211_msg(_vap, _m))					\
		ieee80211_note_frame(_vap, _wh, _fmt, __VA_ARGS__);	\
} while (0)
void ieee80211_note(struct ieee80211vap *, const char *, ...);
void ieee80211_note_mac(struct ieee80211vap *,
	const u_int8_t mac[IEEE80211_ADDR_LEN], const char *, ...);
void ieee80211_note_frame(struct ieee80211vap *,
	const struct ieee80211_frame *, const char *, ...);
#define	ieee80211_msg_debug(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_DEBUG)
#define	ieee80211_msg_dumppkts(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_DUMPPKTS)
#define	ieee80211_msg_input(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_INPUT)
#define	ieee80211_msg_radius(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_RADIUS)
#define	ieee80211_msg_dumpradius(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_RADDUMP)
#define	ieee80211_msg_dumpradkeys(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_RADKEYS)
#define	ieee80211_msg_scan(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_SCAN)
#define	ieee80211_msg_assoc(_vap) \
	ieee80211_msg(_vap, IEEE80211_MSG_ASSOC)
#else
#define	IEEE80211_DPRINTF(_vap, _m, _fmt, ...)
#define	IEEE80211_NOTE(_vap, _m, _ni, _fmt, ...)
#define	IEEE80211_NOTE_FRAME(_vap, _m, _wh, _fmt, ...)
#define	IEEE80211_NOTE_MAC(_vap, _m, _mac, _fmt, ...)
#define	ieee80211_msg_dumppkts(_vap)	0
#define	ieee80211_msg(_vap, _m)		0
#endif

#endif /* _NET80211_IEEE80211_VAR_H_ */
