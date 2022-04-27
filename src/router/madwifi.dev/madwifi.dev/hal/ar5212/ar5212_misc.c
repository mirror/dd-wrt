/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5212/ar5212_misc.c#23 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5212

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"
#ifdef AH_DEBUG
#include "ah_desc.h"			/* NB: for HAL_PHYERR* */
#endif

#include "ar5212/ar5212.h"
#include "ar5212/ar5212reg.h"
#include "ar5212/ar5212phy.h"
#ifdef AH_SUPPORT_AR5311
#include "ar5212/ar5311reg.h"
#endif

#define	AR_NUM_GPIO	6		/* 6 GPIO pins */
#define	AR_GPIOD_MASK	0x0000002F	/* GPIO data reg r/w mask */

extern void ar5212SetRateDurationTable(struct ath_hal *, HAL_CHANNEL *);

void
ar5212GetMacAddress(struct ath_hal *ah, u_int8_t *mac)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	OS_MEMCPY(mac, ahp->ah_macaddr, IEEE80211_ADDR_LEN);
}

HAL_BOOL
ar5212SetMacAddress(struct ath_hal *ah, const u_int8_t *mac)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	OS_MEMCPY(ahp->ah_macaddr, mac, IEEE80211_ADDR_LEN);
	return AH_TRUE;
}

void
ar5212GetBssIdMask(struct ath_hal *ah, u_int8_t *mask)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	OS_MEMCPY(mask, ahp->ah_bssidmask, IEEE80211_ADDR_LEN);
}

HAL_BOOL
ar5212SetBssIdMask(struct ath_hal *ah, const u_int8_t *mask)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	/* save it since it must be rewritten on reset */
	OS_MEMCPY(ahp->ah_bssidmask, mask, IEEE80211_ADDR_LEN);

	OS_REG_WRITE(ah, AR_BSSMSKL, LE_READ_4(ahp->ah_bssidmask));
	OS_REG_WRITE(ah, AR_BSSMSKU, LE_READ_2(ahp->ah_bssidmask + 4));
	return AH_TRUE;
}

/*
 * Attempt to change the cards operating regulatory domain to the given value
 * Returns: A_EINVAL for an unsupported regulatory domain.
 *          A_HARDWARE for an unwritable EEPROM or bad EEPROM version
 */
HAL_BOOL
ar5212SetRegulatoryDomain(struct ath_hal *ah,
	u_int16_t regDomain, HAL_STATUS *status)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	HAL_STATUS ecode;

	if (AH_PRIVATE(ah)->ah_currentRD == regDomain) {
		ecode = HAL_EINVAL;
		goto bad;
	}
	if (ahp->ah_eeprotect & AR_EEPROM_PROTECT_WP_128_191) {
		ecode = HAL_EEWRITE;
		goto bad;
	}
#ifdef AH_SUPPORT_WRITE_REGDOMAIN
	if (ath_hal_eepromWrite(ah, AR_EEPROM_REG_DOMAIN, regDomain)) {
		HALDEBUG(ah, "%s: set regulatory domain to %u (0x%x)\n",
			__func__, regDomain, regDomain);
		AH_PRIVATE(ah)->ah_currentRD = regDomain;
		return AH_TRUE;
	}
#endif
	ecode = HAL_EIO;
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
ar5212GetWirelessModes(struct ath_hal *ah)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	u_int mode = 0;

	if (ahp->ah_Amode) {
		mode = HAL_MODE_11A;
		if (!ahp->ah_turbo5Disable)
			mode |= HAL_MODE_TURBO | HAL_MODE_108A;
	}
	if (ahp->ah_Bmode)
		mode |= HAL_MODE_11B;
	if (ahp->ah_Gmode &&
	    AH_PRIVATE(ah)->ah_subvendorid != AR_SUBVENDOR_ID_NOG) {
		mode |= HAL_MODE_11G;
		if (!ahp->ah_turbo2Disable) 
			mode |= HAL_MODE_108G;
	}
	return mode;
}

/*
 * Set the interrupt and GPIO values so the ISR can disable RF
 * on a switch signal.  Assumes GPIO port and interrupt polarity
 * are set prior to call.
 */
void
ar5212EnableRfKill(struct ath_hal *ah)
{
#ifndef AH_SUPPORT_AR7100
	u_int16_t rfsilent = AH_PRIVATE(ah)->ah_rfsilent;
	int select = MS(rfsilent, AR_EEPROM_RFSILENT_GPIO_SEL);
	int polarity = MS(rfsilent, AR_EEPROM_RFSILENT_POLARITY);

	/*
	 * Configure the desired GPIO port for input
	 * and enable baseband rf silence.
	 */
	ath_hal_gpioCfgInput(ah, select);
	OS_REG_SET_BIT(ah, AR_PHY(0), 0x00002000);
	/*
	 * If radio disable switch connection to GPIO bit x is enabled
	 * program GPIO interrupt.
	 * If rfkill bit on eeprom is 1, setupeeprommap routine has already
	 * verified that it is a later version of eeprom, it has a place for
	 * rfkill bit and it is set to 1, indicating that GPIO bit x hardware
	 * connection is present.
	 */
	ath_hal_gpioSetIntr(ah, select,
	    (ath_hal_gpioGet(ah, select) == polarity ? !polarity : polarity));
#endif
}

#ifndef AH_SUPPORT_AR5312
/*
 * Change the LED blinking pattern to correspond to the connectivity
 */
void
ar5212SetLedState(struct ath_hal *ah, HAL_LED_STATE state)
{
	static const u_int32_t ledbits[8] = {
		AR_PCICFG_LEDCTL_NONE,	/* HAL_LED_INIT */
		AR_PCICFG_LEDCTL_PEND,	/* HAL_LED_SCAN */
		AR_PCICFG_LEDCTL_PEND,	/* HAL_LED_AUTH */
		AR_PCICFG_LEDCTL_ASSOC,	/* HAL_LED_ASSOC*/
		AR_PCICFG_LEDCTL_ASSOC,	/* HAL_LED_RUN */
		AR_PCICFG_LEDCTL_NONE,
		AR_PCICFG_LEDCTL_NONE,
		AR_PCICFG_LEDCTL_NONE,
	};
	u_int32_t bits;

	bits = OS_REG_READ(ah, AR_PCICFG);
	if (IS_2417(ah)) {
		/*
		 * Enable LED for Nala. There is a bit marked reserved
		 * that must be set and we also turn on the power led.
		 * Because we mark s/w LED control setting the control
		 * status bits below is meangless (the driver must flash
		 * the LED(s) using the GPIO lines).
		 */
		bits = (bits &~ AR_PCICFG_LEDMODE)
		     | SM(AR_PCICFG_LEDMODE_POWON, AR_PCICFG_LEDMODE)
#if 0
		     | SM(AR_PCICFG_LEDMODE_NETON, AR_PCICFG_LEDMODE)
#endif
		     | 0x08000000;
	}
	bits = (bits &~ AR_PCICFG_LEDCTL)
	     | SM(ledbits[state & 0x7], AR_PCICFG_LEDCTL);
	OS_REG_WRITE(ah, AR_PCICFG, bits);
}

#endif

/*
 * Change association related fields programmed into the hardware.
 * Writing a valid BSSID to the hardware effectively enables the hardware
 * to synchronize its TSF to the correct beacons and receive frames coming
 * from that BSSID. It is called by the SME JOIN operation.
 */
void
ar5212WriteAssocid(struct ath_hal *ah, const u_int8_t *bssid, u_int16_t assocId)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	/* XXX save bssid for possible re-use on reset */
	OS_MEMCPY(ahp->ah_bssid, bssid, IEEE80211_ADDR_LEN);
	OS_REG_WRITE(ah, AR_BSS_ID0, LE_READ_4(ahp->ah_bssid));
	OS_REG_WRITE(ah, AR_BSS_ID1, LE_READ_2(ahp->ah_bssid+4) |
				     ((assocId & 0x3fff)<<AR_BSS_ID1_AID_S));
}

/*
 * Get the current hardware tsf for stamlme
 */
u_int64_t
ar5212GetTsf64(struct ath_hal *ah)
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
 * Get the current hardware tsf for stamlme
 */
u_int32_t
ar5212GetTsf32(struct ath_hal *ah)
{
	return OS_REG_READ(ah, AR_TSF_L32);
}

/*
 * Reset the current hardware tsf for stamlme.
 */
void
ar5212ResetTsf(struct ath_hal *ah)
{

	u_int32_t val = OS_REG_READ(ah, AR_BEACON);

	OS_REG_WRITE(ah, AR_BEACON, val | AR_BEACON_RESET_TSF);
	/*
	 * workaround for hw bug! when resetting the TSF, write twice to the
	 * corresponding register; each write to the RESET_TSF bit toggles
	 * the internal signal to cause a reset of the TSF - but if the signal
	 * is left high, it will reset the TSF on the next chip reset also!
	 * writing the bit an even number of times fixes this issue
	 */
	OS_REG_WRITE(ah, AR_BEACON, val | AR_BEACON_RESET_TSF);
}

/*
 * Set or clear hardware basic rate bit
 * Set hardware basic rate set if basic rate is found
 * and basic rate is equal or less than 2Mbps
 */
void
ar5212SetBasicRate(struct ath_hal *ah, HAL_RATE_SET *rs)
{
	HAL_CHANNEL_INTERNAL *chan = AH_PRIVATE(ah)->ah_curchan;
	u_int32_t reg;
	u_int8_t xset;
	int i;

	if (chan == AH_NULL || !IS_CHAN_CCK(chan))
		return;
	xset = 0;
	for (i = 0; i < rs->rs_count; i++) {
		u_int8_t rset = rs->rs_rates[i];
		/* Basic rate defined? */
		if ((rset & 0x80) && (rset &= 0x7f) >= xset)
			xset = rset;
	}
	/*
	 * Set the h/w bit to reflect whether or not the basic
	 * rate is found to be equal or less than 2Mbps.
	 */
	reg = OS_REG_READ(ah, AR_STA_ID1);
	if (xset && xset/2 <= 2)
		OS_REG_WRITE(ah, AR_STA_ID1, reg | AR_STA_ID1_BASE_RATE_11B);
	else
		OS_REG_WRITE(ah, AR_STA_ID1, reg &~ AR_STA_ID1_BASE_RATE_11B);
}

/*
 * Grab a semi-random value from hardware registers - may not
 * change often
 */
u_int32_t
ar5212GetRandomSeed(struct ath_hal *ah)
{
	u_int32_t nf;

	nf = (OS_REG_READ(ah, AR_PHY(25)) >> 19) & 0x1ff;
	if (nf & 0x100)
		nf = 0 - ((nf ^ 0x1ff) + 1);
	return (OS_REG_READ(ah, AR_TSF_U32) ^
		OS_REG_READ(ah, AR_TSF_L32) ^ nf);
}
#ifndef AH_SUPPORT_AR5312

/*
 * Detect if our card is present
 */
HAL_BOOL
ar5212DetectCardPresent(struct ath_hal *ah)
{
	u_int16_t macVersion, macRev;
	u_int32_t v;

	/*
	 * Read the Silicon Revision register and compare that
	 * to what we read at attach time.  If the same, we say
	 * a card/device is present.
	 */
	v = OS_REG_READ(ah, AR_SREV) & AR_SREV_ID;
	macVersion = v >> AR_SREV_ID_S;
	macRev = v & AR_SREV_REVISION;
	return (AH_PRIVATE(ah)->ah_macVersion == macVersion &&
		AH_PRIVATE(ah)->ah_macRev == macRev);
}
#endif
void
ar5212EnableMibCounters(struct ath_hal *ah)
{
	/* NB: this just resets the mib counter machinery */
	OS_REG_WRITE(ah, AR_MIBC,
	    ~(AR_MIBC_COW | AR_MIBC_FMC | AR_MIBC_CMC | AR_MIBC_MCS) & 0x0f);
}

void 
ar5212DisableMibCounters(struct ath_hal *ah)
{
	OS_REG_WRITE(ah, AR_MIBC,  AR_MIBC | AR_MIBC_CMC);
}

/*
 * Update MIB Counters
 */
void
ar5212UpdateMibCounters(struct ath_hal *ah, HAL_MIB_STATS* stats)
{
	stats->ackrcv_bad += OS_REG_READ(ah, AR_ACK_FAIL);
	stats->rts_bad	  += OS_REG_READ(ah, AR_RTS_FAIL);
	stats->fcs_bad	  += OS_REG_READ(ah, AR_FCS_FAIL);
	stats->rts_good	  += OS_REG_READ(ah, AR_RTS_OK);
	stats->beacons	  += OS_REG_READ(ah, AR_BEACON_CNT);
}

/*
 * Detect if the HW supports spreading a CCK signal on channel 14
 */
HAL_BOOL
ar5212IsJapanChannelSpreadSupported(struct ath_hal *ah)
{
	return AH_TRUE;
}

/*
 * Get the rssi of frame curently being received.
 */
u_int32_t
ar5212GetCurRssi(struct ath_hal *ah)
{
    if (AR_SREV_MERLIN_10_OR_LATER(ah))
        return (OS_REG_READ(ah, AR9280_PHY_CURRENT_RSSI) & 0xff);
    else
        return (OS_REG_READ(ah, AR_PHY_CURRENT_RSSI) & 0xff);
}

u_int
ar5212GetDefAntenna(struct ath_hal *ah)
{   
	return (OS_REG_READ(ah, AR_DEF_ANTENNA) & 0x7);
}   

void
ar5212SetDefAntenna(struct ath_hal *ah, u_int antenna)
{
	OS_REG_WRITE(ah, AR_DEF_ANTENNA, (antenna & 0x7));
}

HAL_ANT_SETTING
ar5212GetAntennaSwitch(struct ath_hal *ah)
{
	return AH5212(ah)->ah_diversityControl;
}

HAL_BOOL
ar5212SetAntennaSwitch(struct ath_hal *ah, HAL_ANT_SETTING settings)
{
	const HAL_CHANNEL_INTERNAL *chan = AH_PRIVATE(ah)->ah_curchan;

	if (chan == AH_NULL) {
		AH5212(ah)->ah_diversityControl = settings;
		return AH_TRUE;
	}
	return ar5212SetAntennaSwitchInternal(ah, settings, chan);
}

HAL_BOOL
ar5212IsSleepAfterBeaconBroken(struct ath_hal *ah)
{
	return AH_TRUE;
}

HAL_BOOL
ar5212SetSifsTime(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	if ((us == (u_int) -1) || (us <= 2))
		return AH_FALSE;

	/* convert to system clocks */

	ahp->ah_sifstime = us;

	if ((us-2) > ath_hal_mac_usec(ah, 0xffff)) {
		us = ath_hal_mac_usec(ah, 0xffff) + 2;
	}
	OS_REG_WRITE(ah, AR_D_GBL_IFS_SIFS, ath_hal_mac_clks(ah, us - 2));
	return AH_TRUE;
}

u_int
ar5212GetSifsTime(struct ath_hal *ah)
{
	u_int clks = OS_REG_READ(ah, AR_D_GBL_IFS_SIFS) & 0xffff;
	return ath_hal_mac_usec(ah, clks) + 2;	/* convert from system clocks */
}

HAL_BOOL
ar5212SetEifsTime(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	if (us == (u_int) -1)
		return AH_FALSE;

	/* convert to system clocks */

	ahp->ah_eifstime = us;
	if (us > ath_hal_mac_usec(ah, 0xffff)) {
		us = ath_hal_mac_usec(ah, 0xffff);
	}
	OS_REG_WRITE(ah, AR_D_GBL_IFS_EIFS, ath_hal_mac_clks(ah, us));
	return AH_TRUE;
}

u_int
ar5212GetEifsTime(struct ath_hal *ah)
{
	u_int clks = OS_REG_READ(ah, AR_D_GBL_IFS_EIFS) & 0xffff;
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

HAL_BOOL
ar5212SetSlotTime(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	if (us == (u_int) -1)
		return AH_FALSE;

	if (us > ath_hal_mac_usec(ah, 0xffff)) {
		us = ath_hal_mac_usec(ah, 0xffff);
	}

	/* convert to system clocks */
	OS_REG_WRITE(ah, AR_D_GBL_IFS_SLOT, ath_hal_mac_clks(ah, us));
	ahp->ah_slottime = us;
	return AH_TRUE;
}

u_int
ar5212GetSlotTime(struct ath_hal *ah)
{
	u_int clks = OS_REG_READ(ah, AR_D_GBL_IFS_SLOT) & 0xffff;
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

HAL_BOOL
ar5212SetAckTimeout(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	if (us == (u_int) -1)
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
ar5212GetAckTimeout(struct ath_hal *ah)
{
	u_int clks = MS(OS_REG_READ(ah, AR_TIME_OUT), AR_TIME_OUT_ACK);
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

u_int
ar5212GetAckCTSRate(struct ath_hal *ah)
{
	return ((AH5212(ah)->ah_staId1Defaults & AR_STA_ID1_ACKCTS_6MB) == 0);
}

HAL_BOOL
ar5212SetAckCTSRate(struct ath_hal *ah, u_int high)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

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
ar5212SetCTSTimeout(struct ath_hal *ah, u_int us)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	if (us == (u_int) -1)
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
ar5212GetCTSTimeout(struct ath_hal *ah)
{
	u_int clks = MS(OS_REG_READ(ah, AR_TIME_OUT), AR_TIME_OUT_CTS);
	return ath_hal_mac_usec(ah, clks);	/* convert from system clocks */
}

/* Setup decompression for given key index */
HAL_BOOL
ar5212SetDecompMask(struct ath_hal *ah, u_int16_t keyidx, int en)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

        if (keyidx >= HAL_DECOMP_MASK_SIZE)
                return HAL_EINVAL; 
        OS_REG_WRITE(ah, AR_DCM_A, keyidx);
        OS_REG_WRITE(ah, AR_DCM_D, en ? AR_DCM_D_EN : 0);
        ahp->ah_decompMask[keyidx] = en;

        return AH_TRUE;
}

/* Setup coverage class */
void
ar5212SetCoverageClass(struct ath_hal *ah, u_int8_t coverageclass, int now)
{
	u_int32_t slot, timeout, eifs;
	u_int clkRate;

	AH_PRIVATE(ah)->ah_coverageClass = coverageclass;

	if (now) {

		if (AH_PRIVATE(ah)->ah_coverageClass == 0)
			return;

		/* Don't apply coverage class to non A channels */
		if (!IS_CHAN_A(AH_PRIVATE(ah)->ah_curchan))
			return;

		/* Get clock rate */
		clkRate = ath_hal_mac_clks(ah, 1);

		/* Compute EIFS */
		if (IS_CHAN_HT40(AH_PRIVATE(ah)->ah_curchan)) {
			/* 
			 * NB: ath_hal_mac_clks returns the clock with HT40 correction
			 * for ACK/CTS timeout. 
			 * Apparently slot and eifs rate need to skip this, hence the >> 1
			 */
			slot = IFS_SLOT_FULL_RATE + 
					(coverageclass * 3 * (clkRate >> 1));
			eifs = IFS_EIFS_HALF_RATE +
					(coverageclass * 6 * (clkRate >> 1));
		} else if (IS_CHAN_HALF_RATE(AH_PRIVATE(ah)->ah_curchan)) {
			slot = IFS_SLOT_HALF_RATE + 
					(coverageclass * 3 * (clkRate >> 1));
			eifs = IFS_EIFS_HALF_RATE +
					(coverageclass * 6 * (clkRate >> 1));
		} else if (IS_CHAN_QUARTER_RATE(AH_PRIVATE(ah)->ah_curchan)) {
			slot = IFS_SLOT_QUARTER_RATE + 
				(coverageclass * 3 * (clkRate >> 2));
			eifs = IFS_EIFS_QUARTER_RATE +
				(coverageclass * 6 * (clkRate >> 2));
		} else if (IS_CHAN_SUBQUARTER_RATE(AH_PRIVATE(ah)->ah_curchan)) {
			slot = IFS_SLOT_SUBQUARTER_RATE + 
				(coverageclass * 3 * (clkRate >> 3));
			eifs = IFS_EIFS_SUBQUARTER_RATE +
				(coverageclass * 6 * (clkRate >> 3));
		} else { /* full rate */
			slot = IFS_SLOT_FULL_RATE + 
					(coverageclass * 3 * clkRate);
			eifs = IFS_EIFS_FULL_RATE + 
					(coverageclass * 6 * clkRate);
		}

		/* Add additional time for air propagation for ACK and CTS
		 * timeouts. This value is in core clocks.
  		 */
		timeout = (ACK_CTS_TIMEOUT_11A + 
			((coverageclass * 3) * clkRate)) & AR_TIME_OUT_ACK;
	
		/* Write the values back to registers: slot, eifs, ack/cts
		 * timeouts.
		 */
	
		OS_REG_WRITE(ah, AR_D_GBL_IFS_SLOT, slot);
		OS_REG_WRITE(ah, AR_D_GBL_IFS_EIFS, eifs);
		OS_REG_WRITE(ah, AR_TIME_OUT, ((timeout << AR_TIME_OUT_CTS_S) 
							| timeout));
	}

	return;
}

void
ar5212SetPCUConfig(struct ath_hal *ah)
{
	ar5212SetOperatingMode(ah, AH_PRIVATE(ah)->ah_opmode);
}

/*
 * Return whether an external 32KHz crystal should be used
 * to reduce power consumption when sleeping.  We do so if
 * the crystal is present (obtained from EEPROM) and if we
 * are not running as an AP and are configured to use it.
 */
HAL_BOOL
ar5212Use32KHzclock(struct ath_hal *ah, HAL_OPMODE opmode)
{
	if (opmode != HAL_M_HOSTAP) {
		struct ath_hal_5212 *ahp = AH5212(ah);
		return ahp->ah_exist32kHzCrystal &&
		       (ahp->ah_enable32kHzClock == USE_32KHZ ||
		        ahp->ah_enable32kHzClock == AUTO_32KHZ);
	} else
		return AH_FALSE;
}

/*
 * If 32KHz clock exists, use it to lower power consumption during sleep
 *
 * Note: If clock is set to 32 KHz, delays on accessing certain
 *       baseband registers (27-31, 124-127) are required.
 */
void
ar5212SetupClock(struct ath_hal *ah, HAL_OPMODE opmode)
{
	if (!ar5212Use32KHzclock(ah, opmode)) {
		ar5212RestoreClock(ah, opmode);
		return;
	}

	/*
	 * Enable clocks to be turned OFF in BB during sleep
	 * and also enable turning OFF 32MHz/40MHz Refclk
	 * from A2.
	 */
	OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_CONTROL, 0x1f);
	if (IS_5112(ah) || IS_2417(ah) || IS_2425(ah)) {
		OS_REG_WRITE(ah, AR_PHY_REFCLKPD, 0x14);
	} else {
		OS_REG_WRITE(ah, AR_PHY_REFCLKPD, 0x18);
	}
	OS_REG_RMW_FIELD(ah, AR_USEC, AR_USEC_USEC32, 1);
	OS_REG_WRITE(ah, AR_TSF_PARM, 61);  /* 32 KHz TSF incr */
	OS_REG_RMW_FIELD(ah, AR_PCICFG, AR_PCICFG_SCLK_SEL, 1);

	if (IS_2413(ah) || IS_5413(ah) || IS_2417(ah) || IS_2425(ah)) {
		OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_LIMIT,   0x26);
		if (IS_2417(ah)) {
			OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,        0x0a);
		} else {
			OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,        0x0d);
		}
		OS_REG_WRITE(ah, AR_PHY_M_SLEEP,           0x07);
		OS_REG_WRITE(ah, AR_PHY_REFCLKDLY,         0x3f);
		/* # Set sleep clock rate to 32 KHz. */
		OS_REG_RMW_FIELD(ah, AR_PCICFG, AR_PCICFG_SCLK_RATE_IND, 0x2);
	} else {
		OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_LIMIT,   0x0a);
		OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,        0x0c);
		OS_REG_WRITE(ah, AR_PHY_M_SLEEP,           0x03);
		OS_REG_WRITE(ah, AR_PHY_REFCLKDLY,         0x20);
		OS_REG_RMW_FIELD(ah, AR_PCICFG, AR_PCICFG_SCLK_RATE_IND, 0x3);
	}
}

/*
 * If 32KHz clock exists, turn it off and turn back on the 32Mhz
 */
void
ar5212RestoreClock(struct ath_hal *ah, HAL_OPMODE opmode)
{
	/* # Set sleep clock rate back to 32 MHz. */
	OS_REG_RMW_FIELD(ah, AR_PCICFG, AR_PCICFG_SCLK_RATE_IND, 0);
	OS_REG_RMW_FIELD(ah, AR_PCICFG, AR_PCICFG_SCLK_SEL, 0);

	OS_REG_WRITE(ah, AR_TSF_PARM, 1);	/* 32 MHz TSF incr */

	/*
	 * Restore BB registers to power-on defaults
	 */
	OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_CONTROL, 0x1f);
	OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_LIMIT,   0x7f);
	if (IS_2417(ah)) {
		OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,    0x0a);
	} else {
		OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,    0x0e);
	}
	OS_REG_WRITE(ah, AR_PHY_M_SLEEP,           0x0c);
	OS_REG_WRITE(ah, AR_PHY_REFCLKDLY,         0xff);
	if (IS_5112(ah) || IS_2417(ah) || IS_2425(ah)) {
		OS_REG_WRITE(ah, AR_PHY_REFCLKPD, 0x14);
		OS_REG_RMW_FIELD(ah, AR_USEC, AR_USEC_USEC32, 39);
	} else {
		OS_REG_WRITE(ah, AR_PHY_REFCLKPD, 0x18);
		OS_REG_RMW_FIELD(ah, AR_USEC, AR_USEC_USEC32, 31);
	}
}

int16_t
interpolate_signed(u_int16_t target, u_int16_t srcLeft, u_int16_t srcRight,
	int16_t targetLeft, int16_t targetRight)
{
	int16_t rv;

	if (srcRight != srcLeft) {
		rv = ((target - srcLeft)*targetRight +
		      (srcRight - target)*targetLeft) / (srcRight - srcLeft);
	} else {
		rv = targetLeft;
	}
	return rv;
}


/*
 * Returns interpolated or the scaled up interpolated value
 */
u_int16_t
interpolate(u_int16_t target, u_int16_t srcLeft, u_int16_t srcRight,
	u_int16_t targetLeft, u_int16_t targetRight)
{
	u_int16_t rv;
	int16_t lRatio;

	/* to get an accurate ratio, always scale, if want to scale, then don't scale back down */
	if ((targetLeft * targetRight) == 0)
		return 0;

	if (srcRight != srcLeft) {
		/*
		 * Note the ratio always need to be scaled,
		 * since it will be a fraction.
		 */
		lRatio = (target - srcLeft) * EEP_SCALE / (srcRight - srcLeft);
		if (lRatio < 0) {
		    /* Return as Left target if value would be negative */
		    rv = targetLeft;
		} else if (lRatio > EEP_SCALE) {
		    /* Return as Right target if Ratio is greater than 100% (SCALE) */
		    rv = targetRight;
		} else {
			rv = (lRatio * targetRight + (EEP_SCALE - lRatio) *
					targetLeft) / EEP_SCALE;
		}
	} else {
		rv = targetLeft;
	}
	return rv;
}

void
ar5212GetLowerUpperIndex(u_int16_t v, u_int16_t *lp, u_int16_t listSize,
                          u_int32_t *vlo, u_int32_t *vhi)
{
	u_int32_t target = v;
	u_int16_t *ep = lp+listSize;
	u_int16_t *tp;

	/*
	 * Check first and last elements for out-of-bounds conditions.
	 */
	if (target < lp[0]) {
		*vlo = *vhi = 0;
		return;
	}
	if (target >= ep[-1]) {
		*vlo = *vhi = listSize - 1;
		return;
	}

	/* look for value being near or between 2 values in list */
	for (tp = lp; tp < ep; tp++) {
		/*
		 * If value is close to the current value of the list
		 * then target is not between values, it is one of the values
		 */
		if (*tp == target) {
			*vlo = *vhi = tp - lp;
			return;
		}
		/*
		 * Look for value being between current value and next value
		 * if so return these 2 values
		 */
		if (target < tp[1]) {
			*vlo = tp - lp;
			*vhi = *vlo + 1;
			return;
		}
	}
}

/*
 * Adjust NF based on statistical values for 5GHz frequencies.
 * Default method: this may be overridden by the rf backend.
 */
int16_t
ar5212GetNfAdjust(struct ath_hal *ah, const HAL_CHANNEL_INTERNAL *c)
{
	static const struct {
		u_int16_t freqLow;
		int16_t	  adjust;
	} adjustDef[] = {
		{ 5790,	11 },	/* NB: ordered high -> low */
		{ 5730, 10 },
		{ 5690,  9 },
		{ 5660,  8 },
		{ 5610,  7 },
		{ 5530,  5 },
		{ 5450,  4 },
		{ 5379,  2 },
		{ 5209,  0 },
		{ 3000,  1 },
		{    0,  0 },
	};
	int i;

	for (i = 0; c->channel <= adjustDef[i].freqLow; i++)
		;
	return adjustDef[i].adjust;
}

HAL_STATUS
ar5212GetCapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t *result)
{
#define	MACVERSION(ah)	AH_PRIVATE(ah)->ah_macVersion
	struct ath_hal_5212 *ahp = AH5212(ah);
	const HAL_CAPABILITIES *pCap = &AH_PRIVATE(ah)->ah_caps;
	const struct ar5212AniState *ani;

	switch (type) {
	case HAL_CAP_CIPHER:		/* cipher handled in hardware */
		switch (capability) {
		case HAL_CIPHER_AES_CCM:			
			return pCap->halCipherAesCcmSupport ?
				HAL_OK : HAL_ENOTSUPP;
		case HAL_CIPHER_AES_OCB:
		case HAL_CIPHER_TKIP:
		case HAL_CIPHER_WEP:
		case HAL_CIPHER_MIC:
		case HAL_CIPHER_CLR:
			return HAL_OK;
		default:
			return HAL_ENOTSUPP;
		}
	case HAL_CAP_TKIP_MIC:		/* handle TKIP MIC in hardware */
		switch (capability) {
		case 0:			/* hardware capability */
			return HAL_OK;
		case 1:
			return (ahp->ah_staId1Defaults &
			    AR_STA_ID1_CRPT_MIC_ENABLE) ?  HAL_OK : HAL_ENXIO;
		}
	case HAL_CAP_TKIP_SPLIT:	/* hardware TKIP uses split keys */
		switch (capability) {
		case 0:			/* hardware capability */
			return pCap->halTkipMicTxRxKeySupport ?
				HAL_ENXIO : HAL_OK;
		case 1:			/* current setting */
			return (ahp->ah_miscMode &
			    AR_MISC_MODE_MIC_NEW_LOC_ENABLE) ? HAL_ENXIO : HAL_OK;
		}
		return HAL_EINVAL;
	case HAL_CAP_WME_TKIPMIC:	/* hardware can do TKIP MIC w/ WMM */
		/* XXX move to capability bit */
		return MACVERSION(ah) > AR_SREV_VERSION_VENICE ||
		    (MACVERSION(ah) == AR_SREV_VERSION_VENICE &&
		     AH_PRIVATE(ah)->ah_macRev >= 8) ? HAL_OK : HAL_ENOTSUPP;
	case HAL_CAP_DIVERSITY:		/* hardware supports fast diversity */
		switch (capability) {
		case 0:			/* hardware capability */
			return HAL_OK;
		case 1:			/* current setting */
			return (OS_REG_READ(ah, AR_PHY_CCK_DETECT) &
				AR_PHY_CCK_DETECT_BB_ENABLE_ANT_FAST_DIV) ?
				HAL_OK : HAL_ENXIO;
		}
		return HAL_EINVAL;
	case HAL_CAP_DIAG:
		*result = AH_PRIVATE(ah)->ah_diagreg;
		return HAL_OK;
	case HAL_CAP_TPC:
		switch (capability) {
		case 0:			/* hardware capability */
			return HAL_OK;
		case 1:
			return ahp->ah_tpcEnabled ? HAL_OK : HAL_ENXIO;
		}
		return HAL_OK;
	case HAL_CAP_PHYDIAG:		/* radar pulse detection capability */
		switch(capability) {
		case HAL_CAP_RADAR:
			return (ahp->ah_Amode ? HAL_OK: HAL_ENXIO);
		case HAL_CAP_AR:
			if ((ahp->ah_Gmode) || (ahp->ah_Bmode))
				return HAL_OK;
			else
				return HAL_ENXIO;
		}
		return HAL_ENXIO;
	case HAL_CAP_MCAST_KEYSRCH:	/* multicast frame keycache search */
		switch (capability) {
		case 0:			/* hardware capability */
			return HAL_OK;
		case 1:
			return (ahp->ah_staId1Defaults &
			    AR_STA_ID1_MCAST_KSRCH) ? HAL_OK : HAL_ENXIO;
		}
		return HAL_EINVAL;
	case HAL_CAP_TSF_ADJUST:	/* hardware has beacon tsf adjust */
		switch (capability) {
		case 0:			/* hardware capability */
			return pCap->halTsfAddSupport ? HAL_OK : HAL_ENOTSUPP;
		case 1:
			return (ahp->ah_miscMode & AR_MISC_MODE_TX_ADD_TSF) ?
				HAL_OK : HAL_ENXIO;
		}
		return HAL_EINVAL;
	case HAL_CAP_TPC_ACK:
		*result = MS(ahp->ah_macTPC, AR_TPC_ACK);
		return HAL_OK;
	case HAL_CAP_TPC_CTS:
		*result = MS(ahp->ah_macTPC, AR_TPC_CTS);
		return HAL_OK;
	case HAL_CAP_INTMIT:		/* interference mitigation */
		switch (capability) {
		case 0:			/* hardware capability */
			return HAL_OK;
		case 1:
			return (ahp->ah_procPhyErr & HAL_ANI_ENA) ?
				HAL_OK : HAL_ENXIO;
		case 2:			/* HAL_ANI_NOISE_IMMUNITY_LEVEL */
		case 3:			/* HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION */
		case 4:			/* HAL_ANI_CCK_WEAK_SIGNAL_THR */
		case 5:			/* HAL_ANI_FIRSTEP_LEVEL */
		case 6:			/* HAL_ANI_SPUR_IMMUNITY_LEVEL */
			ani = ar5212AniGetCurrentState(ah);
			if (ani == AH_NULL)
				return HAL_ENXIO;
			switch (capability) {
			case 2:	*result = ani->noiseImmunityLevel; break;
			case 3: *result = !ani->ofdmWeakSigDetectOff; break;
			case 4: *result = ani->cckWeakSigThreshold; break;
			case 5: *result = ani->firstepLevel; break;
			case 6: *result = ani->spurImmunityLevel; break;
			}
			return HAL_OK;
		}
		return HAL_EINVAL;
	case HAL_CAP_DMABURST_RX:
		*result = ahp->ah_rxDmaBurst;
		return HAL_OK;
	case HAL_CAP_DMABURST_TX:
		*result = ahp->ah_txDmaBurst;
		return HAL_OK;
	default:
		return ath_hal_getcapability(ah, type, capability, result);
	}
#undef MACVERSION
}

HAL_BOOL
ar5212SetCapability(struct ath_hal *ah, HAL_CAPABILITY_TYPE type,
	u_int32_t capability, u_int32_t setting, HAL_STATUS *status)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	struct ath_hal_5212 *ahp = AH5212(ah);
	const HAL_CAPABILITIES *pCap = &AH_PRIVATE(ah)->ah_caps;
	u_int32_t v;

	switch (type) {
	case HAL_CAP_XR:
		if (setting)
			ahp->ah_xrEnable = AH_TRUE;
		else
			ahp->ah_xrEnable = AH_FALSE;
		return AH_TRUE;  
	case HAL_CAP_TKIP_MIC:		/* handle TKIP MIC in hardware */
		if (setting)
			ahp->ah_staId1Defaults |= AR_STA_ID1_CRPT_MIC_ENABLE;
		else
			ahp->ah_staId1Defaults &= ~AR_STA_ID1_CRPT_MIC_ENABLE;
		return AH_TRUE;
	case HAL_CAP_TKIP_SPLIT:	/* hardware TKIP uses split keys */
		if (!pCap->halTkipMicTxRxKeySupport)
			return AH_FALSE;
		/* NB: true =>'s use split key cache layout */
		if (setting)
			ahp->ah_miscMode &= ~AR_MISC_MODE_MIC_NEW_LOC_ENABLE;
		else
			ahp->ah_miscMode |= AR_MISC_MODE_MIC_NEW_LOC_ENABLE;
		/* NB: write here so keys can be setup w/o a reset */
		OS_REG_WRITE(ah, AR_MISC_MODE, ahp->ah_miscMode);
		return AH_TRUE;
	case HAL_CAP_DIVERSITY:
		v = OS_REG_READ(ah, AR_PHY_CCK_DETECT);
		if (setting)
			v |= AR_PHY_CCK_DETECT_BB_ENABLE_ANT_FAST_DIV;
		else
			v &= ~AR_PHY_CCK_DETECT_BB_ENABLE_ANT_FAST_DIV;
		OS_REG_WRITE(ah, AR_PHY_CCK_DETECT, v);
		return AH_TRUE;
	case HAL_CAP_DIAG:		/* hardware diagnostic support */
		/*
		 * NB: could split this up into virtual capabilities,
		 *     (e.g. 1 => ACK, 2 => CTS, etc.) but it hardly
		 *     seems worth the additional complexity.
		 */
		AH_PRIVATE(ah)->ah_diagreg = setting;
		OS_REG_WRITE(ah, AR_DIAG_SW, AH_PRIVATE(ah)->ah_diagreg);
		return AH_TRUE;
	case HAL_CAP_TPC:
		ahp->ah_tpcEnabled = (setting != 0);
		return AH_TRUE;
	case HAL_CAP_MCAST_KEYSRCH:	/* multicast frame keycache search */
		if (setting)
			ahp->ah_staId1Defaults |= AR_STA_ID1_MCAST_KSRCH;
		else
			ahp->ah_staId1Defaults &= ~AR_STA_ID1_MCAST_KSRCH;
		return AH_TRUE;
	case HAL_CAP_TPC_ACK:
	case HAL_CAP_TPC_CTS:
		setting += ahp->ah_txPowerIndexOffset;
		if (setting > 63)
			setting = 63;
		if (type == HAL_CAP_TPC_ACK) {
			ahp->ah_macTPC &= AR_TPC_ACK;
			ahp->ah_macTPC |= MS(setting, AR_TPC_ACK);
		} else {
			ahp->ah_macTPC &= AR_TPC_CTS;
			ahp->ah_macTPC |= MS(setting, AR_TPC_CTS);
		}
		ar5212SetTpc(ah, AH_PRIVATE(ah)->ah_curchan);
		return AH_TRUE;
	case HAL_CAP_INTMIT: {		/* interference mitigation */
		static const HAL_ANI_CMD cmds[] = {
			HAL_ANI_PRESENT,
			HAL_ANI_MODE,
			HAL_ANI_NOISE_IMMUNITY_LEVEL,
			HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION,
			HAL_ANI_CCK_WEAK_SIGNAL_THR,
			HAL_ANI_FIRSTEP_LEVEL,
			HAL_ANI_SPUR_IMMUNITY_LEVEL,
		};
		return capability < N(cmds) ?
			ar5212AniControl(ah, cmds[capability], setting) :
			AH_FALSE;
	}
	case HAL_CAP_DMABURST_RX:
		ahp->ah_rxDmaBurst = setting & AR_DMASIZE;
		return AH_TRUE;
	case HAL_CAP_DMABURST_TX:
		ahp->ah_txDmaBurst = setting & AR_DMASIZE;
		return AH_TRUE;
	case HAL_CAP_TSF_ADJUST:	/* hardware has beacon tsf adjust */
		if (pCap->halTsfAddSupport) {
			if (setting)
				ahp->ah_miscMode |= AR_MISC_MODE_TX_ADD_TSF;
			else
				ahp->ah_miscMode &= ~AR_MISC_MODE_TX_ADD_TSF;
			return AH_TRUE;
		}
		/* fall thru... */
	default:
		return ath_hal_setcapability(ah, type, capability,
				setting, status);
	}
#undef N
}

HAL_BOOL
ar5212GetDiagState(struct ath_hal *ah, int request,
	const void *args, u_int32_t argsize,
	void **result, u_int32_t *resultsize)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	(void) ahp;
	if (ath_hal_getdiagstate(ah, request, args, argsize, result, resultsize))
		return AH_TRUE;
	switch (request) {
#ifdef AH_PRIVATE_DIAG
	const EEPROM_POWER_EXPN_5112 *pe;

	case HAL_DIAG_EEPROM:
		*result = &ahp->ah_eeprom;
		*resultsize = sizeof(HAL_EEPROM);
		return AH_TRUE;
	case HAL_DIAG_EEPROM_EXP_11A:
	case HAL_DIAG_EEPROM_EXP_11B:
	case HAL_DIAG_EEPROM_EXP_11G:
		pe = &ahp->ah_modePowerArray5112[
			request - HAL_DIAG_EEPROM_EXP_11A];
		*result = pe->pChannels;
		*resultsize = (*result == AH_NULL) ? 0 :
			roundup(sizeof(u_int16_t) * pe->numChannels,
				sizeof(u_int32_t)) +
			sizeof(EXPN_DATA_PER_CHANNEL_5112) * pe->numChannels;
		return AH_TRUE;
	case HAL_DIAG_RFGAIN:
		*result = &ahp->ah_gainValues;
		*resultsize = sizeof(GAIN_VALUES);
		return AH_TRUE;
	case HAL_DIAG_RFGAIN_CURSTEP:
		*result = __DECONST(void *, ahp->ah_gainValues.currStep);
		*resultsize = (*result == AH_NULL) ?
			0 : sizeof(GAIN_OPTIMIZATION_STEP);
		return AH_TRUE;
	case HAL_DIAG_PCDAC:
		*result = ahp->ah_pcdacTable;
		*resultsize = ahp->ah_pcdacTableSize;
		return AH_TRUE;
	case HAL_DIAG_TXRATES:
		*result = &ahp->ah_ratesArray[0];
		*resultsize = sizeof(ahp->ah_ratesArray);
		return AH_TRUE;
#endif /* AH_PRIVATE_DIAG */
	case HAL_DIAG_ANI_CURRENT:
		*result = ar5212AniGetCurrentState(ah);
		*resultsize = (*result == AH_NULL) ?
			0 : sizeof(struct ar5212AniState);
		return AH_TRUE;
	case HAL_DIAG_ANI_STATS:
		*result = ar5212AniGetCurrentStats(ah);
		*resultsize = (*result == AH_NULL) ?
			0 : sizeof(struct ar5212Stats);
		return AH_TRUE;
	case HAL_DIAG_ANI_CMD:
		if (argsize != 2*sizeof(u_int32_t))
			return AH_FALSE;
		ar5212AniControl(ah, ((const u_int32_t *)args)[0],
			((const u_int32_t *)args)[1]);
		return AH_TRUE;
	case HAL_DIAG_ANI_PARAMS:
		/*
		 * NB: We assume struct ar5212AniParams is identical
		 * to HAL_ANI_PARAMS; if they diverge then we'll need
		 * to handle it here
		 */
		if (argsize == 0 && args == AH_NULL) {
			struct ar5212AniState *aniState =
			    ar5212AniGetCurrentState(ah);
			if (aniState == AH_NULL)
				return AH_FALSE;
			*result = __DECONST(void *, aniState->params);
			*resultsize = sizeof(struct ar5212AniParams);
			return AH_TRUE;
		} else {
			if (argsize != sizeof(struct ar5212AniParams))
				return AH_FALSE;
			return ar5212AniSetParams(ah, args, args);
		}
	}
	return AH_FALSE;
}


void ar5212SetTpc(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *ichan)
{
	u_int32_t ackTpcPow, ctsTpcPow, chirpTpcPow, powerVal;
	int8_t twiceAntennaGain, twiceAntennaReduction;
	HAL_CHANNEL *chan = (HAL_CHANNEL *) ichan;
	struct ath_hal_5212 *ahp = AH5212(ah);

	if (ichan == AH_NULL)
		return;

	/* TPC for self-generated frames */
	if (IS_CHAN_5GHZ(chan)) {
		twiceAntennaGain = ahp->ah_antennaGainMax[0];
	} else {
		twiceAntennaGain = ahp->ah_antennaGainMax[1];
	}
	twiceAntennaReduction =
		ath_hal_getantennareduction(ah, chan, twiceAntennaGain);

	/* TPC for self-generated frames */
	ackTpcPow = MS(ahp->ah_macTPC, AR_TPC_ACK);
	if ((ackTpcPow-ahp->ah_txPowerIndexOffset) > ichan->maxTxPower)
		ackTpcPow = ichan->maxTxPower+ahp->ah_txPowerIndexOffset;

	if (ackTpcPow > (2*ichan->maxRegTxPower - twiceAntennaReduction))
		ackTpcPow = (2*ichan->maxRegTxPower - twiceAntennaReduction)
			+ ahp->ah_txPowerIndexOffset;

	ctsTpcPow = MS(ahp->ah_macTPC, AR_TPC_CTS);
	if ((ctsTpcPow-ahp->ah_txPowerIndexOffset) > ichan->maxTxPower)
		ctsTpcPow = ichan->maxTxPower+ahp->ah_txPowerIndexOffset;

	if (ctsTpcPow > (2*ichan->maxRegTxPower - twiceAntennaReduction))
		ctsTpcPow = (2*ichan->maxRegTxPower - twiceAntennaReduction)
			+ ahp->ah_txPowerIndexOffset;

	chirpTpcPow = MS(ahp->ah_macTPC, AR_TPC_CHIRP);
	if ((chirpTpcPow-ahp->ah_txPowerIndexOffset) > ichan->maxTxPower)
		chirpTpcPow = ichan->maxTxPower+ahp->ah_txPowerIndexOffset;

	if (chirpTpcPow > (2*ichan->maxRegTxPower - twiceAntennaReduction))
		chirpTpcPow = (2*ichan->maxRegTxPower - twiceAntennaReduction)
			+ ahp->ah_txPowerIndexOffset;

	if (ackTpcPow > 63)
		ackTpcPow = 63;
	if (ctsTpcPow > 63)
		ctsTpcPow = 63;
	if (chirpTpcPow > 63)
		chirpTpcPow = 63;

	powerVal = SM(ackTpcPow, AR_TPC_ACK) |
		SM(ctsTpcPow, AR_TPC_CTS) |
		SM(chirpTpcPow, AR_TPC_CHIRP);
	OS_REG_WRITE(ah, AR_TPC, powerVal);
}


#endif /* AH_SUPPORT_AR5212 */
