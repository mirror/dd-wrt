/*-
 * Copyright (c) 2004 Video54 Technologies, Inc.
 * Copyright (c) 2004 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/ratectrl/linux.c#10 $
 */
#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

/*
 * Atheros rate control algorithm (Linux-specific code)
 */
#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/cache.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>

#include <asm/uaccess.h>

#include <net80211/if_media.h>
#include <net80211/ieee80211_var.h>

#include "if_athvar.h"
#include "ah_desc.h"

#include "ratectrl.h"
#include "pktlog/pktlog.h"

/*
 * Attach to a device instance.  Setup the public definition
 * of how much per-node space we need and setup the private
 * phy tables that have rate control parameters.  These tables
 * are normally part of the Atheros hal but are not included
 * in our hal as the rate control data was not being used and
 * was considered proprietary (at the time).
 */
struct ath_ratectrl *
ath_rate_attach(struct ath_softc *sc)
{
	struct atheros_softc *asc;

	asc = kmalloc(sizeof(struct atheros_softc), GFP_ATOMIC);
	if (asc == NULL)
		return NULL;
	memset(asc, 0, sizeof(struct atheros_softc));
	asc->arc.arc_space = sizeof(struct atheros_node);
	asc->arc.arc_vap_space = sizeof(struct atheros_vap);
	/*
	 * Use the magic number to figure out the chip type.
	 * There's probably a better way to do this but for
	 * now this suffices.
	 *
	 * NB: We don't have a separate set of tables for the
	 *     5210; treat it like a 5211 since it has the same
	 *     tx descriptor format and (hopefully) sufficiently
	 *     similar operating characteristics to work ok.
	 */
	switch (sc->sc_ah->ah_magic) {
	case 0x19570405:
	default:		/* XXX 5210 */
		ar5211AttachRateTables(asc);
		break;
	case 0x19541014:	/* 5212 */
		ar5212AttachRateTables(asc);
		break;
	}
	return &asc->arc;
}
EXPORT_SYMBOL(ath_rate_attach);

void
ath_rate_detach(struct ath_ratectrl *rc)
{
	kfree(rc);
}
EXPORT_SYMBOL(ath_rate_detach);

/*
 * Initialize per-node rate control state.
 */
void
ath_rate_node_init(struct ath_softc *sc, struct ath_node *an)
{
	rcSibInit(sc, an);
}
EXPORT_SYMBOL(ath_rate_node_init);

/*
 * Cleanup per-node rate control state.
 */
void
ath_rate_node_cleanup(struct ath_softc *sc, struct ath_node *an)
{
	/* NB: nothing to do */
}
EXPORT_SYMBOL(ath_rate_node_cleanup);

/*
 * Return the next series 0 transmit rate and setup for a callback
 * to install the multi-rate transmit data if appropriate.  We cannot
 * install the multi-rate transmit data here because the caller is
 * going to initialize the tx descriptor and so would clobber whatever
 * we write. Note that we choose an arbitrary series 0 try count to
 * insure we get called back; this permits us to defer calculating
 * the actual number of tries until the callback at which time we
 * can just copy the pre-calculated series data.
 */
void
ath_rate_findrate(struct ath_softc *sc, struct ath_node *an,
	int shortPreamble, size_t frameLen,
	u_int8_t *rix, int *try0, u_int8_t *txrate, u_int8_t *lix)
{
	struct atheros_node *oan = ATH_NODE_ATHEROS(an);
	struct atheros_softc *asc = (struct atheros_softc *) sc->sc_rc;
	struct atheros_vap *avap = ATH_VAP_ATHEROS(an->an_node.ni_vap);
	const RATE_TABLE *pRateTable = avap->rateTable;

	if (asc->fixedrix == IEEE80211_FIXED_RATE_NONE) {
		*rix = rcRateFind(sc, oan, frameLen,pRateTable, lix);
		if (sc->sc_mrretry)
			*try0 = ATH_TXMAXTRY-1;	/* anything != ATH_TXMAXTRY */
		else
			*try0 = ATH_TXMAXTRY;
	} else {
		*rix = asc->fixedrix;
		*try0 = ATH_TXMAXTRY;
	}
	*txrate = pRateTable->info[*rix].rateCode
		| (shortPreamble ? pRateTable->info[*rix].shortPreamble : 0);
	/* update ni_txrate for status reports */
	an->an_node.ni_txrate = oan->rixMap[*rix];
#if TURBO_PRIME
	/* XXX map from merged table to split for driver */
	if (IS_CHAN_TURBO(&sc->sc_curchan) && *rix >= (pRateTable->rateCount-pRateTable->numTurboRates))
		*rix -= (pRateTable->rateCount-pRateTable->numTurboRates);
#endif
}
EXPORT_SYMBOL(ath_rate_findrate);

/*
 * Install the extended tx descriptor state.  This should only
 * ever be called for devices that support multi-rate retry.
 * Note that we cheat here and block-copy the pre-calculated
 * series data for all series.
 */
void
ath_rate_setupxtxdesc(struct ath_softc *sc, struct ath_node *an,
	struct ath_desc *ds, int shortPreamble, u_int8_t rix, u_int8_t lix)
{
	struct atheros_node *oan = ATH_NODE_ATHEROS(an);
	struct atheros_softc *asc = (struct atheros_softc *) sc->sc_rc;
	const RATE_TABLE *pRateTable = asc->hwRateTable[sc->sc_curmode];
	const void *retrySched;

	if (!sc->sc_mrretry) {
		RATE_DPRINTF(sc, "%s: no multi-rate retry!\n", __func__);
		/* XXX something is wrong */
		return;
	}
#if TURBO_PRIME
	/* XXX map from driver to merged table index */
	if (IS_CHAN_TURBO(&sc->sc_curchan))
		rix += (pRateTable->rateCount-pRateTable->numTurboRates);
#endif
	/* NB: only called for data frames */
	if (oan->txRateCtrl.probeRate) {
		retrySched = shortPreamble ?
			(const void *) &pRateTable->info[rix].probeShortSched :
			(const void *) &pRateTable->info[rix].probeSched;
	} else {
		retrySched = shortPreamble ?
			(const void *) &pRateTable->info[rix].shortSched :
			(const void *) &pRateTable->info[rix].normalSched;
	}
	memcpy(ds->ds_hw, retrySched, 2*sizeof(A_UINT32));

	if (sc->sc_curchan.privFlags & CHANNEL_4MS_LIMIT) {
        	u_int32_t *retryPtr, *ratePtr;
        	u_int32_t rate, prevRate, rateKbps, limitRateKbps;
        	u_int32_t i,index;

		retryPtr = (u_int32_t *) ds->ds_hw;
		ratePtr  = (u_int32_t *) retryPtr+1;
		prevRate = (*ratePtr) & 0x1F;
 
		/* Calculate the Rate limit */
		limitRateKbps = pRateTable->info[lix].rateKbps;
 
		for (i = 1; i < 4; i++) {
			/* Check if retry count is set */
			if ((*retryPtr >> (16 + 4*i)) & 0xF) {
				rate = (*ratePtr >> (i * 5)) & 0x1F;
				index = pRateTable->rateCodeToIndex[rate];
				rateKbps = pRateTable->info[index].rateKbps;
				if (rateKbps < limitRateKbps) {
					*ratePtr &= ~((0x1F) << (i*5));
					*ratePtr |= (prevRate << (i*5));
				} else {
					prevRate = rate;
				}
			} else {
				/* Retries are 0 from here */
				break;
			}
		}
	}
}
EXPORT_SYMBOL(ath_rate_setupxtxdesc);

#include <ar5212/ar5212desc.h>

#define	MS(_v, _f)	(((_v) & _f) >> _f##_S)

/*
 * Process a tx descriptor for a completed transmit (success or failure).
 */
void
ath_rate_tx_complete(struct ath_softc *sc,
	struct ath_node *an, const struct ath_desc *ds)
{
	struct atheros_vap *avap = ATH_VAP_ATHEROS(an->an_node.ni_vap);
	const RATE_TABLE *pRateTable = avap->rateTable;
	struct ath_tx_status	*txstat = ds->ds_txstat;
	u_int8_t txRate = txstat->ts_rate &~ HAL_TXSTAT_ALTRATE;

	if (pRateTable->rateCodeToIndex[txRate] == (u_int8_t) -1) {
		/*
		 * This can happen, for example, when switching bands
		 * and pending tx's are processed before the queue
		 * is flushed (should fix mode switch to ensure this
		 * does not happen).
		 */
		RATE_DPRINTF(sc, "%s: no mapping for rate code 0x%x",
			__func__, txRate);
		return;
	}
	if (txstat->ts_rate & HAL_TXSTAT_ALTRATE) {
		struct ar5212_desc *ads = AR5212DESC(ds);
		int finalTSIdx = MS(ads->ds_txstatus1, AR_FinalTSIndex);
		int series;

		/*
		 * Process intermediate rates that failed.
		 */
		for (series = 0; series < finalTSIdx; series++) {
			int rate, tries;

			/* NB: we know series <= 2 */
			switch (series) {
			case 0:
				rate = MS(ads->ds_ctl3, AR_XmitRate0);
				tries = MS(ads->ds_ctl2, AR_XmitDataTries0);
				break;
			case 1:
				rate = MS(ads->ds_ctl3, AR_XmitRate1);
				tries = MS(ads->ds_ctl2, AR_XmitDataTries1);
				break;
			default:
				rate = MS(ads->ds_ctl3, AR_XmitRate2);
				tries = MS(ads->ds_ctl2, AR_XmitDataTries2);
				break;
			}

			PKTLOG_TX_PKT(NULL, 
				(ds->ds_ctl0 & AR_FrameLen), 
				series, txstat->ts_rssi,
				tries, 
				txstat->ts_status,
				(A_UINT16)0,
				(A_UINT16)(txstat->ts_seqnum),
				txstat->ts_antenna,
				(A_UINT16)txstat->ts_tstamp);

			rcUpdate(sc, an
				, 2
				, pRateTable->rateCodeToIndex[rate]
				, tries
				, txstat->ts_rssi
				, txstat->ts_antenna
				, pRateTable
			);
		}
	}

	PKTLOG_TX_PKT(NULL, 
		(ds->ds_ctl0 & AR_FrameLen), 
		txRate, txstat->ts_rssi,
		txstat->ts_shortretry + txstat->ts_longretry,
		txstat->ts_status,
		(A_UINT16)0,
		(A_UINT16)(txstat->ts_seqnum),
		txstat->ts_antenna + 1,
		(A_UINT16)txstat->ts_tstamp);

	rcUpdate(sc, an
		, (txstat->ts_status & HAL_TXERR_XRETRY) != 0
		, pRateTable->rateCodeToIndex[txRate]
		, txstat->ts_shortretry + txstat->ts_longretry
		, txstat->ts_rssi
		, txstat->ts_antenna
		, pRateTable
	);
}
EXPORT_SYMBOL(ath_rate_tx_complete);

/*
 * Update rate-control state on station associate/reassociate.
 */
void
ath_rate_newassoc(struct ath_softc *sc, struct ath_node *an, int isnew)
{
	if (isnew) {
		struct atheros_softc *asc = (struct atheros_softc *) sc->sc_rc;
		struct atheros_node *oan = ATH_NODE_ATHEROS(an);

		rcSibUpdate(sc, an, 0);
		/* 
		 * Set an initial tx rate for the net80211 layer.
		 * Even though noone uses it, it wants to validate
		 * the setting before entering RUN state so if there
		 * was a pervious setting from a different node it
		 * may be invalid.
		 */
		if (asc->fixedrix != IEEE80211_FIXED_RATE_NONE) 
			an->an_node.ni_txrate = oan->rixMap[asc->fixedrix];
		else
			an->an_node.ni_txrate = 0;
	}
}
EXPORT_SYMBOL(ath_rate_newassoc);

static void
rate_cb(void *arg, struct ieee80211_node *ni)
{
	struct ath_softc *sc = arg;

	ath_rate_newassoc(sc, ATH_NODE(ni), 1);
}

/*
 * Update rate-control state on a device state change.  When
 * operating as a station this includes associate/reassociate
 * with an AP.  Otherwise this gets called, for example, when
 * the we transition to run state when operating as an AP.
 */
void
ath_rate_newstate(struct ieee80211vap *vap, enum ieee80211_state state)
{
	struct ieee80211com *ic = vap->iv_ic;
	struct ath_softc *sc = ic->ic_dev->priv;
	struct atheros_softc *asc = (struct atheros_softc *) sc->sc_rc;
	struct atheros_vap *avap = ATH_VAP_ATHEROS(vap);

	switch (sc->sc_ah->ah_magic) {
	case 0x19541014:	/* 5212 */
    		/* For half and quarter rate channles use different 
		 * rate tables 
		 */
    		if (IEEE80211_IS_CHAN_HALF(ic->ic_curchan)) {
			ar5212SetHalfRateTable(asc);
		} else if (IEEE80211_IS_CHAN_QUARTER(ic->ic_curchan)) {
			ar5212SetQuarterRateTable(asc);
    		} else { /* full rate */
			ar5212SetFullRateTable(asc);
		}
		break;
	default:		/* XXX 5210 */
		break;
	}

	/*
	 * Calculate index of any fixed rate configured.  It is safe
	 * to do this only here as changing/setting the fixed rate
	 * causes the 802.11 state machine to transition (which causes
	 * us to be notified).
	 */
#ifdef ATH_SUPERG_XR 
	if(vap->iv_flags & IEEE80211_F_XR) {
		avap->rateTable = asc->hwRateTable[WIRELESS_MODE_XR];
	} else {
		avap->rateTable = asc->hwRateTable[sc->sc_curmode];
	}
#else
		avap->rateTable = asc->hwRateTable[sc->sc_curmode];
#endif
	if (vap->iv_fixed_rate != IEEE80211_FIXED_RATE_NONE) {
		asc->fixedrix = sc->sc_rixmap[vap->iv_fixed_rate];
		/* NB: check the fixed rate exists */
		if (asc->fixedrix == 0xff)
			asc->fixedrix = IEEE80211_FIXED_RATE_NONE;
	} else
		asc->fixedrix = IEEE80211_FIXED_RATE_NONE;

	if (state == IEEE80211_S_RUN) {
		if (vap->iv_opmode != IEEE80211_M_STA) {
			/*
			 * Sync rates for associated stations and neighbors.
			 */
			ieee80211_iterate_nodes(&ic->ic_sta, rate_cb, sc);
		}
		ath_rate_newassoc(sc, ATH_NODE(vap->iv_bss), 1);
	}
}
EXPORT_SYMBOL(ath_rate_newstate);

void
atheros_setuptable(RATE_TABLE *rt)
{
	int i;

	for (i = 0; i < RATE_TABLE_SIZE; i++)
		rt->rateCodeToIndex[i] = (u_int8_t) -1;
	for (i = 0; i < rt->rateCount; i++) {
		u_int8_t code = rt->info[i].rateCode;

		/* Do not re-initialize rateCodeToIndex when using combined
		 * base + turbo rate tables. i.e the rateCodeToIndex should
		 * always point to base rate index. The ratecontrol module
		 * adjusts the index based on turbo mode.
		 */
		if (rt->rateCodeToIndex[code] == (u_int8_t) -1) {
			rt->rateCodeToIndex[code] = i;
			rt->rateCodeToIndex[ code | rt->info[i].shortPreamble] 
						= i;
		}
	}
}

/*
 * Module glue.
 */
static	char *dev_info = "ath_rate_atheros";

MODULE_AUTHOR("Video54 Technologies, Inc.");
MODULE_DESCRIPTION("Rate control support for Atheros devices");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Proprietary");
#endif

static int __init
init_ath_rate_atheros(void)
{
	/* XXX version is a guess; where should it come from? */
	printk(KERN_INFO "%s: Version 2.0.1\n"
		"Copyright (c) 2001-2004 Atheros Communications, Inc, "
		"All Rights Reserved\n", dev_info);
	ar5211SetupRateTables();
	ar5212SetupRateTables();
	return 0;
}
module_init(init_ath_rate_atheros);

static void __exit
exit_ath_rate_atheros(void)
{
	printk(KERN_INFO "%s: driver unloaded\n", dev_info);
}
module_exit(exit_ath_rate_atheros);
