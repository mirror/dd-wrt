/*-
 * Copyright (c) 2005 John Bicket
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: minstrel.c 1525 2006-04-23 21:05:57Z dyqith $
 */

/* And then Indranet Technologies Ltd sponsored Derek Smithies to work
 * on this code. Derek Smithies (derek@indranet.co.nz) took parts of the 
 * adm module and pasted it into this code base. 
 *
 * This version of John Bicket's code takes the experimental approach one
 * step further.
 * When in auto rate mode, packets are sent at the selected rate.
 * The Hal asks for what alternative rate to use if the selected rate fails.
 * We provide the alternative rate from a random selection of 1.. max rate.
 * Given the probability of success, multiplied with the transmission time,
 * we can determine the rate which maximises packet throughput.
 *
 * Different rates are used for every remote node - some nodes will work 
 * better on different rates.
 * Every second, a timer fires, to assess the throughput at each rate with 
 * each remote node.
 * This timer will then determine the optimum rate for each remote node, based
 * on the performance figures.
 *
 * This code is called minstrel, because we have taken a wandering minstrel
 * approach. Wander around the different rates, singing wherever 
 * you can. And then, look at the performance, and make a choice.
 *
 * It is not an aimless search, there is some direction to the search
 * pattern. But then, the minstels of old only sung where they thought
 * they would get an income. Similarily, we direct thesearch a little.
 *
 *  Enjoy.  Derek Smithies. */

/* This file is an implementation of the SampleRate algorithm
 * in "Bit-rate Selection in Wireless Networks"
 * (http://www.pdos.lcs.mit.edu/papers/jbicket-ms.ps)
 *
 * SampleRate chooses the bit-rate it predicts will provide the most
 * throughput based on estimates of the expected per-packet
 * transmission time for each bit-rate.  SampleRate periodically sends
 * packets at bit-rates other than the current one to estimate when
 * another bit-rate will provide better performance. SampleRate
 * switches to another bit-rate when its estimated per-packet
 * transmission time becomes smaller than the current bit-rate's.
 * SampleRate reduces the number of bit-rates it must sample by
 * eliminating those that could not perform better than the one
 * currently being used.  SampleRate also stops probing at a bit-rate
 * if it experiences several successive losses.
 *
 * The difference between the algorithm in the thesis and the one in this
 * file is that the one in this file uses an EWMA instead of a window.
 *
 * Also, this implementation tracks the average transmission time for
 * a few different packet sizes independently for each link. */

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/cache.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/if_arp.h>
#include <linux/net.h>		/* for net_random */
#include <linux/vmalloc.h>

#include <asm/uaccess.h>

#include <net80211/if_media.h>
#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_rate.h>

#include "if_ath_debug.h"
#include "if_athvar.h"
#include "if_ath_hal.h"
#include "ah_desc.h"

#include "minstrel.h"

#define ONE_SECOND (1000 * 1000)  /* 1 second, or 1000 milliseconds; eternity, in other words */
#define TIMER_INTERVAL 100 /* msecs */

#include "release.h"

static char *version = "1.2 (" RELEASE_VERSION ")";
static char *dev_info = "ath_rate_minstrel";

#define STALE_FAILURE_TIMEOUT_MS 10000
#define ENABLE_MRR 1

/* 10% of the time, send a packet at something other than the optimal rate, which fills
 * the statistics tables nicely. This percentage is applied to the first packet of the
 * multi rate retry chain. */
static int ath_lookaround_rate = 10;
static int ath_ewma_level      = 75;
static int ath_segment_size    = 6000;
static void ath_rate_ctl_reset(struct ath_softc *, struct ieee80211_node *);

/* Calculate the throughput and probability of success for each node
 * we are talking on, based on the statistics collected during the
 * last timer period. */
static void ath_rate_statistics(struct ieee80211_node *ni);


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,52))
MODULE_PARM(ath_lookaround_rate, "i");
MODULE_PARM(ath_ewma_level, "i");
MODULE_PARM(ath_segment_size, "i");
#else
#include <linux/moduleparam.h>
module_param(ath_lookaround_rate, 	int, 0600);
module_param(ath_ewma_level, 		int, 0600);
module_param(ath_segment_size, 		int, 0600);
#endif
MODULE_PARM_DESC(ath_lookaround_rate, " % of packets sent to fill statistics table (10) ");
MODULE_PARM_DESC(ath_ewma_level, " scaling % used in ewma rolloff calculations  (75) ");
MODULE_PARM_DESC(ath_segment_size, " max duration of time to spend in either of the first two mrr segments (6000)");


static __inline int
rate_to_ndx(struct minstrel_node *sn, int rate)
{
		unsigned int x = 0;
		for (x = 0; x < sn->num_rates; x++)
			if (sn->rates[x].rate == rate)
				return x;
		return -1;
}

static void
ath_rate_node_init(struct ath_softc *sc, struct ath_node *an)
{
		/* NB: Assumed to be zero'd by caller */
		ath_rate_ctl_reset(sc, &an->an_node);
}


static void
ath_rate_node_cleanup(struct ath_softc *sc, struct ath_node *an)
{
}

#if 0
static void
ath_rate_node_copy(struct ath_softc *sc,
		struct ath_node *dst, const struct ath_node *src)
{
		struct minstrel_node *odst = ATH_NODE_MINSTREL(dst);
		const struct minstrel_node *osrc = (const struct minstrel_node *)&src[1];
		memcpy(odst, osrc, sizeof(struct minstrel_node));
}
#endif

static void
ath_rate_findrate(struct ath_softc *sc, struct ath_node *an,
		int shortPreamble, size_t frameLen,
		u_int8_t *rix, unsigned int *try0, u_int8_t *txrate)
{
		struct minstrel_node *sn = ATH_NODE_MINSTREL(an);
		struct ieee80211com *ic = &sc->sc_ic;
		unsigned int ndx, offset;
		int mrr;


		if (abs(jiffies - sn->last_update) > msecs_to_jiffies(TIMER_INTERVAL)) {
			ath_rate_statistics(&an->an_node);
			sn->last_update = jiffies;
		}
		if (sn->num_rates <= 0) {
			    printk(KERN_WARNING "%s: no rates for " MAC_FMT "?\n",
			           dev_info,
			           MAC_ADDR(an->an_node.ni_macaddr));
			    return;
		}

		mrr = sc->sc_mrretry && !(ic->ic_flags & IEEE80211_F_USEPROT) && ENABLE_MRR;

		if (sn->static_rate_ndx >= 0) {
			    ndx = sn->static_rate_ndx;
		} else {
			int delta;
			sn->packet_count++;
			sn->random_n = (sn->a * sn->random_n) + sn->b;
			offset = sn->random_n & 0xf;
			delta = (sn->packet_count * ath_lookaround_rate / 100) - sn->sample_count;
			if ((delta > 0) && (offset < 2)) {
				sn->sample_count++;
				sn->is_sampling = 1;
				if (sn->packet_count >= 10000) {
					sn->sample_count = 0;
					sn->packet_count = 0;
				} else if (delta > sn->num_rates * 2) {
					sn->sample_count += ((delta - sn->num_rates * 2) * ath_lookaround_rate) / 100;
				}

				/* Don't look for slowest rate (i.e. slowest
				 * base rate). We must presume that the slowest
				 * rate works fine, or else other management
				 * frames will also be failing - therefore the
				 * link will soon be broken anyway. Indeed,
				 * the slowest rate was used to establish the
				 * link in the first place. */
				ndx = sn->rs_sampleTable[sn->rs_sampleIndex][sn->rs_sampleColumn];

				sn->rs_sampleIndex++;
				if (sn->rs_sampleIndex > (sn->num_rates - 2)) {
					sn->rs_sampleIndex = 0;

					sn->rs_sampleColumn++;
					if (sn->rs_sampleColumn >= MINSTREL_COLUMNS)
						sn->rs_sampleColumn = 0;
				}
				sn->rs_sample_rate = ndx;
				sn->rs_sample_rate_slower =
					sn->perfect_tx_time[ndx] >  sn->perfect_tx_time[sn->max_tp_rate];
				if (sn->rs_sample_rate_slower)
					ndx = sn->max_tp_rate;				
			} else
				ndx = sn->max_tp_rate;
		 }

		if ((sn->static_rate_ndx != -1) || !mrr)
			    *try0 = ATH_TXMAXTRY;
		else
			    *try0 = sn->retry_adjusted_count[ndx];

		KASSERT((ndx < sn->num_rates),
			    ("%s: bad ndx (%d/%d) for " MAC_FMT "?\n",
			     dev_info, ndx, sn->num_rates,
			     MAC_ADDR(an->an_node.ni_macaddr)));

		*rix = sn->rates[ndx].rix;
		if (shortPreamble)
			    *txrate = sn->rates[ndx].shortPreambleRateCode;
		else
			    *txrate = sn->rates[ndx].rateCode;
}


static void
ath_rate_get_mrr(struct ath_softc *sc, struct ath_node *an, int shortPreamble,
		 size_t frame_size, u_int8_t rix, struct ieee80211_mrr *mrr)
{
		struct minstrel_node *sn = ATH_NODE_MINSTREL(an);
		int rc1, rc2, rc3;         /* Index into the rate table, so for example, it is  0..11 */

		if (sn->num_rates <= 0)
			return;

		mrr->privflags = sn->is_sampling;
		if (sn->is_sampling) {
			sn->is_sampling = 0;
			if (sn->rs_sample_rate_slower) {
				rc1 = sn->rs_sample_rate;
				if (sn->sample_count > 0)
					sn->sample_count--;
			} else
				rc1 = sn->max_tp_rate;
		} else {
			rc1 = sn->max_tp_rate2;
		}

		rc2 = sn->max_prob_rate;
		rc3 = 0;

		KASSERT((rc1 >= 0) && (rc1 < sn->num_rates),
			    ("%s: bad rc1 (%d/%d) for " MAC_FMT "?\n",
			     dev_info, rc1, sn->num_rates,
			     MAC_ADDR(an->an_node.ni_macaddr)));

		KASSERT((rc2 >= 0) && (rc2 < sn->num_rates),
			    ("%s: bad rc2 (%d/%d) for " MAC_FMT "?\n",
			     dev_info, rc2, sn->num_rates,
			     MAC_ADDR(an->an_node.ni_macaddr)));

		KASSERT((rc3 >= 0) && (rc3 < sn->num_rates),
			    ("%s: bad rc3 (%d/%d) for " MAC_FMT "?\n",
			     dev_info, rc3, sn->num_rates,
			     MAC_ADDR(an->an_node.ni_macaddr)));

		if (shortPreamble) {
			mrr->rate1 = sn->rates[rc1].shortPreambleRateCode;
			mrr->rate2 = sn->rates[rc2].shortPreambleRateCode;
			mrr->rate3 = sn->rates[rc3].shortPreambleRateCode;
		} else {
			mrr->rate1 = sn->rates[rc1].rateCode;
			mrr->rate2 = sn->rates[rc2].rateCode;
			mrr->rate3 = sn->rates[rc3].rateCode;
		}

		mrr->retries1 = sn->retry_adjusted_count[rc1];
		mrr->retries2 = sn->retry_adjusted_count[rc2];
		mrr->retries3 = sn->retry_adjusted_count[rc3];
}

static void
ath_rate_tx_complete(struct ath_softc *sc,
		struct ath_node *an, const struct ath_buf *bf,
		const struct ieee80211_mrr *mrr)
{
		struct minstrel_node *sn = ATH_NODE_MINSTREL(an);
		struct ieee80211com *ic = &sc->sc_ic;
		const struct ath_tx_status *ts = &bf->bf_dsstatus.ds_txstat;
		const struct ath_desc *ds = &bf->bf_desc[0];
		int final_rate = 0;
		int tries = 0;
		int use_mrr;
		int final_ndx;
		int ndx0, ndx1, ndx2, ndx3;

		/* This is the index in the retry chain we finish at.
		 * With no retransmits, it is always 0.
		 * int finalTSIdx = ads->final_ts_index; */
		final_rate = sc->sc_hwmap[ts->ts_rate & ~HAL_TXSTAT_ALTRATE].ieeerate;
		final_ndx = rate_to_ndx(sn, final_rate);
		if (final_ndx >= sn->num_rates) {
			DPRINTF(sc, ATH_DEBUG_RATE, "%s: final ndx too high\n", __func__);
			final_ndx = 0;
		}
		if (final_ndx < 0) {
			DPRINTF(sc, ATH_DEBUG_RATE, "%s: final ndx too low\n", __func__);
			final_ndx = 0;
		}

		/* 'tries' is the total number of times we have endeavoured to
		 * send this packet, and is a sum of the #attempts at each
		 * level in the multi-rate retry chain */
		tries = ts->ts_longretry + 1;

		if (sn->num_rates <= 0) {
			DPRINTF(sc, ATH_DEBUG_RATE, "%s: " MAC_FMT " %s no rates yet\n", dev_info,
				MAC_ADDR(an->an_node.ni_macaddr), __func__);
			return;
		}

		if (!ts->ts_status)  /* Success when sending a packet*/
			sn->rs_ratesuccess[final_ndx]++;

		use_mrr = sc->sc_mrretry && !(ic->ic_flags & IEEE80211_F_USEPROT) && ENABLE_MRR;

		if (!use_mrr) {
			if ((0 <= final_ndx) && (final_ndx < sn->num_rates)) {
				sn->rs_rateattempts[final_ndx] += tries; /* only one rate was used */
			}
			return;
		}

		/* Now, query the hal/hardware to find out the contents of the multirate retry chain.
		 * If we have it set to 6,3,2,2, this call will always return 6,3,2,2. For some packets, we can
		 * get a mrr of 0, -1, -1, -1, which indicates there is no chain installed for that packet */
		ndx0 = rate_to_ndx(sn, mrr->rate0);
		ndx1 = rate_to_ndx(sn, mrr->rate1);
		ndx2 = rate_to_ndx(sn, mrr->rate2);
		ndx3 = rate_to_ndx(sn, mrr->rate3);

		sn->rs_rateattempts[ndx0] += MIN(tries, mrr->retries0);
		if (tries <= mrr->retries0)
			return;

		if (mrr->retries1 < 0)
			return;
		tries = tries - mrr->retries0;
		sn->rs_rateattempts[ndx1] += MIN(tries, mrr->retries1);
		if (tries <= mrr->retries1)
			return;

		if (bf->rcflags)
			sn->sample_count++;

		if (mrr->retries2 < 0)
			return;
		tries = tries - mrr->retries1;
		sn->rs_rateattempts[ndx2] += MIN(tries, mrr->retries2);
		if (tries <= mrr->retries2)
			return;

		if (mrr->retries3 < 0)
			return;
		tries = tries - mrr->retries2;
		sn->rs_rateattempts[ndx3] += MIN(tries, mrr->retries3);
}

static void
ath_rate_newassoc(struct ath_softc *sc, struct ath_node *an, int isnew)
{
		DPRINTF(sc, ATH_DEBUG_RATE, "%s: " MAC_FMT " %s\n", dev_info,
			MAC_ADDR(an->an_node.ni_macaddr), __func__);
		if (isnew)
			ath_rate_ctl_reset(sc, &an->an_node);
}

static void
ath_fill_sample_table(struct minstrel_node *sn)
{
		unsigned int num_sample_rates = (sn->num_rates - 1);
		/* newIndex varies as 0 .. (num_rates - 2) 
		 * The highest index rate is the slowest and is ignored */
		unsigned int i, column_index, newIndex;
		u_int8_t random_bytes[8];

		/* This should be unnecessary if we are assuming storage is provided
		 * as zeroed */
		memset(sn->rs_sampleTable, 0, sizeof(sn->rs_sampleTable));

		sn->rs_sampleColumn = 0;
		sn->rs_sampleIndex = 0;

		/* Seed value to random number generator, which determines when we
		 * send a sample packet at some non-optimal rate
		 * FIXME: randomise? */
		sn->random_n = 1;
		sn->a = 1664525;
		sn->b = 1013904223;

		if (sn->num_rates > 1) {
			for (column_index = 0; column_index < MINSTREL_COLUMNS; column_index++) {
				for (i = 0; i < num_sample_rates; i++) {
					get_random_bytes(random_bytes, 8);
					newIndex = (i + random_bytes[i & 7]) % num_sample_rates;

					while (sn->rs_sampleTable[newIndex][column_index] != 0)
						newIndex = (newIndex + 1) % num_sample_rates;

					sn->rs_sampleTable[newIndex][column_index] = i + 1;
				}
			}
		}

#if 0
		char rates[200];
		char *p;
		for (column_index = 0; column_index < MINSTREL_COLUMNS; column_index++) {
			    p = rates + sprintf(rates, "rates :: %d ", column_index);
			    for (i = 0; i < num_sample_rates; i++)
			            p += sprintf(p, "%2u ", sn->rs_sampleTable[i][column_index]);
			    DPRINTF(sc, ATH_DEBUG_RATE, "%s\n", rates);
		};
#endif
}

/* Initialize the tables for a node. */
static void
ath_rate_ctl_reset(struct ath_softc *sc, struct ieee80211_node *ni)
{
		struct ath_node *an = ATH_NODE(ni);
		struct minstrel_node *sn = ATH_NODE_MINSTREL(an);
		struct ieee80211vap *vap = ni->ni_vap;
		const HAL_RATE_TABLE *rt = sc->sc_currates;
		unsigned int x;
		int retry_index, tx_time;
		int srate;
		int ndx = 0;

		sn->num_rates = 0;
		sn->max_tp_rate = 0;
		sn->max_tp_rate2 = 0;
		sn->max_prob_rate = 0;
		sn->packet_count = 0;
		sn->sample_count = 0;
		sn->is_sampling = 0;

		if (rt == NULL) {
			DPRINTF(sc, ATH_DEBUG_RATE, "no rates yet! mode %u\n", sc->sc_curmode);
			return;
		}
		sn->static_rate_ndx = -1;
		if (vap->iv_maxrateindex == 0 || ni->ni_rates.rs_nrates <= 0
		    || vap->iv_maxrateindex > ni->ni_rates.rs_nrates)
			sn->num_rates = ni->ni_rates.rs_nrates;
		else
			sn->num_rates = vap->iv_maxrateindex;

		for (x = 0; x < ni->ni_rates.rs_nrates; x++) {
			sn->rs_rateattempts 	[x] = 0;
			sn->rs_thisprob 	[x] = 0;
			sn->rs_ratesuccess 	[x] = 0;
			sn->rs_lastrateattempts [x] = 0;
			sn->rs_lastratesuccess	[x] = 0;
			sn->rs_probability	[x] = 0;
			sn->rs_succ_hist	[x] = 0;
			sn->rs_att_hist 	[x] = 0;
			sn->rs_this_tp 		[x] = 0;
			if (vap->iv_minrateindex && vap->iv_minrateindex<ni->ni_rates.rs_nrates)
			{
			int idx = vap->iv_minrateindex; 
			sn->rates[x].rate = ni->ni_rates.rs_rates[idx] & IEEE80211_RATE_VAL;
			sn->rates[x].rix = sc->sc_rixmap[sn->rates[idx].rate];
			}else{
			sn->rates[x].rate = ni->ni_rates.rs_rates[x] & IEEE80211_RATE_VAL;
			sn->rates[x].rix = sc->sc_rixmap[sn->rates[x].rate];
			}
			if (sn->rates[x].rix == 0xff) {
				DPRINTF(sc, ATH_DEBUG_RATE, "%s: %s ignore bogus rix at %d\n",
					dev_info, __func__, x);
				continue;
			}
			sn->rates[x].rateCode = rt->info[sn->rates[x].rix].rateCode;
			sn->rates[x].shortPreambleRateCode =
				rt->info[sn->rates[x].rix].rateCode |
				rt->info[sn->rates[x].rix].shortPreamble;			
		}

		ath_fill_sample_table(sn);

		ni->ni_txrate = 0;

		if (sn->num_rates <= 0) {
			DPRINTF(sc, ATH_DEBUG_RATE, "%s: %s " MAC_FMT " no rates (fixed %d) \n",
				dev_info, __func__, MAC_ADDR(ni->ni_macaddr),
				vap->iv_fixed_rate);
			/* There are no rates yet; we're done */
			return;
		}

		if (vap->iv_fixed_rate != IEEE80211_FIXED_RATE_NONE) {
			srate = sn->num_rates - 1;

			/* A fixed rate is to be used; ic_fixed_rate is an
			 * index into the supported rate set.  Convert this
			 * to the index into the negotiated rate set for
			 * the node.  We know the rate is there because the
			 * rate set is checked when the station associates. */
			/* NB: the rate set is assumed sorted */
			for (; (srate > 0) && (ni->ni_rates.rs_rates[srate] & IEEE80211_RATE_VAL) != vap->iv_fixed_rate; srate--);

			sn->static_rate_ndx = srate;
			ni->ni_txrate = srate;
			if ((ni->ni_rates.rs_rates[srate] & IEEE80211_RATE_VAL) != vap->iv_fixed_rate)
				EPRINTF(sc, "Invalid static rate, falling back to basic rate\n");
			else
				DPRINTF(sc, ATH_DEBUG_RATE, "%s: %s " MAC_FMT " fixed rate %d%sMbps\n",
					dev_info, __func__, MAC_ADDR(ni->ni_macaddr),
					sn->rates[srate].rate / 2,
					(sn->rates[srate].rate % 2) ? ".5 " : " ");
			return;
		}

		for (x = 0; x < ni->ni_rates.rs_nrates; x++) {
			if (sn->rates[x].rix == 0xff) {
				DPRINTF(sc, ATH_DEBUG_RATE, "%s: %s ignore bogus rix at %d\n",
					dev_info, __func__, x);
				continue;
			}

			sn->rs_rateattempts	[x] = 0;
			sn->rs_thisprob		[x] = 0;
			sn->rs_ratesuccess 	[x] = 0;
			sn->rs_probability 	[x] = 0;
			sn->rs_lastrateattempts [x] = 0;
			sn->rs_lastratesuccess 	[x] = 0;
			sn->rs_succ_hist 	[x] = 0;
			sn->rs_att_hist 	[x] = 0;
			sn->perfect_tx_time 	[x] =
				calc_usecs_unicast_packet(sc, 1200,
							  sn->rates[x].rix,
							  0, 0);
			sn->retry_count 	[x] = 1;
			sn->retry_adjusted_count[x] = 1;

			for (retry_index = 2; retry_index < ATH_TXMAXTRY; retry_index++) {
				tx_time = calc_usecs_unicast_packet(sc, 1200, sn->rates[x].rix, 0, retry_index);
				if (tx_time > ath_segment_size)
					break;
				sn->retry_count[x] = retry_index;
				sn->retry_adjusted_count[x] = retry_index;
			}
		}

#if 0
		DPRINTF(sc, ATH_DEBUG_RATE, "%s: Retry table for this node\n", __func__);
		  for (x = 0; x < ni->ni_rates.rs_nrates; x++)
			     DPRINTF(sc, ATH_DEBUG_RATE, "%2d  %2d %6d  \n", x, sn->retry_count[x], sn->perfect_tx_time[x]);
#endif

		/* Set the initial rate */
		for (ndx = sn->num_rates - 1; ndx > 0; ndx--)
			if (sn->rates[ndx].rate <= 72)
				break;
		sn->current_rate = ndx;

		ni->ni_txrate = sn->current_rate;
}

static void
ath_rate_cb(void *arg, struct ieee80211_node *ni)
{
		ath_rate_ctl_reset(netdev_priv(ni->ni_ic->ic_dev), ni);
}

/* Reset the rate control state for each 802.11 state transition. */
static void
ath_rate_newstate(struct ieee80211vap *vap, enum ieee80211_state newstate)
{
		struct ieee80211com *ic = vap->iv_ic;

		if (newstate == IEEE80211_S_RUN) {
			if (ic->ic_opmode != IEEE80211_M_STA) {
				/* Sync rates for associated stations and neighbors. */
				ieee80211_iterate_nodes(&ic->ic_sta, ath_rate_cb, NULL);
			}
			ath_rate_newassoc(netdev_priv(ic->ic_dev), ATH_NODE(vap->iv_bss), 1);
		}
}


static void
ath_rate_statistics(struct ieee80211_node *ni)
{
		struct ath_node *an = ATH_NODE(ni);
		struct ieee80211_rateset *rs = &ni->ni_rates;
		struct minstrel_node *rn = ATH_NODE_MINSTREL(an);
		unsigned int i;
		u_int32_t p;
		u_int32_t micro_secs;
		u_int32_t max_prob,    index_max_prob;
		u_int32_t max_tp,      index_max_tp,      index_max_tp2;

		/* Calculate statistics for each date rate in the table */
		/* 'micro_secs' is the time to transmit 1200 bytes, or 9600 bits. */
		for (i = 0; i < rs->rs_nrates; i++) {
			micro_secs = rn->perfect_tx_time[i];
			if (micro_secs == 0)
				micro_secs = ONE_SECOND;

			    if (rn->rs_rateattempts[i] != 0) {
			            p = (rn->rs_ratesuccess[i] * 18000) / rn->rs_rateattempts[i];
			            rn->rs_succ_hist[i] += rn->rs_ratesuccess[i];
			            rn->rs_att_hist[i]  += rn->rs_rateattempts[i];
				rn->rs_thisprob[i] = p;
				p = ((p * (100 - ath_ewma_level)) + (rn->rs_probability[i] * ath_ewma_level)) / 100;
				rn->rs_probability[i] = p;
				rn->rs_this_tp[i] = p * (ONE_SECOND / micro_secs);
				rn->rs_lastratesuccess[i] = rn->rs_ratesuccess[i];
				rn->rs_lastrateattempts[i] = rn->rs_rateattempts[i];
			            rn->rs_ratesuccess[i] = 0;
			            rn->rs_rateattempts[i] = 0;
			} else {
				rn->rs_lastratesuccess[i] = 0;
				rn->rs_lastrateattempts[i] = 0;
			}

			/* Sample less often below the 10% chance of success.
			 * Sample less often above the 95% chance of success.
			 * 'rn->rs_probability' has a scale of 0 (0%) to 18000 (100%), which avoids rounding issues.*/
			if ((rn->rs_probability[i] > 17100) || (rn->rs_probability[i] < 1800)) {
				rn->retry_adjusted_count[i] = rn->retry_count[i] >> 1;
				if (rn->retry_adjusted_count[i] > 2)
					rn->retry_adjusted_count[i] = 2;
			} else
				rn->retry_adjusted_count[i] = rn->retry_count[i];
			if (rn->retry_adjusted_count[i] == 0)
				rn->retry_adjusted_count[i] = 1;
		}

		/* The High speed rates (e.g 54Mbps) is checked last. If
		 * throughput is the same for two rates, we prefer the
		 * lower rate, as this has a better chance of success. */
		max_prob = 0;
		index_max_prob = 0;
		max_tp = 0;
		index_max_tp  = 0;
		index_max_tp2 = 0;

		/* This code could have been moved up into the previous
		 * loop. More readable to have it here */
		for (i = 0; i < rs->rs_nrates; i++) {
			if (max_tp < rn->rs_this_tp[i]) {
				index_max_tp = i;
				max_tp = rn->rs_this_tp[i];
			}

			if (max_prob <  rn->rs_probability[i]) {
				index_max_prob = i;
				max_prob = rn->rs_probability[i];
			}
		}

		max_tp = 0;
		for (i = 0; i < rs->rs_nrates; i++) {
			if ((i != index_max_tp) && (max_tp < rn->rs_this_tp[i])) {
				index_max_tp2 = i;
				max_tp = rn->rs_this_tp[i];
			}
		}

		rn->max_tp_rate   = index_max_tp;
		rn->max_tp_rate2  = index_max_tp2;
		rn->max_prob_rate = index_max_prob;
		rn->current_rate  = index_max_tp;
		ni->ni_txrate = index_max_tp;
}

static struct ath_ratectrl *
ath_rate_attach(struct ath_softc *sc)
{
		struct minstrel_softc *osc;
		DPRINTF(sc, ATH_DEBUG_RATE, "%s: %s\n", dev_info, __func__);

		_MOD_INC_USE(THIS_MODULE, return NULL);
		osc = kmalloc(sizeof(struct minstrel_softc), GFP_ATOMIC);
		if (osc == NULL) {
			    _MOD_DEC_USE(THIS_MODULE);
			return NULL;
		}

		osc->arc.arc_space = sizeof(struct minstrel_node);
		osc->arc.arc_vap_space = 0;

 	osc->sc          = sc;
		osc->sc_dev      = sc->sc_dev;

		return &osc->arc;
}

static void
ath_rate_detach(struct ath_ratectrl *arc)
{
 	struct minstrel_softc *osc = (struct minstrel_softc *) arc;
		kfree(osc);
		_MOD_DEC_USE(THIS_MODULE);
}

#ifdef CONFIG_SYSCTL
static int
ath_proc_read_nodes(struct ieee80211vap *vap, char *buf, int space)
{
		char *p = buf;
		struct ieee80211_node *ni;
		struct ath_node *an;
		struct minstrel_node *odst;
		struct ieee80211_node_table *nt =
			    (struct ieee80211_node_table *) &vap->iv_ic->ic_sta;
		unsigned int x = 0;
		unsigned int this_tp, this_prob, this_eprob;
#ifdef AR_DEBUG
			struct ath_softc *sc = netdev_priv(vap->iv_ic->ic_dev);
#endif

		IEEE80211_NODE_TABLE_LOCK_IRQ(nt);
		TAILQ_FOREACH(ni, &nt->nt_node, ni_list) {
			/* Assume each node needs 1500 bytes */
			if ((buf + space) < (p + 1500)) {
				if ((buf + space) > (p + 100)) {
					p += sprintf(p, "out of room for node " MAC_FMT "\n\n", MAC_ADDR(ni->ni_macaddr));
					break;
				}
				DPRINTF(sc, ATH_DEBUG_RATE, "%s: out of memeory to write tall of the nodes\n", __func__);
			            break;
			}
			an = ATH_NODE(ni);
			odst = ATH_NODE_MINSTREL(an);
			/* Skip ourself */
			if (IEEE80211_ADDR_EQ(vap->iv_myaddr, ni->ni_macaddr))
			        continue;

			p += sprintf(p, "rate data for node: " MAC_FMT "\n", MAC_ADDR(ni->ni_macaddr));
			p += sprintf(p, "rate     throughput  ewma prob   this prob  this succ/attempt   success    attempts\n");
			for (x = 0; x < odst->num_rates; x++) {
				p += sprintf(p, "%s",
					     (x == odst->current_rate) ? "T" : " ");

				p += sprintf(p, "%s",
					     (x == odst->max_tp_rate2) ? "t" : " ");

				p += sprintf(p, "%s",
					     (x == odst->max_prob_rate) ? "P" : " ");

				p += sprintf(p, "%3u%s",
					     odst->rates[x].rate / 2,
					     (odst->rates[x].rate & 0x1) != 0 ? ".5" : "  ");

				this_tp = ((odst->rs_this_tp[x] / 18000) * 96) >> 10;
				this_prob = odst->rs_thisprob[x] / 18;
				this_eprob = odst->rs_probability[x] / 18;
				p += sprintf(p, "  %6u.%1u   %6u.%1u   %6u.%1u        %3u(%3u)   %8llu    %8llu\n",
					     this_tp / 10, this_tp % 10,
					     this_eprob / 10, this_eprob % 10,
					     this_prob / 10, this_prob % 10,
					     odst->rs_lastratesuccess[x],
					     odst->rs_lastrateattempts[x],
					     (unsigned long long)odst->rs_succ_hist[x],
					     (unsigned long long)odst->rs_att_hist[x]);
			}
			p += sprintf(p, "\n");

			p += sprintf(p, "Total packet count::    ideal %d      lookaround %d\n\n", odst->packet_count, odst->sample_count);
		}
		IEEE80211_NODE_TABLE_UNLOCK_IRQ(nt);

		return (p - buf);
}

static int
ath_proc_ratesample_open(struct inode *inode, struct file *file)
{
		struct proc_ieee80211_priv *pv = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
		struct proc_dir_entry *dp = PDE(inode);
		struct ieee80211vap *vap = dp->data;
#else
		struct ieee80211vap *vap = (struct ieee80211vap *)PDE_DATA(inode);
#endif
		if (!(file->private_data = kmalloc(sizeof(struct proc_ieee80211_priv),
			                            GFP_KERNEL)))
			    return -ENOMEM;

		/* Initially allocate both read and write buffers */
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

		/* Now read the data into the buffer */
		pv->rlen = ath_proc_read_nodes(vap, pv->rbuf, MAX_PROC_IEEE80211_SIZE);
		return 0;
}

static struct file_operations ath_proc_ratesample_ops = {
		.read = NULL,
		.write = NULL,
		.open = ath_proc_ratesample_open,
		.release = NULL,
};

static void
ath_rate_dynamic_proc_register(struct ieee80211vap *vap)
{
		/* Create proc entries for the rate control algorithm */
		ieee80211_proc_vcreate(vap, &ath_proc_ratesample_ops, "rate_info");
}

#endif /* CONFIG_SYSCTL */

static struct ieee80211_rate_ops ath_rate_ops = {
		.ratectl_id = IEEE80211_RATE_MINSTREL,
		.node_init = ath_rate_node_init,
		.node_cleanup = ath_rate_node_cleanup,
		.findrate = ath_rate_findrate,
		.get_mrr = ath_rate_get_mrr,
		.tx_complete = ath_rate_tx_complete,
		.newassoc = ath_rate_newassoc,
		.newstate = ath_rate_newstate,
		.attach = ath_rate_attach,
		.detach = ath_rate_detach,
		.dynamic_proc_register = ath_rate_dynamic_proc_register,
};

#include <net80211/module.h>

MODULE_AUTHOR("John Bicket/Derek Smithies");
MODULE_DESCRIPTION("Minstrel Rate bit-rate selection algorithm for Atheros devices");
#ifdef MODULE_VERSION
MODULE_VERSION(RELEASE_VERSION);
#endif
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static int __init ath_rate_minstrel_init(void)
{
		printk(KERN_INFO "%s: Minstrel automatic rate control "
		       "algorithm %s\n", dev_info, version);
		printk(KERN_INFO "%s: look around rate set to %d%%\n",
		       dev_info, ath_lookaround_rate);
		printk(KERN_INFO "%s: EWMA rolloff level set to %d%%\n",
		       dev_info, ath_ewma_level);
		printk(KERN_INFO "%s: max segment size in the mrr set "
		       "to %d us\n", dev_info, ath_segment_size);
		return ieee80211_rate_register(&ath_rate_ops);
}
module_init(ath_rate_minstrel_init);

static void __exit ath_rate_minstrel_exit(void)
{
		ieee80211_rate_unregister(&ath_rate_ops);
		printk(KERN_INFO "%s: unloaded\n", dev_info);
}
module_exit(ath_rate_minstrel_exit);

/* The comment below is magic for those who use emacs to edit this file. */
/* With the comment below, the tab key does auto indent to 8 spaces.     */

/*
 * Local Variables:
 * mode:c
 * c-file-style:linux
 * c-basic-offset:8
 * End:
 */
