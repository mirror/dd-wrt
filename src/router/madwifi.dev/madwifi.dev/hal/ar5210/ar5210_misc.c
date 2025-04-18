/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5210/ar5210_misc.c#6 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5210

#include "ah.h"
#include "ah_internal.h"

#include "ar5210/ar5210.h"
#include "ar5210/ar5210reg.h"
#include "ar5210/ar5210phy.h"

#define	AR_NUM_GPIO	6		/* 6 GPIO bits */
#define	AR_GPIOD_MASK	0x2f		/* 6-bit mask */

void
ar5210GetMacAddress(struct ath_hal *ah, u_int8_t *mac)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	OS_MEMCPY(mac, ahp->ah_macaddr, IEEE80211_ADDR_LEN);
}

HAL_BOOL
ar5210SetMacAddress(struct ath_hal *ah, const u_int8_t *mac)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	OS_MEMCPY(ahp->ah_macaddr, mac, IEEE80211_ADDR_LEN);
	return AH_TRUE;
}

void
ar5210GetBssIdMask(struct ath_hal *ah, u_int8_t *mask)
{
	static const u_int8_t ones[IEEE80211_ADDR_LEN] =
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	OS_MEMCPY(mask, ones, IEEE80211_ADDR_LEN);
}

HAL_BOOL
ar5210SetBssIdMask(struct ath_hal *ah, const u_int8_t *mask)
{
	return AH_FALSE;
}

/*
 * Read 16 bits of data from the specified EEPROM offset.
 */
HAL_BOOL
ar5210EepromRead(struct ath_hal *ah, u_int off, u_int16_t *data)
{
	if (ah->ah_eepromRead &&
		ah->ah_eepromRead(ah, off, data))
		return AH_TRUE;

	(void) OS_REG_READ(ah, AR_EP_AIR(off));	/* activate read op */
	if (!ath_hal_wait(ah, AR_EP_STA,
	    AR_EP_STA_RDCMPLT | AR_EP_STA_RDERR, AR_EP_STA_RDCMPLT)) {
		HALDEBUG(ah, "%s: read failed for entry 0x%x\n", __func__,
			AR_EP_AIR(off));
		return AH_FALSE;
	}
	*data = OS_REG_READ(ah, AR_EP_RDATA) & 0xffff;
	return AH_TRUE;
}

#ifdef AH_SUPPORT_WRITE_EEPROM
/*
 * Write 16 bits of data to the specified EEPROM offset.
 */
HAL_BOOL
ar5210EepromWrite(struct ath_hal *ah, u_int off, u_int16_t data)
{
	OS_REG_WRITE(ah, AR_EP_AIR(off), data);	/* active write op */
	if (!ath_hal_wait(ah, AR_EP_STA,
	    AR_EP_STA_WRCMPLT | AR_EP_STA_WRERR, AR_EP_STA_WRCMPLT)) {
		HALDEBUG(ah, "%s: write failed for entry 0x%x, data 0x%x\n",
			__func__, AR_EP_AIR(off), data);
		return AH_FALSE;
	}
	return AH_TRUE;
}
#endif /* AH_SUPPORT_WRITE_EEPROM */

/*
 * Attempt to change the cards operating regulatory domain to the given value
 * Returns: A_EINVAL for an unsupported regulatory domain.
 *          A_HARDWARE for an unwritable EEPROM or bad EEPROM version
 */
HAL_BOOL
ar5210SetRegulatoryDomain(struct ath_hal *ah,
	u_int16_t regDomain, HAL_STATUS *status)
{
	struct ath_hal_5210 *ahp = AH5210(ah);
#ifdef AH_SUPPORT_WRITE_REGDOMAIN
	u_int32_t pcicfg;
#endif
	HAL_STATUS ecode;

	if (AH_PRIVATE(ah)->ah_currentRD == regDomain) {
		ecode = HAL_EINVAL;
		goto bad;
	}
	/*
	 * Check if EEPROM is configured to allow this; must
	 * be a proper version and the protection bits must
	 * permit re-writing that segment of the EEPROM.
	 */
	HALASSERT(ahp->ah_eeversion == 1);
	if (ahp->ah_eeprotect & AR_EEPROM_PROTOTECT_WP_128_191) {
		ecode = HAL_EEWRITE;
		goto bad;
	}
#ifdef AH_SUPPORT_WRITE_REGDOMAIN
	/*
	 * Enable the EEPROM for writing and set the value.
	 */
	pcicfg = OS_REG_READ(ah, AR_PCICFG);
	OS_REG_WRITE(ah, AR_PCICFG, pcicfg | AR_PCICFG_EEPROMSEL);

	if (ar5210EepromWrite(ah, AR_EEPROM_REG_DOMAIN, regDomain)) {
		HALDEBUG(ah, "%s: set regulatory domain to %u (0x%x)\n",
			__func__, regDomain, regDomain);
		AH_PRIVATE(ah)->ah_currentRD = regDomain;
		ecode = HAL_OK;
	} else {
		ecode = HAL_EIO;
	}

	OS_REG_WRITE(ah, AR_PCICFG, pcicfg);

	if (ecode == HAL_OK)
		return AH_TRUE;
	/* fall thru... */
#else
	ecode = HAL_EIO;		/* disallow all writes */
#endif
bad:
	if (status)
		*status = ecode;
	return AH_FALSE;
}

/*
 * Return the wireless modes (a,b,g,t) supported by hardware.
 *
 * This value is what is actually supported by the hardware
 * and is unaffected by regulatory/country code settings.
 *
 */
u_int
ar5210GetWirelessModes(struct ath_hal *ah)
{
	/* XXX could enable turbo mode but can't do all rates */
	return HAL_MODE_11A;
}

/*
 * Called if RfKill is supported (according to EEPROM).  Set the interrupt and
 * GPIO values so the ISR and can disable RF on a switch signal
 */
void
ar5210EnableRfKill(struct ath_hal *ah)
{
	u_int16_t rfsilent = AH_PRIVATE(ah)->ah_rfsilent;
	int select = MS(rfsilent, AR_EEPROM_RFSILENT_GPIO_SEL);
	int polarity = MS(rfsilent, AR_EEPROM_RFSILENT_POLARITY);

	/*
	 * If radio disable switch connection to GPIO bit 0 is enabled
	 * program GPIO interrupt.
	 * If rfkill bit on eeprom is 1, setupeeprommap routine has already
	 * verified that it is a later version of eeprom, it has a place for
	 * rfkill bit and it is set to 1, indicating that GPIO bit 0 hardware
	 * connection is present.
	 */
	ar5210Gpio0SetIntr(ah, select, (ar5210GpioGet(ah, select) == polarity));
}

/*
 * Configure GPIO Output lines
 */
HAL_BOOL
ar5210GpioCfgOutput(struct ath_hal *ah, u_int32_t gpio)
{
	HALASSERT(gpio < AR_NUM_GPIO);

	OS_REG_WRITE(ah, AR_GPIOCR, 
		  (OS_REG_READ(ah, AR_GPIOCR) &~ AR_GPIOCR_ALL(gpio))
		| AR_GPIOCR_OUT1(gpio));

	return AH_TRUE;
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL
ar5210GpioCfgInput(struct ath_hal *ah, u_int32_t gpio)
{
	HALASSERT(gpio < AR_NUM_GPIO);

	OS_REG_WRITE(ah, AR_GPIOCR, 
		  (OS_REG_READ(ah, AR_GPIOCR) &~ AR_GPIOCR_ALL(gpio))
		| AR_GPIOCR_IN(gpio));

	return AH_TRUE;
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL
ar5210GpioSet(struct ath_hal *ah, u_int32_t gpio, u_int32_t val)
{
	u_int32_t reg;

	HALASSERT(gpio < AR_NUM_GPIO);

	reg =  OS_REG_READ(ah, AR_GPIODO);
	reg &= ~(1 << gpio);
	reg |= (val&1) << gpio;

	OS_REG_WRITE(ah, AR_GPIODO, reg);
	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t
ar5210GpioGet(struct ath_hal *ah, u_int32_t gpio)
{
	if (gpio < AR_NUM_GPIO) {
		u_int32_t val = OS_REG_READ(ah, AR_GPIODI);
		val = ((val & AR_GPIOD_MASK) >> gpio) & 0x1;
		return val;
	} else  {
		return 0xffffffff;
	}
}

/*
 * Set the GPIO 0 Interrupt
 */
void
ar5210Gpio0SetIntr(struct ath_hal *ah, u_int gpio, u_int32_t ilevel)
{
	u_int32_t val = OS_REG_READ(ah, AR_GPIOCR);

	/* Clear the bits that we will modify. */
	val &= ~(AR_GPIOCR_INT_SEL(gpio) | AR_GPIOCR_INT_SELH | AR_GPIOCR_INT_ENA |
			AR_GPIOCR_ALL(gpio));

	val |= AR_GPIOCR_INT_SEL(gpio) | AR_GPIOCR_INT_ENA;
	if (ilevel)
		val |= AR_GPIOCR_INT_SELH;

	/* Don't need to change anything for low level interrupt. */
	OS_REG_WRITE(ah, AR_GPIOCR, val);

	/* Change the interrupt mask. */
	ar5210SetInterrupts(ah, AH5210(ah)->ah_maskReg | HAL_INT_GPIO);
}

/*
 * Change the LED blinking pattern to correspond to the connectivity
 */
void
ar5210SetLedState(struct ath_hal *ah, HAL_LED_STATE state)
{
	u_int32_t val;

	val = OS_REG_READ(ah, AR_PCICFG);
	switch (state) {
	case HAL_LED_INIT:
		val &= ~(AR_PCICFG_LED_PEND | AR_PCICFG_LED_ACT);
		break;
	case HAL_LED_RUN:
		/* normal blink when connected */
		val &= ~AR_PCICFG_LED_PEND;
		val |= AR_PCICFG_LED_ACT;
		break;
	default:
		val |= AR_PCICFG_LED_PEND;
		val &= ~AR_PCICFG_LED_ACT;
		break;
	}
	OS_REG_WRITE(ah, AR_PCICFG, val);
}

/*
 * Return 1 or 2 for the corresponding antenna that is in use
 */
u_int
ar5210GetDefAntenna(struct ath_hal *ah)
{
	u_int32_t val = OS_REG_READ(ah, AR_STA_ID1);
	return (val & AR_STA_ID1_DEFAULT_ANTENNA ?  2 : 1);
}

void
ar5210SetDefAntenna(struct ath_hal *ah, u_int antenna)
{
	u_int32_t val = OS_REG_READ(ah, AR_STA_ID1);

	if (antenna != (val & AR_STA_ID1_DEFAULT_ANTENNA ?  2 : 1)) {
		/*
		 * Antenna change requested, force a toggle of the default.
		 */
		OS_REG_WRITE(ah, AR_STA_ID1, val | AR_STA_ID1_DEFAULT_ANTENNA);
	}
}

HAL_ANT_SETTING
ar5210GetAntennaSwitch(struct ath_hal *ah)
{
	return HAL_ANT_VARIABLE;
}

HAL_BOOL
ar5210SetAntennaSwitch(struct ath_hal *ah, HAL_ANT_SETTING settings)
{
	/* XXX not sure how to fix antenna */
	return (settings == HAL_ANT_VARIABLE);
}

/*
 * Change association related fields programmed into the hardware.
 * Writing a valid BSSID to the hardware effectively enables the hardware
 * to synchronize its TSF to the correct beacons and receive frames coming
 * from that BSSID. It is called by the SME JOIN operation.
 */
void
ar5210WriteAssocid(struct ath_hal *ah, const u_int8_t *bssid, u_int16_t assocId)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	/* XXX save bssid for possible re-use on reset */
	OS_MEMCPY(ahp->ah_bssid, bssid, IEEE80211_ADDR_LEN);
	OS_REG_WRITE(ah, AR_BSS_ID0, LE_READ_4(ahp->ah_bssid));
	OS_REG_WRITE(ah, AR_BSS_ID1, LE_READ_2(ahp->ah_bssid+4) |
				     ((assocId & 0x3fff)<<AR_BSS_ID1_AID_S));
	if (assocId == 0)
		OS_REG_SET_BIT(ah, AR_STA_ID1, AR_STA_ID1_NO_PSPOLL);
	else
		OS_REG_CLR_BIT(ah, AR_STA_ID1, AR_STA_ID1_NO_PSPOLL);
}

/*
 * Get the current hardware tsf for stamlme.
 */
u_int64_t
ar5210GetTsf64(struct ath_hal *ah)
{
	u_int32_t low1, low2, u32;

	/* sync multi-word read */
	low1 = OS_REG_READ(ah, AR_TSF_L32);
	u32 = OS_REG_READ(ah, AR_TSF_U32);
	low2 = OS_REG_READ(ah, AR_TSF_L32);
	if (low2 < low1) {	/* roll over */
		/*
		 * If we are not preempted this will work.  If we are
		 * then we re-reading AR_TSF_U32 does no good as the
		 * low bits will be meaningless.  Likewise reading
		 * L32, U32, U32, then comparing the last two reads
		 * (as the VxWorks hal does) to check for rollover
		 * doesn't help if preempted--so we take this approach
		 * as it costs one less PCI read which can be noticeable
		 * when doing things like timestamping packets in
		 * monitor mode.
		 */
		u32++;
	}
	return (((u_int64_t) u32) << 32) | ((u_int64_t) low2);
}

/*
 * Get the current hardware tsf for stamlme.
 */
u_int32_t
ar5210GetTsf32(struct ath_hal *ah)
{
	return OS_REG_READ(ah, AR_TSF_L32);
}

/*
 * Reset the current hardware tsf for stamlme
 */
void
ar5210ResetTsf(struct ath_hal *ah)
{
	u_int32_t val = OS_REG_READ(ah, AR_BEACON);

	OS_REG_WRITE(ah, AR_BEACON, val | AR_BEACON_RESET_TSF);
}

/*
 * Grab a semi-random value from hardware registers - may not
 * change often
 */
u_int32_t
ar5210GetRandomSeed(struct ath_hal *ah)
{
	u_int32_t nf;

	nf = (OS_REG_READ(ah, AR_PHY_BASE + (25 << 2)) >> 19) & 0x1ff;
	if (nf & 0x100)
		nf = 0 - ((nf ^ 0x1ff) + 1);
	return (OS_REG_READ(ah, AR_TSF_U32) ^
		OS_REG_READ(ah, AR_TSF_L32) ^ nf);
}

/*
 * Detect if our card is present
 */
HAL_BOOL
ar5210DetectCardPresent(struct ath_hal *ah)
{
	/*
	 * Read the Silicon Revision register and compare that
	 * to what we read at attach time.  If the same, we say
	 * a card/device is present.
	 */
	return (AH_PRIVATE(ah)->ah_macRev == (OS_REG_READ(ah, AR_SREV) & 0xff));
}

/*
 * Update MIB Counters
 */
void
ar5210UpdateMibCounters(struct ath_hal *ah, HAL_MIB_STATS *stats)
{
	stats->ackrcv_bad += OS_REG_READ(ah, AR_ACK_FAIL);
	stats->rts_bad	  += OS_REG_READ(ah, AR_RTS_FAIL);
	stats->fcs_bad	  += OS_REG_READ(ah, AR_FCS_FAIL);
	stats->rts_good	  += OS_REG_READ(ah, AR_RTS_OK);
	stats->beacons	  += OS_REG_READ(ah, AR_BEACON_CNT);
}

HAL_BOOL
ar5210SetSifsTime(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	if (us <= 0)
		return AH_FALSE;

	/* convert to system clocks */
	if (us > ath_hal_mac_usec(ah, 0xffff)) {
		us = ath_hal_mac_usec(ah, 0xffff);
	}

	OS_REG_RMW_FIELD(ah, AR_IFS0, AR_IFS0_SIFS, ath_hal_mac_clks(ah, us));
	ahp->ah_sifstime = us;
	return AH_TRUE;
}

u_int
ar5210GetSifsTime(struct ath_hal *ah)
{
	u_int clks = OS_REG_READ(ah, AR_IFS0) & 0x7ff;
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

HAL_BOOL
ar5210SetSlotTime(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	if (us <= 0)
		return AH_FALSE;

	if (us > ath_hal_mac_usec(ah, 0xffff)) {
		us = ath_hal_mac_usec(ah, 0xffff);
	}

	/* convert to system clocks */
	OS_REG_WRITE(ah, AR_SLOT_TIME, ath_hal_mac_clks(ah, us));
	ahp->ah_slottime = us;
	return AH_TRUE;
}

u_int
ar5210GetSlotTime(struct ath_hal *ah)
{
	u_int clks = OS_REG_READ(ah, AR_SLOT_TIME) & 0xffff;
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

HAL_BOOL
ar5210SetAckTimeout(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	if (us <= 0)
		return AH_FALSE;

	if (us > ath_hal_mac_usec(ah, MS(0xffffffff, AR_TIME_OUT_ACK))) {
		HALDEBUG(ah, "%s: bad ack timeout %u\n", __func__, us);
		us = ath_hal_mac_usec(ah, MS(0xffffffff, AR_TIME_OUT_ACK));
	}

	/* convert to system clocks */
	OS_REG_RMW_FIELD(ah, AR_TIME_OUT,
		AR_TIME_OUT_ACK, ath_hal_mac_clks(ah, us));
	ahp->ah_acktimeout = us;

	return AH_TRUE;
}

u_int
ar5210GetAckTimeout(struct ath_hal *ah)
{
	u_int clks = MS(OS_REG_READ(ah, AR_TIME_OUT), AR_TIME_OUT_ACK);
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

u_int
ar5210GetAckCTSRate(struct ath_hal *ah)
{
	return ((AH5210(ah)->ah_staId1Defaults & AR_STA_ID1_ACKCTS_6MB) == 0);
}

HAL_BOOL
ar5210SetAckCTSRate(struct ath_hal *ah, u_int high)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	if (high) {
		OS_REG_CLR_BIT(ah, AR_STA_ID1, AR_STA_ID1_ACKCTS_6MB);
		ahp->ah_staId1Defaults &= ~AR_STA_ID1_ACKCTS_6MB;
	} else {
		OS_REG_SET_BIT(ah, AR_STA_ID1, AR_STA_ID1_ACKCTS_6MB);
		ahp->ah_staId1Defaults |= AR_STA_ID1_ACKCTS_6MB;
	}
	return AH_TRUE;
}

HAL_BOOL
ar5210SetCTSTimeout(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5210 *ahp = AH5210(ah);

	if (us <= 0)
		return AH_FALSE;

	if (us > ath_hal_mac_usec(ah, MS(0xffffffff, AR_TIME_OUT_CTS))) {
		HALDEBUG(ah, "%s: bad cts timeout %u\n", __func__, us);
		us = ath_hal_mac_usec(ah, MS(0xffffffff, AR_TIME_OUT_CTS));
	}

	/* convert to system clocks */
	OS_REG_RMW_FIELD(ah, AR_TIME_OUT,
		AR_TIME_OUT_CTS, ath_hal_mac_clks(ah, us));
	ahp->ah_ctstimeout = us;
	return AH_TRUE;
}

u_int
ar5210GetCTSTimeout(struct ath_hal *ah)
{
	u_int clks = MS(OS_REG_READ(ah, AR_TIME_OUT), AR_TIME_OUT_CTS);
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

#define	AR_DIAG_SW_DIS_CRYPTO	(AR_DIAG_SW_DIS_ENC | AR_DIAG_SW_DIS_DEC)

HAL_STATUS
ar5210GetCapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t *result)
{

	switch (type) {
	case HAL_CAP_CIPHER:		/* cipher handled in hardware */
		return (capability == HAL_CIPHER_WEP ? HAL_OK : HAL_ENOTSUPP);
	default:
		return ath_hal_getcapability(ah, type, capability, result);
	}
}

HAL_BOOL
ar5210SetCapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t setting, HAL_STATUS *status)
{

	switch (type) {
	case HAL_CAP_DIAG:		/* hardware diagnostic support */
		/*
		 * NB: could split this up into virtual capabilities,
		 *     (e.g. 1 => ACK, 2 => CTS, etc.) but it hardly
		 *     seems worth the additional complexity.
		 */
#ifdef AH_DEBUG
		AH_PRIVATE(ah)->ah_diagreg = setting;
#else
		AH_PRIVATE(ah)->ah_diagreg = setting & 0x6;	/* ACK+CTS */
#endif
		OS_REG_WRITE(ah, AR_DIAG_SW, AH_PRIVATE(ah)->ah_diagreg);
		return AH_TRUE;
	case HAL_CAP_RXORN_FATAL:	/* HAL_INT_RXORN treated as fatal  */
		return AH_FALSE;	/* NB: disallow */
	default:
		return ath_hal_setcapability(ah, type, capability,
			setting, status);
	}
}

HAL_BOOL
ar5210GetDiagState(struct ath_hal *ah, int request,
	const void *args, u_int32_t argsize,
	void **result, u_int32_t *resultsize)
{
#ifdef AH_PRIVATE_DIAG
	u_int32_t pcicfg;
	HAL_BOOL ok;

	switch (request) {
	case HAL_DIAG_EEPROM:
		/* XXX */
		break;
	case HAL_DIAG_EEREAD:
		if (argsize != sizeof(u_int16_t))
			return AH_FALSE;
		pcicfg = OS_REG_READ(ah, AR_PCICFG);
		OS_REG_WRITE(ah, AR_PCICFG, pcicfg | AR_PCICFG_EEPROMSEL);
		ok = ath_hal_eepromRead(ah, *(const u_int16_t *)args, *result);
		OS_REG_WRITE(ah, AR_PCICFG, pcicfg);
		if (ok)
			*resultsize = sizeof(u_int16_t);
		return ok;
#ifdef AH_SUPPORT_WRITE_EEPROM
	case HAL_DIAG_EEWRITE: {
		HAL_DIAG_EEVAL *ee;

		if (argsize != sizeof(HAL_DIAG_EEVAL))
			return AH_FALSE;
		ee = __DECONST(HAL_DIAG_EEVAL *, args);
		pcicfg = OS_REG_READ(ah, AR_PCICFG);
		OS_REG_WRITE(ah, AR_PCICFG, pcicfg | AR_PCICFG_EEPROMSEL);
		ok = ath_hal_eepromWrite(ah, ee->ee_off, ee->ee_data);
		OS_REG_WRITE(ah, AR_PCICFG, pcicfg);
		return ok;
	}
#endif /* AH_SUPPORT_WRITE_EEPROM */
	}
#endif
	return ath_hal_getdiagstate(ah, request,
		args, argsize, result, resultsize);
}
#endif /* AH_SUPPORT_AR5210 */
