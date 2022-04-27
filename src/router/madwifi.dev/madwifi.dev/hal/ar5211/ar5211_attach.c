/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5211/ar5211_attach.c#11 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5211

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5211/ar5211.h"
#include "ar5211/ar5211reg.h"
#include "ar5211/ar5211phy.h"

static HAL_BOOL ar5211GetChannelEdges(struct ath_hal *ah,
		u_int16_t flags, u_int16_t *low, u_int16_t *high);
static HAL_BOOL ar5211GetChipPowerLimits(struct ath_hal *ah,
		HAL_CHANNEL *chans, u_int32_t nchans);

static const struct ath_hal_private ar5211hal = {{
	.ah_magic			= AR5211_MAGIC,
	.ah_abi				= HAL_ABI_VERSION,
	.ah_countryCode			= CTRY_DEFAULT,

	.ah_getRateTable		= ar5211GetRateTable,
	.ah_detach			= ar5211Detach,
	.ah_getWirelessModes		= ar5211GetWirelessModes,

	/* Reset Functions */
	.ah_reset			= ar5211Reset,
	.ah_phyDisable			= ar5211PhyDisable,
	.ah_disable			= ar5211Disable,
	.ah_setPCUConfig		= ar5211SetPCUConfig,
	.ah_perCalibration		= ar5211PerCalibration,
	.ah_setTxPowerLimit		= ar5211SetTxPowerLimit,
	.ah_getChanNoise		= ath_hal_getChanNoise,


	/* Transmit functions */
	.ah_updateTxTrigLevel		= ar5211UpdateTxTrigLevel,
	.ah_setupTxQueue		= ar5211SetupTxQueue,
	.ah_setTxQueueProps             = ar5211SetTxQueueProps,
	.ah_getTxQueueProps             = ar5211GetTxQueueProps,
	.ah_releaseTxQueue		= ar5211ReleaseTxQueue,
	.ah_resetTxQueue		= ar5211ResetTxQueue,
	.ah_getTxDP			= ar5211GetTxDP,
	.ah_setTxDP			= ar5211SetTxDP,
	.ah_numTxPending		= ar5211NumTxPending,
	.ah_startTxDma			= ar5211StartTxDma,
	.ah_stopTxDma			= ar5211StopTxDma,
	.ah_setupTxDesc			= ar5211SetupTxDesc,
	.ah_setupXTxDesc		= ar5211SetupXTxDesc,
	.ah_fillTxDesc			= ar5211FillTxDesc,
	.ah_procTxDesc			= ar5211ProcTxDesc,
	.ah_getTxIntrQueue		= AH_DUMMY,
	.ah_reqTxIntrDesc 		= ar5211IntrReqTxDesc,

	/* RX Functions */
	.ah_getRxDP			= ar5211GetRxDP,
	.ah_setRxDP			= ar5211SetRxDP,
	.ah_enableReceive		= ar5211EnableReceive,
	.ah_stopDmaReceive		= ar5211StopDmaReceive,
	.ah_startPcuReceive		= ar5211StartPcuReceive,
	.ah_stopPcuReceive		= ar5211StopPcuReceive,
	.ah_setMulticastFilter		= ar5211SetMulticastFilter,
	.ah_setMulticastFilterIndex	= ar5211SetMulticastFilterIndex,
	.ah_clrMulticastFilterIndex	= ar5211ClrMulticastFilterIndex,
	.ah_getRxFilter			= ar5211GetRxFilter,
	.ah_setRxFilter			= ar5211SetRxFilter,
	.ah_setupRxDesc			= ar5211SetupRxDesc,
	.ah_procRxDesc			= ar5211ProcRxDesc,
	.ah_rxMonitor			= AH_DUMMY,
	.ah_procMibEvent		= AH_DUMMY,

	/* Misc Functions */
	.ah_getCapability		= ar5211GetCapability,
	.ah_setCapability		= ar5211SetCapability,
	.ah_getDiagState		= ar5211GetDiagState,
	.ah_getMacAddress		= ar5211GetMacAddress,
	.ah_setMacAddress		= ar5211SetMacAddress,
	.ah_getBssIdMask		= ar5211GetBssIdMask,
	.ah_setBssIdMask		= ar5211SetBssIdMask,
	.ah_setRegulatoryDomain		= ar5211SetRegulatoryDomain,
	.ah_setLedState			= ar5211SetLedState,
	.ah_writeAssocid		= ar5211WriteAssocid,
	.ah_gpioCfgInput		= ar5211GpioCfgInput,
	.ah_gpioCfgOutput		= ar5211GpioCfgOutput,
	.ah_gpioGet			= ar5211GpioGet,
	.ah_gpioSet			= ar5211GpioSet,
	.ah_gpioSetIntr			= ar5211GpioSetIntr,
	.ah_getTsf32			= ar5211GetTsf32,
	.ah_getTsf64			= ar5211GetTsf64,
	.ah_resetTsf			= ar5211ResetTsf,
	.ah_detectCardPresent		= ar5211DetectCardPresent,
	.ah_updateMibCounters		= ar5211UpdateMibCounters,
	.ah_getRfGain			= ar5211GetRfgain,
	.ah_getDefAntenna		= ar5211GetDefAntenna,
	.ah_setDefAntenna		= ar5211SetDefAntenna,
	.ah_getAntennaSwitch		= ar5211GetAntennaSwitch,
	.ah_setAntennaSwitch		= ar5211SetAntennaSwitch,
	.ah_setSifsTime			= ar5211SetSifsTime,
	.ah_getSifsTime			= ar5211GetSifsTime,
	.ah_setSlotTime			= ar5211SetSlotTime,
	.ah_getSlotTime			= ar5211GetSlotTime,
	.ah_setAckTimeout		= ar5211SetAckTimeout,
	.ah_getAckTimeout		= ar5211GetAckTimeout,
	.ah_setAckCTSRate		= ar5211SetAckCTSRate,
	.ah_getAckCTSRate		= ar5211GetAckCTSRate,
	.ah_setCTSTimeout		= ar5211SetCTSTimeout,
	.ah_getCTSTimeout		= ar5211GetCTSTimeout,
	.ah_setDecompMask               = AH_DUMMY,
	.ah_setCoverageClass            = AH_DUMMY,

	/* Key Cache Functions */
	.ah_getKeyCacheSize		= ar5211GetKeyCacheSize,
	.ah_resetKeyCacheEntry		= ar5211ResetKeyCacheEntry,
	.ah_isKeyCacheEntryValid	= ar5211IsKeyCacheEntryValid,
	.ah_setKeyCacheEntry		= ar5211SetKeyCacheEntry,
	.ah_setKeyCacheEntryMac		= ar5211SetKeyCacheEntryMac,

	/* Power Management Functions */
	.ah_setPowerMode		= ar5211SetPowerMode,
	.ah_getPowerMode		= ar5211GetPowerMode,

	/* Beacon Functions */
	.ah_setBeaconTimers		= ar5211SetBeaconTimers,
	.ah_beaconInit			= ar5211BeaconInit,
	.ah_setStationBeaconTimers	= ar5211SetStaBeaconTimers,
	.ah_resetStationBeaconTimers	= ar5211ResetStaBeaconTimers,

	/* Interrupt Functions */
	.ah_isInterruptPending		= ar5211IsInterruptPending,
	.ah_getPendingInterrupts	= ar5211GetPendingInterrupts,
	.ah_getInterrupts		= ar5211GetInterrupts,
	.ah_setInterrupts		= ar5211SetInterrupts },

	.ah_getChannelEdges		= ar5211GetChannelEdges,
	.ah_eepromRead			= ar5211EepromRead,
#ifdef AH_SUPPORT_WRITE_EEPROM
	.ah_eepromWrite			= ar5211EepromWrite,
#endif
	.ah_gpioCfgInput		= ar5211GpioCfgInput,
	.ah_gpioCfgOutput		= ar5211GpioCfgOutput,
	.ah_gpioGet			= ar5211GpioGet,
	.ah_gpioSet			= ar5211GpioSet,
	.ah_gpioSetIntr			= ar5211GpioSetIntr,
	.ah_getChipPowerLimits		= ar5211GetChipPowerLimits,
};

static HAL_BOOL ar5211ChipTest(struct ath_hal *);
static HAL_BOOL ar5211FillCapabilityInfo(struct ath_hal *ah);

/*
 * TODO: Need to talk to Praveen about this, these are
 * not valid 2.4 channels, either we change these
 * or I need to change the beanie coding to accept these
 */
static const u_int16_t channels2_4[] = { 2412, 2447, 2484 };

/*
 * Return the revsion id for the radio chip.  This
 * fetched via the PHY.
 */
static u_int32_t
ar5211GetRadioRev(struct ath_hal *ah)
{
	u_int32_t val;
	int i;

	OS_REG_WRITE(ah, (AR_PHY_BASE + (0x34 << 2)), 0x00001c16);
	for (i = 0; i < 8; i++)
		OS_REG_WRITE(ah, (AR_PHY_BASE + (0x20 << 2)), 0x00010000);
	val = (OS_REG_READ(ah, AR_PHY_BASE + (256 << 2)) >> 24) & 0xff;
	val = ((val & 0xf0) >> 4) | ((val & 0x0f) << 4);
	return ath_hal_reverseBits(val, 8);
}

/*
 * Attach for an AR5211 part.
 */
struct ath_hal *
ar5211Attach(u_int16_t devid, HAL_SOFTC sc,
	HAL_BUS_TAG st, HAL_BUS_HANDLE sh, HAL_STATUS *status)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
	struct ath_hal_5211 *ahp;
	struct ath_hal *ah;
	u_int i;
	u_int32_t sum, val;
	u_int16_t eeval;
	HAL_STATUS ecode;

	HALDEBUG(AH_NULL, "%s: sc %p st %p sh %p\n",
		__func__, sc, (void*) st, (void*) sh);

	/* NB: memory is returned zero'd */
	ahp = ath_hal_malloc(sizeof (struct ath_hal_5211));
	if (ahp == AH_NULL) {
		HALDEBUG(AH_NULL, "%s: cannot allocate memory for state "
			"block\n", __func__);
		ecode = HAL_ENOMEM;
		goto bad;
	}
	ah = &ahp->ah_priv.h;
	/* set initial values */
	OS_MEMCPY(&ahp->ah_priv, &ar5211hal, sizeof(struct ath_hal_private));
	ahp->ah_priv.ah_memchannels = (HAL_CHANNEL_INTERNAL *) ath_hal_malloc(HAL_CHANNELS * sizeof(HAL_CHANNEL_INTERNAL));
	ah->ah_sc = sc;
	ah->ah_st = st;
	ah->ah_sh = sh;

	AH_PRIVATE(ah)->ah_devid = devid;
	AH_PRIVATE(ah)->ah_subvendorid = 0;	/* XXX */

	AH_PRIVATE(ah)->ah_powerLimit = MAX_RATE_POWER;
	AH_PRIVATE(ah)->ah_tpScale = HAL_TP_SCALE_MAX;	/* no scaling */

	ahp->ah_diversityControl = HAL_ANT_VARIABLE;
	ahp->ah_staId1Defaults = 0;
	ahp->ah_rssiThr = INIT_RSSI_THR;
	ahp->ah_sifstime = (u_int) -1;
	ahp->ah_slottime = (u_int) -1;
	ahp->ah_acktimeout = (u_int) -1;
	ahp->ah_ctstimeout = (u_int) -1;

	if (!ar5211ChipReset(ah, AH_FALSE)) {	/* reset chip */
		HALDEBUG(ah, "%s: chip reset failed\n", __func__);
		ecode = HAL_EIO;
		goto bad;
	}
	if (AH_PRIVATE(ah)->ah_devid == AR5211_FPGA11B) {
		/* set it back to OFDM mode to be able to read analog rev id */
		OS_REG_WRITE(ah, AR5211_PHY_MODE, AR5211_PHY_MODE_OFDM);
		OS_REG_WRITE(ah, AR_PHY_PLL_CTL, AR_PHY_PLL_CTL_44);
		OS_DELAY(1000);
	}

	/* Read Revisions from Chips */
	val = OS_REG_READ(ah, AR_SREV) & AR_SREV_ID_M;
#if AH_BYTE_ORDER == AH_BIG_ENDIAN
	if (((val >> AR_SREV_ID_S) == 0) && ((val & AR_SREV_REVISION_M) == 0)) {
		ah->ah_swapped = AH_TRUE;
		val = OS_REG_READ(ah, AR_SREV) & AR_SREV_ID_M;
	}
#endif
	AH_PRIVATE(ah)->ah_macVersion = val >> AR_SREV_ID_S;
	AH_PRIVATE(ah)->ah_macRev = val & AR_SREV_REVISION_M;

	if (AH_PRIVATE(ah)->ah_macVersion < AR_SREV_VERSION_MAUI_2 ||
	    AH_PRIVATE(ah)->ah_macVersion > AR_SREV_VERSION_OAHU) {
		HALDEBUG(ah, "%s: Mac Chip Rev 0x%x is not supported by this "
			"driver\n", __func__, AH_PRIVATE(ah)->ah_macVersion);
		ecode = HAL_ENOTSUPP;
		goto bad;
	}

	AH_PRIVATE(ah)->ah_phyRev = OS_REG_READ(ah, AR_PHY_CHIP_ID);

	if (!ar5211ChipTest(ah)) {
		HALDEBUG(ah, "%s: hardware self-test failed\n", __func__);
		ecode = HAL_ESELFTEST;
		goto bad;
	}

	/* Set correct Baseband to analog shift setting to access analog chips. */
	if (AH_PRIVATE(ah)->ah_macVersion >= AR_SREV_VERSION_OAHU) {
		OS_REG_WRITE(ah, AR_PHY_BASE, 0x00000007);
	} else {
		OS_REG_WRITE(ah, AR_PHY_BASE, 0x00000047);
	}
	OS_DELAY(2000);

	/* Read Radio Chip Rev Extract */
	AH_PRIVATE(ah)->ah_analog5GhzRev = ar5211GetRadioRev(ah);
	if ((AH_PRIVATE(ah)->ah_analog5GhzRev & 0xf0) != RAD5_SREV_MAJOR) {
		HALDEBUG(ah, "%s: 5G Radio Chip Rev 0x%02X is not supported by "
			"this driver\n", __func__,
			AH_PRIVATE(ah)->ah_analog5GhzRev);
		ecode = HAL_ENOTSUPP;
		goto bad;
	}

	if (!ar5211EepromRead(ah, AR_EEPROM_VERSION, &eeval)) {
		HALDEBUG(ah, "%s: unable to read EEPROM version\n", __func__);
		ecode = HAL_EEREAD;
		goto bad;
	}
	if (eeval < AR_EEPROM_VER3) {
		HALDEBUG(ah, "%s: unsupported EEPROM version "
			"%u (0x%x) found\n", __func__, eeval, eeval);
		ecode = HAL_EEVERSION;
		goto bad;
	}
	ahp->ah_eeversion = eeval;

	val = (OS_REG_READ(ah, AR_PCICFG) & AR_PCICFG_EEPROM_SIZE_M) >>
               AR_PCICFG_EEPROM_SIZE_S;
	if (val != AR_PCICFG_EEPROM_SIZE_16K) {
		HALDEBUG(ah, "%s: unsupported EEPROM size "
			"%u (0x%x) found\n", __func__, val, val);
		ecode = HAL_EESIZE;
		goto bad;
	}

	if (!ar5211EepromRead(ah, AR_EEPROM_PROTECT, &eeval)) {
		HALDEBUG(ah, "%s: cannot read EEPROM protection "
			"bits; read locked?\n", __func__);
		ecode = HAL_EEREAD;
		goto bad;
	}
	HALDEBUG(ah, "EEPROM protect 0x%x\n", eeval);
	ahp->ah_eeprotect = eeval;
	/* XXX check proper access before continuing */

	/*
	 * Read the Atheros EEPROM entries and calculate the checksum.
	 */
	sum = 0;
	for (i = 0; i < AR_EEPROM_ATHEROS_MAX; i++) {
		if (!ar5211EepromRead(ah, AR_EEPROM_ATHEROS(i), &eeval)) {
			ecode = HAL_EEREAD;
			goto bad;
		}
		sum ^= eeval;
	}
	if (sum != 0xffff) {
		HALDEBUG(ah, "%s: bad EEPROM checksum 0x%x\n", __func__, sum);
		ecode = HAL_EEBADSUM;
		goto bad;
	}

	ahp->ah_numChannels11a = NUM_11A_EEPROM_CHANNELS;
	ahp->ah_numChannels2_4 = NUM_2_4_EEPROM_CHANNELS;

	for (i = 0; i < NUM_11A_EEPROM_CHANNELS; i ++)
		ahp->ah_dataPerChannel11a[i].numPcdacValues = NUM_PCDAC_VALUES;

	/* the channel list for 2.4 is fixed, fill this in here */
	for (i = 0; i < NUM_2_4_EEPROM_CHANNELS; i++) {
		ahp->ah_channels11b[i] = channels2_4[i];
		ahp->ah_channels11g[i] = channels2_4[i];
		ahp->ah_dataPerChannel11b[i].numPcdacValues = NUM_PCDAC_VALUES;
		ahp->ah_dataPerChannel11g[i].numPcdacValues = NUM_PCDAC_VALUES;
	}

	if (!ath_hal_readEepromIntoDataset(ah, &ahp->ah_eeprom)) {
		ecode = HAL_EEREAD;	/* XXX */
		goto bad;
        }

        /* If Bmode and AR5211, verify 2.4 analog exists */
	if (AH_PRIVATE(ah)->ah_macVersion >= AR_SREV_VERSION_OAHU &&
	    ahp->ah_Bmode) {
		/* Set correct Baseband to analog shift setting to access analog chips. */
		OS_REG_WRITE(ah, AR_PHY_BASE, 0x00004007);
		OS_DELAY(2000);
		AH_PRIVATE(ah)->ah_analog2GhzRev = ar5211GetRadioRev(ah);

		/* Set baseband for 5GHz chip */
		OS_REG_WRITE(ah, AR_PHY_BASE, 0x00000007);
		OS_DELAY(2000);
		if ((AH_PRIVATE(ah)->ah_analog2GhzRev & 0xF0) != RAD2_SREV_MAJOR) {
			HALDEBUG(ah, "%s: 2G Radio Chip Rev 0x%x is "
				"not supported by this driver\n",
				__func__, AH_PRIVATE(ah)->ah_analog2GhzRev);
			ecode = HAL_ENOTSUPP;
			goto bad;
		}
	} else {
		ahp->ah_Bmode = 0;
        }

/*        if (!ar5211EepromRead(ah, AR_EEPROM_REG_DOMAIN, &eeval)) {
		HALDEBUG(ah, "%s: cannot read regulator domain from EEPROM\n",
			__func__);
		ecode = HAL_EEREAD;
		goto bad;
        }*/
        eeval = 0x10;
	ahp->ah_regdomain = eeval;
	AH_PRIVATE(ah)->ah_currentRD = ahp->ah_regdomain;
	AH_PRIVATE(ah)->ah_getNfAdjust = ar5211GetNfAdjust;
	
	/*
	 * Got everything we need now to setup the capabilities.
	 */
	(void) ar5211FillCapabilityInfo(ah);

	/* Initialize gain ladder thermal calibration structure */
	ar5211InitializeGainValues(ah);

	sum = 0;
	for (i = 0; i < 3; i++) {
		if (!ar5211EepromRead(ah, AR_EEPROM_MAC(2-i), &eeval)) {
			HALDEBUG(ah, "%s: cannot read EEPROM "
				"location %u\n", __func__, i);
			ecode = HAL_EEREAD;
			goto bad;
		}
		sum += eeval;
		ahp->ah_macaddr[2*i] = eeval >> 8;
		ahp->ah_macaddr[2*i + 1] = eeval & 0xff;
	}
	if (sum == 0 || sum == 0xffff*3) {
		HALDEBUG(ah, "%s: mac address read failed: %s\n",
		    __func__, ath_hal_ether_sprintf(ahp->ah_macaddr));
		ecode = HAL_EEBADMAC;
		goto bad;
	}

	HALDEBUG(ah, "%s: return ah %p\n", __func__, ah);

	return ah;
bad:
	if (ahp)
		ar5211Detach((struct ath_hal *) ahp);
	if (status)
		*status = ecode;
	return AH_NULL;
#undef N
}

void
ar5211Detach(struct ath_hal *ah)
{
	struct ath_hal_5211 *ahp = AH5211(ah);
	HALDEBUG(ah, "%s: ah %p\n", __func__, ah);

	HALASSERT(ah != AH_NULL);
	HALASSERT(ah->ah_magic == AR5211_MAGIC);

	ath_hal_eepromDetach(ah, &AH5211(ah)->ah_eeprom);
	ath_hal_free(ahp->ah_priv.ah_memchannels);
	ath_hal_free(ah);
	
}

static HAL_BOOL
ar5211ChipTest(struct ath_hal *ah)
{
	u_int32_t regAddr[2] = { AR_STA_ID0, AR_PHY_BASE+(8 << 2) };
	u_int32_t regHold[2];
	u_int32_t patternData[4] =
	    { 0x55555555, 0xaaaaaaaa, 0x66666666, 0x99999999 };
	int i, j;

	/* Test PHY & MAC registers */
	for (i = 0; i < 2; i++) {
		u_int32_t addr = regAddr[i];
		u_int32_t wrData, rdData;

		regHold[i] = OS_REG_READ(ah, addr);
		for (j = 0; j < 0x100; j++) {
			wrData = (j << 16) | j;
			OS_REG_WRITE(ah, addr, wrData);
			rdData = OS_REG_READ(ah, addr);
			if (rdData != wrData) {
				HALDEBUG(ah,
"%s: address test failed addr: 0x%08x - wr:0x%08x != rd:0x%08x\n",
				__func__, addr, wrData, rdData);
				return AH_FALSE;
			}
		}
		for (j = 0; j < 4; j++) {
			wrData = patternData[j];
			OS_REG_WRITE(ah, addr, wrData);
			rdData = OS_REG_READ(ah, addr);
			if (wrData != rdData) {
				HALDEBUG(ah,
"%s: address test failed addr: 0x%08x - wr:0x%08x != rd:0x%08x\n",
					__func__, addr, wrData, rdData);
				return AH_FALSE;
			}
		}
		OS_REG_WRITE(ah, regAddr[i], regHold[i]);
	}
	OS_DELAY(100);
	return AH_TRUE;
}

/*
 * Store the channel edges for the requested operational mode
 */
static HAL_BOOL
ar5211GetChannelEdges(struct ath_hal *ah,
	u_int16_t flags, u_int16_t *low, u_int16_t *high)
{
	if (flags & CHANNEL_5GHZ) {
		*low = 4920;
		*high = 6075;
		return AH_TRUE;
	}
	if (flags & CHANNEL_2GHZ && AH5211(ah)->ah_Bmode) {
		*low = 2312;
		*high = 2732;
		return AH_TRUE;
	}
	return AH_FALSE;
}

static HAL_BOOL
ar5211GetChipPowerLimits(struct ath_hal *ah, HAL_CHANNEL *chans, u_int32_t nchans)
{
	HAL_CHANNEL *chan;
	int i;

	/* XXX fill in, this is just a placeholder */
	for (i = 0; i < nchans; i++) {
		chan = &chans[i];
		HALDEBUG(ah, "%s: no min/max power for %u/0x%x\n",
		    __func__, chan->channel, chan->channelFlags);
		chan->maxTxPower = MAX_RATE_POWER;
		chan->minTxPower = 0;
	}
	return AH_TRUE;
}

/*
 * Fill all software cached or static hardware state information.
 */
static HAL_BOOL
ar5211FillCapabilityInfo(struct ath_hal *ah)
{
	struct ath_hal_5211 *ahp = AH5211(ah);
	struct ath_hal_private *ahpriv = AH_PRIVATE(ah);
	HAL_CAPABILITIES *pCap = &ahpriv->ah_caps;

        /* TODO: Remove this hack to fix bad reg domains */
        if (AH_PRIVATE(ah)->ah_currentRD == 1) {
		HAL_STATUS ecode;

		HALDEBUG(ah, "Warning, fixing regulatory domain of 1\n");
		/* XXX check return value */
		ar5211SetRegulatoryDomain(ah, 0x10, &ecode);
        }

	/* Construct wireless mode from EEPROM */
	pCap->halWirelessModes = 0;
	if (ahp->ah_Amode) {
		pCap->halWirelessModes |= HAL_MODE_11A;
		if (!ahp->ah_turbo5Disable)
			pCap->halWirelessModes |= HAL_MODE_TURBO;
	}
	if (ahp->ah_Bmode)
		pCap->halWirelessModes |= HAL_MODE_11B;

	pCap->halLow2GhzChan = 2312;
	pCap->halHigh2GhzChan = 2732;
	pCap->halLow5GhzChan = 4920;
	pCap->halHigh5GhzChan = 6075;

	pCap->halChanSpreadSupport = AH_TRUE;
	pCap->halSleepAfterBeaconBroken = AH_TRUE;
	pCap->halPSPollBroken = AH_TRUE;
	pCap->halVEOLSupport = AH_TRUE;

	pCap->halTotalQueues = HAL_NUM_TX_QUEUES;
	pCap->halKeyCacheSize = 128;

	/* XXX not needed */
	pCap->halChanHalfRate = AH_FALSE;
	pCap->halChanQuarterRate = AH_FALSE;

	if (ahp->ah_rfKill &&
	    ar5211EepromRead(ah, AR_EEPROM_RFSILENT, &ahpriv->ah_rfsilent)) {
		/* NB: enabled by default */
		ahpriv->ah_rfkillEnabled = AH_TRUE;
		pCap->halRfSilentSupport = AH_TRUE;
	}

	pCap->halTstampPrecision = 13;

	/* XXX might be ok w/ some chip revs */
	ahpriv->ah_rxornIsFatal = AH_TRUE;
	return AH_TRUE;
}
#endif /* AH_SUPPORT_AR5211 */
