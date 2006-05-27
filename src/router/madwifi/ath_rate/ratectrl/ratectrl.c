/*
 * $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/ratectrl/ratectrl.c#9 $
 *
 * Copyright (c) 2001-2003 Atheros Communications, Inc., All Rights Reserved
 *
 * DESCRIPTION
 *
 * This file contains the data structures and routines for transmit rate
 * control.
 *
 * Releases prior to 2.0.1 used a different algorithm that was based on
 * an active of unicast packets at different rate.  Starting in 2.0.1
 * a new algorithm was introduced.
 *
 * Basic algorithm:
 *   - Keep track of the ackRssi from each successful transmit.
 *   - For each user (SIB entry) maintain a table of rssi thresholds, one
 *     for each rate (eg: 8 or 4). This is the minimum rssi required for
 *     each rate for this user.
 *   - Pick the rate by lookup in the rssi table.
 *   - Update the rssi thresholds based on number of retries and Xretries.
 *     Also, slowly reduce the rssi thresholds over time towards their
 *     theoretical minimums.  In this manner the thresholds will adapt
 *     to the channel characteristics, like delay spread etc, while not
 *     staying abnormally high.
 *
 * More specific details:
 *  - The lkupRssi we use to lookup is computed as follows:
 *  - Use the median of the last three ackRssis.
 *  - Reduce the lkupRssi if the last ackRssi is older than 32msec.
 *    Specifically, reduce it linearly by up to 10dB for ages between
 *    25msec and 185msec, and a fixed 10dB for ages more than 185msec.
 *  - Reduce the lkupRssi by 1 or 2 dB if the packet is long or very long
 *    (this tweaks the rate for longer versus short packets).
 *  - When we get Xretries, reduce lkupRssi by 10dB, unless this was
 *    a probe (see below).
 *  - Maintain a maxRate (really an index, not an actual rate).  We don't
 *    pick rates above the maxRate.  The rssi lookup is free to choose any
 *    rate between [0,maxRate] on each packet.
 *
 *  The maxRate is increased by periodically probing:
 *    - No more than once every 100msec, and only if lkupRssi is high
 *      enough, we try a rate one higher than the current max rate.  If
 *      it succeeds with no retries then we increase the maxRate by one.
 *
 *  The rssi lkup is then free to use this new rate whenever the
 *  rssi warrants.
 *
 *  The maxRate is decreased if a rate looks poor (way too many
 *  retries or a couple of Xretries without many good packets in
 *  between):
 *    - We maintain a packet error rate estimate (per) for each rate.
 *      Each time we get retries or Xretries we increase the PER.
 *      Each time we get no retries we decrease the PER.  If the
 *      PER ever exceeds X (eg: an Xretry will increase the PER by Y)
 *      we set the maxRate to one below this rate.
 */

#ifdef __FreeBSD__
#include <dev/ath/ath_rate/atheros/ratectrl.h>
#else
#include "ratectrl.h"
#include "pktlog/pktlog.h"
#endif

/* Access functions for validTxRateMask */

static void
rcInitValidTxMask(struct TxRateCtrl_s *pRc)
{
    pRc->validTxRateMask = 0;
}

static void
rcSetValidTxMask(struct TxRateCtrl_s *pRc, A_UINT8 index, A_BOOL validTxRate)
{
    ASSERT(index < pRc->rateTableSize);

    if (validTxRate) {
        pRc->validTxRateMask |= (1 << index);
    } else {
        pRc->validTxRateMask &= ~(1 << index);
    }
}

/* Iterators for validTxRateMask */

static INLINE A_BOOL
rcGetFirstValidTxRate(struct TxRateCtrl_s *pRc, A_UINT8 *pIndex, const RATE_TABLE *pRateTable, 
                      A_UINT8 turboPhyOnly, WLAN_PHY turboPhy)
{
    A_UINT32    mask = pRc->validTxRateMask;
    A_UINT8     i;

    for (i = 0; i < pRc->rateTableSize; i++) {
        if (mask & (1 << i)) {
            if ((!turboPhyOnly) || (pRateTable->info[i].phy == turboPhy)) {
                *pIndex = i;
                return TRUE;
            }
        }
    }

    /*
     * No valid rates - this should not happen.
     *
     * Note - Removed an assert here because the dummy sib
     * was causing the assert to trigger.
     */
    *pIndex = 0;

    return FALSE;
}

static INLINE A_BOOL
rcGetNextValidTxRate(struct TxRateCtrl_s *pRc, A_UINT8 curValidTxRate, A_UINT8 *pNextIndex, 
                     const RATE_TABLE *pRateTable, A_UINT8 excludeTurboPhy, WLAN_PHY turboPhy)
{
    A_UINT32    mask = pRc->validTxRateMask;
    A_UINT8     i;

    for (i = curValidTxRate + 1; i < pRc->rateTableSize; i++) {
        if (mask & (1 << i) &&
            (!excludeTurboPhy || pRateTable->info[i].phy != turboPhy))
        {
            *pNextIndex = i;
            return TRUE;
        }
    }

    /* No more valid rates */
    *pNextIndex = 0;

    return FALSE;
}

static A_BOOL
rcGetPrevValidTxRate(struct TxRateCtrl_s *pRc, A_UINT8 curValidTxRate, A_UINT8 *pPrevIndex )
{
    A_UINT32    mask = pRc->validTxRateMask;
    int         i;

    if (curValidTxRate == 0) {
        *pPrevIndex = 0;
        return FALSE;
    }

    for (i = curValidTxRate - 1; i >= 0; i--) {
        if (mask & (1 << i)) {
            *pPrevIndex = (A_UINT8)i;
            return TRUE;
        }
    }

    /* No more valid rates */
    *pPrevIndex = 0;

    return FALSE;
}

static A_BOOL
rcGetNextLowerValidTxRate(const RATE_TABLE *pTable, struct TxRateCtrl_s *pRc, 
                          A_UINT8 curValidTxRate, A_UINT8 *pNextIndex )
{
    A_UINT32    mask = pRc->validTxRateMask;
    A_UINT8     i;

    *pNextIndex = 0;
    for (i = 1; i < pRc->rateTableSize; i++) {
        if ((mask & (1 << i)) && 
            (pTable->info[i].rateKbps < pTable->info[curValidTxRate].rateKbps))
        {
            if (*pNextIndex) {
                if (pTable->info[i].rateKbps > pTable->info[*pNextIndex].rateKbps) {
                    *pNextIndex = i;
                }
            } else {
                *pNextIndex = i;
            }
        }
    }

    if (*pNextIndex) {
        return TRUE;
    } else {
        *pNextIndex = curValidTxRate;
        return FALSE;
    }
}

/*
 *  Update the SIB's rate control information
 *
 *  This should be called when the supported rates change
 *  (e.g. SME operation, wireless mode change)
 *
 *  It will determine which rates are valid for use.
 */
void
rcSibUpdate(struct ath_softc *sc, struct ath_node *an, A_BOOL keepState)
{
    struct atheros_node	*pSib	    = ATH_NODE_ATHEROS(an);
    struct atheros_softc *asc	    = (struct atheros_softc *) sc->sc_rc;
    const RATE_TABLE    *pRateTable ;
    struct ieee80211_rateset *pRateSet = &an->an_node.ni_rates;
    struct TxRateCtrl_s *pRc        = &pSib->txRateCtrl;
    A_UINT8             i, j, hi = 0,count;
    int                 rateCount;

#ifdef ATH_SUPERG_XR
	if(an->an_node.ni_vap && an->an_node.ni_vap->iv_flags & IEEE80211_F_XR ) 
		pRateTable = asc->hwRateTable[WIRELESS_MODE_XR];
	else
		pRateTable = asc->hwRateTable[sc->sc_curmode];
#else
		pRateTable = asc->hwRateTable[sc->sc_curmode];
#endif
    /* Initial rate table size. Will change depending on the working rate set */
    pRc->rateTableSize = MAX_TX_RATE_TBL;

    /* Initialize thresholds according to the global rate table */
    for (i = 0 ; (i < pRc->rateTableSize) && (!keepState); i++) {
        pRc->state[i].rssiThres = pRateTable->info[i].rssiAckValidMin;
        pRc->state[i].per       = 0;
    }

    /* Determine the valid rates */
    rcInitValidTxMask(pRc);
    rateCount = pRateTable->rateCount;
#ifdef notyet
    if (wlanIs5211Channel14(pSib)) {
        rateCount = 2;
    }
#endif

    count = 0;
    if (!pRateSet->rs_nrates) {
        /* No working rate, use valid rates */
        for (i = 0; i < rateCount; i++) {
            if (pRateTable->info[i].valid == TRUE) {
                pRc->validRateIndex[count] = i;
                count ++;
                rcSetValidTxMask(pRc, i, TRUE);
                hi = A_MAX(hi, i);
		pSib->rixMap[i] = 0;
            }
        }

        pRc->maxValidRate = count;
        pRc->maxValidTurboRate = pRateTable->numTurboRates;
    } else {
        A_UINT8  turboCount;
        A_UINT32 mask;

        /* Use intersection of working rates and valid rates */
        turboCount = 0;
        for (i = 0; i < pRateSet->rs_nrates; i++) {
            for (j = 0; j < rateCount; j++) {
                if (((pRateSet->rs_rates[i] & 0x7F) == (pRateTable->info[j].dot11Rate & 0x7F)) &&
                    (pRateTable->info[j].valid == TRUE))
                {
                    rcSetValidTxMask(pRc, j, TRUE);
                    hi = A_MAX(hi, j);
		    pSib->rixMap[j] = i;
                }
            }
        }

        /* Get actually valid rate index, previous we get it from rate table,
         * now get rate table which include all working rate, so we need make
         * sure our valid rate table align with working rate */
        mask = pRc->validTxRateMask;
        for (i = 0; i < pRc->rateTableSize; i ++) {
            if (mask & (1 << i)) {
                pRc->validRateIndex[count] = i;
                count ++;
                if (pRateTable->info[i].phy == WLAN_PHY_TURBO) {
                    turboCount ++;
                }
            }
        }

        pRc->maxValidRate = count;
        pRc->maxValidTurboRate = turboCount;
    }

    pRc->rateTableSize = hi + 1;
    pRc->rateMax = A_MIN(hi, pRateTable->initialRateMax);

    ASSERT(pRc->rateTableSize <= MAX_TX_RATE_TBL);
}

/*
 *  This routine is called to initialize the rate control parameters
 *  in the SIB. It is called initially during system initialization
 *  or when a station is associated with the AP.
 */
void
rcSibInit(struct ath_softc *sc, struct ath_node *an)
{
    struct atheros_node	*pSib	    = ATH_NODE_ATHEROS(an);
    struct TxRateCtrl_s *pRc        = &pSib->txRateCtrl;

#if 0
    /* NB: caller assumed to zero state */
    A_MEM_ZERO((char *)pSib, sizeof(*pSib));
#endif
    pRc->rssiDownTime = A_MS_TICKGET();
}


/*
 * Return the median of three numbers
 */
static INLINE A_RSSI
median(A_RSSI a, A_RSSI b, A_RSSI c)
{
    if (a >= b) {
        if (b >= c) {
            return b;
        } else if (a > c) {
            return c;
        } else {
            return a;
        }
    } else {
        if (a >= c) {
            return a;
        } else if (b >= c) {
            return c;
        } else {
            return b;
        }
    }
}

/*
 * Determines and returns the new Tx rate index.
 */
A_UINT16
rcRateFind(struct ath_softc *sc, struct atheros_node *pSib, A_UINT32 frameLen,const RATE_TABLE *pRateTable, A_UINT8 *lix)
{
    struct ieee80211com	 *ic	      = &sc->sc_ic;
#ifdef notyet
    WLAN_STA_CONFIG      *pConfig     = &pdevInfo->staConfig;
    VPORT_BSS            *pVportXrBss = GET_XR_BSS(pdevInfo);
    WLAN_DATA_MAC_HEADER *pHdr        = pDesc->pBufferVirtPtr.header;
#endif
    struct TxRateCtrl_s  *pRc;
    A_UINT32             dt;
    A_UINT32             bestThruput,thisThruput;
    A_UINT32             nowMsec;
    A_UINT8              rate, nextRate, bestRate, limitRate;
    A_UINT16             limitRateKbps;
    A_RSSI               rssiLast, rssiReduce;
#if TURBO_PRIME
    A_UINT8              currentPrimeState = IS_CHAN_TURBO(&sc->sc_curchan);
                                             /* 0 = regular; 1 = turbo */
    A_UINT8              primeInUse        = sc->sc_dturbo;
#else
    A_UINT8              primeInUse        = 0;
    A_UINT8              currentPrimeState = 0;
#endif
#ifdef notyet
    A_UINT8              xrRateAdaptation = FALSE;
#endif
    A_BOOL               isChanTurbo      = FALSE;
    A_UINT8              maxIndex, minIndex;
    A_INT8               index;
    A_BOOL               isProbing = FALSE;

    /* have the real rate control logic kick in */
    pRc = &pSib->txRateCtrl;

#if TURBO_PRIME
    /*
     * Reset primeInUse state, if we are currently using XR 
     * rate tables or if we have any clients associated in XR mode
     */
#ifdef notyet
    if ((pRateTable == sc->sc_rates[WLAN_MODE_XR]) ||
        (isXrAp(sc) && (pVportXrBss->bss.numAssociatedClients > 0)))
    {
	currentPrimeState = 0;
        primeInUse = 0;
    }
#endif
   
    /* make sure that rateMax is correct when using TURBO_PRIME tables */
    if (currentPrimeState)
    {
	pRc->rateMax = pRateTable->rateCount - 1;
    } else
    {
	pRc->rateMax = pRateTable->rateCount - 1 - pRateTable->numTurboRates;
    }
#endif /* TURBO_PRIME */

    rssiLast   = median(pRc->rssiLast, pRc->rssiLastPrev, pRc->rssiLastPrev2);
    rssiReduce = 0;

    /*
     * Age (reduce) last ack rssi based on how old it is.
     * The bizarre numbers are so the delta is 160msec,
     * meaning we divide by 16.
     *   0msec   <= dt <= 25msec:   don't derate
     *   25msec  <= dt <= 185msec:  derate linearly from 0 to 10dB
     *   185msec <= dt:             derate by 10dB
     */
    nowMsec = A_MS_TICKGET();
    dt = nowMsec - pRc->rssiTime;

    if (dt >= 185) {
        rssiReduce = 10;
    } else if (dt >= 25) {
        rssiReduce = (A_UINT8)((dt - 25) >> 4);
    }

    /* Reduce rssi for long packets */
    if (frameLen > 800) {
        rssiReduce += 1;    /* need 1 more dB for packets > 800 bytes */
    }

    /* Now reduce rssiLast by rssiReduce */
    rssiLast -= rssiReduce;
    if (rssiLast < 0 ) {
		if (rssiLast <  pRc->state[0].rssiThres) {
			rssiLast =  pRc->state[0].rssiThres;
		}
    }

    /*
     * Now look up the rate in the rssi table and return it.
     * If no rates match then we return 0 (lowest rate)
     */

    bestThruput = 0;
    maxIndex    = pRc->maxValidRate - 1;
#ifdef notyet
    minIndex    = pDesc->pVportBss->bss.minimumRateIndex;;
#else
    minIndex    = 0;
#endif
    isChanTurbo = IS_CHAN_TURBO(&sc->sc_curchan);

    /* 
     * If we use TURBO_PRIME table, but we aren't in turbo mode,
     * we need to reduce maxIndex to actual size
     */
    if (pRc->maxValidTurboRate) {
        if (!isChanTurbo) {
            maxIndex -= pRc->maxValidTurboRate;
            /* This shouldn't happen unless we use wrong rate table */
            if (maxIndex == 0) {
                pSib->lastRateKbps = pRateTable->info[0].rateKbps;
                return 0;
            }
        } else {
            if (pRc->maxValidRate > pRc->maxValidTurboRate) {
                minIndex = pRc->maxValidRate - pRc->maxValidTurboRate;
            }
        }
    }

    bestRate =  pRc->validRateIndex[minIndex];
    /*
     * Try the higher rate first. It will reduce memory moving time
     * if we have very good channel characteristics.
     */
    for (index = maxIndex; index >= minIndex ; index--) {
        rate = pRc->validRateIndex[index];

        if (rssiLast < pRc->state[rate].rssiThres) {
            continue;
        }

        thisThruput = pRateTable->info[rate].userRateKbps * 
                      (100 - pRc->state[rate].per);

        if (bestThruput <= thisThruput) {
            bestThruput = thisThruput;
            bestRate    = rate;
        }
    }

    rate = bestRate;
    
    /* Following are recorded as a part of pktLog feature */
    pRc->misc[4]  = primeInUse;
    pRc->misc[5]  = currentPrimeState;
    pRc->misc[9]  = pRc->rateTableSize;
    pRc->misc[8]  = rcGetNextValidTxRate(pRc, rate, &pRc->misc[7], pRateTable,
                                         !isChanTurbo, WLAN_PHY_TURBO);
    pRc->misc[10] = rate;

    pRc->rssiLastLkup = rssiLast;

#ifdef notyet
   /*
    * When we have exhausted half of our max retries, we should force to use
    * the lowest rate in the current table
    */
    if (pDesc->swretryCount && 
        (pDesc->swretryCount >= pConfig->swRetryMaxRetries / 2)) 
    {
        rate = minIndex;
    }
#endif
 
    /*
     * Must check the actual rate (rateKbps) to account for non-monoticity of
     * 11g's rate table
     */

    if (rate > pRc->rateMax) {
        rate = pRc->rateMax;

        /*
         * Always probe the next rate in the rate Table (ignoring monotonicity).
         * Reason:  If OFDM is broken, when rateMax = 5.5, it will probe
         *          11 Mbps first.
         */
        if (rcGetNextValidTxRate(pRc, rate, &nextRate, pRateTable,
                                 !isChanTurbo, WLAN_PHY_TURBO) &&
            (nowMsec - pRc->probeTime > pRateTable->probeInterval) &&
            (pRc->hwMaxRetryPktCnt >= 4))
        {
            rate                  = nextRate;
            pRc->probeRate        = (A_UINT8)rate;
            pRc->probeTime        = nowMsec;
            pRc->hwMaxRetryPktCnt = 0;

            isProbing = TRUE;
        }
    }

    /*
     * Make sure rate is not higher than the allowed maximum.
     * We should also enforce the min, but I suspect the min is
     * normally 1 rather than 0 because of the rate 9 vs 6 issue
     * in the old code.
     */
    if (rate > (pSib->txRateCtrl.rateTableSize - 1)) {
        rate = pSib->txRateCtrl.rateTableSize - 1;
    }

#if TURBO_PRIME
    /* turboPrime - make mode switch recommendation */
    if (primeInUse && (ic->ic_opmode == IEEE80211_M_HOSTAP)) {
        if (currentPrimeState == 0) { /* regular mode */
            pRc->recommendedPrimeState = 0;   /* default:  don't switch */
            if (rate > pRateTable->regularToTurboThresh) {
                pRc->switchCount++;
                if (pRc->switchCount >= pRateTable->pktCountThresh) {
                    A_UINT8 targetRate = pRateTable->regularToTurboThresh;

                    pRc->switchCount = pRateTable->pktCountThresh;
                    /* make sure threshold rate is valid; if not, find the closest valid rate */
                    if ((pRc->validTxRateMask & (1 << targetRate))
                        || (rcGetNextValidTxRate(pRc, targetRate, &targetRate, pRateTable, 
                                                 FALSE, WLAN_PHY_TURBO)))
                    {
                        if (pRc->state[targetRate].per <= 20) {
                            pRc->recommendedPrimeState = 1;   /* recommend switch to Turbo mode */
                        }
                    }
                }
            } else {
                pRc->switchCount = 0;
            }
        } else {  /* turbo mode */
            pRc->recommendedPrimeState = 1;  /* default: don't switch */
            if (rate < pRateTable->turboToRegularThresh) {
                pRc->switchCount++;
                if (pRc->switchCount >= pRateTable->pktCountThresh) {
                    pRc->switchCount = pRateTable->pktCountThresh;
                    /* Assumption: regular is always better than Turbo in terms of sensitivity */
                    pRc->recommendedPrimeState = 0;   /* recommend switch to regular mode */
                }
            } else {
                pRc->switchCount = 0;
            }
        }
        /* Use latest recommendation.  TODO: vote */
        sc->sc_rate_recn_state = pRc->recommendedPrimeState;
    }
#endif /* TURBO_PRIME */

#ifdef notyet
    if (ic->ic_opmode == IEEE80211_M_STA) {
        /* 
         * If the user is forcing us to use XR mode, there is no
         * need to do XR rate adaptation
         */
        if (pConfig->userClist && 
            (pdevInfo->bssDescr->pChannel->channelFlags & CHANNEL_XR))
        {
            xrRateAdaptation = FALSE;
        } else {
            xrRateAdaptation = 
                VALID_ATH_ADVCAP_ELEMENT(&pdevInfo->bssDescr->athAdvCapElement) ?
                pdevInfo->bssDescr->athAdvCapElement.info.useXr : 0;
        }
    }

    /* notify XR of successfully transmitted frames */
    if (xrRateAdaptation && pdevInfo->bssDescr->opMode == WIRELESS_MODE_XR) {
        if (rate >= pConfig->xrToNormRateThresh) {
            cservXrModeChangeNotify(pdevInfo, 1);
        } else {
            cservXrRateTooLowNotify(pdevInfo);
        }
    }
#endif
    if (sc->sc_curchan.privFlags & CHANNEL_4MS_LIMIT) {
        /* Find Limiting rate and rate index to 3.8ms */
        limitRateKbps = ((A_UINT16)frameLen * 80) / 38;
        for (limitRate = minIndex;
            (pRateTable->info[limitRate].rateKbps < limitRateKbps && 
            limitRate < maxIndex); limitRate++);
        *lix = limitRate;
  
        if (pRateTable->info[rate].rateKbps < limitRateKbps) {
            /* Best rate has come down below limitRateKbps */
            rate = limitRate;
        }
    }

    PKTLOG_RATE_CTL_GET(sc, pRc->misc, *pRc, (A_UINT16)frameLen,
                        (A_UINT8)rate, (A_UINT8)0, 
                        (A_UINT8)(sc->sc_defant == 1)? 0 : 1);

    /* record selected rate, which is used to decide if we want to do fast frame */
    if (!isProbing) {
        pSib->lastRateKbps = pRateTable->info[rate].rateKbps;
    }
    return rate;
}

/*
 * This routine is called by the Tx interrupt service routine to give
 * the status of previous frames.
 */
void
rcUpdate(struct ath_softc *sc, struct ath_node *an,
    A_BOOL Xretries, int txRate, int retries, A_RSSI rssiAck, A_UINT8 curTxAnt,const  RATE_TABLE   *pRateTable)
{
    struct ieee80211com	 *ic	    = &sc->sc_ic;
    struct atheros_node	*pSib       = ATH_NODE_ATHEROS(an);
    struct TxRateCtrl_s *pRc;
    A_UINT32            nowMsec     = A_MS_TICKGET();
    A_BOOL              stateChange = FALSE;
    A_UINT8             lastPer;
    int                 rate,count;

    static A_UINT32 nRetry2PerLookup[10] = {
        100 * 0 / 1,
	100 * 1 / 8,
	100 * 1 / 2,
        100 * 3 / 4,
        100 * 4 / 5,
        100 * 5 / 6,
        100 * 6 / 7,
        100 * 7 / 8,
        100 * 8 / 9,
        100 * 9 / 10
    };


    pRc        = &pSib->txRateCtrl;

#if TURBO_PRIME
    /* TURBO_PRIME - choose the correct rate index when in turbo mode */
    if (sc->sc_curchan.channelFlags & CHANNEL_TURBO)
    {
        if (txRate < pRateTable->initialRateMax + 1) {
            rate = txRate + pRateTable->numTurboRates;

            if (pRateTable->info[txRate].rateCode != pRateTable->info[rate].rateCode) {
		int i;

                rate++;

                for (i = rate; i < pRateTable->rateCount; i ++) {
                    if (pRateTable->info[txRate].rateCode == pRateTable->info[i].rateCode) {
                        txRate = i;
                        break;
                    }
                }
            } else {
                txRate = rate;
            }
        }
    }
#endif /* TURBO_PRIME */

    lastPer = pRc->state[txRate].per;

    ASSERT(retries >= 0 && retries < MAX_TX_RETRIES);
    ASSERT(txRate >= 0 && txRate < pRc->rateTableSize);

    if (Xretries) {
        /* Update the PER. */
     if (Xretries == 1) {
        pRc->state[txRate].per += 35;
        if (pRc->state[txRate].per > 100) {
            pRc->state[txRate].per = 100;
        }
     } else {
	count = sizeof(nRetry2PerLookup) / sizeof(nRetry2PerLookup[0]);
	if (retries >= count) {
	    retries = count - 1;
	}
	/* new_PER = 3/4*old_PER + 1/4*(currentPER) */
	pRc->state[txRate].per = (A_UINT8)(pRc->state[txRate].per - (pRc->state[txRate].per >> 2) +
				 (nRetry2PerLookup[retries] >> 2));
     }
        for (rate = txRate + 1; rate < pRc->rateTableSize; rate++) {
            if (pRateTable->info[rate].phy != pRateTable->info[txRate].phy) {
                break;
            }

            if (pRc->state[rate].per < pRc->state[txRate].per) {
                pRc->state[rate].per = pRc->state[txRate].per;
            }
        }

        /* Update the RSSI threshold */
        /*
         * Oops, it didn't work at all.  Set the required rssi
         * to the rssiAck we used to lookup the rate, plus 4dB.
         * The immediate effect is we won't try this rate again
         * until we get an rssi at least 4dB higher.
         */
        if (txRate > 0) {
            A_RSSI rssi = A_MAX(pRc->rssiLastLkup, pRc->rssiLast - 2);

            if (pRc->state[txRate].rssiThres + 2 < rssi) {
                pRc->state[txRate].rssiThres += 4;
            } else if (pRc->state[txRate].rssiThres < rssi + 2) {
                pRc->state[txRate].rssiThres = rssi + 2;
            } else {
                pRc->state[txRate].rssiThres += 2;
            }

            pRc->hwMaxRetryRate = (A_UINT8)txRate;
            stateChange         = TRUE;
        }

        /*
         * Also, if we are not doing a probe, we force a significant
         * backoff by reducing rssiLast.  This doesn't have a big
         * impact since we will bump the rate back up as soon as we
         * get any good ACK.  RateMax will also be set to the current
         * txRate if the failed frame is not a probe.
         */
        if (pRc->probeRate == 0 || pRc->probeRate != txRate) {
            pRc->rssiLast      = 10 * pRc->state[txRate].rssiThres / 16;
            pRc->rssiLastPrev  = pRc->rssiLast;
            pRc->rssiLastPrev2 = pRc->rssiLast;
        }

        pRc->probeRate = 0;
    } else {
        /* Update the PER. */
        /* Make sure it doesn't index out of array's bounds. */
        count = sizeof(nRetry2PerLookup) / sizeof(nRetry2PerLookup[0]);
        if (retries >= count) {
            retries = count - 1;
        }

        /* new_PER = 7/8*old_PER + 1/8*(currentPER) */
        pRc->state[txRate].per = (A_UINT8)(pRc->state[txRate].per - (pRc->state[txRate].per >> 3) +
                                 (nRetry2PerLookup[retries] >> 3));

        /* Update the RSSI threshold */
	if (pSib->antTx != curTxAnt) {
	    /*
	     * Hw does AABBAA on transmit attempts, and has flipped on this transmit.
	     */
	    pSib->antTx = curTxAnt;	/* 0 or 1 */
	    sc->sc_stats.ast_ant_txswitch++;
	    pRc->antFlipCnt = 1;

	    if (ic->ic_opmode != IEEE80211_M_HOSTAP && !sc->sc_diversity) {
		/*
		 * Update rx ant (default) to this transmit antenna if:
		 *   1. The very first try on the other antenna succeeded and
		 *      with a very good ack rssi.
		 *   2. Or if we find ourselves succeeding for RX_FLIP_THRESHOLD
		 *      consecutive transmits on the other antenna;
		 * NOTE that the default antenna is preserved across a chip
		 *      reset by the hal software
		 */
		if (retries == 2 && rssiAck >= pRc->rssiLast + 2) {
			(*sc->sc_setdefantenna)(sc, 
				curTxAnt = curTxAnt ? 2 : 1);
		}
	    }
	} else if (ic->ic_opmode != IEEE80211_M_HOSTAP) {
	    if (!sc->sc_diversity && pRc->antFlipCnt < RX_FLIP_THRESHOLD) {
		pRc->antFlipCnt++;
		if (pRc->antFlipCnt == RX_FLIP_THRESHOLD) {
			(*sc->sc_setdefantenna)(sc,
				curTxAnt = curTxAnt ? 2 : 1);
		}
	    }
	}

        pRc->rssiLastPrev2 = pRc->rssiLastPrev;
        pRc->rssiLastPrev  = pRc->rssiLast;
        pRc->rssiLast      = rssiAck;
        pRc->rssiTime      = nowMsec;

        /*
         * If we got at most one retry then increase the max rate if
         * this was a probe.  Otherwise, ignore the probe.
         */

        if (pRc->probeRate && pRc->probeRate == txRate) {
            if (retries > 1) {
                pRc->probeRate = 0;
            } else {
                pRc->rateMax = pRc->probeRate;

                if (pRc->state[pRc->probeRate].per > 45) {
                    pRc->state[pRc->probeRate].per = 20;
                }

                pRc->probeRate = 0;

                /*
                 * Since this probe succeeded, we allow the next probe
                 * twice as soon.  This allows the maxRate to move up
                 * faster if the probes are succesful.
                 */
                pRc->probeTime = nowMsec - pRateTable->probeInterval / 2;
            }
        }

        if (retries > 0) {
            /*
             * Don't update anything.  We don't know if this was because
             * of collisions or poor signal.
             *
             * Later: if rssiAck is close to pRc->state[txRate].rssiThres
             * and we see lots of retries, then we could increase
             * pRc->state[txRate].rssiThres.
             */
        } else {
            /*
             * It worked with no retries.  First ignore bogus (small)
             * rssiAck values.
             */
            if (pRc->hwMaxRetryPktCnt < 255) {
                pRc->hwMaxRetryPktCnt++;
            }

            if (rssiAck >= pRateTable->info[txRate].rssiAckValidMin) {
                /* Average the rssi */
                if (txRate != pRc->rssiSumRate) {
                    pRc->rssiSumRate = txRate;
                    pRc->rssiSum     = pRc->rssiSumCnt = 0;
                }

                pRc->rssiSum += rssiAck;
                pRc->rssiSumCnt++;

                if (pRc->rssiSumCnt >= 4) {
                    A_RSSI32 rssiAckAvg = (pRc->rssiSum + 2) / 4;

                    pRc->rssiSum = pRc->rssiSumCnt = 0;

                    /* Now reduce the current rssi threshold. */
                    if ((rssiAckAvg < pRc->state[txRate].rssiThres + 2) &&
                        (pRc->state[txRate].rssiThres > pRateTable->info[txRate].rssiAckValidMin))
                    {
                        pRc->state[txRate].rssiThres--;
                    }

                    stateChange = TRUE;
                }
            }
        }
    }

    /*
     * If this rate looks bad (high PER) then stop using it for
     * a while (except if we are probing).
     */
    if (pRc->state[txRate].per > 60 && txRate > 0 &&
        pRateTable->info[txRate].rateKbps <= pRateTable->info[pRc->rateMax].rateKbps)
    {
        rcGetNextLowerValidTxRate(pRateTable, pRc, (A_UINT8) txRate, &pRc->rateMax);

        /* Don't probe for a little while. */
        pRc->probeTime = nowMsec;
    }

    if (stateChange) {
        /*
         * Make sure the rates above this have higher rssi thresholds.
         * (Note:  Monotonicity is kept within the OFDM rates and within the CCK rates.
         *         However, no adjustment is made to keep the rssi thresholds monotonically
         *         increasing between the CCK and OFDM rates.)
         */
        for (rate = txRate; rate < pRc->rateTableSize - 1; rate++) {
            if (pRateTable->info[rate+1].phy != pRateTable->info[txRate].phy) {
                break;
            }

            if (pRc->state[rate].rssiThres + pRateTable->info[rate].rssiAckDeltaMin >
                pRc->state[rate+1].rssiThres)
            {
                pRc->state[rate+1].rssiThres =
                    pRc->state[rate].rssiThres + pRateTable->info[rate].rssiAckDeltaMin;
            }
        }

        /* Make sure the rates below this have lower rssi thresholds. */
        for (rate = txRate - 1; rate >= 0; rate--) {
            if (pRateTable->info[rate].phy != pRateTable->info[txRate].phy) {
                break;
            }

            if (pRc->state[rate].rssiThres + pRateTable->info[rate].rssiAckDeltaMin >
                pRc->state[rate+1].rssiThres)
            {
                if (pRc->state[rate+1].rssiThres < pRateTable->info[rate].rssiAckDeltaMin) {
                    pRc->state[rate].rssiThres = 0;
                } else {
                    pRc->state[rate].rssiThres =
                        pRc->state[rate+1].rssiThres - pRateTable->info[rate].rssiAckDeltaMin;
                }

                if (pRc->state[rate].rssiThres < pRateTable->info[rate].rssiAckValidMin) {
                    pRc->state[rate].rssiThres = pRateTable->info[rate].rssiAckValidMin;
                }
            }
        }
    }

    /* Make sure the rates below this have lower PER */
    /* Monotonicity is kept only for rates below the current rate. */
    if (pRc->state[txRate].per < lastPer) {
        for (rate = txRate - 1; rate >= 0; rate--) {
            if (pRateTable->info[rate].phy != pRateTable->info[txRate].phy) {
                break;
            }

            if (pRc->state[rate].per > pRc->state[rate+1].per) {
                pRc->state[rate].per = pRc->state[rate+1].per;
            }
        }
    }

    /* Every so often, we reduce the thresholds and PER (different for CCK and OFDM). */
    if (nowMsec - pRc->rssiDownTime >= pRateTable->rssiReduceInterval) {
        for (rate = 0; rate < pRc->rateTableSize; rate++) {
            if (pRc->state[rate].rssiThres > pRateTable->info[rate].rssiAckValidMin) {
                pRc->state[rate].rssiThres -= 1;
            }
//            pRc->state[rate].per = 7*pRc->state[rate].per/8;
        }

        pRc->rssiDownTime = nowMsec;
    }
    /* Every so often, we reduce the thresholds and PER (different for CCK and OFDM). */
    if (nowMsec - pRc->perDownTime >= (pRateTable->rssiReduceInterval/2)) {
        for (rate = 0; rate < pRc->rateTableSize; rate++) {
            pRc->state[rate].per = 7*pRc->state[rate].per/8;
        }

        pRc->perDownTime = nowMsec;
    }

    PKTLOG_RATE_CTL_UPDATE(sc, *pRc, (A_UINT8)0,
                           sc->sc_curchan.channelFlags & CHANNEL_TURBO,
                           sc->sc_curchan.channelFlags & CHANNEL_TURBO,
                           pRc->rateTableSize, (A_UINT8)txRate, (A_UINT8)Xretries,
                           (A_UINT8)retries, (A_UINT16)0, rssiAck, 
                           (A_UINT8)0, (A_UINT8)pSib->antTx) ;
}

A_UINT8
rcRateValueToIndex(A_UINT32 txRateKbps, struct ath_softc *sc)
{
    struct atheros_softc *asc = (struct atheros_softc *) sc->sc_rc;
    const RATE_TABLE *pRateTable;
    A_UINT8 rate  = 0;
    A_UINT8 index = 0;

    pRateTable = asc->hwRateTable[sc->sc_curmode];

    while (rate < pRateTable->rateCount) {
        if (pRateTable->info[rate].valid &&
            txRateKbps == pRateTable->info[rate].rateKbps)
        {
            index = rate;
            break;
        }
        rate++;
    }

    return index;
}

A_UINT8
rcGetSigQuality(A_UINT8 txRate, struct ath_softc *sc, struct ath_node *an)
{
    struct atheros_node	*pSib = ATH_NODE_ATHEROS(an);
    struct atheros_softc *asc = (struct atheros_softc *) sc->sc_rc;
    const RATE_TABLE *pRateTable;
    A_UINT32     rateKbps;
    A_UINT32     maxRateKbps;
    A_UINT8      qual = 0;
    A_UINT8      per;

    pRateTable  = asc->hwRateTable[sc->sc_curmode];
    maxRateKbps = pRateTable->info[pRateTable->rateCount - 1].rateKbps;
    rateKbps    = pRateTable->info[txRate].rateKbps;
    per         = pSib->txRateCtrl.state[txRate].per;

    qual = (A_UINT8)((rateKbps * (100 - per)) / maxRateKbps);

    return qual;
}

/*
 * rcGetBestCckRate - given a current tx rate index and tx descriptor,
 * find the previous CCK rate from the working rate table.  Only
 * meaningful in 11g mode.  Returns the best CCK rate if found
 * else original rate.
 */
A_UINT16
rcGetBestCckRate(struct ath_softc *sc, struct ath_node *an, A_UINT16 curRateIndex)
{
    struct TxRateCtrl_s *pRc;
    A_UINT8             prevIndex;
    struct atheros_node	*pSib       = ATH_NODE_ATHEROS(an);
    struct atheros_softc *asc       = (struct atheros_softc *) sc->sc_rc;
    const RATE_TABLE    *pRateTable = asc->hwRateTable[sc->sc_curmode];

    pRc = &pSib->txRateCtrl;

    prevIndex = (A_UINT8)curRateIndex;
    do {
        if (pRateTable->info[prevIndex].phy == WLAN_PHY_CCK) {
            return (A_UINT16)prevIndex;
        }
    } while (rcGetPrevValidTxRate(pRc, prevIndex, &prevIndex));

    return curRateIndex;
}
