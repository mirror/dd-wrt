/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5211/ar5211_interrupts.c#2 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5211

#include "ah.h"
#include "ah_internal.h"

#include "ar5211/ar5211.h"
#include "ar5211/ar5211reg.h"

/*
 * Checks to see if an interrupt is pending on our NIC
 *
 * Returns: TRUE    if an interrupt is pending
 *          FALSE   if not
 */
HAL_BOOL
ar5211IsInterruptPending(struct ath_hal *ah)
{
	return OS_REG_READ(ah, AR_INTPEND) != 0;
}

/*
 * Reads the Interrupt Status Register value from the NIC, thus deasserting
 * the interrupt line, and returns both the masked and unmasked mapped ISR
 * values.  The value returned is mapped to abstract the hw-specific bit
 * locations in the Interrupt Status Register.
 *
 * Returns: A hardware-abstracted bitmap of all non-masked-out
 *          interrupts pending, as well as an unmasked value
 */
HAL_BOOL
ar5211GetPendingInterrupts(struct ath_hal *ah, HAL_INT *masked)
{
	u_int32_t isr;

	isr = OS_REG_READ(ah, AR_ISR_RAC);
	if (isr == 0xffffffff) {
		*masked = 0;
		return AH_FALSE;
	}

	*masked = isr & HAL_INT_COMMON;

	if (isr & AR_ISR_HIUERR)
		*masked |= HAL_INT_FATAL;
	if (isr & (AR_ISR_RXOK | AR_ISR_RXERR))
		*masked |= HAL_INT_RX;
	if (isr & (AR_ISR_TXOK | AR_ISR_TXDESC | AR_ISR_TXERR | AR_ISR_TXEOL))
		*masked |= HAL_INT_TX;
	/*
	 * Receive overrun is usually non-fatal on Oahu/Spirit.
	 *
	 * BUT early silicon had a bug which causes the rx to fail
	 * and the chip must be reset.
	 *
	 * AND even AR5311 v3.1 and AR5211 v4.2 silicon seems to
	 * have a problem with RXORN which hangs the receive path
	 * and requires a chip reset to proceed (see bug 3996).
	 * So for now, we force a hardware reset in all cases.
	 */
	if ((isr & AR_ISR_RXORN) && AH_PRIVATE(ah)->ah_rxornIsFatal) {
		HALDEBUG(ah, "%s: receive FIFO overrun interrupt\n", __func__);
		*masked |= HAL_INT_FATAL;
	}

	/*
	 * On fatal errors collect ISR state for debugging.
	 */
	if (*masked & HAL_INT_FATAL) {
		AH_PRIVATE(ah)->ah_fatalState[0] = isr;
		AH_PRIVATE(ah)->ah_fatalState[1] = OS_REG_READ(ah, AR_ISR_S0_S);
		AH_PRIVATE(ah)->ah_fatalState[2] = OS_REG_READ(ah, AR_ISR_S1_S);
		AH_PRIVATE(ah)->ah_fatalState[3] = OS_REG_READ(ah, AR_ISR_S2_S);
		AH_PRIVATE(ah)->ah_fatalState[4] = OS_REG_READ(ah, AR_ISR_S3_S);
		AH_PRIVATE(ah)->ah_fatalState[5] = OS_REG_READ(ah, AR_ISR_S4_S);
		HALDEBUG(ah, "%s: fatal error, ISR_RAC=0x%x ISR_S2_S=0x%x\n",
			__func__, isr, AH_PRIVATE(ah)->ah_fatalState[3]);
	}
	return AH_TRUE;
}

HAL_INT
ar5211GetInterrupts(struct ath_hal *ah)
{
	return AH5211(ah)->ah_maskReg;
}

/*
 * Atomically enables NIC interrupts.  Interrupts are passed in
 * via the enumerated bitmask in ints.
 */
HAL_INT
ar5211SetInterrupts(struct ath_hal *ah, HAL_INT ints)
{
	struct ath_hal_5211 *ahp = AH5211(ah);
	u_int32_t omask = ahp->ah_maskReg;
	u_int32_t mask;

	HALDEBUG(ah, "%s: 0x%x => 0x%x\n", __func__, omask, ints);

	/*
	 * Disable interrupts here before reading & modifying
	 * the mask so that the ISR does not modify the mask
	 * out from under us.
	 */
	if (omask & HAL_INT_GLOBAL) {
		HALDEBUG(ah, "%s: disable IER\n", __func__);
		OS_REG_WRITE(ah, AR_IER, AR_IER_DISABLE);
		/* XXX??? */
		(void) OS_REG_READ(ah, AR_IER);	/* flush write to HW */
	}

	mask = ints & HAL_INT_COMMON;
	if (ints & HAL_INT_TX) {
		if (ahp->ah_txOkInterruptMask)
			mask |= AR_IMR_TXOK;
		if (ahp->ah_txErrInterruptMask)
			mask |= AR_IMR_TXERR;
		if (ahp->ah_txDescInterruptMask)
			mask |= AR_IMR_TXDESC;
		if (ahp->ah_txEolInterruptMask)
			mask |= AR_IMR_TXEOL;
	}
	if (ints & HAL_INT_RX)
		mask |= AR_IMR_RXOK | AR_IMR_RXERR | AR_IMR_RXDESC;
	if (ints & HAL_INT_FATAL) {
		/*
		 * NB: ar5212Reset sets MCABT+SSERR+DPERR in AR_IMR_S2
		 *     so enabling HIUERR enables delivery.
		 */
		mask |= AR_IMR_HIUERR;
	}

	/* Write the new IMR and store off our SW copy. */
	HALDEBUG(ah, "%s: new IMR 0x%x\n", __func__, mask);
	OS_REG_WRITE(ah, AR_IMR, mask);
	ahp->ah_maskReg = ints;

	/* Re-enable interrupts as appropriate. */
	if (ints & HAL_INT_GLOBAL) {
		HALDEBUG(ah, "%s: enable IER\n", __func__);
		OS_REG_WRITE(ah, AR_IER, AR_IER_ENABLE);
	}

	return omask;
}
#endif /* AH_SUPPORT_AR5211 */
