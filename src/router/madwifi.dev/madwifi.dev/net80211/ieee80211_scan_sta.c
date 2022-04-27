/*-
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
 * $Id: ieee80211_scan_sta.c 3268 2008-01-26 20:48:11Z mtaylor $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 station scanning support.
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/delay.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>

/*
 * Parameters for managing cache entries:
 *
 * o a station with STA_FAILS_MAX failures is not considered
 *   when picking a candidate
 * o a station that hasn't had an update in STA_PURGE_SCANS
 *   (background) scans is discarded
 * o after STA_FAILS_AGE seconds we clear the failure count
 */
#define	STA_FAILS_MAX	2	/* assoc failures before ignored */
#define	STA_FAILS_AGE	(2 * 60)	/* time before clearing fails (secs) */
#define	STA_PURGE_SCANS	2	/* age for purging entries (scans) */

#define RSSI_LPF_LEN	10
#define	RSSI_EP_MULTIPLIER	(1<<7)	/* pow2 to optimize out * and / */
#define RSSI_IN(x)		((x) * RSSI_EP_MULTIPLIER)
#define LPF_RSSI(x, y, len)	(((x) * ((len) - 1) + (y)) / (len))
#define RSSI_LPF(x, y) do {						\
	if ((y) >= -20)							\
		x = LPF_RSSI((x), RSSI_IN((y)), RSSI_LPF_LEN);			\
} while (0)
#define	EP_RND(x, mul) \
	((((x)%(mul)) >= ((mul)/2)) ? howmany(x, mul) : (x)/(mul))
#define	RSSI_GET(x)	EP_RND(x, RSSI_EP_MULTIPLIER)

struct sta_entry {
	struct ieee80211_scan_entry base;
	 TAILQ_ENTRY(sta_entry) se_list;
	 LIST_ENTRY(sta_entry) se_hash;
	u_int8_t se_fails;	/* failure to associate count */
	u_int8_t se_seen;	/* seen during current scan */
	u_int8_t se_notseen;	/* not seen in previous scans */
	u_int32_t se_avgrssi;	/* LPF rssi state */
	unsigned long se_lastupdate;	/* time of last update */
	unsigned long se_lastfail;	/* time of last failure */
	unsigned long se_lastassoc;	/* time of last association */
	u_int se_scangen;	/* iterator scan gen# */
};

#define	STA_HASHSIZE	32
/* simple hash is enough for variation of macaddr */
#define	STA_HASH(addr)	\
	(((const u_int8_t *)(addr))[IEEE80211_ADDR_LEN - 1] % STA_HASHSIZE)

struct sta_table {
	spinlock_t st_lock;	/* on scan table */
	 TAILQ_HEAD(, sta_entry) st_entry;	/* all entries */
	 ATH_LIST_HEAD(, sta_entry) st_hash[STA_HASHSIZE];
	spinlock_t st_scanlock;	/* on st_scangen */
	u_int st_scangen;	/* gen# for iterator */
	int st_newscan;
	struct IEEE80211_TQ_STRUCT st_actiontq;	/* tasklet for "action" */
	struct ieee80211_scan_entry st_selbss;	/* selected bss for action tasklet */
	int (*st_action)(struct ieee80211vap *, const struct ieee80211_scan_entry *);
};

#define	SCAN_STA_LOCK_INIT(_st, _name)					\
	spin_lock_init(&(_st)->st_lock)
#define	SCAN_STA_LOCK_DESTROY(_st)
#define	SCAN_STA_LOCK_IRQ(_st) do {					\
	unsigned long __stlockflags;					\
	spin_lock_irqsave(&(_st)->st_lock, __stlockflags);
#define	SCAN_STA_UNLOCK_IRQ(_st)					\
	spin_unlock_irqrestore(&(_st)->st_lock, __stlockflags);		\
} while (0)
#define	SCAN_STA_UNLOCK_IRQ_EARLY(_st)					\
	spin_unlock_irqrestore(&(_st)->st_lock, __stlockflags);

#define	SCAN_STA_GEN_LOCK_INIT(_st, _name)				\
	spin_lock_init(&(_st)->st_scanlock)
#define	SCAN_STA_GEN_LOCK_DESTROY(_st)
#define	SCAN_STA_GEN_LOCK(_st)		spin_lock(&(_st)->st_scanlock);
#define	SCAN_STA_GEN_UNLOCK(_st)	spin_unlock(&(_st)->st_scanlock);

static void sta_flush_table(struct sta_table *);
static int match_bss(struct ieee80211vap *, const struct ieee80211_scan_state *, const struct sta_entry *);
static void action_tasklet(IEEE80211_TQUEUE_ARG);

/*
 * Attach prior to any scanning work.
 */
static int sta_attach(struct ieee80211_scan_state *ss)
{
	struct sta_table *st;

	_MOD_INC_USE(THIS_MODULE, return 0);

	MALLOC(st, struct sta_table *, sizeof(struct sta_table), M_80211_SCAN, M_NOWAIT | M_ZERO);
	if (st == NULL)
		return 0;
	SCAN_STA_LOCK_INIT(st, "scan_sta");
	SCAN_STA_GEN_LOCK_INIT(st, "scan_sta_gen");
	TAILQ_INIT(&st->st_entry);
	IEEE80211_INIT_TQUEUE(&st->st_actiontq, action_tasklet, ss);
	ss->ss_priv = st;
	return 1;
}

/*
 * Cleanup any private state.
 */
static int sta_detach(struct ieee80211_scan_state *ss)
{
	struct sta_table *st = ss->ss_priv;

	if (st != NULL) {
		IEEE80211_CANCEL_TQUEUE(&st->st_actiontq);
		sta_flush_table(st);
		FREE(st, M_80211_SCAN);
	}

	_MOD_DEC_USE(THIS_MODULE);
	return 1;
}

/*
 * Flush all per-scan state.
 */
static int sta_flush(struct ieee80211_scan_state *ss)
{
	struct sta_table *st = ss->ss_priv;

	SCAN_STA_LOCK_IRQ(st);
	sta_flush_table(st);
	SCAN_STA_UNLOCK_IRQ(st);
	ss->ss_last = 0;
	return 0;
}

/*
 * Flush all entries in the scan cache.
 */
static void sta_flush_table(struct sta_table *st)
{
	struct sta_entry *se, *next;

	TAILQ_FOREACH_SAFE(se, &st->st_entry, se_list, next) {
		TAILQ_REMOVE(&st->st_entry, se, se_list);
		LIST_REMOVE(se, se_hash);
		FREE(se, M_80211_SCAN);
	}
}

static void saveie(u_int8_t **iep, const u_int8_t *ie, int preserve)
{
	if (preserve && *iep)
		return;
	if (ie == NULL)
		*iep = NULL;
	else
		ieee80211_saveie(iep, ie);
}

static inline int is_empty_ssid(u_int8_t *ssid)
{
	if (!ssid)
		return 1;
	if (ssid[1] == 0)
		return 1;
	if ((ssid[1] == 1) && (ssid[2] == 0))
		return 1;
	return 0;
}

/*
 * Process a beacon or probe response frame; create an
 * entry in the scan cache or update any previous entry.
 */
static int sta_add(struct ieee80211_scan_state *ss, const struct ieee80211_scanparams *sp, const struct ieee80211_frame *wh, int subtype, int rssi, u_int64_t rtsf)
{
#define	ISPROBE(_st)	((_st) == IEEE80211_FC0_SUBTYPE_PROBE_RESP)
#define	PICK1ST(_ss) \
	((ss->ss_flags & (IEEE80211_SCAN_PICK1ST | IEEE80211_SCAN_GOTPICK)) == \
	IEEE80211_SCAN_PICK1ST)
	struct sta_table *st = ss->ss_priv;
	const u_int8_t *macaddr = wh->i_addr2;
	struct ieee80211vap *vap = ss->ss_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct sta_entry *se;
	struct ieee80211_scan_entry *ise;
	int hash;

	/* workaround for broken APs that broadcast NULL BSSIDs */
	if (memcmp(wh->i_addr3, "\x00\x00\x00\x00\x00\x00", 6) == 0)
		return 0;

	hash = STA_HASH(macaddr);
	SCAN_STA_LOCK_IRQ(st);
	LIST_FOREACH(se, &st->st_hash[hash], se_hash)
	    if (IEEE80211_ADDR_EQ(se->base.se_macaddr, macaddr) && (is_empty_ssid(sp->ssid) || (sp->ssid[1] == se->base.se_ssid[1] && !memcmp(se->base.se_ssid + 2, sp->ssid + 2, se->base.se_ssid[1]))))
		goto found;

	MALLOC(se, struct sta_entry *, sizeof(struct sta_entry), M_80211_SCAN, M_NOWAIT | M_ZERO);
	if (se == NULL) {
		SCAN_STA_UNLOCK_IRQ_EARLY(st);
		return 0;
	}
	se->se_scangen = st->st_scangen - 1;
	IEEE80211_ADDR_COPY(se->base.se_macaddr, macaddr);
	TAILQ_INSERT_TAIL(&st->st_entry, se, se_list);
	LIST_INSERT_HEAD(&st->st_hash[hash], se, se_hash);

found:
	ise = &se->base;

	/* XXX ap beaconing multiple ssid w/ same bssid */
	if (!is_empty_ssid(sp->ssid) && (ISPROBE(subtype) || is_empty_ssid(ise->se_ssid)))
		memcpy(ise->se_ssid, sp->ssid, 2 + sp->ssid[1]);

	memcpy(ise->se_rates, sp->rates, 2 + IEEE80211_SANITISE_RATESIZE(sp->rates[1]));
	if (sp->xrates != NULL) {
		memcpy(ise->se_xrates, sp->xrates, 2 + IEEE80211_SANITISE_RATESIZE(sp->xrates[1]));
	} else
		ise->se_xrates[1] = 0;

	IEEE80211_ADDR_COPY(ise->se_bssid, wh->i_addr3);
	/*
	 * Record rssi data using extended precision LPF filter.
	 */
	if (se->se_lastupdate == 0)	/* first sample */
		se->se_avgrssi = RSSI_IN(rssi);
	else			/* avg w/ previous samples */
		RSSI_LPF(se->se_avgrssi, rssi);
	se->base.se_rssi = RSSI_GET(se->se_avgrssi);
	ise->se_rtsf = rtsf;
	memcpy(ise->se_tstamp.data, sp->tstamp, sizeof(ise->se_tstamp));
	ise->se_intval = sp->bintval;
	ise->se_capinfo = sp->capinfo;
	ise->se_chan = ic->ic_curchan;
	ise->se_fhdwell = sp->fhdwell;
	ise->se_fhindex = sp->fhindex;
	ise->se_erp = sp->erp;
	ise->se_timoff = sp->timoff;
	if (sp->tim != NULL) {
		const struct ieee80211_tim_ie *tim = (const struct ieee80211_tim_ie *)sp->tim;
		ise->se_dtimperiod = tim->tim_period;
	}
	saveie(&ise->se_wme_ie, sp->wme, 0);
	saveie(&ise->se_wpa_ie, sp->wpa, !sp->isprobe);
	saveie(&ise->se_rsn_ie, sp->rsn, !sp->isprobe);
	saveie(&ise->se_ath_ie, sp->ath, 0);
	saveie(&ise->se_mtik_ie, sp->mtik, 0);

	/* clear failure count after STA_FAIL_AGE passes */
	if (se->se_fails && (jiffies - se->se_lastfail) > STA_FAILS_AGE * HZ) {
		se->se_fails = 0;
		IEEE80211_NOTE_MAC(vap, IEEE80211_MSG_SCAN, macaddr, "%s: fails %u", __func__, se->se_fails);
	}

	se->se_lastupdate = jiffies;	/* update time */
	se->se_seen = 1;
	se->se_notseen = 0;

	SCAN_STA_UNLOCK_IRQ(st);

	/*
	 * If looking for a quick choice and nothing's
	 * been found check here.
	 */
	if (PICK1ST(ss) && match_bss(vap, ss, se) == 0)
		ss->ss_flags |= IEEE80211_SCAN_GOTPICK;

	return 1;
#undef PICK1ST
#undef ISPROBE
}

/*
 * Start a station-mode scan by populating the channel list.
 */
static int sta_start(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct sta_table *st = ss->ss_priv;
	enum ieee80211_phymode mode;
	struct ieee80211_channel *c;
	int i;

	ss->ss_last = 0;
	ieee80211_scan_add_channels(ic, ss, vap->iv_des_mode);
	ss->ss_next = 0;

	/* XXX tunables */
	/* 
	 * The scanner will stay on station for ss_maxdwell ms (using a 
	 * timer), collecting responses.  ss_maxdwell can adjusted downward
	 * so the station gets back on channel before DTIM occurs.  If the
	 * station receives probe responses before ss_mindwell has elapsed, the
	 * timer continues.  If it receives probe responses after ss_mindwell
	 * then the timer is cancelled and the next channel is chosen.  
	 * Basically, you are going to get the mindwell if you are scanning an
	 * occupied channel in the real world and the maxdwell if it's empty.
	 * 
	 * This seems somehow wrong to me, as you tend to want to fish where the
	 * fish is bitin'.  
	 * 
	 * I'm bumping mindwell up to 60ms (was 20ms).  This gives us a reasonable
	 * chance to find all the APs with active scans, and should pick up 
	 * everything within a few passes for passive.
	 */
	ss->ss_mindwell = msecs_to_jiffies(60);	/* 60ms */
	ss->ss_maxdwell = msecs_to_jiffies(200);	/* 200ms */

#ifdef IEEE80211_DEBUG
	if (ieee80211_msg_scan(vap)) {
		printk("%s: scan set ", vap->iv_dev->name);
		ieee80211_scan_dump_channels(ss);
		printk(" dwell min %ld max %ld\n", ss->ss_mindwell, ss->ss_maxdwell);
	}
#endif				/* IEEE80211_DEBUG */

	st->st_newscan = 1;

	return 0;
}

/*
 * Restart a bg scan.
 */
static int sta_restart(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct sta_table *st = ss->ss_priv;

	st->st_newscan = 1;
	return 0;
}

/*
 * Cancel an ongoing scan.
 */
static int sta_cancel(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct sta_table *st = ss->ss_priv;

	IEEE80211_CANCEL_TQUEUE(&st->st_actiontq);
	return 0;
}

static u_int8_t maxrate(const struct ieee80211_scan_entry *se)
{
	u_int8_t max, r;
	int i;

	max = 0;
	for (i = 0; i < se->se_rates[1]; i++) {
		r = se->se_rates[2 + i] & IEEE80211_RATE_VAL;
		if (r > max)
			max = r;
	}
	for (i = 0; i < se->se_xrates[1]; i++) {
		r = se->se_xrates[2 + i] & IEEE80211_RATE_VAL;
		if (r > max)
			max = r;
	}
	return max;
}

/*
 * Compare the capabilities of two entries and decide which is
 * more desirable (return >0 if a is considered better).  Note
 * that we assume compatibility/usability has already been checked
 * so we don't need to (e.g. validate whether privacy is supported).
 * Used to select the best scan candidate for association in a BSS.
 */
static int sta_compare(const struct sta_entry *a, const struct sta_entry *b)
{
	u_int8_t maxa, maxb;
	int weight;

	/* privacy support preferred */
	if ((a->base.se_capinfo & IEEE80211_CAPINFO_PRIVACY) && (b->base.se_capinfo & IEEE80211_CAPINFO_PRIVACY) == 0)
		return 1;
	if ((a->base.se_capinfo & IEEE80211_CAPINFO_PRIVACY) == 0 && (b->base.se_capinfo & IEEE80211_CAPINFO_PRIVACY))
		return -1;

	/* compare count of previous failures */
	weight = b->se_fails - a->se_fails;
	if (abs(weight) > 1)
		return weight;

	if (abs(b->base.se_rssi - a->base.se_rssi) < 5) {
		/* best/max rate preferred if signal level close enough XXX */
		maxa = maxrate(&a->base);
		maxb = maxrate(&b->base);
		if (maxa != maxb)
			return maxa - maxb;
		/* XXX use freq for channel preference */
		/* for now just prefer 5Ghz band to all other bands */
		if (IEEE80211_IS_CHAN_5GHZ(a->base.se_chan) && !IEEE80211_IS_CHAN_5GHZ(b->base.se_chan))
			return 1;
		if (!IEEE80211_IS_CHAN_5GHZ(a->base.se_chan) && IEEE80211_IS_CHAN_5GHZ(b->base.se_chan))
			return -1;
	}
	/* all things being equal, use signal level */
	return a->base.se_rssi - b->base.se_rssi;
}

/*
 * Check rate set suitability and return the best supported rate.
 */
static int check_rate(struct ieee80211vap *vap, const struct ieee80211_scan_entry *se)
{
#define	RV(v)	((v) & IEEE80211_RATE_VAL)
	struct ieee80211com *ic = vap->iv_ic;
	const struct ieee80211_rateset *srs;
	int i, j, nrs, r, okrate, badrate, fixedrate;
	const u_int8_t *rs;

	okrate = badrate = fixedrate = 0;

	srs = &ic->ic_sup_rates[ieee80211_chan2ratemode(ic->ic_curchan, -1)];
	nrs = se->se_rates[1];
	rs = se->se_rates + 2;
	fixedrate = IEEE80211_FIXED_RATE_NONE;
again:
	for (i = 0; i < nrs; i++) {
		r = RV(rs[i]);
		badrate = r;
		/*
		 * Check any fixed rate is included. 
		 */
		if (r == vap->iv_fixed_rate)
			fixedrate = r;
		/*
		 * Check against our supported rates.
		 */
		for (j = 0; j < srs->rs_nrates; j++)
			if (r == RV(srs->rs_rates[j])) {
				if (r > okrate)	/* NB: track max */
					okrate = r;
				break;
			}
	}
	if (rs == se->se_rates + 2) {
		/* scan xrates too; sort of an algol68-style for loop */
		nrs = se->se_xrates[1];
		rs = se->se_xrates + 2;
		goto again;
	}
	if (okrate == 0 || vap->iv_fixed_rate != fixedrate)
		return badrate | IEEE80211_RATE_BASIC;
	else
		return RV(okrate);
#undef RV
}

static int match_ssid(const u_int8_t *ie, int nssid, const struct ieee80211_scan_ssid ssids[])
{
	int i;

	for (i = 0; i < nssid; i++) {
		if (ie[1] == ssids[i].len && memcmp(ie + 2, ssids[i].ssid, ie[1]) == 0)
			return 1;
	}
	return 0;
}

/*
 * Test a scan candidate for suitability/compatibility.
 */
static int match_bss(struct ieee80211vap *vap, const struct ieee80211_scan_state *ss, const struct sta_entry *se0)
{
	struct ieee80211com *ic = vap->iv_ic;
	const struct ieee80211_scan_entry *se = &se0->base;
	u_int8_t rate;
	int fail;

	fail = 0;
	if (isclr(ic->ic_chan_active, ieee80211_chan2ieee(ic, se->se_chan)))
		fail |= 0x01;

	if (vap->iv_opmode == IEEE80211_M_IBSS) {
		if ((se->se_capinfo & IEEE80211_CAPINFO_IBSS) == 0)
			fail |= 0x02;
	} else {
		if ((se->se_capinfo & IEEE80211_CAPINFO_ESS) == 0)
			fail |= 0x02;
	}
	if (vap->iv_flags & IEEE80211_F_PRIVACY) {
		if ((se->se_capinfo & IEEE80211_CAPINFO_PRIVACY) == 0)
			fail |= 0x04;
	} else {
		/* XXX does this mean privacy is supported or required? */
		if (se->se_capinfo & IEEE80211_CAPINFO_PRIVACY)
			fail |= 0x04;
	}
	rate = check_rate(vap, se);
	if (rate & IEEE80211_RATE_BASIC)
		fail |= 0x08;
	if (ss->ss_nssid != 0 && !match_ssid(se->se_ssid, ss->ss_nssid, ss->ss_ssid))
		fail |= 0x10;
	if ((vap->iv_flags & IEEE80211_F_DESBSSID) && !IEEE80211_ADDR_EQ(vap->iv_des_bssid, se->se_bssid))
		fail |= 0x20;
	if (se0->se_fails >= STA_FAILS_MAX)
		fail |= 0x40;
	if (se0->se_notseen >= STA_PURGE_SCANS)
		fail |= 0x80;
#ifdef IEEE80211_DEBUG
	if (ieee80211_msg_is_reported(vap, IEEE80211_MSG_SCAN | IEEE80211_MSG_ROAM)) {
		printk(" %03x", fail);
		printk(" %c " MAC_FMT, fail & 0x40 ? '=' : fail & 0x80 ? '^' : fail ? '-' : '+', MAC_ADDR(se->se_macaddr));
		printk(" " MAC_FMT "%c", MAC_ADDR(se->se_bssid), fail & 0x20 ? '!' : ' ');
		printk(" %3d%c", ieee80211_chan2ieee(ic, se->se_chan), fail & 0x01 ? '!' : ' ');
		printk(" %+4d", se->se_rssi);
		printk(" %2dM%c", (rate & IEEE80211_RATE_VAL) / 2, fail & 0x08 ? '!' : ' ');
		printk(" %4s%c", (se->se_capinfo & IEEE80211_CAPINFO_ESS) ? "ess" : (se->se_capinfo & IEEE80211_CAPINFO_IBSS) ? "ibss" : "????", fail & 0x02 ? '!' : ' ');
		printk(" %3s%c ", (se->se_capinfo & IEEE80211_CAPINFO_PRIVACY) ? "wep" : "no", fail & 0x04 ? '!' : ' ');
		ieee80211_print_essid(se->se_ssid + 2, se->se_ssid[1]);
		printk("%s\n", fail & 0x10 ? "!" : "");
	}
#endif
	return fail;
}

static void sta_update_notseen(struct sta_table *st)
{
	struct sta_entry *se;

	SCAN_STA_LOCK_IRQ(st);
	TAILQ_FOREACH(se, &st->st_entry, se_list) {
		/*
		 * If seen then reset and don't bump the count;
		 * otherwise bump the ``not seen'' count.  Note
		 * that this ensures that stations for which we
		 * see frames while not scanning but not during
		 * this scan will not be penalized.
		 */
		if (se->se_seen)
			se->se_seen = 0;
		else
			se->se_notseen++;
	}
	SCAN_STA_UNLOCK_IRQ(st);
}

static void sta_dec_fails(struct sta_table *st)
{
	struct sta_entry *se;

	SCAN_STA_LOCK_IRQ(st);
	TAILQ_FOREACH(se, &st->st_entry, se_list)
	    if (se->se_fails)
		se->se_fails--;
	SCAN_STA_UNLOCK_IRQ(st);
}

static struct sta_entry *select_bss(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se, *selbs = NULL;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN | IEEE80211_MSG_ROAM, " %s\n", "macaddr          bssid         chan  rssi  rate flag  wep  essid");
	SCAN_STA_LOCK_IRQ(st);
	TAILQ_FOREACH(se, &st->st_entry, se_list) {
		if (match_bss(vap, ss, se) == 0) {
			if (selbs == NULL)
				selbs = se;
			else if (sta_compare(se, selbs) > 0)
				selbs = se;
		}
	}
	SCAN_STA_UNLOCK_IRQ(st);

	return selbs;
}

/*
 * Pick an ap or ibss network to join or find a channel
 * to use to start an ibss network.
 */
static int sta_pick_bss(struct ieee80211_scan_state *ss, struct ieee80211vap *vap, int (*action)(struct ieee80211vap *, const struct ieee80211_scan_entry *), u_int32_t flags)
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *selbss = NULL;

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN, "%s Checking scan results\n", __func__);

	KASSERT(vap->iv_opmode == IEEE80211_M_STA, ("wrong mode %u", vap->iv_opmode));

	if (st->st_newscan) {
		sta_update_notseen(st);
		st->st_newscan = 0;
	}
	if (ss->ss_flags & IEEE80211_SCAN_NOPICK) {
		/*
		 * Manual/background scan, don't select+join the
		 * bss, just return.  The scanning framework will
		 * handle notification that this has completed.
		 */
		ss->ss_flags &= ~IEEE80211_SCAN_NOPICK;
		return 1;
	}
	/*
	 * Automatic sequencing; look for a candidate and
	 * if found join the network.
	 */
	/* NB: unlocked read should be ok */
	if (TAILQ_FIRST(&st->st_entry) == NULL) {
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN, "%s: no scan candidate\n", __func__);
	      notfound:
		/*
		 * If nothing suitable was found decrement
		 * the failure counts so entries will be
		 * reconsidered the next time around.  We
		 * really want to do this only for STAs
		 * where we've previously had some success.
		 */
		sta_dec_fails(st);
		st->st_newscan = 1;
		return 0;	/* restart scan */
	}
	st->st_action = ss->ss_ops->scan_default;
	if (action)
		st->st_action = action;
	if ((selbss = select_bss(ss, vap)) == NULL) {
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN, "%s: select_bss failed\n", __func__);
		goto notfound;
	}
	st->st_selbss = selbss->base;

	/* 
	 * Must defer action to avoid possible recursive call through 80211
	 * state machine, which would result in recursive locking.
	 */
	IEEE80211_SCHEDULE_TQUEUE(&st->st_actiontq);

	return 1;		/* terminate scan */
}

/*
 * Lookup an entry in the scan cache.  We assume we're
 * called from the bottom half or such that we don't need
 * to block the bottom half so that it's safe to return
 * a reference to an entry w/o holding the lock on the table.
 */
static struct sta_entry *sta_lookup(struct sta_table *st, const u_int8_t macaddr[IEEE80211_ADDR_LEN], struct ieee80211_scan_ssid *essid)
{
	struct sta_entry *se;
	int hash = STA_HASH(macaddr);

	SCAN_STA_LOCK_IRQ(st);
	LIST_FOREACH(se, &st->st_hash[hash], se_hash)
	    if (IEEE80211_ADDR_EQ(se->base.se_macaddr, macaddr) && (essid->len == se->base.se_ssid[1] && !memcmp(se->base.se_ssid + 2, essid->ssid, se->base.se_ssid[1])))
		break;
	SCAN_STA_UNLOCK_IRQ(st);

	return se;		/* NB: unlocked */
}

static void sta_roam_check(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct ieee80211_node *ni = vap->iv_bss;
	struct ieee80211com *ic = vap->iv_ic;
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se, *selbs = NULL;
	u_int8_t roamRate, curRate;
	int8_t roamRssi, curRssi;

	se = sta_lookup(st, ni->ni_macaddr, ss->ss_ssid);
	if (se == NULL) {
		/* XXX something is wrong */
		return;
	}

	/* XXX do we need 11g too? */
	if (IEEE80211_IS_CHAN_ANYG(ic->ic_bsschan)) {
		roamRate = vap->iv_roam.rate11b;
		roamRssi = vap->iv_roam.rssi11b;
	} else if (IEEE80211_IS_CHAN_B(ic->ic_bsschan)) {
		roamRate = vap->iv_roam.rate11bOnly;
		roamRssi = vap->iv_roam.rssi11bOnly;
	} else {
		roamRate = vap->iv_roam.rate11a;
		roamRssi = vap->iv_roam.rssi11a;
	}
	/* NB: the most up to date rssi is in the node, not the scan cache */
	curRssi = ic->ic_node_getrssi(ni);
	if (vap->iv_fixed_rate == IEEE80211_FIXED_RATE_NONE) {
		curRate = ni->ni_rates.rs_rates[ni->ni_txrate] & IEEE80211_RATE_VAL;
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_ROAM, "%s: currssi %d currate %u roamrssi %d roamrate %u\n", __func__, curRssi, curRate, roamRssi, roamRate);
	} else {
		curRate = roamRate;	/* NB: ensure compare below fails */
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_ROAM, "%s: currssi %d roamrssi %d\n", __func__, curRssi, roamRssi);
	}
	if ((vap->iv_flags & IEEE80211_F_BGSCAN) && time_after(jiffies, ic->ic_lastscan + vap->iv_scanvalid)) {
		/*
		 * Scan cache contents is too old; check about updating it.
		 */
		if (curRate < roamRate || curRssi < roamRssi) {
			/*
			 * Thresholds exceeded, force a scan now so we
			 * have current state to make a decision with.
			 */
			ieee80211_bg_scan(vap);
		} else if (time_after(jiffies, ic->ic_lastdata + vap->iv_bgscanidle)) {
			/*
			 * We're not in need of a new ap, but idle;
			 * kick off a bg scan to replenish the cache.
			 */
			ieee80211_bg_scan(vap);
		}
	} else {
		/*
		 * Scan cache contents are warm enough to use;
		 * check if a new ap should be used and switch.
		 * XXX deauth current ap
		 */
		if (curRate < roamRate || curRssi < roamRssi) {
			se->base.se_rssi = curRssi;
			selbs = select_bss(ss, vap);
			if (selbs != NULL && selbs != se)
				ieee80211_sta_join(vap, &selbs->base);
		}
	}
}

/*
 * Age entries in the scan cache.
 * XXX also do roaming since it's convenient
 */
static void sta_age(struct ieee80211_scan_state *ss)
{
	struct ieee80211vap *vap = ss->ss_vap;
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se, *next;

	SCAN_STA_LOCK_IRQ(st);
	TAILQ_FOREACH_SAFE(se, &st->st_entry, se_list, next) {
		if (se->se_notseen > STA_PURGE_SCANS) {
			TAILQ_REMOVE(&st->st_entry, se, se_list);
			LIST_REMOVE(se, se_hash);
			FREE(se, M_80211_SCAN);
		}
	}
	SCAN_STA_UNLOCK_IRQ(st);
	/*
	 * If rate control is enabled check periodically to see if
	 * we should roam from our current connection to one that
	 * might be better.  This only applies when we're operating
	 * in sta mode and automatic roaming is set.
	 * XXX defer if busy
	 * XXX repeater station
	 */
	KASSERT(vap->iv_opmode == IEEE80211_M_STA, ("wrong mode %u", vap->iv_opmode));
	if (vap->iv_opmode == IEEE80211_M_STA && vap->iv_ic->ic_roaming == IEEE80211_ROAMING_AUTO && vap->iv_state >= IEEE80211_S_RUN)
		/* XXX vap is implicit */
		sta_roam_check(ss, vap);
}

/*
 * Iterate over the entries in the scan cache, invoking
 * the callback function on each one.
 */
static int sta_iterate(struct ieee80211_scan_state *ss, ieee80211_scan_iter_func * f, void *arg)
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se;
	u_int gen;
	int res = 0;

	SCAN_STA_GEN_LOCK(st);
	gen = st->st_scangen++;
restart:
	SCAN_STA_LOCK_IRQ(st);
	TAILQ_FOREACH(se, &st->st_entry, se_list) {
		if (se->se_scangen != gen) {
			se->se_scangen = gen;
			/* update public state */
			se->base.se_age = jiffies - se->se_lastupdate;
			SCAN_STA_UNLOCK_IRQ_EARLY(st);

			res = (*f) (arg, &se->base);

			if (res != 0)
				/* We probably ran out of buffer space. */
				goto done;

			goto restart;
		}
	}

	SCAN_STA_UNLOCK_IRQ(st);

done:
	SCAN_STA_GEN_UNLOCK(st);

	return res;
}

static void sta_assoc_fail(struct ieee80211_scan_state *ss, const u_int8_t macaddr[IEEE80211_ADDR_LEN], int reason)
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se;

	/* Let outside apps to decide what peer is blacklisted */
	if (ss->ss_vap->iv_ic->ic_roaming == IEEE80211_ROAMING_MANUAL)
		return;

	se = sta_lookup(st, macaddr, ss->ss_ssid);
	if (se != NULL) {
		se->se_fails++;
		se->se_lastfail = jiffies;
		IEEE80211_NOTE_MAC(ss->ss_vap, IEEE80211_MSG_SCAN, macaddr, "%s: reason %u fails %u", __func__, reason, se->se_fails);
	}
}

static void sta_assoc_success(struct ieee80211_scan_state *ss, const u_int8_t macaddr[IEEE80211_ADDR_LEN])
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se;

	se = sta_lookup(st, macaddr, ss->ss_ssid);
	if (se != NULL) {
#if 0
		se->se_fails = 0;
		IEEE80211_NOTE_MAC(ss->ss_vap, IEEE80211_MSG_SCAN, macaddr, "%s: fails %u", __func__, se->se_fails);
#endif
		se->se_lastassoc = jiffies;
	}
}

static const struct ieee80211_scanner sta_default = {
	.scan_name = "default",
	.scan_attach = sta_attach,
	.scan_detach = sta_detach,
	.scan_start = sta_start,
	.scan_restart = sta_restart,
	.scan_cancel = sta_cancel,
	.scan_end = sta_pick_bss,
	.scan_flush = sta_flush,
	.scan_add = sta_add,
	.scan_age = sta_age,
	.scan_iterate = sta_iterate,
	.scan_assoc_fail = sta_assoc_fail,
	.scan_assoc_success = sta_assoc_success,
	.scan_default = ieee80211_sta_join,
};

/*
 * Select a channel to start an adhoc network on.
 * The channel list was populated with appropriate
 * channels so select one that looks least occupied.
 * XXX need regulatory domain constraints
 */
static struct ieee80211_channel *adhoc_pick_channel(struct ieee80211_scan_state *ss)
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se;
	struct ieee80211_channel *c, *bestchan;
	int i, bestrssi, maxrssi;

	bestchan = NULL;
	bestrssi = -1;

	SCAN_STA_LOCK_IRQ(st);
	for (i = 0; i < ss->ss_last; i++) {
		c = ss->ss_chans[i];
		maxrssi = 0;
		TAILQ_FOREACH(se, &st->st_entry, se_list) {
			if (se->base.se_chan != c)
				continue;
			if (se->base.se_rssi > maxrssi)
				maxrssi = se->base.se_rssi;
		}
		if (bestchan == NULL || maxrssi < bestrssi)
			bestchan = c;
	}
	SCAN_STA_UNLOCK_IRQ(st);

	return bestchan;
}

/*
 * Pick an ibss network to join or find a channel
 * to use to start an ibss network.
 */
static int adhoc_pick_bss(struct ieee80211_scan_state *ss, struct ieee80211vap *vap, int (*action)(struct ieee80211vap *, const struct ieee80211_scan_entry *), u_int32_t flags)
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *selbs = NULL;
	struct ieee80211_channel *chan;

	KASSERT(vap->iv_opmode == IEEE80211_M_IBSS || vap->iv_opmode == IEEE80211_M_AHDEMO, ("wrong opmode %u", vap->iv_opmode));

	if (st->st_newscan) {
		sta_update_notseen(st);
		st->st_newscan = 0;
	}
	if (ss->ss_flags & IEEE80211_SCAN_NOPICK) {
		/*
		 * Manual/background scan, don't select+join the
		 * bss, just return.  The scanning framework will
		 * handle notification that this has completed.
		 */
		ss->ss_flags &= ~IEEE80211_SCAN_NOPICK;
		return 1;
	}

	st->st_action = ss->ss_ops->scan_default;
	if (action)
		st->st_action = action;

	/*
	 * Automatic sequencing; look for a candidate and
	 * if found join the network.
	 */
	/* NB: unlocked read should be ok */
	if (TAILQ_FIRST(&st->st_entry) == NULL || (selbs = select_bss(ss, vap)) == NULL) {
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN, "%s: no scan candidate\n", __func__);
		if (vap->iv_des_nssid) {
			/*
			 * No existing adhoc network to join and we have
			 * an ssid; start one up.  If no channel was
			 * specified, try to select a channel.
			 */
			if (vap->iv_des_chan == IEEE80211_CHAN_ANYC)
				chan = adhoc_pick_channel(ss);
			else
				chan = vap->iv_des_chan;
			if (chan != NULL) {
				struct ieee80211_scan_entry se;

				memset(&se, 0, sizeof(se));
				se.se_chan = chan;
				st->st_selbss = se;
				/* defer action */
				IEEE80211_SCHEDULE_TQUEUE(&st->st_actiontq);
				return 1;
			}
		}
		/*
		 * If nothing suitable was found decrement
		 * the failure counts so entries will be
		 * reconsidered the next time around.  We
		 * really want to do this only for STAs
		 * where we've previously had some success.
		 */
		sta_dec_fails(st);
		st->st_newscan = 1;
		return 0;	/* restart scan */
	}

	/* 
	 * Must defer action to avoid possible recursive call through 80211
	 * state machine, which would result in recursive locking.
	 */
	st->st_selbss = selbs->base;
	IEEE80211_SCHEDULE_TQUEUE(&st->st_actiontq);

	return 1;		/* terminate scan */
}

/*
 * Age entries in the scan cache.
 */
static void adhoc_age(struct ieee80211_scan_state *ss)
{
	struct sta_table *st = ss->ss_priv;
	struct sta_entry *se, *next;

	SCAN_STA_LOCK_IRQ(st);
	TAILQ_FOREACH_SAFE(se, &st->st_entry, se_list, next) {
		if (se->se_notseen > STA_PURGE_SCANS) {
			TAILQ_REMOVE(&st->st_entry, se, se_list);
			LIST_REMOVE(se, se_hash);
			FREE(se, M_80211_SCAN);
		}
	}
	SCAN_STA_UNLOCK_IRQ(st);
}

/*
 * Default action to execute when a scan entry is found for adhoc
 * mode.  Return 1 on success, 0 on failure
 */
static int adhoc_default_action(struct ieee80211vap *vap, const struct ieee80211_scan_entry *se)
{
	u_int8_t zeroMacAddr[IEEE80211_ADDR_LEN];

	memset(&zeroMacAddr, 0, IEEE80211_ADDR_LEN);
	ieee80211_create_ibss(vap, se->se_chan);
	return 1;
}

static const struct ieee80211_scanner adhoc_default = {
	.scan_name = "default",
	.scan_attach = sta_attach,
	.scan_detach = sta_detach,
	.scan_start = sta_start,
	.scan_restart = sta_restart,
	.scan_cancel = sta_cancel,
	.scan_end = adhoc_pick_bss,
	.scan_flush = sta_flush,
	.scan_add = sta_add,
	.scan_age = adhoc_age,
	.scan_iterate = sta_iterate,
	.scan_assoc_fail = sta_assoc_fail,
	.scan_assoc_success = sta_assoc_success,
	.scan_default = adhoc_default_action,
};

static void action_tasklet(IEEE80211_TQUEUE_ARG data)
{
	struct ieee80211_scan_state *ss = (struct ieee80211_scan_state *)data;
	struct sta_table *st = (struct sta_table *)ss->ss_priv;
	struct ieee80211vap *vap = ss->ss_vap;
	struct ieee80211_channel *chan;

	if ((*ss->ss_ops->scan_default) (vap, &st->st_selbss))
		return;

	switch (vap->iv_opmode) {
	case IEEE80211_M_STA:
		sta_dec_fails(st);
		st->st_newscan = 1;
		break;
	default:
		/* ADHOC */
		if (vap->iv_des_nssid) {
			/*
			 * No existing adhoc network to join and we have
			 * an ssid; start one up.  If no channel was
			 * specified, try to select a channel.
			 */
			if (vap->iv_des_chan == IEEE80211_CHAN_ANYC)
				chan = adhoc_pick_channel(ss);
			else
				chan = vap->iv_des_chan;
			if (chan != NULL) {
				struct ieee80211_scan_entry se;

				memset(&se, 0, sizeof(se));
				se.se_chan = chan;
				if ((*ss->ss_ops->scan_default) (vap, &se))
					return;
			}
		}
		/*
		 * If nothing suitable was found decrement
		 * the failure counts so entries will be
		 * reconsidered the next time around.  We
		 * really want to do this only for STAs
		 * where we've previously had some success.
		 */
		sta_dec_fails(st);
		st->st_newscan = 1;
		break;
	}

	/* 
	 * restart scan 
	 */

	/* no ap, clear the flag for a new scan */
	vap->iv_ic->ic_flags &= ~IEEE80211_F_SCAN;
	if ((ss->ss_flags & IEEE80211_SCAN_USECACHE) == 0)
		ieee80211_start_scan(vap, ss->ss_flags, ss->ss_duration, ss->ss_nssid, ss->ss_ssid);
}

#include "module.h"

/*
 * Module glue.
 */
MODULE_AUTHOR("Errno Consulting, Sam Leffler");
MODULE_DESCRIPTION("802.11 wireless support: default station scanner");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static int __init init_scanner_sta(void)
{
	ieee80211_scanner_register(IEEE80211_M_STA, &sta_default);
	ieee80211_scanner_register(IEEE80211_M_IBSS, &adhoc_default);
	ieee80211_scanner_register(IEEE80211_M_AHDEMO, &adhoc_default);
	return 0;
}

module_init(init_scanner_sta);

static void __exit exit_scanner_sta(void)
{
	ieee80211_scanner_unregister_all(&sta_default);
	ieee80211_scanner_unregister_all(&adhoc_default);
}

module_exit(exit_scanner_sta);
