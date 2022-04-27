/*
 * Copyright (c) 2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_attach.c#6 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#if !defined(AH_SUPPORT_2133)
#error "No 5416 RF support defined"
#endif

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"
#include "ar5416/ar5416eeprom.h"

static HAL_BOOL ar5416FillCapabilityInfo(struct ath_hal *ah);
static HAL_STATUS ar5416GetEeprom(struct ath_hal *ah);
static u_int16_t ar5416GetSpurChan(struct ath_hal *ah, int i, HAL_BOOL is2GHz);
static HAL_BOOL ar5416ReadEepromCTLInfo(struct ath_hal *ah);

static void
ar5416AniSetup(struct ath_hal *ah)
{
	static const struct ar5212AniParams aniparams = {
		.maxNoiseImmunityLevel	= 4,	/* levels 0..4 */
		.totalSizeDesired	= { -34, -41, -48, -55, -62 },
		.coarseHigh		= { -18, -18, -16, -14, -12 },
		.coarseLow		= { -52, -56, -60, -64, -70 },
		.firpwr			= { -80, -82, -82, -82, -82 },
		.maxSpurImmunityLevel	= 2,
		.cycPwrThr1		= { 2, 4, 6, 8, 10, 12, 14, 16 },
		.maxFirstepLevel	= 2,	/* levels 0..2 */
		.firstep		= { 0, 4, 8 },
		.ofdmTrigHigh		= 500,
		.ofdmTrigLow		= 200,
		.cckTrigHigh		= 200,
		.cckTrigLow		= 100,
		.rssiThrHigh		= 40,
		.rssiThrLow		= 7,
		.period			= 100,
	};
	/* NB: ANI is not enabled yet */
	ar5212AniAttach(ah, &aniparams, &aniparams, AH_FALSE);
}

/*
 * Attach for an AR5416 part.
 */
struct ath_hal *
ar5416Attach(u_int16_t devid, HAL_SOFTC sc,
	HAL_BUS_TAG st, HAL_BUS_HANDLE sh, HAL_STATUS *status)
{
	struct ath_hal_5416 *ahp5416;
	struct ath_hal_5212 *ahp;
	struct ath_hal *ah;
	u_int32_t val, sum;
	HAL_STATUS ecode;
	HAL_BOOL rfStatus;
	u_int16_t eeval, antmask;
	int i;

	HALDEBUG(AH_NULL, "%s: sc %p st %u sh %p\n",
		__func__, sc, st, (void*) sh);

	/* NB: memory is returned zero'd */
	ahp5416 = ath_hal_malloc(sizeof (struct ath_hal_5416));
	if (ahp5416 == AH_NULL) {
		HALDEBUG(AH_NULL, "%s: cannot allocate memory for "
			"state block\n", __func__);
		*status = HAL_ENOMEM;
		return AH_NULL;
	}
	ahp = &ahp5416->ah_5212;
	ar5212InitState(ahp, devid, sc, st, sh, status);
	ah = &ahp->ah_priv.h;

	/* override 5212 methods for our needs */
	ah->ah_magic			= AR5416_MAGIC;
	ah->ah_getRateTable		= ar5416GetRateTable;
	ah->ah_detach			= ar5416Detach;
	ah->ah_getWirelessModes= ar5416GetWirelessModes;

	/* Reset functions */
	ah->ah_reset			= ar5416Reset;
	ah->ah_phyDisable		= ar5416PhyDisable;
	ah->ah_disable			= ar5416Disable;
	ah->ah_perCalibration		= ar5416PerCalibration;
	ah->ah_setTxPowerLimit		= ar5416SetTxPowerLimit;

	/* Transmit functions */
	ah->ah_setupTxDesc		= ar5416SetupTxDesc;
	ah->ah_setupXTxDesc		= ar5416SetupXTxDesc;
	ah->ah_fillTxDesc		= ar5416FillTxDesc;
	ah->ah_procTxDesc		= ar5416ProcTxDesc;

	/* Receive Functions */
	ah->ah_startPcuReceive		= ar5416StartPcuReceive;
	ah->ah_stopPcuReceive		= ar5416StopPcuReceive;
	ah->ah_setupRxDesc		= ar5416SetupRxDesc;
	ah->ah_procRxDesc		= ar5416ProcRxDesc;

	/* Misc Functions */
	ah->ah_setLedState		= ar5416SetLedState;
	ah->ah_gpioCfgOutput		= ar5416GpioCfgOutput;
	ah->ah_gpioCfgInput		= ar5416GpioCfgInput;
	ah->ah_gpioGet			= ar5416GpioGet;
	ah->ah_gpioSet			= ar5416GpioSet;
	ah->ah_gpioSetIntr		= ar5416GpioSetIntr;
	ah->ah_resetTsf			= ar5416ResetTsf;
	ah->ah_getRfGain		= ar5416GetRfgain;
	ah->ah_setAntennaSwitch		= ar5416SetAntennaSwitch;
	ah->ah_setDecompMask		= ar5416SetDecompMask;
	ah->ah_setCoverageClass		= ar5416SetCoverageClass;
	ah->ah_getCapability		= ar5416GetCapability,
	ah->ah_setCapability		= ar5416SetCapability,
#if 0
	ah->ah_getCCAThreshold		= ar5416GetCCAThreshold;
	ah->ah_setCCAThreshold		= ar5416SetCCAThreshold; 
#endif

	ah->ah_resetKeyCacheEntry	= ar5416ResetKeyCacheEntry;
	ah->ah_setKeyCacheEntry		= ar5416SetKeyCacheEntry;

	/* Power Management Functions */
	ah->ah_setPowerMode		= ar5416SetPowerMode;

	/* Beacon Management Functions */
	ah->ah_setBeaconTimers		= ar5416SetBeaconTimers;
	ah->ah_beaconInit		= ar5416BeaconInit;
	ah->ah_setStationBeaconTimers	= ar5416SetStaBeaconTimers;
	ah->ah_resetStationBeaconTimers	= ar5416ResetStaBeaconTimers;

	/* XXX 802.11n Functions */
#if 0
	ah->ah_chainTxDesc		= ar5416ChainTxDesc;
	ah->ah_setupFirstTxDesc		= ar5416SetupFirstTxDesc;
	ah->ah_setupLastTxDesc		= ar5416SetupLastTxDesc;
	ah->ah_set11nRateScenario	= ar5416Set11nRateScenario;
	ah->ah_set11nAggrMiddle		= ar5416Set11nAggrMiddle;
	ah->ah_clr11nAggr		= ar5416Clr11nAggr;
	ah->ah_set11nBurstDuration	= ar5416Set11nBurstDuration;
	ah->ah_get11nExtBusy		= ar5416Get11nExtBusy;
	ah->ah_set11nMac2040		= ar5416Set11nMac2040;
	ah->ah_get11nRxClear		= ar5416Get11nRxClear;
	ah->ah_set11nRxClear		= ar5416Set11nRxClear;
#endif

	/* Interrupt functions */
	ah->ah_isInterruptPending	= ar5416IsInterruptPending;
	ah->ah_getPendingInterrupts	= ar5416GetPendingInterrupts;
	ah->ah_setInterrupts		= ar5416SetInterrupts;

	ahp->ah_priv.ah_eepromRead	= ar5416EepromRead;
#ifdef AH_SUPPORT_WRITE_EEPROM
	ahp->ah_priv.ah_eepromWrite	= ar5416EepromWrite;
#endif
	ahp->ah_priv.ah_gpioCfgOutput	= ar5416GpioCfgOutput;
	ahp->ah_priv.ah_gpioCfgInput	= ar5416GpioCfgInput;
	ahp->ah_priv.ah_gpioGet		= ar5416GpioGet;
	ahp->ah_priv.ah_gpioSet		= ar5416GpioSet;
	ahp->ah_priv.ah_gpioSetIntr	= ar5416GpioSetIntr;
	ahp->ah_priv.ah_getChipPowerLimits = ar5416GetChipPowerLimits;
	ahp->ah_priv.ah_getSpurChan	= ar5416GetSpurChan;
	ahp->ah_priv.ah_refreshCalibration = ar5416ReadEepromCTLInfo;

	ahp->ah_rxDmaBurst = HAL_DMABURST_128B;
	ahp->ah_txDmaBurst = HAL_DMABURST_128B;

	if (!ar5416SetResetReg(ah, HAL_RESET_POWER_ON)) {
		/* reset chip */
		HALDEBUG(ah, "%s: couldn't reset chip\n", __func__);
		ecode = HAL_EIO;
		goto bad;
	}

	if (!ar5416SetPowerMode(ah, HAL_PM_AWAKE, AH_TRUE)) {
		HALDEBUG(ah, "%s: couldn't wakeup chip\n", __func__);
		ecode = HAL_EIO;
		goto bad;
	}
	/* Read Revisions from Chips before taking out of reset */
	val = OS_REG_READ(ah, AR_SREV) & AR_SREV_ID;
#if AH_BYTE_ORDER == AH_BIG_ENDIAN
	if (val == 0) {
		ah->ah_swapped = AH_TRUE;
		val = OS_REG_READ(ah, AR_SREV) & AR_SREV_ID;
	}
#endif
	if (val == 0xFF) {
		/* new SREV format */
		val = OS_REG_READ(ah, AR_SREV);
		AH_PRIVATE(ah)->ah_macVersion = MS(val, AR_SREV_VERSION2);
		AH_PRIVATE(ah)->ah_macVersion = (AH_PRIVATE(ah)->ah_macVersion << 4) | 0xF;
		AH_PRIVATE(ah)->ah_macRev = MS(val, AR_SREV_REVISION2);
	} else {
		AH_PRIVATE(ah)->ah_macVersion = MS(val, AR_SREV_VERSION);
		AH_PRIVATE(ah)->ah_macRev = val & AR_SREV_REVISION_N;
	}

	/*
	 * XXX - Do we need a board specific chain mask?
	 * Start by setting all Owl devices to 2x2
	 */
        AH5416(ah)->ah_rx_chainmask = ar5416EepromGet(ah, EEP_TXMASK);   
        AH5416(ah)->ah_tx_chainmask = ar5416EepromGet(ah, EEP_RXMASK);
        if (!AR_SREV_MERLIN(ah))
	{
//        AH5416(ah)->ah_rx_chainmask = (ar5416GpioGet(ah, 0)) ? 0x5 : AR5416_DEFAULT_RXCHAINMASK;
//        AH5416(ah)->ah_tx_chainmask = (ar5416GpioGet(ah, 0)) ? 0x5 : AR5416_DEFAULT_TXCHAINMASK;
        AH5416(ah)->ah_rx_chainmask = AR5416_DEFAULT_RXCHAINMASK;
        AH5416(ah)->ah_tx_chainmask = AR5416_DEFAULT_TXCHAINMASK;
	}

	AH5416(ah)->ah_clksel = 0;		/* XXX */
	/* NB: ah_keytype is initialized to zero which is ok */
#if 0
	ah->ah_descinfo.rxctl_numwords = RXCTL_NUMWORDS(ah);
	ah->ah_descinfo.rxctl_offset = RXCTL_OFFSET(ah);
	ah->ah_descinfo.rxstatus_numwords = RXSTATUS_NUMWORDS(ah);
	ah->ah_descinfo.rxstatus_offset = RXSTATUS_OFFSET(ah);

	ah->ah_descinfo.txctl_numwords = TXCTL_NUMWORDS(ah);
	ah->ah_descinfo.txctl_offset = TXCTL_OFFSET(ah);
	ah->ah_descinfo.txstatus_numwords = TXSTATUS_NUMWORDS(ah);
	ah->ah_descinfo.txstatus_offset = TXSTATUS_OFFSET(ah);
#endif
	if (!ar5416ChipReset(ah, AH_NULL)) {	/* reset chip */
		HALDEBUG(ah, "%s: chip reset failed\n", __func__);
		ecode = HAL_EIO;
		goto bad;
	}

	AH_PRIVATE(ah)->ah_phyRev = OS_REG_READ(ah, AR_PHY_CHIP_ID);

	if (!ar5212ChipTest(ah)) {
		HALDEBUG(ah, "%s: hardware self-test failed\n", __func__);
		ecode = HAL_ESELFTEST;
		goto bad;
	}

	/*
	 * Set correct Baseband to analog shift
	 * setting to access analog chips.
	 */
	OS_REG_WRITE(ah, AR_PHY(0), 0x00000007);

	/* Read Radio Chip Rev Extract */
	OS_DELAY(4000);
	AH_PRIVATE(ah)->ah_analog5GhzRev = ar5212GetRadioRev(ah);
	switch (AH_PRIVATE(ah)->ah_analog5GhzRev & AR_RADIO_SREV_MAJOR) {
        case AR_RAD2133_SREV_MAJOR:
        case AR_RAD2122_SREV_MAJOR:	
		break;
	default:
		if (AH_PRIVATE(ah)->ah_analog5GhzRev == 0) {
			AH_PRIVATE(ah)->ah_analog5GhzRev =
				AR_RAD5133_SREV_MAJOR;
			break;
		}
		/* NB: silently accept anything in release code per Atheros */
		HALDEBUG(ah, "%s: 5G Radio Chip Rev 0x%02X is not known to "
			"this driver\n", __func__,
			AH_PRIVATE(ah)->ah_analog5GhzRev);
	}

	ecode = ar5416GetEeprom(ah);
	if (ecode != HAL_OK)
		goto bad;

	/* fill in ob and db info */
	/* 5GHz params */
	ahp->ah_ob1 = (u_int16_t)ar5416EepromGet(ah, EEP_OB_5);
	ahp->ah_db1 = (u_int16_t)ar5416EepromGet(ah, EEP_DB_5);
	ahp->ah_ob2 = ahp->ah_ob1;
	ahp->ah_db2 = ahp->ah_db1;
	ahp->ah_ob3 = ahp->ah_ob1;
	ahp->ah_db3 = ahp->ah_db1;
	ahp->ah_ob4 = ahp->ah_ob1;
	ahp->ah_db4 = ahp->ah_db1;
    
	/* 2GHz Params */
	ahp->ah_obFor24 = (u_int16_t)ar5416EepromGet(ah, EEP_OB_2);
	ahp->ah_dbFor24 = (u_int16_t)ar5416EepromGet(ah, EEP_DB_2);
	ahp->ah_obFor24g = ahp->ah_obFor24;
	ahp->ah_dbFor24g = ahp->ah_dbFor24;
	ahp->ah_ob2GHz[0] = ahp->ah_obFor24;
	ahp->ah_db2GHz[0] = ahp->ah_dbFor24;
	ahp->ah_ob2GHz[1] = ahp->ah_obFor24;
	ahp->ah_db2GHz[1] = ahp->ah_dbFor24;

	/* No 32KHz crystal */
	ahp->ah_exist32kHzCrystal = AH_FALSE;

	ar5416ReadEepromCTLInfo(ah);		/* Get CTLs */

	/*
	 * Got everything we need now to setup the capabilities.
	 */
	if (!ar5416FillCapabilityInfo(ah)) {
		ecode = HAL_EEREAD;
		goto bad;
	}

	/* Get the maximum number of RX/TX chains */
	antmask = (u_int16_t) ar5416EepromGet(ah, EEP_TXMASK);
	for (sum=0; antmask; antmask = antmask >> 1)
		sum += antmask & 0x0001;
	AH_PRIVATE(ah)->ah_maxNumTxChain = (u_int8_t) sum;

	antmask = (u_int16_t) ar5416EepromGet(ah, EEP_RXMASK);
	for (sum=0; antmask; antmask = antmask >> 1)
		sum += antmask & 0x0001;
	AH_PRIVATE(ah)->ah_maxNumRxChain = (u_int8_t) sum;

	/* XXX How about the serial number ? */
	/* Read Reg Domain */
	ahp->ah_regdomain = ar5416EepromGet(ah, EEP_REG_0);
	AH_PRIVATE(ah)->ah_currentRD = ahp->ah_regdomain;

	/*
	 * ah_miscMode is populated by ar5416FillCapabilityInfo()
	 * starting from griffin. Set here to make sure that
	 * AR_MISC_MODE_MIC_NEW_LOC_ENABLE is set before a GTK is
	 * placed into hardware ( rdar://4628146 )
	 */
	if (ahp->ah_miscMode != 0)
		OS_REG_WRITE(ah, AR_MISC_MODE, ahp->ah_miscMode);

	rfStatus = AH_FALSE;
#ifdef AH_SUPPORT_2133
	switch (AH_PRIVATE(ah)->ah_analog5GhzRev & AR_RADIO_SREV_MAJOR) {
	case AR_RAD5133_SREV_MAJOR:
	case AR_RAD5122_SREV_MAJOR:
	case AR_RAD2133_SREV_MAJOR:
	case AR_RAD2122_SREV_MAJOR:
		rfStatus = ar2133RfAttach(ah, &ecode);
		HALDEBUG(ah, "%s: Attaching AR2133 radio\n", __func__);
		break;
	default:
		HALDEBUG(ah, "%s: Unknown radio (%X) connected to AR5416\n",__func__,AH_PRIVATE(ah)->ah_analog5GhzRev & AR_RADIO_SREV_MAJOR);
		ecode = HAL_ENOTSUPP;
	}
#else
	ecode = HAL_ENOTSUPP;
#endif
	if (!rfStatus) {
		HALDEBUG(ah, "%s: RF setup failed, status %u\n",
			__func__, ecode);
		goto bad;
	}
	/*
	 * Set noise floor adjust method; we arrange a
	 * direct call instead of thunking.
	 */
	AH_PRIVATE(ah)->ah_getNfAdjust = ahp->ah_rfHal.getNfAdjust;

	/* Initialize gain ladder thermal calibration structure */
	ar5212InitializeGainValues(ah);

	/* Get MAC Address */
	sum = 0;
	for (i = 0; i < 3; i++) {
		eeval = ar5416EepromGet(ah, AR_EEPROM_MAC(i));
		sum += eeval;
		ahp->ah_macaddr[2*i] = eeval >> 8;
		ahp->ah_macaddr[2*i + 1] = eeval & 0xff;
	}
	if (sum == 0 || sum == 0xffff*3) {
	        HALDEBUG(ah, "%s: mac address read failed: %s(0x%x)\n",
			 __func__, ath_hal_ether_sprintf(ahp->ah_macaddr),sum);
	        return AH_FALSE;
	}

	HALDEBUG(ah, "%s: return ah %p\n", __func__, ah);

	ar5416AniSetup(ah);
	/* Setup of Radar/AR structures happens in ath_hal_initchannels*/

	return ah;
bad:
	if (ahp)
		ar5416Detach((struct ath_hal *) ahp);
	if (status)
		*status = ecode;
	return AH_NULL;
}

void
ar5416Detach(struct ath_hal *ah)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	HALDEBUG(ah, "%s: ah %p\n", __func__, ah);

	HALASSERT(ah != AH_NULL);
	HALASSERT(ah->ah_magic == AR5416_MAGIC);

	ar5212AniDetach(ah);
	ar5212RfDetach(ah);
	ah->ah_disable(ah);
	ar5416SetPowerMode(ah, HAL_PM_FULL_SLEEP, AH_TRUE);
	ath_hal_free(ahp->ah_priv.ah_memchannels);
	ath_hal_free(ah);
}

/*
 * Fill all software cached or static hardware state information.
 * Return failure if capabilities are to come from EEPROM and
 * cannot be read.
 */
static HAL_BOOL
ar5416FillCapabilityInfo(struct ath_hal *ah)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	struct ath_hal_private *ahpriv = AH_PRIVATE(ah);
	HAL_CAPABILITIES *pCap = &ahpriv->ah_caps;
	u_int16_t eeval, capField;
	
	/* Construct wireless mode from EEPROM */
	pCap->halWirelessModes = 0;
	eeval = ar5416EepromGet(ah, EEP_OP_MODE);
	if (eeval & AR5416_OPFLAGS_11A) {
		pCap->halWirelessModes |= HAL_MODE_11A
				       |  HAL_MODE_11NA_HT20
				       |  HAL_MODE_11NA_HT40PLUS
				       |  HAL_MODE_11NA_HT40MINUS
				       ;
		ahp->ah_Amode = AH_TRUE;
	}
	if (eeval & AR5416_OPFLAGS_11G) {
		pCap->halWirelessModes |= HAL_MODE_11G
				       |  HAL_MODE_11NG_HT20
				       |  HAL_MODE_11NG_HT40PLUS
				       |  HAL_MODE_11NG_HT40MINUS
				       ;
    		ahp->ah_Gmode = AH_TRUE;
		ahp->ah_Bmode = AH_TRUE;
		/* WAR for 20833 */
		pCap->halWirelessModes |= HAL_MODE_11A
				       |  HAL_MODE_11NA_HT20
				       |  HAL_MODE_11NA_HT40PLUS
				       |  HAL_MODE_11NA_HT40MINUS
				       ;
	}

	/* Read the capability EEPROM location */
	capField = ar5416EepromGet(ah, EEP_OP_CAP);

	pCap->halLow2GhzChan = 2192;
	pCap->halHigh2GhzChan = 2732;

	pCap->halLow5GhzChan = 4915;
	pCap->halHigh5GhzChan = 6075;

	pCap->halCipherCkipSupport = AH_FALSE;
	pCap->halCipherTkipSupport = AH_TRUE;
	pCap->halCipherAesCcmSupport = !(capField & AR_EEPROM_EEPCAP_AES_DIS);

	pCap->halMicCkipSupport    = AH_FALSE;
	pCap->halMicTkipSupport    = AH_TRUE;
	pCap->halMicAesCcmSupport  = !(capField & AR_EEPROM_EEPCAP_AES_DIS);
	/*
	 * Starting with Griffin TX+RX mic keys can be combined
	 * in one key cache slot.
	 */
	pCap->halTkipMicTxRxKeySupport = AH_TRUE;
	pCap->halChanSpreadSupport = AH_TRUE;
	pCap->halSleepAfterBeaconBroken = AH_TRUE;

	pCap->halCompressSupport   =
		!(capField & AR_EEPROM_EEPCAP_COMPRESS_DIS) &&
		(pCap->halWirelessModes & (HAL_MODE_11A|HAL_MODE_11G)) != 0;
	pCap->halBurstSupport = !(capField & AR_EEPROM_EEPCAP_BURST_DIS);
	pCap->halFastFramesSupport =
		!(capField & AR_EEPROM_EEPCAP_FASTFRAME_DIS) &&
		(pCap->halWirelessModes & (HAL_MODE_11A|HAL_MODE_11G)) != 0;
	pCap->halChapTuningSupport = AH_TRUE;
	pCap->halTurboPrimeSupport = AH_TRUE;

	pCap->halTurboGSupport = pCap->halWirelessModes & HAL_MODE_108G;
	/* Give XR support unless both disable bits are set */
	pCap->halXrSupport = !(ahp->ah_disableXr5 && ahp->ah_disableXr2);

	pCap->halPSPollBroken = AH_TRUE;	/* XXX fixed in later revs? */
	pCap->halVEOLSupport = AH_TRUE;
	pCap->halBssIdMaskSupport = AH_TRUE;
	pCap->halMcastKeySrchSupport = AH_TRUE;
	pCap->halTsfAddSupport = AH_TRUE;

	if (capField & AR_EEPROM_EEPCAP_MAXQCU)
		pCap->halTotalQueues = MS(capField, AR_EEPROM_EEPCAP_MAXQCU);
	else
		pCap->halTotalQueues = HAL_NUM_TX_QUEUES;

	if (capField & AR_EEPROM_EEPCAP_KC_ENTRIES)
		pCap->halKeyCacheSize =
			1 << MS(capField, AR_EEPROM_EEPCAP_KC_ENTRIES);
	else
		pCap->halKeyCacheSize = AR5416_KEYTABLE_SIZE;

	/* XXX not needed */
	pCap->halChanHalfRate = AH_TRUE;
	pCap->halChanQuarterRate = AH_TRUE;
//	pCap->halChanHalfRate = AH_FALSE;
//	pCap->halChanQuarterRate = AH_FALSE;

	pCap->halHTSupport = AH_TRUE;
	pCap->halTstampPrecision = 32;
	pCap->halHwPhyCounterSupport = AH_TRUE;

	if (ahp->ah_rfKill &&
	    ath_hal_eepromRead(ah, EEP_RF_SILENT, &ahpriv->ah_rfsilent)) {
		/* NB: enabled by default */
		ahpriv->ah_rfkillEnabled = AH_TRUE;
		pCap->halRfSilentSupport = AH_TRUE;
	}

	ahpriv->ah_rxornIsFatal = AH_FALSE;

	return AH_TRUE;
}

#if 0
/*
 * Control inclusion of Target Power Override (<= 14.3)
 * define any of the following to enable the respective mode's override.
 */
#define AH_AR5416_OVRD_TGT_PWR_5G	/* 11a legacy */
#define AH_AR5416_OVRD_TGT_PWR_5GHT20	/* 11na HT20 */
#define AH_AR5416_OVRD_TGT_PWR_5GHT40	/* 11na HT40 */
#define AH_AR5416_OVRD_TGT_PWR_CCK	/* 11b legacy */
#define AH_AR5416_OVRD_TGT_PWR_2G	/* 11g legacy */
#define AH_AR5416_OVRD_TGT_PWR_2GHT20	/* 11ng HT20 */
#define AH_AR5416_OVRD_TGT_PWR_2GHT40	/* 11ng HT40 */
#endif

#if defined(AH_AR5416_OVRD_TGT_PWR_5G)		|| \
	defined(AH_AR5416_OVRD_TGT_PWR_5GHT20)	|| \
	defined(AH_AR5416_OVRD_TGT_PWR_5GHT40)	|| \
	defined(AH_AR5416_OVRD_TGT_PWR_CCK) 	|| \
	defined(AH_AR5416_OVRD_TGT_PWR_2G)		|| \
	defined(AH_AR5416_OVRD_TGT_PWR_2GHT20)	|| \
	defined(AH_AR5416_OVRD_TGT_PWR_2GHT40)
#define	AH_AR5416_OVRD_TGT_PWR		/* Control general feature */
static void ar5416OverrideTgtPower(struct ath_hal *ah, struct ar5416eeprom *);
#endif

u_int32_t
ar5416EepromGet(struct ath_hal *ah, EEPROM_PARAM param)
{
#define	CHAN_A_IDX	0
#define	CHAN_B_IDX	1
	struct ar5416eeprom *eep = &AH5416(ah)->ah_5416eeprom;
	MODAL_EEP_HEADER *pModal = eep->modalHeader;
	BASE_EEP_HEADER  *pBase  = &eep->baseEepHeader;

	switch (param) {
        case EEP_NFTHRESH_5:
		/* 5GHz Threshold is a signed value.
		 * Use cast to ensure proper sign extension
		 */
		return (int8_t)pModal[0].noiseFloorThreshCh[0];
        case EEP_NFTHRESH_2:
		/* 2GHz Threshold is a signed value. 
		 * Use cast to ensure proper sign extension
		 */
		return (int8_t)pModal[1].noiseFloorThreshCh[0];
        case AR_EEPROM_MAC(0):
		return pBase->macAddr[0] << 8 | pBase->macAddr[1];
        case AR_EEPROM_MAC(1):
		return pBase->macAddr[2] << 8 | pBase->macAddr[3];
        case AR_EEPROM_MAC(2):
		return pBase->macAddr[4] << 8 | pBase->macAddr[5];
        case EEP_REG_0:
		return pBase->regDmn[0];
        case EEP_REG_1:
		return pBase->regDmn[1];
        case EEP_OP_CAP:
		return pBase->deviceCap;
        case EEP_OP_MODE:
		return pBase->opCapFlags;
        case EEP_RF_SILENT:
		return pBase->rfSilent;
	case EEP_OB_5:
		return pModal[CHAN_A_IDX].ob;
    	case EEP_DB_5:
		return pModal[CHAN_A_IDX].db;
    	case EEP_OB_2:
		return pModal[CHAN_B_IDX].ob;
    	case EEP_DB_2:
		return pModal[CHAN_B_IDX].db;
    	case EEP_MINOR_REV:
		return pBase->version & AR5416_EEP_VER_MINOR_MASK;
	case EEP_TXMASK:
		return pBase->txMask;
	case EEP_RXMASK:
		return pBase->rxMask;
        default:
		HALASSERT(0);
		return 0;
    }
#undef CHAN_A_IDX
#undef CHAN_B_IDX
}

/* XXX conditionalize by target byte order */
#ifndef bswap16
static __inline__ u_int16_t
__bswap16(u_int16_t _x)
{
 	return ((u_int16_t)(
	      (((const u_int8_t *)(&_x))[0]    ) |
	      (((const u_int8_t *)(&_x))[1]<< 8))
	);
}
#endif

/* Do structure specific swaps if Eeprom format is non native to host */
static void
eepromSwap(struct ath_hal *ah)
{
	struct ar5416eeprom *eep = &AH5416(ah)->ah_5416eeprom;
	u_int32_t integer,i,j;
	u_int16_t word;
	MODAL_EEP_HEADER *pModal;

	/* convert Base Eep header */
	word = __bswap16(eep->baseEepHeader.length);
	eep->baseEepHeader.length = word;

	word = __bswap16(eep->baseEepHeader.checksum);
	eep->baseEepHeader.checksum = word;

	word = __bswap16(eep->baseEepHeader.version);
	eep->baseEepHeader.version = word;

	word = __bswap16(eep->baseEepHeader.regDmn[0]);
	eep->baseEepHeader.regDmn[0] = word;

	word = __bswap16(eep->baseEepHeader.regDmn[1]);
	eep->baseEepHeader.regDmn[1] = word;

	word = __bswap16(eep->baseEepHeader.rfSilent);
	eep->baseEepHeader.rfSilent = word;

	word = __bswap16(eep->baseEepHeader.blueToothOptions);
	eep->baseEepHeader.blueToothOptions = word; 

	word = __bswap16(eep->baseEepHeader.deviceCap);
	eep->baseEepHeader.deviceCap = word;

	/* convert Modal Eep header */
	for (j=0; j <2; j++) {

		pModal = &eep->modalHeader[j];
	
		/* XXX linux/ah_osdep.h only defines __bswap32 for BE */
		integer = __bswap32(pModal->antCtrlCommon);
		pModal->antCtrlCommon = integer;

		for (i=0; i<AR5416_MAX_CHAINS; i++) {
			integer = __bswap32(pModal->antCtrlChain[i]);
			pModal->antCtrlChain[i] = integer;
		}

		for (i=0; i<AR5416_EEPROM_MODAL_SPURS; i++) {
			word = __bswap16(pModal->spurChans[i].spurChan);
			pModal->spurChans[i].spurChan = word;
		}
	}
}

static HAL_STATUS
ar5416GetEeprom(struct ath_hal *ah)
{
#define	NW(a)	(sizeof(a) / sizeof(u_int16_t))
	struct ar5416eeprom *eep = &AH5416(ah)->ah_5416eeprom;
	u_int16_t *eep_data, magic;
	HAL_BOOL need_swap;
	u_int w, off, len;
	u_int32_t sum;
 
	if (!ath_hal_eepromRead(ah, AR5416_EEPROM_MAGIC_OFFSET, &magic)) {
		HALDEBUG(ah, "%s Error reading Eeprom MAGIC\n", __func__);
		return HAL_EEREAD;
	}
	HALDEBUG(ah, "%s Eeprom Magic = 0x%x\n", __func__, magic);
	if (magic != AR5416_EEPROM_MAGIC) {
		HALDEBUG(ah, "Bad magic number\n");
		return HAL_EEMAGIC;
	}

	eep_data = (u_int16_t *)eep;
	for (w = 0; w < NW(struct ar5416eeprom); w++) {
		off = owl_eep_start_loc + w;	/* NB: AP71 starts at 0 */
		if (!ath_hal_eepromRead(ah, off, &eep_data[w])) {
			HALDEBUG(ah, "%s eeprom read error at offset 0x%x\n", 
				__func__, off);
			return HAL_EEREAD;
		}
	}
	/* Convert to eeprom native eeprom endian format */
	if (isBigEndian()) {
		for (w = 0; w < NW(struct ar5416eeprom); w++)
			eep_data[w] = __bswap16(eep_data[w]);
	}

	/*
	 * At this point, we're in the native eeprom endian format
	 * Now, determine the eeprom endian by looking at byte 26??
	 */
	need_swap = ((eep->baseEepHeader.eepMisc & AR5416_EEPMISC_BIG_ENDIAN) != 0) ^ isBigEndian();
	if (need_swap) {
		HALDEBUG(ah, "Byte swap EEPROM contents.\n");
		len = __bswap16(eep->baseEepHeader.length);
	} else {
		len = eep->baseEepHeader.length;
	}
	len = (AH_MIN(len, sizeof(struct ar5416eeprom))) / sizeof(u_int16_t);
	
	/* Apply the checksum, done in native eeprom format */
	/* XXX - Need to check to make sure checksum calculation is done
	 * in the correct endian format.  Right now, it seems it would
	 * cast the raw data to host format and do the calculation, which may
	 * not be correct as the calculation may need to be done in the native
	 * eeprom format 
	 */
	sum = 0;
	for (w = 0; w < len; w++)
		sum ^= eep_data[w];
	/* Check CRC - Attach should fail on a bad checksum */
//	if (sum != 0xffff) {
//		HALDEBUG(ah, "Bad EEPROM checksum 0x%x (Len=%u)\n", sum, len);
//		return HAL_EEBADSUM;
//	}

	if (need_swap)
		eepromSwap(ah);		/* byte swap multi-byte data */

	HALDEBUG(ah,"%s Eeprom Version %u.%u\n",
		 __func__,owl_get_eep_ver(eep),owl_get_eep_rev(eep));

	/* NB: must be after all byte swapping */
	if (owl_get_eep_ver(eep) != AR5416_EEP_VER) {
		HALDEBUG(ah, "Bad EEPROM version 0x%x\n", owl_get_eep_ver(eep));
		return HAL_EEBADSUM;
	}

#ifdef AH_AR5416_OVRD_TGT_PWR
	/*
	 * 14.3 EEPROM contains low target powers.
	 * Hardcode until EEPROM > 14.3
	 */
	if (owl_get_eep_ver(eep) == 14 && owl_get_eep_rev(eep) <= 3) {
		HALDEBUG(ah, "Override Target Powers. "
			"EEPROM Version is %d.%d, Device Type %d\n",
			owl_get_eep_ver(eep), owl_get_eep_rev(eep),
			eep->baseEepHeader.deviceType);
		ar5416OverrideTgtPower(ah, eep);
	}
#endif /* AH_AR5416_OVRD_TGT_PWR */

	return HAL_OK;
#undef NW
}

static u_int16_t 
ar5416GetSpurChan(struct ath_hal *ah, int i, HAL_BOOL is2GHz)
{ 
	struct ar5416eeprom *eep = &AH5416(ah)->ah_5416eeprom;
	
	HALASSERT(0 <= i && i <  AR_EEPROM_MODAL_SPURS);
	return eep->modalHeader[is2GHz].spurChans[i].spurChan;
}

/**************************************************************************
 * fbin2freq
 *
 * Get channel value from binary representation held in eeprom
 * RETURNS: the frequency in MHz
 */
static u_int16_t
fbin2freq(u_int8_t fbin, HAL_BOOL is2GHz)
{
    /*
     * Reserved value 0xFF provides an empty definition both as
     * an fbin and as a frequency - do not convert
     */
    if (fbin == AR5416_BCHAN_UNUSED) {
        return fbin;
    }

    return (u_int16_t)((is2GHz) ? (2300 + fbin) : (4800 + 5 * fbin));
}

/*
 * Copy EEPROM Conformance Testing Limits contents 
 * into the allocated space
 */
/* USE CTLS from chain zero */ 
#define CTL_CHAIN	0 

static HAL_BOOL
ar5416ReadEepromCTLInfo(struct ath_hal *ah)
{
	HAL_EEPROM *ee = &AH5212(ah)->ah_eeprom;
	RD_EDGES_POWER *rep = ee->ee_rdEdgesPower;
	struct ar5416eeprom *pEeprom = &AH5416(ah)->ah_5416eeprom;
	int i, j;
	
	HALASSERT(AR5416_NUM_CTLS <= sizeof(ee->ee_rdEdgesPower)/(NUM_EDGES));

	for (i = 0; pEeprom->ctlIndex[i] != 0 && i < AR5416_NUM_CTLS; i++) {
		for (j = 0; j < NUM_EDGES; j ++) {
		/* XXX Confirm this is the right thing to do when an invalid channel is stored */
			if (pEeprom->ctlData[i].ctlEdges[CTL_CHAIN][j].bChannel == AR5416_BCHAN_UNUSED) {
				rep[j].rdEdge = 0;
				rep[j].twice_rdEdgePower = 0;
				rep[j].flag = 0;
			} else {
				rep[j].rdEdge = fbin2freq(pEeprom->ctlData[i].ctlEdges[CTL_CHAIN][j].bChannel, 
				((pEeprom->ctlIndex[i] & CTL_MODE_M) != CTL_11A) );
				rep[j].twice_rdEdgePower = pEeprom->ctlData[i].ctlEdges[CTL_CHAIN][j].tPower;
				rep[j].flag = (pEeprom->ctlData[i].ctlEdges[CTL_CHAIN][j].flag != 0);
			}
		}
		rep += NUM_EDGES;
	}
	ee->ee_numCtls = i;
	HALDEBUG(ah,"%s Numctls = %u\n",__func__,i);
	return AH_TRUE;
}

#ifdef AH_AR5416_OVRD_TGT_PWR
/*
 * Force override of Target Power values for AR5416, EEPROM versions <= 14.3
 */
static void
ar5416OverrideTgtPower(struct ath_hal *ah, struct ar5416eeprom *eep)
{

	int ii;
	CAL_TARGET_POWER_LEG	*force_5G;
	CAL_TARGET_POWER_HT 	*force_5GHT20;
	CAL_TARGET_POWER_HT 	*force_5GHT40;
	CAL_TARGET_POWER_LEG	*force_Cck;
	CAL_TARGET_POWER_LEG	*force_2G;
	CAL_TARGET_POWER_HT 	*force_2GHT20;
	CAL_TARGET_POWER_HT 	*force_2GHT40;

#define FB5(_f) ((_f - 4800)/5)
#define FB2(_f) (_f - 2300)
#define DH(_d)	(_d*2)

/* XB72 overrides */
static  CAL_TARGET_POWER_LEG	xbForceTargetPower5G[AR5416_NUM_5G_20_TARGET_POWERS] = { \
	{FB5(5180), {DH(17), DH(16), DH(15), DH(14)}},
	{FB5(5320), {DH(17), DH(16), DH(15), DH(14)}},
	{FB5(5700), {DH(17), DH(16), DH(15), DH(14)}},
	{FB5(5825), {DH(17), DH(16), DH(15), DH(14)}},
	{0xff,		{0, 0, 0, 0}},
	{0xff,		{0, 0, 0, 0}},
	{0xff,		{0, 0, 0, 0}},
	{0xff,		{0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	xbForceTargetPower5GHT20[AR5416_NUM_5G_20_TARGET_POWERS] = { \
	{FB5(5180), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5240), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5320), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5500), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5700), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5745), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5825), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	xbForceTargetPower5GHT40[AR5416_NUM_5G_40_TARGET_POWERS] = { \
	{FB5(5180), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5240), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5320), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5500), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5700), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5745), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{FB5(5825), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(16), DH(13), DH(10)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};

static  CAL_TARGET_POWER_LEG	xbForceTargetPowerCck[AR5416_NUM_2G_CCK_TARGET_POWERS] = { \
	{FB2(2412), {DH(19), DH(19), DH(19), DH(19)}},
	{FB2(2484), {DH(19), DH(19), DH(19), DH(19)}},
	{0xff,		{0, 0, 0, 0}}};

static  CAL_TARGET_POWER_LEG	xbForceTargetPower2G[AR5416_NUM_2G_20_TARGET_POWERS] = { \
	{FB2(2412), {DH(18), DH(17), DH(16), DH(15)}},
	{FB2(2437), {DH(18), DH(17), DH(16), DH(15)}},
	{FB2(2472), {DH(18), DH(17), DH(16), DH(15)}},
	{0xff,		{0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	xbForceTargetPower2GHT20[AR5416_NUM_2G_20_TARGET_POWERS] = { \
	{FB2(2412), {DH(18), DH(18), DH(18), DH(18), DH(17), DH(17), DH(13), DH(12)}},
	{FB2(2437), {DH(18), DH(18), DH(18), DH(18), DH(17), DH(17), DH(13), DH(12)}},
	{FB2(2472), {DH(18), DH(18), DH(18), DH(18), DH(17), DH(17), DH(13), DH(12)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	xbForceTargetPower2GHT40[AR5416_NUM_2G_40_TARGET_POWERS] = { \
	{FB2(2412), {DH(18), DH(18), DH(18), DH(18), DH(17), DH(17), DH(13), DH(12)}},
	{FB2(2437), {DH(18), DH(18), DH(18), DH(18), DH(17), DH(17), DH(13), DH(12)}},
	{FB2(2472), {DH(18), DH(18), DH(18), DH(18), DH(17), DH(17), DH(13), DH(12)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};

/* MB72 Overrides */
static  CAL_TARGET_POWER_LEG	mbForceTargetPower5G[AR5416_NUM_5G_20_TARGET_POWERS] = { \
	{FB5(5180), {DH(17), DH(16), DH(15), DH(13)}},
	{FB5(5320), {DH(17), DH(16), DH(15), DH(13)}},
	{FB5(5700), {DH(17), DH(16), DH(15), DH(13)}},
	{FB5(5825), {DH(17), DH(16), DH(15), DH(13)}},
	{0xff,		{0, 0, 0, 0}},
	{0xff,		{0, 0, 0, 0}},
	{0xff,		{0, 0, 0, 0}},
	{0xff,		{0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	mbForceTargetPower5GHT20[AR5416_NUM_5G_20_TARGET_POWERS] = { \
	{FB5(5180), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5240), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5320), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5500), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5700), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5745), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5825), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	mbForceTargetPower5GHT40[AR5416_NUM_5G_40_TARGET_POWERS] = { \
	{FB5(5180), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5240), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5320), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5500), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5700), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5745), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{FB5(5825), {DH(17), DH(17), DH(17), DH(17), DH(17), DH(17), DH(12), DH(10)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};


static  CAL_TARGET_POWER_LEG	mbForceTargetPowerCck[AR5416_NUM_2G_CCK_TARGET_POWERS] = { \
	{FB2(2412), {DH(19), DH(19), DH(19), DH(19)}},
	{FB2(2484), {DH(19), DH(19), DH(19), DH(19)}},
	{0xff,		{0, 0, 0, 0}}};

static  CAL_TARGET_POWER_LEG	mbForceTargetPower2G[AR5416_NUM_2G_20_TARGET_POWERS] = { \
	{FB2(2412), {DH(17), DH(17), DH(16), DH(13)}},
	{FB2(2437), {DH(17), DH(17), DH(16), DH(13)}},
	{FB2(2472), {DH(17), DH(17), DH(16), DH(13)}},
	{0xff,		{0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	mbForceTargetPower2GHT20[AR5416_NUM_2G_20_TARGET_POWERS] = { \
	{FB2(2412), {DH(19), DH(19), DH(19), DH(19), DH(19), DH(17), DH(13), DH(11)}},
	{FB2(2437), {DH(19), DH(19), DH(19), DH(19), DH(19), DH(17), DH(13), DH(11)}},
	{FB2(2472), {DH(19), DH(19), DH(19), DH(19), DH(19), DH(17), DH(13), DH(11)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};

static  CAL_TARGET_POWER_HT	mbForceTargetPower2GHT40[AR5416_NUM_2G_40_TARGET_POWERS] = { \
	{FB2(2412), {DH(19), DH(19), DH(19), DH(19), DH(19), DH(17), DH(13), DH(11)}},
	{FB2(2437), {DH(19), DH(19), DH(19), DH(19), DH(19), DH(17), DH(13), DH(11)}},
	{FB2(2472), {DH(19), DH(19), DH(19), DH(19), DH(19), DH(17), DH(13), DH(11)}},
	{0xff,		{0, 0, 0, 0, 0, 0, 0, 0}}};

#undef FB5
#undef FB2
#undef DH

	/* Device specific pointers */
	if (eep->baseEepHeader.deviceType == 3) {
		/* miniPci */
		force_5G	 = &mbForceTargetPower5G[0];
		force_5GHT20 = mbForceTargetPower5GHT20;
		force_5GHT40 = mbForceTargetPower5GHT40;
		force_Cck	 = mbForceTargetPowerCck;
		force_2G	 = mbForceTargetPower2G;
		force_2GHT20 = mbForceTargetPower2GHT20;
		force_2GHT40 = mbForceTargetPower2GHT40;
	}
	else if (eep->baseEepHeader.deviceType == 5) {
		/* PciExpress */
		force_5G	 = xbForceTargetPower5G;
		force_5GHT20 = xbForceTargetPower5GHT20;
		force_5GHT40 = xbForceTargetPower5GHT40;
		force_Cck	 = xbForceTargetPowerCck;
		force_2G	 = xbForceTargetPower2G;
		force_2GHT20 = xbForceTargetPower2GHT20;
		force_2GHT40 = xbForceTargetPower2GHT40;
	}
	else {
		return;
	}

	/* update ram copy of eeprom */
	ii = 0; /* happy compiler */
#ifdef AH_AR5416_OVRD_TGT_PWR_5G
	HALDEBUG(ah, "forced AH_AR5416_OVRD_TGT_PWR_5G\n");
	for (ii = 0; ii < AR5416_NUM_5G_20_TARGET_POWERS; ii++) {
		eep->calTargetPower5G[ii] = force_5G[ii];
	}
#endif

#ifdef AH_AR5416_OVRD_TGT_PWR_5GHT20
	HALDEBUG(ah, "forced AH_AR5416_OVRD_TGT_PWR_5GHT20\n");
	for (ii = 0; ii < AR5416_NUM_5G_20_TARGET_POWERS; ii++) {
		eep->calTargetPower5GHT20[ii] = force_5GHT20[ii];
	}
#endif

#ifdef AH_AR5416_OVRD_TGT_PWR_5GHT40
	HALDEBUG(ah, "forced AH_AR5416_OVRD_TGT_PWR_5GHT40\n");
	for (ii = 0; ii < AR5416_NUM_5G_40_TARGET_POWERS; ii++) {
		eep->calTargetPower5GHT40[ii] = force_5GHT40[ii];
	}
#endif

#ifdef AH_AR5416_OVRD_TGT_PWR_CCK
	HALDEBUG(ah, "forced AH_AR5416_OVRD_TGT_PWR_CCK\n");
	for (ii = 0; ii < AR5416_NUM_2G_CCK_TARGET_POWERS; ii++) {
		eep->calTargetPowerCck[ii] = force_Cck[ii];
	}
#endif

#ifdef AH_AR5416_OVRD_TGT_PWR_2G
	HALDEBUG(ah, "forced AH_AR5416_OVRD_TGT_PWR_2G\n");
	for (ii = 0; ii < AR5416_NUM_2G_20_TARGET_POWERS; ii++) {
		eep->calTargetPower2G[ii] = force_2G[ii];
	}
#endif

#ifdef AH_AR5416_OVRD_TGT_PWR_2GHT20
	HALDEBUG(ah, "forced AH_AR5416_OVRD_TGT_PWR_2GHT20\n");
	for (ii = 0; ii < AR5416_NUM_2G_20_TARGET_POWERS; ii++) {
		eep->calTargetPower2GHT20[ii] = force_2GHT20[ii];
	}
#endif

#ifdef AH_AR5416_OVRD_TGT_PWR_2GHT40
	HALDEBUG(ah, "forced AH_AR5416_OVRD_TGT_PWR_2GHT40\n");
	for (ii = 0; ii < AR5416_NUM_2G_40_TARGET_POWERS; ii++) {
		eep->calTargetPower2GHT40[ii] = force_2GHT40[ii];
	}
#endif
}
#endif /* AH_AR5416_OVRD_TGT_PWR */

#endif /* AH_SUPPORT_AR5416 */
