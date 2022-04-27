/*
 * Copyright (c) 2002-2005 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_misc.c#3 $
 */
 
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"
#ifdef AH_DEBUG
#include "ah_desc.h"                    /* NB: for HAL_PHYERR* */
#endif

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"

/*
 * Return the wireless modes (a,b,g,t) supported by hardware.
 *
 * This value is what is actually supported by the hardware
 * and is unaffected by regulatory/country code settings.
 *
 */
u_int
ar5416GetWirelessModes(struct ath_hal *ah)
{
	u_int mode;

	mode = ar5212GetWirelessModes(ah);
	if (mode & HAL_MODE_11A)
		mode |= HAL_MODE_11NA_HT20
		     |  HAL_MODE_11NA_HT40PLUS
		     |  HAL_MODE_11NA_HT40MINUS
		     ;
	if (mode & HAL_MODE_11G)
		mode |= HAL_MODE_11NG_HT20
		     |  HAL_MODE_11NG_HT40PLUS
		     |  HAL_MODE_11NG_HT40MINUS
		     ;
	return mode;
}

/*
 * Change the LED blinking pattern to correspond to the connectivity
 */
void
ar5416SetLedState(struct ath_hal *ah, HAL_LED_STATE state)
{
	static const u_int32_t ledbits[8] = {
		AR_MAC_LED_ASSOC_NONE,		/* HAL_LED_INIT */
		AR_MAC_LED_ASSOC_PENDING,	/* HAL_LED_SCAN */
		AR_MAC_LED_ASSOC_PENDING,	/* HAL_LED_AUTH */
		AR_MAC_LED_ASSOC_ACTIVE,	/* HAL_LED_ASSOC*/
		AR_MAC_LED_ASSOC_ACTIVE,	/* HAL_LED_RUN */
		AR_MAC_LED_ASSOC_NONE,
		AR_MAC_LED_ASSOC_NONE,
		AR_MAC_LED_ASSOC_NONE,
	};
	u_int32_t bits;

	bits = OS_REG_READ(ah, AR_MAC_LED);
	bits = (bits &~ AR_MAC_LED_MODE)
	     | SM(AR_MAC_LED_MODE_POWON, AR_MAC_LED_MODE)
#if 1
	     | SM(AR_MAC_LED_MODE_NETON, AR_MAC_LED_MODE)
#endif
	     ;
	bits = (bits &~ AR_MAC_LED_ASSOC)
	     | SM(ledbits[state & 0x7], AR_MAC_LED_ASSOC);
	OS_REG_WRITE(ah, AR_MAC_LED, bits);
}

/*
 * Reset the current hardware tsf for stamlme.
 */
void
ar5416ResetTsf(struct ath_hal *ah)
{
	u_int32_t v;
	int i;

	for (i = 0; i < 10; i++) {
		v = OS_REG_READ(ah, AR_SLP32_MODE);
		if ((v & AR_SLP32_TSF_WRITE_STATUS) == 0)
			break;
		OS_DELAY(10);
	}
	OS_REG_WRITE(ah, AR_RESET_TSF, AR_RESET_TSF_ONCE);	
}

HAL_BOOL
ar5416SetAntennaSwitch(struct ath_hal *ah, HAL_ANT_SETTING settings)
{
	return AH_TRUE;
}

/* Setup decompression for given key index */
HAL_BOOL
ar5416SetDecompMask(struct ath_hal *ah, u_int16_t keyidx, int en)
{
	return HAL_OK;
}

/* Setup coverage class */
void
ar5416SetCoverageClass(struct ath_hal *ah, u_int8_t coverageclass, int now)
{
}

/*
 * Read the CCA threshold from the baseband
 *
 * thresh62[0] is CCA threshold for control channel
 * thresh62[1] is CCA threshold for extension channel
 */
void
ar5416GetCCAThreshold(struct ath_hal *ah, int8_t thresh62[2])
{
	thresh62[0] = MS(OS_REG_READ(ah, AR_PHY_CCA), AR_PHY_CCA_THRESH62);
	thresh62[1] = MS(OS_REG_READ(ah, AR_PHY_EXT_CCA), AR_PHY_EXT_CCA_THRESH62);
}

/*
 * Set the CCA threshold in the baseband
 *
 * thresh62[0] is CCA threshold for control channel
 * thresh62[1] is CCA threshold for extension channel
 */  
HAL_BOOL
ar5416SetCCAThreshold(struct ath_hal *ah, const int8_t thresh62[2])
{
#if 0
	/* XXX config option to enable support */
	OS_REG_RMW_FIELD(ah, AR_PHY_CCA, AR_PHY_CCA_THRESH62, thresh62[0]);
	OS_REG_RMW_FIELD(ah, AR_PHY_EXT_CCA, AR_PHY_EXT_CCA_THRESH62, thresh62[1]);
#endif
	return AH_FALSE;
}

/*
 * Return approximation of extension channel busy over an time interval
 * 0% (clear) -> 100% (busy)
 *
 */
u_int32_t
ar5416Get11nExtBusy(struct ath_hal *ah)
{
    struct ath_hal_5416 *ahp = AH5416(ah);
    u_int32_t busy; /* percentage */
    u_int32_t cycleCount, ctlBusy, extBusy;

    ctlBusy = OS_REG_READ(ah, AR_RCCNT);
    extBusy = OS_REG_READ(ah, AR_EXTRCCNT);
    cycleCount = OS_REG_READ(ah, AR_CCCNT);

    if (ahp->ah_cycleCount == 0 || ahp->ah_cycleCount > cycleCount) {
        /*
         * Cycle counter wrap (or initial call); it's not possible
         * to accurately calculate a value because the registers
         * right shift rather than wrap--so punt and return 0.
         */
        busy = 0;
        HALDEBUG(ah, "%s: cycle counter wrap. ExtBusy = 0\n", __func__);

    } else {
        u_int32_t cycleDelta = cycleCount - ahp->ah_cycleCount;
        u_int32_t ctlBusyDelta = ctlBusy - ahp->ah_ctlBusy;
        u_int32_t extBusyDelta = extBusy - ahp->ah_extBusy;
        u_int32_t ctlClearDelta = 0;

        /* Compute control channel rxclear.
         * The cycle delta may be less than the control channel delta.
         * This could be solved by freezing the timers (or an atomic read,
         * if one was available). Checking for the condition should be
         * sufficient.
         */
        if (cycleDelta > ctlBusyDelta) {
            ctlClearDelta = cycleDelta - ctlBusyDelta;
        }

        /* Compute ratio of extension channel busy to control channel clear
         * as an approximation to extension channel cleanliness.
         *
         * According to the hardware folks, ext rxclear is undefined
         * if the ctrl rxclear is de-asserted (i.e. busy)
         */
        if (ctlClearDelta) {
            busy = (extBusyDelta * 100) / ctlClearDelta;
        } else {
            busy = 100;
        }
        if (busy > 100) {
            busy = 100;
        }
#if 0
        HALDEBUG(ah, "%s: cycleDelta 0x%x, ctlBusyDelta 0x%x, "
             "extBusyDelta 0x%x, ctlClearDelta 0x%x, "
             "busy %d\n",
              __func__, cycleDelta, ctlBusyDelta, extBusyDelta, ctlClearDelta, busy);
#endif
    }

    ahp->ah_cycleCount = cycleCount;
    ahp->ah_ctlBusy = ctlBusy;
    ahp->ah_extBusy = extBusy;

    return busy;
}

/*
 * Configure 20/40 operation
 *
 * 20/40 = joint rx clear (control and extension)
 * 20    = rx clear (control)
 *
 * - NOTE: must stop MAC (tx) and requeue 40 MHz packets as 20 MHz when changing
 *         from 20/40 => 20 only
 */
void
ar5416Set11nMac2040(struct ath_hal *ah, HAL_HT_MACMODE mode)
{
    u_int32_t macmode;

    /* Configure MAC for 20/40 operation */
    if (mode == HAL_HT_MACMODE_2040) {
        macmode = AR_2040_JOINED_RX_CLEAR;
    } else {
        macmode = 0;
    }
    OS_REG_WRITE(ah, AR_2040_MODE, macmode);
}

/*
 * Get Rx clear (control/extension channel)
 *
 * Returns active low (busy) for ctrl/ext channel
 * Owl 2.0
 */
HAL_HT_RXCLEAR
ar5416Get11nRxClear(struct ath_hal *ah)
{
    HAL_HT_RXCLEAR rxclear = 0;
    u_int32_t val;

    val = OS_REG_READ(ah, AR_DIAG_SW);

    /* control channel */
    if (val & AR_DIAG_RX_CLEAR_CTL_LOW) {
        rxclear |= HAL_RX_CLEAR_CTL_LOW;
    }
    /* extension channel */
    if (val & AR_DIAG_RX_CLEAR_CTL_LOW) {
        rxclear |= HAL_RX_CLEAR_EXT_LOW;
    }
    return rxclear;
}

/*
 * Set Rx clear (control/extension channel)
 *
 * Useful for forcing the channel to appear busy for
 * debugging/diagnostics
 * Owl 2.0
 */
void
ar5416Set11nRxClear(struct ath_hal *ah, HAL_HT_RXCLEAR rxclear)
{
    /* control channel */
    if (rxclear & HAL_RX_CLEAR_CTL_LOW) {
        OS_REG_SET_BIT(ah, AR_DIAG_SW, AR_DIAG_RX_CLEAR_CTL_LOW);
    } else {
        OS_REG_CLR_BIT(ah, AR_DIAG_SW, AR_DIAG_RX_CLEAR_CTL_LOW);
    }
    /* extension channel */
    if (rxclear & HAL_RX_CLEAR_EXT_LOW) {
        OS_REG_SET_BIT(ah, AR_DIAG_SW, AR_DIAG_RX_CLEAR_EXT_LOW);
    } else {
        OS_REG_CLR_BIT(ah, AR_DIAG_SW, AR_DIAG_RX_CLEAR_EXT_LOW);
    }
}

HAL_STATUS
ar5416GetCapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t *result)
{
	switch (type) {
	case HAL_CAP_CHAINMASK:
		switch(capability) {
		case 0: /* RX */
			*result = AH5416(ah)->ah_rx_chainmask;
			break;
		case 1: /* TX */
			*result = AH5416(ah)->ah_tx_chainmask;
			break;
		}
		break;
	default:
		return ar5212GetCapability(ah, type, capability, result);
	}
	return HAL_OK;
}

HAL_BOOL
ar5416SetCapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t setting, HAL_STATUS *status)
{
	switch (type) {
	case HAL_CAP_CHAINMASK:
		switch(capability) {
		case 0: /* RX */
			AH5416(ah)->ah_rx_chainmask = setting;
			break;
		case 1: /* TX */
			AH5416(ah)->ah_tx_chainmask = setting;
			break;
		}
		break;
	default:
		return ar5212SetCapability(ah, type, capability,
				setting, status);
	}
	return AH_TRUE;
}


/* Determine if the baseband is hung by reading the Observation Bus Register */
HAL_BOOL
ar5416DetectBbHang(struct ath_hal *ah, HAL_BOOL dfs_hang_check)
{
#define NUM_STATUS_READS 50
    u_int32_t hang_sig = 0;
    int i=0, hang_sig1=0, hang_sig2=0, hang_sig3=0;
    static int hang_sig1_count=0, hang_sig2_count=0, hang_sig3_count=0;

    /* Check the PCU Observation Bus 1 register (0x806c) NUM_STATUS_READS times
     *
     * 3 hang signatures -
     * [1] bits 8,9,11 are 0. State machine state (bits 25-31) is 0x1E
     * [2] bits 8,9 are 1, bit 11 is 0. State machine state (bits 25-31) is 0x52
     * [3] bits 8,9 are 1, bit 11 is 0. State machine state (bits 25-31) is 0x18
     */
#define HAL_HANG_SIG_1 0x1E000000
#define HAL_HANG_SIG_2 0x52000B00
#define HAL_HANG_SIG_3 0x18000B00
#define HAL_HANG_SIG_MASK 0x7E000B00

    for (i=1; i <= NUM_STATUS_READS; i++) {
        hang_sig = OS_REG_READ(ah, AR_OBS_BUS_1) & HAL_HANG_SIG_MASK;

        if (hang_sig == HAL_HANG_SIG_1) {
               hang_sig1++;
        }
        if (dfs_hang_check == AH_TRUE &&
            (hang_sig == HAL_HANG_SIG_2)) {
               hang_sig2++;
        }
        if (dfs_hang_check == AH_TRUE &&
            (hang_sig == HAL_HANG_SIG_3)) {
               hang_sig3++;
        }

        /* Either the hang signature never existed, or it got cleared */
        if ((hang_sig1 != i) && (hang_sig2 != i) && (hang_sig3 != i)) {
                return AH_FALSE;
        }
    }

    /* If we have got to this point, the hang signature has been
     * consistently seen for NUM_STATUS_READS and we are in a potential
     * hang situation
     */
    if (hang_sig1==NUM_STATUS_READS) hang_sig1_count++;
    if (hang_sig2==NUM_STATUS_READS) hang_sig2_count++;
    if (hang_sig3==NUM_STATUS_READS) hang_sig3_count++;

    ath_hal_printf(ah, "%s <0x806c>=0x%x sig1count=%d sig2count=%d "
             "sig3count=%d\n", __func__, hang_sig, hang_sig1_count,
             hang_sig2_count, hang_sig3_count);

    return AH_TRUE;

#undef HAL_HANG_SIG_1
#undef HAL_HANG_SIG_2
#undef HAL_HANG_SIG_3
#undef HAL_HANG_SIG_MASK
#undef NUM_STATUS_READS
} /* end - ar5416DetectBbHang () */

#endif /* AH_SUPPORT_AR5416 */
