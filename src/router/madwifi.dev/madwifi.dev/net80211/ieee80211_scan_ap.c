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
 * $Id: ieee80211_scan_ap.c 3310 2008-01-30 20:23:49Z mentor $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 ap scanning support.
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
#include "sort.c"
#else
#include <linux/sort.h>
#endif

#include "if_media.h"

#include <net80211/ieee80211_var.h>

#define	AP_PURGE_SCANS	2	/* age for purging entries (scans) */
#define RSSI_LPF_LEN	10
#define	RSSI_EP_MULTIPLIER	(1 << 7)	/* pow2 to optimize out * and / */
#define RSSI_IN(x)		((x) * RSSI_EP_MULTIPLIER)
#define LPF_RSSI(x, y, len)	(((x) * ((len) - 1) + (y)) / (len))
#define RSSI_LPF(x, y) do {						\
	if ((y) >= -20)							\
		x = LPF_RSSI((x), RSSI_IN((y)), RSSI_LPF_LEN);		\
} while (0)
#define	EP_RND(x, mul) \
	((((x)%(mul)) >= ((mul)/2)) ? howmany(x, mul) : (x)/(mul))
#define	RSSI_GET(x)	EP_RND(x, RSSI_EP_MULTIPLIER)
#define	AP_HASHSIZE	32
/* simple hash is enough for variation of macaddr */
#define	AP_HASH(addr)	\
	(((const u_int8_t *)(addr))[IEEE80211_ADDR_LEN - 1] % AP_HASHSIZE)
#define	SCAN_AP_LOCK_INIT(_st, _name)					\
	spin_lock_init(&(_st)->as_lock)
#define	SCAN_AP_LOCK_DESTROY(_st)
#define	SCAN_AP_LOCK_IRQ(_st) do {					\
	unsigned long __stlockflags;					\
	spin_lock_irqsave(&(_st)->as_lock, __stlockflags);
#define	SCAN_AP_UNLOCK_IRQ(_st)						\
	spin_unlock_irqrestore(&(_st)->as_lock, __stlockflags);		\
} while (0)
#define	SCAN_AP_UNLOCK_IRQ_EARLY(_st)					\
	spin_unlock_irqrestore(&(_st)->as_lock, __stlockflags);

#define	SCAN_AP_GEN_LOCK_INIT(_st, _name)				\
	spin_lock_init(&(_st)->as_scanlock)
#define	SCAN_AP_GEN_LOCK_DESTROY(_st)
#define	SCAN_AP_GEN_LOCK(_st)		spin_lock(&(_st)->as_scanlock);
#define	SCAN_AP_GEN_UNLOCK(_st)	spin_unlock(&(_st)->as_scanlock);

struct scan_entry {
	struct ieee80211_scan_entry base;
	 TAILQ_ENTRY(scan_entry) se_list;
	 LIST_ENTRY(scan_entry) se_hash;
	u_int8_t se_seen;	/* seen during current scan */
	u_int8_t se_notseen;	/* not seen in previous scans */
	u_int32_t se_avgrssi;	/* LPF rssi state */
	unsigned long se_lastupdate;	/* time of last update */
	unsigned long se_lastfail;	/* time of last failure */
	unsigned long se_lastassoc;	/* time of last association */
	u_int se_scangen;	/* iterator scan gen# */
};

struct ap_state {
	int as_maxrssi[IEEE80211_CHAN_MAX];	/* Used for channel selection */

	/* These fields are just for scan caching for returning responses to
	 * wireless extensions.  i.e. show peers, APs, etc. */
	spinlock_t as_lock;	/* on scan table */
	int as_newscan;		/* trigger for updating 
				 * seen/not-seen for aging */
	 TAILQ_HEAD(, scan_entry) as_entry;	/* all entries */
	 ATH_LIST_HEAD(, scan_entry) as_hash[AP_HASHSIZE];
	spinlock_t as_scanlock;	/* on as_scangen */
	u_int as_scangen;	/* gen# for iterator */

	struct IEEE80211_TQ_STRUCT as_actiontq;	/* tasklet for "action" */
	struct ieee80211_scan_entry as_selbss;	/* selected bss for action tasklet */
	int (*as_action)(struct ieee80211vap *, const struct ieee80211_scan_entry *);
};

static int ap_flush(struct ieee80211_scan_state *);
static void action_tasklet(IEEE80211_TQUEUE_ARG);

/*
 * Attach prior to any scanning work.
 */
static int ap_attach(struct ieee80211_scan_state *ss)
{
	struct ap_state *as;

	_MOD_INC_USE(THIS_MODULE, return 0);

	MALLOC(as, struct ap_state *, sizeof(struct ap_state), M_80211_SCAN, M_NOWAIT | M_ZERO);
	if (as == NULL)
		return 0;
	SCAN_AP_LOCK_INIT(as, "scan_ap");
	SCAN_AP_GEN_LOCK_INIT(as, "scan_ap_gen");
	TAILQ_INIT(&as->as_entry);
	IEEE80211_INIT_TQUEUE(&as->as_actiontq, action_tasklet, ss);
	ss->ss_priv = as;
	ap_flush(ss);
	return 1;
}

/*
 * Cleanup any private state.
 */
static int ap_detach(struct ieee80211_scan_state *ss)
{
	struct ap_state *as = ss->ss_priv;

	if (as != NULL) {
		ap_flush(ss);
		IEEE80211_CANCEL_TQUEUE(&as->as_actiontq);
		FREE(as, M_80211_SCAN);
	}

	_MOD_DEC_USE(THIS_MODULE);
	return 1;
}

/*
 * Flush all per-scan state.
 */
static int ap_flush(struct ieee80211_scan_state *ss)
{
	struct ap_state *as = ss->ss_priv;
	struct scan_entry *se, *next;

	SCAN_AP_LOCK_IRQ(as);
	memset(as->as_maxrssi, 0, sizeof(as->as_maxrssi));
	TAILQ_FOREACH_SAFE(se, &as->as_entry, se_list, next) {
		TAILQ_REMOVE(&as->as_entry, se, se_list);
		LIST_REMOVE(se, se_hash);
		FREE(se, M_80211_SCAN);
	}
	ss->ss_last = 0;	/* ensure no channel will be picked */
	SCAN_AP_UNLOCK_IRQ(as);
	return 0;
}

/* This function must be invoked with locks acquired */
static void saveie(u_int8_t **iep, const u_int8_t *ie)
{
	if (ie == NULL)
		*iep = NULL;
	else
		ieee80211_saveie(iep, ie);
}

/*
 * Start an ap scan by populating the channel list.
 */
static int ap_start(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct ap_state *as = ss->ss_priv;
	struct ieee80211com *ic = NULL;
	int i;
	unsigned int mode = 0;
	unsigned long sflags;

	SCAN_AP_LOCK_IRQ(as);
	ic = vap->iv_ic;

	spin_lock_irqsave(&channel_lock, sflags);
	ieee80211_scan_set_bss_channel(ic, NULL);
	spin_unlock_irqrestore(&channel_lock, sflags);

	/* Determine mode flags to match, or leave zero for auto mode */
	ss->ss_last = 0;
	ieee80211_scan_add_channels(ic, ss, vap->iv_des_mode);

	ss->ss_next = 0;
	/* XXX tunables */
	ss->ss_mindwell = msecs_to_jiffies(200);	/* 200ms */
	ss->ss_maxdwell = msecs_to_jiffies(300);	/* 300ms */

#ifdef IEEE80211_DEBUG
	if (ieee80211_msg_scan(vap)) {
		printk("%s: scan set ", vap->iv_dev->name);
		ieee80211_scan_dump_channels(ss);
		printk(" dwell min %ld max %ld\n", ss->ss_mindwell, ss->ss_maxdwell);
	}
#endif				/* IEEE80211_DEBUG */

	as->as_newscan = 1;
	SCAN_AP_UNLOCK_IRQ(as);
	return 0;
}

/*
 * Restart a bg scan.
 */
static int ap_restart(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct ap_state *as = ss->ss_priv;
	as->as_newscan = 1;
	return 0;
}

/*
 * Cancel an ongoing scan.
 */
static int ap_cancel(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct ap_state *as = ss->ss_priv;
	IEEE80211_CANCEL_TQUEUE(&as->as_actiontq);
	return 0;
}

/*
 * Record max rssi on channel.
 */
static int ap_add(struct ieee80211_scan_state *ss, const struct ieee80211_scanparams *sp, const struct ieee80211_frame *wh, int subtype, int rssi, u_int64_t rtsf)
{
	struct ap_state *as = ss->ss_priv;
	const u_int8_t *macaddr = wh->i_addr2;
	struct ieee80211vap *vap = ss->ss_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct scan_entry *se = NULL;
	struct ieee80211_scan_entry *ise = NULL;
	int hash = AP_HASH(macaddr);
	int chan;

	/* This section provides scan results to wireless extensions */
	SCAN_AP_LOCK_IRQ(as);

	chan = ieee80211_chan2ieee(ic, ic->ic_curchan);
	/* This is the only information used for channel selection by AP */
	if (rssi > as->as_maxrssi[chan])
		as->as_maxrssi[chan] = rssi;
	LIST_FOREACH(se, &as->as_hash[hash], se_hash)
	    if (IEEE80211_ADDR_EQ(se->base.se_macaddr, macaddr) && (sp->ssid[1] == se->base.se_ssid[1]) && !memcmp(se->base.se_ssid + 2, sp->ssid + 2, se->base.se_ssid[1]))
		goto found;

	MALLOC(se, struct scan_entry *, sizeof(struct scan_entry), M_80211_SCAN, M_NOWAIT | M_ZERO);

	if (se == NULL) {
		SCAN_AP_UNLOCK_IRQ_EARLY(as);
		return 0;
	}

	se->se_scangen = as->as_scangen - 1;
	IEEE80211_ADDR_COPY(se->base.se_macaddr, macaddr);
	TAILQ_INSERT_TAIL(&as->as_entry, se, se_list);
	LIST_INSERT_HEAD(&as->as_hash[hash], se, se_hash);

found:
	ise = &se->base;

	/* XXX: AP beaconing multiple SSID w/ same BSSID */
	if ((sp->ssid[1] != 0) && ((subtype == IEEE80211_FC0_SUBTYPE_PROBE_RESP) || (ise->se_ssid[1] == 0)))
		memcpy(ise->se_ssid, sp->ssid, 2 + sp->ssid[1]);

	memcpy(ise->se_rates, sp->rates, IEEE80211_SANITISE_RATESIZE(2 + sp->rates[1]));
	if (sp->xrates != NULL) {
		memcpy(ise->se_xrates, sp->xrates, IEEE80211_SANITISE_RATESIZE(2 + sp->xrates[1]));
	} else
		ise->se_xrates[1] = 0;

	IEEE80211_ADDR_COPY(ise->se_bssid, wh->i_addr3);

	/* Record RSSI data using extended precision LPF filter. */
	if (se->se_lastupdate == 0)	/* First sample */
		se->se_avgrssi = RSSI_IN(rssi);
	else			/* Avg. w/ previous samples */
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

	saveie(&ise->se_wme_ie, sp->wme);
	saveie(&ise->se_wpa_ie, sp->wpa);
	saveie(&ise->se_rsn_ie, sp->rsn);
	saveie(&ise->se_ath_ie, sp->ath);

	se->se_lastupdate = jiffies;	/* update time */
	se->se_seen = 1;
	se->se_notseen = 0;

	SCAN_AP_UNLOCK_IRQ(as);

	return 1;
}

struct pc_params {
	struct ieee80211vap *vap;
	struct ieee80211_scan_state *ss;
	int flags;
};

struct channel {
	struct ieee80211_channel *chan;
	int orig;
	struct pc_params *params;
};

/* This function must be invoked with locks acquired */
static int pc_cmp_radar(struct ieee80211_channel *a, struct ieee80211_channel *b)
{
	/* a is better than b (return < 0) when b is marked while a is not */
	return !!IEEE80211_IS_CHAN_RADAR(a) - !!IEEE80211_IS_CHAN_RADAR(b);
}

/* This function must be invoked with locks acquired */
static int pc_cmp_keepmode(struct pc_params *params, struct ieee80211_channel *a, struct ieee80211_channel *b)
{
	struct ieee80211com *ic = params->vap->iv_ic;
	struct ieee80211_channel *cur = ic->ic_bsschan;

	if (!(params->flags & IEEE80211_SCAN_KEEPMODE))
		return 0;

	/* a is better than b (return < 0) when (a, cur) have the same mode 
	 * and (b, cur) do not. */
	return !!IEEE80211_ARE_CHANS_SAME_MODE(b, cur) - !!IEEE80211_ARE_CHANS_SAME_MODE(a, cur);
}

/* This function must be invoked with locks acquired */
static int pc_cmp_sc(struct ieee80211com *ic, struct ieee80211_channel *a, struct ieee80211_channel *b)
{
	/* a is better than b (return < 0) when a has more chan nodes than b */
	return ic->ic_chan_nodes[b->ic_ieee] - ic->ic_chan_nodes[a->ic_ieee];
}

/* This function must be invoked with locks acquired */
static int pc_cmp_rssi(struct ap_state *as, struct ieee80211_channel *a, struct ieee80211_channel *b)
{
	/* a is better than b (return < 0) when a has rssi less than b */
	return as->as_maxrssi[a->ic_ieee] - as->as_maxrssi[b->ic_ieee];
}

/* This function must be invoked with locks acquired */
static int pc_cmp_idletime(struct ieee80211_channel *a, struct ieee80211_channel *b)
{
	if (!a->ic_idletime || !b->ic_idletime)
		return 0;

	/* a is better than b (return < 0) when a has more idle and less bias time than b */
	return ((100 - (u32)a->ic_idletime) + ieee80211_scan_get_bias(a)) - ((100 - (u32)b->ic_idletime) + ieee80211_scan_get_bias(b));
}

/* This function must be invoked with locks acquired */
static int pc_cmp_samechan(struct ieee80211com *ic, struct ieee80211_channel *a, struct ieee80211_channel *b)
{
	struct ieee80211_channel *ic_bsschan = ic->ic_bsschan;
	if (ic_bsschan == IEEE80211_CHAN_ANYC)
		return 0;
	/* a is better than b (return < 0) when a is current (and b is not) */
	return (b == ic_bsschan) - (a == ic_bsschan);
}

/* This function must be invoked with locks acquired */
static int pc_cmp_orig(struct channel *a, struct channel *b)
{
	return a->orig - b->orig;
}

/* This function must be invoked with locks acquired */
static int pc_cmp(const void *_a, const void *_b)
{
	struct ieee80211_channel *a = ((struct channel *)_a)->chan;
	struct ieee80211_channel *b = ((struct channel *)_b)->chan;
	struct pc_params *params = ((struct channel *)_a)->params;
	struct ieee80211com *ic = params->vap->iv_ic;
	int res;

#define EVALUATE_CRITERION(name, ...) do {			\
	if ((res = pc_cmp_##name(__VA_ARGS__)) != 0) 		\
		return res;					\
} while (0)

	EVALUATE_CRITERION(radar, a, b);
	EVALUATE_CRITERION(keepmode, params, a, b);
	EVALUATE_CRITERION(idletime, a, b);
	EVALUATE_CRITERION(sc, ic, a, b);
	/* XXX: rssi useless? pick_channel evaluates it anyway */
	EVALUATE_CRITERION(rssi, params->ss->ss_priv, a, b);
	EVALUATE_CRITERION(samechan, ic, a, b);
	EVALUATE_CRITERION(orig, (struct channel *)_a, (struct channel *)_b);

#undef EVALUATE_CRITERION
	return res;
}

/* This function must be invoked with locks acquired */
static void pc_swap(void *a, void *b, int n)
{
	struct ieee80211_channel *t = ((struct channel *)a)->chan;
	int i;

	((struct channel *)a)->chan = ((struct channel *)b)->chan;
	((struct channel *)b)->chan = t;

	i = ((struct channel *)a)->orig;
	((struct channel *)a)->orig = ((struct channel *)b)->orig;
	((struct channel *)b)->orig = i;

	/* (struct channel *)x->params doesn't have to be swapped, because it 
	 * is the same across all channels */
}

/* Pick a quiet channel to use for ap operation.
 * Must be invoked while we hold the locks. */
static struct ieee80211_channel *pick_channel(struct ieee80211_scan_state *ss, struct ieee80211vap *vap, u_int32_t flags)
{
	struct ieee80211com *ic = vap->iv_ic;
	unsigned int i, best_rssi;
	int ss_last = ss->ss_last;
	struct ieee80211_channel *best;
	struct ap_state *as = ss->ss_priv;
	struct channel *chans;	/* actually ss_last-1 is required */
	struct channel *c = NULL;
	struct pc_params params = { vap, ss, flags };
	int benefit = 0;
	int sta_assoc = 0;

	chans = (struct channel *)kmalloc(ss_last * sizeof(struct channel), GFP_ATOMIC);
	for (i = 0; i < ss_last; i++) {
		chans[i].chan = ss->ss_chans[i];
		chans[i].orig = i;
		chans[i].params = &params;
	}

	sort(chans, ss_last, sizeof(*chans), pc_cmp, pc_swap);

#ifdef IEEE80211_DEBUG
	for (i = 0; i < ss_last; i++) {
		int chan = ieee80211_chan2ieee(ic, chans[i].chan);

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN, "%s: channel %u, "
				  "rssi %d, radar %d, cn %d, km %d\n",
				  __func__, chan, as->as_maxrssi[chan], IEEE80211_IS_CHAN_RADAR(chans[i].chan), ic->ic_chan_nodes[chans[i].chan->ic_ieee], !!IEEE80211_ARE_CHANS_SAME_MODE(chans[i].chan, ic->ic_bsschan));
	}
#endif

	best = NULL;

	for (i = 0; i < ss_last; i++) {
		c = &chans[i];

		/* Verify channel is not marked for non-occupancy */
		if (IEEE80211_IS_CHAN_RADAR(c->chan))
			continue;

		if ((ic->ic_bsschan != NULL) && (ic->ic_bsschan != IEEE80211_CHAN_ANYC)) {

			/* Make sure the channels are the same mode */
			if ((flags & IEEE80211_SCAN_KEEPMODE) && !IEEE80211_ARE_CHANS_SAME_MODE(c->chan, ic->ic_bsschan))
				/* break the loop as the subsequent chans won't be 
				 * better */
				break;
		}

		best = c->chan;
		break;
	}

	if (best != NULL) {
		i = best->ic_ieee;
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN, "%s: best: channel %u rssi %d\n", __func__, i, as->as_maxrssi[i]);
	}
	kfree(chans);
	return best;
}

static int ap_end(struct ieee80211_scan_state *ss, struct ieee80211vap *vap, int (*action)(struct ieee80211vap *, const struct ieee80211_scan_entry *), u_int32_t flags)
{
	struct ap_state *as = ss->ss_priv;
	struct ieee80211_channel *bestchan = NULL;
	struct ieee80211com *ic = NULL;
	unsigned long sflags;
	int res = 1;

	SCAN_AP_LOCK_IRQ(as);

	KASSERT(vap->iv_opmode == IEEE80211_M_HOSTAP, ("wrong opmode %u", vap->iv_opmode));

	ic = vap->iv_ic;

	/* if we're already running, switch back to the home channel */
	if ((vap->iv_state == IEEE80211_S_RUN) && (ic->ic_bsschan != IEEE80211_CHAN_ANYC)) {
		ic->ic_curchan = ic->ic_bsschan;
		ic->ic_set_channel(ic);
		goto out;
	}

	/* record stats for the channel that was scanned last */
	ic->ic_set_channel(ic);
	spin_lock_irqsave(&channel_lock, sflags);
	ieee80211_scan_set_bss_channel(ic, NULL);
	bestchan = pick_channel(ss, vap, flags);
	if (bestchan == NULL) {
		spin_unlock_irqrestore(&channel_lock, sflags);
		if (ss->ss_last > 0) {
			/* no suitable channel, should not happen */
			printk(KERN_ERR "%s: %s: no suitable channel! " "(should not happen)\n", DEV_NAME(vap->iv_dev), __func__);
		}
		res = 1;	/* Do NOT restart scan */
	} else {
		struct ieee80211_scan_entry se;
		int i;
		/* XXX: notify all VAPs? */
		/* if this is a dynamic turbo frequency , start with normal 
		 * mode first */
		if (IEEE80211_IS_CHAN_TURBO(bestchan) && !IEEE80211_IS_CHAN_STURBO(bestchan)) {
			if ((bestchan = ieee80211_find_channel(ic, bestchan->ic_freq, bestchan->ic_flags & ~IEEE80211_CHAN_TURBO)) == NULL) {
				/* should never happen ?? */
				spin_unlock_irqrestore(&channel_lock, sflags);
				SCAN_AP_UNLOCK_IRQ_EARLY(as);
				return 0;
			}
		}
		for (i = (bestchan - &ic->ic_channels[0]) / sizeof(*bestchan) + 1; i < ic->ic_nchans; i++) {
			if ((ic->ic_channels[i].ic_freq == bestchan->ic_freq) && ic->ic_channels[i].ic_flags == bestchan->ic_flags)
//                      if ((ic->ic_channels[i].ic_freq == bestchan->ic_freq) && IEEE80211_IS_CHAN_ANYG(&ic->ic_channels[i]))
				bestchan = &ic->ic_channels[i];
		}
		memset(&se, 0, sizeof(se));
		se.se_chan = bestchan;

		as->as_action = ss->ss_ops->scan_default;
		if (action)
			as->as_action = action;
		as->as_selbss = se;

		ieee80211_scan_set_bss_channel(ic, bestchan);
		spin_unlock_irqrestore(&channel_lock, sflags);

		/* Must defer action to avoid possible recursive call through 
		 * 80211 state machine, which would result in recursive 
		 * locking. */
		IEEE80211_SCHEDULE_TQUEUE(&as->as_actiontq);
		res = 1;
	}

out:
	SCAN_AP_UNLOCK_IRQ(as);
	return res;
}

static void ap_age(struct ieee80211_scan_state *ss)
{
	struct ap_state *as = ss->ss_priv;
	struct scan_entry *se, *next;

	SCAN_AP_LOCK_IRQ(as);
	TAILQ_FOREACH_SAFE(se, &as->as_entry, se_list, next) {
		if (se->se_notseen > AP_PURGE_SCANS) {
			TAILQ_REMOVE(&as->as_entry, se, se_list);
			LIST_REMOVE(se, se_hash);
			FREE(se, M_80211_SCAN);
		}
	}
	SCAN_AP_UNLOCK_IRQ(as);
}

static int ap_iterate(struct ieee80211_scan_state *ss, ieee80211_scan_iter_func * f, void *arg)
{
	struct ap_state *as = ss->ss_priv;
	struct scan_entry *se;
	u_int gen;
	int res = 0;

	SCAN_AP_GEN_LOCK(as);
	gen = as->as_scangen++;
restart:
	SCAN_AP_LOCK_IRQ(as);
	TAILQ_FOREACH(se, &as->as_entry, se_list) {
		if (se->se_scangen != gen) {
			se->se_scangen = gen;
			/* update public state */
			se->base.se_age = jiffies - se->se_lastupdate;
			SCAN_AP_UNLOCK_IRQ_EARLY(as);

			res = (*f) (arg, &se->base);

			/* We probably ran out of buffer space. */
			if (res != 0)
				goto done;

			goto restart;
		}
	}

	SCAN_AP_UNLOCK_IRQ(as);

done:
	SCAN_AP_GEN_UNLOCK(as);

	return res;
}

static void ap_assoc_success(struct ieee80211_scan_state *ss, const u_int8_t macaddr[IEEE80211_ADDR_LEN])
{
	/* should not be called */
}

static void ap_assoc_fail(struct ieee80211_scan_state *ss, const u_int8_t macaddr[IEEE80211_ADDR_LEN], int reason)
{
	/* should not be called */
}

/*
 * Default action to execute when a scan entry is found for ap
 * mode.  Return 1 on success, 0 on failure
 */
static int ap_default_action(struct ieee80211vap *vap, const struct ieee80211_scan_entry *se)
{
	ieee80211_create_ibss(vap, se->se_chan);

	return 1;
}

static void action_tasklet(IEEE80211_TQUEUE_ARG data)
{
	struct ieee80211_scan_state *ss = (struct ieee80211_scan_state *)data;
	struct ap_state *as = (struct ap_state *)ss->ss_priv;
	struct ieee80211vap *vap = ss->ss_vap;
	SCAN_AP_LOCK_IRQ(as);
	if (as->as_newscan) {
		struct scan_entry *se;
		TAILQ_FOREACH(se, &as->as_entry, se_list) {
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
		as->as_newscan = 0;
	}
	SCAN_AP_UNLOCK_IRQ(as);

	(*ss->ss_ops->scan_default) (vap, &as->as_selbss);
}

static const struct ieee80211_scanner ap_default = {
	.scan_name = "default",
	.scan_attach = ap_attach,
	.scan_detach = ap_detach,
	.scan_start = ap_start,
	.scan_restart = ap_restart,
	.scan_cancel = ap_cancel,
	.scan_end = ap_end,
	.scan_flush = ap_flush,
	.scan_add = ap_add,
	.scan_age = ap_age,
	.scan_iterate = ap_iterate,
	.scan_assoc_success = ap_assoc_success,
	.scan_assoc_fail = ap_assoc_fail,
	.scan_default = ap_default_action,
};

#include "module.h"

/*
 * Module glue.
 */
MODULE_AUTHOR("Errno Consulting, Sam Leffler");
MODULE_DESCRIPTION("802.11 wireless support: default ap scanner");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static int __init init_scanner_ap(void)
{
	ieee80211_scanner_register(IEEE80211_M_HOSTAP, &ap_default);
	return 0;
}

module_init(init_scanner_ap);

static void __exit exit_scanner_ap(void)
{
	ieee80211_scanner_unregister_all(&ap_default);
}

module_exit(exit_scanner_ap);
