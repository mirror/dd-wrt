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
 * $Id: ieee80211_node.c 1547 2006-05-12 03:44:02Z dyqith $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 node handling support.
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>
#include <net80211/if_athproto.h>

/*
 * Association id's are managed with a bit vector.
 */
#define	IEEE80211_AID_SET(_vap, _b) \
	((_vap)->iv_aid_bitmap[IEEE80211_AID(_b) / 32] |= \
		(1 << (IEEE80211_AID(_b) % 32)))
#define	IEEE80211_AID_CLR(_vap, _b) \
	((_vap)->iv_aid_bitmap[IEEE80211_AID(_b) / 32] &= \
		~(1 << (IEEE80211_AID(_b) % 32)))
#define	IEEE80211_AID_ISSET(_vap, _b) \
	((_vap)->iv_aid_bitmap[IEEE80211_AID(_b) / 32] & (1 << (IEEE80211_AID(_b) % 32)))

static int ieee80211_sta_join1(struct ieee80211_node *);

static struct ieee80211_node *node_alloc(struct ieee80211_node_table *,
	struct ieee80211vap *);
static void node_cleanup(struct ieee80211_node *);
static void node_free(struct ieee80211_node *);
static u_int8_t node_getrssi(const struct ieee80211_node *);

static void _ieee80211_free_node(struct ieee80211_node *);
static void node_reclaim(struct ieee80211_node_table *, struct ieee80211_node*);

static void ieee80211_node_timeout(unsigned long);

static void ieee80211_node_table_init(struct ieee80211com *,
	struct ieee80211_node_table *, const char *, int);
static void ieee80211_node_table_cleanup(struct ieee80211_node_table *);
static void ieee80211_node_table_reset(struct ieee80211_node_table *,
	struct ieee80211vap *);
static void ieee80211_node_wds_ageout(unsigned long);

MALLOC_DEFINE(M_80211_NODE, "80211node", "802.11 node state");

void
ieee80211_node_attach(struct ieee80211com *ic)
{
	ieee80211_node_table_init(ic, &ic->ic_sta, "station",
		IEEE80211_INACT_INIT);
	init_timer(&ic->ic_inact);
	ic->ic_inact.function = ieee80211_node_timeout;
	ic->ic_inact.data = (unsigned long) ic;
	ic->ic_inact.expires = jiffies + IEEE80211_INACT_WAIT * HZ;
	add_timer(&ic->ic_inact);

	ic->ic_node_alloc = node_alloc;
	ic->ic_node_free = node_free;
	ic->ic_node_cleanup = node_cleanup;
	ic->ic_node_getrssi = node_getrssi;
}

void
ieee80211_node_detach(struct ieee80211com *ic)
{
	del_timer(&ic->ic_inact);
	ieee80211_node_table_cleanup(&ic->ic_sta);

}

void
ieee80211_node_vattach(struct ieee80211vap *vap)
{
	/* default station inactivity timer setings */
	vap->iv_inact_init = IEEE80211_INACT_INIT;
	vap->iv_inact_auth = IEEE80211_INACT_AUTH;
	vap->iv_inact_run = IEEE80211_INACT_RUN;
	vap->iv_inact_probe = IEEE80211_INACT_PROBE;
}

void
ieee80211_node_latevattach(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_rsnparms *rsn;

	/*
	 * Allocate these only if needed.  Beware that we
	 * know adhoc mode doesn't support ATIM yet...
	 */
	if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
		if (vap->iv_max_aid == 0)
			vap->iv_max_aid = IEEE80211_AID_DEF;
		else if (vap->iv_max_aid > IEEE80211_AID_MAX)
			vap->iv_max_aid = IEEE80211_AID_MAX;
		MALLOC(vap->iv_aid_bitmap, u_int32_t *,
			howmany(vap->iv_max_aid, 32) * sizeof(u_int32_t),
			M_DEVBUF, M_NOWAIT | M_ZERO);
		if (vap->iv_aid_bitmap == NULL) {
			/* XXX no way to recover */
			printf("%s: no memory for AID bitmap!\n", __func__);
			vap->iv_max_aid = 0;
		}
	}

	ieee80211_reset_bss(vap);
	/*
	 * Setup "global settings" in the bss node so that
	 * each new station automatically inherits them.
	 */
	rsn = &vap->iv_bss->ni_rsn;
	/* WEP, TKIP, and AES-CCM are always supported */
	rsn->rsn_ucastcipherset |= 1 << IEEE80211_CIPHER_WEP;
	rsn->rsn_ucastcipherset |= 1 << IEEE80211_CIPHER_TKIP;
	rsn->rsn_ucastcipherset |= 1 << IEEE80211_CIPHER_AES_CCM;
	if (ic->ic_caps & IEEE80211_C_AES)
		rsn->rsn_ucastcipherset |= 1 << IEEE80211_CIPHER_AES_OCB;
	if (ic->ic_caps & IEEE80211_C_CKIP)
		rsn->rsn_ucastcipherset |= 1 << IEEE80211_CIPHER_CKIP;
	/*
	 * Default unicast cipher to WEP for 802.1x use.  If
	 * WPA is enabled the management code will set these
	 * values to reflect.
	 */
	rsn->rsn_ucastcipher = IEEE80211_CIPHER_WEP;
	rsn->rsn_ucastkeylen = 104 / NBBY;
	/*
	 * WPA says the multicast cipher is the lowest unicast
	 * cipher supported.  But we skip WEP which would
	 * otherwise be used based on this criteria.
	 */
	rsn->rsn_mcastcipher = IEEE80211_CIPHER_TKIP;
	rsn->rsn_mcastkeylen = 128 / NBBY;

	/*
	 * We support both WPA-PSK and 802.1x; the one used
	 * is determined by the authentication mode and the
	 * setting of the PSK state.
	 */
	rsn->rsn_keymgmtset = WPA_ASE_8021X_UNSPEC | WPA_ASE_8021X_PSK;
	rsn->rsn_keymgmt = WPA_ASE_8021X_PSK;

	vap->iv_auth = ieee80211_authenticator_get(vap->iv_bss->ni_authmode);
}

void
ieee80211_node_vdetach(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;

	ieee80211_node_table_reset(&ic->ic_sta, vap);
	if (vap->iv_bss != NULL) {
		ieee80211_free_node(vap->iv_bss);
		vap->iv_bss = NULL;
	}
	if (vap->iv_aid_bitmap != NULL) {
		FREE(vap->iv_aid_bitmap, M_DEVBUF);
		vap->iv_aid_bitmap = NULL;
	}
}

/* 
 * Port authorize/unauthorize interfaces for use by an authenticator.
 */

void
ieee80211_node_authorize(struct ieee80211_node *ni)
{
	ni->ni_flags |= IEEE80211_NODE_AUTH;
	ni->ni_inact_reload = ni->ni_vap->iv_inact_run;
}
EXPORT_SYMBOL(ieee80211_node_authorize);

void
ieee80211_node_unauthorize(struct ieee80211_node *ni)
{
	ni->ni_flags &= ~IEEE80211_NODE_AUTH;
}
EXPORT_SYMBOL(ieee80211_node_unauthorize);

/*
 * Set/change the channel.  The rate set is also updated
 * to ensure a consistent view by drivers.
 */
static __inline void
ieee80211_node_set_chan(struct ieee80211com *ic, struct ieee80211_node *ni)
{
	struct ieee80211_channel *chan = ic->ic_bsschan;

	KASSERT(chan != IEEE80211_CHAN_ANYC, ("bss channel not setup"));
	ni->ni_chan = chan;
#ifdef ATH_SUPERG_XR 
	if (ni->ni_vap->iv_flags & IEEE80211_F_XR)
		ni->ni_rates = ic->ic_sup_xr_rates;
	else
#endif
	ni->ni_rates = ic->ic_sup_rates[ieee80211_chan2mode(chan)];
}

static __inline void
copy_bss(struct ieee80211_node *nbss, const struct ieee80211_node *obss)
{
	/* propagate useful state */
	nbss->ni_authmode = obss->ni_authmode;
	nbss->ni_ath_flags = obss->ni_ath_flags;
	nbss->ni_txpower = obss->ni_txpower;
	nbss->ni_vlan = obss->ni_vlan;
	nbss->ni_rsn = obss->ni_rsn;
	/* XXX statistics? */
}

void
ieee80211_create_ibss(struct ieee80211vap* vap, struct ieee80211_channel *chan)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN,
		"%s: creating ibss on channel %u\n", __func__,
		ieee80211_chan2ieee(ic, chan));

	/* Check to see if we already have a node for this mac */
	ni = ieee80211_find_node(&ic->ic_sta, vap->iv_myaddr);
	if (ni == NULL) {
		ni = ieee80211_alloc_node(&ic->ic_sta, vap, vap->iv_myaddr);
		if (ni == NULL) {
			/* XXX recovery? */
			return;
		}
	}
	else
		ieee80211_free_node(ni);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, "%s: %p<%s> refcnt %d\n",
		__func__, vap->iv_bss, ether_sprintf(vap->iv_bss->ni_macaddr),
		ieee80211_node_refcnt(ni));

	IEEE80211_ADDR_COPY(ni->ni_bssid, vap->iv_myaddr);
	ni->ni_esslen = vap->iv_des_ssid[0].len;
	memcpy(ni->ni_essid, vap->iv_des_ssid[0].ssid, ni->ni_esslen);
	if (vap->iv_bss != NULL)
		copy_bss(ni, vap->iv_bss);
	ni->ni_intval = ic->ic_lintval;
#ifdef ATH_SUPERG_XR 
	if (vap->iv_flags & IEEE80211_F_XR) {
		ni->ni_intval *= IEEE80211_XR_BEACON_FACTOR;
	}
#endif
	if (vap->iv_flags & IEEE80211_F_PRIVACY)
		ni->ni_capinfo |= IEEE80211_CAPINFO_PRIVACY;
	if (ic->ic_phytype == IEEE80211_T_FH) {
		ni->ni_fhdwell = 200;	/* XXX */
		ni->ni_fhindex = 1;
	}
	if (vap->iv_opmode == IEEE80211_M_IBSS) {
		vap->iv_flags |= IEEE80211_F_SIBSS;
		ni->ni_capinfo |= IEEE80211_CAPINFO_IBSS;	/* XXX */
		if (vap->iv_flags & IEEE80211_F_DESBSSID)
			IEEE80211_ADDR_COPY(ni->ni_bssid, vap->iv_des_bssid);
		else
			ni->ni_bssid[0] |= 0x02;	/* local bit for IBSS */
	} else if (vap->iv_opmode == IEEE80211_M_AHDEMO) {
		if (vap->iv_flags & IEEE80211_F_DESBSSID)
		    IEEE80211_ADDR_COPY(ni->ni_bssid, vap->iv_des_bssid);
		else {
		    ni->ni_bssid[0] = 0x00;
		    ni->ni_bssid[1] = 0x00;
		    ni->ni_bssid[2] = 0x00;
		    ni->ni_bssid[3] = 0x00;
		    ni->ni_bssid[4] = 0x00;
		    ni->ni_bssid[5] = 0x00;
		}
	}
#ifdef ATH_SUPERG_DYNTURBO
	if (vap->iv_opmode == IEEE80211_M_HOSTAP) {
		ni->ni_ath_flags = vap->iv_ath_cap;
		/*
		 * no dynamic turbo and AR on a static turbo channel.
		 * no dynamic turbo and AR on non-turbo channel.
		 * no AR on 5GHZ channel .
		 */        
		if (IEEE80211_IS_CHAN_STURBO(chan) || 
		    !ieee80211_find_channel(ic, chan->ic_freq, chan->ic_flags | IEEE80211_CHAN_TURBO))
			ni->ni_ath_flags &= ~(IEEE80211_ATHC_TURBOP | IEEE80211_ATHC_AR);
		if (IEEE80211_IS_CHAN_5GHZ(chan)) 
			ni->ni_ath_flags &= ~IEEE80211_ATHC_AR;
	}
#endif
	/* 
	 * Fix the channel and related attributes.
	 */
	ic->ic_bsschan = chan;
	ieee80211_node_set_chan(ic, ni);
	ic->ic_curmode = ieee80211_chan2mode(chan);

	/* Update country ie information */
	ieee80211_build_countryie(ic);

	if (IEEE80211_IS_CHAN_HALF(chan))
		ni->ni_rates = ic->ic_sup_half_rates;
	else if (IEEE80211_IS_CHAN_QUARTER(chan))
		ni->ni_rates = ic->ic_sup_quarter_rates;

	(void) ieee80211_sta_join1(ieee80211_ref_node(ni));
}
EXPORT_SYMBOL(ieee80211_create_ibss);

/*
 * Reset bss state on transition to the INIT state.
 * Clear any stations from the table (they have been
 * deauth'd) and reset the bss node (clears key, rate,
 * etc. state).
 */
void
ieee80211_reset_bss(struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni, *obss;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, "%s: old bss %p<%s> refcnt %d\n",
		__func__, vap->iv_bss, ether_sprintf(vap->iv_bss->ni_macaddr),
		ieee80211_node_refcnt(vap->iv_bss));

	ieee80211_node_table_reset(&ic->ic_sta, vap);
	/* XXX multi-bss wrong */
	ieee80211_reset_erp(ic, ic->ic_curmode);

	ni = ieee80211_alloc_node(&ic->ic_sta, vap, vap->iv_myaddr);
	KASSERT(ni != NULL, ("unable to setup inital BSS node"));
	obss = vap->iv_bss;
	vap->iv_bss = ieee80211_ref_node(ni);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, "%s: new bss %p<%s> refcnt %d\n",
		__func__, vap->iv_bss, ether_sprintf(vap->iv_bss->ni_macaddr),
		ieee80211_node_refcnt(vap->iv_bss));

	if (obss != NULL) {
		copy_bss(ni, obss);
		ni->ni_intval = ic->ic_lintval;
		ieee80211_free_node(obss);
	}
}

static int
match_ssid(const struct ieee80211_node *ni,
	int nssid, const struct ieee80211_scan_ssid ssids[])
{
	int i;

	for (i = 0; i < nssid; i++) {
		if (ni->ni_esslen == ssids[i].len &&
		    memcmp(ni->ni_essid, ssids[i].ssid, ni->ni_esslen) == 0)
			return 1;
	}
	return 0;
}

/*
 * Test a node for suitability/compatibility.
 */
static int
check_bss(struct ieee80211vap *vap, struct ieee80211_node *ni)
{
	struct ieee80211com *ic = ni->ni_ic;
        u_int8_t rate;

	if (isclr(ic->ic_chan_active, ieee80211_chan2ieee(ic, ni->ni_chan)))
		return 0;
	if (vap->iv_opmode == IEEE80211_M_IBSS) {
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_IBSS) == 0)
			return 0;
	} else {
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_ESS) == 0)
			return 0;
	}
	if (vap->iv_flags & IEEE80211_F_PRIVACY) {
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_PRIVACY) == 0)
			return 0;
	} else {
		/* XXX does this mean privacy is supported or required? */
		if (ni->ni_capinfo & IEEE80211_CAPINFO_PRIVACY)
			return 0;
	}
	rate = ieee80211_fix_rate(ni, IEEE80211_F_DONEGO | IEEE80211_F_DOFRATE);
	if (rate & IEEE80211_RATE_BASIC)
		return 0;
	if (vap->iv_des_nssid != 0 &&
	    !match_ssid(ni, vap->iv_des_nssid, vap->iv_des_ssid))
		return 0;
	if ((vap->iv_flags & IEEE80211_F_DESBSSID) &&
	    !IEEE80211_ADDR_EQ(vap->iv_des_bssid, ni->ni_bssid))
		return 0;
	return 1;
}

#ifdef IEEE80211_DEBUG
/*
 * Display node suitability/compatibility.
 */
static void
check_bss_debug(struct ieee80211vap *vap, struct ieee80211_node *ni)
{
	struct ieee80211com *ic = ni->ni_ic;
        u_int8_t rate;
        int fail;

	fail = 0;
	if (isclr(ic->ic_chan_active, ieee80211_chan2ieee(ic, ni->ni_chan)))
		fail |= 0x01;
	if (vap->iv_opmode == IEEE80211_M_IBSS) {
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_IBSS) == 0)
			fail |= 0x02;
	} else {
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_ESS) == 0)
			fail |= 0x02;
	}
	if (vap->iv_flags & IEEE80211_F_PRIVACY) {
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_PRIVACY) == 0)
			fail |= 0x04;
	} else {
		/* XXX does this mean privacy is supported or required? */
		if (ni->ni_capinfo & IEEE80211_CAPINFO_PRIVACY)
			fail |= 0x04;
	}
	rate = ieee80211_fix_rate(ni, IEEE80211_F_DONEGO | IEEE80211_F_DOFRATE);
	if (rate & IEEE80211_RATE_BASIC)
		fail |= 0x08;
	if (vap->iv_des_nssid != 0 &&
	    !match_ssid(ni, vap->iv_des_nssid, vap->iv_des_ssid))
		fail |= 0x10;
	if ((vap->iv_flags & IEEE80211_F_DESBSSID) &&
	    !IEEE80211_ADDR_EQ(vap->iv_des_bssid, ni->ni_bssid))
		fail |= 0x20;

	printf(" %c %s", fail ? '-' : '+', ether_sprintf(ni->ni_macaddr));
	printf(" %s%c", ether_sprintf(ni->ni_bssid), fail & 0x20 ? '!' : ' ');
	printf(" %3d%c",
		ieee80211_chan2ieee(ic, ni->ni_chan), fail & 0x01 ? '!' : ' ');
	printf(" %+4d", ni->ni_rssi);
	printf(" %2dM%c", (rate & IEEE80211_RATE_VAL) / 2,
		fail & 0x08 ? '!' : ' ');
	printf(" %4s%c",
		(ni->ni_capinfo & IEEE80211_CAPINFO_ESS) ? "ess" :
			(ni->ni_capinfo & IEEE80211_CAPINFO_IBSS) ? "ibss" :
				"????",
	    	fail & 0x02 ? '!' : ' ');
	printf(" %3s%c ",
	    	(ni->ni_capinfo & IEEE80211_CAPINFO_PRIVACY) ?  "wep" : "no",
	    	fail & 0x04 ? '!' : ' ');
	ieee80211_print_essid(ni->ni_essid, ni->ni_esslen);
	printf("%s\n", fail & 0x10 ? "!" : "");
}
#endif /* IEEE80211_DEBUG */
 
/*
 * Handle 802.11 ad hoc network merge.  The
 * convention, set by the Wireless Ethernet Compatibility Alliance
 * (WECA), is that an 802.11 station will change its BSSID to match
 * the "oldest" 802.11 ad hoc network, on the same channel, that
 * has the station's desired SSID.  The "oldest" 802.11 network
 * sends beacons with the greatest TSF timestamp.
 *
 * The caller is assumed to validate TSF's before attempting a merge.
 *
 * Return !0 if the BSSID changed, 0 otherwise.
 */
int
ieee80211_ibss_merge(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
#ifdef IEEE80211_DEBUG
	struct ieee80211com *ic = ni->ni_ic;
#endif
	if (ni == vap->iv_bss ||
	    IEEE80211_ADDR_EQ(ni->ni_bssid, vap->iv_bss->ni_bssid)) {
		/* unchanged, nothing to do */
		return 0;
	}
	if (!check_bss(vap, ni)) {
		/* capabilities mismatch */
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC,
		    "%s: merge failed, capabilities mismatch\n", __func__);
#ifdef IEEE80211_DEBUG
		if (ieee80211_msg_assoc(vap))
			check_bss_debug(vap, ni);
#endif
		vap->iv_stats.is_ibss_capmismatch++;
		return 0;
	}
	IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC,
		"%s: new bssid %s: %s preamble, %s slot time%s\n", __func__,
		ether_sprintf(ni->ni_bssid),
		ic->ic_flags & IEEE80211_F_SHPREAMBLE ? "short" : "long",
		ic->ic_flags & IEEE80211_F_SHSLOT ? "short" : "long",
		ic->ic_flags & IEEE80211_F_USEPROT ? ", protection" : "");
	return ieee80211_sta_join1(ieee80211_ref_node(ni));
}
EXPORT_SYMBOL(ieee80211_ibss_merge);

static __inline int
ssid_equal(const struct ieee80211_node *a, const struct ieee80211_node *b)
{
	return (a->ni_esslen == b->ni_esslen &&
		memcmp(a->ni_essid, b->ni_bssid, a->ni_esslen) == 0);
}

/*
 * Join the specified IBSS/BSS network.  The node is assumed to
 * be passed in with a reference already held for use in assigning
 * to iv_bss.
 */
static int
ieee80211_sta_join1(struct ieee80211_node *selbs)
{
	struct ieee80211vap *vap = selbs->ni_vap;
	struct ieee80211com *ic = selbs->ni_ic;
	struct ieee80211_node *obss;
	int canreassoc;

	if (vap->iv_opmode == IEEE80211_M_IBSS) {
		/*
		 * Delete unusable rates; we've already checked
		 * that the negotiated rate set is acceptable.
		 */
		ieee80211_fix_rate(selbs, IEEE80211_F_DODEL);
	}

	/*
	 * Committed to selbs, setup state.
	 */
	obss = vap->iv_bss;
	/*
	 * Check if old+new node have the same ssid in which
	 * case we can reassociate when operating in sta mode.
	 */
	canreassoc = (obss != NULL &&
		vap->iv_state == IEEE80211_S_RUN && ssid_equal(obss, selbs));
	vap->iv_bss = selbs;
	if (obss != NULL)
		ieee80211_free_node(obss);
	ic->ic_bsschan = selbs->ni_chan;
	ic->ic_curchan = ic->ic_bsschan;
	ic->ic_curmode = ieee80211_chan2mode(ic->ic_curchan);
	ic->ic_set_channel(ic);
	/*
	 * Set the erp state (mostly the slot time) to deal with
	 * the auto-select case; this should be redundant if the
	 * mode is locked.
	 */ 
	ieee80211_reset_erp(ic, ic->ic_curmode);
	ieee80211_wme_initparams(vap);

	if (vap->iv_opmode == IEEE80211_M_STA) {
		/*
		 * Act as if we received a DEAUTH frame in case we are
		 * invoked from the RUN state.  This will cause us to try
		 * to re-authenticate if we are operating as a station.
		 */
		if (canreassoc) {
			vap->iv_nsparams.newstate = IEEE80211_S_ASSOC;
			vap->iv_nsparams.arg = 0;
			IEEE80211_SCHEDULE_TQUEUE(&vap->iv_stajoin1tq);
		} else {
			vap->iv_nsparams.newstate = IEEE80211_S_AUTH;
			vap->iv_nsparams.arg = IEEE80211_FC0_SUBTYPE_DEAUTH;
			IEEE80211_SCHEDULE_TQUEUE(&vap->iv_stajoin1tq);
		}
	} else {
		vap->iv_nsparams.newstate = IEEE80211_S_RUN;
		vap->iv_nsparams.arg = -1;
		IEEE80211_SCHEDULE_TQUEUE(&vap->iv_stajoin1tq);
	}
	return 1;
}

void
ieee80211_sta_join1_tasklet(IEEE80211_TQUEUE_ARG data)
{
	struct ieee80211vap *vap= (struct ieee80211vap *) data;
	int rc;

	rc = ieee80211_new_state(vap, vap->iv_nsparams.newstate, vap->iv_nsparams.arg);
	vap->iv_nsparams.result = rc;
	vap->iv_nsdone = 1;
}
EXPORT_SYMBOL(ieee80211_sta_join1_tasklet);

int
ieee80211_sta_join(struct ieee80211vap *vap,
	const struct ieee80211_scan_entry *se)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;

	ni = ieee80211_find_node(&ic->ic_sta, se->se_macaddr);
	if (ni == NULL) {
		ni = ieee80211_alloc_node(&ic->ic_sta, vap, se->se_macaddr);
		if (ni == NULL) {
			/* XXX msg */
			return 0;
		}
	} else
		ieee80211_free_node(ni);

	/*
	 * Expand scan state into node's format.
	 * XXX may not need all this stuff
	 */
	ni->ni_authmode = vap->iv_bss->ni_authmode;		/* inherit authmode from iv_bss */
	/* inherit the WPA setup as well (structure copy!) */
	ni->ni_rsn = vap->iv_bss->ni_rsn; 
	IEEE80211_ADDR_COPY(ni->ni_bssid, se->se_bssid);
	ni->ni_esslen = se->se_ssid[1];
	memcpy(ni->ni_essid, se->se_ssid + 2, ni->ni_esslen);
	ni->ni_rstamp = se->se_rstamp;
	ni->ni_tstamp.tsf = se->se_tstamp.tsf;
	ni->ni_intval = se->se_intval;
	ni->ni_capinfo = se->se_capinfo;
	ni->ni_chan = se->se_chan;
	ni->ni_timoff = se->se_timoff;
	ni->ni_fhdwell = se->se_fhdwell;
	ni->ni_fhindex = se->se_fhindex;
	ni->ni_erp = se->se_erp;
	ni->ni_rssi = se->se_rssi;
	if (se->se_wpa_ie != NULL)
		ieee80211_saveie(&ni->ni_wpa_ie, se->se_wpa_ie);
	if (se->se_rsn_ie != NULL)
		ieee80211_saveie(&ni->ni_rsn_ie, se->se_rsn_ie);
	if (se->se_wme_ie != NULL)
		ieee80211_saveie(&ni->ni_wme_ie, se->se_wme_ie);
	if (se->se_ath_ie != NULL)
		ieee80211_saveath(ni, se->se_ath_ie);

	vap->iv_dtim_period = se->se_dtimperiod;
	vap->iv_dtim_count = 0;

	/* NB: must be after ni_chan is setup */
	ieee80211_setup_rates(ni, se->se_rates, se->se_xrates,
		IEEE80211_F_DOSORT | IEEE80211_F_DONEGO | IEEE80211_F_DODEL);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, 
	"%s: %p<%s> refcnt %d\n", __func__, ni, ether_sprintf(ni->ni_macaddr), 
	ieee80211_node_refcnt(ni)+1);

	return ieee80211_sta_join1(ieee80211_ref_node(ni));
}
EXPORT_SYMBOL(ieee80211_sta_join);

/*
 * Leave the specified IBSS/BSS network.  The node is assumed to
 * be passed in with a held reference.
 */
void
ieee80211_sta_leave(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;

	/* WDS/Repeater: Stop software beacon timer for STA */
	if (vap->iv_opmode == IEEE80211_M_STA &&
	    vap->iv_flags_ext & IEEE80211_FEXT_SWBMISS) {
		del_timer(&vap->iv_swbmiss);
	}

	ic->ic_node_cleanup(ni);
	ieee80211_notify_node_leave(ni);
}

/*
 * Node table support.
 */

static void
ieee80211_node_table_init(struct ieee80211com *ic,
	struct ieee80211_node_table *nt,	const char *name, int inact)
{
	nt->nt_ic = ic;
	IEEE80211_NODE_LOCK_INIT(nt, ic->ic_dev->name);
	IEEE80211_SCAN_LOCK_INIT(nt, ic->ic_dev->name);
	TAILQ_INIT(&nt->nt_node);
	nt->nt_name = name;
	nt->nt_scangen = 1;
	nt->nt_inact_init = inact;
	init_timer(&nt->nt_wds_aging_timer);
	nt->nt_wds_aging_timer.function = ieee80211_node_wds_ageout;
	nt->nt_wds_aging_timer.data = (unsigned long) nt;
	mod_timer(&nt->nt_wds_aging_timer, jiffies + HZ * WDS_AGING_TIMER_VAL);
}

static struct ieee80211_node *
node_alloc(struct ieee80211_node_table *nt, struct ieee80211vap *vap)
{
	struct ieee80211_node *ni;

	MALLOC(ni, struct ieee80211_node *, sizeof(struct ieee80211_node),
		M_80211_NODE, M_NOWAIT | M_ZERO);

	return ni;
}

/*
 * Reclaim any resources in a node and reset any critical
 * state.  Typically nodes are free'd immediately after,
 * but in some cases the storage may be reused so we need
 * to ensure consistent state (should probably fix that).
 *
 * Context: hwIRQ, softIRQ and process context
 */
static void
node_cleanup(struct ieee80211_node *ni)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	struct ieee80211vap *vap = ni->ni_vap;
	int i;

	/* NB: preserve ni_table */
	if (ni->ni_flags & IEEE80211_NODE_PWR_MGT) {
		if (vap->iv_opmode != IEEE80211_M_STA)
			vap->iv_ps_sta--;
		ni->ni_flags &= ~IEEE80211_NODE_PWR_MGT;
		IEEE80211_NOTE(vap, IEEE80211_MSG_POWER, ni,
			"power save mode off, %u sta's in ps mode",
			vap->iv_ps_sta);

		if (ni->ni_flags & IEEE80211_NODE_UAPSD_TRIG) {
			ni->ni_flags &= ~IEEE80211_NODE_UAPSD_TRIG;
			IEEE80211_LOCK_IRQ(ni->ni_ic);
			ni->ni_ic->ic_uapsdmaxtriggers--;
			IEEE80211_UNLOCK_IRQ(ni->ni_ic);
		}
	}
	/*
	 * Clear AREF flag that marks the authorization refcnt bump
	 * has happened.  This is probably not needed as the node
	 * should always be removed from the table so not found but
	 * do it just in case.
	 */
	ni->ni_flags &= ~IEEE80211_NODE_AREF;

	/*
	 * Drain power save queue and, if needed, clear TIM.
	 */
	if (ieee80211_node_saveq_drain(ni) != 0 && vap->iv_set_tim != NULL)
		vap->iv_set_tim(ni, 0);

	ni->ni_associd = 0;
	if (ni->ni_challenge != NULL) {
		FREE(ni->ni_challenge, M_DEVBUF);
		ni->ni_challenge = NULL;
	}
	/*
	 * Preserve SSID, WPA, and WME ie's so the bss node is
	 * reusable during a re-auth/re-assoc state transition.
	 * If we remove these data they will not be recreated
	 * because they come from a probe-response or beacon frame
	 * which cannot be expected prior to the association-response.
	 * This should not be an issue when operating in other modes
	 * as stations leaving always go through a full state transition
	 * which will rebuild this state.
	 *
	 * XXX does this leave us open to inheriting old state?
	 */
	for (i = 0; i < N(ni->ni_rxfrag); i++)
		if (ni->ni_rxfrag[i] != NULL) {
			dev_kfree_skb_any(ni->ni_rxfrag[i]);
			ni->ni_rxfrag[i] = NULL;
		}
	ieee80211_crypto_delkey(vap, &ni->ni_ucastkey, ni);
	ni->ni_rxkeyoff = 0;
#undef N
}

static void
node_free(struct ieee80211_node *ni)
{
	struct ieee80211com *ic = ni->ni_ic;

	ic->ic_node_cleanup(ni);
	if (ni->ni_wpa_ie != NULL)
		FREE(ni->ni_wpa_ie, M_DEVBUF);
	if (ni->ni_rsn_ie != NULL)
		FREE(ni->ni_rsn_ie, M_DEVBUF);
	if (ni->ni_wme_ie != NULL)
		FREE(ni->ni_wme_ie, M_DEVBUF);
	if (ni->ni_ath_ie != NULL)
		FREE(ni->ni_ath_ie, M_DEVBUF);
	IEEE80211_NODE_SAVEQ_DESTROY(ni);
	FREE(ni, M_80211_NODE);
}

static u_int8_t
node_getrssi(const struct ieee80211_node *ni)
{
	return ni->ni_rssi;
}

/*
 * Create an entry in the specified node table.  The node
 * is setup with the mac address, an initial reference count,
 * and some basic parameters obtained from global state.
 * This interface is not intended for general use, it is
 * used by the routines below to create entries with a
 * specific purpose.
 */
struct ieee80211_node *
ieee80211_alloc_node(struct ieee80211_node_table *nt,
	struct ieee80211vap *vap, const u_int8_t *macaddr)
{
	struct ieee80211com *ic = nt->nt_ic;
	struct ieee80211_node *ni;
	int hash;
	int i;

	ni = ic->ic_node_alloc(nt, vap);
	if (ni == NULL) {
		/* XXX msg */
		vap->iv_stats.is_rx_nodealloc++;
		return NULL;
	}

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE,
		"%s: %p<%s> in %s table, refcnt %d\n", __func__, ni,
		ether_sprintf(macaddr), nt->nt_name,
		ieee80211_node_refcnt(ni)+1);

	IEEE80211_ADDR_COPY(ni->ni_macaddr, macaddr);
	hash = IEEE80211_NODE_HASH(macaddr);
	ieee80211_node_initref(ni);		/* mark referenced */
	ni->ni_chan = IEEE80211_CHAN_ANYC;
	ni->ni_authmode = IEEE80211_AUTH_OPEN;
	ni->ni_txpower = ic->ic_txpowlimit;	/* max power */
	ieee80211_crypto_resetkey(vap, &ni->ni_ucastkey, IEEE80211_KEYIX_NONE);
	ni->ni_inact_reload = nt->nt_inact_init;
	ni->ni_inact = ni->ni_inact_reload;
	ni->ni_ath_defkeyindex = IEEE80211_INVAL_DEFKEY;
	ni->ni_rxkeyoff = 0;
	IEEE80211_NODE_SAVEQ_INIT(ni, "unknown");

	IEEE80211_NODE_LOCK_IRQ(nt);
	ni->ni_vap = vap;
	ni->ni_ic = ic;
	ni->ni_table = nt;
	TAILQ_INSERT_TAIL(&nt->nt_node, ni, ni_list);
	LIST_INSERT_HEAD(&nt->nt_hash[hash], ni, ni_hash);
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	for (i = 0; i < N(ni->ni_rxfrag); i++)
		ni->ni_rxfrag[i] = NULL;
#undef N
	ni->ni_challenge = NULL;
	IEEE80211_NODE_UNLOCK_IRQ(nt);

	WME_UAPSD_NODE_TRIGSEQINIT(ni);

	return ni;
}
EXPORT_SYMBOL(ieee80211_alloc_node);

/* Add wds address to the node table */
int
ieee80211_add_wds_addr(struct ieee80211_node_table *nt,
	struct ieee80211_node *ni, const u_int8_t *macaddr, u_int8_t wds_static)
{
	int hash;
	struct ieee80211_wds_addr *wds;

	MALLOC(wds, struct ieee80211_wds_addr *, sizeof(struct ieee80211_wds_addr),
	       M_80211_WDS, M_NOWAIT | M_ZERO);
	if (wds == NULL) {
		/* XXX msg */
		return 1;
	}
	if (wds_static)
		wds->wds_agingcount = WDS_AGING_STATIC;
	else
		wds->wds_agingcount = WDS_AGING_COUNT;
	hash = IEEE80211_NODE_HASH(macaddr);
	IEEE80211_ADDR_COPY(wds->wds_macaddr, macaddr);
	ieee80211_ref_node(ni);		/* Reference node */
	wds->wds_ni = ni;
	IEEE80211_NODE_LOCK_IRQ(nt);
	LIST_INSERT_HEAD(&nt->nt_wds_hash[hash], wds, wds_hash);
	IEEE80211_NODE_UNLOCK_IRQ(nt);
	return 0;
}
EXPORT_SYMBOL(ieee80211_add_wds_addr);

/* remove wds address from the wds hash table */
void
ieee80211_remove_wds_addr(struct ieee80211_node_table *nt, const u_int8_t *macaddr)
{
	int hash;
	struct ieee80211_wds_addr *wds;

	hash = IEEE80211_NODE_HASH(macaddr);
	IEEE80211_NODE_LOCK_IRQ(nt);
	LIST_FOREACH(wds, &nt->nt_wds_hash[hash], wds_hash) {
		if (IEEE80211_ADDR_EQ(wds->wds_macaddr, macaddr)) {
			if (ieee80211_node_dectestref(wds->wds_ni)) {
				_ieee80211_free_node(wds->wds_ni);
				LIST_REMOVE(wds, wds_hash);
				FREE(wds, M_80211_WDS);
				break;
			}
		}
	}
	IEEE80211_NODE_UNLOCK_IRQ(nt);
}
EXPORT_SYMBOL(ieee80211_remove_wds_addr);


/* Remove node references from wds table */
void
ieee80211_del_wds_node(struct ieee80211_node_table *nt, struct ieee80211_node *ni)
{
	int hash;
	struct ieee80211_wds_addr *wds;

	IEEE80211_NODE_LOCK_IRQ(nt);
	for (hash = 0; hash < IEEE80211_NODE_HASHSIZE; hash++) {
		LIST_FOREACH(wds, &nt->nt_wds_hash[hash], wds_hash) {
			if (wds->wds_ni == ni) {
				if (ieee80211_node_dectestref(ni)) {
					_ieee80211_free_node(ni);
					LIST_REMOVE(wds, wds_hash);
					FREE(wds, M_80211_WDS);
				}
			}
		}
	}
	IEEE80211_NODE_UNLOCK_IRQ(nt);
}
EXPORT_SYMBOL(ieee80211_del_wds_node);

static void 
ieee80211_node_wds_ageout(unsigned long data)
{
	struct ieee80211_node_table *nt = (struct ieee80211_node_table *)data;
	int hash;
	struct ieee80211_wds_addr *wds;

	IEEE80211_NODE_LOCK_IRQ(nt);
	for (hash = 0; hash < IEEE80211_NODE_HASHSIZE; hash++) {
		LIST_FOREACH(wds, &nt->nt_wds_hash[hash], wds_hash) {
			if (wds->wds_agingcount != WDS_AGING_STATIC) {
				if (!wds->wds_agingcount) {
					if (ieee80211_node_dectestref(wds->wds_ni)) {
						_ieee80211_free_node(wds->wds_ni);  
						LIST_REMOVE(wds, wds_hash);
						FREE(wds, M_80211_WDS);
					}
				} else
					wds->wds_agingcount--;
			}
		}
	}
	IEEE80211_NODE_UNLOCK_IRQ(nt);
	mod_timer(&nt->nt_wds_aging_timer, jiffies + HZ * WDS_AGING_TIMER_VAL);
}


/*
 * Craft a temporary node suitable for sending a management frame
 * to the specified station.  We craft only as much state as we
 * need to do the work since the node will be immediately reclaimed
 * once the send completes.
 */
struct ieee80211_node *
ieee80211_tmp_node(struct ieee80211vap *vap, const u_int8_t *macaddr)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;
	int i;

	ni = ic->ic_node_alloc(&ic->ic_sta,vap);
	if (ni != NULL) {
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, 
		"%s: %p<%s> refcnt %d\n", __func__, ni, ether_sprintf(macaddr), 
		ieee80211_node_refcnt(ni)+1);

		IEEE80211_ADDR_COPY(ni->ni_macaddr, macaddr);
		IEEE80211_ADDR_COPY(ni->ni_bssid, vap->iv_bss->ni_bssid);
		ieee80211_node_initref(ni);		/* mark referenced */
		ni->ni_txpower = vap->iv_bss->ni_txpower;
		ni->ni_vap = vap;
		/* NB: required by ieee80211_fix_rate */
		ieee80211_node_set_chan(ic, ni);
		ieee80211_crypto_resetkey(vap, &ni->ni_ucastkey,
			IEEE80211_KEYIX_NONE);
		/* XXX optimize away */
		IEEE80211_NODE_SAVEQ_INIT(ni, "unknown");

		ni->ni_table = NULL;		/* NB: pedantic */
		ni->ni_ic = ic;
#define	N(a)	(sizeof(a)/sizeof(a[0]))
		for (i = 0; i < N(ni->ni_rxfrag); i++)
			ni->ni_rxfrag[i] = NULL;
#undef N
		ni->ni_challenge = NULL;
	} else {
		/* XXX msg */
		vap->iv_stats.is_rx_nodealloc++;
	}
	return ni;
}

/*
 * Add the specified station to the station table.
 */
struct ieee80211_node *
ieee80211_dup_bss(struct ieee80211vap *vap, const u_int8_t *macaddr)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;
	int i;

	ni = ieee80211_alloc_node(&ic->ic_sta, vap, macaddr);
	if (ni != NULL) {
		/*
		 * Inherit from iv_bss.
		 */
		ni->ni_authmode = vap->iv_bss->ni_authmode;
		ni->ni_txpower = vap->iv_bss->ni_txpower;
		ni->ni_vlan = vap->iv_bss->ni_vlan;	/* XXX?? */
		IEEE80211_ADDR_COPY(ni->ni_bssid, vap->iv_bss->ni_bssid);
		ieee80211_node_set_chan(ic, ni);
		ni->ni_rsn = vap->iv_bss->ni_rsn;
#define	N(a)	(sizeof(a)/sizeof(a[0]))
		for (i = 0; i < N(ni->ni_rxfrag); i++)
			ni->ni_rxfrag[i] = NULL;
#undef N
	}
	return ni;
}

static struct ieee80211_node *
_ieee80211_find_wds_node(struct ieee80211_node_table *nt, const u_int8_t *macaddr)
{
	struct ieee80211_node *ni;
	struct ieee80211_wds_addr *wds;
	int hash;
	IEEE80211_NODE_LOCK_ASSERT(nt);

	hash = IEEE80211_NODE_HASH(macaddr);
	LIST_FOREACH(wds, &nt->nt_wds_hash[hash], wds_hash) {
		if (IEEE80211_ADDR_EQ(wds->wds_macaddr, macaddr)) {
			ni = wds->wds_ni;
			if (wds->wds_agingcount != WDS_AGING_STATIC)
				wds->wds_agingcount = WDS_AGING_COUNT; /* reset the aging count */
			ieee80211_ref_node(ni);
			return ni;
		}
	}
	return NULL;
}

static struct ieee80211_node *
#ifdef IEEE80211_DEBUG_REFCNT
_ieee80211_find_node_debug(struct ieee80211_node_table *nt,
	const u_int8_t *macaddr, const char *func, int line)
#else
_ieee80211_find_node(struct ieee80211_node_table *nt, const u_int8_t *macaddr)
#endif
{
	struct ieee80211_node *ni;
	int hash;
	struct ieee80211_wds_addr *wds;

	IEEE80211_NODE_LOCK_ASSERT(nt);

	hash = IEEE80211_NODE_HASH(macaddr);
	LIST_FOREACH(ni, &nt->nt_hash[hash], ni_hash) {
		if (IEEE80211_ADDR_EQ(ni->ni_macaddr, macaddr)) {
			ieee80211_ref_node(ni);	/* mark referenced */
#ifdef IEEE80211_DEBUG_REFCNT
			IEEE80211_DPRINTF(ni->ni_vap, IEEE80211_MSG_NODE,
				"%s (%s:%u) %p<%s> refcnt %d\n", __func__,
				func, line,
				ni, ether_sprintf(ni->ni_macaddr),
				ieee80211_node_refcnt(ni));
#endif
			return ni;
		}
	}
	
	/* Now, we look for the desired mac address in the 4 address
	   nodes. */
	LIST_FOREACH(wds, &nt->nt_wds_hash[hash], wds_hash) {
		if (IEEE80211_ADDR_EQ(wds->wds_macaddr, macaddr)) {
			ni = wds->wds_ni;
			ieee80211_ref_node(ni);
			return ni;
		}
	}
	return NULL;
}
#ifdef IEEE80211_DEBUG_REFCNT
#define	_ieee80211_find_node(nt, mac) \
	_ieee80211_find_node_debug(nt, mac, func, line)
#endif

struct ieee80211_node *
ieee80211_find_wds_node(struct ieee80211_node_table *nt, const u_int8_t *macaddr)
{
	struct ieee80211_node *ni;

	IEEE80211_NODE_LOCK_IRQ(nt);
	ni = _ieee80211_find_wds_node(nt, macaddr);
	IEEE80211_NODE_UNLOCK_IRQ(nt);
	return ni;
}
EXPORT_SYMBOL(ieee80211_find_wds_node);

struct ieee80211_node *
#ifdef IEEE80211_DEBUG_REFCNT
ieee80211_find_node_debug(struct ieee80211_node_table *nt,
	const u_int8_t *macaddr, const char *func, int line)
#else
ieee80211_find_node(struct ieee80211_node_table *nt, const u_int8_t *macaddr)
#endif
{
	struct ieee80211_node *ni;

	IEEE80211_NODE_LOCK_IRQ(nt);
	ni = _ieee80211_find_node(nt, macaddr);
	IEEE80211_NODE_UNLOCK_IRQ(nt);
	return ni;
}
#ifdef IEEE80211_DEBUG_REFCNT
EXPORT_SYMBOL(ieee80211_find_node_debug);
#else
EXPORT_SYMBOL(ieee80211_find_node);
#endif

/*
 * Fake up a node; this handles node discovery in adhoc mode.
 * Note that for the driver's benefit we we treat this like
 * an association so the driver has an opportunity to setup
 * it's private state.
 *
 * Caller must ieee80211_ref_node()
 */
struct ieee80211_node *
ieee80211_fakeup_adhoc_node(struct ieee80211vap *vap,
	const u_int8_t macaddr[IEEE80211_ADDR_LEN])
{
	struct ieee80211_node *ni;

	ni = ieee80211_dup_bss(vap, macaddr);
	if (ni != NULL) {
		/* XXX no rate negotiation; just dup */
		ni->ni_rates = vap->iv_bss->ni_rates;
		if (vap->iv_ic->ic_newassoc != NULL)
			vap->iv_ic->ic_newassoc(ni, 1);
		/* XXX not right for 802.1x/WPA */
		ieee80211_node_authorize(ni);

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, 
		"%s: %p<%s> refcnt %d\n", __func__, ni, ether_sprintf(macaddr), 
		ieee80211_node_refcnt(ni));
	}
	return ni;
}

/*
 * Do node discovery in adhoc mode on receipt of a beacon
 * or probe response frame.  Note that for the driver's
 * benefit we treat this like an association so the
 * driver has an opportunity to setup it's private state.
 */
struct ieee80211_node *
ieee80211_add_neighbor(struct ieee80211vap *vap,	const struct ieee80211_frame *wh,
	const struct ieee80211_scanparams *sp)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;

	ni = ieee80211_dup_bss(vap, wh->i_addr2);	/* XXX alloc_node? */
	/* TODO: not really putting itself in a table */
	if (ni != NULL) {
		ni->ni_esslen = sp->ssid[1];
		memcpy(ni->ni_essid, sp->ssid + 2, sp->ssid[1]);
		IEEE80211_ADDR_COPY(ni->ni_bssid, wh->i_addr3);
		memcpy(ni->ni_tstamp.data, sp->tstamp, sizeof(ni->ni_tstamp));
		ni->ni_intval = sp->bintval;
		ni->ni_capinfo = sp->capinfo;
		ni->ni_chan = ic->ic_curchan;
		ni->ni_fhdwell = sp->fhdwell;
		ni->ni_fhindex = sp->fhindex;
		ni->ni_erp = sp->erp;
		ni->ni_timoff = sp->timoff;
		if (sp->wme != NULL)
			ieee80211_saveie(&ni->ni_wme_ie, sp->wme);
		if (sp->wpa != NULL)
			ieee80211_saveie(&ni->ni_wpa_ie, sp->wpa);
		if (sp->rsn != NULL)
			ieee80211_saveie(&ni->ni_rsn_ie, sp->rsn);
		if (sp->ath != NULL)
			ieee80211_saveath(ni, sp->ath);

		/* NB: must be after ni_chan is setup */
		ieee80211_setup_rates(ni, sp->rates, sp->xrates, IEEE80211_F_DOSORT);

		if (ic->ic_newassoc != NULL)
			ic->ic_newassoc(ni, 1);
		/* XXX not right for 802.1x/WPA */
		ieee80211_node_authorize(ni);
		if (vap->iv_opmode == IEEE80211_M_AHDEMO) {
			/*
			 * Blindly propagate capabilities based on the
			 * local configuration.  In particular this permits
			 * us to use QoS to disable ACK's and to use short
			 * preamble on 2.4G channels.
			 */
			if (vap->iv_flags & IEEE80211_F_WME)
				ni->ni_flags |= IEEE80211_NODE_QOS;
			if (vap->iv_flags & IEEE80211_F_SHPREAMBLE)
				ni->ni_capinfo |= IEEE80211_CAPINFO_SHORT_PREAMBLE;
		}

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE, 
		"%s: %p<%s> refcnt %d\n", __func__, ni, ether_sprintf(ni->ni_macaddr), 
		ieee80211_node_refcnt(ni));
	}
	return ni;
}

/*
 * Locate the node for sender, track state, and then pass the
 * (referenced) node up to the 802.11 layer for its use.  We
 * return NULL when the sender is unknown; the driver is required
 * locate the appropriate virtual ap in that case; possibly
 * sending it to all (using ieee80211_input_all).
 */
struct ieee80211_node *
#ifdef IEEE80211_DEBUG_REFCNT
ieee80211_find_rxnode_debug(struct ieee80211com *ic,
	const struct ieee80211_frame_min *wh, const char *func, int line)
#else
ieee80211_find_rxnode(struct ieee80211com *ic,
	const struct ieee80211_frame_min *wh)
#endif
{
#define	IS_CTL(wh) \
	((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL)
#define	IS_PSPOLL(wh) \
	((wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_PS_POLL)
	struct ieee80211_node_table *nt;
	struct ieee80211_node *ni;

	/* XXX check ic_bss first in station mode */
	/* XXX 4-address frames? */
	nt = &ic->ic_sta;
	IEEE80211_NODE_LOCK_IRQ(nt);
	if (IS_CTL(wh) && !IS_PSPOLL(wh) /*&& !IS_RTS(ah)*/)
		ni = _ieee80211_find_node(nt, wh->i_addr1);
	else
		ni = _ieee80211_find_node(nt, wh->i_addr2);
	IEEE80211_NODE_UNLOCK_IRQ(nt);

	return ni;
#undef IS_PSPOLL
#undef IS_CTL
}
#ifdef IEEE80211_DEBUG_REFCNT
EXPORT_SYMBOL(ieee80211_find_rxnode_debug);
#else
EXPORT_SYMBOL(ieee80211_find_rxnode);
#endif

/*
 * Return a reference to the appropriate node for sending
 * a data frame.  This handles node discovery in adhoc networks.
 */
struct ieee80211_node *
#ifdef IEEE80211_DEBUG_REFCNT
ieee80211_find_txnode_debug(struct ieee80211vap *vap, const u_int8_t *mac,
	const char *func, int line)
#else
ieee80211_find_txnode(struct ieee80211vap *vap, const u_int8_t *mac)
#endif
{
	struct ieee80211_node_table *nt;
	struct ieee80211_node *ni;

	/*
	 * The destination address should be in the node table
	 * unless we are operating in station mode or this is a
	 * multicast/broadcast frame.
	 */
	if (vap->iv_opmode == IEEE80211_M_STA || IEEE80211_IS_MULTICAST(mac))
		return ieee80211_ref_node(vap->iv_bss);

	/* XXX can't hold lock across dup_bss due to recursive locking */
	nt = &vap->iv_ic->ic_sta;
	IEEE80211_NODE_LOCK_IRQ(nt);
	ni = _ieee80211_find_node(nt, mac);
	IEEE80211_NODE_UNLOCK_IRQ(nt);

	if (ni == NULL) {
		if (vap->iv_opmode == IEEE80211_M_IBSS ||
		    vap->iv_opmode == IEEE80211_M_AHDEMO) {
			/*
			 * In adhoc mode cons up a node for the destination.
			 * Note that we need an additional reference for the
			 * caller to be consistent with _ieee80211_find_node.
			 */
			ni = ieee80211_fakeup_adhoc_node(vap, mac);
			if (ni != NULL)
				(void) ieee80211_ref_node(ni);
		} else {
			IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_OUTPUT, mac,
				"no node, discard frame (%s)", __func__);
			vap->iv_stats.is_tx_nonode++;
		}
	}
	return ni;
}
#ifdef IEEE80211_DEBUG_REFCNT
EXPORT_SYMBOL(ieee80211_find_txnode_debug);
#else
EXPORT_SYMBOL(ieee80211_find_txnode);
#endif

/* Caller must lock the IEEE80211_NODE_LOCK
 *
 * Context: hwIRQ, softIRQ and process context
 */
static void
_ieee80211_free_node(struct ieee80211_node *ni)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211_node_table *nt = ni->ni_table;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_NODE,
		"%s: %p<%s> in %s table, refcnt %d\n", __func__, ni,
		ether_sprintf(ni->ni_macaddr),
		nt != NULL ? nt->nt_name : "<gone>",
		ieee80211_node_refcnt(ni));

	if (vap->iv_aid_bitmap != NULL)
		IEEE80211_AID_CLR(vap, ni->ni_associd);
	if (nt != NULL) {
		TAILQ_REMOVE(&nt->nt_node, ni, ni_list);
		LIST_REMOVE(ni, ni_hash);
	}
	vap->iv_ic->ic_node_free(ni);
}

void
#ifdef IEEE80211_DEBUG_REFCNT
ieee80211_free_node_debug(struct ieee80211_node *ni, const char *func, int line)
#else
ieee80211_free_node(struct ieee80211_node *ni)
#endif
{
	struct ieee80211_node_table *nt = ni->ni_table;
	struct ieee80211com *ic = ni->ni_ic;

#ifdef IEEE80211_DEBUG_REFCNT
	IEEE80211_DPRINTF(ni->ni_vap, IEEE80211_MSG_NODE,
		"%s (%s:%u) %p<%s> refcnt %d\n", __func__, func, line, ni,
		 ether_sprintf(ni->ni_macaddr), ieee80211_node_refcnt(ni) - 1);
#endif
	/*
	 * XXX: may need to lock out the following race. we dectestref
	 *      and determine it's time to free the node. between the if()
	 *      and lock, we take an rx intr to receive a frame from this
	 *      node. the rx path (tasklet or intr) bumps this node's
	 *      refcnt and xmits a response frame. eventually that response
	 *      will get reaped, and the reaping code will attempt to use
	 *      the node. the code below will delete the node prior
	 *      to the reap and we could get a crash.
	 *
	 *      as a stopgap before delving deeper, lock intrs to
	 *      prevent this case.
	 */
	IEEE80211_LOCK_IRQ(ic);
	if (ieee80211_node_dectestref(ni)) {
		/*
		 * Beware; if the node is marked gone then it's already
		 * been removed from the table and we cannot assume the
		 * table still exists.  Regardless, there's no need to lock
		 * the table.
		 */
		if (ni->ni_table != NULL) {
			IEEE80211_NODE_LOCK(nt);
			_ieee80211_free_node(ni);
			IEEE80211_NODE_UNLOCK(nt);
		} else
			_ieee80211_free_node(ni);
	}
	IEEE80211_UNLOCK_IRQ(ic);
}
#ifdef IEEE80211_DEBUG_REFCNT
EXPORT_SYMBOL(ieee80211_free_node_debug);
#else
EXPORT_SYMBOL(ieee80211_free_node);
#endif

/*
 * Reclaim a node.  If this is the last reference count then
 * do the normal free work.  Otherwise remove it from the node
 * table and mark it gone by clearing the back-reference.
 */
static void
node_reclaim(struct ieee80211_node_table *nt, struct ieee80211_node *ni)
{

	IEEE80211_DPRINTF(ni->ni_vap, IEEE80211_MSG_NODE,
		"%s: remove %p<%s> from %s table, refcnt %d\n",
		__func__, ni, ether_sprintf(ni->ni_macaddr),
		nt->nt_name, ieee80211_node_refcnt(ni)-1);
	if (!ieee80211_node_dectestref(ni)) {
		/*
		 * Other references are present, just remove the
		 * node from the table so it cannot be found.  When
		 * the references are dropped storage will be
		 * reclaimed.  This normally only happens for ic_bss.
		 */
		TAILQ_REMOVE(&nt->nt_node, ni, ni_list);
		LIST_REMOVE(ni, ni_hash);
		ni->ni_table = NULL;		/* clear reference */
	} else
		_ieee80211_free_node(ni);
}

static void
ieee80211_node_table_reset(struct ieee80211_node_table *nt,
	struct ieee80211vap *match)
{
	struct ieee80211_node *ni, *next;

	IEEE80211_NODE_LOCK_IRQ(nt);
	TAILQ_FOREACH_SAFE(ni, &nt->nt_node, ni_list, next) {
		if (match != NULL && ni->ni_vap != match)
			continue;
		if (ni->ni_associd != 0) {
			struct ieee80211vap *vap = ni->ni_vap;

			if (vap->iv_auth->ia_node_leave != NULL)
				vap->iv_auth->ia_node_leave(ni);
			if (vap->iv_aid_bitmap != NULL)
				IEEE80211_AID_CLR(vap, ni->ni_associd);
		}
		node_reclaim(nt, ni);
	}
	IEEE80211_NODE_UNLOCK_IRQ(nt);
}

static void
ieee80211_node_table_cleanup(struct ieee80211_node_table *nt)
{
	struct ieee80211_node *ni, *next;

	TAILQ_FOREACH_SAFE(ni, &nt->nt_node, ni_list, next) {
		if (ni->ni_associd != 0) {
			struct ieee80211vap *vap = ni->ni_vap;

			if (vap->iv_auth->ia_node_leave != NULL)
				vap->iv_auth->ia_node_leave(ni);
			if (vap->iv_aid_bitmap != NULL)
				IEEE80211_AID_CLR(vap, ni->ni_associd);
		}
		node_reclaim(nt, ni);
	}
	del_timer(&nt->nt_wds_aging_timer);
	IEEE80211_SCAN_LOCK_DESTROY(nt);
	IEEE80211_NODE_LOCK_DESTROY(nt);
}

/*
 * Timeout inactive stations and do related housekeeping.
 * Note that we cannot hold the node lock while sending a
 * frame as this would lead to a LOR.  Instead we use a
 * generation number to mark nodes that we've scanned and
 * drop the lock and restart a scan if we have to time out
 * a node.  Since we are single-threaded by virtue of
 * controlling the inactivity timer we can be sure this will
 * process each node only once.
 *
 * Context: softIRQ (tasklet)
 */
static void
ieee80211_timeout_stations(struct ieee80211_node_table *nt)
{
	struct ieee80211com *ic = nt->nt_ic;
	struct ieee80211_node *ni;
	u_int gen;
	int isadhoc;

	isadhoc = (ic->ic_opmode == IEEE80211_M_IBSS ||
		   ic->ic_opmode == IEEE80211_M_AHDEMO);
	IEEE80211_SCAN_LOCK_IRQ(nt); 
	gen = ++nt->nt_scangen;
restart:
	IEEE80211_NODE_LOCK_IRQ(nt);
	TAILQ_FOREACH(ni, &nt->nt_node, ni_list) {
		if (ni->ni_scangen == gen)	/* previously handled */
			continue;
		/*
		 * Ignore entries for which have yet to receive an
		 * authentication frame.  These are transient and
		 * will be reclaimed when the last reference to them
		 * goes away (when frame xmits complete).
		 */
		if (ic->ic_opmode == IEEE80211_M_HOSTAP &&
		    (ni->ni_flags & IEEE80211_NODE_AREF) == 0)
			continue;
		ni->ni_scangen = gen;
		/*
		 * Free fragment if not needed anymore
		 * (last fragment older than 1s).
		 * XXX doesn't belong here
		 */
		if (ni->ni_rxfrag[0] != NULL &&
		    jiffies > ni->ni_rxfragstamp + HZ) {
			dev_kfree_skb(ni->ni_rxfrag[0]);
			ni->ni_rxfrag[0] = NULL;
		}
		/*
		 * Special case ourself; we may be idle for extended periods
		 * of time and regardless reclaiming our state is wrong.
		 */
		if (ni == ni->ni_vap->iv_bss) {
			/* NB: don't permit it to go negative */
			if (ni->ni_inact > 0)
				ni->ni_inact--;
			continue;
		}
		ni->ni_inact--;
		if (ni->ni_associd != 0 || isadhoc) {
			struct ieee80211vap *vap = ni->ni_vap;
			/*
			 * Age frames on the power save queue.
			 */
			if (ieee80211_node_saveq_age(ni) != 0 &&
			    IEEE80211_NODE_SAVEQ_QLEN(ni) == 0 &&
			    vap->iv_set_tim != NULL)
				vap->iv_set_tim(ni, 0);
			/*
			 * Probe the station before time it out.  We
			 * send a null data frame which may not be
			 * universally supported by drivers (need it
			 * for ps-poll support so it should be...).
			 */
			if (0 < ni->ni_inact &&
			    ni->ni_inact <= vap->iv_inact_probe) {
				IEEE80211_NOTE(vap,
					IEEE80211_MSG_INACT | IEEE80211_MSG_NODE,
					ni, "%s",
					"probe station due to inactivity");
				/*
				 * Grab a reference before unlocking the table
				 * so the node cannot be reclaimed before we
				 * send the frame. ieee80211_send_nulldata
				 * understands we've done this and reclaims the
				 * ref for us as needed.
				 */
				ieee80211_ref_node(ni);
				IEEE80211_NODE_UNLOCK_IRQ_EARLY(nt);
				ieee80211_send_nulldata(ni);
				/* XXX stat? */
				goto restart;
			}
		}
		if (ni->ni_inact <= 0) {
			IEEE80211_NOTE(ni->ni_vap,
				IEEE80211_MSG_INACT | IEEE80211_MSG_NODE, ni,
				"station timed out due to inactivity (refcnt %u)",
				ieee80211_node_refcnt(ni));
			/*
			 * Send a deauthenticate frame and drop the station.
			 * We grab a reference before unlocking the table so
			 * the node cannot be reclaimed before we complete our
			 * work.
			 *
			 * Separately we must drop the node lock before sending
			 * in case the driver takes a lock, as this may result
			 * in a LOR between the node lock and the driver lock.
			 */
			ni->ni_vap->iv_stats.is_node_timeout++;
			ieee80211_ref_node(ni);
			IEEE80211_NODE_UNLOCK_IRQ_EARLY(nt);
			if (ni->ni_associd != 0) {
				IEEE80211_SEND_MGMT(ni,
					IEEE80211_FC0_SUBTYPE_DEAUTH,
					IEEE80211_REASON_AUTH_EXPIRE);
			}
			ieee80211_node_leave(ni);
			ieee80211_free_node(ni);
			goto restart;
		}
	}
	IEEE80211_NODE_UNLOCK_IRQ(nt);

	IEEE80211_SCAN_UNLOCK_IRQ(nt);
}

/*
 * Per-ieee80211com inactivity timer callback.
 */
static void
ieee80211_node_timeout(unsigned long arg)
{
	struct ieee80211com *ic = (struct ieee80211com *) arg;

	ieee80211_scan_timeout(ic);
	ieee80211_timeout_stations(&ic->ic_sta);

	ic->ic_inact.expires = jiffies + IEEE80211_INACT_WAIT * HZ;
	add_timer(&ic->ic_inact);
}

void
ieee80211_iterate_nodes(struct ieee80211_node_table *nt, ieee80211_iter_func *f, void *arg)
{
	struct ieee80211_node *ni;
	u_int gen;

	IEEE80211_SCAN_LOCK_IRQ(nt);
	gen = ++nt->nt_scangen;
restart:
	IEEE80211_NODE_LOCK(nt);
	TAILQ_FOREACH(ni, &nt->nt_node, ni_list) {
		if (ni->ni_scangen != gen) {
			ni->ni_scangen = gen;
			(void) ieee80211_ref_node(ni);
			IEEE80211_NODE_UNLOCK(nt);
			(*f)(arg, ni);
			ieee80211_free_node(ni);
			goto restart;
		}
	}
	IEEE80211_NODE_UNLOCK(nt);

	IEEE80211_SCAN_UNLOCK_IRQ(nt);
}
EXPORT_SYMBOL(ieee80211_iterate_nodes);

void
ieee80211_dump_node(struct ieee80211_node_table *nt, struct ieee80211_node *ni)
{
	int i;

	printf("0x%p: mac %s refcnt %d\n", ni,
		ether_sprintf(ni->ni_macaddr), ieee80211_node_refcnt(ni));
	printf("\tscangen %u authmode %u flags 0x%x\n",
		ni->ni_scangen, ni->ni_authmode, ni->ni_flags);
	printf("\tassocid 0x%x txpower %u vlan %u\n",
		ni->ni_associd, ni->ni_txpower, ni->ni_vlan);
	printf ("rxfragstamp %u\n", ni->ni_rxfragstamp);
	for (i = 0; i < 17; i++) {
		printf("\t%d: txseq %u rxseq %u fragno %u\n", i, 
		       ni->ni_txseqs[i],
		       ni->ni_rxseqs[i] >> IEEE80211_SEQ_SEQ_SHIFT,
		       ni->ni_rxseqs[i] & IEEE80211_SEQ_FRAG_MASK);
	}
	printf("\trstamp %u rssi %u intval %u capinfo 0x%x\n",
		ni->ni_rstamp, ni->ni_rssi, ni->ni_intval, ni->ni_capinfo);
	printf("\tbssid %s essid \"%.*s\" channel %u:0x%x\n",
		ether_sprintf(ni->ni_bssid),
		ni->ni_esslen, ni->ni_essid,
		ni->ni_chan != IEEE80211_CHAN_ANYC ?
			ni->ni_chan->ic_freq : IEEE80211_CHAN_ANY,
		ni->ni_chan != IEEE80211_CHAN_ANYC ? ni->ni_chan->ic_flags : 0);
	printf("\tinact %u txrate %u\n",
		ni->ni_inact, ni->ni_txrate);
}

void
ieee80211_dump_nodes(struct ieee80211_node_table *nt)
{
	ieee80211_iterate_nodes(nt,
		(ieee80211_iter_func *) ieee80211_dump_node, nt);
}

/*
 * Handle a station joining an 11g network.
 */
static void
ieee80211_node_join_11g(struct ieee80211_node *ni)
{
	struct ieee80211com *ic = ni->ni_ic;
#ifdef IEEE80211_DEBUG
	struct ieee80211vap *vap = ni->ni_vap;
#endif
	IEEE80211_LOCK_ASSERT(ic);

	KASSERT(IEEE80211_IS_CHAN_ANYG(ic->ic_bsschan),
	     ("not in 11g, bss %u:0x%x, curmode %u", ic->ic_bsschan->ic_freq,
	      ic->ic_bsschan->ic_flags, ic->ic_curmode));

	/*
	 * Station isn't capable of short slot time.  Bump
	 * the count of long slot time stations and disable
	 * use of short slot time.  Note that the actual switch
	 * over to long slot time use may not occur until the
	 * next beacon transmission (per sec. 7.3.1.4 of 11g).
	 */
	if ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME) == 0) {
		ic->ic_longslotsta++;
		IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
			"station needs long slot time, count %d",
			ic->ic_longslotsta);
		/* XXX vap's w/ conflicting needs won't work */
		if (!IEEE80211_IS_CHAN_108G(ic->ic_bsschan)) {
			/*
			 * Don't force slot time when switched to turbo
			 * mode as non-ERP stations won't be present; this
			 * need only be done when on the normal G channel.
			 */
			ieee80211_set_shortslottime(ic, 0);
		}
	}
	/*
	 * If the new station is not an ERP station
	 * then bump the counter and enable protection
	 * if configured.
	 */
	if (!ieee80211_iserp_rateset(ic, &ni->ni_rates)) {
		ic->ic_nonerpsta++;
		IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
			"station is !ERP, %d non-ERP stations associated",
			ic->ic_nonerpsta);
		/*
		 * If protection is configured, enable it.
		 */
		if (ic->ic_protmode != IEEE80211_PROT_NONE) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC,
				"%s: enable use of protection\n", __func__);
			ic->ic_flags |= IEEE80211_F_USEPROT;
		}
		/*
		 * If station does not support short preamble
		 * then we must enable use of Barker preamble.
		 */
		if ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_PREAMBLE) == 0) {
			IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
				"%s", "station needs long preamble");
			ic->ic_flags |= IEEE80211_F_USEBARKER;
			ic->ic_flags &= ~IEEE80211_F_SHPREAMBLE;
		}

		/* Update ERP element if this is first non ERP station */
		if (ic->ic_nonerpsta == 1)
			ic->ic_flags_ext |= IEEE80211_FEXT_ERPUPDATE;
	} else
		ni->ni_flags |= IEEE80211_NODE_ERP;
}

void
ieee80211_node_join(struct ieee80211_node *ni, int resp)
{
	struct ieee80211com *ic = ni->ni_ic;
	struct ieee80211vap *vap = ni->ni_vap;
	int newassoc;

	if (ni->ni_associd == 0) {
		u_int16_t aid;

		KASSERT(vap->iv_aid_bitmap != NULL, ("no aid bitmap"));
		/*
		 * It would be good to search the bitmap
		 * more efficiently, but this will do for now.
		 */
		for (aid = 1; aid < vap->iv_max_aid; aid++)
			if (!IEEE80211_AID_ISSET(vap, aid))
				break;
		if (aid >= vap->iv_max_aid) {
			IEEE80211_SEND_MGMT(ni, resp,
				IEEE80211_REASON_ASSOC_TOOMANY);
			ieee80211_node_leave(ni);
			return;
		}
		ni->ni_associd = aid | 0xc000;

		IEEE80211_LOCK_IRQ(ic);
		IEEE80211_AID_SET(vap, ni->ni_associd);
		vap->iv_sta_assoc++;
		ic->ic_sta_assoc++;
#ifdef ATH_SUPERG_XR
		if (ni->ni_vap->iv_flags & IEEE80211_F_XR)
			ic->ic_xr_sta_assoc++;
#endif
		if (IEEE80211_ATH_CAP(vap, ni, IEEE80211_ATHC_TURBOP))
			ic->ic_dt_sta_assoc++;

		if (IEEE80211_IS_CHAN_ANYG(ic->ic_bsschan))
			ieee80211_node_join_11g(ni);
		IEEE80211_UNLOCK_IRQ(ic);

		newassoc = 1;
	} else
		newassoc = 0;

	IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DEBUG, ni,
		"station %sassociated at aid %d: %s preamble, %s slot time"
		"%s%s%s%s%s%s%s",
		newassoc ? "" : "re",
		IEEE80211_NODE_AID(ni),
		ic->ic_flags & IEEE80211_F_SHPREAMBLE ? "short" : "long",
		ic->ic_flags & IEEE80211_F_SHSLOT ? "short" : "long",
		ic->ic_flags & IEEE80211_F_USEPROT ? ", protection" : "",
		ni->ni_flags & IEEE80211_NODE_QOS ? ", QoS" : "",
		IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_TURBOP) ?
			", turbo" : "",
		IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_COMP) ?
			", compression" : "",
		IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_FF) ?
			", fast-frames" : "",
		IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_XR) ? ", XR" : "",
		IEEE80211_ATH_CAP(vap, ni, IEEE80211_NODE_AR) ? ", AR" : ""
	);

	/* give driver a chance to setup state like ni_txrate */
	if (ic->ic_newassoc != NULL)
		ic->ic_newassoc(ni, newassoc);
	ni->ni_inact_reload = vap->iv_inact_auth;
	ni->ni_inact = ni->ni_inact_reload;
	IEEE80211_SEND_MGMT(ni, resp, IEEE80211_STATUS_SUCCESS);
	/* tell the authenticator about new station */
	if (vap->iv_auth->ia_node_join != NULL)
		vap->iv_auth->ia_node_join(ni);
	ieee80211_notify_node_join(ni, newassoc);
}

/*
 * Handle a station leaving an 11g network.
 */
static void
ieee80211_node_leave_11g(struct ieee80211_node *ni)
{
	struct ieee80211com *ic = ni->ni_ic;
	struct ieee80211vap *vap = ni->ni_vap;

	IEEE80211_LOCK_ASSERT(ic);

	KASSERT(IEEE80211_IS_CHAN_ANYG(ic->ic_bsschan),
		("not in 11g, bss %u:0x%x, curmode %u",
		ic->ic_bsschan->ic_freq, ic->ic_bsschan->ic_flags,
		ic->ic_curmode));

	/*
	 * If a long slot station do the slot time bookkeeping.
	 */
	if ((ni->ni_capinfo & IEEE80211_CAPINFO_SHORT_SLOTTIME) == 0) {
		/* this can be 0 on mode changes from B -> G */
		if (ic->ic_longslotsta > 0)
			ic->ic_longslotsta--;
		IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
			"long slot time station leaves, count now %d",
			ic->ic_longslotsta);
		if (ic->ic_longslotsta == 0) {
			/*
			 * Re-enable use of short slot time if supported
			 * and not operating in IBSS mode (per spec).
			 */
			if ((ic->ic_caps & IEEE80211_C_SHSLOT) &&
			    vap->iv_opmode != IEEE80211_M_IBSS) {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC,
					"%s: re-enable use of short slot time\n",
					__func__);
				ieee80211_set_shortslottime(ic, 1);
			}
		}
	}
	/*
	 * If a non-ERP station do the protection-related bookkeeping.
	 */
	if ((ni->ni_flags & IEEE80211_NODE_ERP) == 0) {
		/* this can be 0 on mode changes from B -> G */
		if (ic->ic_nonerpsta > 0)
			ic->ic_nonerpsta--;
		IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC, ni,
			"non-ERP station leaves, count now %d", ic->ic_nonerpsta);
		if (ic->ic_nonerpsta == 0) {
			IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC,
				"%s: disable use of protection\n", __func__);
			ic->ic_flags &= ~IEEE80211_F_USEPROT;
			/* XXX verify mode? */
			if (ic->ic_caps & IEEE80211_C_SHPREAMBLE) {
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_ASSOC,
					"%s: re-enable use of short preamble\n",
					__func__);
				ic->ic_flags |= IEEE80211_F_SHPREAMBLE;
				ic->ic_flags &= ~IEEE80211_F_USEBARKER;
			}
			ic->ic_flags_ext |= IEEE80211_FEXT_ERPUPDATE;
		}
	}
}

/*
 * Handle bookkeeping for a station/neighbor leaving
 * the bss when operating in ap or adhoc modes.
 */
void
ieee80211_node_leave(struct ieee80211_node *ni)
{
	struct ieee80211com *ic = ni->ni_ic;
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211_node_table *nt = ni->ni_table;

	IEEE80211_NOTE(vap, IEEE80211_MSG_ASSOC | IEEE80211_MSG_DEBUG, ni,
		"station with aid %d leaves (refcnt %u)",
		IEEE80211_NODE_AID(ni), ieee80211_node_refcnt(ni));

	/*
	 * If node wasn't previously associated all
	 * we need to do is reclaim the reference.
	 */
	/* XXX ibss mode bypasses 11g and notification */
	if (ni->ni_associd == 0)
		goto done;
	/*
	 * Tell the authenticator the station is leaving.
	 * Note that we must do this before yanking the
	 * association id as the authenticator uses the
	 * associd to locate it's state block.
	 */
	if (vap->iv_auth->ia_node_leave != NULL)
		vap->iv_auth->ia_node_leave(ni);

	IEEE80211_LOCK_IRQ(ic);
	if (vap->iv_aid_bitmap != NULL)
		IEEE80211_AID_CLR(vap, ni->ni_associd);
	ni->ni_associd = 0;
	vap->iv_sta_assoc--;
	ic->ic_sta_assoc--;
#ifdef ATH_SUPERG_XR
	if (ni->ni_vap->iv_flags & IEEE80211_F_XR)
		ic->ic_xr_sta_assoc--;
#endif
	if (IEEE80211_ATH_CAP(vap, ni, IEEE80211_ATHC_TURBOP))
		ic->ic_dt_sta_assoc--;

	if (IEEE80211_IS_CHAN_ANYG(ic->ic_bsschan))
		ieee80211_node_leave_11g(ni);
	IEEE80211_UNLOCK_IRQ(ic);
	/*
	 * Cleanup station state.  In particular clear various
	 * state that might otherwise be reused if the node
	 * is reused before the reference count goes to zero
	 * (and memory is reclaimed).
	 */
	ieee80211_sta_leave(ni);
done:
	/*
	 * Remove the node from any table it's recorded in and
	 * drop the caller's reference.  Removal from the table
	 * is important to ensure the node is not reprocessed
	 * for inactivity.
	 */
	if (nt != NULL) {
		IEEE80211_NODE_LOCK_IRQ(nt);
		node_reclaim(nt, ni);
		IEEE80211_NODE_UNLOCK_IRQ(nt);
		ieee80211_remove_wds_addr(nt,ni->ni_macaddr);
		ieee80211_del_wds_node(nt,ni);
	} else
		ieee80211_free_node(ni);
}
EXPORT_SYMBOL(ieee80211_node_leave);

u_int8_t
ieee80211_getrssi(struct ieee80211com *ic)
{
#define	NZ(x)	((x) == 0 ? 1 : (x))
	struct ieee80211_node_table *nt = &ic->ic_sta;
	struct ieee80211vap *vap;
	u_int32_t rssi_samples, rssi_total;
	struct ieee80211_node *ni;

	rssi_total = 0;
	rssi_samples = 0;
	switch (ic->ic_opmode) {
	case IEEE80211_M_IBSS:		/* average of all ibss neighbors */
		/* XXX locking */
		TAILQ_FOREACH(ni, &nt->nt_node, ni_list)
			if (ni->ni_capinfo & IEEE80211_CAPINFO_IBSS) {
				rssi_samples++;
				rssi_total += ic->ic_node_getrssi(ni);
			}
		break;
	case IEEE80211_M_AHDEMO:	/* average of all neighbors */
		/* XXX locking */
		TAILQ_FOREACH(ni, &nt->nt_node, ni_list) {
			if (memcmp(ni->ni_vap->iv_myaddr, ni->ni_macaddr, 
						IEEE80211_ADDR_LEN)!=0) {
				rssi_samples++;
				rssi_total += ic->ic_node_getrssi(ni);
			}
		}
		break;
	case IEEE80211_M_HOSTAP:	/* average of all associated stations */
		/* XXX locking */
		TAILQ_FOREACH(ni, &nt->nt_node, ni_list)
			if (IEEE80211_AID(ni->ni_associd) != 0) {
				rssi_samples++;
				rssi_total += ic->ic_node_getrssi(ni);
			}
		break;
	case IEEE80211_M_MONITOR:	/* XXX */
	case IEEE80211_M_STA:		/* use stats from associated ap */
	default:
		TAILQ_FOREACH(vap, &ic->ic_vaps, iv_next)
			if (vap->iv_bss != NULL) {
				rssi_samples++;
				rssi_total += ic->ic_node_getrssi(vap->iv_bss);
			}
		break;
	}
	return rssi_total / NZ(rssi_samples);
#undef NZ
}
EXPORT_SYMBOL(ieee80211_getrssi);

void
ieee80211_node_reset(struct ieee80211_node *ni, struct ieee80211vap *vap)
{
	if (ni != NULL) {
		struct ieee80211_node_table *nt = ni->ni_table;
		if (!nt)
			nt = &vap->iv_ic->ic_sta;
		IEEE80211_ADDR_COPY(ni->ni_bssid, vap->iv_bss->ni_bssid);
		ni->ni_prev_vap = ni->ni_vap;
		ni->ni_vap = vap;
		ni->ni_ic = vap->iv_ic;
		/* 
		 * if node not found in the node table
		 * add it to the node table .
		 */
		if(nt && ieee80211_find_node(nt, ni->ni_macaddr) != ni) {
			int hash = IEEE80211_NODE_HASH(ni->ni_macaddr);
			IEEE80211_NODE_LOCK_IRQ(nt);
			TAILQ_INSERT_TAIL(&nt->nt_node, ni, ni_list);
			LIST_INSERT_HEAD(&nt->nt_hash[hash], ni, ni_hash);
			ni->ni_table = nt;
			IEEE80211_NODE_UNLOCK_IRQ(nt);
		}
	}
}
