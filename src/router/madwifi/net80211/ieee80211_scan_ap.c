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
 * $Id: ieee80211_scan_ap.c 1426 2006-02-01 20:07:11Z mrenzmann $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * IEEE 802.11 ap scanning support.
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/delay.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>

struct ap_state {
	int as_maxrssi[IEEE80211_CHAN_MAX];
	struct IEEE80211_TQ_STRUCT as_actiontq;	/* tasklet for "action" */
	struct ieee80211_scan_entry as_selbss;	/* selected bss for action tasklet */
	int (*as_action)(struct ieee80211vap *, const struct ieee80211_scan_entry *);
};

static int ap_flush(struct ieee80211_scan_state *);
static void action_tasklet(IEEE80211_TQUEUE_ARG);

/*
 * Attach prior to any scanning work.
 */
static int
ap_attach(struct ieee80211_scan_state *ss)
{
	struct ap_state *as;

	_MOD_INC_USE(THIS_MODULE, return 0);

	MALLOC(as, struct ap_state *, sizeof(struct ap_state),
		M_SCANCACHE, M_NOWAIT);
	ss->ss_priv = as;
	IEEE80211_INIT_TQUEUE(&as->as_actiontq, action_tasklet, ss);
	ap_flush(ss);
	return 1;
}

/*
 * Cleanup any private state.
 */
static int
ap_detach(struct ieee80211_scan_state *ss)
{
	struct ap_state *as = ss->ss_priv;

	if (as != NULL)
		FREE(as, M_SCANCACHE);

	_MOD_DEC_USE(THIS_MODULE);
	return 1;
}

/*
 * Flush all per-scan state.
 */
static int
ap_flush(struct ieee80211_scan_state *ss)
{
	struct ap_state *as = ss->ss_priv;

	memset(as->as_maxrssi, 0, sizeof(as->as_maxrssi));
	ss->ss_last = 0;		/* ensure no channel will be picked */
	return 0;
}

static int
find11gchannel(struct ieee80211com *ic, int i, int freq)
{
	const struct ieee80211_channel *c;
	int j;

	/*
	 * The normal ordering in the channel list is b channel
	 * immediately followed by g so optimize the search for
	 * this.  We'll still do a full search just in case.
	 */
	for (j = i+1; j < ic->ic_nchans; j++) {
		c = &ic->ic_channels[j];
		if (c->ic_freq == freq && IEEE80211_IS_CHAN_ANYG(c))
			return 1;
	}
	for (j = 0; j < i; j++) {
		c = &ic->ic_channels[j];
		if (c->ic_freq == freq && IEEE80211_IS_CHAN_ANYG(c))
			return 1;
	}
	return 0;
}

/*
 * Start an ap scan by populating the channel list.
 */
static int
ap_start(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_channel *c;
	int i;

	ss->ss_last = 0;
	if (vap->iv_des_mode == IEEE80211_MODE_AUTO) {
		for (i = 0; i < ic->ic_nchans; i++) {
			c = &ic->ic_channels[i];
			if (IEEE80211_IS_CHAN_TURBO(c)) {
				/* XR is not supported on turbo channels */
				if (vap->iv_ath_cap & IEEE80211_ATHC_XR)
					continue;
				/* dynamic channels are scanned in base mode */
				if (!IEEE80211_IS_CHAN_ST(c))
					continue;
			} else {
				/*
				 * Use any 11g channel instead of 11b one.
				 */
				if (IEEE80211_IS_CHAN_B(c) &&
				    find11gchannel(ic, i, c->ic_freq))
					continue;
			}
			if (c->ic_flags & IEEE80211_CHAN_RADAR)
				continue;
			if (ss->ss_last >= IEEE80211_SCAN_MAX)
				break;
			ss->ss_chans[ss->ss_last++] = c;
		}
	} else {
		static const u_int chanflags[] = {
			0,			/* IEEE80211_MODE_AUTO */
			IEEE80211_CHAN_A,	/* IEEE80211_MODE_11A */
			IEEE80211_CHAN_B,	/* IEEE80211_MODE_11B */
			IEEE80211_CHAN_PUREG,	/* IEEE80211_MODE_11G */
			IEEE80211_CHAN_FHSS,	/* IEEE80211_MODE_FH */
			IEEE80211_CHAN_108A,	/* IEEE80211_MODE_TURBO_A */
			IEEE80211_CHAN_108G,	/* IEEE80211_MODE_TURBO_G */
			IEEE80211_CHAN_ST,	/* IEEE80211_MODE_TURBO_STATIC_A */
		};
		u_int modeflags;

		modeflags = chanflags[vap->iv_des_mode];
		if (vap->iv_ath_cap & IEEE80211_ATHC_TURBOP && modeflags != IEEE80211_CHAN_ST) {
			if (vap->iv_des_mode == IEEE80211_MODE_11G)
				modeflags = IEEE80211_CHAN_108G;
			else
				modeflags = IEEE80211_CHAN_108A;
		}
		for (i = 0; i < ic->ic_nchans; i++) {
			c = &ic->ic_channels[i];
			if ((c->ic_flags & modeflags) != modeflags)
				continue;
			/* XR is not supported on turbo channels */
			if (IEEE80211_IS_CHAN_TURBO(c) && vap->iv_ath_cap & IEEE80211_ATHC_XR)
				continue;
			if (ss->ss_last >= IEEE80211_SCAN_MAX)
				break;
			/* 
			 * do not select static turbo channels if the mode is not
			 * static turbo .
			 */
			if (IEEE80211_IS_CHAN_STURBO(c) && vap->iv_des_mode != IEEE80211_MODE_MAX ) 
				continue;
			/* No dfs interference detected channels */
			if (c->ic_flags & IEEE80211_CHAN_RADAR)
				continue;
			ss->ss_chans[ss->ss_last++] = c;
		}
	}
	ss->ss_next = 0;
	/* XXX tunables */
	ss->ss_mindwell = msecs_to_jiffies(200);	/* 200ms */
	ss->ss_maxdwell = msecs_to_jiffies(300);	/* 300ms */

#ifdef IEEE80211_DEBUG
	if (ieee80211_msg_scan(vap)) {
		printf("%s: scan set ", vap->iv_dev->name);
		ieee80211_scan_dump_channels(ss);
		printf(" dwell min %ld max %ld\n",
			ss->ss_mindwell, ss->ss_maxdwell);
	}
#endif /* IEEE80211_DEBUG */

	return 0;
}

/*
 * Restart a bg scan.
 */
static int
ap_restart(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	return 0;
}

/*
 * Cancel an ongoing scan.
 */
static int
ap_cancel(struct ieee80211_scan_state *ss, struct ieee80211vap *vap)
{
	struct ap_state *as = ss->ss_priv;

	IEEE80211_CANCEL_TQUEUE(&as->as_actiontq);
	return 0;
}

/*
 * Record max rssi on channel.
 */
static int
ap_add(struct ieee80211_scan_state *ss, const struct ieee80211_scanparams *sp,
	const struct ieee80211_frame *wh, int subtype, int rssi, int rstamp)
{
	struct ap_state *as = ss->ss_priv;
	struct ieee80211vap *vap = ss->ss_vap;
	struct ieee80211com *ic = vap->iv_ic;
	int chan;

	chan = ieee80211_chan2ieee(ic, ic->ic_curchan);
	/* XXX better quantification of channel use? */
	/* XXX count bss's? */
	if (rssi > as->as_maxrssi[chan])
		as->as_maxrssi[chan] = rssi;
	/* XXX interference, turbo requirements */
	return 1;
}

/*
 * Pick a quiet channel to use for ap operation.
 */
static int
ap_end(struct ieee80211_scan_state *ss, struct ieee80211vap *vap,
       int (*action)(struct ieee80211vap *, const struct ieee80211_scan_entry *),
       u_int32_t flags)
{
	struct ap_state *as = ss->ss_priv;
	struct ieee80211com *ic = vap->iv_ic;
	int i, chan, bestchan, bestchanix;
	unsigned int random;
	KASSERT(vap->iv_opmode == IEEE80211_M_HOSTAP,
		("wrong opmode %u", vap->iv_opmode));
	/* XXX select channel more intelligently, e.g. channel spread, power */
	bestchan = -1;
	bestchanix = 0;		/* NB: silence compiler */
	if (ic->ic_isdfsregdomain && (ic->ic_curmode == IEEE80211_MODE_11A)) {
	   if (ss->ss_last) {
try_again:
		/* Pick up a random channel to start */
		get_random_bytes(&random, sizeof(random));

		i = random % ss->ss_last;
			 

		if (ss->ss_chans[i]->ic_flags & IEEE80211_CHAN_RADAR)
		    goto try_again; 
		if (flags & IEEE80211_SCAN_KEEPMODE) {
			if (ic->ic_curchan != NULL) {
				if ((ss->ss_chans[i]->ic_flags & IEEE80211_CHAN_ALLTURBO) != (ic->ic_curchan->ic_flags & IEEE80211_CHAN_ALLTURBO))
					goto try_again;
			}
		}

		chan = ieee80211_chan2ieee(ic, ss->ss_chans[i]);

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN,
			    		"%s: picked random channel %u\n",
	    		__func__, chan);

		bestchan = chan;
		bestchanix = i;
	   }
	} else {
		/* NB: use scan list order to preserve channel preference */
		for (i = 0; i < ss->ss_last; i++) {
			/*
		 	 * If the channel is unoccupied the max rssi
		 	 * should be zero; just take it.  Otherwise
		 	 * track the channel with the lowest rssi and
		 	 * use that when all channels appear occupied.
		 	*/
			/*
		 	 * Check for channel interference, and if found,
		 	 * skip the channel.  We assume that all channels
		 	 * will be checked so atleast one can be found
		 	 * suitable and will change.  IF this changes,
		 	 * then we must know when we "have to" change
		 	 * channels for radar and move off.
		 	*/

			if (ss->ss_chans[i]->ic_flags & IEEE80211_CHAN_RADAR)
				continue;

			chan = ieee80211_chan2ieee(ic, ss->ss_chans[i]);

			IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN,
		    	"%s: channel %u rssi %d bestchan %d bestchan rssi %d\n",
		    	__func__, chan, as->as_maxrssi[chan],
		    	bestchan, bestchan != -1 ? as->as_maxrssi[bestchan] : 0);

			if (as->as_maxrssi[chan] == 0) {
			bestchan = chan;
				bestchanix = i;
				/* XXX use other considerations */
				break;
			}
			if (bestchan == -1 ||
		    		as->as_maxrssi[chan] < as->as_maxrssi[bestchan])
					bestchan = chan;
		}
    }

	if (bestchan == -1) {
		/* no suitable channel, should not happen */
		IEEE80211_DPRINTF(vap, IEEE80211_MSG_SCAN,
			    	"%s: no suitable channel! (should not happen)\n", __func__); 
		/* XXX print something? */
		return 0;			/* restart scan */
	} else {
		struct ieee80211_channel *c;
		struct ieee80211_scan_entry se;
		/* XXX notify all vap's? */
		/* if this is a dynamic turbo frequency , start with normal mode first */

		c = ss->ss_chans[bestchanix];
		if (IEEE80211_IS_CHAN_TURBO(c) && !IEEE80211_IS_CHAN_STURBO(c)) { 
			if ((c = ieee80211_find_channel(ic, c->ic_freq, 
				c->ic_flags & ~IEEE80211_CHAN_TURBO)) == NULL) {
				/* should never happen ?? */
				return 0;
			}
		}
		memset(&se, 0, sizeof(se));
		se.se_chan = c;

		as->as_action = ss->ss_ops->scan_default;
		if (action)
			as->as_action = action;
		as->as_selbss = se;
		
		/* 
		 * Must defer action to avoid possible recursive call through 80211
		 * state machine, which would result in recursive locking.
		 */
		IEEE80211_SCHEDULE_TQUEUE(&as->as_actiontq);
		return 1;
	}
}

static void
ap_age(struct ieee80211_scan_state *ss)
{
	/* XXX is there anything meaningful to do? */
}

static void
ap_iterate(struct ieee80211_scan_state *ss,
	ieee80211_scan_iter_func *f, void *arg)
{
	/* NB: nothing meaningful we can do */
}

static void
ap_assoc_success(struct ieee80211_scan_state *ss,
	const u_int8_t macaddr[IEEE80211_ADDR_LEN])
{
	/* should not be called */
}

static void
ap_assoc_fail(struct ieee80211_scan_state *ss,
	const u_int8_t macaddr[IEEE80211_ADDR_LEN], int reason)
{
	/* should not be called */
}

/*
 * Default action to execute when a scan entry is found for ap
 * mode.  Return 1 on success, 0 on failure
 */
static int
ap_default_action(struct ieee80211vap *vap,
	const struct ieee80211_scan_entry *se)
{
	ieee80211_create_ibss(vap, se->se_chan);
	return 1;
}

static void
action_tasklet(IEEE80211_TQUEUE_ARG data)
{
	struct ieee80211_scan_state *ss = (struct ieee80211_scan_state *)data;
	struct ap_state *as = (struct ap_state *)ss->ss_priv;
	struct ieee80211vap *vap = ss->ss_vap;

	(*ss->ss_ops->scan_default)(vap, &as->as_selbss);
}

/*
 * Module glue.
 */
MODULE_AUTHOR("Errno Consulting, Sam Leffler");
MODULE_DESCRIPTION("802.11 wireless support: default ap scanner");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static const struct ieee80211_scanner ap_default = {
	.scan_name		= "default",
	.scan_attach		= ap_attach,
	.scan_detach		= ap_detach,
	.scan_start		= ap_start,
	.scan_restart		= ap_restart,
	.scan_cancel		= ap_cancel,
	.scan_end		= ap_end,
	.scan_flush		= ap_flush,
	.scan_add		= ap_add,
	.scan_age		= ap_age,
	.scan_iterate		= ap_iterate,
	.scan_assoc_success	= ap_assoc_success,
	.scan_assoc_fail	= ap_assoc_fail,
	.scan_default		= ap_default_action,
};

static int __init
init_scanner_ap(void)
{
	ieee80211_scanner_register(IEEE80211_M_HOSTAP, &ap_default);
	return 0;
}
module_init(init_scanner_ap);

static void __exit
exit_scanner_ap(void)
{
	ieee80211_scanner_unregister_all(&ap_default);
}
module_exit(exit_scanner_ap);
