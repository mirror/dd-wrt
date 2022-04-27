/*
 * Copyright (c) 2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_interrupts.c#1 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_internal.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"

/*
 * Checks to see if an interrupt is pending on our NIC
 *
 * Returns: TRUE    if an interrupt is pending
 *          FALSE   if not
 */
HAL_BOOL
ar5416IsInterruptPending(struct ath_hal *ah)
{
	u_int32_t isr = OS_REG_READ(ah, AR_INTR_ASYNC_CAUSE);
	/* 
	 * Some platforms trigger our ISR before applying power to
	 * the card, so make sure the INTPEND is really 1, not 0xffffffff.
	 */
	return (isr != AR_INTR_SPURIOUS && (isr & AR_INTR_MAC_IRQ) != 0);
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
ar5416GetPendingInterrupts(struct ath_hal *ah, HAL_INT *masked)
{
	u_int32_t isr, isr0, isr1;
	u_int32_t mask2=0;
	struct ath_hal_5212 *ahp = AH5212(ah);

	isr = OS_REG_READ(ah, AR_ISR);
	if (isr & AR_ISR_BCNMISC) {
		u_int32_t isr2;
		isr2 = OS_REG_READ(ah, AR_ISR_S2);
		if (isr2 & AR_ISR_S2_TIM)
			mask2 |= HAL_INT_TIM;
		if (isr2 & AR_ISR_S2_DTIM)
			mask2 |= HAL_INT_DTIM;
		if (isr2 & AR_ISR_S2_DTIMSYNC)
			mask2 |= HAL_INT_DTIMSYNC;
		if (isr2 & (AR_ISR_S2_CABEND ))
			mask2 |= HAL_INT_CABEND;
		if (isr2 & AR_ISR_S2_GTT)
			mask2 |= HAL_INT_GTT;
		if (isr2 & AR_ISR_S2_CST)
			mask2 |= HAL_INT_CST;	
	}
	isr = OS_REG_READ(ah, AR_ISR_RAC);

	if (isr == 0xffffffff) {
		*masked = 0;
		return AH_FALSE;;
	}

	*masked = isr & HAL_INT_COMMON;

	/* 
	 * WAR 23803: On Owl 2.0, treat some BMISS as fatal to reset chip except
	 * when asserted with RXEOL which happens when driver stops.
	 * Check for baseband hang before setting HAL_INT_FATAL.
	 */
	if ((isr & (AR_ISR_BMISS|AR_ISR_RXEOL)) == AR_ISR_BMISS) {
		if (ar5416DetectBbHang(ah, AH_FALSE))
			*masked |= HAL_INT_FATAL;
	}

	if (isr & AR_ISR_HIUERR)
		*masked |= HAL_INT_FATAL;
	if (isr & (AR_ISR_RXOK | AR_ISR_RXERR))
		*masked |= HAL_INT_RX;
	if (isr & (AR_ISR_TXOK | AR_ISR_TXDESC | AR_ISR_TXERR | AR_ISR_TXEOL)) {
		*masked |= HAL_INT_TX;
		isr0 = OS_REG_READ(ah, AR_ISR_S0_S);
		ahp->ah_intrTxqs |= MS(isr0, AR_ISR_S0_QCU_TXOK);
		ahp->ah_intrTxqs |= MS(isr0, AR_ISR_S0_QCU_TXDESC);
		isr1 = OS_REG_READ(ah, AR_ISR_S1_S);
		ahp->ah_intrTxqs |= MS(isr1, AR_ISR_S1_QCU_TXERR);
		ahp->ah_intrTxqs |= MS(isr1, AR_ISR_S1_QCU_TXEOL);
	}

	/* Interrupt Mitigation on AR5416 */
#ifdef AR5416_INT_MITIGATION
	if (isr & (AR_ISR_RXMINTR | AR_ISR_RXINTM))
		*masked |= HAL_INT_RX;
	if (isr & (AR_ISR_TXMINTR | AR_ISR_TXINTM))
		*masked |= HAL_INT_TX;
#endif
	*masked |= mask2;

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

/*
 * Atomically enables NIC interrupts.  Interrupts are passed in
 * via the enumerated bitmask in ints.
 */
HAL_INT
ar5416SetInterrupts(struct ath_hal *ah, HAL_INT ints)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	u_int32_t omask = ahp->ah_maskReg;
	u_int32_t mask,mask2;

	HALDEBUG(ah, "%s: 0x%x => 0x%x\n", __func__, omask, ints);

	if (omask & HAL_INT_GLOBAL) {
		HALDEBUG(ah, "%s: disable IER\n", __func__);
		OS_REG_WRITE(ah, AR_IER, AR_IER_DISABLE);
		(void) OS_REG_READ(ah, AR_IER);   /* flush write to HW */
	}

	mask = ints & HAL_INT_COMMON;
	mask2 = 0;

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
#ifdef AR5416_INT_MITIGATION
	/*
	 * Overwrite default mask if Interrupt mitigation
	 * is specified for AR5416
	 */
	mask = ints & HAL_INT_COMMON;
	if (ints & HAL_INT_TX)
		mask |= AR_IMR_TXMINTR | AR_IMR_TXINTM;
	if (ints & HAL_INT_RX)
		mask |= AR_IMR_RXERR | AR_IMR_RXMINTR | AR_IMR_RXINTM;
#endif
	if (ints & (HAL_INT_BMISC)) {
		mask |= AR_IMR_BCNMISC;
		if (ints & HAL_INT_TIM)
			mask2 |= AR_IMR_S2_TIM;
		if (ints & HAL_INT_DTIM)
			mask2 |= AR_IMR_S2_DTIM;
		if (ints & HAL_INT_DTIMSYNC)
			mask2 |= AR_IMR_S2_DTIMSYNC;
		if (ints & HAL_INT_CABEND)
			mask2 |= (AR_IMR_S2_CABEND );
		if (ints & HAL_INT_GTT)
			mask2 |= AR_IMR_S2_GTT;			
		if (ints & HAL_INT_CST)
			mask2 |= AR_IMR_S2_CST;
	}
	if (ints & HAL_INT_FATAL) {
		/*
		 * NB: ar5416Reset sets MCABT+SSERR+DPERR in AR_IMR_S2
		 *     so enabling HIUERR enables delivery.
		 */
		mask |= AR_IMR_HIUERR;
	}

	/* Write the new IMR and store off our SW copy. */
	HALDEBUG(ah, "%s: new IMR 0x%x\n", __func__, mask);
	OS_REG_WRITE(ah, AR_IMR, mask);
	mask = OS_REG_READ(ah, AR_IMR_S2) & ~(AR_IMR_S2_TIM |
					AR_IMR_S2_DTIM |
					AR_IMR_S2_DTIMSYNC |
					AR_IMR_S2_CABEND |
					AR_IMR_S2_CABTO  |
					AR_IMR_S2_TSFOOR |
					AR_IMR_S2_GTT |
					AR_IMR_S2_CST);
 
	OS_REG_WRITE(ah, AR_IMR_S2, mask | mask2 );

	ahp->ah_maskReg = ints;

	/* Re-enable interrupts if they were enabled before. */
	if (ints & HAL_INT_GLOBAL) {
		HALDEBUG(ah, "%s: enable IER\n", __func__);
		OS_REG_WRITE(ah, AR_IER, AR_IER_ENABLE);
	}

	OS_REG_WRITE(ah, AR_INTR_ASYNC_ENABLE, AR_INTR_MAC_IRQ);
	OS_REG_WRITE(ah, AR_INTR_ASYNC_MASK, AR_INTR_MAC_IRQ);

	/*
	 * debug - enable to see all synchronous interrupts status
	 */
	OS_REG_WRITE(ah, AR_INTR_SYNC_ENABLE, AR_INTR_SYNC_ALL);
	return omask;
}
#endif /* AH_SUPPORT_AR5416 */
