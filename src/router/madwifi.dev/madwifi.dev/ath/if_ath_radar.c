/*
 * This software is distributed under the terms of the
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
 * $Id: if_ath_radar.c 2464 2007-06-15 22:51:56Z mtaylor $
 */
#include "opt_ah.h"
#include "if_ath_debug.h"

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/cache.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/param.h>

#include "if_ethersubr.h"	/* for ETHER_IS_MULTICAST */
#include "if_media.h"
#include "if_llc.h"

#include <net80211/ieee80211_radiotap.h>
#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_monitor.h>
#include <net80211/ieee80211_rate.h>

#ifdef USE_HEADERLEN_RESV
#include <net80211/if_llc.h>
#endif

#include "net80211/if_athproto.h"
#include "if_athvar.h"

#include "ah_desc.h"

#include "ah_devid.h"		/* XXX to identify chipset */

#ifdef ATH_PCI			/* PCI BUS */
#include "if_ath_pci.h"
#endif				/* PCI BUS */
#ifdef ATH_AHB			/* AHB BUS */
#include "if_ath_ahb.h"
#endif				/* AHB BUS */

#undef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#include "ah.h"
#include "if_ath_hal.h"

#ifdef ATH_TX99_DIAG
#include "ath_tx99.h"
#endif

#include "ah_os.h"
#include "if_ath_radar.h"

#define sizetab(t) (sizeof(t)/sizeof(t[0]))
#define nofloat_pct(_value, _pct) \
	( (_value * (1000 + _pct)) / 1000 )

#ifndef list_for_each_entry_reverse
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_entry((head)->prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head); 	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))
#endif

struct radar_pattern_specification {
	/* The name of the rule/specification (i.e. what did we detect) */
	const char *name;
	/* Interval MIN = 1000000 / FREQ - 2% 
	 * (a.k.a. Pulse/Burst Repetition Interval) */
	u_int32_t min_rep_int;
	/* Interval MAX = 1000000 / FREQ + 2% 
	 * (a.k.a. Pulse/Burst Repetition Interval) */
	u_int32_t max_rep_int;
	/* Do we adjust the min/max interval values dynamically 
	 * based upon running mean interval? */
	HAL_BOOL dyn_ints;
	/* Fuzz factor dynamic matching, as unsigned integer percentage 
	 * of variation (i.e. 2 for +/- 2% timing) */
	u_int32_t fuzz_pct;
	/* Match MIN (Minimum Pulse/Burst events required) */
	u_int32_t min_pulse;
	/* Match MIN duration (Minimum Pulse/Burst events 
	 * required including missed) */
	u_int32_t min_evts;
	/* Match MAX duration (Maximum Pulse/Burst events 
	 * required including missed) */
	u_int32_t max_evts;
	/* Maximum consecutive missing pulses */
	u_int32_t max_consecutive_missing;
	/* Maximum missing pulses */
	u_int32_t max_missing;
	/* Match on absolute distance to PRI/PRF midpoint */
	HAL_BOOL match_midpoint;
};

static struct radar_pattern_specification radar_patterns[] = {
#ifdef DFS_DOMAIN_ETSI
	{ "ETSI [ 200]", 4900, 5100, AH_FALSE, 20, 3, 4, 10, 4, 8, AH_TRUE },
	{ "ETSI [ 300]", 3267, 3399, AH_FALSE, 20, 3, 4, 10, 4, 6, AH_TRUE },
	{ "ETSI [ 500]", 1960, 2040, AH_FALSE, 20, 4, 4, 10, 4, 8, AH_TRUE },
	{ "ETSI [ 750]", 1307, 1359, AH_FALSE, 20, 5, 4, 15, 4, 13, AH_TRUE },
	{ "ETSI [ 800]", 1225, 1275, AH_FALSE, 20, 4, 4, 10, 4, 8, AH_TRUE },
	{ "ETSI [1000]", 980, 1020, AH_FALSE, 20, 4, 4, 10, 4, 8, AH_TRUE },
	{ "ETSI [1200]", 817, 849, AH_FALSE, 20, 5, 4, 15, 4, 13, AH_TRUE },
	{ "ETSI [1500]", 653, 679, AH_FALSE, 20, 5, 4, 15, 4, 6, AH_TRUE },
	{ "ETSI [1600]", 613, 637, AH_FALSE, 20, 5, 4, 15, 4, 7, AH_TRUE },
	{ "ETSI [2000]", 490, 510, AH_FALSE, 20, 7, 4, 20, 4, 10, AH_TRUE },
	{ "ETSI [2300]", 426, 442, AH_FALSE, 20, 9, 4, 25, 6, 20, AH_TRUE },
	{ "ETSI [3000]", 327, 339, AH_FALSE, 20, 7, 4, 20, 5, 20, AH_TRUE },
	{ "ETSI [3500]", 280, 290, AH_FALSE, 20, 9, 4, 25, 2, 20, AH_TRUE },
	{ "ETSI [4000]", 245, 255, AH_FALSE, 20, 7, 4, 20, 5, 20, AH_TRUE },
#endif
#ifdef DFS_DOMAIN_FCC
	{ "FCC [1,1399-1714]", 1399, 1714, AH_TRUE, 10, 5, 10, 18, 4, 6, AH_FALSE },
	{ "FCC [2,147-235]", 147, 235, AH_TRUE, 10, 8, 10, 29, 6, 12, AH_FALSE },
	{ "FCC [3-4,196-273]", 196, 273, AH_TRUE, 10, 8, 8, 18, 2, 16, AH_FALSE },
	{ "FCC [3-4,275-352]", 275, 352, AH_TRUE, 10, 8, 8, 18, 2, 16, AH_FALSE },
	{ "FCC [3-4,354-431]", 354, 431, AH_TRUE, 10, 8, 8, 18, 2, 16, AH_FALSE },
	{ "FCC [3-4,433-510]", 433, 510, AH_TRUE, 10, 8, 8, 18, 2, 16, AH_FALSE },
	{ "FCC [3-4,235-313]", 235, 313, AH_TRUE, 10, 8, 8, 18, 2, 16, AH_FALSE },
	{ "FCC [3-4,314-392]", 314, 392, AH_TRUE, 10, 8, 8, 18, 2, 16, AH_FALSE },
	{ "FCC [3-4,393-471]", 393, 471, AH_TRUE, 10, 8, 8, 18, 2, 16, AH_FALSE }
#endif
};

#ifdef AR_DEBUG
static u_int32_t interval_to_frequency(u_int32_t pri);
#endif

/* Returns true if radar detection is enabled. */
int ath_radar_is_enabled(struct ath_softc *sc)
{
	struct ath_hal *ah = sc->sc_ah;
	if (ar_device(sc->devid) >= 5211)
		return ((OS_REG_READ(ah, AR5K_AR5212_PHY_ERR_FIL) & AR5K_AR5212_PHY_ERR_FIL_RADAR) && (sc->sc_imask & HAL_INT_RXPHY) && (ath_hal_intrget(ah) & HAL_INT_RXPHY));
	else
		return ((sc->sc_imask & HAL_INT_RXPHY) && (ath_hal_intrget(ah) & HAL_INT_RXPHY));
	return 0;
}

/* Read the radar pulse detection parameters. */
void ath_radar_get_params(struct ath_softc *sc, RADAR_PARAM * rp)
{
	u_int32_t radar = ath_reg_read(sc, AR5K_PHY_RADAR);
	rp->rp_fir_filter_output_power_thr = (radar & AR5K_PHY_RADAR_FIRPWROUTTHR) >> AR5K_PHY_RADAR_FIRPWROUTTHR_S;
	rp->rp_radar_rssi_thr = (radar & AR5K_PHY_RADAR_PULSERSSITHR) >> AR5K_PHY_RADAR_PULSERSSITHR_S;
	rp->rp_pulse_height_thr = (radar & AR5K_PHY_RADAR_PULSEHEIGHTTHR) >> AR5K_PHY_RADAR_PULSEHEIGHTTHR_S;
	rp->rp_pulse_rssi_thr = (radar & AR5K_PHY_RADAR_RADARRSSITHR) >> AR5K_PHY_RADAR_RADARRSSITHR_S;
	rp->rp_inband_thr = (radar & AR5K_PHY_RADAR_INBANDTHR) >> AR5K_PHY_RADAR_INBANDTHR_S;
}

/* Update the radar pulse detection parameters. 
 * If rp is NULL, defaults are used for all fields.
 * If any member of rp is set to RADAR_PARAM_USE_DEFAULT, the default
 * is used for that field. */
void ath_radar_set_params(struct ath_softc *sc, RADAR_PARAM * rp)
{
#define BUILD_PHY_RADAR_FIELD(_MASK,_SHIFT,_FIELD) \
	((NULL == rp || (rp->_FIELD == RADAR_PARAM_USE_DEFAULT)) ? \
		((AR5K_PHY_RADAR_ENABLED_AR5213 & (_MASK))) : \
		((rp->_FIELD << (_SHIFT)) & (_MASK)))
	ath_reg_write(sc, AR5K_PHY_RADAR,
		      BUILD_PHY_RADAR_FIELD(AR5K_PHY_RADAR_FIRPWROUTTHR,
					    AR5K_PHY_RADAR_FIRPWROUTTHR_S,
					    rp_fir_filter_output_power_thr) |
		      BUILD_PHY_RADAR_FIELD(AR5K_PHY_RADAR_RADARRSSITHR,
					    AR5K_PHY_RADAR_RADARRSSITHR_S,
					    rp_pulse_rssi_thr) |
		      BUILD_PHY_RADAR_FIELD(AR5K_PHY_RADAR_PULSEHEIGHTTHR,
					    AR5K_PHY_RADAR_PULSEHEIGHTTHR_S,
					    rp_pulse_height_thr) |
		      BUILD_PHY_RADAR_FIELD(AR5K_PHY_RADAR_PULSERSSITHR, AR5K_PHY_RADAR_PULSERSSITHR_S, rp_radar_rssi_thr) | BUILD_PHY_RADAR_FIELD(AR5K_PHY_RADAR_INBANDTHR, AR5K_PHY_RADAR_INBANDTHR_S, rp_inband_thr)
	    );
#undef BUILD_PHY_RADAR_FIELD
}

/* This is called on channel change to enable radar detection for 5211+ chips.  
 * NOTE: AR5210 doesn't have radar pulse detection support. */
int ath_radar_update(struct ath_softc *sc)
{

	struct ath_hal *ah = sc->sc_ah;
#ifdef AR_DEBUG
	struct net_device *dev = sc->sc_dev;
#endif
	struct ieee80211com *ic = &sc->sc_ic;
	int required = 0;

	/* Do not attempt to change radar state when bg scanning is
	 * the cause */
	if (ic->ic_flags & IEEE80211_F_SCAN)
		return 1;

	/* Update the DFS flags (as a sanity check) */
	required = ath_radar_is_dfs_required(sc, &sc->sc_curchan);
	/* configure radar pulse detector register using default values, but do
	 * not toggle the enable bit.  XXX: allow tweaking?? */
	ath_radar_set_params(sc, NULL);
	if (ar_device(sc->devid) >= 5211) {
		HAL_INT old_ier = ath_hal_intrget(ah);
		HAL_INT new_ier = old_ier;
		unsigned int old_radar = OS_REG_READ(ah, AR5K_PHY_RADAR);
		unsigned int old_filter = OS_REG_READ(ah, AR5K_AR5212_PHY_ERR_FIL);
		unsigned int old_rxfilt = ath_hal_getrxfilter(ah);
		unsigned int old_mask = sc->sc_imask;
		unsigned int new_radar = old_radar;
		unsigned int new_filter = old_filter;
		unsigned int new_mask = old_mask;
		unsigned int new_rxfilt = old_rxfilt;

		ath_hal_intrset(ah, old_ier & ~HAL_INT_GLOBAL);
		if ((required) && (ic->ic_flags & IEEE80211_F_DOTH)) {
			new_radar |= AR5K_PHY_RADAR_ENABLE;
			new_filter |= AR5K_AR5212_PHY_ERR_FIL_RADAR;
			new_rxfilt |= (HAL_RX_FILTER_PHYERR | HAL_RX_FILTER_PHYRADAR);
			new_mask |= HAL_INT_RXPHY;
			new_ier |= HAL_INT_RXPHY;
		} else {
			new_radar &= ~AR5K_PHY_RADAR_ENABLE;
			new_filter &= ~AR5K_AR5212_PHY_ERR_FIL_RADAR;
			new_rxfilt &= ~HAL_RX_FILTER_PHYRADAR;
			new_mask &= ~HAL_INT_RXPHY;
			new_ier &= ~HAL_INT_RXPHY;
		}

		if (old_filter != new_filter)
			OS_REG_WRITE(ah, AR5K_AR5212_PHY_ERR_FIL, new_filter);
		if (old_radar != new_radar)
			OS_REG_WRITE(ah, AR5K_PHY_RADAR, new_radar);
		if (old_rxfilt != new_rxfilt)
			ath_hal_setrxfilter(ah, new_rxfilt);

		sc->sc_imask = new_mask;
		if (DFLAG_ISSET(sc, ATH_DEBUG_DOTH) && ((old_radar != new_radar) || (old_filter != new_filter) || (old_rxfilt != new_rxfilt) || (old_mask != new_mask) || (old_ier != new_ier))) {
			DPRINTF(sc, ATH_DEBUG_DOTH, "%s: %s: Radar detection %s.\n", DEV_NAME(dev), __func__, required ? "enabled" : "disabled");
		}
		ath_hal_intrset(ah, new_ier);
	}

	return (required == ath_radar_is_enabled(sc));
}

/* Returns true if DFS is required for the regulatory domain, country and 
 * combination in use. 
 * XXX: Need to add regulatory rules in here.  This is too conservative! */
int ath_radar_is_dfs_required(struct ath_softc *sc, HAL_CHANNEL *hchan)
{
	/* For FCC: 5250 to 5350MHz (channel 52 to 60) and for Europe added 
	 * 5470 to 5725 MHz (channel 100 to 140). 
	 * Being conservative, go with the entire band from 5250-5725 MHz. */
	return (hchan->privFlags & CHANNEL_DFS) ? 1 : 0;
}

static struct ath_rp *pulse_head(struct ath_softc *sc)
{
	return list_entry(sc->sc_rp_list.next, struct ath_rp, list);
}

static struct ath_rp *pulse_tail(struct ath_softc *sc)
{
	return list_entry(sc->sc_rp_list.prev, struct ath_rp, list);
}

static struct ath_rp *pulse_prev(struct ath_rp *pulse)
{
	return list_entry(pulse->list.prev, struct ath_rp, list);
}

#define CR_FALLTHROUGH		0
#define CR_NULL			1
#define CR_EXCESS_INTERVALS	2
#define CR_INTERVALS 		3
#define CR_EXCESS_DURATION	4
#define CR_DURATION		5
#define CR_PULSES		6
#define CR_MISSES		7
#define CR_MIDPOINT_A		8
#define CR_MIDPOINT_B		9
#define CR_MIDPOINT_C		10
#define CR_NOISE		11

#define MR_MATCH 		0
#define MR_FAIL_MIN_INTERVALS	1
#define MR_FAIL_REQD_MATCHES	2
#define MR_FAIL_MAX_MISSES	3
#define MR_FAIL_MIN_PERIOD	4
#define MR_FAIL_MAX_PERIOD	5

#ifdef AR_DEBUG
static const char *get_match_result_desc(u_int32_t code)
{
	switch (code) {
	case MR_MATCH:
		return "MATCH";
	case MR_FAIL_MIN_INTERVALS:
		return "TOO-SHORT";
	case MR_FAIL_REQD_MATCHES:
		return "TOO-FEW";
	case MR_FAIL_MAX_MISSES:
		return "TOO-LOSSY";
	case MR_FAIL_MIN_PERIOD:
		return "PRI<MIN";
	case MR_FAIL_MAX_PERIOD:
		return "PRI>MAX";
	default:
		return "unknown";
	}
}
#endif

static int32_t match_radar(u_int32_t matched,
			   u_int32_t missed, u_int32_t mean_period, u_int32_t noise, u_int32_t min_evts, u_int32_t max_evts, u_int32_t min_rep_int, u_int32_t max_rep_int, u_int32_t min_pulse, u_int32_t max_misses)
{
	/* Not a match: insufficient overall burst length */
	if ((matched + missed) < min_evts)
		return MR_FAIL_MIN_INTERVALS;

	/* Not a match: insufficient match count */
	if (matched < min_pulse)
		return MR_FAIL_REQD_MATCHES;

	/* Not a match: too many missies */
	if (missed > max_misses)
		return MR_FAIL_MAX_MISSES;

	/* Not a match, PRI out of range */
	if (mean_period < min_rep_int)
		return MR_FAIL_MIN_PERIOD;

	/* Not a match, PRI out of range */
	if (mean_period > max_rep_int)
		return MR_FAIL_MAX_PERIOD;

	return MR_MATCH;
}

static int32_t compare_radar_matches(int32_t a_matched,
				     int32_t a_missed,
				     int32_t a_mean_period,
				     int32_t a_noise,
				     int32_t a_min_evts,
				     int32_t a_max_evts,
				     int32_t a_min_rep_int,
				     int32_t a_max_rep_int,
				     int32_t a_min_pulse,
				     int32_t a_max_misses,
				     HAL_BOOL a_match_midpoint,
				     int32_t b_matched,
				     int32_t b_missed,
				     int32_t b_mean_period,
				     int32_t b_noise, int32_t b_min_evts, int32_t b_max_evts, int32_t b_min_rep_int, int32_t b_max_rep_int, int32_t b_min_pulse, int32_t b_max_misses, HAL_BOOL b_match_midpoint)
{
	/* Intermediate calculations */
	int32_t a_total = a_matched + a_missed;
	int32_t b_total = b_matched + b_missed;
	int32_t a_excess_total = MAX((int32_t) (a_total - (int32_t) a_max_evts), 0);
	int32_t b_excess_total = MAX((int32_t) (b_total - (int32_t) b_max_evts), 0);
	u_int64_t a_duration = a_total * a_mean_period;
	u_int64_t b_duration = b_total * b_mean_period;
	u_int64_t a_excess_duration = a_excess_total * a_mean_period;
	u_int64_t b_excess_duration = b_excess_total * b_mean_period;
	u_int64_t a_dist_from_pri_mid = labs(a_mean_period - (a_min_rep_int + ((a_max_rep_int - a_min_rep_int) / 2)));
	u_int64_t b_dist_from_pri_mid = labs(b_mean_period - (b_min_rep_int + ((b_max_rep_int - b_min_rep_int) / 2)));
	/* Did one radar have fewer excess total pulse intervals than the 
	 * other? */
	if (a_excess_total != b_excess_total)
		return ((a_excess_total < b_excess_total) ? 1 : -1) * CR_EXCESS_INTERVALS;
	/* Was one pulse longer chronologically, even though totals matched? */
	else if (a_excess_duration != b_excess_duration)
		return ((a_excess_duration < b_excess_duration) ? 1 : -1) * CR_EXCESS_DURATION;
	/* Did one get more matches? */
	if (a_matched != b_matched)
		return (a_matched > b_matched ? 1 : -1) * CR_PULSES;
	/* Both waveforms are the same length, same total. 
	 * Did one get more misses? */
	if (a_missed != b_missed)
		return (a_missed < b_missed ? 1 : -1) * CR_MISSES;
	/* Did one get more noise? */
	if (a_noise != b_noise)
		return (a_noise < b_noise ? 1 : -1) * CR_NOISE;
	/* If both waveforms were not too long in terms of intervals */
	if (0 == (a_excess_total + b_excess_total)) {
		/* Did one waveform have to match more events than the other? */
		if (a_total != b_total)
			return ((a_total > b_total) ? 1 : -1) * CR_INTERVALS;
		/* Was one waveform longer than the other */
		if (a_duration != b_duration)
			return ((a_duration > b_duration) ? 1 : -1) * CR_DURATION;
	}
	/* both durations are legal, but one is closer to the original PRF/PRI */
	if (a_dist_from_pri_mid != b_dist_from_pri_mid) {
		if (a_match_midpoint && b_match_midpoint) {
			/* Which pattern is closer to midpoint? */
			return ((a_dist_from_pri_mid < b_dist_from_pri_mid) ? 1 : -1) * CR_MIDPOINT_A;
		} else if (a_match_midpoint) {
			/* If not within spitting distance of midpoint, reject */
			return ((a_dist_from_pri_mid < 3) ? 1 : -1) * CR_MIDPOINT_B;

		} else if (b_match_midpoint) {
			/* If not within spitting distance of midpoint, reject */
			return ((b_dist_from_pri_mid >= 3) ? 1 : -1) * CR_MIDPOINT_C;
		}
	}
	return -CR_FALLTHROUGH;
}

#ifdef ATH_RADAR_LONG_PULSE

struct lp_burst {
	u_int32_t lpb_num_pulses;
	u_int32_t lpb_num_noise;
	u_int32_t lpb_tsf_delta;
	u_int64_t lpb_tsf_rel;
	u_int64_t lpb_min_possible_tsf;	/* noise vs real pulses */
	u_int64_t lpb_max_possible_tsf;	/* noise vs real pulses */
};

static const u_int32_t LP_MIN_BC = 8;
static const u_int32_t LP_MAX_BC = 20;
static const u_int32_t LP_NUM_BC = 13;	/* (LP_MAX_BC - LP_MIN_BC + 1); */
static const u_int64_t LP_TSF_FUZZ_US = 32768;	/* (1<<15) because rs_tstamp 
						 * rollover errors */
static const u_int32_t LP_MIN_PRI = 1000;
static const u_int32_t LP_MAX_PRI = 2000;

static void rp_analyze_long_pulse_bscan(struct ath_softc *sc, struct ath_rp *last_pulse, u_int32_t *num_bursts, size_t bursts_buflen, struct lp_burst *bursts)
{
	int i = 0;
	struct ath_rp *newer = NULL;
	struct ath_rp *cur = last_pulse;
	struct ath_rp *older = pulse_prev(last_pulse);
	u_int32_t waveform_num_bursts = 0;

	if (num_bursts)
		*num_bursts = 0;

	for (;;) {
		/* check if we are at the end of the list */
		if (&cur->list == &sc->sc_rp_head)
			break;
		if (!cur->rp_allocated)
			break;

		if (NULL != newer) {
			u_int64_t tsf_delta = 0;
			u_int64_t tsf_adjustment = 0;

			/* Figure out TSF delta, taking into account
			 * up to one multiple of (1<<15) of clock jitter 
			 * due to interrupt latency */
			tsf_delta = newer->rp_tsf - cur->rp_tsf;
			if ((tsf_delta - LP_TSF_FUZZ_US) >= LP_MIN_PRI && (tsf_delta - LP_TSF_FUZZ_US) <= LP_MAX_PRI) {
				tsf_adjustment = LP_TSF_FUZZ_US;
				tsf_delta -= tsf_adjustment;
			}

			/* If we are in range for pulse, assume it is a pulse */
			if ((tsf_delta >= LP_MIN_PRI) && (tsf_delta <= LP_MAX_PRI)) {
				bursts[waveform_num_bursts].lpb_num_pulses++;
				bursts[waveform_num_bursts].lpb_min_possible_tsf = cur->rp_tsf - tsf_adjustment;
			} else if (tsf_delta < LP_MIN_PRI) {
				bursts[waveform_num_bursts].lpb_num_noise++;
				/* It may have been THE pulse after all... */
				bursts[waveform_num_bursts].lpb_min_possible_tsf = cur->rp_tsf - tsf_adjustment;
			} else {	/* tsf_delta > LP_MAX_PRI */
				bursts[waveform_num_bursts].lpb_num_pulses++;
				bursts[waveform_num_bursts].lpb_min_possible_tsf = cur->rp_tsf;
				/* Do not overrun bursts_buflen */
				if ((waveform_num_bursts + 1) >= bursts_buflen) {
					break;
				}
				waveform_num_bursts++;
				bursts[waveform_num_bursts].lpb_tsf_delta = tsf_delta;
				bursts[waveform_num_bursts].lpb_min_possible_tsf = cur->rp_tsf;
				bursts[waveform_num_bursts].lpb_max_possible_tsf = cur->rp_tsf;
			}
		} else {
			bursts[waveform_num_bursts].lpb_max_possible_tsf = cur->rp_tsf;
		}

		/* advance to next pulse */
		newer = cur;
		cur = pulse_prev(cur);
		older = pulse_prev(cur);
	}
	if (num_bursts) {
		bursts[waveform_num_bursts].lpb_num_pulses++;
		waveform_num_bursts++;
		*num_bursts = waveform_num_bursts;
	}
	for (i = 0; i < waveform_num_bursts; i++)
		bursts[i].lpb_tsf_rel = bursts[i].lpb_max_possible_tsf - bursts[waveform_num_bursts - 1].lpb_min_possible_tsf;
}

static HAL_BOOL rp_analyze_long_pulse(struct ath_softc *sc, struct ath_rp *last_pulse, u_int32_t *bc, u_int32_t *matched, u_int32_t *missed, u_int32_t *noise, u_int32_t *pulses)
{
	int i;
	int32_t found_radar = 0;
	int32_t found_burst_count = 0;
	int32_t matching_burst_count = 0;
	u_int32_t best_bc = 0;
	u_int32_t best_matched = 0;
	u_int32_t best_missed = 0;
	u_int32_t best_noise = 0;
	u_int32_t best_pulses = 0;

	struct lp_burst bursts[LP_MAX_BC];
	memset(&bursts, 0, sizeof(bursts));

	if (bc)
		*bc = 0;
	if (matched)
		*matched = 0;
	if (missed)
		*missed = 0;
	if (noise)
		*noise = 0;

	rp_analyze_long_pulse_bscan(sc, last_pulse, &found_burst_count, LP_MAX_BC, &bursts[0]);
	/* Find the matches */
	for (matching_burst_count = LP_MAX_BC; matching_burst_count >= LP_MIN_BC; matching_burst_count--) {
		int32_t first_matched_index = -1;
		int32_t last_matched_index = -1;
		int32_t match_burst_index = 0;
		int32_t found_burst_index = 0;
		int32_t burst_period = (12000000 / matching_burst_count);
		int32_t waveform_offset = 0;
		int32_t total_big_gaps = 0;
		int32_t matched_span = 0;
		int32_t missed_bursts = 0;
		int32_t matched_bursts = 0;
		for (i = 0; i < matching_burst_count; i++) {
			int32_t d = bursts[i].lpb_tsf_delta;
			while (d >= burst_period)
				d -= burst_period;
			total_big_gaps += d;
			waveform_offset = MAX(waveform_offset, d);
		}
		waveform_offset *= -1;

		found_burst_index = 0;
		for (match_burst_index = 0; match_burst_index < matching_burst_count; match_burst_index++) {
			int64_t limit_high = (burst_period * (matching_burst_count - 1 - match_burst_index + 1)) + (2 * LP_TSF_FUZZ_US);
			int64_t limit_low = (burst_period * (matching_burst_count - 1 - match_burst_index)) - (2 * LP_TSF_FUZZ_US);
			/* If the burst is too old, skip it... it's noise too... */
			while ((((int64_t) bursts[found_burst_index].lpb_tsf_rel + waveform_offset) > limit_high)) {
				if (found_burst_index < (found_burst_count - 1))
					found_burst_index++;
				else
					break;
			}
			if ((((int64_t) bursts[found_burst_index].lpb_tsf_rel + waveform_offset) <= limit_high) && (((int64_t) bursts[found_burst_index].lpb_tsf_rel + waveform_offset) >= limit_low)) {
				if (-1 == first_matched_index) {
					first_matched_index = match_burst_index;
					matched_span = 1;
				}
				if (last_matched_index < match_burst_index) {
					last_matched_index = match_burst_index;
				}
				DPRINTF(sc, ATH_DEBUG_DOTHFILT,
					"LP %2dp] [%2d/%2d] %10lld "
					"in {%lld:%lld}] PASS\n", matching_burst_count, found_burst_index, match_burst_index, (int64_t) bursts[found_burst_index].lpb_tsf_rel - waveform_offset, limit_low, limit_high);
				matched_bursts++;
				found_burst_index++;
			} else {
				DPRINTF(sc, ATH_DEBUG_DOTHFILT,
					"LP %2dp] [%2d/%2d] %10lld "
					"in {%lld:%lld}] MISSED\n", matching_burst_count, match_burst_index, found_burst_index, (int64_t) bursts[found_burst_index].lpb_tsf_rel - waveform_offset, limit_low, limit_high);
				missed_bursts++;
			}
		}
		matched_span = last_matched_index - first_matched_index;
		DPRINTF(sc, ATH_DEBUG_DOTHFILT, "LP %2dp] burst_period=%10d, "
			"waveform_offset=%10d, matches=%2d/%2d, "
			"result=%s\n", matching_burst_count, burst_period, waveform_offset, matched_span, matching_burst_count, (matching_burst_count == matched_span) ? "MATCH" : "MISMATCH");
		/* XXX - Add comparison logic rather than taking first/last
		 * match based upon ATH_DEBUG_DOTHFILTNOSC? */
		if (matched_span >= (matching_burst_count - 4)) {
			found_radar++;
			best_bc = matching_burst_count;
			best_matched = matched_bursts;
			best_missed = missed_bursts;
			best_noise = 0;
			best_pulses = 0;
			for (i = 0; i <= found_burst_index; i++) {
				best_noise += bursts[match_burst_index].lpb_num_noise;
				best_pulses += bursts[match_burst_index].lpb_num_pulses;
			}
			if (!DFLAG_ISSET(sc, ATH_DEBUG_DOTHFILTNOSC))
				break;
		}
	}

	if (bc)
		*bc = best_bc;
	if (matched)
		*matched = best_matched;
	if (missed)
		*missed = best_missed;
	if (noise)
		*noise = best_noise;
	if (pulses)
		*pulses = best_pulses;

	return found_radar ? AH_TRUE : AH_FALSE;
}
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */

static HAL_BOOL rp_analyse_short_pulse(struct ath_softc *sc, struct ath_rp *last_pulse, u_int32_t *index, u_int32_t *pri, u_int32_t *matching_pulses, u_int32_t *missed_pulses, u_int32_t *noise_pulses)
{
#ifdef AR_DEBUG
	struct net_device *dev = sc->sc_dev;
#endif
	int i;
	int best_index = -1;
	unsigned int best_matched = 0;
	unsigned int best_noise = 0;
	unsigned int best_missed = 0;
	unsigned int best_pri = 0;
	unsigned int best_cr = 0;

	u_int64_t t0_min, t0_max, t1, t_min, t_max;
	u_int32_t noise = 0, matched = 0, missed = 0, partial_miss = 0;
	struct ath_rp *pulse;
	u_int32_t pulse_count_minimum = 0;
	struct radar_pattern_specification *pattern = NULL;
	struct radar_pattern_specification *best_pattern = NULL;

	u_int32_t new_period = 0;
	u_int64_t last_tsf = 0;
	u_int32_t last_seen_period = 0;
	u_int32_t sum_periods = 0;
	u_int32_t mean_period = 0;
	u_int32_t adjusted_max_rep_int = 0;
	u_int32_t adjusted_min_rep_int = 0;

	if (index)
		*index = 0;
	if (pri)
		*pri = 0;
	if (noise_pulses)
		*noise_pulses = 0;
	if (matching_pulses)
		*matching_pulses = 0;
	if (missed_pulses)
		*missed_pulses = 0;

	/* we need at least sc_rp_min (>2 pulses) */
	pulse_count_minimum = sc->sc_rp_min;
	if ((sc->sc_rp_num < pulse_count_minimum) || (sc->sc_rp_num < 2))
		return 0;

	/* Search algorithm:
	 *
	 * - since we have a limited and known number of radar patterns, we
	 *   loop on all possible radar pulse period
	 *
	 * - we start the search from the last timestamp (t1), going
	 *   backward in time, up to the point for the first possible radar
	 *   pulse, ie t0_min - PERIOD * BURST_MAX
	 *
	 * - on this timescale, we matched the number of hit/missed using T -
	 *   PERIOD*n taking into account the 2% error margin (using
	 *   min_rep_int, max_rep_int)
	 *
	 * At the end, we have a number of pulse hit for each PRF
	 *
	 * TSF will roll over after just over 584,542 years of operation
	 * without restart.
	 * 
	 * This exceeds all known Atheros MTBF so, forget about TSF roll over.
	 */

	/* t1 is the timestamp of the last radar pulse */
	t1 = (u_int64_t)last_pulse->rp_tsf;

	/* loop through all patterns */
	for (i = 0; i < sizetab(radar_patterns); i++) {
		pattern = &radar_patterns[i];

		/* underflow */
		if ((pattern->min_rep_int * (pattern->min_evts - 1)) >= t1) {
			DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
				"%s: %s skipped (last pulse isn't old enough to"
				" match this pattern).  %10llu >= %10llu.\n", DEV_NAME(dev), pattern->name, (u_int64_t)(pattern->min_rep_int * pattern->min_evts), (u_int64_t)t1);
			continue;
		}

		/* this min formula is to check for underflow.  It's the
		 * minimum needed duration to gather specified number of
		 * matches, assuming minimum match interval. */
		t0_min = (pattern->min_rep_int * pattern->min_evts) < t1 ? t1 - (pattern->min_rep_int * pattern->min_evts) : 0;

		/* this max formula is to stop when we exceed maximum time
		 * period for the pattern.  It's the oldest possible TSF that
		 * could match. */
		t0_max = (pattern->max_rep_int * pattern->max_evts) < t1 ? t1 - (pattern->max_rep_int * pattern->max_evts) : 0;

		/* we directly start with the timestamp before t1 */
		pulse = pulse_prev(last_pulse);

		/* initial values for t_min, t_max */
		t_min = pattern->max_rep_int < t1 ? t1 - pattern->max_rep_int : 0;
		t_max = pattern->min_rep_int < t1 ? t1 - pattern->min_rep_int : 0;

		last_tsf = t1;
		last_seen_period = 0;
		sum_periods = 0;
		matched = 0;
		noise = 0;
		missed = 0;
		partial_miss = 0;
		mean_period = 0;
		adjusted_max_rep_int = pattern->max_rep_int;
		adjusted_min_rep_int = pattern->min_rep_int;

		for (;;) {
			if (mean_period && pattern->dyn_ints) {
				u_int32_t fuzz_pct = pattern->fuzz_pct;
				adjusted_max_rep_int = MIN(nofloat_pct(mean_period, fuzz_pct), pattern->max_rep_int);

				adjusted_min_rep_int = MAX(nofloat_pct(mean_period, -fuzz_pct), pattern->min_rep_int);
			} else {
				adjusted_max_rep_int = pattern->max_rep_int;
				adjusted_min_rep_int = pattern->min_rep_int;
			}

			/* check if we are at the end of the list */
			if (&pulse->list == &sc->sc_rp_list)
				break;
			if (!pulse->rp_allocated)
				break;

			/* Do not go too far... this is an optimization to not
			 * keep checking after we hit maximum time span for the
			 * pattern. */
			if (pulse->rp_tsf < t0_max) {
				DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
					"%s: %s matching stopped (pulse->rp_tsf"
					" < t0_max).  t1=%10llu t0_max=%10llu " "t_min=%10llu t_max=%10llu matched=%u " "missed=%u\n", DEV_NAME(dev), pattern->name, t1, t0_max, t_min, t_max, matched, missed);
				break;
			}
			/* if we missed more than specified number of pulses,
			 * we stop searching */
			if (partial_miss > pattern->max_consecutive_missing) {
				DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
					"%s: %s matching stopped (too many "
					"consecutive pulses missing). %d>%d " "matched=%u. missed=%u.\n", DEV_NAME(dev), pattern->name, partial_miss, pattern->max_consecutive_missing, matched, missed);
				break;
			}

			new_period = (u_int64_t)
			    (last_tsf && last_tsf > pulse->rp_tsf) ? last_tsf - pulse->rp_tsf : 0;
			if (pulse->rp_tsf > t_max) {
				DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
					"%s: %-17s [%2d] %5s [**:**] tsf: "
					"%10llu [range: %5llu-%5llu].  width: "
					"%3d. period: %4llu. last_period: %4llu"
					". mean_period: %4llu. last_tsf: %10llu"
					".\n",
					DEV_NAME(dev),
					pattern->name,
					pulse->rp_index,
					"noise",
					(u_int64_t)pulse->rp_tsf,
					(u_int64_t)t_min, (u_int64_t)t_max, (u_int8_t)pulse->rp_width, (u_int64_t)new_period, (u_int64_t)last_seen_period, (u_int64_t)mean_period, (u_int64_t)last_tsf);
				/* this event is noise, ignore it */
				pulse = pulse_prev(pulse);
				noise++;
			} else if (pulse->rp_tsf >= t_min) {
				/* we found a match */
				matched++;
				if (partial_miss)
					new_period /= (partial_miss + 1);
				missed += partial_miss;
				partial_miss = 0;
				sum_periods += new_period;
				mean_period = matched ? (sum_periods / matched) : 0;
				if (mean_period && pattern->dyn_ints && (mean_period > pattern->max_rep_int || mean_period < pattern->min_rep_int)) {
					DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
						"%s: %s mean period deviated "
						"from original range [period: " "%4u, range: %4u-%4u]\n", DEV_NAME(dev), pattern->name, mean_period, pattern->min_rep_int, pattern->max_rep_int);
					break;
				}

				/* Remember we are scanning backwards... */
				DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
					"%s: %-17s [%2d] %5s [%2d:%-2d] tsf: "
					"%10llu [range: %5llu-%5llu].  width: "
					"%3d. period: %4llu. last_period: %4llu"
					". mean_period: %4llu. last_tsf: %10llu"
					".\n",
					DEV_NAME(dev),
					pattern->name,
					pulse->rp_index,
					"match",
					MAX(matched + missed + partial_miss - 1,
					    0),
					(matched + missed + partial_miss),
					(u_int64_t)pulse->rp_tsf,
					(u_int64_t)t_min, (u_int64_t)t_max, (u_int8_t)pulse->rp_width, (u_int64_t)new_period, (u_int64_t)last_seen_period, (u_int64_t)mean_period, (u_int64_t)last_tsf);

				/* record tsf and period */
				last_seen_period = new_period;
				last_tsf = pulse->rp_tsf;

				/* advance to next pulse */
				pulse = pulse_prev(pulse);

				/* update bounds */
				t_min = adjusted_max_rep_int < last_tsf ? last_tsf - adjusted_max_rep_int : 0;
				t_max = adjusted_min_rep_int < last_tsf ? last_tsf - adjusted_min_rep_int : 0;
			} else {
				partial_miss++;
				/* if we missed more than specified number of 
				 * pulses, we stop searching */
				if ((missed + partial_miss) > pattern->max_missing) {
					DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
						"%s: %s matching stopped (too "
						"many total pulses missing). " "%d>%d  matched=%u. missed=%u." "\n", DEV_NAME(dev), pattern->name, missed, pattern->max_missing, matched, missed);
					break;
				}
				/* Default mean period to approximate center 
				 * of range.  Remember we are scanning 
				 * backwards... */
				DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
					"%s: %-17s [**] %5s [%2d:%-2d] tsf: "
					"(missing) [range: %5llu-%5llu].  "
					"width: ***. period: ****. last_period:"
					" %4llu. mean_period: %4llu. last_tsf: "
					"%10llu.\n",
					DEV_NAME(dev),
					pattern->name,
					"missed",
					MAX(matched + missed + partial_miss - 1,
					    0), (matched + missed + partial_miss), (u_int64_t)t_min, (u_int64_t)t_max, (u_int64_t)last_seen_period, (u_int64_t)mean_period, (u_int64_t)last_tsf);

				/* update bounds */
				t_min = adjusted_max_rep_int < t_min ? t_min - adjusted_max_rep_int : 0;
				t_max = adjusted_min_rep_int < t_max ? t_max - adjusted_min_rep_int : 0;
			}
		}

		/* print counters for this PRF */
		if (matched > 1) {
			int compare_result = CR_FALLTHROUGH;
			int match_result = MR_MATCH;
			/* we add one to the matched since we counted only the 
			 * time differences */
			/* matched++; not sure... */

			/* check if PRF counters match a known radar, if we are
			 * confident enought */
			if (MR_MATCH == (match_result = match_radar(matched,
								    missed,
								    mean_period, noise, pattern->min_evts, pattern->max_evts, pattern->min_rep_int, pattern->max_rep_int, pattern->min_pulse, pattern->max_missing))) {
				compare_result = (NULL == best_pattern) ? CR_NULL :
				    compare_radar_matches(matched,
							  missed,
							  mean_period,
							  noise,
							  pattern->min_evts,
							  pattern->max_evts,
							  pattern->min_rep_int,
							  pattern->max_rep_int,
							  pattern->min_pulse,
							  pattern->max_missing,
							  pattern->match_midpoint,
							  best_matched,
							  best_missed,
							  best_pri,
							  best_noise,
							  best_pattern->min_evts,
							  best_pattern->max_evts, best_pattern->min_rep_int, best_pattern->max_rep_int, best_pattern->min_pulse, best_pattern->max_missing, best_pattern->match_midpoint);
			}
			if (DFLAG_ISSET(sc, ATH_DEBUG_DOTHFILT)) {
				DPRINTF(sc, ATH_DEBUG_DOTHFILT,
					"%s: [%02d] %13s: %-17s [match=%2u {%2u"
					"..%2u},missed=%2u/%2u,dur=%2d {%2u.."
					"%2u},noise=%2u/%2u,cr:%d]\n",
					DEV_NAME(dev),
					last_pulse->rp_index,
					compare_result > CR_FALLTHROUGH ?
					"NEW-BEST" :
					get_match_result_desc(match_result),
					pattern->name,
					matched, pattern->min_pulse, pattern->max_evts, missed, pattern->max_missing, matched + missed, pattern->min_evts, pattern->max_evts, noise, matched + noise, compare_result);
			}

			if (compare_result > CR_FALLTHROUGH) {
				best_matched = matched;
				best_missed = missed;
				best_index = i;
				best_pattern = pattern;
				best_pri = mean_period;
				best_noise = noise;
				best_cr = compare_result;
			} else if (compare_result <= CR_FALLTHROUGH) {
				DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
					"%s: %s match not better than best so " "far.  cr: %d matched: %d  missed: " "%d min_evts: %d\n", DEV_NAME(dev), pattern->name, compare_result, matched, missed, pattern->min_evts);
			}
		}
	}
	if (-1 != best_index) {
		DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE,
			"%s: [%02d] %10s: %-17s [match=%2u {%2u..%2u},missed="
			"%2u/%2u,dur=%2d {%2u..%2u},noise=%2u/%2u,cr=%2d] "
			"RI=%-9u RF=%-4u\n",
			DEV_NAME(sc->sc_dev),
			last_pulse->rp_index,
			"BEST/PULSE",
			best_pattern->name,
			best_matched,
			best_pattern->min_pulse,
			best_pattern->max_evts,
			best_missed,
			best_pattern->max_missing,
			(best_matched + best_missed), best_pattern->min_evts, best_pattern->max_evts, best_noise, (best_matched + best_noise), best_cr, best_pri, interval_to_frequency(best_pri));
		if (index)
			*index = best_index;
		if (pri)
			*pri = best_pri;
		if (matching_pulses)
			*matching_pulses = best_matched;
		if (noise_pulses)
			*noise_pulses = best_noise;
		if (missed_pulses)
			*missed_pulses = best_missed;
	}

	return (-1 != best_index) ? AH_TRUE : AH_FALSE;
}

#ifdef AR_DEBUG
static u_int32_t interval_to_frequency(u_int32_t interval)
{
	/* Calculate BRI from PRI */
	u_int32_t frequency = interval ? (1000000 / interval) : 0;
	/* Round to nearest multiple of 50 */
	return frequency + ((frequency % 50) >= 25 ? 50 : 0) - (frequency % 50);
}
#endif

#ifdef ATH_RADAR_LONG_PULSE
static const char *get_longpulse_desc(int lp)
{
	switch (lp) {
	case 8:
		return "FCC [5,  8 pulses]";
	case 9:
		return "FCC [5,  9 pulses]";
	case 10:
		return "FCC [5, 10 pulses]";
	case 11:
		return "FCC [5, 11 pulses]";
	case 12:
		return "FCC [5, 12 pulses]";
	case 13:
		return "FCC [5, 13 pulses]";
	case 14:
		return "FCC [5, 14 pulses]";
	case 15:
		return "FCC [5, 15 pulses]";
	case 16:
		return "FCC [5, 16 pulses]";
	case 17:
		return "FCC [5, 17 pulses]";
	case 18:
		return "FCC [5, 18 pulses]";
	case 19:
		return "FCC [5, 19 pulses]";
	case 20:
		return "FCC [5, 20 pulses]";
	default:
		return "FCC [5, invalid pulses]";
	}
}
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */

static HAL_BOOL rp_analyse(struct ath_softc *sc)
{
	HAL_BOOL radar = 0;
	struct ath_rp *pulse;

	/* Best short pulse match */
	int32_t best_index = -1;
	u_int32_t best_pri = 0;
	u_int32_t best_matched = 0;
	u_int32_t best_missed = 0;
	u_int32_t best_noise = 0;
	int32_t best_cr = 0;

#ifdef ATH_RADAR_LONG_PULSE
	/* Best long pulse match */
	u_int32_t best_lp_bc = 0;
	u_int32_t best_lp_matched = 0;
	u_int32_t best_lp_missed = 0;
	u_int32_t best_lp_noise = 0;
	u_int32_t best_lp_pulses = 0;
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
	u_int32_t pass = 0;
	struct radar_pattern_specification *best_pattern = NULL;

	/* start the analysis by the last pulse since it might speed up
	 * things and then move backward for all non-analyzed pulses.
	 * For debugging ONLY - we continue to run this scan after radar is 
	 * detected, processing all pulses... even when they come in after an 
	 * iteration of all pulses that were present when this function was 
	 * invoked.  This can happen at some radar waveforms where we will 
	 * match the first few pulses and then the rest of the burst will come 
	 * in, but never be analyzed.
	 */

	while (pulse_tail(sc)->rp_allocated && !pulse_tail(sc)->rp_analyzed && (AH_FALSE == radar || (DFLAG_ISSET(sc, ATH_DEBUG_DOTHFILT) && ++pass <= 3))) {

		list_for_each_entry_reverse(pulse, &sc->sc_rp_list, list) {
			if (!pulse->rp_allocated)
				break;
			if (pulse->rp_analyzed)
				break;

			/* Skip pulse analysis after we have confirmed radar 
			 * presence unless we are debugging and have 
			 * disabled short-circuit logic.  In this case,
			 * we'll go through ALL the signatures and find 
			 * the best match to convince ourselves this code works.
			 */
			if (AH_FALSE == radar || DFLAG_ISSET(sc, ATH_DEBUG_DOTHFILTNOSC)) {
				/* short pulse match status */
				u_int32_t index = 0;
				u_int32_t pri = 0;
				u_int32_t matched = 0;
				u_int32_t missed = 0;
				u_int32_t noise = 0;
#ifdef ATH_RADAR_LONG_PULSE

				/* long pulse match status */
				u_int32_t lp_bc = 0;
				u_int32_t lp_matched = 0;
				u_int32_t lp_missed = 0;
				u_int32_t lp_noise = 0;
				u_int32_t lp_pulses = 0;
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
				if (rp_analyse_short_pulse(sc, pulse, &index, &pri, &matched, &missed, &noise)) {
					int compare_result = (!radar || best_index == -1) ? CR_NULL : compare_radar_matches(matched,
															    missed,
															    pri,
															    noise,
															    radar_patterns[index].min_evts,
															    radar_patterns[index].max_evts,
															    radar_patterns[index].min_rep_int,
															    radar_patterns[index].max_rep_int,
															    radar_patterns[index].min_pulse,
															    radar_patterns[index].max_missing,
															    radar_patterns[index].match_midpoint,
															    best_matched,
															    best_missed,
															    best_pri,
															    best_noise,
															    radar_patterns[best_index].min_evts,
															    radar_patterns[best_index].max_evts,
															    radar_patterns[best_index].min_rep_int,
															    radar_patterns[best_index].max_rep_int,
															    radar_patterns[best_index].min_pulse,
															    radar_patterns[best_index].max_missing,
															    radar_patterns[best_index].match_midpoint);
					if (compare_result > CR_FALLTHROUGH) {
						/* Update best match */
						best_matched = matched;
						best_missed = missed;
						best_index = index;
						best_pri = pri;
						best_noise = noise;
						radar = AH_TRUE;
						best_cr = compare_result;
					}
					DPRINTF(sc, ATH_DEBUG_DOTHFILT,
						"%s: %10s: %-17s [match=%2u "
						"{%2u..%2u}, missed=%2u/%2u, "
						"dur=%2d {%2u..%2u}, "
						"noise=%2u/%2u, cr=%2d] "
						"RI=%-9u RF=%-4u\n",
						DEV_NAME(sc->sc_dev),
						(compare_result > CR_FALLTHROUGH) ?
						"BETTER" : "WORSE",
						radar_patterns[index].name,
						matched,
						radar_patterns[index].min_pulse,
						radar_patterns[index].max_evts,
						missed,
						radar_patterns[index].max_missing,
						(matched + missed), radar_patterns[index].min_evts, radar_patterns[index].max_evts, noise, (matched + noise), compare_result, pri, interval_to_frequency(pri));
				}
#ifdef ATH_RADAR_LONG_PULSE
				if (rp_analyze_long_pulse(sc, pulse, &lp_bc, &lp_matched, &lp_missed, &lp_noise, &lp_pulses)) {
					/* XXX: Do we care about best match?? */
					radar = AH_TRUE;
					best_lp_bc = lp_bc;
					best_lp_matched = lp_matched;
					best_lp_missed = lp_missed;
					best_lp_noise = lp_noise;
					best_lp_pulses = lp_pulses;
				}
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
			}
			pulse->rp_analyzed = 1;
		}
	}
	if (AH_TRUE == radar) {
#ifdef ATH_RADAR_LONG_PULSE
		if (!best_lp_bc) {
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
			best_pattern = &radar_patterns[best_index];
			DPRINTF(sc, ATH_DEBUG_DOTHFILT,
				"%s: %10s: %-17s [match=%2u {%2u..%2u},missed="
				"%2u/%2u,dur=%2d {%2u..%2u},noise=%2u/%2u,cr=%2d] "
				"RI=%-9u RF=%-4u\n",
				DEV_NAME(sc->sc_dev),
				"BEST MATCH",
				best_pattern->name,
				best_matched,
				best_pattern->min_pulse,
				best_pattern->max_evts,
				best_missed,
				best_pattern->max_missing,
				(best_matched + best_missed), best_pattern->min_evts, best_pattern->max_evts, best_noise, (best_matched + best_noise), best_cr, best_pri, interval_to_frequency(best_pri));
#ifdef ATH_RADAR_LONG_PULSE
		} else {
			DPRINTF(sc, ATH_DEBUG_DOTHFILT,
				"%s: %10s: %-17s [match=%2u {%2u..%2u},missed="
				"%2u/%2u,noise=%2u/%2u]\n",
				DEV_NAME(sc->sc_dev),
				"BEST MATCH", get_longpulse_desc(best_lp_bc), best_lp_bc, (best_lp_bc - 4), best_lp_bc, best_lp_missed, (best_lp_bc - (best_lp_bc - 4)), best_lp_noise, (best_lp_pulses + best_lp_noise)
			    );
		}
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
		if (DFLAG_ISSET(sc, ATH_DEBUG_DOTHFILT)) {
			DPRINTF(sc, ATH_DEBUG_DOTHFILT, "%s: ========================================\n", DEV_NAME(sc->sc_dev));
			DPRINTF(sc, ATH_DEBUG_DOTHFILT, "%s: ==BEGIN RADAR SAMPLE====================\n", DEV_NAME(sc->sc_dev));
			DPRINTF(sc, ATH_DEBUG_DOTHFILT, "%s: ========================================\n", DEV_NAME(sc->sc_dev));

#ifdef ATH_RADAR_LONG_PULSE
			if (!best_lp_bc) {
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
				best_pattern = &radar_patterns[best_index];
				DPRINTF(sc, ATH_DEBUG_DOTHPULSES,
					"%s: Sample contains data matching %s "
					"[match=%2u {%2u..%2u}, "
					"missed=%2u/%2u, dur=%2d {%2u..%2u}, "
					"noise=%2u/%2u,cr=%d] RI=%-9u RF=%-4u\n",
					DEV_NAME(sc->sc_dev),
					best_pattern->name,
					best_matched,
					best_pattern->min_pulse,
					best_pattern->max_evts,
					best_missed,
					best_pattern->max_missing,
					best_matched + best_missed, best_pattern->min_evts, best_pattern->max_evts, best_noise, best_noise + best_matched, best_cr, best_pri, interval_to_frequency(best_pri));
#ifdef ATH_RADAR_LONG_PULSE
			} else {
				DPRINTF(sc, ATH_DEBUG_DOTHPULSES, "%s: Sample contains data matching %s\n", DEV_NAME(sc->sc_dev), get_longpulse_desc(best_lp_bc));
			}
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */

			ath_rp_print(sc, 0 /* analyzed pulses only */ );
			DPRINTF(sc, ATH_DEBUG_DOTHFILT, "%s: ========================================\n", DEV_NAME(sc->sc_dev));
			DPRINTF(sc, ATH_DEBUG_DOTHFILT, "%s: ==END RADAR SAMPLE======================\n", DEV_NAME(sc->sc_dev));
			DPRINTF(sc, ATH_DEBUG_DOTHFILT, "%s: ========================================\n", DEV_NAME(sc->sc_dev));
		}
#ifdef ATH_RADAR_LONG_PULSE
		if (!best_lp_bc)
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
			ath_radar_detected(sc, radar_patterns[best_index].name);
#ifdef ATH_RADAR_LONG_PULSE
		else
			ath_radar_detected(sc, get_longpulse_desc(best_lp_bc));
#endif				/* #ifdef ATH_RADAR_LONG_PULSE */
	}
	return radar;
}

/* initialize ath_softc members so sensible values */
static void ath_rp_clear(struct ath_softc *sc)
{
	sc->sc_rp = NULL;
	INIT_LIST_HEAD(&sc->sc_rp_list);
	sc->sc_rp_num = 0;
	sc->sc_rp_analyse = NULL;
}

static void ath_rp_tasklet(TQUEUE_ARG data)
{
	struct net_device *dev = (struct net_device *)data;
	struct ath_softc *sc = netdev_priv(dev);

	if (sc->sc_rp_analyse != NULL)
		sc->sc_rp_analyse(sc);
}

void ath_rp_init(struct ath_softc *sc)
{
	struct net_device *dev = sc->sc_dev;
	int i;

	ath_rp_clear(sc);

	sc->sc_rp = (struct ath_rp *)kmalloc(sizeof(struct ath_rp) * ATH_RADAR_PULSE_NR, GFP_KERNEL);
	memset(sc->sc_rp, 0, sizeof(struct ath_rp) * ATH_RADAR_PULSE_NR);

	if (sc->sc_rp == NULL)
		return;

	/* initialize the circular list */
	INIT_LIST_HEAD(&sc->sc_rp_list);
	for (i = 0; i < ATH_RADAR_PULSE_NR; i++) {
		sc->sc_rp[i].rp_index = i;
		list_add_tail(&sc->sc_rp[i].list, &sc->sc_rp_list);
	}

	sc->sc_rp_num = 0;
	sc->sc_rp_analyse = rp_analyse;

	/* compute sc_rp_min */
	sc->sc_rp_min = 2;
	for (i = 0; i < sizetab(radar_patterns); i++)
		sc->sc_rp_min = MIN(sc->sc_rp_min, radar_patterns[i].min_pulse);

	/* default values is properly handle pulses and detected radars */
	sc->sc_rp_ignored = 0;
	sc->sc_radar_ignored = 0;

	ATH_INIT_TQUEUE(&sc->sc_rp_tq, ath_rp_tasklet, dev);
}

void ath_rp_done(struct ath_softc *sc)
{
	/* free what we allocated in ath_rp_init() */
	kfree(sc->sc_rp);

	ath_rp_clear(sc);
}

void ath_rp_record(struct ath_softc *sc, u_int64_t tsf, u_int8_t rssi, u_int8_t width, HAL_BOOL is_simulated)
{
#ifdef AR_DEBUG
	struct net_device *dev = sc->sc_dev;
#endif
	struct ath_rp *pulse;

	DPRINTF(sc, ATH_DEBUG_DOTHPULSES, "%s: ath_rp_record: " "tsf=%10llu rssi=%3u width=%3u%s\n", DEV_NAME(dev), tsf, rssi, width, sc->sc_rp_ignored ? " (ignored)" : "");

	if (sc->sc_rp_ignored) {
		return;
	}

	/* pulses width 255 seems to trigger false detection of radar. we
	 * ignored it then. */

	if (width == 255) {
		/* ignored */
		return;
	}

	/* check if the new radar pulse is after the last one recorded, or
	 * else, we flush the history */
	pulse = pulse_tail(sc);
	if (tsf < pulse->rp_tsf) {
		if (is_simulated == AH_TRUE && 0 == tsf) {
			DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE, "%s: %s: ath_rp_flush: simulated tsf " "reset.  tsf =%10llu, rptsf =%10llu\n", DEV_NAME(dev), __func__, tsf, pulse->rp_tsf);
			ath_rp_flush(sc);
		} else if ((pulse->rp_tsf - tsf) > (1 << 15)) {
			DPRINTF(sc, ATH_DEBUG_DOTHFILTVBSE, "%s: %s: ath_rp_flush: tsf reset.  " "(rp_tsf - tsf > 0x8000) tsf=%10llu, rptsf=" "%10llu\n", DEV_NAME(dev), __func__, tsf, pulse->rp_tsf);
			ath_rp_flush(sc);
		} else {
			DPRINTF(sc, ATH_DEBUG_DOTHFILT, "%s: %s: tsf jitter/bug detected: tsf =%10llu, " "rptsf =%10llu, rp_tsf - tsf = %10llu\n", DEV_NAME(dev), __func__, tsf, pulse->rp_tsf, pulse->rp_tsf - tsf);
		}
	}

	/* remove the head of the list */
	pulse = pulse_head(sc);
	list_del(&pulse->list);

	pulse->rp_tsf = tsf;
	pulse->rp_rssi = rssi;
	pulse->rp_width = width;
	pulse->rp_allocated = 1;
	pulse->rp_analyzed = 0;

	/* add at the tail of the list */
	list_add_tail(&pulse->list, &sc->sc_rp_list);
	if (ATH_RADAR_PULSE_NR > sc->sc_rp_num)
		sc->sc_rp_num++;
}

void ath_rp_print_mem(struct ath_softc *sc, int analyzed_pulses_only)
{
	struct ath_rp *pulse;
	u_int64_t oldest_tsf = ~0;
	int i;
	IPRINTF(sc, "Pulse dump of %spulses using sc_rp containing " "%d allocated pulses.\n", analyzed_pulses_only ? "analyzed " : "", sc->sc_rp_num);

	/* Find oldest TSF value so we can print relative times */
	for (i = 0; i < ATH_RADAR_PULSE_NR; i++) {
		pulse = &sc->sc_rp[i];
		if (pulse->rp_allocated && pulse->rp_tsf < oldest_tsf)
			oldest_tsf = pulse->rp_tsf;
	}

	for (i = 0; i < ATH_RADAR_PULSE_NR; i++) {
		pulse = &sc->sc_rp[i];
		if (!pulse->rp_allocated)
			break;
		if ((!analyzed_pulses_only) || pulse->rp_analyzed)
			IPRINTF(sc, "Pulse [%3d, %p] : relative_tsf=%10llu "
				"tsf=%10llu rssi=%3u width=%3u allocated=%d "
				"analyzed=%d next=%p prev=%p\n",
				pulse->rp_index, pulse, pulse->rp_tsf - oldest_tsf, pulse->rp_tsf, pulse->rp_rssi, pulse->rp_width, pulse->rp_allocated, pulse->rp_analyzed, pulse->list.next, pulse->list.prev);
	}
}

void ath_rp_print(struct ath_softc *sc, int analyzed_pulses_only)
{
	struct ath_rp *pulse;
	u_int64_t oldest_tsf = ~0;

	IPRINTF(sc, "Pulse dump of %spulses from ring buffer containing %d " "pulses.\n", analyzed_pulses_only ? "analyzed " : "", sc->sc_rp_num);

	/* Find oldest TSF value so we can print relative times */
	oldest_tsf = ~0;
	list_for_each_entry_reverse(pulse, &sc->sc_rp_list, list)
	    if (pulse->rp_allocated && pulse->rp_tsf < oldest_tsf)
		oldest_tsf = pulse->rp_tsf;

	list_for_each_entry_reverse(pulse, &sc->sc_rp_list, list) {
		if (!pulse->rp_allocated)
			continue;
		if ((!analyzed_pulses_only) || pulse->rp_analyzed)
			IPRINTF(sc, "Pulse [%3d, %p] : relative_tsf=%10llu "
				"tsf=%10llu rssi=%3u width=%3u allocated=%d "
				"analyzed=%d next=%p prev=%p\n",
				pulse->rp_index, pulse, pulse->rp_tsf - oldest_tsf, pulse->rp_tsf, pulse->rp_rssi, pulse->rp_width, pulse->rp_allocated, pulse->rp_analyzed, pulse->list.next, pulse->list.prev);
	}
}

void ath_rp_flush(struct ath_softc *sc)
{
	struct ath_rp *pulse;
	list_for_each_entry_reverse(pulse, &sc->sc_rp_list, list)
	    pulse->rp_allocated = 0;
	sc->sc_rp_num = 0;
}
