/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5211/ar5211_recv.c#2 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5211

#include "ah.h"
#include "ah_internal.h"
#include "ah_desc.h"

#include "ar5211/ar5211.h"
#include "ar5211/ar5211reg.h"
#include "ar5211/ar5211desc.h"

/*
 * Get the RXDP.
 */
u_int32_t
ar5211GetRxDP(struct ath_hal *ah)
{
	return OS_REG_READ(ah, AR_RXDP);
}

/*
 * Set the RxDP.
 */
void
ar5211SetRxDP(struct ath_hal *ah, u_int32_t rxdp)
{
	OS_REG_WRITE(ah, AR_RXDP, rxdp);
	HALASSERT(OS_REG_READ(ah, AR_RXDP) == rxdp);
}


/*
 * Set Receive Enable bits.
 */
void
ar5211EnableReceive(struct ath_hal *ah)
{
	OS_REG_WRITE(ah, AR_CR, AR_CR_RXE);
}

/*
 * Stop Receive at the DMA engine
 */
HAL_BOOL
ar5211StopDmaReceive(struct ath_hal *ah)
{
	OS_REG_WRITE(ah, AR_CR, AR_CR_RXD);	/* Set receive disable bit */
	if (!ath_hal_wait(ah, AR_CR, AR_CR_RXE, 0)) {
#ifdef AH_DEBUG
		HALDEBUG(ah, "%s failed to stop in 10ms\n"
				   "AR_CR=0x%08X\nAR_DIAG_SW=0x%08X\n"
				   , __func__
				   , OS_REG_READ(ah, AR_CR)
				   , OS_REG_READ(ah, AR_DIAG_SW)
		);
#endif
		return AH_FALSE;
	} else {
		return AH_TRUE;
	}
}

/*
 * Start Transmit at the PCU engine (unpause receive)
 */
void
ar5211StartPcuReceive(struct ath_hal *ah)
{
	OS_REG_WRITE(ah, AR_DIAG_SW,
		OS_REG_READ(ah, AR_DIAG_SW) & ~(AR_DIAG_SW_DIS_RX));
}

/*
 * Stop Transmit at the PCU engine (pause receive)
 */
void
ar5211StopPcuReceive(struct ath_hal *ah)
{
	OS_REG_WRITE(ah, AR_DIAG_SW,
		OS_REG_READ(ah, AR_DIAG_SW) | AR_DIAG_SW_DIS_RX);
}

/*
 * Set multicast filter 0 (lower 32-bits)
 *			   filter 1 (upper 32-bits)
 */
void
ar5211SetMulticastFilter(struct ath_hal *ah, u_int32_t filter0, u_int32_t filter1)
{
	OS_REG_WRITE(ah, AR_MCAST_FIL0, filter0);
	OS_REG_WRITE(ah, AR_MCAST_FIL1, filter1);
}

/*
 * Clear multicast filter by index
 */
HAL_BOOL
ar5211ClrMulticastFilterIndex(struct ath_hal *ah, u_int32_t ix)
{
	u_int32_t val;

	if (ix >= 64)
		return AH_FALSE;
	if (ix >= 32) {
		val = OS_REG_READ(ah, AR_MCAST_FIL1);
		OS_REG_WRITE(ah, AR_MCAST_FIL1, (val &~ (1<<(ix-32))));
	} else {
		val = OS_REG_READ(ah, AR_MCAST_FIL0);
		OS_REG_WRITE(ah, AR_MCAST_FIL0, (val &~ (1<<ix)));
	}
	return AH_TRUE;
}

/*
 * Set multicast filter by index
 */
HAL_BOOL
ar5211SetMulticastFilterIndex(struct ath_hal *ah, u_int32_t ix)
{
	u_int32_t val;

	if (ix >= 64)
		return AH_FALSE;
	if (ix >= 32) {
		val = OS_REG_READ(ah, AR_MCAST_FIL1);
		OS_REG_WRITE(ah, AR_MCAST_FIL1, (val | (1<<(ix-32))));
	} else {
		val = OS_REG_READ(ah, AR_MCAST_FIL0);
		OS_REG_WRITE(ah, AR_MCAST_FIL0, (val | (1<<ix)));
	}
	return AH_TRUE;
}

/*
 * Get receive filter.
 */
u_int32_t
ar5211GetRxFilter(struct ath_hal *ah)
{
	return OS_REG_READ(ah, AR_RX_FILTER);
}

/*
 * Set receive filter.
 */
void
ar5211SetRxFilter(struct ath_hal *ah, u_int32_t bits)
{
	OS_REG_WRITE(ah, AR_RX_FILTER, bits);
}

/*
 * Initialize RX descriptor, by clearing the status and clearing
 * the size.  This is not strictly HW dependent, but we want the
 * control and status words to be opaque above the hal.
 */
HAL_BOOL
ar5211SetupRxDesc(struct ath_hal *ah, struct ath_desc *ds,
	u_int32_t size, u_int flags)
{
	struct ar5211_desc *ads = AR5211DESC(ds);

	ads->ds_ctl0 = 0;
	ads->ds_ctl1 = size & AR_BufLen;
	if (ads->ds_ctl1 != size) {
		HALDEBUG(ah, "%s: buffer size %u too large\n",
			__func__, size);
		return AH_FALSE;
	}
	if (flags & HAL_RXDESC_INTREQ)
		ads->ds_ctl1 |= AR_RxInterReq;
	ads->ds_status0 = ads->ds_status1 = 0;

	return AH_TRUE;
}

/*
 * Process an RX descriptor, and return the status to the caller.
 * Copy some hardware specific items into the software portion
 * of the descriptor.
 *
 * NB: the caller is responsible for validating the memory contents
 *     of the descriptor (e.g. flushing any cached copy).
 */
HAL_STATUS
ar5211ProcRxDesc(struct ath_hal *ah, struct ath_desc *ds,
	u_int32_t pa, struct ath_desc *nds, u_int64_t tsf,
	struct ath_rx_status *rs)
{
	struct ar5211_desc *ads = AR5211DESC(ds);
	struct ar5211_desc *ands = AR5211DESC(nds);

	if ((ads->ds_status1 & AR_Done) == 0)
		return HAL_EINPROGRESS;
	/*
	 * Given the use of a self-linked tail be very sure that the hw is
	 * done with this descriptor; the hw may have done this descriptor
	 * once and picked it up again...make sure the hw has moved on.
	 */
	if ((ands->ds_status1 & AR_Done) == 0 && OS_REG_READ(ah, AR_RXDP) == pa)
		return HAL_EINPROGRESS;

	rs->rs_datalen = ads->ds_status0 & AR_DataLen;
	rs->rs_tstamp = MS(ads->ds_status1, AR_RcvTimestamp);
	rs->rs_status = 0;
	if ((ads->ds_status1 & AR_FrmRcvOK) == 0) {
		if (ads->ds_status1 & AR_CRCErr)
			rs->rs_status |= HAL_RXERR_CRC;
		else if (ads->ds_status1 & AR_DecryptCRCErr)
			rs->rs_status |= HAL_RXERR_DECRYPT;
		else {
			rs->rs_status |= HAL_RXERR_PHY;
			rs->rs_phyerr = MS(ads->ds_status1, AR_PHYErr);
		}
	}
	/* XXX what about KeyCacheMiss? */
	rs->rs_rssi = MS(ads->ds_status0, AR_RcvSigStrength);
	if (ads->ds_status1 & AR_KeyIdxValid)
		rs->rs_keyix = MS(ads->ds_status1, AR_KeyIdx);
	else
		rs->rs_keyix = HAL_RXKEYIX_INVALID;
	/* NB: caller expected to do rate table mapping */
	rs->rs_rate = MS(ads->ds_status0, AR_RcvRate);
	rs->rs_antenna  = MS(ads->ds_status0, AR_RcvAntenna);
	rs->rs_more = (ads->ds_status0 & AR_More) ? 1 : 0;

	return HAL_OK;
}
#endif /* AH_SUPPORT_AR5211 */
