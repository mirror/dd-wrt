/*
 * Copyright (c) 2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_beacon.c#2 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_internal.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"

#define TU_TO_USEC(_tu)		((_tu) << 10)

/*
 * Initialize all of the hardware registers used to
 * send beacons.  Note that for station operation the
 * driver calls ar5212SetStaBeaconTimers instead.
 */
void
ar5416SetBeaconTimers(struct ath_hal *ah, const HAL_BEACON_TIMERS *bt)
{
	u_int32_t bperiod;

	OS_REG_WRITE(ah, AR_NEXT_TBTT_TIMER, TU_TO_USEC(bt->bt_nexttbtt));
	OS_REG_WRITE(ah, AR_NEXT_DMA_BEACON_ALERT, TU_TO_USEC(bt->bt_nextdba >> 3));
	OS_REG_WRITE(ah, AR_NEXT_SWBA, TU_TO_USEC(bt->bt_nextswba >> 3));
	if (bt->bt_flags & AR_NDP_TIMER_EN) {
		/*
		 * Set the ATIM window 
		 * Our hardware does not support an ATIM window of 0
		 * (beacons will not work).  If the ATIM windows is 0,
		 * force it to 1.
		 */
		OS_REG_WRITE(ah, AR_NEXT_NDP_TIMER, TU_TO_USEC(bt->bt_nextatim > 0 ? bt->bt_nextatim : 1));
	}

	/* NB: no adhoc/power save mode */
	bperiod = TU_TO_USEC(bt->bt_intval & HAL_BEACON_PERIOD);
	OS_REG_WRITE(ah, AR5416_BEACON_PERIOD, bperiod);
	OS_REG_WRITE(ah, AR_DMA_BEACON_PERIOD, bperiod);
	OS_REG_WRITE(ah, AR_SWBA_PERIOD, bperiod);
	OS_REG_WRITE(ah, AR_NDP_PERIOD, bperiod);


	/*
	 * Reset TSF if required.
	 */
	if (bt->bt_intval & AR_BEACON_RESET_TSF) {
		AH_PRIVATE(ah)->ah_curchan->ah_tsf_last = 0;
		ar5416ResetTsf(ah);
	}


	/* enable timers */
	/* NB: flags == 0 handled specially for backwards compatibility */
	OS_REG_SET_BIT(ah, AR_TIMER_MODE,
	    bt->bt_flags != 0 ? bt->bt_flags :
		AR_TBTT_TIMER_EN | AR_DBA_TIMER_EN | AR_SWBA_TIMER_EN);

	switch (AH_PRIVATE(ah)->ah_opmode) {
	case HAL_M_IBSS:
		OS_REG_SET_BIT(ah, AR_TXCFG, AR_TXCFG_ADHOC_BEACON_ATIM_TX_POLICY);
		break;
	default:
		OS_REG_CLR_BIT(ah, AR_TXCFG, AR_TXCFG_ADHOC_BEACON_ATIM_TX_POLICY);
		break;
	}
}

/*
 * Initializes all of the hardware registers used to
 * send beacons.  Note that for station operation the
 * driver calls ar5212SetStaBeaconTimers instead.
 */
void
ar5416BeaconInit(struct ath_hal *ah,
	u_int32_t next_beacon, u_int32_t beacon_period)
{
	HAL_BEACON_TIMERS bt;

	bt.bt_nexttbtt = next_beacon;
	/* 
	 * TIMER1: in AP/adhoc mode this controls the DMA beacon
	 * alert timer; otherwise it controls the next wakeup time.
	 * TIMER2: in AP mode, it controls the SBA beacon alert
	 * interrupt; otherwise it sets the start of the next CFP.
	 */
	bt.bt_flags = 0;
	switch (AH_PRIVATE(ah)->ah_opmode) {
	case HAL_M_STA:
	case HAL_M_MONITOR:
		bt.bt_nextdba = 0xffff;
		bt.bt_nextswba = 0x7ffff;
		bt.bt_flags = AR_TBTT_TIMER_EN;
		break;
	case HAL_M_IBSS:
#ifdef notyet
		bt.bt_flags |= AR_NDP_TIMER_EN;
#endif
		/* fall through */
	case HAL_M_HOSTAP:
		bt.bt_nextdba = (next_beacon -
			ath_hal_dma_beacon_response_time) << 3;	/* 1/8 TU */
		bt.bt_nextswba = (next_beacon -
			ath_hal_sw_beacon_response_time) << 3;	/* 1/8 TU */
		bt.bt_flags |= AR_TBTT_TIMER_EN | AR_DBA_TIMER_EN |
			AR_SWBA_TIMER_EN;
		break;
	}
	bt.bt_nextatim = next_beacon + 1;
	bt.bt_intval = beacon_period &
		(AR_BEACON_PERIOD | AR_BEACON_RESET_TSF | AR_BEACON_EN);
	ar5416SetBeaconTimers(ah, &bt);
}

#define AR_BEACON_PERIOD_MAX	0xffff

void
ar5416ResetStaBeaconTimers(struct ath_hal *ah)
{
	u_int32_t val;

	OS_REG_WRITE(ah, AR_NEXT_TBTT_TIMER, 0);		/* no beacons */
	val = OS_REG_READ(ah, AR_STA_ID1);
	val |= AR_STA_ID1_PWR_SAV;		/* XXX */
	/* tell the h/w that the associated AP is not PCF capable */
	OS_REG_WRITE(ah, AR_STA_ID1,
		val & ~(AR_STA_ID1_USE_DEFANT | AR_STA_ID1_PCF));
	OS_REG_WRITE(ah, AR5416_BEACON_PERIOD, AR_BEACON_PERIOD_MAX);
	OS_REG_WRITE(ah, AR_DMA_BEACON_PERIOD, AR_BEACON_PERIOD_MAX);
}

/*
 * Set all the beacon related bits on the h/w for stations
 * i.e. initializes the corresponding h/w timers;
 * also tells the h/w whether to anticipate PCF beacons
 */
void
ar5416SetStaBeaconTimers(struct ath_hal *ah, const HAL_BEACON_STATE *bs)
{
	u_int32_t nextTbtt, nextdtim,beaconintval, dtimperiod;

	HALASSERT(bs->bs_intval != 0);

	/* NB: no cfp setting since h/w automatically takes care */
	OS_REG_WRITE(ah, AR_NEXT_TBTT_TIMER, TU_TO_USEC(bs->bs_nexttbtt));

	/*
	 * Start the beacon timers by setting the BEACON register
	 * to the beacon interval; no need to write tim offset since
	 * h/w parses IEs.
	 */
	OS_REG_WRITE(ah, AR5416_BEACON_PERIOD,
			 TU_TO_USEC(bs->bs_intval & HAL_BEACON_PERIOD));
	OS_REG_WRITE(ah, AR_DMA_BEACON_PERIOD,
			 TU_TO_USEC(bs->bs_intval & HAL_BEACON_PERIOD));

	/*
	 * Configure the BMISS interrupt.  Note that we
	 * assume the caller blocks interrupts while enabling
	 * the threshold.
	 */
	HALASSERT(bs->bs_bmissthreshold <=
		(AR_RSSI_THR_BM_THR >> AR_RSSI_THR_BM_THR_S));
	OS_REG_RMW_FIELD(ah, AR_RSSI_THR,
		AR_RSSI_THR_BM_THR, bs->bs_bmissthreshold);

	/*
	 * Program the sleep registers to correlate with the beacon setup.
	 */

	/*
	 * Oahu beacons timers on the station were used for power
	 * save operation (waking up in anticipation of a beacon)
	 * and any CFP function; Venice does sleep/power-save timers
	 * differently - so this is the right place to set them up;
	 * don't think the beacon timers are used by venice sta hw
	 * for any useful purpose anymore
	 * Setup venice's sleep related timers
	 * Current implementation assumes sw processing of beacons -
	 *   assuming an interrupt is generated every beacon which
	 *   causes the hardware to become awake until the sw tells
	 *   it to go to sleep again; beacon timeout is to allow for
	 *   beacon jitter; cab timeout is max time to wait for cab
	 *   after seeing the last DTIM or MORE CAB bit
	 */
#define CAB_TIMEOUT_VAL     10 /* in TU */
#define BEACON_TIMEOUT_VAL  10 /* in TU */
#define SLEEP_SLOP          3  /* in TU */

	/*
	 * For max powersave mode we may want to sleep for longer than a
	 * beacon period and not want to receive all beacons; modify the
	 * timers accordingly; make sure to align the next TIM to the
	 * next DTIM if we decide to wake for DTIMs only
	 */
	beaconintval = bs->bs_intval & HAL_BEACON_PERIOD;
	HALASSERT(beaconintval != 0);
	if (bs->bs_sleepduration > beaconintval) {
		HALASSERT(roundup(bs->bs_sleepduration, beaconintval) ==
				bs->bs_sleepduration);
		beaconintval = bs->bs_sleepduration;
	}
	dtimperiod = bs->bs_dtimperiod;
	if (bs->bs_sleepduration > dtimperiod) {
		HALASSERT(dtimperiod == 0 ||
			roundup(bs->bs_sleepduration, dtimperiod) ==
				bs->bs_sleepduration);
		dtimperiod = bs->bs_sleepduration;
	}
	HALASSERT(beaconintval <= dtimperiod);
	if (beaconintval == dtimperiod)
		nextTbtt = bs->bs_nextdtim;
	else
		nextTbtt = bs->bs_nexttbtt;
	nextdtim = bs->bs_nextdtim;

	OS_REG_WRITE(ah, AR_NEXT_DTIM,
		TU_TO_USEC(bs->bs_nextdtim - SLEEP_SLOP));
	OS_REG_WRITE(ah, AR_NEXT_TIM, TU_TO_USEC(nextTbtt - SLEEP_SLOP));

	/* cab timeout is now in 1/8 TU */
	OS_REG_WRITE(ah, AR_SLEEP1,
		SM((CAB_TIMEOUT_VAL << 3), AR5416_SLEEP1_CAB_TIMEOUT)
		| AR_SLEEP1_ASSUME_DTIM);
	/* beacon timeout is now in 1/8 TU */
	OS_REG_WRITE(ah, AR_SLEEP2,
		SM((BEACON_TIMEOUT_VAL << 3), AR5416_SLEEP2_BEACON_TIMEOUT));

	OS_REG_WRITE(ah, AR_TIM_PERIOD, beaconintval);
	OS_REG_WRITE(ah, AR_DTIM_PERIOD, dtimperiod);
	OS_REG_SET_BIT(ah, AR_TIMER_MODE, AR_TBTT_TIMER_EN | AR_TIM_TIMER_EN 
			| AR_DTIM_TIMER_EN);
	HALDEBUG(ah, "%s: next DTIM %d\n", __func__, bs->bs_nextdtim);
	HALDEBUG(ah, "%s: next beacon %d\n", __func__, nextTbtt);
	HALDEBUG(ah, "%s: beacon period %d\n", __func__, beaconintval);
	HALDEBUG(ah, "%s: DTIM period %d\n", __func__, dtimperiod);
#undef CAB_TIMEOUT_VAL
#undef BEACON_TIMEOUT_VAL
#undef SLEEP_SLOP
}
#endif /* AH_SUPPORT_AR5416 */
