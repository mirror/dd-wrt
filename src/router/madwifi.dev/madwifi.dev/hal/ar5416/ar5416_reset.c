/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_reset.c#7 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_xr.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"

/* Add static register initialization vectors */
#include "ar5416/ar5416.ini"
#ifdef AH_SUPPORT_AR9000
#include "ar5416/ar5416_sowl.ini"
#endif
#ifdef AH_SUPPORT_AR9200
#include "ar5416/ar9280.ini"
#include "ar5416/ar9280_merlin2.ini"
#endif

/*
 * Temporary cap on maximum power used in rate => power table
 * as workaround for board issues.
 */
#if 0
#define TEMP_POWER_CAP      1
#endif

/* Eeprom versioning macros. Returns true if the version is equal or newer than the ver specified */ 
#define IS_EEP_MINOR_V2(_ah) (ar5416EepromGet( (_ah), EEP_MINOR_REV)  >= AR5416_EEP_MINOR_VER_2)
#define IS_EEP_MINOR_V3(_ah) (ar5416EepromGet( (_ah), EEP_MINOR_REV)  >= AR5416_EEP_MINOR_VER_3)
#define IS_EEP_MINOR_V7(_ah) (ar5416EepromGet( (_ah), EEP_MINOR_REV)  >= AR5416_EEP_MINOR_VER_7)
#define IS_EEP_MINOR_V9(_ah) (ar5416EepromGet( (_ah), EEP_MINOR_REV)  >= AR5416_EEP_MINOR_VER_9)

/* Additional Time delay to wait after activiting the Base band */
#define BASE_ACTIVATE_DELAY	100	/* 100 usec */
#define PLL_SETTLE_DELAY	300	/* 300 usec */
#define RTC_PLL_SETTLE_DELAY    1000    /* 1 ms     */

static HAL_BOOL ar5416SetTransmitPower(struct ath_hal *ah, 
	HAL_CHANNEL_INTERNAL *chan, u_int16_t *rfXpdGain);

#if 0
static HAL_BOOL	ar5416ChannelChange(struct ath_hal *, HAL_CHANNEL *);
#endif
static int16_t ar5416GetNf(struct ath_hal *, HAL_CHANNEL_INTERNAL *);
static void ar5416SetDeltaSlope(struct ath_hal *, HAL_CHANNEL_INTERNAL *);
static void ar5416SpurMitigate(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan);
#ifdef AH_SUPPORT_AR9200
static void ar9280SpurMitigate(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan);
#endif

/* Owl specific stuff */
#define NUM_NOISEFLOOR_READINGS 6       /* 3 chains * (ctl + ext) */

HAL_BOOL ar5212SetXrMode(struct ath_hal *ah, HAL_OPMODE opmode,HAL_CHANNEL *chan);

static HAL_BOOL ar5416SetResetPowerOn(struct ath_hal *ah);
static HAL_BOOL ar5416SetReset(struct ath_hal *ah, int type);
static void ar5416InitPLL(struct ath_hal *ah, HAL_CHANNEL *chan);
static HAL_BOOL ar5416SetBoardValues(struct ath_hal *, HAL_CHANNEL_INTERNAL *);
static HAL_BOOL ar5416SetPowerPerRateTable(struct ath_hal *ah,
	struct ar5416eeprom *pEepData, 
	HAL_CHANNEL_INTERNAL *chan, int16_t *ratesArray,
	u_int16_t cfgCtl, u_int16_t AntennaReduction,
	u_int16_t twiceMaxRegulatoryPower, 
	u_int16_t powerLimit);
static HAL_BOOL ar5416SetPowerCalTable(struct ath_hal *ah,
	struct ar5416eeprom *pEepData,
	HAL_CHANNEL_INTERNAL *chan,
	int16_t *pTxPowerIndexOffset);
static u_int16_t ar5416GetMaxEdgePower(u_int16_t freq,
	CAL_CTL_EDGES *pRdEdgesPower, HAL_BOOL is2GHz);
static void ar5416GetTargetPowers(struct ath_hal *ah, 
	HAL_CHANNEL_INTERNAL *chan, CAL_TARGET_POWER_HT *powInfo,
	u_int16_t numChannels, CAL_TARGET_POWER_HT *pNewPower,
	u_int16_t numRates, HAL_BOOL isHt40Target);
static void ar5416GetTargetPowersLeg(struct ath_hal *ah, 
	HAL_CHANNEL_INTERNAL *chan, CAL_TARGET_POWER_LEG *powInfo,
	u_int16_t numChannels, CAL_TARGET_POWER_LEG *pNewPower,
	u_int16_t numRates, HAL_BOOL isExtTarget);

static void ar5416Set11nRegs(struct ath_hal *ah, HAL_CHANNEL *chan);
static void ar5416GetGainBoundariesAndPdadcs(struct ath_hal *ah, 
	HAL_CHANNEL_INTERNAL *chan, CAL_DATA_PER_FREQ *pRawDataSet,
	u_int8_t * bChans, u_int16_t availPiers,
	u_int16_t tPdGainOverlap, int16_t *pMinCalPower,
	u_int16_t * pPdGainBoundaries, u_int8_t * pPDADCValues,
	u_int16_t numXpdGains);
static HAL_BOOL getLowerUpperIndex(u_int8_t target, u_int8_t *pList,
	u_int16_t listSize,  u_int16_t *indexL, u_int16_t *indexR);
static HAL_BOOL ar5416FillVpdTable(u_int8_t pwrMin, u_int8_t pwrMax,
	u_int8_t *pPwrList, u_int8_t *pVpdList,
	u_int16_t numIntercepts, u_int8_t *pRetVpdList);

static void ar5416GetNoiseFloor(struct ath_hal *ah,
	u_int16_t nfarray[NUM_NOISEFLOOR_READINGS]);
#ifdef AH_SUPPORT_AR9000
static void ar5416EepromSetSowlAddac(struct ath_hal *ah,
	HAL_CHANNEL_INTERNAL *chan, int *regWrites);
#endif


/*
 * WAR for bug 6773.  OS_DELAY() does a PIO READ on the PCI bus which allows
 * other cards' DMA reads to complete in the middle of our reset.
 */
#define WAR_6773(x) do {		\
	if ((++(x) % 64) == 0)		\
		OS_DELAY(1);		\
} while (0)

#define IS_TIMER_REG(_reg) \
	(((_reg) >= AR_NEXT_TBTT_TIMER) && ((_reg) <= AR_TIMER_MODE))

#define REG_WRITE_ARRAY_9280(regArray, column, regWr) do {	\
	int r;								\
	for (r = 0; r < N(regArray); r++) {				\
		u_int32_t reg = (regArray)[r][0]; \
		u_int32_t value = ar9280IniFixup(ah,reg,(regArray)[r][(column)]); \
		OS_REG_WRITE(ah, reg, value);\
		WAR_6773(regWr);					\
		if ((reg >= 0x00007800) && (reg <= 0x00007898)) \
			OS_DELAY(100); \
	}								\
} while (0)

#define REG_WRITE_ARRAY(regArray, column, regWr) do {	\
	int r;								\
	for (r = 0; r < N(regArray); r++) {				\
		u_int32_t reg = (regArray)[r][0]; \
		u_int32_t value = (regArray)[r][(column)]; \
		OS_REG_WRITE(ah, reg, value);\
		WAR_6773(regWr);					\
	}								\
} while (0)

#ifdef AH_SUPPORT_AR9200
static inline u_int32_t
ar9280IniFixup(struct ath_hal *ah, u_int32_t reg, u_int32_t value)
{
    struct ar5416eeprom *eep = &AH5416(ah)->ah_5416eeprom;
    BASE_EEP_HEADER  *pBase  = &eep->baseEepHeader;

	switch(reg) {
	/*
	 ** Need to set the external/internal regulator bit to the proper value on 9280.
	 ** Can only write this ONCE.
	 */
	case AR_AN_TOP2:
		switch (AH_PRIVATE(ah)->ah_devid) {
	    case AR5416_DEVID_AR9280_PCI:
			/*
			 ** Check for an EEPROM data structure of "0x0b" or better
			 */
            if ( (pBase->version & 0xff) > 0x0a) {
                value &= ~AR_AN_TOP2_PWDCLKIND;
                value |= AR_AN_TOP2_PWDCLKIND & ( pBase->pwdclkind <<  AR_AN_TOP2_PWDCLKIND_S);
            }
			break;
		}
    }
    return value;
}
#endif


static inline void
ar5416InitChainMasks(struct ath_hal *ah)
{
    struct ath_hal_5416 *ahp = AH5416(ah);
    int rx_chainmask, tx_chainmask;

    rx_chainmask = ahp->ah_rx_chainmask;
    tx_chainmask = ahp->ah_tx_chainmask;

    switch (rx_chainmask) {
        case 0x5:
            OS_REG_SET_BIT(ah, AR_PHY_ANALOG_SWAP, AR_PHY_SWAP_ALT_CHAIN);
            /*
             * fall through !
             */
        case 0x3:
            if ((AH_PRIVATE(ah))->ah_macVersion <= AR_SREV_VERSION_SOWL) {
                /*
                 * workaround for OWL 1.0 cal failure, always cal 3 chains for
                 * multi chain -- then after cal set true mask value
                 */
                OS_REG_WRITE(ah, AR_PHY_RX_CHAINMASK, 0x7);
                OS_REG_WRITE(ah, AR_PHY_CAL_CHAINMASK, 0x7);
                break;
            }
            /*
             * fall through !
             */
        case 0x1:
        case 0x7:
            OS_REG_WRITE(ah, AR_PHY_RX_CHAINMASK, rx_chainmask);
            OS_REG_WRITE(ah, AR_PHY_CAL_CHAINMASK, rx_chainmask);
            break;
        default:
            break;
    }

    OS_REG_WRITE(ah, AR_SELFGEN_MASK, tx_chainmask);

    if (tx_chainmask == 0x5)
        OS_REG_SET_BIT(ah, AR_PHY_ANALOG_SWAP, AR_PHY_SWAP_ALT_CHAIN);

     if(AR_SREV_HOWL(ah))
         OS_REG_WRITE(ah, AR_PHY_ANALOG_SWAP,
             OS_REG_READ(ah, AR_PHY_ANALOG_SWAP) | 0x00000001);
}


/*
 * Places the device in and out of reset and then places sane
 * values in the registers based on EEPROM config, initialization
 * vectors (as determined by the mode), and station configuration
 *
 * resetType is used to preserve DMA/PCU registers across
 * a HW Reset during channel change.
 */
HAL_BOOL
ar5416Reset(struct ath_hal *ah, HAL_OPMODE opmode,
	HAL_CHANNEL *chan, HAL_RESET_TYPE resetType, HAL_STATUS *status)
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
#define	FAIL(_code)	do { ecode = _code; goto bad; } while (0)
	struct ath_hal_5212 *ahp = AH5212(ah);
	HAL_CHANNEL_INTERNAL *ichan;
	u_int32_t softLedCfg;
	u_int32_t saveFrameSeqCount, saveDefAntenna, saveLedState;
	u_int32_t macStaId1, synthDelay;
	u_int16_t rfXpdGain[2];
	u_int modesIndex, freqIndex;
	HAL_STATUS ecode;
	int i, regWrites = 0;
	u_int32_t timerMode = 0;
	u_int32_t testReg, powerVal;
	u_int32_t ackTpcPow, ctsTpcPow, chirpTpcPow;
#ifdef AH_SUPPORT_PPMUPDATE
	u_int32_t saveForceVal=0, tmpReg;
#endif /* AH_SUPPORT_PPMUPDATE */

	OS_MARK(ah, AH_MARK_RESET, resetType);
#define	IS(_c,_f)	(((_c)->channelFlags & _f) || 0)
	if ((IS(chan, CHANNEL_2GHZ) ^ IS(chan, CHANNEL_5GHZ)) == 0) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; not marked as "
			 "2GHz or 5GHz\n", __func__,
			chan->channel, chan->channelFlags);
		FAIL(HAL_EINVAL);
	}
	if ((IS(chan, CHANNEL_OFDM) ^ IS(chan, CHANNEL_CCK)) == 0) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; not marked as "
			"OFDM or CCK\n", __func__,
			chan->channel, chan->channelFlags);
		FAIL(HAL_EINVAL);
	}
#undef IS

	/* Bring out of sleep mode */
	if (!ar5416SetPowerMode(ah, HAL_PM_AWAKE, AH_TRUE))
		FAIL(HAL_EIO);

	/*
	 * Map public channel to private.
	 */
	ichan = ath_hal_checkchannel(ah, chan);
	if (ichan == AH_NULL) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; no mapping\n",
			__func__, chan->channel, chan->channelFlags);
		FAIL(HAL_EINVAL);
	} else {
		HALDEBUG(ah, "%s: Ch=%u Max=%d Min=%d\n",__func__,
			ichan->channel,ichan->maxTxPower,ichan->minTxPower);
	}
	switch (opmode) {
	case HAL_M_STA:
	case HAL_M_IBSS:
	case HAL_M_HOSTAP:
	case HAL_M_MONITOR:
		break;
	default:
		HALDEBUG(ah, "%s: invalid operating mode %u\n",
			__func__, opmode);
		FAIL(HAL_EINVAL);
		break;
	}
	HALASSERT(ahp->ah_eeversion >= AR_EEPROM_VER3);

	/* Preserve certain DMA hardware registers on a channel change */
	if (resetType != HAL_RESET_FULL) {
		/*
		 * AR5212 WAR
		 *
		 * On Venice, the TSF is almost preserved across a reset;
		 * it requires the WAR of doubling writes to the RESET_TSF
		 * bit in the AR_BEACON register; it also has the quirk
		 * of the TSF going back in time on the station (station
		 * latches onto the last beacon's tsf during a reset 50%
		 * of the times); the latter is not a problem for adhoc
		 * stations since as long as the TSF is behind, it will
		 * get resynchronized on receiving the next beacon; the
		 * TSF going backwards in time could be a problem for the
		 * sleep operation (supported on infrastructure stations
		 * only) - the best and most general fix for this situation
		 * is to resynchronize the various sleep/beacon timers on
		 * the receipt of the next beacon i.e. when the TSF itself
		 * gets resynchronized to the AP's TSF - power save is
		 * needed to be temporarily disabled until that time
		 *
		 * Need to save the sequence number to restore it after
		 * the reset!
		 */
		saveFrameSeqCount = OS_REG_READ(ah, AR_D_SEQNUM);

		/* this timer reg will be clobbered on some hw */
		timerMode = OS_REG_READ(ah, AR_TIMER_MODE);
	} else
		saveFrameSeqCount = 0;		/* NB: silence compiler */

#ifdef AH_SUPPORT_PPMUPDATE
	/* Preserve force ppm state */
	saveForceVal = OS_REG_READ(ah, AR_PHY_TIMING2) &
		(AR_PHY_TIMING2_USE_FORCE | AR_PHY_TIMING2_FORCE_VAL);
#endif /* AH_SUPPORT_PPMUPDATE */

	/*
	 * Preserve the bmiss rssi threshold and count threshold
	 * across resets
	 */
	if (!ahp->ah_rssiThr)
		ahp->ah_rssiThr = INIT_RSSI_THR;
	else
		ahp->ah_rssiThr = OS_REG_READ(ah, AR_RSSI_THR);

	/*
	 * Preserve the antenna on a channel change
	 */
	saveDefAntenna = OS_REG_READ(ah, AR_DEF_ANTENNA);
	if (saveDefAntenna == 0)		/* XXX magic constants */
		saveDefAntenna = 1;

	/* Save hardware flag before chip reset clears the register */
	macStaId1 = OS_REG_READ(ah, AR_STA_ID1) & 
		(AR_STA_ID1_BASE_RATE_11B | AR_STA_ID1_USE_DEFANT);

	/* Save led state from pci config register */
	saveLedState = OS_REG_READ(ah, AR_MAC_LED) &
		(AR_MAC_LED_ASSOC | AR_MAC_LED_MODE |
		 AR_MAC_LED_BLINK_THRESH_SEL | AR_MAC_LED_BLINK_SLOW);
	softLedCfg = OS_REG_READ(ah, AR_GPIO_INTR_OUT);	

	/*
	 * Adjust gain parameters before reset if
	 * there's an outstanding gain updated.
	 */
	(void) ar5416GetRfgain(ah);

	if (!ar5416ChipReset(ah, chan)) {
		HALDEBUG(ah, "%s: chip reset failed\n", __func__);
		FAIL(HAL_EIO);
	}

	/* Restore bmiss rssi & count thresholds */
	OS_REG_WRITE(ah, AR_RSSI_THR, ahp->ah_rssiThr);

	/* Setup the indices for the next set of register array writes */
	/* XXX Ignore 11n dynamic mode on the AR5416 for the moment */
	switch (chan->channelFlags & CHANNEL_ALL) {
	case CHANNEL_A:
    	case CHANNEL_A_HT20:
                modesIndex = 1;
                freqIndex  = 1;
		break;

    	case CHANNEL_A_HT40PLUS:
    	case CHANNEL_A_HT40MINUS:
                modesIndex = 2;
                freqIndex  = 1;
	    	break;
	case CHANNEL_XR_A:
		modesIndex = 1;
		freqIndex  = 1;
		break;
	case CHANNEL_T:
	case CHANNEL_XR_T:
		modesIndex = 2;
		freqIndex  = 1;
		break;
	case CHANNEL_B:
		/* treat channel B as channel G , no  B mode suport in owl */
		modesIndex = 4;
		freqIndex  = 2;
		break;
	case CHANNEL_PUREG:
	case CHANNEL_G_HT20:
		modesIndex = 4;
		freqIndex  = 2;
		break;
    	case CHANNEL_G_HT40PLUS:
    	case CHANNEL_G_HT40MINUS:
		modesIndex = 3;
		freqIndex  = 2;
		break;
	case CHANNEL_XR_G:
		modesIndex = 4;
		freqIndex  = 2;
		break;
	case CHANNEL_108G:
		modesIndex = 5;
		freqIndex  = 2;
		break;
	default:
		HALDEBUG(ah, "%s: invalid channel flags 0x%x\n",
			__func__, chan->channelFlags);
		FAIL(HAL_EINVAL);
	}

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	/* Set correct Baseband to analog shift setting to access analog chips. */
	OS_REG_WRITE(ah, AR_PHY(0), 0x00000007);

	/*
	 * Write addac shifts
	 */
	OS_REG_WRITE(ah, AR_PHY_ADC_SERIAL_CTL, AR_PHY_SEL_EXTERNAL_RADIO);
#ifdef AH_SUPPORT_AR9000
	if (AR_SREV_SOWL_10_OR_LATER(ah))
		ar5416EepromSetSowlAddac(ah, ichan, &regWrites);
	else
#endif 
	 {
		u_int32_t addacBuf[N(ar5416Addac)][2];
		ath_hal_memcpy(addacBuf, ar5416Addac, sizeof(ar5416Addac));
		if (!IS_5416V2_2(ah)) {
			/* override CLKDRV value */
			addacBuf[31][1] = 0;
		}
		REG_WRITE_ARRAY(addacBuf, 1, regWrites);
	}

	OS_REG_WRITE(ah, AR_PHY_ADC_SERIAL_CTL, AR_PHY_SEL_INTERNAL_ADDAC);

#ifdef AH_SUPPORT_AR9200
	if (AR_SREV_MERLIN_20_OR_LATER(ah)) {
		REG_WRITE_ARRAY_9280(ar9280Modes_merlin2, modesIndex, regWrites);
		REG_WRITE_ARRAY_9280(ar9280Common_merlin2, 1, regWrites);
	} else if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
		REG_WRITE_ARRAY(ar9280Modes_merlin, modesIndex, regWrites);
		REG_WRITE_ARRAY(ar9280Common_merlin, 1, regWrites);
	} else
#endif 
#ifdef AH_SUPPORT_AR9000
	if (AR_SREV_SOWL_10_OR_LATER(ah)) {
		REG_WRITE_ARRAY(ar5416Modes_sowl, modesIndex, regWrites);
		REG_WRITE_ARRAY(ar5416Common_sowl, 1, regWrites);
	} else
#endif
	{
		REG_WRITE_ARRAY(ar5416Modes, modesIndex, regWrites);
		REG_WRITE_ARRAY(ar5416Common, 1, regWrites);
	}
	/* Write Common Array Parameters */

	/* Setup 11n MAC/Phy mode registers */
	ar5416Set11nRegs(ah,chan);

    /*
     * Moved ar5416InitChainMasks() here to ensure the swap bit is set before
     * the pdadc table is written.  Swap must occur before any radio dependent
     * replicated register access.  The pdadc curve addressing in particular
     * depends on the consistent setting of the swap bit.
     */
    ar5416InitChainMasks(ah);

	ahp->ah_rfHal.writeRegs(ah, modesIndex, freqIndex, regWrites);

    /* Merlin 2.0/2.1 - For 5MHz spaced 11A channels, apply Fast Clock modal values */
#ifdef AH_SUPPORT_AR9200
    if (AR_SREV_MERLIN_20(ah) && IS_CHAN_A_5MHZ_SPACED(chan)) {
        REG_WRITE_ARRAY(ar9280Modes_fast_clock_merlin2, modesIndex, regWrites);  
    }
#endif

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	HALDEBUG(ah, ">>>2 %s: AR_PHY_DAG_CTRLCCK=0x%x\n",
		__func__, OS_REG_READ(ah,AR_PHY_DAG_CTRLCCK));
	HALDEBUG(ah, ">>>2 %s: AR_PHY_ADC_CTL=0x%x\n",
		__func__, OS_REG_READ(ah,AR_PHY_ADC_CTL));	

	/* Set the mute mask to the correct default */
	if (AH_PRIVATE(ah)->ah_phyRev >= AR_PHY_CHIP_ID_REV_2)
		OS_REG_WRITE(ah, AR_SEQ_MASK, 0x0000000F);

	if (AH_PRIVATE(ah)->ah_phyRev >= AR_PHY_CHIP_ID_REV_3) {
		/* Clear reg to alllow RX_CLEAR line debug */
		OS_REG_WRITE(ah, AR_PHY_BLUETOOTH,  0);
	}
	if (AH_PRIVATE(ah)->ah_phyRev >= AR_PHY_CHIP_ID_REV_4) {
#ifdef notyet
		/* Enable burst prefetch for the data queues */
		OS_REG_RMW_FIELD(ah, AR_D_FPCTL, ... );
		/* Enable double-buffering */
		OS_REG_CLR_BIT(ah, AR_TXCFG, AR_TXCFG_DBL_BUF_DIS);
#endif
	}

	/* Set ADC/DAC select values */
	OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL, 0x0e);

	/* Setup the transmit power values. */
	if (!ar5416SetTransmitPower(ah, ichan, rfXpdGain)) {
		HALDEBUG(ah, "%s: error init'ing transmit power\n", __func__);
		FAIL(HAL_EIO);
	}

	/* Write the analog registers */
	if (!ahp->ah_rfHal.setRfRegs(ah, ichan, freqIndex, rfXpdGain)) {
		HALDEBUG(ah, "%s: ar5212SetRfRegs failed\n", __func__);
		FAIL(HAL_EIO);
	}

	if (AR_SREV_OWL_20_OR_LATER(ah) && !AR_SREV_MERLIN_10_OR_LATER(ah)) {
		/* 
		 * Disable BB clock gating
		 * Necessary to avoid hangs in Owl 2.0
		 */
		OS_REG_WRITE(ah, 0x9800+(651<<2), 0x11);
	}

	/* Write delta slope for OFDM enabled modes (A, G, Turbo) */
	if (IS_CHAN_OFDM(chan)|| IS_CHAN_HT(chan))
		ar5416SetDeltaSlope(ah, ichan);

#ifdef AH_SUPPORT_AR9200
	if (AR_SREV_MERLIN_10_OR_LATER(ah))
		ar9280SpurMitigate(ah, ichan);
	else
#endif
	ar5416SpurMitigate(ah, ichan);



	/* Setup board specific options for EEPROM version 3 */
	if (!ar5416SetBoardValues(ah, ichan)) {
		HALDEBUG(ah, "%s: error setting board options\n", __func__);
		FAIL(HAL_EIO);
	}

	/* Restore certain DMA hardware registers on a channel change */
	if (resetType != HAL_RESET_FULL) {
		OS_REG_WRITE(ah, AR_TIMER_MODE, timerMode);
		OS_REG_WRITE(ah, AR_D_SEQNUM, saveFrameSeqCount);
	}

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	OS_REG_WRITE(ah, AR_STA_ID0, LE_READ_4(ahp->ah_macaddr));
	OS_REG_WRITE(ah, AR_STA_ID1, LE_READ_2(ahp->ah_macaddr + 4)
		| macStaId1
		| AR_STA_ID1_RTS_USE_DEF
		| ahp->ah_staId1Defaults
	);
	ar5212SetOperatingMode(ah, opmode);

	/* Set Venice BSSID mask according to current state */
	OS_REG_WRITE(ah, AR_BSSMSKL, LE_READ_4(ahp->ah_bssidmask));
	OS_REG_WRITE(ah, AR_BSSMSKU, LE_READ_2(ahp->ah_bssidmask + 4));

	/* Restore previous led state */
	OS_REG_WRITE(ah, AR_MAC_LED, OS_REG_READ(ah, AR_MAC_LED) | saveLedState);
	/* Restore soft Led state to GPIO */
	OS_REG_WRITE(ah, AR_GPIO_INTR_OUT, softLedCfg);

	/* Restore previous antenna */
	OS_REG_WRITE(ah, AR_DEF_ANTENNA, saveDefAntenna);

#ifdef AH_SUPPORT_PPMUPDATE
	/* Restore force ppm state */
	tmpReg = OS_REG_READ(ah, AR_PHY_TIMING2) &~
		(AR_PHY_TIMING2_USE_FORCE | AR_PHY_TIMING2_FORCE_VAL);
	OS_REG_WRITE(ah, AR_PHY_TIMING2, tmpReg | saveForceVal);
#endif /* AH_SUPPORT_PPMUPDATE */

	/* then our BSSID */
	OS_REG_WRITE(ah, AR_BSS_ID0, LE_READ_4(ahp->ah_bssid));
	OS_REG_WRITE(ah, AR_BSS_ID1, LE_READ_2(ahp->ah_bssid + 4));

	OS_REG_WRITE(ah, AR_ISR, ~0);		/* cleared on write */

	if (!ar5212SetChannel(ah, ichan))
		FAIL(HAL_EIO);

	OS_MARK(ah, AH_MARK_RESET_LINE, __LINE__);

	ar5416SetCoverageClass(ah, AH_PRIVATE(ah)->ah_coverageClass, 1);

	/*
	 * AR5416/Owl Setup
	 *  Disable seq number generation in hw
	 *  Setup DMA registers 
	 *  Setup Baseband Chain Masks  
	 */
	/*
	 * disable seq number generation in hw
	 */
	 OS_REG_WRITE(ah, AR_STA_ID1,
	     OS_REG_READ(ah, AR_STA_ID1) | AR_STA_ID1_PRESERVE_SEQNUM);
	 
	/* DMA Setup */

	/*
	 * set AHB_MODE not to do cacheline prefetches
	 */

	OS_REG_WRITE(ah, AR_AHB_MODE, 
		(OS_REG_READ(ah, AR_AHB_MODE)) | AR_AHB_PREFETCH_RD_EN);

	/*
	 * Setup receive FIFO threshold to hold off TX activities
	 */
	OS_REG_WRITE(ah, AR_RXFIFO_CFG, 0x200);
	
	/*
	 * reduce the number of usable entries in PCU TXBUF to avoid
	 * wrap around bugs. (bug 20428)
	 */
	OS_REG_WRITE(ah, AR_PCU_TXBUF_CTRL, AR_PCU_TXBUF_CTRL_USABLE_SIZE);
	
	/*
	 * program OBS bus to see MAC interrupts
	 */
	OS_REG_WRITE(ah, AR_OBS, 8);

	/*
	 * GTT debug mode setting
	 */
	// OS_REG_WRITE(ah, 0x64, 0x00320000);
	// OS_REG_WRITE(ah, 0x68, 7);
	// OS_REG_WRITE(ah, 0x4080, 0xC);

#ifdef AR5416_INT_MITIGATION
	OS_REG_WRITE(ah, AR_MIRT, 0);
	OS_REG_RMW_FIELD(ah, AR_RIMT, AR_RIMT_LAST, 500);
	OS_REG_RMW_FIELD(ah, AR_RIMT, AR_RIMT_FIRST, 2000);
#endif	    

	/*
	 * Setup fast diversity.
	 * Fast diversity can be enabled or disabled via regadd.txt.
	 * Default is enabled.
	 * For reference,
	 *    Disable: reg        val
	 *             0x00009860 0x00009d18 (if 11a / 11g, else no change)
	 *             0x00009970 0x192bb514
	 *             0x0000a208 0xd03e4648
	 *
	 *    Enable:  0x00009860 0x00009d10 (if 11a / 11g, else no change)
	 *             0x00009970 0x192fb514
	 *             0x0000a208 0xd03e6788
	 */
	
	/* XXX Setup pre PHY ENABLE EAR additions */
	/*
	 * Wait for the frequency synth to settle (synth goes on
	 * via AR_PHY_ACTIVE_EN).  Read the phy active delay register.
	 * Value is in 100ns increments.
	  */
	synthDelay = OS_REG_READ(ah, AR_PHY_RX_DELAY) & AR_PHY_RX_DELAY_DELAY;
	if (IS_CHAN_CCK(chan)) {
		synthDelay = (4 * synthDelay) / 22;
	} else {
		synthDelay /= 10;
	}
	
	/* Turn on PLL on 5416 */
	HALDEBUG(ah,"%s %s channel\n",__func__,IS_CHAN_5GHZ(chan) ? "5GHz" : "2GHz");
	ar5416InitPLL(ah, chan);

	/* Activate the PHY (includes baseband activate and synthesizer on) */
	OS_REG_WRITE(ah, AR_PHY_ACTIVE, AR_PHY_ACTIVE_EN);
	
	/* 
	 * There is an issue if the AP starts the calibration before
	 * the base band timeout completes.  This could result in the
	 * rx_clear false triggering.  As a workaround we add delay an
	 * extra BASE_ACTIVATE_DELAY usecs to ensure this condition
	 * does not happen.
	 */
	if (IS_CHAN_HALF_RATE(AH_PRIVATE(ah)->ah_curchan)) {
		OS_DELAY((synthDelay << 1) + BASE_ACTIVATE_DELAY);
	} else if (IS_CHAN_QUARTER_RATE(AH_PRIVATE(ah)->ah_curchan)) {
		OS_DELAY((synthDelay << 2) + BASE_ACTIVATE_DELAY);
	} else {
		OS_DELAY(synthDelay + BASE_ACTIVATE_DELAY);
	}

	/*
	 * WAR for bug 9031
	 * The udelay method is not reliable with notebooks.
	 * Need to check to see if the baseband is ready
	 */
	testReg = OS_REG_READ(ah, AR_PHY_TESTCTRL);
	/* Selects the Tx hold */
	OS_REG_WRITE(ah, AR_PHY_TESTCTRL, AR_PHY_TESTCTRL_TXHOLD);
	/* test if baseband not ready */	
	for (i = 0; i < 20 && (OS_REG_READ(ah, 0x9c24) & 0x10); i++)
		OS_DELAY(200);
	OS_REG_WRITE(ah, AR_PHY_TESTCTRL, testReg);

	/* Calibrate the AGC and start a NF calculation */
	OS_REG_WRITE(ah, AR_PHY_AGC_CONTROL,
		  OS_REG_READ(ah, AR_PHY_AGC_CONTROL)
		| AR_PHY_AGC_CONTROL_CAL
		| AR_PHY_AGC_CONTROL_NF);


	/* XXX IQ-Cal on the 5416 is not working yet */
	if (!IS_CHAN_B(chan) && ahp->ah_bIQCalibration != IQ_CAL_DONE) {
		/* Start IQ calibration w/ 2^(INIT_IQCAL_LOG_COUNT_MAX+1) samples */
		OS_REG_RMW_FIELD(ah, AR_PHY_TIMING_CTRL4, 
				AR_PHY_TIMING_CTRL4_IQCAL_LOG_COUNT_MAX,
				INIT_IQCAL_LOG_COUNT_MAX);
		OS_REG_WRITE(ah, AR_PHY_CALMODE, 0);
		OS_REG_SET_BIT(ah, AR_PHY_TIMING_CTRL4,
				AR_PHY_TIMING_CTRL4_DO_IQCAL);
			ahp->ah_bIQCalibration = IQ_CAL_RUNNING;

		/* 
		 * XXX: WAR for beacon hang. Only pass control on to the driver again
		 * when the IQ cal is finished
		 */
		ath_hal_wait(ah, AR_PHY_TIMING_CTRL4, AR_PHY_TIMING_CTRL4_DO_IQCAL, 0);
	} else

		ahp->ah_bIQCalibration = IQ_CAL_INACTIVE;		

	/* Setup compression registers */
	ar5212SetCompRegs(ah);		/* XXX not needed? */

	/* Set 1:1 QCU to DCU mapping for all queues */
	for (i = 0; i < AR_NUM_DCU; i++)
		OS_REG_WRITE(ah, AR_DQCUMASK(i), 1 << i);

	ahp->ah_intrTxqs = 0;
	for (i = 0; i < AH_PRIVATE(ah)->ah_caps.halTotalQueues; i++)
		ar5212ResetTxQueue(ah, i);

	/*
	 * Setup interrupt handling.  Note that ar5212ResetTxQueue
	 * manipulates the secondary IMR's as queues are enabled
	 * and disabled.  This is done with RMW ops to insure the
	 * settings we make here are preserved.
	 */
        ahp->ah_maskReg = AR_IMR_TXERR | AR_IMR_TXURN
			| AR_IMR_RXERR | AR_IMR_RXORN
                        | AR_IMR_BCNMISC;

#ifdef AR5416_INT_MITIGATION
       	ahp->ah_maskReg |= AR_IMR_TXINTM | AR_IMR_RXINTM
			|  AR_IMR_TXMINTR | AR_IMR_RXMINTR;
#else
        ahp->ah_maskReg |= AR_IMR_TXOK | AR_IMR_RXOK;
#endif	
	if (opmode == HAL_M_HOSTAP)
		ahp->ah_maskReg |= AR_IMR_MIB;
	OS_REG_WRITE(ah, AR_IMR, ahp->ah_maskReg);
	/* Enable bus errors that are OR'd to set the HIUERR bit */
	
	OS_REG_WRITE(ah, AR_IMR_S2,
		OS_REG_READ(ah, AR_IMR_S2) | AR_IMR_S2_GTT | AR_IMR_S2_CST);

	if (!ath_hal_wait(ah, AR_PHY_AGC_CONTROL, AR_PHY_AGC_CONTROL_CAL, 0)) {
		HALDEBUG(ah, "%s: offset calibration failed to complete in 1ms;"
			" noisy environment?\n", __func__);
	}
	/*
         * bring out rx clear on the AP71.
         */
#if 0
	/* GPIOs on FPGA platform used for other stuff. Writing to these locks up the FPGA */
	OS_REG_WRITE(ah, 0x404c, 0xf)
	OS_REG_WRITE(ah, 0x404c, 0xc00);
	OS_REG_WRITE(ah, 0x4060, ((4 << 5) | 3));
	OS_REG_WRITE(ah, 0x4060, ((4 << 10) |(3 << 5) ));
	OS_REG_WRITE(ah, 0x4060, (4 << 25)); /* XXX  Rx clear on GPIO 5 MB72 only */
	OS_REG_WRITE(ah, 0x4060, (4 << 10)); /* XXX  Rx clear on GPIO 2 XB72 only */
#endif
	/* Reset AR_PHY_SLEEP_SCAL (DAC Clock select 0x9878) to zero */
	OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL, 0);
	HALDEBUG(ah,"%d >>>%s: AR_PHY_SLEEP_SCAL=0x%x\n",
		__LINE__, __func__,OS_REG_READ(ah,AR_PHY_SLEEP_SCAL));

#ifdef AH_ENABLE_LOGPHYERR
        /* XXX enable the phy error generation in to desriptors */
	if (ath_hal_enablephyerr) {
                int val;
                /* enable all phy errors */
                OS_REG_WRITE(ah,AR_PHY_ERR,0xffffffff);
                val=OS_REG_READ(ah,0x34);
                OS_REG_WRITE(ah,0x34, (val | 0x10));
                /* turn on permiscuous mode */
                val=OS_REG_READ(ah,0x803c);
                OS_REG_WRITE(ah,0x803c, (val | 0x20));

        } 
#endif
	/* XXX Setup post reset EAR additions */

#ifdef AH_SUPPORT_XR
	/* it should be changed to IS_CHAN_XR once the reg domain sets the XR flags on channels */
	if ((opmode != HAL_M_STA) && ahp->ah_xrEnable && !ar5212SetXrMode(ah, opmode,chan)) {
		HALDEBUG(ah, "%s: unable to setup XR mode\n", __func__);
		FAIL(HAL_EIO);
	}
#endif /* AH_SUPPORT_XR */

	/* QoS support */
	OS_REG_WRITE(ah, AR_QOS_CONTROL, 0x100aa);	/* XXX magic */
	OS_REG_WRITE(ah, AR_QOS_SELECT, 0x3210);	/* XXX magic */

	/* Turn on NOACK Support for QoS packets */
	OS_REG_WRITE(ah, AR_NOACK,
		SM(2, AR_NOACK_2BIT_VALUE) |
		SM(5, AR_NOACK_BIT_OFFSET) |
		SM(0, AR_NOACK_BYTE_OFFSET));
		
    	/*
    	 * initialize TXOP for all TIDs
    	 */
	OS_REG_WRITE(ah, AR_TXOP_X, AR_TXOP_X_VAL);
	OS_REG_WRITE(ah, AR_TXOP_0_3, 0xFFFFFFFF);
	OS_REG_WRITE(ah, AR_TXOP_4_7, 0xFFFFFFFF);
	OS_REG_WRITE(ah, AR_TXOP_8_11, 0xFFFFFFFF);
	OS_REG_WRITE(ah, AR_TXOP_12_15, 0xFFFFFFFF);

	/*
	 * 5416 baseband will check the per rate power table
	 * and select the lower of the two
	 */
	ackTpcPow = 63;
	ctsTpcPow = 63;
	chirpTpcPow = 63;
	powerVal = SM(ackTpcPow, AR_TPC_ACK) |
		SM(ctsTpcPow, AR_TPC_CTS) |
		SM(chirpTpcPow, AR_TPC_CHIRP);
	OS_REG_WRITE(ah, AR_TPC, powerVal);

	/* set the proper DMA size */
	OS_REG_RMW_FIELD(ah, AR_TXCFG, AR_DMASIZE, ahp->ah_txDmaBurst);
	OS_REG_RMW_FIELD(ah, AR_RXCFG, AR_DMASIZE, ahp->ah_rxDmaBurst);

	/* Restore user-specified settings */
	if (ahp->ah_miscMode != 0)
		OS_REG_WRITE(ah, AR_MISC_MODE, ahp->ah_miscMode);
	if (ahp->ah_slottime != (u_int) -1)
		ar5212SetSlotTime(ah, ahp->ah_slottime);
	if (ahp->ah_acktimeout != (u_int) -1)
		ar5212SetAckTimeout(ah, ahp->ah_acktimeout);
	if (ahp->ah_ctstimeout != (u_int) -1)
		ar5212SetCTSTimeout(ah, ahp->ah_ctstimeout);
	if (AH_PRIVATE(ah)->ah_diagreg != 0)
		OS_REG_WRITE(ah, AR_DIAG_SW, AH_PRIVATE(ah)->ah_diagreg);
#if 0 /* XXX Todo */
	if (ahp->ah_globaltxtimeout != (u_int) -1)
        	ar5416SetGlobalTxTimeout(ah, ahp->ah_globaltxtimeout);
#endif
	AH_PRIVATE(ah)->ah_opmode = opmode;	/* record operating mode */

	if (resetType != HAL_RESET_FULL) {
		if (!(ichan->privFlags & CHANNEL_DFS)) {
			ichan->privFlags &= ~CHANNEL_INTERFERENCE;
			ichan->channelFlags &= ~CHANNEL_CW_INT;
			}
		chan->channelFlags = ichan->channelFlags;
		chan->privFlags = ichan->privFlags;
		chan->maxRegTxPower = ichan->maxRegTxPower;
		chan->maxTxPower = ichan->maxTxPower;
		chan->minTxPower = ichan->minTxPower;
		AH_PRIVATE(ah)->ah_curchan->ah_channel_time=0;
		AH_PRIVATE(ah)->ah_curchan->ah_tsf_last = ar5212GetTsf64(ah);
	}

	HALDEBUG(ah, "%s: done\n", __func__);

	OS_MARK(ah, AH_MARK_RESET_DONE, 0);

	return AH_TRUE;
bad:
	OS_MARK(ah, AH_MARK_RESET_DONE, ecode);
	if (status)
		*status = ecode;
	return AH_FALSE;
#undef FAIL
#undef N
}

#if 0
/*
 * This channel change evaluates whether the selected hardware can
 * perform a synthesizer-only channel change (no reset).  If the
 * TX is not stopped, or the RFBus cannot be granted in the given
 * time, the function returns false as a reset is necessary
 */
HAL_BOOL
ar5416ChannelChange(struct ath_hal *ah, HAL_CHANNEL *chan)
{
	u_int32_t       ulCount;
	u_int32_t   data, synthDelay, qnum;
	u_int16_t   rfXpdGain[4];
	HAL_BOOL    txStopped = AH_TRUE;
	struct ath_hal_5212 *ahp = AH5212(ah);
	HAL_CHANNEL_INTERNAL *ichan;

	/*
	 * Map public channel to private.
	 */
	ichan = ath_hal_checkchannel(ah, chan);

	/* TX must be stopped or RF Bus grant will not work */
	for (qnum = 0; qnum < AH_PRIVATE(ah)->ah_caps.halTotalQueues; qnum++) {
		if (ar5212NumTxPending(ah, qnum)) {
			txStopped = AH_FALSE;
			break;
		}
	}
	if (!txStopped)
		return AH_FALSE;

	/* Kill last Baseband Rx Frame */
	OS_REG_WRITE(ah, AR_PHY_RFBUS_REQ, AR_PHY_RFBUS_REQ_REQUEST); /* Request analog bus grant */
	for (ulCount = 0; ulCount < 100; ulCount++) {
		if (OS_REG_READ(ah, AR_PHY_RFBUS_GNT))
			break;
		OS_DELAY(5);
	}
	if (ulCount >= 100)
		return AH_FALSE;

	ar5416Set11nRegs(ah, chan);	/* NB: setup 5416-specific regs */

	/* Change the synth */
	if (!ar5212SetChannel(ah, ichan))
		return AH_FALSE;

	/*
	 * Wait for the frequency synth to settle (synth goes on via PHY_ACTIVE_EN).
	 * Read the phy active delay register. Value is in 100ns increments.
	 */
	data = OS_REG_READ(ah, AR_PHY_RX_DELAY) & AR_PHY_RX_DELAY_DELAY;
	if (IS_CHAN_CCK(ichan)) {
		synthDelay = (4 * data) / 22;
	} else {
		synthDelay = data / 10;
	}

	OS_DELAY(synthDelay + BASE_ACTIVATE_DELAY);

	/* Setup the transmit power values. */
	if (!ar5416SetTransmitPower(ah, ichan, rfXpdGain)) {
		HALDEBUG(ah, "%s: error init'ing transmit power\n", __func__);
		return AH_FALSE;
	}

	/* Release the RFBus Grant */
	OS_REG_WRITE(ah, AR_PHY_RFBUS_REQ, 0);

	/* Write delta slope for OFDM enabled modes (A, G, Turbo) */
	if (IS_CHAN_OFDM(ichan)|| IS_CHAN_HT(chan)) {
		if (ahp->ah_eeprom.ee_version >= AR_EEPROM_VER5_3 &&
		    !IS_CHAN_B(chan))
			ar5212SetSpurMitigation(ah, ichan);
		ar5416SetDeltaSlope(ah, ichan);
	}

	/* Start Noise Floor Cal */
	OS_REG_SET_BIT(ah, AR_PHY_AGC_CONTROL, AR_PHY_AGC_CONTROL_NF);

	/* Copy over internal channel flags to public hal channel */

	if (!(ichan->privFlags & CHANNEL_DFS)) 
		ichan->privFlags &= ~CHANNEL_INTERFERENCE;
	chan->channelFlags = ichan->channelFlags;
	chan->privFlags = ichan->privFlags;
	chan->maxRegTxPower = ichan->maxRegTxPower;
	chan->maxTxPower = ichan->maxTxPower;
	chan->minTxPower = ichan->minTxPower;
	AH_PRIVATE(ah)->ah_curchan->ah_channel_time=0;
	AH_PRIVATE(ah)->ah_curchan->ah_tsf_last = ar5212GetTsf64(ah);
	return AH_TRUE;
}
#endif

/*
 * Places the hardware into reset and then pulls it out of reset
 */
HAL_BOOL
ar5416ChipReset(struct ath_hal *ah, HAL_CHANNEL *chan)
{
	u_int32_t rfMode = 0;

	OS_MARK(ah, AH_MARK_CHIPRESET, chan ? chan->channel : 0);
	/*
	 * Warm reset is optimistic.
	 */
	if (!ar5416SetResetReg(ah, HAL_RESET_WARM))
	       return AH_FALSE;

	/* Bring out of sleep mode (AGAIN) */
	if (!ar5416SetPowerMode(ah, HAL_PM_AWAKE, AH_TRUE))
	       return AH_FALSE;

	/*
	 * Perform warm reset before the mode/PLL/turbo registers
	 * are changed in order to deactivate the radio.  Mode changes
	 * with an active radio can result in corrupted shifts to the
	 * radio device.
	 */
	if (chan != AH_NULL) { 
		/* treat channel B as channel G , no  B mode suport in owl */


		rfMode |= (IS_CHAN_G(chan) || IS_CHAN_B(chan)) ?
			AR_PHY_MODE_DYNAMIC : AR_PHY_MODE_OFDM;
		if (!AR_SREV_MERLIN_10_OR_LATER(ah)) {
    			rfMode |= (IS_CHAN_5GHZ(chan)) ? AR_PHY_MODE_RF5GHZ : AR_PHY_MODE_RF2GHZ;
		}
		/* Merlin 2.0/2.1 - Set appropriate Phy mode bits for 5MHz spaced channels */
#ifdef AH_SUPPORT_AR9200
		if (AR_SREV_MERLIN_20(ah) && IS_CHAN_A_5MHZ_SPACED(chan)) {
		    rfMode |= (AR_PHY_MODE_DYNAMIC | AR_PHY_MODE_DYN_CCK_DISABLE);
		}
#endif
		OS_REG_WRITE(ah, AR_PHY_MODE, rfMode);
	}
	return AH_TRUE;	
}

/*
 * Recalibrate the lower PHY chips to account for temperature/environment
 * changes.
 */
HAL_BOOL
ar5416PerCalibration(struct ath_hal *ah,  HAL_CHANNEL *chan, HAL_BOOL longCal, HAL_BOOL *isIQdone)
{
#define IQ_CAL_TRIES    10
	struct ath_hal_5212 *ahp = AH5212(ah);
	HAL_CHANNEL_INTERNAL *ichan;
	int32_t qCoff, qCoffDenom;
	int32_t iqCorrMeas, iCoff, iCoffDenom;
	u_int32_t powerMeasQ, powerMeasI;

	OS_MARK(ah, AH_MARK_PERCAL, chan->channel);
	*isIQdone = AH_FALSE;
	ichan = ath_hal_checkchannel(ah, chan);
	if (ichan == AH_NULL) {
		HALDEBUG(ah, "%s: invalid channel %u/0x%x; no mapping\n",
			__func__, chan->channel, chan->channelFlags);
		return AH_FALSE;
	}

	/* XXX EAR */
	if ((ahp->ah_bIQCalibration == IQ_CAL_DONE) ||
	    (ahp->ah_bIQCalibration == IQ_CAL_INACTIVE))
		*isIQdone = AH_TRUE;

	/* IQ calibration in progress. Check to see if it has finished. */
	if (ahp->ah_bIQCalibration == IQ_CAL_RUNNING &&
	    !(OS_REG_READ(ah, AR_PHY_TIMING_CTRL4) & AR_PHY_TIMING_CTRL4_DO_IQCAL)) {
		int i;

		/* IQ Calibration has finished. */
		ahp->ah_bIQCalibration = IQ_CAL_INACTIVE;
		*isIQdone = AH_TRUE;

		/* workaround for misgated IQ Cal results */
		i = 0;
		do {
			/* Read calibration results. */
			powerMeasI = OS_REG_READ(ah, AR_PHY_IQCAL_RES_PWR_MEAS_I);
			powerMeasQ = OS_REG_READ(ah, AR_PHY_IQCAL_RES_PWR_MEAS_Q);
			iqCorrMeas = OS_REG_READ(ah, AR_PHY_IQCAL_RES_IQ_CORR_MEAS);
			if (powerMeasI && powerMeasQ)
				break;
			/* Do we really need this??? */
			OS_REG_WRITE (ah,  AR_PHY_TIMING_CTRL4,
				      OS_REG_READ(ah,  AR_PHY_TIMING_CTRL4) |
				      AR_PHY_TIMING_CTRL4_DO_IQCAL);
		} while (++i < IQ_CAL_TRIES);

		/*
		 * Prescale these values to remove 64-bit operation
		 * requirement at the loss of a little precision.
		 */
		iCoffDenom = (powerMeasI / 2 + powerMeasQ / 2) / 128;
		qCoffDenom = powerMeasQ / 128;

		/* Protect against divide-by-0 and loss of sign bits. */
		if (iCoffDenom != 0 && qCoffDenom >= 2) {
			iCoff = (int8_t)(-iqCorrMeas) / iCoffDenom;
			/* IQCORR_Q_I_COFF is a signed 6 bit number */
			if (iCoff < -32) {
				iCoff = -32;
			} else if (iCoff > 31) {
				iCoff = 31;
			}

			/* IQCORR_Q_Q_COFF is a signed 5 bit number */
			qCoff = (powerMeasI / qCoffDenom) - 128;
			if (qCoff < -16) {
				qCoff = -16;
			} else if (qCoff > 15) {
				qCoff = 15;
			}
#ifdef CALIBRATION_DEBUG
			HALDEBUG(ah, "****************** MISGATED IQ CAL! *******************\n");
			HALDEBUG(ah, "time       = %d, i = %d, \n",
				OS_GETUPTIME(ah), i);
			HALDEBUG(ah, "powerMeasI = 0x%08x\n", powerMeasI);
			HALDEBUG(ah, "powerMeasQ = 0x%08x\n", powerMeasQ);
			HALDEBUG(ah, "iqCorrMeas = 0x%08x\n", iqCorrMeas);
			HALDEBUG(ah, "iCoff      = %d\n", iCoff);
			HALDEBUG(ah, "qCoff      = %d\n", qCoff);
#endif
			/* Write values and enable correction */
			OS_REG_RMW_FIELD(ah, AR_PHY_TIMING_CTRL4,
				AR_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF, iCoff);
			OS_REG_RMW_FIELD(ah, AR_PHY_TIMING_CTRL4,
				AR_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF, qCoff);
			OS_REG_SET_BIT(ah, AR_PHY_TIMING_CTRL4, 
				AR_PHY_TIMING_CTRL4_IQCORR_ENABLE);

			ahp->ah_bIQCalibration = IQ_CAL_DONE;
			ichan->iqCalValid = AH_TRUE;
			ichan->iCoff = iCoff;
			ichan->qCoff = qCoff;
		}
	} else if (!IS_CHAN_B(chan) &&
		   ahp->ah_bIQCalibration == IQ_CAL_DONE &&
		   !ichan->iqCalValid) {
		/*
		 * Start IQ calibration if configured channel has changed.
		 * Use a magic number of 15 based on default value.
		 */
		OS_REG_RMW_FIELD(ah, AR_PHY_TIMING_CTRL4,
			AR_PHY_TIMING_CTRL4_IQCAL_LOG_COUNT_MAX,
			INIT_IQCAL_LOG_COUNT_MAX);
		OS_REG_SET_BIT(ah, AR_PHY_TIMING_CTRL4,
			AR_PHY_TIMING_CTRL4_DO_IQCAL);
		ahp->ah_bIQCalibration = IQ_CAL_RUNNING;
	}
	/* XXX EAR */

	/* Check noise floor results */
	if (longCal) {
		ar5416GetNf(ah, ichan);
		if ((ichan->channelFlags & CHANNEL_CW_INT) == 0) {
			/* run noise floor calibration */
			OS_REG_SET_BIT(ah, AR_PHY_AGC_CONTROL, AR_PHY_AGC_CONTROL_NF);
#if 0
			/* Perform calibration for 5Ghz channels */
			if (IS_CHAN_5GHZ(chan))
				ar5212RequestRfgain(ah);
#endif
		/* XXX EAR */
		}	
	}
	return AH_TRUE;
#undef IQ_CAL_TRIES
}

static void 
ar5416GetNoiseFloor(struct ath_hal *ah,
	u_int16_t nfarray[NUM_NOISEFLOOR_READINGS])
{
	struct ath_hal_5416 *ahp = AH5416(ah);
	int16_t nf;

	switch (ahp->ah_rx_chainmask) {
        case 0x7:
	if (!AR_SREV_MERLIN(ah)) {
		nf = MS(OS_REG_READ(ah, AR_PHY_CH2_CCA), AR_PHY_CH2_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ctl] [chain 2] is %d\n", nf);
		nfarray[4] = nf;

		nf = MS(OS_REG_READ(ah, AR_PHY_CH2_EXT_CCA), AR_PHY_CH2_EXT_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ext] [chain 2] is %d\n", nf);
		nfarray[5] = nf;
		/* fall thru... */
	}
        case 0x3:
        case 0x5:
	if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
		nf = MS(OS_REG_READ(ah, AR_PHY_CH1_CCA), AR9280_PHY_CH1_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ctl] [chain 1] is %d\n", nf);
		nfarray[2] = nf;


		nf = MS(OS_REG_READ(ah, AR_PHY_CH1_EXT_CCA), AR9280_PHY_CH1_EXT_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ext] [chain 1] is %d\n", nf);
		nfarray[3] = nf;
	}else{	
	
		nf = MS(OS_REG_READ(ah, AR_PHY_CH1_CCA), AR_PHY_CH1_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ctl] [chain 1] is %d\n", nf);
		nfarray[2] = nf;


		nf = MS(OS_REG_READ(ah, AR_PHY_CH1_EXT_CCA), AR_PHY_CH1_EXT_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ext] [chain 1] is %d\n", nf);
		nfarray[3] = nf;
	}
		/* fall thru... */
        case 0x1:
	if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
		nf = MS(OS_REG_READ(ah, AR_PHY_CCA), AR9280_PHY_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ctl] [chain 0] is %d\n", nf);
		nfarray[0] = nf;

		nf = MS(OS_REG_READ(ah, AR_PHY_EXT_CCA), AR9280_PHY_EXT_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ext] [chain 0] is %d\n", nf);
		nfarray[1] = nf;
	}else{	
		nf = MS(OS_REG_READ(ah, AR_PHY_CCA), AR_PHY_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ctl] [chain 0] is %d\n", nf);
		nfarray[0] = nf;

		nf = MS(OS_REG_READ(ah, AR_PHY_EXT_CCA), AR_PHY_EXT_MINCCA_PWR);
		if (nf & 0x100)
			nf = 0 - ((nf ^ 0x1ff) + 1);
		HALDEBUG(ah, "NF calibrated [ext] [chain 0] is %d\n", nf);
		nfarray[1] = nf;

	}
		break;
	}
}

static HAL_BOOL
ar5416GetEepromNoiseFloorThresh(struct ath_hal *ah,
	const HAL_CHANNEL_INTERNAL *chan, int16_t *nft)
{
	switch (chan->channelFlags & CHANNEL_ALL_NOTURBO) {
	case CHANNEL_A:
	case CHANNEL_A_HT20:
	case CHANNEL_A_HT40PLUS:
	case CHANNEL_A_HT40MINUS:
		*nft = (int16_t) ar5416EepromGet(ah, EEP_NFTHRESH_5);
		break;
	case CHANNEL_B:
	case CHANNEL_G:
	case CHANNEL_G_HT20:
	case CHANNEL_G_HT40PLUS:
	case CHANNEL_G_HT40MINUS:
		*nft = (int16_t) ar5416EepromGet(ah, EEP_NFTHRESH_2);
		break;
	default:
		HALDEBUG(ah, "%s: invalid channel flags 0x%x\n",
			__func__, chan->channelFlags);
		return AH_FALSE;
	}
	return AH_TRUE;
}

/*
 * Read the NF and check it against the noise floor threshhold
 */
static int16_t
ar5416GetNf(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan)
{
	int16_t nf, nfThresh;

	if (OS_REG_READ(ah, AR_PHY_AGC_CONTROL) & AR_PHY_AGC_CONTROL_NF) {
		HALDEBUG(ah, "%s: NF did not complete in calibration window\n",
			__func__);
		nf = 0;
	} else {
		/* Finished NF cal, check against threshold */
		u_int16_t nfarray[NUM_NOISEFLOOR_READINGS]= {0};
			
		/* TODO - enhance for multiple chains and ext ch */
		ar5416GetNoiseFloor(ah, nfarray);
		nf = nfarray[0];
		if (ar5416GetEepromNoiseFloorThresh(ah, chan, &nfThresh)) {
			if (nf > nfThresh) {
				HALDEBUG(ah, "%s: noise floor failed detected; "
					"detected %d, threshold %d\n", __func__,
					nf, nfThresh);
				/*
				 * NB: Don't discriminate 2.4 vs 5Ghz, if this
				 *     happens it indicates a problem regardless
				 *     of the band.
				 */
				chan->channelFlags |= CHANNEL_CW_INT;
				nf = 0;
			}
		} else {
			nf = 0;
		}
		chan->rawNoiseFloor = nf;
	}
	return nf;
}

/*
 * Delta slope coefficient computation.
 * Required for OFDM operation.
 */
static void
ar5416GetDeltaSlopeValues(struct ath_hal *ah, u_int32_t coef_scaled,
                          u_int32_t *coef_mantissa, u_int32_t *coef_exponent)
{
#define COEF_SCALE_S 24
    u_int32_t coef_exp, coef_man;
    /*
     * ALGO -> coef_exp = 14-floor(log2(coef));
     * floor(log2(x)) is the highest set bit position
     */
    for (coef_exp = 31; coef_exp > 0; coef_exp--)
            if ((coef_scaled >> coef_exp) & 0x1)
                    break;
    /* A coef_exp of 0 is a legal bit position but an unexpected coef_exp */
    HALASSERT(coef_exp);
    coef_exp = 14 - (coef_exp - COEF_SCALE_S);

    /*
     * ALGO -> coef_man = floor(coef* 2^coef_exp+0.5);
     * The coefficient is already shifted up for scaling
     */
    coef_man = coef_scaled + (1 << (COEF_SCALE_S - coef_exp - 1));

    *coef_mantissa = coef_man >> (COEF_SCALE_S - coef_exp);
    *coef_exponent = coef_exp - 16;

#undef COEF_SCALE_S    
}

void
ar5416SetDeltaSlope(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan)
{
#define INIT_CLOCKMHZSCALED	0x64000000
	u_int32_t coef_scaled, ds_coef_exp, ds_coef_man;
	u_int32_t clockMhzScaled = INIT_CLOCKMHZSCALED;

	CHAN_CENTERS centers;

	if (IS_CHAN_TURBO(chan))
		clockMhzScaled *= 2;
	/* half and quarter rate can divide the scaled clock by 2 or 4 respectively */
	/* scale for selected channel bandwidth */ 
	if (IS_CHAN_HALF_RATE(chan)) {
		clockMhzScaled = clockMhzScaled >> 1;
	} else if (IS_CHAN_QUARTER_RATE(chan)) {
		clockMhzScaled = clockMhzScaled >> 2;
	} 

	/*
	 * ALGO -> coef = 1e8/fcarrier*fclock/40;
	 * scaled coef to provide precision for this floating calculation 
	 */
	ar5416GetChannelCenters(ah, chan, &centers);
	coef_scaled = clockMhzScaled / centers.synth_center;		
	
 	ar5416GetDeltaSlopeValues(ah, coef_scaled, &ds_coef_man, &ds_coef_exp);

	OS_REG_RMW_FIELD(ah, AR_PHY_TIMING3,
		AR_PHY_TIMING3_DSC_MAN, ds_coef_man);
	OS_REG_RMW_FIELD(ah, AR_PHY_TIMING3,
		AR_PHY_TIMING3_DSC_EXP, ds_coef_exp);
		
        /*
         * For Short GI,
         * scaled coeff is 9/10 that of normal coeff
         */ 
        coef_scaled = (9 * coef_scaled)/10;

        ar5416GetDeltaSlopeValues(ah, coef_scaled, &ds_coef_man, &ds_coef_exp);

        /* for short gi */
        OS_REG_RMW_FIELD(ah, AR_PHY_HALFGI,
                AR_PHY_HALFGI_DSC_MAN, ds_coef_man);
        OS_REG_RMW_FIELD(ah, AR_PHY_HALFGI,
                AR_PHY_HALFGI_DSC_EXP, ds_coef_exp);	
#undef INIT_CLOCKMHZSCALED
}

#define SPUR_RSSI_THRESH 40

static void
ar5416SpurMitigate(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan)
{
    static const int pilot_mask_reg[4] = { AR_PHY_TIMING7, AR_PHY_TIMING8,
                AR_PHY_PILOT_MASK_01_30, AR_PHY_PILOT_MASK_31_60 };
    static const int chan_mask_reg[4] = { AR_PHY_TIMING9, AR_PHY_TIMING10,
                AR_PHY_CHANNEL_MASK_01_30, AR_PHY_CHANNEL_MASK_31_60 };
    static const int inc[4] = { 0, 100, 0, 0 };

    int bb_spur = AR_NO_SPUR;
    int bin, cur_bin;
    int spur_freq_sd;
    int spur_delta_phase;
    int denominator;
    int upper, lower, cur_vit_mask;
    int tmp, new;
    int i;

    int8_t mask_m[123];
    int8_t mask_p[123];
    int8_t mask_amt;
    int tmp_mask;
    int cur_bb_spur;
    HAL_BOOL is2GHz = IS_CHAN_2GHZ(chan);

    OS_MEMZERO(mask_m, sizeof(mask_m));
    OS_MEMZERO(mask_p, sizeof(mask_p));

    /*
     * Need to verify range +/- 9.5 for static ht20, otherwise spur
     * is out-of-band and can be ignored.
     */
    for (i = 0; i < AR5416_EEPROM_MODAL_SPURS; i++) {
        cur_bb_spur = ath_hal_getSpurChan(ah, i, is2GHz);
        if (AR_NO_SPUR == cur_bb_spur)
            break;
        cur_bb_spur = cur_bb_spur - (chan->channel * 10);
        if ((cur_bb_spur > -95) && (cur_bb_spur < 95)) {
            bb_spur = cur_bb_spur;
            break;
        }
    }
    if (AR_NO_SPUR == bb_spur)
        return;

    bin = bb_spur * 32;

    tmp = OS_REG_READ(ah, AR_PHY_TIMING_CTRL4_CHAIN(0));
    new = tmp | (AR_PHY_TIMING_CTRL4_ENABLE_SPUR_RSSI |
        AR_PHY_TIMING_CTRL4_ENABLE_SPUR_FILTER |
        AR_PHY_TIMING_CTRL4_ENABLE_CHAN_MASK |
        AR_PHY_TIMING_CTRL4_ENABLE_PILOT_MASK);

    OS_REG_WRITE(ah, AR_PHY_TIMING_CTRL4_CHAIN(0), new);

    new = (AR_PHY_SPUR_REG_MASK_RATE_CNTL |
        AR_PHY_SPUR_REG_ENABLE_MASK_PPM |
        AR_PHY_SPUR_REG_MASK_RATE_SELECT |
        AR_PHY_SPUR_REG_ENABLE_VIT_SPUR_RSSI |
        SM(SPUR_RSSI_THRESH, AR_PHY_SPUR_REG_SPUR_RSSI_THRESH));
    OS_REG_WRITE(ah, AR_PHY_SPUR_REG, new);
    /*
     * Should offset bb_spur by +/- 10 MHz for dynamic 2040 MHz
     * config, no offset for HT20.
     * spur_delta_phase = bb_spur/40 * 2**21 for static ht20,
     * /80 for dyn2040.
     */
    spur_delta_phase = ((bb_spur * 524288) / 100) &
        AR_PHY_TIMING11_SPUR_DELTA_PHASE;
    /*
     * in 11A mode the denominator of spur_freq_sd should be 40 and
     * it should be 44 in 11G
     */
    denominator = IS_CHAN_2GHZ(chan) ? 440 : 400;
    spur_freq_sd = ((bb_spur * 2048) / denominator) & 0x3ff;

    new = (AR_PHY_TIMING11_USE_SPUR_IN_AGC |
        SM(spur_freq_sd, AR_PHY_TIMING11_SPUR_FREQ_SD) |
        SM(spur_delta_phase, AR_PHY_TIMING11_SPUR_DELTA_PHASE));
    OS_REG_WRITE(ah, AR_PHY_TIMING11, new);


    /*
     * ============================================
     * pilot mask 1 [31:0] = +6..-26, no 0 bin
     * pilot mask 2 [19:0] = +26..+7
     *
     * channel mask 1 [31:0] = +6..-26, no 0 bin
     * channel mask 2 [19:0] = +26..+7
     */
    //cur_bin = -26;
    cur_bin = -6000;
    upper = bin + 100;
    lower = bin - 100;

    for (i = 0; i < 4; i++) {
        int pilot_mask = 0;
        int chan_mask  = 0;
        int bp         = 0;
        for (bp = 0; bp < 30; bp++) {
            if ((cur_bin > lower) && (cur_bin < upper)) {
                pilot_mask = pilot_mask | 0x1 << bp;
                chan_mask  = chan_mask | 0x1 << bp;
            }
            cur_bin += 100;
        }
        cur_bin += inc[i];
        OS_REG_WRITE(ah, pilot_mask_reg[i], pilot_mask);
        OS_REG_WRITE(ah, chan_mask_reg[i], chan_mask);
    }

    /* =================================================
     * viterbi mask 1 based on channel magnitude
     * four levels 0-3
     *  - mask (-27 to 27) (reg 64,0x9900 to 67,0x990c)
     *      [1 2 2 1] for -9.6 or [1 2 1] for +16
     *  - enable_mask_ppm, all bins move with freq
     *
     *  - mask_select,    8 bits for rates (reg 67,0x990c)
     *  - mask_rate_cntl, 8 bits for rates (reg 67,0x990c)
     *      choose which mask to use mask or mask2
     */

    /*
     * viterbi mask 2  2nd set for per data rate puncturing
     * four levels 0-3
     *  - mask_select, 8 bits for rates (reg 67)
     *  - mask (-27 to 27) (reg 98,0x9988 to 101,0x9994)
     *      [1 2 2 1] for -9.6 or [1 2 1] for +16
     */
    cur_vit_mask = 6100;
    upper        = bin + 120;
    lower        = bin - 120;

    for (i = 0; i < 123; i++) {
        if ((cur_vit_mask > lower) && (cur_vit_mask < upper)) {
            if ((abs(cur_vit_mask - bin)) < 75) {
                mask_amt = 1;
            } else {
                mask_amt = 0;
            }
            if (cur_vit_mask < 0) {
                mask_m[abs(cur_vit_mask / 100)] = mask_amt;
            } else {
                mask_p[cur_vit_mask / 100] = mask_amt;
            }
        }
        cur_vit_mask -= 100;
    }

    tmp_mask = (mask_m[46] << 30) | (mask_m[47] << 28)
          | (mask_m[48] << 26) | (mask_m[49] << 24)
          | (mask_m[50] << 22) | (mask_m[51] << 20)
          | (mask_m[52] << 18) | (mask_m[53] << 16)
          | (mask_m[54] << 14) | (mask_m[55] << 12)
          | (mask_m[56] << 10) | (mask_m[57] <<  8)
          | (mask_m[58] <<  6) | (mask_m[59] <<  4)
          | (mask_m[60] <<  2) | (mask_m[61] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK_1, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_VIT_MASK2_M_46_61, tmp_mask);

    tmp_mask =             (mask_m[31] << 28)
          | (mask_m[32] << 26) | (mask_m[33] << 24)
          | (mask_m[34] << 22) | (mask_m[35] << 20)
          | (mask_m[36] << 18) | (mask_m[37] << 16)
          | (mask_m[48] << 14) | (mask_m[39] << 12)
          | (mask_m[40] << 10) | (mask_m[41] <<  8)
          | (mask_m[42] <<  6) | (mask_m[43] <<  4)
          | (mask_m[44] <<  2) | (mask_m[45] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK_2, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_M_31_45, tmp_mask);

    tmp_mask = (mask_m[16] << 30) | (mask_m[16] << 28)
          | (mask_m[18] << 26) | (mask_m[18] << 24)
          | (mask_m[20] << 22) | (mask_m[20] << 20)
          | (mask_m[22] << 18) | (mask_m[22] << 16)
          | (mask_m[24] << 14) | (mask_m[24] << 12)
          | (mask_m[25] << 10) | (mask_m[26] <<  8)
          | (mask_m[27] <<  6) | (mask_m[28] <<  4)
          | (mask_m[29] <<  2) | (mask_m[30] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK_3, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_M_16_30, tmp_mask);

    tmp_mask = (mask_m[ 0] << 30) | (mask_m[ 1] << 28)
          | (mask_m[ 2] << 26) | (mask_m[ 3] << 24)
          | (mask_m[ 4] << 22) | (mask_m[ 5] << 20)
          | (mask_m[ 6] << 18) | (mask_m[ 7] << 16)
          | (mask_m[ 8] << 14) | (mask_m[ 9] << 12)
          | (mask_m[10] << 10) | (mask_m[11] <<  8)
          | (mask_m[12] <<  6) | (mask_m[13] <<  4)
          | (mask_m[14] <<  2) | (mask_m[15] <<  0);
    OS_REG_WRITE(ah, AR_PHY_MASK_CTL, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_M_00_15, tmp_mask);

    tmp_mask =             (mask_p[15] << 28)
          | (mask_p[14] << 26) | (mask_p[13] << 24)
          | (mask_p[12] << 22) | (mask_p[11] << 20)
          | (mask_p[10] << 18) | (mask_p[ 9] << 16)
          | (mask_p[ 8] << 14) | (mask_p[ 7] << 12)
          | (mask_p[ 6] << 10) | (mask_p[ 5] <<  8)
          | (mask_p[ 4] <<  6) | (mask_p[ 3] <<  4)
          | (mask_p[ 2] <<  2) | (mask_p[ 1] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_1, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_15_01, tmp_mask);

    tmp_mask =             (mask_p[30] << 28)
          | (mask_p[29] << 26) | (mask_p[28] << 24)
          | (mask_p[27] << 22) | (mask_p[26] << 20)
          | (mask_p[25] << 18) | (mask_p[24] << 16)
          | (mask_p[23] << 14) | (mask_p[22] << 12)
          | (mask_p[21] << 10) | (mask_p[20] <<  8)
          | (mask_p[19] <<  6) | (mask_p[18] <<  4)
          | (mask_p[17] <<  2) | (mask_p[16] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_2, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_30_16, tmp_mask);

    tmp_mask =             (mask_p[45] << 28)
          | (mask_p[44] << 26) | (mask_p[43] << 24)
          | (mask_p[42] << 22) | (mask_p[41] << 20)
          | (mask_p[40] << 18) | (mask_p[39] << 16)
          | (mask_p[38] << 14) | (mask_p[37] << 12)
          | (mask_p[36] << 10) | (mask_p[35] <<  8)
          | (mask_p[34] <<  6) | (mask_p[33] <<  4)
          | (mask_p[32] <<  2) | (mask_p[31] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_3, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_45_31, tmp_mask);

    tmp_mask = (mask_p[61] << 30) | (mask_p[60] << 28)
          | (mask_p[59] << 26) | (mask_p[58] << 24)
          | (mask_p[57] << 22) | (mask_p[56] << 20)
          | (mask_p[55] << 18) | (mask_p[54] << 16)
          | (mask_p[53] << 14) | (mask_p[52] << 12)
          | (mask_p[51] << 10) | (mask_p[50] <<  8)
          | (mask_p[49] <<  6) | (mask_p[48] <<  4)
          | (mask_p[47] <<  2) | (mask_p[46] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_4, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_61_45, tmp_mask);
}

#ifdef AH_SUPPORT_AR9200
#define	AR_BASE_FREQ_2GHZ	2300
#define	AR_BASE_FREQ_5GHZ	4900
#define	AR_SPUR_FEEQ_BOUND_HT40	19
#define	AR_SPUR_FEEQ_BOUND_HT20	10

static void
ar9280SpurMitigate(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *ichan)
{
    static const int pilot_mask_reg[4] = { AR_PHY_TIMING7, AR_PHY_TIMING8,
                AR_PHY_PILOT_MASK_01_30, AR_PHY_PILOT_MASK_31_60 };
    static const int chan_mask_reg[4] = { AR_PHY_TIMING9, AR_PHY_TIMING10,
                AR_PHY_CHANNEL_MASK_01_30, AR_PHY_CHANNEL_MASK_31_60 };
    static int inc[4] = { 0, 100, 0, 0 };

    int bb_spur = AR_NO_SPUR;
    int freq;
    int bin, cur_bin;
    int bb_spur_off, spur_subchannel_sd;
    int spur_freq_sd;
    int spur_delta_phase;
    int denominator;
    int upper, lower, cur_vit_mask;
    int tmp, newVal;
    int i;
    CHAN_CENTERS centers;

    int8_t mask_m[123];
    int8_t mask_p[123];
    int8_t mask_amt;
    int tmp_mask;
    int cur_bb_spur;
    HAL_BOOL is2GHz = IS_CHAN_2GHZ(ichan);

    OS_MEMZERO(&mask_m, sizeof(int8_t) * 123);
    OS_MEMZERO(&mask_p, sizeof(int8_t) * 123);

    ar5416GetChannelCenters(ah, ichan, &centers);
    freq = centers.synth_center;

    /*
     * Need to verify range +/- 9.38 for static ht20 and +/- 18.75 for ht40,
     * otherwise spur is out-of-band and can be ignored.
     */
    for (i = 0; i < AR5416_EEPROM_MODAL_SPURS; i++) {
        cur_bb_spur = ath_hal_getSpurChan(ah, i, is2GHz);
        /* Get actual spur freq in MHz from EEPROM read value */ 
        if (is2GHz) {
            cur_bb_spur =  (cur_bb_spur / 10) + AR_BASE_FREQ_2GHZ;
        } else {
            cur_bb_spur =  (cur_bb_spur / 10) + AR_BASE_FREQ_5GHZ;
        }

        if (AR_NO_SPUR == cur_bb_spur)
            break;
        cur_bb_spur = cur_bb_spur - freq;

        if (IS_CHAN_HT40(ichan)) {
            if ((cur_bb_spur > -AR_SPUR_FEEQ_BOUND_HT40) && 
                (cur_bb_spur < AR_SPUR_FEEQ_BOUND_HT40)) {
                bb_spur = cur_bb_spur;
                break;
            }
        } else if ((cur_bb_spur > -AR_SPUR_FEEQ_BOUND_HT20) &&
                   (cur_bb_spur < AR_SPUR_FEEQ_BOUND_HT20)) {
            bb_spur = cur_bb_spur;
            break;
        }
    }

    if (AR_NO_SPUR == bb_spur) {
#if 1
        /*
         * MRC CCK can interfere with beacon detection and cause deaf/mute.
         * Disable MRC CCK for now.
         */
        OS_REG_CLR_BIT(ah, AR_PHY_FORCE_CLKEN_CCK, AR_PHY_FORCE_CLKEN_CCK_MRC_MUX);
#else
        /* Enable MRC CCK if no spur is found in this channel. */
        OS_REG_SET_BIT(ah, AR_PHY_FORCE_CLKEN_CCK, AR_PHY_FORCE_CLKEN_CCK_MRC_MUX);
#endif
        return;
    } else {
        /* 
         * For Merlin, spur can break CCK MRC algorithm. Disable CCK MRC if spur
         * is found in this channel.
         */
        OS_REG_CLR_BIT(ah, AR_PHY_FORCE_CLKEN_CCK, AR_PHY_FORCE_CLKEN_CCK_MRC_MUX);
    }

    bin = bb_spur * 320;

    tmp = OS_REG_READ(ah, AR_PHY_TIMING_CTRL4_CHAIN(0));

    newVal = tmp | (AR_PHY_TIMING_CTRL4_ENABLE_SPUR_RSSI |
        AR_PHY_TIMING_CTRL4_ENABLE_SPUR_FILTER |
        AR_PHY_TIMING_CTRL4_ENABLE_CHAN_MASK |
        AR_PHY_TIMING_CTRL4_ENABLE_PILOT_MASK);
    OS_REG_WRITE(ah, AR_PHY_TIMING_CTRL4_CHAIN(0), newVal);

    newVal = (AR_PHY_SPUR_REG_MASK_RATE_CNTL |
        AR_PHY_SPUR_REG_ENABLE_MASK_PPM |
        AR_PHY_SPUR_REG_MASK_RATE_SELECT |
        AR_PHY_SPUR_REG_ENABLE_VIT_SPUR_RSSI |
        SM(SPUR_RSSI_THRESH, AR_PHY_SPUR_REG_SPUR_RSSI_THRESH));
    OS_REG_WRITE(ah, AR_PHY_SPUR_REG, newVal);

    /* Pick control or extn channel to cancel the spur */
    if (IS_CHAN_HT40(ichan)) {
        if (bb_spur < 0) {
            spur_subchannel_sd = 1;
            bb_spur_off = bb_spur + 10;
        } else {
            spur_subchannel_sd = 0;
            bb_spur_off = bb_spur - 10;
        }
    } else {
        spur_subchannel_sd = 0;
        bb_spur_off = bb_spur;
    }

    /*
     * spur_delta_phase = bb_spur/40 * 2**21 for static ht20,
     * /80 for dyn2040.
     */
    if (IS_CHAN_HT40(ichan))
        spur_delta_phase = ((bb_spur * 262144) / 10) & AR_PHY_TIMING11_SPUR_DELTA_PHASE;    
    else
        spur_delta_phase = ((bb_spur * 524288) / 10) & AR_PHY_TIMING11_SPUR_DELTA_PHASE;

    /*
     * in 11A mode the denominator of spur_freq_sd should be 40 and
     * it should be 44 in 11G
     */
    denominator = IS_CHAN_2GHZ(ichan) ? 44 : 40;
    spur_freq_sd = ((bb_spur_off * 2048) / denominator) & 0x3ff;

    newVal = (AR_PHY_TIMING11_USE_SPUR_IN_AGC |
        SM(spur_freq_sd, AR_PHY_TIMING11_SPUR_FREQ_SD) |
        SM(spur_delta_phase, AR_PHY_TIMING11_SPUR_DELTA_PHASE));
    OS_REG_WRITE(ah, AR_PHY_TIMING11, newVal);

    /* Choose to cancel between control and extension channels */
    newVal = spur_subchannel_sd << AR_PHY_SFCORR_SPUR_SUBCHNL_SD_S;
    OS_REG_WRITE(ah, AR_PHY_SFCORR_EXT, newVal);

    /*
     * ============================================
     * Set Pilot and Channel Masks
     *
     * pilot mask 1 [31:0] = +6..-26, no 0 bin
     * pilot mask 2 [19:0] = +26..+7
     *
     * channel mask 1 [31:0] = +6..-26, no 0 bin
     * channel mask 2 [19:0] = +26..+7
     */
    cur_bin = -6000;
    upper = bin + 100;
    lower = bin - 100;

    for (i = 0; i < 4; i++) {
        int pilot_mask = 0;
        int chan_mask  = 0;
        int bp         = 0;
        for (bp = 0; bp < 30; bp++) {
            if ((cur_bin > lower) && (cur_bin < upper)) {
                pilot_mask = pilot_mask | 0x1 << bp;
                chan_mask  = chan_mask | 0x1 << bp;
            }
            cur_bin += 100;
        }
        cur_bin += inc[i];
        OS_REG_WRITE(ah, pilot_mask_reg[i], pilot_mask);
        OS_REG_WRITE(ah, chan_mask_reg[i], chan_mask);
    }

    /* =================================================
     * viterbi mask 1 based on channel magnitude
     * four levels 0-3
     *  - mask (-27 to 27) (reg 64,0x9900 to 67,0x990c)
     *      [1 2 2 1] for -9.6 or [1 2 1] for +16
     *  - enable_mask_ppm, all bins move with freq
     *
     *  - mask_select,    8 bits for rates (reg 67,0x990c)
     *  - mask_rate_cntl, 8 bits for rates (reg 67,0x990c)
     *      choose which mask to use mask or mask2
     */

    /*
     * viterbi mask 2  2nd set for per data rate puncturing
     * four levels 0-3
     *  - mask_select, 8 bits for rates (reg 67)
     *  - mask (-27 to 27) (reg 98,0x9988 to 101,0x9994)
     *      [1 2 2 1] for -9.6 or [1 2 1] for +16
     */
    cur_vit_mask = 6100;
    upper        = bin + 120;
    lower        = bin - 120;

    for (i = 0; i < 123; i++) {
        if ((cur_vit_mask > lower) && (cur_vit_mask < upper)) {
            if ((abs(cur_vit_mask - bin)) < 75) {
                mask_amt = 1;
            } else {
                mask_amt = 0;
            }
            if (cur_vit_mask < 0) {
                mask_m[abs(cur_vit_mask / 100)] = mask_amt;
            } else {
                mask_p[cur_vit_mask / 100] = mask_amt;
            }
        }
        cur_vit_mask -= 100;
    }

    tmp_mask = (mask_m[46] << 30) | (mask_m[47] << 28)
          | (mask_m[48] << 26) | (mask_m[49] << 24)
          | (mask_m[50] << 22) | (mask_m[51] << 20)
          | (mask_m[52] << 18) | (mask_m[53] << 16)
          | (mask_m[54] << 14) | (mask_m[55] << 12)
          | (mask_m[56] << 10) | (mask_m[57] <<  8)
          | (mask_m[58] <<  6) | (mask_m[59] <<  4)
          | (mask_m[60] <<  2) | (mask_m[61] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK_1, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_VIT_MASK2_M_46_61, tmp_mask);

    tmp_mask =             (mask_m[31] << 28)
          | (mask_m[32] << 26) | (mask_m[33] << 24)
          | (mask_m[34] << 22) | (mask_m[35] << 20)
          | (mask_m[36] << 18) | (mask_m[37] << 16)
          | (mask_m[48] << 14) | (mask_m[39] << 12)
          | (mask_m[40] << 10) | (mask_m[41] <<  8)
          | (mask_m[42] <<  6) | (mask_m[43] <<  4)
          | (mask_m[44] <<  2) | (mask_m[45] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK_2, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_M_31_45, tmp_mask);

    tmp_mask = (mask_m[16] << 30) | (mask_m[16] << 28)
          | (mask_m[18] << 26) | (mask_m[18] << 24)
          | (mask_m[20] << 22) | (mask_m[20] << 20)
          | (mask_m[22] << 18) | (mask_m[22] << 16)
          | (mask_m[24] << 14) | (mask_m[24] << 12)
          | (mask_m[25] << 10) | (mask_m[26] <<  8)
          | (mask_m[27] <<  6) | (mask_m[28] <<  4)
          | (mask_m[29] <<  2) | (mask_m[30] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK_3, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_M_16_30, tmp_mask);

    tmp_mask = (mask_m[ 0] << 30) | (mask_m[ 1] << 28)
          | (mask_m[ 2] << 26) | (mask_m[ 3] << 24)
          | (mask_m[ 4] << 22) | (mask_m[ 5] << 20)
          | (mask_m[ 6] << 18) | (mask_m[ 7] << 16)
          | (mask_m[ 8] << 14) | (mask_m[ 9] << 12)
          | (mask_m[10] << 10) | (mask_m[11] <<  8)
          | (mask_m[12] <<  6) | (mask_m[13] <<  4)
          | (mask_m[14] <<  2) | (mask_m[15] <<  0);
    OS_REG_WRITE(ah, AR_PHY_MASK_CTL, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_M_00_15, tmp_mask);

    tmp_mask =             (mask_p[15] << 28)
          | (mask_p[14] << 26) | (mask_p[13] << 24)
          | (mask_p[12] << 22) | (mask_p[11] << 20)
          | (mask_p[10] << 18) | (mask_p[ 9] << 16)
          | (mask_p[ 8] << 14) | (mask_p[ 7] << 12)
          | (mask_p[ 6] << 10) | (mask_p[ 5] <<  8)
          | (mask_p[ 4] <<  6) | (mask_p[ 3] <<  4)
          | (mask_p[ 2] <<  2) | (mask_p[ 1] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_1, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_15_01, tmp_mask);

    tmp_mask =             (mask_p[30] << 28)
          | (mask_p[29] << 26) | (mask_p[28] << 24)
          | (mask_p[27] << 22) | (mask_p[26] << 20)
          | (mask_p[25] << 18) | (mask_p[24] << 16)
          | (mask_p[23] << 14) | (mask_p[22] << 12)
          | (mask_p[21] << 10) | (mask_p[20] <<  8)
          | (mask_p[19] <<  6) | (mask_p[18] <<  4)
          | (mask_p[17] <<  2) | (mask_p[16] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_2, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_30_16, tmp_mask);

    tmp_mask =             (mask_p[45] << 28)
          | (mask_p[44] << 26) | (mask_p[43] << 24)
          | (mask_p[42] << 22) | (mask_p[41] << 20)
          | (mask_p[40] << 18) | (mask_p[39] << 16)
          | (mask_p[38] << 14) | (mask_p[37] << 12)
          | (mask_p[36] << 10) | (mask_p[35] <<  8)
          | (mask_p[34] <<  6) | (mask_p[33] <<  4)
          | (mask_p[32] <<  2) | (mask_p[31] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_3, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_45_31, tmp_mask);

    tmp_mask = (mask_p[61] << 30) | (mask_p[60] << 28)
          | (mask_p[59] << 26) | (mask_p[58] << 24)
          | (mask_p[57] << 22) | (mask_p[56] << 20)
          | (mask_p[55] << 18) | (mask_p[54] << 16)
          | (mask_p[53] << 14) | (mask_p[52] << 12)
          | (mask_p[51] << 10) | (mask_p[50] <<  8)
          | (mask_p[49] <<  6) | (mask_p[48] <<  4)
          | (mask_p[47] <<  2) | (mask_p[46] <<  0);
    OS_REG_WRITE(ah, AR_PHY_BIN_MASK2_4, tmp_mask);
    OS_REG_WRITE(ah, AR_PHY_MASK2_P_61_45, tmp_mask);
}
#endif /* AH_SUPPORT_AR9280 */

/*
 * Set a limit on the overall output power.  Used for dynamic
 * transmit power control and the like.
 *
 * NB: limit is in units of 0.5 dbM.
 */
HAL_BOOL
ar5416SetTxPowerLimit(struct ath_hal *ah, u_int32_t limit)
{
	u_int16_t dummyXpdGains[2];
	u_int16_t expow;
	/* the high 16 bits are used for extra txpower */
	expow = limit >> 16;
        limit &= 0xFFFF;

	AH_PRIVATE(ah)->ah_powerLimit = AH_MIN(limit, MAX_RATE_POWER);
	AH_PRIVATE(ah)->ah_extra_txpow = expow;
	return ar5416SetTransmitPower(ah, AH_PRIVATE(ah)->ah_curchan,
			dummyXpdGains);
}

HAL_BOOL
ar5416GetChipPowerLimits(struct ath_hal *ah, HAL_CHANNEL *chans, u_int32_t nchans)
{
    struct ath_hal_5212 *ahp = AH5212(ah);

    return ahp->ah_rfHal.getChipPowerLim(ah, chans, nchans);
}


/* XXX gag, this is sick */
typedef enum Ar5416_Rates {
	rate6mb,  rate9mb,  rate12mb, rate18mb,
	rate24mb, rate36mb, rate48mb, rate54mb,
	rate1l,   rate2l,   rate2s,   rate5_5l,
	rate5_5s, rate11l,  rate11s,  rateXr,
	rateHt20_0, rateHt20_1, rateHt20_2, rateHt20_3,
	rateHt20_4, rateHt20_5, rateHt20_6, rateHt20_7,
	rateHt40_0, rateHt40_1, rateHt40_2, rateHt40_3,
	rateHt40_4, rateHt40_5, rateHt40_6, rateHt40_7,
	rateDupCck, rateDupOfdm, rateExtCck, rateExtOfdm,
	Ar5416RateSize
} AR5416_RATES;

/**************************************************************
 * ar5416SetTransmitPower
 *
 * Set the transmit power in the baseband for the given
 * operating channel and mode.
 */
static HAL_BOOL
ar5416SetTransmitPower(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan, u_int16_t *rfXpdGain)
{
#define POW_SM(_r, _s)     (((_r) & 0x3f) << (_s))
#define N(a)            (sizeof (a) / sizeof (a[0]))

    MODAL_EEP_HEADER	*pModal;
    struct ath_hal_5212 *ahp = AH5212(ah);
    int16_t		ratesArray[Ar5416RateSize];
    int16_t		txPowerIndexOffset = 0;
    u_int8_t		ht40PowerIncForPdadc = 2;	
    int			i;
    u_int16_t		cfgCtl;
    u_int16_t		powerLimit;
    u_int16_t		twiceAntennaReduction;
    u_int16_t		twiceMaxRegulatoryPower;
    int16_t		maxPower;
    struct ar5416eeprom	*pEepData = &AH5416(ah)->ah_5416eeprom;
    struct ath_hal_private *ahpriv = AH_PRIVATE(ah);

    
    HALASSERT(owl_get_eep_ver(pEepData) == AR5416_EEP_VER);

    if (IS_5416_EMU_EEP(ah))
         return AH_TRUE;

    /* Setup info for the actual eeprom */
    ath_hal_memzero(ratesArray, sizeof(ratesArray));
    cfgCtl = ath_hal_getctl(ah, (HAL_CHANNEL *)chan);
    powerLimit = chan->maxRegTxPower * 2;
    twiceAntennaReduction = chan->antennaMax;
    twiceMaxRegulatoryPower = AH_MIN(MAX_RATE_POWER, AH_PRIVATE(ah)->ah_powerLimit); 
    pModal = &pEepData->modalHeader[IS_CHAN_2GHZ(chan)];
    HALDEBUG(ah,"%s Channel=%u CfgCtl=%u\n",
    			__func__,chan->channel, cfgCtl );      
  
    if (IS_EEP_MINOR_V2(ah)) {
        ht40PowerIncForPdadc = pModal->ht40PowerIncForPdadc;
    }
 
    if (!ar5416SetPowerPerRateTable(ah, pEepData,  chan,
                                    &ratesArray[0],cfgCtl,
                                    twiceAntennaReduction,
				    twiceMaxRegulatoryPower, powerLimit)) {
        HALDEBUG(ah, "ar5416EepromSetTransmitPower: unable to set tx power per rate table\n");
        return AH_FALSE;
    }

    if (!ar5416SetPowerCalTable(ah,  pEepData, chan, &txPowerIndexOffset)) {
        HALDEBUG(ah, "ar5416EepromSetTransmitPower: unable to set power table\n");
        return AH_FALSE;
    }
  
    maxPower = AH_MAX(ratesArray[rate6mb], ratesArray[rateHt20_0]);

    if (IS_CHAN_2GHZ(chan)) {
        maxPower = AH_MAX(maxPower, ratesArray[rate1l]);
    }

    if (IS_CHAN_HT40(chan)) {
        maxPower = AH_MAX(maxPower, ratesArray[rateHt40_0]);
    }

    ahp->ah_tx6PowerInHalfDbm = maxPower;   

    if (AR_SREV_MERLIN_10_OR_LATER(ah))
        AH_PRIVATE(ah)->ah_maxPowerLevel = maxPower + AR5416_PWR_TABLE_OFFSET*2;
    else
        AH_PRIVATE(ah)->ah_maxPowerLevel = maxPower;

    ahp->ah_txPowerIndexOffset = txPowerIndexOffset;

    /*
     * txPowerIndexOffset is set by the SetPowerTable() call -
     *  adjust the rate table (0 offset if rates EEPROM not loaded)
     */
    for (i = 0; i < N(ratesArray); i++) {
        ratesArray[i] = (int16_t)(txPowerIndexOffset + ratesArray[i]);
        if (ratesArray[i] > AR5416_MAX_RATE_POWER)
            ratesArray[i] = AR5416_MAX_RATE_POWER;
#ifdef TEMP_POWER_CAP
        /*
         * workaround: cap on max power 
         * ratesArray table is in half dB steps
         */
        if (ratesArray[i] > 2 * ath_hal_maxTPC) {
            ratesArray[i] = 2 * ath_hal_maxTPC;
        }
#endif
    }

    if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
        for (i = 0; i < Ar5416RateSize; i++) {
            ratesArray[i] -= AR5416_PWR_TABLE_OFFSET*2;
        }
    }


#ifdef AH_EEPROM_DUMP
    ar5416PrintPowerPerRate(ah, ratesArray);
#endif

    /* Write the OFDM power per rate set */
    OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE1,
        POW_SM(ratesArray[rate18mb] + ahpriv->ah_extra_txpow, 24)
          | POW_SM(ratesArray[rate12mb] + ahpriv->ah_extra_txpow, 16)
          | POW_SM(ratesArray[rate9mb] + ahpriv->ah_extra_txpow, 8)
          | POW_SM(ratesArray[rate6mb] + ahpriv->ah_extra_txpow, 0)
    );
    OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE2,
        POW_SM(ratesArray[rate54mb] + ahpriv->ah_extra_txpow, 24)
          | POW_SM(ratesArray[rate48mb] + ahpriv->ah_extra_txpow, 16)
          | POW_SM(ratesArray[rate36mb] + ahpriv->ah_extra_txpow, 8)
          | POW_SM(ratesArray[rate24mb] + ahpriv->ah_extra_txpow, 0)
    );

    if (IS_CHAN_2GHZ(chan)) {
        /* Write the CCK power per rate set */
        OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE3,
            POW_SM(ratesArray[rate2s] + ahpriv->ah_extra_txpow, 24)
              | POW_SM(ratesArray[rate2l] + ahpriv->ah_extra_txpow,  16)
              | POW_SM(ratesArray[rateXr] + ahpriv->ah_extra_txpow,  8) /* XR target power */
              | POW_SM(ratesArray[rate1l] + ahpriv->ah_extra_txpow,   0)
        );
        OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE4,
            POW_SM(ratesArray[rate11s] + ahpriv->ah_extra_txpow, 24)
              | POW_SM(ratesArray[rate11l] + ahpriv->ah_extra_txpow, 16)
              | POW_SM(ratesArray[rate5_5s] + ahpriv->ah_extra_txpow, 8)
              | POW_SM(ratesArray[rate5_5l] + ahpriv->ah_extra_txpow, 0)
        );
    HALDEBUG(ah,"%s AR_PHY_POWER_TX_RATE3=0x%x AR_PHY_POWER_TX_RATE4=0x%x\n",
    			__func__, OS_REG_READ(ah,AR_PHY_POWER_TX_RATE3),
			OS_REG_READ(ah,AR_PHY_POWER_TX_RATE4)); 
    }

    /* Write the HT20 power per rate set */
    OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE5,
        POW_SM(ratesArray[rateHt20_3], 24)
          | POW_SM(ratesArray[rateHt20_2] + ahpriv->ah_extra_txpow, 16)
          | POW_SM(ratesArray[rateHt20_1] + ahpriv->ah_extra_txpow, 8)
          | POW_SM(ratesArray[rateHt20_0] + ahpriv->ah_extra_txpow, 0)
    );
    OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE6,
        POW_SM(ratesArray[rateHt20_7], 24)
          | POW_SM(ratesArray[rateHt20_6] + ahpriv->ah_extra_txpow, 16)
          | POW_SM(ratesArray[rateHt20_5] + ahpriv->ah_extra_txpow, 8)
          | POW_SM(ratesArray[rateHt20_4] + ahpriv->ah_extra_txpow, 0)
    );

    if (IS_CHAN_HT40(chan)) {
        /* Write the HT40 power per rate set */
	/* Correct PAR difference between HT40 and HT20/LEGACY */
        OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE7,
            POW_SM(ratesArray[rateHt40_3] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 24)
              | POW_SM(ratesArray[rateHt40_2] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 16)
              | POW_SM(ratesArray[rateHt40_1] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 8)
              | POW_SM(ratesArray[rateHt40_0] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 0)
        );
        OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE8,
            POW_SM(ratesArray[rateHt40_7] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 24)
              | POW_SM(ratesArray[rateHt40_6] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 16)
              | POW_SM(ratesArray[rateHt40_5] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 8)
              | POW_SM(ratesArray[rateHt40_4] + ht40PowerIncForPdadc + ahpriv->ah_extra_txpow, 0)
        );
        /* Write the Dup/Ext 40 power per rate set */
        OS_REG_WRITE(ah, AR_PHY_POWER_TX_RATE9,
            POW_SM(ratesArray[rateExtOfdm] + ahpriv->ah_extra_txpow, 24)
              | POW_SM(ratesArray[rateExtCck] + ahpriv->ah_extra_txpow, 16)
              | POW_SM(ratesArray[rateDupOfdm] + ahpriv->ah_extra_txpow, 8)
              | POW_SM(ratesArray[rateDupCck] + ahpriv->ah_extra_txpow, 0)
        );
    }

    /* Write the Power subtraction for dynamic chain changing, for per-packet powertx */
    OS_REG_WRITE(ah, AR_PHY_POWER_TX_SUB,
        POW_SM(pModal->pwrDecreaseFor3Chain, 6)
          | POW_SM(pModal->pwrDecreaseFor2Chain, 0)
    );
    return AH_TRUE;
#undef POW_SM
#undef N
}

/*
 * Exported call to check for a recent gain reading and return
 * the current state of the thermal calibration gain engine.
 */
HAL_RFGAIN
ar5416GetRfgain(struct ath_hal *ah)
{
	return HAL_RFGAIN_INACTIVE;
}

/*
 * Places all of hardware into reset
 */
HAL_BOOL
ar5416Disable(struct ath_hal *ah)
{
	if (!ar5416SetPowerMode(ah, HAL_PM_AWAKE, AH_TRUE))
		return AH_FALSE;
	return ar5416SetResetReg(ah, HAL_RESET_COLD);
}

/*
 * Places the PHY and Radio chips into reset.  A full reset
 * must be called to leave this state.  The PCI/MAC/PCU are
 * not placed into reset as we must receive interrupt to
 * re-enable the hardware.
 */
HAL_BOOL
ar5416PhyDisable(struct ath_hal *ah)
{
	return ar5416SetResetReg(ah, HAL_RESET_WARM);
}

/*
 * Write the given reset bit mask into the reset register
 */
HAL_BOOL
ar5416SetResetReg(struct ath_hal *ah, u_int32_t type)
{
	/*
	 * Set force wake
	 */
	OS_REG_WRITE(ah, AR_RTC_FORCE_WAKE,
	     AR_RTC_FORCE_WAKE_EN | AR_RTC_FORCE_WAKE_ON_INT);

	switch (type) {
	case HAL_RESET_POWER_ON:
		return ar5416SetResetPowerOn(ah);
		break;
	case HAL_RESET_WARM:
	case HAL_RESET_COLD:
		return ar5416SetReset(ah, type);
		break;
	default:
		return AH_FALSE;
	}
}

static HAL_BOOL
ar5416SetResetPowerOn(struct ath_hal *ah)
{
    /* Power On Reset (Hard Reset) */

    /*
     * Set force wake
     *	
     * If the MAC was running, previously calling
     * reset will wake up the MAC but it may go back to sleep
     * before we can start polling. 
     * Set force wake  stops that 
     * This must be called before initiating a hard reset.
     */
    OS_REG_WRITE(ah, AR_RTC_FORCE_WAKE,
            AR_RTC_FORCE_WAKE_EN | AR_RTC_FORCE_WAKE_ON_INT);    

    /*
     * RTC reset and clear
     */
    OS_REG_WRITE(ah, AR_RTC_RESET, 0);
    OS_DELAY(20);
    OS_REG_WRITE(ah, AR_RTC_RESET, 1);

    /*
     * Poll till RTC is ON
     */
    if (!ath_hal_wait(ah, AR_RTC_STATUS, AR_RTC_PM_STATUS_M, AR_RTC_STATUS_ON)) {
        HALDEBUG(ah, "%s: RTC not waking up\n", __FUNCTION__);
        return AH_FALSE;
    }

    return ar5416SetReset(ah, HAL_RESET_COLD);
}

#define WAR_19660

static HAL_BOOL
ar5416SetReset(struct ath_hal *ah, int type)
{
    u_int32_t tmpReg;

#ifdef WAR_19660
	/**
	 * Fix for bug 36139.
	 */
	if (AR_SREV_HOWL(ah)) {
		u_int32_t val = OS_REG_READ(ah, AR_RTC_DERIVED_CLK);
		val &= ~AR_RTC_DERIVED_CLK_PERIOD;
		val |= SM(1, AR_RTC_DERIVED_CLK_PERIOD);
		OS_REG_WRITE(ah, AR_RTC_DERIVED_CLK, val);
		(void)OS_REG_READ(ah, AR_RTC_DERIVED_CLK);
	}

	/* disable timers before reset.  
	 * Because of a hardware bug in Owl 1.0, timers are not disabled
	 * on a warm reset.  See bug 19660.
	 * XXX - TODO - remove me for Owl 2.0
	 */
	if (IS_5416V1(ah))
		OS_REG_CLR_BIT(ah, AR_TIMER_MODE, 0xFF);
#endif

    /*
     * Reset AHB
     */
    /* Bug26871 */
    tmpReg = OS_REG_READ(ah, AR_INTR_SYNC_CAUSE);
    if (tmpReg & (AR_INTR_SYNC_LOCAL_TIMEOUT|AR_INTR_SYNC_RADM_CPL_TIMEOUT)) {
	OS_REG_WRITE(ah, AR_INTR_SYNC_ENABLE, 0);
	OS_REG_WRITE(ah, AR_RC, AR_RC_AHB|AR_RC_HOSTIF);
    } else {
	OS_REG_WRITE(ah, AR_RC, AR_RC_AHB);
    }

    /*
     * Set Mac(BB,Phy) Warm Reset
     */
    switch (type) {
    case HAL_RESET_WARM:
            OS_REG_WRITE(ah, AR_RTC_RC, AR_RTC_RC_MAC_WARM);
            break;
        case HAL_RESET_COLD:
            OS_REG_WRITE(ah, AR_RTC_RC, AR_RTC_RC_MAC_WARM|AR_RTC_RC_MAC_COLD);
            break;
        default:
            HALASSERT(0);
            break;
    }
	OS_DELAY(50); /* 50 usec */

    /*
     * Clear resets and force wakeup
     */
    OS_REG_WRITE(ah, AR_RTC_RC, 0);
    if (!ath_hal_wait(ah, AR_RTC_RC, AR_RTC_RC_M, 0)) {
        HALDEBUG(ah, "%s: RTC stuck in MAC reset\n", __FUNCTION__);
        return AH_FALSE;
    }

    /* Clear AHB reset */
    OS_REG_WRITE(ah, AR_RC, 0);

    /*
     * Force wake
     */
    OS_REG_WRITE(ah, AR_RTC_FORCE_WAKE, AR_RTC_FORCE_WAKE_EN | AR_RTC_FORCE_WAKE_ON_INT);

   /* Set register and descriptor swapping on 
    * Bigendian platforms on cold reset 
    */
    if (type == HAL_RESET_COLD) {
#if AH_BYTE_ORDER == AH_BIG_ENDIAN
		HAL_BOOL swapped = AH_FALSE;
		u_int32_t mask;

		mask = INIT_CONFIG_STATUS | AR_CFG_SWRD;
#ifndef AH_NEED_DESC_SWAP
		mask |= AR_CFG_SWTD;
#endif
		if (OS_REG_READ(ah, AR_CFG) & AR_CFG_SWRG)
			swapped = AH_TRUE;

#ifndef AH_SUPPORT_AR7100
		if (!ah->ah_swapped)
			mask |= AR_CFG_SWRG;
		if (!ah->ah_swapped && !swapped)
			OS_REG_WRITE(ah, AR_CFG, LE_READ_4(&mask));
		else
#endif
			OS_REG_WRITE(ah, AR_CFG, mask);

#else
		OS_REG_WRITE(ah, AR_CFG, INIT_CONFIG_STATUS);
#endif
	}

    return AH_TRUE;
}

#define IS_RAD51xx(ah) \
	 (((AH_PRIVATE(ah)->ah_analog5GhzRev & 0xF0) == AR_RAD5133_SREV_MAJOR) || \
	  ((AH_PRIVATE(ah)->ah_analog5GhzRev & 0xF0) == AR_RAD5122_SREV_MAJOR) )
static void
ar5416InitPLL(struct ath_hal *ah, HAL_CHANNEL *chan)
{
	u_int32_t pll;


	if (AR_SREV_HOWL(ah)) {
		if (chan && IS_CHAN_5GHZ(chan))
			pll = 0x1450;
		else
			pll = 0x1458;
	}
	else{
		    if (AR_SREV_MERLIN_10_OR_LATER(ah)) {

            pll = SM(0x5, AR_RTC_SOWL_PLL_REFDIV);

            if (chan && IS_CHAN_HALF_RATE(chan)) {
                pll |= SM(0x1, AR_RTC_SOWL_PLL_CLKSEL);
            } else if (chan && IS_CHAN_QUARTER_RATE(chan)) {
                pll |= SM(0x2, AR_RTC_SOWL_PLL_CLKSEL);
            }
            if (chan && IS_CHAN_5GHZ(chan)) {
                pll |= SM(0x28, AR_RTC_SOWL_PLL_DIV);
                
                /* PLL WAR for Merlin 2.0/2.1
                 * For 10/20 MHz spaced channels, set 0x2850 to prevent reset-to-reset variation 
                 * For 5 MHz spacing channels, set 0x142c to improve Tx EVM
                 */
#ifdef AH_SUPPORT_AR9200
                if (AR_SREV_MERLIN_20(ah)) {
                    if (((chan->channel % 20) == 0) || ((chan->channel % 10) == 0)) 
                        pll = 0x2850;
                    else {
                        pll = 0x142c;
                    }
				}
#endif
            } else {
                pll |= SM(0x2c, AR_RTC_SOWL_PLL_DIV);
            }
	 } else if (AR_SREV_SOWL_10_OR_LATER(ah)) {
	      pll = SM(0x5, AR_RTC_SOWL_PLL_REFDIV);

	      if (chan && IS_CHAN_HALF_RATE(chan)) {
		      pll |= SM(0x1, AR_RTC_SOWL_PLL_CLKSEL);
	      } else if (chan && IS_CHAN_QUARTER_RATE(chan)) {
		      pll |= SM(0x2, AR_RTC_SOWL_PLL_CLKSEL);
	      }
	      if (chan && IS_CHAN_5GHZ(chan)) {
		      pll |= SM(0x50, AR_RTC_SOWL_PLL_DIV);
	      } else {
		      pll |= SM(0x58, AR_RTC_SOWL_PLL_DIV);
	      }
	} else {
		pll = AR_RTC_PLL_REFDIV_5 | AR_RTC_PLL_DIV2;

		if (chan && IS_CHAN_HALF_RATE(chan)) {
		    pll |= SM(0x1, AR_RTC_PLL_CLKSEL);
		} else if (chan && IS_CHAN_QUARTER_RATE(chan)) {
		    pll |= SM(0x2, AR_RTC_PLL_CLKSEL);
		}
		if (chan && IS_CHAN_5GHZ(chan)) {
			pll |= SM(0xa, AR_RTC_PLL_DIV);
		} else {
			pll |= SM(0xb, AR_RTC_PLL_DIV);
	    }
	}
	}
	OS_REG_WRITE(ah, AR_RTC_PLL_CONTROL, pll);
	/* TODO:
	* For multi-band owl, switch between bands by reiniting the PLL.
	*/
	
	OS_DELAY(RTC_PLL_SETTLE_DELAY);

	OS_REG_WRITE(ah, AR_RTC_SLEEP_CLK, AR_RTC_FORCE_DERIVED_CLK);
}

/*
 * Read EEPROM header info and program the device for correct operation
 * given the channel value.
 */
static HAL_BOOL
ar5416SetBoardValues(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan)
{
    MODAL_EEP_HEADER	*pModal;
    int			i, regChainOffset;
    struct ar5416eeprom	*eep = &AH5416(ah)->ah_5416eeprom;
    u_int8_t		txRxAttenLocal;    /* workaround for eeprom versions <= 14.2 */
    int8_t		thresh62[2];

    HALASSERT(owl_get_eep_ver(eep) == AR5416_EEP_VER);
    pModal = &(eep->modalHeader[IS_CHAN_2GHZ(chan)]);

    txRxAttenLocal = IS_CHAN_2GHZ(chan) ? 23 : 38;    /* workaround for eeprom versions <= 14.2 */

    if (IS_5416_EMU_EEP(ah))
        return AH_TRUE;

    OS_REG_WRITE(ah, AR_PHY_SWITCH_COM, pModal->antCtrlCommon);
    for (i = 0; i < AR5416_MAX_CHAINS; i++) { 
        if (AR_SREV_MERLIN(ah)) {
            if (i >= 2) break;
        }
       	   if (!(IS_5416V1(ah)) &&
            (AH5416(ah)->ah_rx_chainmask == 0x5 ||
	     AH5416(ah)->ah_tx_chainmask == 0x5) && i != 0) {
            /* Regs are swapped from chain 2 to 1 for 5416 2_0 with 
             * only chains 0 and 2 populated 
             */
            regChainOffset = (i == 1) ? 0x2000 : 0x1000;
        } else {
            regChainOffset = i * 0x1000;
        }

        OS_REG_WRITE(ah, AR_PHY_SWITCH_CHAIN_0 + regChainOffset, pModal->antCtrlChain[i]);
        OS_REG_WRITE(ah, AR_PHY_TIMING_CTRL4 + regChainOffset, 
        	(OS_REG_READ(ah, AR_PHY_TIMING_CTRL4 + regChainOffset) &
        	~(AR_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF | AR_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF)) |
        	SM(pModal->iqCalICh[i], AR_PHY_TIMING_CTRL4_IQCORR_Q_I_COFF) |
        	SM(pModal->iqCalQCh[i], AR_PHY_TIMING_CTRL4_IQCORR_Q_Q_COFF));

        /*
         * Large signal upgrade.
         */

        if ((i == 0) || !IS_5416V1(ah)) {




            if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
                /* 
                 * From Merlin(AR9280),
                 *     txRxAttenCh[] maps to chain specific xatten1_hyst_margin(0x9848/0xa848 13:7)
                 *     rxTxMarginCh[] maps to chain specific xatten2_hyst_margin (0x9848/0xa848 20:14)
                 */
		if (IS_EEP_MINOR_V3(ah))
		{
		    OS_REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ + regChainOffset, AR_PHY_GAIN_2GHZ_XATTEN1_MARGIN, pModal->bswMargin[i]);
                    OS_REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ + regChainOffset, AR_PHY_GAIN_2GHZ_XATTEN1_DB, pModal->bswAtten[i]);
                    OS_REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ + regChainOffset, AR_PHY_GAIN_2GHZ_XATTEN2_MARGIN, pModal->xatten2Margin[i]);
                    OS_REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ + regChainOffset, AR_PHY_GAIN_2GHZ_XATTEN2_DB, pModal->xatten2Db[i]);   
		}
                OS_REG_RMW_FIELD(ah, AR_PHY_RXGAIN + regChainOffset, AR9280_PHY_RXGAIN_TXRX_ATTEN, txRxAttenLocal);
                OS_REG_RMW_FIELD(ah, AR_PHY_RXGAIN + regChainOffset, AR9280_PHY_RXGAIN_TXRX_MARGIN, pModal->rxTxMarginCh[i]);
            } else {
            OS_REG_WRITE(ah, AR_PHY_RXGAIN + regChainOffset, (OS_REG_READ(ah, AR_PHY_RXGAIN + regChainOffset) & ~AR_PHY_RXGAIN_TXRX_ATTEN) |
			SM(IS_EEP_MINOR_V3(ah)  ? pModal->txRxAttenCh[i] : txRxAttenLocal,
				AR_PHY_RXGAIN_TXRX_ATTEN));

            OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + regChainOffset, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + regChainOffset) & ~AR_PHY_GAIN_2GHZ_RXTX_MARGIN) |
			SM(pModal->rxTxMarginCh[i], AR_PHY_GAIN_2GHZ_RXTX_MARGIN));
            }


        }
    }
#if 0 /* Remove per Don Breslin */
    /* write previous IQ results */
    OS_REG_SET_BIT(ah, AR_PHY_TIMING_CTRL4, AR_PHY_TIMING_CTRL4_IQCORR_ENABLE);
#endif

    /* 
     *  There are two methods to fix PCI write unreliable to analog register
     *    space (0x7800- 0x7898):
     *    1. Use analog long shift register.
     *    2. Use delay.
     *    but, the new value can not be written correcly by using method 1. So
     *      we select method 2 to work around this issue. (Reported Bug:32840)
     */
    if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
        u_int32_t regValue;
        
        if (IS_CHAN_2GHZ(chan)) {
            // Update OB DB for RF2G CH0
            regValue = OS_REG_READ(ah, AR_AN_RF2G1_CH0); 
            regValue &= (~AR_AN_RF2G1_CH0_OB) & (~AR_AN_RF2G1_CH0_DB);
            OS_REG_WRITE(ah, AR_AN_RF2G1_CH0, regValue | SM(pModal->ob,AR_AN_RF2G1_CH0_OB) |
                                                     SM(pModal->db,AR_AN_RF2G1_CH0_DB));
            OS_DELAY(100);   
            // Update OB DB for RF2G CH1
            regValue = OS_REG_READ(ah, AR_AN_RF2G1_CH1); 
            regValue &= (~AR_AN_RF2G1_CH1_OB) & (~AR_AN_RF2G1_CH1_DB);
            OS_REG_WRITE(ah, AR_AN_RF2G1_CH1, regValue | SM(pModal->ob,AR_AN_RF2G1_CH1_OB) |
                                                     SM(pModal->db,AR_AN_RF2G1_CH1_DB));
            OS_DELAY(100);
        } else {
            // Update OB DB for RF5G CH0
            regValue = OS_REG_READ(ah, AR_AN_RF5G1_CH0); 
            regValue &= (~AR_AN_RF5G1_CH0_OB5) & (~AR_AN_RF5G1_CH0_DB5);
            OS_REG_WRITE(ah, AR_AN_RF5G1_CH0, regValue | SM(pModal->ob,AR_AN_RF5G1_CH0_OB5) |
                                                     SM(pModal->db,AR_AN_RF5G1_CH0_DB5));
            OS_DELAY(100);   
            // Update OB DB for RF5G CH1
            regValue = OS_REG_READ(ah, AR_AN_RF5G1_CH1); 
            regValue &= (~AR_AN_RF5G1_CH1_OB5) & (~AR_AN_RF5G1_CH1_DB5);
            OS_REG_WRITE(ah, AR_AN_RF5G1_CH1, regValue | SM(pModal->ob,AR_AN_RF5G1_CH1_OB5) |
                                                     SM(pModal->db,AR_AN_RF5G1_CH1_DB5));
            OS_DELAY(100);   
        }
        // Update XPABias and LocalBias
        regValue = OS_REG_READ(ah, AR_AN_TOP2);   
        regValue &= (~AR_AN_TOP2_XPABIAS_LVL) & (~AR_AN_TOP2_LOCALBIAS); 
        OS_REG_WRITE(ah, AR_AN_TOP2, regValue | SM(pModal->xpaBiasLvl,AR_AN_TOP2_XPABIAS_LVL) | 
                                                SM(pModal->local_bias,AR_AN_TOP2_LOCALBIAS));
        OS_DELAY(100);
        OS_REG_RMW_FIELD(ah, AR_PHY_XPA_CFG, AR_PHY_FORCE_XPA_CFG,pModal->force_xpaon ); 
        OS_DELAY(100);
    }



    OS_REG_RMW_FIELD(ah, AR_PHY_SETTLING, AR_PHY_SETTLING_SWITCH, pModal->switchSettling);
    OS_REG_RMW_FIELD(ah, AR_PHY_DESIRED_SZ, AR_PHY_DESIRED_SZ_ADC, pModal->adcDesiredSize);
    if (!AR_SREV_MERLIN_10_OR_LATER(ah)) {
        OS_REG_RMW_FIELD(ah, AR_PHY_DESIRED_SZ, AR_PHY_DESIRED_SZ_PGA, pModal->pgaDesiredSize);
    }
    OS_REG_WRITE(ah, AR_PHY_RF_CTL4,
        SM(pModal->txEndToXpaOff, AR_PHY_RF_CTL4_TX_END_XPAA_OFF)
        | SM(pModal->txEndToXpaOff, AR_PHY_RF_CTL4_TX_END_XPAB_OFF)
        | SM(pModal->txFrameToXpaOn, AR_PHY_RF_CTL4_FRAME_XPAA_ON)
        | SM(pModal->txFrameToXpaOn, AR_PHY_RF_CTL4_FRAME_XPAB_ON));

    OS_REG_RMW_FIELD(ah, AR_PHY_RF_CTL3, AR_PHY_TX_END_TO_A2_RX_ON, pModal->txEndToRxOn);
    
    /* XXX We need to specify the CCA threshold for the extension channel in the eeprom. */
#ifdef USE_FIXED_CCA
    thresh62[0] = thresh62[1] = FIXED_CCA_THRESHOLD;
#else 
    thresh62[0] = pModal->thresh62;
    thresh62[1] = pModal->thresh62;
#endif

    ar5416SetCCAThreshold(ah,thresh62);
    
    /* Minor Version Specific application */
    if (IS_EEP_MINOR_V2(ah)) {
        OS_REG_RMW_FIELD(ah, AR_PHY_RF_CTL2,  AR_PHY_TX_FRAME_TO_DATA_START, pModal->txFrameToDataStart);
        OS_REG_RMW_FIELD(ah, AR_PHY_RF_CTL2,  AR_PHY_TX_FRAME_TO_PA_ON, pModal->txFrameToPaOn);    
    }	
    
    if (IS_EEP_MINOR_V3(ah)) {
	if (IS_CHAN_HT40(chan)) {
		/* Overwrite switch settling with HT40 value */
		OS_REG_RMW_FIELD(ah, AR_PHY_SETTLING, AR_PHY_SETTLING_SWITCH, pModal->swSettleHt40);
	}
	
        if ((!IS_5416V1(ah)) &&
            (  AH5416(ah)->ah_rx_chainmask == 0x5 || AH5416(ah)->ah_tx_chainmask == 0x5)){
            /* Reg Offsets are swapped for logical mapping */
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x1000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x1000) & ~AR_PHY_GAIN_2GHZ_BSW_MARGIN) |
			SM(pModal->bswMargin[2], AR_PHY_GAIN_2GHZ_BSW_MARGIN));
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x1000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x1000) & ~AR_PHY_GAIN_2GHZ_BSW_ATTEN) |
			SM(pModal->bswAtten[2], AR_PHY_GAIN_2GHZ_BSW_ATTEN));
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x2000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x2000) & ~AR_PHY_GAIN_2GHZ_BSW_MARGIN) |
			SM(pModal->bswMargin[1], AR_PHY_GAIN_2GHZ_BSW_MARGIN));
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x2000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x2000) & ~AR_PHY_GAIN_2GHZ_BSW_ATTEN) |
			SM(pModal->bswAtten[1], AR_PHY_GAIN_2GHZ_BSW_ATTEN));
        } else {
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x1000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x1000) & ~AR_PHY_GAIN_2GHZ_BSW_MARGIN) |
			SM(pModal->bswMargin[1], AR_PHY_GAIN_2GHZ_BSW_MARGIN));
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x1000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x1000) & ~AR_PHY_GAIN_2GHZ_BSW_ATTEN) |
			SM(pModal->bswAtten[1], AR_PHY_GAIN_2GHZ_BSW_ATTEN));
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x2000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x2000) & ~AR_PHY_GAIN_2GHZ_BSW_MARGIN) |
			SM(pModal->bswMargin[2],AR_PHY_GAIN_2GHZ_BSW_MARGIN));
		OS_REG_WRITE(ah, AR_PHY_GAIN_2GHZ + 0x2000, (OS_REG_READ(ah, AR_PHY_GAIN_2GHZ + 0x2000) & ~AR_PHY_GAIN_2GHZ_BSW_ATTEN) |
			SM(pModal->bswAtten[2], AR_PHY_GAIN_2GHZ_BSW_ATTEN));
        }
        OS_REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ, AR_PHY_GAIN_2GHZ_BSW_MARGIN, pModal->bswMargin[0]);
        OS_REG_RMW_FIELD(ah, AR_PHY_GAIN_2GHZ, AR_PHY_GAIN_2GHZ_BSW_ATTEN, pModal->bswAtten[0]);
    }
    return AH_TRUE;
}

/*
 * Helper functions common for AP/CB/XB
 */

/*
 * ar5416SetPowerPerRateTable
 *
 * Sets the transmit power in the baseband for the given
 * operating channel and mode.
 */
static HAL_BOOL
ar5416SetPowerPerRateTable(struct ath_hal *ah, struct ar5416eeprom *pEepData,
                           HAL_CHANNEL_INTERNAL *chan,
                           int16_t *ratesArray, u_int16_t cfgCtl,
                           u_int16_t AntennaReduction, 
                           u_int16_t twiceMaxRegulatoryPower,
                           u_int16_t powerLimit)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))
/* Local defines to distinguish between extension and control CTL's */
#define EXT_ADDITIVE (0x8000)
#define CTL_11A_EXT (CTL_11A | EXT_ADDITIVE)
#define CTL_11G_EXT (CTL_11G | EXT_ADDITIVE)
#define CTL_11B_EXT (CTL_11B | EXT_ADDITIVE)

	struct ath_hal_5212 *ahp=AH5212(ah);
	u_int16_t twiceMaxEdgePower = AR5416_MAX_RATE_POWER;
	int i;
	int16_t  twiceLargestAntenna;
	CAL_CTL_DATA *rep;
	CAL_TARGET_POWER_LEG targetPowerOfdm, targetPowerCck = {0, {0, 0, 0, 0}};
	CAL_TARGET_POWER_LEG targetPowerOfdmExt = {0, {0, 0, 0, 0}}, targetPowerCckExt = {0, {0, 0, 0, 0}};
	CAL_TARGET_POWER_HT  targetPowerHt20, targetPowerHt40 = {0, {0, 0, 0, 0}};
	int16_t scaledPower, minCtlPower;

#define SUB_NUM_CTL_MODES_AT_5G_40 2   /* excluding HT40, EXT-OFDM */
#define SUB_NUM_CTL_MODES_AT_2G_40 3   /* excluding HT40, EXT-OFDM, EXT-CCK */
	u_int16_t ctlModesFor11a[] = {CTL_11A, CTL_5GHT20, CTL_11A_EXT, CTL_5GHT40};
	u_int16_t ctlModesFor11g[] = {CTL_11B, CTL_11G, CTL_2GHT20, CTL_11B_EXT, CTL_11G_EXT, CTL_2GHT40};
	u_int16_t numCtlModes, *pCtlMode, ctlMode, freq;
	CHAN_CENTERS centers;

	ar5416GetChannelCenters(ah,  chan, &centers);

	/* Compute TxPower reduction due to Antenna Gain */

	twiceLargestAntenna = AH_MAX(AH_MAX(pEepData->modalHeader[IS_CHAN_2GHZ(chan)].antennaGainCh[0],
					pEepData->modalHeader[IS_CHAN_2GHZ(chan)].antennaGainCh[1]),
					pEepData->modalHeader[IS_CHAN_2GHZ(chan)].antennaGainCh[2]);
#if 0
	/* Turn it back on if we need to calculate per chain antenna gain reduction */
	/* Use only if the expected gain > 6dbi */
	/* Chain 0 is always used */
	twiceLargestAntenna = pEepData->modalHeader[IS_CHAN_2GHZ(chan)].antennaGainCh[0];

	/* Look at antenna gains of Chains 1 and 2 if the TX mask is set */
	if (ahp->ah_tx_chainmask & 0x2)
		twiceLargestAntenna = AH_MAX(twiceLargestAntenna,
			pEepData->modalHeader[IS_CHAN_2GHZ(chan)].antennaGainCh[1]);

	if (ahp->ah_tx_chainmask & 0x4)
		twiceLargestAntenna = AH_MAX(twiceLargestAntenna,
			pEepData->modalHeader[IS_CHAN_2GHZ(chan)].antennaGainCh[2]);
#endif
	twiceLargestAntenna = (int16_t)AH_MIN((AntennaReduction) - twiceLargestAntenna, 0);
	
	ahp->ah_antennaGainMax[IS_CHAN_2GHZ(chan)] = twiceLargestAntenna;	

	/* 
	 * scaledPower is the minimum of the user input power level and
	 * the regulatory allowed power level
	 */
	scaledPower = AH_MIN(powerLimit, twiceMaxRegulatoryPower + twiceLargestAntenna);

	/* Reduce scaled Power by number of chains active to get to per chain tx power level */
	/* TODO: better value than these? */
	switch (owl_get_ntxchains(AH5416(ah)->ah_tx_chainmask)) {
	case 1:
		break;
	case 2:
		scaledPower -= pEepData->modalHeader[IS_CHAN_2GHZ(chan)].pwrDecreaseFor2Chain;
		break;
	case 3:
		scaledPower -= pEepData->modalHeader[IS_CHAN_2GHZ(chan)].pwrDecreaseFor3Chain;
		break;
	default:
		return AH_FALSE; /* Unsupported number of chains */
	}

	scaledPower = AH_MAX(0, scaledPower);

	/* Get target powers from EEPROM - our baseline for TX Power */
	if (IS_CHAN_2GHZ(chan)) {
		/* Setup for CTL modes */
		numCtlModes = N(ctlModesFor11g) - SUB_NUM_CTL_MODES_AT_2G_40; /* CTL_11B, CTL_11G, CTL_2GHT20 */
		pCtlMode = ctlModesFor11g;

		ar5416GetTargetPowersLeg(ah,  chan, pEepData->calTargetPowerCck,
				AR5416_NUM_2G_CCK_TARGET_POWERS, &targetPowerCck, 4, AH_FALSE);
		ar5416GetTargetPowersLeg(ah,  chan, pEepData->calTargetPower2G,
				AR5416_NUM_2G_20_TARGET_POWERS, &targetPowerOfdm, 4, AH_FALSE);
		ar5416GetTargetPowers(ah,  chan, pEepData->calTargetPower2GHT20,
				AR5416_NUM_2G_20_TARGET_POWERS, &targetPowerHt20, 8, AH_FALSE);

		if (IS_CHAN_HT40(chan)) {
			numCtlModes = N(ctlModesFor11g);    /* All 2G CTL's */

			ar5416GetTargetPowers(ah,  chan, pEepData->calTargetPower2GHT40,
				AR5416_NUM_2G_40_TARGET_POWERS, &targetPowerHt40, 8, AH_TRUE);
			/* Get target powers for extension channels */
			ar5416GetTargetPowersLeg(ah,  chan, pEepData->calTargetPowerCck,
				AR5416_NUM_2G_CCK_TARGET_POWERS, &targetPowerCckExt, 4, AH_TRUE);
			ar5416GetTargetPowersLeg(ah,  chan, pEepData->calTargetPower2G,
				AR5416_NUM_2G_20_TARGET_POWERS, &targetPowerOfdmExt, 4, AH_TRUE);
		}
	} else {
		/* Setup for CTL modes */
		numCtlModes = N(ctlModesFor11a) - SUB_NUM_CTL_MODES_AT_5G_40; /* CTL_11A, CTL_5GHT20 */
		pCtlMode = ctlModesFor11a;

		ar5416GetTargetPowersLeg(ah,  chan, pEepData->calTargetPower5G,
				AR5416_NUM_5G_20_TARGET_POWERS, &targetPowerOfdm, 4, AH_FALSE);
		ar5416GetTargetPowers(ah,  chan, pEepData->calTargetPower5GHT20,
				AR5416_NUM_5G_20_TARGET_POWERS, &targetPowerHt20, 8, AH_FALSE);

		if (IS_CHAN_HT40(chan)) {
			numCtlModes = N(ctlModesFor11a); /* All 5G CTL's */

			ar5416GetTargetPowers(ah,  chan, pEepData->calTargetPower5GHT40,
				AR5416_NUM_5G_40_TARGET_POWERS, &targetPowerHt40, 8, AH_TRUE);
			ar5416GetTargetPowersLeg(ah,  chan, pEepData->calTargetPower5G,
				AR5416_NUM_5G_20_TARGET_POWERS, &targetPowerOfdmExt, 4, AH_TRUE);
		}
	}

	/*
	 * For MIMO, need to apply regulatory caps individually across dynamically
	 * running modes: CCK, OFDM, HT20, HT40
	 *
	 * The outer loop walks through each possible applicable runtime mode.
	 * The inner loop walks through each ctlIndex entry in EEPROM.
	 * The ctl value is encoded as [7:4] == test group, [3:0] == test mode.
	 *
	 */
	for (ctlMode = 0; ctlMode < numCtlModes; ctlMode++) {

		HAL_BOOL isHt40CtlMode = (pCtlMode[ctlMode] == CTL_5GHT40) ||
								 (pCtlMode[ctlMode] == CTL_2GHT40);
		if (isHt40CtlMode) {
			freq = centers.ctl_center;
		} else if (pCtlMode[ctlMode] & EXT_ADDITIVE) {
			freq = centers.ext_center;
		} else {
			freq = centers.ctl_center;
		}

		/* walk through each CTL index stored in EEPROM */
		for (i = 0; (i < AR5416_NUM_CTLS) && pEepData->ctlIndex[i]; i++) {
			u_int16_t twiceMinEdgePower;

			/* compare test group from regulatory channel list with test mode from pCtlMode list */
			if ((((cfgCtl & ~CTL_MODE_M) | (pCtlMode[ctlMode] & CTL_MODE_M)) == pEepData->ctlIndex[i]) ||
				(((cfgCtl & ~CTL_MODE_M) | (pCtlMode[ctlMode] & CTL_MODE_M)) == 
				 ((pEepData->ctlIndex[i] & CTL_MODE_M) | SD_NO_CTL))) {
				rep = &(pEepData->ctlData[i]);
				twiceMinEdgePower = ar5416GetMaxEdgePower(freq,
							rep->ctlEdges[owl_get_ntxchains(AH5416(ah)->ah_tx_chainmask) - 1],
							IS_CHAN_2GHZ(chan));
				if ((cfgCtl & ~CTL_MODE_M) == SD_NO_CTL) {
					/* Find the minimum of all CTL edge powers that apply to this channel */
					twiceMaxEdgePower = AH_MIN(twiceMaxEdgePower, twiceMinEdgePower);
				} else {
					/* specific */
					twiceMaxEdgePower = twiceMinEdgePower;
					break;
				}
			}
		}
		minCtlPower = (u_int8_t)AH_MIN(twiceMaxEdgePower, scaledPower);
		/* Apply ctl mode to correct target power set */
		switch(pCtlMode[ctlMode]) {
		case CTL_11B:
			for (i = 0; i < N(targetPowerCck.tPow2x); i++) {
				targetPowerCck.tPow2x[i] = (u_int8_t)AH_MIN(targetPowerCck.tPow2x[i], minCtlPower);
			}
			break;
		case CTL_11A:
		case CTL_11G:
			for (i = 0; i < N(targetPowerOfdm.tPow2x); i++) {
				targetPowerOfdm.tPow2x[i] = (u_int8_t)AH_MIN(targetPowerOfdm.tPow2x[i], minCtlPower);
			}
			break;
		case CTL_5GHT20:
		case CTL_2GHT20:
			for (i = 0; i < N(targetPowerHt20.tPow2x); i++) {
				targetPowerHt20.tPow2x[i] = (u_int8_t)AH_MIN(targetPowerHt20.tPow2x[i], minCtlPower);
			}
			break;
		case CTL_11B_EXT:
			targetPowerCckExt.tPow2x[0] = (u_int8_t)AH_MIN(targetPowerCckExt.tPow2x[0], minCtlPower);
			break;
		case CTL_11A_EXT:
		case CTL_11G_EXT:
			targetPowerOfdmExt.tPow2x[0] = (u_int8_t)AH_MIN(targetPowerOfdmExt.tPow2x[0], minCtlPower);
			break;
		case CTL_5GHT40:
		case CTL_2GHT40:
			for (i = 0; i < N(targetPowerHt40.tPow2x); i++) {
				targetPowerHt40.tPow2x[i] = (u_int8_t)AH_MIN(targetPowerHt40.tPow2x[i], minCtlPower);
			}
			break;
		default:
			return AH_FALSE;
			break;
		}
	} /* end ctl mode checking */

	/* Set rates Array from collected data */
	ratesArray[rate6mb] = ratesArray[rate9mb] = ratesArray[rate12mb] = ratesArray[rate18mb] = ratesArray[rate24mb] = targetPowerOfdm.tPow2x[0];
	ratesArray[rate36mb] = targetPowerOfdm.tPow2x[1];
	ratesArray[rate48mb] = targetPowerOfdm.tPow2x[2];
	ratesArray[rate54mb] = targetPowerOfdm.tPow2x[3];
	ratesArray[rateXr] = targetPowerOfdm.tPow2x[0];

	for (i = 0; i < N(targetPowerHt20.tPow2x); i++) {
		ratesArray[rateHt20_0 + i] = targetPowerHt20.tPow2x[i];
	}

	if (IS_CHAN_2GHZ(chan)) {
		ratesArray[rate1l]  = targetPowerCck.tPow2x[0];
		ratesArray[rate2s] = ratesArray[rate2l]  = targetPowerCck.tPow2x[1];
		ratesArray[rate5_5s] = ratesArray[rate5_5l] = targetPowerCck.tPow2x[2];
		ratesArray[rate11s] = ratesArray[rate11l] = targetPowerCck.tPow2x[3];
	}
	if (IS_CHAN_HT40(chan)) {
		for (i = 0; i < N(targetPowerHt40.tPow2x); i++) {
			ratesArray[rateHt40_0 + i] = targetPowerHt40.tPow2x[i];
		}
		ratesArray[rateDupOfdm] = targetPowerHt40.tPow2x[0];
		ratesArray[rateDupCck]  = targetPowerHt40.tPow2x[0];
		ratesArray[rateExtOfdm] = targetPowerOfdmExt.tPow2x[0];
		if (IS_CHAN_2GHZ(chan)) {
			ratesArray[rateExtCck]  = targetPowerCckExt.tPow2x[0];
		}
	}
	return AH_TRUE;
#undef EXT_ADDITIVE
#undef CTL_11A_EXT
#undef CTL_11G_EXT
#undef CTL_11B_EXT
#undef SUB_NUM_CTL_MODES_AT_5G_40
#undef SUB_NUM_CTL_MODES_AT_2G_40
#undef N
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
 * ar5416GetMaxEdgePower
 *
 * Find the maximum conformance test limit for the given channel and CTL info
 */
static u_int16_t
ar5416GetMaxEdgePower(u_int16_t freq, CAL_CTL_EDGES *pRdEdgesPower, HAL_BOOL is2GHz)
{
    u_int16_t twiceMaxEdgePower = AR5416_MAX_RATE_POWER;
    int      i;

    /* Get the edge power */
    for (i = 0; (i < AR5416_NUM_BAND_EDGES) && (pRdEdgesPower[i].bChannel != AR5416_BCHAN_UNUSED) ; i++) {
        /*
         * If there's an exact channel match or an inband flag set
         * on the lower channel use the given rdEdgePower
         */
        if (freq == fbin2freq(pRdEdgesPower[i].bChannel, is2GHz)) {
            twiceMaxEdgePower = pRdEdgesPower[i].tPower;
            break;
        } else if ((i > 0) && (freq < fbin2freq(pRdEdgesPower[i].bChannel, is2GHz))) {
            if (fbin2freq(pRdEdgesPower[i - 1].bChannel, is2GHz) < freq && pRdEdgesPower[i - 1].flag) {
                twiceMaxEdgePower = pRdEdgesPower[i - 1].tPower;
            }
            /* Leave loop - no more affecting edges possible in this monotonic increasing list */
            break;
        }
    }
    HALASSERT(twiceMaxEdgePower > 0);
    return twiceMaxEdgePower;
}

/**************************************************************
 * ar5416GetTargetPowers
 *
 * Return the rates of target power for the given target power table
 * channel, and number of channels
 */
static void
ar5416GetTargetPowers(struct ath_hal *ah,  HAL_CHANNEL_INTERNAL *chan,
                      CAL_TARGET_POWER_HT *powInfo, u_int16_t numChannels,
                      CAL_TARGET_POWER_HT *pNewPower, u_int16_t numRates,
                      HAL_BOOL isHt40Target)
{
    u_int16_t clo, chi;
    int i;
    int matchIndex = -1, lowIndex = -1;
    u_int16_t freq;
    CHAN_CENTERS centers;

    ar5416GetChannelCenters(ah,  chan, &centers);
    freq = isHt40Target ? centers.synth_center : centers.ctl_center;

    /* Copy the target powers into the temp channel list */
    if (freq <= fbin2freq(powInfo[0].bChannel, IS_CHAN_2GHZ(chan))) {
        matchIndex = 0;
    } else {
        for (i = 0; (i < numChannels) && (powInfo[i].bChannel != AR5416_BCHAN_UNUSED); i++) {
            if (freq == fbin2freq(powInfo[i].bChannel, IS_CHAN_2GHZ(chan))) {
                matchIndex = i;
                break;
            } else if ((freq < fbin2freq(powInfo[i].bChannel, IS_CHAN_2GHZ(chan))) &&
                       (freq > fbin2freq(powInfo[i - 1].bChannel, IS_CHAN_2GHZ(chan))))
            {
                lowIndex = i - 1;
                break;
            }
        }
        if ((matchIndex == -1) && (lowIndex == -1)) {
            HALASSERT(freq > fbin2freq(powInfo[i - 1].bChannel, IS_CHAN_2GHZ(chan)));
            matchIndex = i - 1;
        }
    }

    if (matchIndex != -1) {
        OS_MEMCPY(pNewPower, &powInfo[matchIndex], sizeof(*pNewPower));
    } else {
        HALASSERT(lowIndex != -1);
        /*
         * Get the lower and upper channels, target powers,
         * and interpolate between them.
         */
        clo = fbin2freq(powInfo[lowIndex].bChannel, IS_CHAN_2GHZ(chan));
        chi = fbin2freq(powInfo[lowIndex + 1].bChannel, IS_CHAN_2GHZ(chan));

        for (i = 0; i < numRates; i++) {
            pNewPower->tPow2x[i] = (u_int8_t)interpolate_signed(freq, clo, chi,
                                   powInfo[lowIndex].tPow2x[i], powInfo[lowIndex + 1].tPow2x[i]);
        }
    }
}
/**************************************************************
 * ar5416GetTargetPowersLeg
 *
 * Return the four rates of target power for the given target power table
 * channel, and number of channels
 */
static void
ar5416GetTargetPowersLeg(struct ath_hal *ah, 
                         HAL_CHANNEL_INTERNAL *chan,
                         CAL_TARGET_POWER_LEG *powInfo, u_int16_t numChannels,
                         CAL_TARGET_POWER_LEG *pNewPower, u_int16_t numRates,
			 HAL_BOOL isExtTarget)
{
    u_int16_t clo, chi;
    int i;
    int matchIndex = -1, lowIndex = -1;
    u_int16_t freq;
    CHAN_CENTERS centers;

    ar5416GetChannelCenters(ah,  chan, &centers);
    freq = (isExtTarget) ? centers.ext_center :centers.ctl_center;

    /* Copy the target powers into the temp channel list */
    if (freq <= fbin2freq(powInfo[0].bChannel, IS_CHAN_2GHZ(chan))) {
        matchIndex = 0;
    } else {
        for (i = 0; (i < numChannels) && (powInfo[i].bChannel != AR5416_BCHAN_UNUSED); i++) {
            if (freq == fbin2freq(powInfo[i].bChannel, IS_CHAN_2GHZ(chan))) {
                matchIndex = i;
                break;
            } else if ((freq < fbin2freq(powInfo[i].bChannel, IS_CHAN_2GHZ(chan))) &&
                       (freq > fbin2freq(powInfo[i - 1].bChannel, IS_CHAN_2GHZ(chan))))
            {
                lowIndex = i - 1;
                break;
            }
        }
        if ((matchIndex == -1) && (lowIndex == -1)) {
            HALASSERT(freq > fbin2freq(powInfo[i - 1].bChannel, IS_CHAN_2GHZ(chan)));
            matchIndex = i - 1;
        }
    }

    if (matchIndex != -1) {
		OS_MEMCPY(pNewPower, &powInfo[matchIndex], sizeof(*pNewPower));
    } else {
        HALASSERT(lowIndex != -1);
        /*
         * Get the lower and upper channels, target powers,
         * and interpolate between them.
         */
        clo = fbin2freq(powInfo[lowIndex].bChannel, IS_CHAN_2GHZ(chan));
        chi = fbin2freq(powInfo[lowIndex + 1].bChannel, IS_CHAN_2GHZ(chan));

        for (i = 0; i < numRates; i++) {
            pNewPower->tPow2x[i] = (u_int8_t)interpolate_signed(freq, clo, chi,
                                   powInfo[lowIndex].tPow2x[i], powInfo[lowIndex + 1].tPow2x[i]);
        }
    }
}

/**************************************************************
 * ar5416SetPowerCalTable
 *
 * Pull the PDADC piers from cal data and interpolate them across the given
 * points as well as from the nearest pier(s) to get a power detector
 * linear voltage to power level table.
 */
static HAL_BOOL
ar5416SetPowerCalTable(struct ath_hal *ah, struct ar5416eeprom *pEepData, HAL_CHANNEL_INTERNAL *chan, int16_t *pTxPowerIndexOffset)
{
    CAL_DATA_PER_FREQ *pRawDataset;
    u_int8_t  *pCalBChans = AH_NULL;
    u_int16_t pdGainOverlap_t2;
    static u_int8_t  pdadcValues[AR5416_NUM_PDADC_VALUES];
    u_int16_t gainBoundaries[AR5416_PD_GAINS_IN_MASK];
    u_int16_t numPiers, i, j;
    int16_t  tMinCalPower;
    u_int16_t numXpdGain, xpdMask;
    u_int16_t xpdGainValues[AR5416_NUM_PD_GAINS];
    u_int32_t reg32, regOffset, regChainOffset;

    ath_hal_memzero(xpdGainValues, sizeof(xpdGainValues));
    
    xpdMask = pEepData->modalHeader[IS_CHAN_2GHZ(chan)].xpdGain;

    if (IS_EEP_MINOR_V2(ah)) {
        pdGainOverlap_t2 = pEepData->modalHeader[IS_CHAN_2GHZ(chan)].pdGainOverlap;
    } else { 
    	pdGainOverlap_t2 = (u_int16_t)(MS(OS_REG_READ(ah, AR_PHY_TPCRG5), AR_PHY_TPCRG5_PD_GAIN_OVERLAP));
    }

    if (IS_CHAN_2GHZ(chan)) {
        pCalBChans = pEepData->calFreqPier2G;
        numPiers = AR5416_NUM_2G_CAL_PIERS;
    } else {
        pCalBChans = pEepData->calFreqPier5G;
        numPiers = AR5416_NUM_5G_CAL_PIERS;
    }

    numXpdGain = 0;
    /* Calculate the value of xpdgains from the xpdGain Mask */
    for (i = 1; i <= AR5416_PD_GAINS_IN_MASK; i++) {
        if ((xpdMask >> (AR5416_PD_GAINS_IN_MASK - i)) & 1) {
            if (numXpdGain >= AR5416_NUM_PD_GAINS) {
                HALASSERT(0);
                break;
            }
            xpdGainValues[numXpdGain] = (u_int16_t)(AR5416_PD_GAINS_IN_MASK - i);
            numXpdGain++;
        }
    }
    
    /* Write the detector gain biases and their number */
    OS_REG_WRITE(ah, AR_PHY_TPCRG1, (OS_REG_READ(ah, AR_PHY_TPCRG1) & 
    	~(AR_PHY_TPCRG1_NUM_PD_GAIN | AR_PHY_TPCRG1_PD_GAIN_1 | AR_PHY_TPCRG1_PD_GAIN_2 | AR_PHY_TPCRG1_PD_GAIN_3)) | 
	SM(numXpdGain - 1, AR_PHY_TPCRG1_NUM_PD_GAIN) | SM(xpdGainValues[0], AR_PHY_TPCRG1_PD_GAIN_1 ) |
	SM(xpdGainValues[1], AR_PHY_TPCRG1_PD_GAIN_2) | SM(xpdGainValues[2],  AR_PHY_TPCRG1_PD_GAIN_3));

    for (i = 0; i < AR5416_MAX_CHAINS; i++) {

            if ( !(IS_5416V1(ah) ) && 
            ( AH5416(ah)->ah_rx_chainmask == 0x5 || AH5416(ah)->ah_tx_chainmask == 0x5) && (i != 0)) {
            /* Regs are swapped from chain 2 to 1 for 5416 2_0 with 
             * only chains 0 and 2 populated 
             */
            regChainOffset = (i == 1) ? 0x2000 : 0x1000;
        } else {
            regChainOffset = i * 0x1000;
        }

        if (pEepData->baseEepHeader.txMask & (1 << i)) {
            if (IS_CHAN_2GHZ(chan)) {
                pRawDataset = pEepData->calPierData2G[i];
            } else {
                pRawDataset = pEepData->calPierData5G[i];
            }

            ar5416GetGainBoundariesAndPdadcs(ah,  chan, pRawDataset,
                                             pCalBChans, numPiers,
                                             pdGainOverlap_t2,
                                             &tMinCalPower, gainBoundaries,
                                             pdadcValues, numXpdGain);

            if ((i == 0) || !(IS_5416V1(ah))) {
                /*
                 * Note the pdadc table may not start at 0 dBm power, could be
                 * negative or greater than 0.  Need to offset the power
                 * values by the amount of minPower for griffin
                 */

                OS_REG_WRITE(ah, AR_PHY_TPCRG5 + regChainOffset,
                     SM(pdGainOverlap_t2, AR_PHY_TPCRG5_PD_GAIN_OVERLAP) |
                     SM(gainBoundaries[0], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_1)  |
                     SM(gainBoundaries[1], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_2)  |
                     SM(gainBoundaries[2], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_3)  |
                     SM(gainBoundaries[3], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_4));
            }

            /* Write the power values into the baseband power table */
            regOffset = AR_PHY_BASE + (672 << 2) + regChainOffset;

            for (j = 0; j < 32; j++) {
                reg32 = ((pdadcValues[4*j + 0] & 0xFF) << 0)  |
                    ((pdadcValues[4*j + 1] & 0xFF) << 8)  |
                    ((pdadcValues[4*j + 2] & 0xFF) << 16) |
                    ((pdadcValues[4*j + 3] & 0xFF) << 24) ;
                OS_REG_WRITE(ah, regOffset, reg32);

#ifdef PDADC_DUMP
		HALDEBUG(ah, "PDADC: Chain %d | PDADC %3d Value %3d | PDADC %3d Value %3d | PDADC %3d Value %3d | PDADC %3d Value %3d |\n",
			       i,
			       4*j, pdadcValues[4*j],
			       4*j+1, pdadcValues[4*j + 1],
			       4*j+2, pdadcValues[4*j + 2],
			       4*j+3, pdadcValues[4*j + 3]);
#endif
                regOffset += 4;
            }
        }
    }
    *pTxPowerIndexOffset = 0;

    return AH_TRUE;
}

/**************************************************************
 * ar5416GetGainBoundariesAndPdadcs
 *
 * Uses the data points read from EEPROM to reconstruct the pdadc power table
 * Called by ar5416SetPowerCalTable only.
 */
static void
ar5416GetGainBoundariesAndPdadcs(struct ath_hal *ah, 
                                 HAL_CHANNEL_INTERNAL *chan, CAL_DATA_PER_FREQ *pRawDataSet,
                                 u_int8_t * bChans,  u_int16_t availPiers,
                                 u_int16_t tPdGainOverlap, int16_t *pMinCalPower, u_int16_t * pPdGainBoundaries,
                                 u_int8_t * pPDADCValues, u_int16_t numXpdGains)
{

    int       i, j, k;
    int16_t   ss;         /* potentially -ve index for taking care of pdGainOverlap */
    u_int16_t  idxL, idxR, numPiers; /* Pier indexes */

    /* filled out Vpd table for all pdGains (chanL) */
    static u_int8_t   vpdTableL[AR5416_NUM_PD_GAINS][AR5416_MAX_PWR_RANGE_IN_HALF_DB];

    /* filled out Vpd table for all pdGains (chanR) */
    static u_int8_t   vpdTableR[AR5416_NUM_PD_GAINS][AR5416_MAX_PWR_RANGE_IN_HALF_DB];

    /* filled out Vpd table for all pdGains (interpolated) */
    static u_int8_t   vpdTableI[AR5416_NUM_PD_GAINS][AR5416_MAX_PWR_RANGE_IN_HALF_DB];

    u_int8_t   *pVpdL, *pVpdR, *pPwrL, *pPwrR;
    u_int8_t   minPwrT4[AR5416_NUM_PD_GAINS];
    u_int8_t   maxPwrT4[AR5416_NUM_PD_GAINS];
    int16_t   vpdStep;
    int16_t   tmpVal;
    u_int16_t  sizeCurrVpdTable, maxIndex, tgtIndex;
    HAL_BOOL    match;
    int16_t  minDelta = 0;
    CHAN_CENTERS centers;

    ar5416GetChannelCenters(ah, chan, &centers);

    /* Trim numPiers for the number of populated channel Piers */
    for (numPiers = 0; numPiers < availPiers; numPiers++) {
        if (bChans[numPiers] == AR5416_BCHAN_UNUSED) {
            break;
        }
    }

    /* Find pier indexes around the current channel */
    match = getLowerUpperIndex((u_int8_t)FREQ2FBIN(centers.synth_center, IS_CHAN_2GHZ(chan)),
			bChans, numPiers, &idxL, &idxR);

    if (match) {
        /* Directly fill both vpd tables from the matching index */
        for (i = 0; i < numXpdGains; i++) {
            minPwrT4[i] = pRawDataSet[idxL].pwrPdg[i][0];
            maxPwrT4[i] = pRawDataSet[idxL].pwrPdg[i][4];
            ar5416FillVpdTable(minPwrT4[i], maxPwrT4[i], pRawDataSet[idxL].pwrPdg[i],
                               pRawDataSet[idxL].vpdPdg[i], AR5416_PD_GAIN_ICEPTS, vpdTableI[i]);
        }
    } else {
        for (i = 0; i < numXpdGains; i++) {
            pVpdL = pRawDataSet[idxL].vpdPdg[i];
            pPwrL = pRawDataSet[idxL].pwrPdg[i];
            pVpdR = pRawDataSet[idxR].vpdPdg[i];
            pPwrR = pRawDataSet[idxR].pwrPdg[i];

            /* Start Vpd interpolation from the max of the minimum powers */
            minPwrT4[i] = AH_MAX(pPwrL[0], pPwrR[0]);

            /* End Vpd interpolation from the min of the max powers */
            maxPwrT4[i] = AH_MIN(pPwrL[AR5416_PD_GAIN_ICEPTS - 1], pPwrR[AR5416_PD_GAIN_ICEPTS - 1]);
            HALASSERT(maxPwrT4[i] > minPwrT4[i]);

            /* Fill pier Vpds */
            ar5416FillVpdTable(minPwrT4[i], maxPwrT4[i], pPwrL, pVpdL, AR5416_PD_GAIN_ICEPTS, vpdTableL[i]);
            ar5416FillVpdTable(minPwrT4[i], maxPwrT4[i], pPwrR, pVpdR, AR5416_PD_GAIN_ICEPTS, vpdTableR[i]);

            /* Interpolate the final vpd */
            for (j = 0; j <= (maxPwrT4[i] - minPwrT4[i]) / 2; j++) {
                vpdTableI[i][j] = (u_int8_t)(interpolate_signed((u_int16_t)FREQ2FBIN(centers.synth_center, IS_CHAN_2GHZ(chan)),
                    bChans[idxL], bChans[idxR], vpdTableL[i][j], vpdTableR[i][j]));
            }
        }
    }
    *pMinCalPower = (int16_t)(minPwrT4[0] / 2);

    k = 0; /* index for the final table */
    for (i = 0; i < numXpdGains; i++) {
        if (i == (numXpdGains - 1)) {
            pPdGainBoundaries[i] = (u_int16_t)(maxPwrT4[i] / 2);
        } else {
            pPdGainBoundaries[i] = (u_int16_t)((maxPwrT4[i] + minPwrT4[i+1]) / 4);
        }

        pPdGainBoundaries[i] = (u_int16_t)AH_MIN(AR5416_MAX_RATE_POWER, pPdGainBoundaries[i]);

	/*
         * WORKAROUND for OWL 1.0 until we get a per chain gain boundary register.
         * this is not the best solution
	 */
        if ((i == 0) && IS_5416V1(ah) ) {
	    /*
             * fix the gain delta, but get a delta that can be applied to min to
             * keep the upper power values accurate, don't think max needs to
             * be adjusted because should not be at that area of the table?
	     */
            minDelta = pPdGainBoundaries[0] - 23;
            pPdGainBoundaries[0] = 23;
        }
        else {
            minDelta = 0;
        }

        /* Find starting index for this pdGain */
        if (i == 0) {
            if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
                ss = (int16_t)(0 - (minPwrT4[i] / 2));
            } else {
                ss = 0; /* for the first pdGain, start from index 0 */
            }
        } else {
	    /* need overlap entries extrapolated below. */
            ss = (int16_t)((pPdGainBoundaries[i-1] - (minPwrT4[i] / 2)) - tPdGainOverlap + 1 + minDelta);
        }
        vpdStep = (int16_t)(vpdTableI[i][1] - vpdTableI[i][0]);
        vpdStep = (int16_t)((vpdStep < 1) ? 1 : vpdStep);
        /*
         *-ve ss indicates need to extrapolate data below for this pdGain
         */
        while ((ss < 0) && (k < (AR5416_NUM_PDADC_VALUES - 1))) {
            tmpVal = (int16_t)(vpdTableI[i][0] + ss * vpdStep);
            pPDADCValues[k++] = (u_int8_t)((tmpVal < 0) ? 0 : tmpVal);
            ss++;
        }

        sizeCurrVpdTable = (u_int8_t)((maxPwrT4[i] - minPwrT4[i]) / 2 +1);
        tgtIndex = (u_int8_t)(pPdGainBoundaries[i] + tPdGainOverlap - (minPwrT4[i] / 2));
        maxIndex = (tgtIndex < sizeCurrVpdTable) ? tgtIndex : sizeCurrVpdTable;

        while ((ss < maxIndex) && (k < (AR5416_NUM_PDADC_VALUES - 1))) {
            pPDADCValues[k++] = vpdTableI[i][ss++];
        }

        vpdStep = (int16_t)(vpdTableI[i][sizeCurrVpdTable - 1] - vpdTableI[i][sizeCurrVpdTable - 2]);
        vpdStep = (int16_t)((vpdStep < 1) ? 1 : vpdStep);
        /*
         * for last gain, pdGainBoundary == Pmax_t2, so will
         * have to extrapolate
         */
        if (tgtIndex > maxIndex) {  /* need to extrapolate above */
            while ((ss <= tgtIndex) && (k < (AR5416_NUM_PDADC_VALUES - 1))) {
                tmpVal = (int16_t)((vpdTableI[i][sizeCurrVpdTable - 1] +
                          (ss - maxIndex +1) * vpdStep));
                pPDADCValues[k++] = (u_int8_t)((tmpVal > 255) ? 255 : tmpVal);
                ss++;
            }
        }               /* extrapolated above */
    }                   /* for all pdGainUsed */

    /* Fill out pdGainBoundaries - only up to 2 allowed here, but hardware allows up to 4 */
    while (i < AR5416_PD_GAINS_IN_MASK) {
        pPdGainBoundaries[i] = pPdGainBoundaries[i-1];
        i++;
    }

    while (k < AR5416_NUM_PDADC_VALUES) {
        pPDADCValues[k] = pPDADCValues[k-1];
        k++;
    }
    return;
}

/**************************************************************
 * getLowerUppderIndex
 *
 * Return indices surrounding the value in sorted integer lists.
 * Requirement: the input list must be monotonically increasing
 *     and populated up to the list size
 * Returns: match is set if an index in the array matches exactly
 *     or a the target is before or after the range of the array.
 */
HAL_BOOL
getLowerUpperIndex(u_int8_t target, u_int8_t *pList, u_int16_t listSize,
                   u_int16_t *indexL, u_int16_t *indexR)
{
    u_int16_t i;

    /*
     * Check first and last elements for beyond ordered array cases.
     */
    if (target <= pList[0]) {
        *indexL = *indexR = 0;
        return AH_TRUE;
    }
    if (target >= pList[listSize-1]) {
        *indexL = *indexR = (u_int16_t)(listSize - 1);
        return AH_TRUE;
    }

    /* look for value being near or between 2 values in list */
    for (i = 0; i < listSize - 1; i++) {
        /*
         * If value is close to the current value of the list
         * then target is not between values, it is one of the values
         */
        if (pList[i] == target) {
            *indexL = *indexR = i;
            return AH_TRUE;
        }
        /*
         * Look for value being between current value and next value
         * if so return these 2 values
         */
        if (target < pList[i + 1]) {
            *indexL = i;
            *indexR = (u_int16_t)(i + 1);
            return AH_FALSE;
        }
    }
    HALASSERT(0);
    *indexL = *indexR = 0;
    return AH_FALSE;
}

/**************************************************************
 * ar5416FillVpdTable
 *
 * Fill the Vpdlist for indices Pmax-Pmin
 * Note: pwrMin, pwrMax and Vpdlist are all in dBm * 4
 */
static HAL_BOOL
ar5416FillVpdTable(u_int8_t pwrMin, u_int8_t pwrMax, u_int8_t *pPwrList,
                   u_int8_t *pVpdList, u_int16_t numIntercepts, u_int8_t *pRetVpdList)
{
    u_int16_t  i, k;
    u_int8_t   currPwr = pwrMin;
    u_int16_t  idxL, idxR;

    HALASSERT(pwrMax > pwrMin);
    for (i = 0; i <= (pwrMax - pwrMin) / 2; i++) {
        getLowerUpperIndex(currPwr, pPwrList, numIntercepts,
                           &(idxL), &(idxR));
        if (idxR < 1)
            idxR = 1;           /* extrapolate below */
        if (idxL == numIntercepts - 1)
            idxL = (u_int16_t)(numIntercepts - 2);   /* extrapolate above */
        if (pPwrList[idxL] == pPwrList[idxR])
            k = pVpdList[idxL];
        else
            k = (u_int16_t)( ((currPwr - pPwrList[idxL]) * pVpdList[idxR] + (pPwrList[idxR] - currPwr) * pVpdList[idxL]) /
                  (pPwrList[idxR] - pPwrList[idxL]) );
        HALASSERT(k < 256);
        pRetVpdList[i] = (u_int8_t)k;
        currPwr += 2;               /* half dB steps */
    }

    return AH_TRUE;
}


static void
ar5416Set11nRegs(struct ath_hal *ah, HAL_CHANNEL *chan)
{
	u_int32_t phymode;
	HAL_HT_MACMODE macmode;		/* MAC - 20/40 mode */

	if (!IS_CHAN_HT(chan))
		return;

	/* Enable 11n HT, 20 MHz */
	phymode = AR_PHY_FC_HT_EN | AR_PHY_FC_SHORT_GI_40
		| AR_PHY_FC_SINGLE_HT_LTF1 | AR_PHY_FC_WALSH;

	/* Configure baseband for dynamic 20/40 operation */
	if (IS_CHAN_HT40(chan)) {
		phymode |= AR_PHY_FC_DYN2040_EN | AR_PHY_FC_SHORT_GI_40;

		/* Configure control (primary) channel at +-10MHz */
		if ((chan->channelFlags & CHANNEL_HT40PLUS))
			phymode |= AR_PHY_FC_DYN2040_PRI_CH;
#if 0
		/* Configure 20/25 spacing */
		if (ht->ht_extprotspacing == HAL_HT_EXTPROTSPACING_25)
			phymode |= AR_PHY_FC_DYN2040_EXT_CH;
#endif
		macmode = HAL_HT_MACMODE_2040;
	} else
		macmode = HAL_HT_MACMODE_20;
	OS_REG_WRITE(ah, AR_PHY_TURBO, phymode);
#ifdef RIFS_EN
	/* RIFS support only in Owl 2.0 */
	if (!(IS_5416V1(ah)) {
		/* RIFS. reg 70: search_start_delay, in CLKs */
		if (IS_CHAN_HT40(chan))
			OS_REG_WRITE(ah, 0x9800+(70<<2), 40);
		else
			OS_REG_WRITE(ah, 0x9800+(70<<2), 20);
	}
#endif
	/* Configure MAC for 20/40 operation */
	ar5416Set11nMac2040(ah, macmode);

	/* global transmit timeout (25 TUs default)*/
	/* XXX - put this elsewhere??? */
	OS_REG_WRITE(ah, AR_GTXTO, 25 << AR_GTXTO_TIMEOUT_LIMIT_S) ;

	/* carrier sense timeout */
	OS_REG_SET_BIT(ah, AR_GTTM, AR_GTTM_CST_USEC);
	OS_REG_WRITE(ah, AR_CST, 1 << AR_CST_TIMEOUT_LIMIT_S);
}

void
ar5416GetChannelCenters(struct ath_hal *ah,
	HAL_CHANNEL_INTERNAL *chan, CHAN_CENTERS *centers)
{
	centers->ctl_center = chan->channel;
	centers->synth_center = chan->channel;
	/*
	 * In 20/40 phy mode, the center frequency is
	 * "between" the control and extension channels.
	 */
	if (chan->channelFlags & CHANNEL_HT40PLUS) {
		centers->synth_center += HT40_CHANNEL_CENTER_SHIFT;
		centers->ext_center =
		    centers->synth_center + HT40_CHANNEL_CENTER_SHIFT;
	} else if (chan->channelFlags & CHANNEL_HT40MINUS) {
		centers->synth_center -= HT40_CHANNEL_CENTER_SHIFT;
		centers->ext_center =
		    centers->synth_center - HT40_CHANNEL_CENTER_SHIFT;
	} else {
		centers->ext_center = chan->channel;
	}
}

#ifdef AH_SUPPORT_AR9000
/**************************************************************
 * ar5416EepromSetAddac
 *
 * Set the ADDAC from eeprom for Sowl.
 */
static void
ar5416EepromSetSowlAddac(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan, int *regWrites)
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
    MODAL_EEP_HEADER *pModal;
	struct ar5416eeprom *eep = &AH5416(ah)->ah_5416eeprom;
    u_int8_t biaslevel;
	u_int32_t addacBuf[N(ar5416Addac_sowl)][2];

	if (AR_SREV_SOWL_10(ah))
		ath_hal_memcpy(addacBuf, ar5416Addac_sowl1_0, sizeof(addacBuf));
	else
		ath_hal_memcpy(addacBuf, ar5416Addac_sowl, sizeof(addacBuf));

	if (!IS_EEP_MINOR_V7(ah))
		return;

    pModal = &(eep->modalHeader[IS_CHAN_2GHZ(chan)]);

    if (pModal->xpaBiasLvl != 0xff) {
        biaslevel = pModal->xpaBiasLvl;
    } else {
        /* Use freqeuncy specific xpa bias level */
        u_int16_t resetFreqBin, freqBin, freqCount=0;
        CHAN_CENTERS centers;

        ar5416GetChannelCenters(ah, chan, &centers);

        resetFreqBin = FREQ2FBIN(centers.synth_center, IS_CHAN_2GHZ(chan));
        freqBin = pModal->xpaBiasLvlFreq[0] & 0xff;
        biaslevel = (u_int8_t)(pModal->xpaBiasLvlFreq[0] >> 14);

        freqCount++;

        while (freqCount < 3) {
            if (pModal->xpaBiasLvlFreq[freqCount] == 0x0)
                break;

            freqBin = pModal->xpaBiasLvlFreq[freqCount] & 0xff;
            if (resetFreqBin >= freqBin) {
                biaslevel = (u_int8_t)(pModal->xpaBiasLvlFreq[freqCount] >> 14);
            } else {
                break;
            }
            freqCount++;
        }
    }

    /* Apply bias level to the ADDAC values in the INI array */
    if (IS_CHAN_2GHZ(chan)) {
		addacBuf[7][1] = (addacBuf[7][1] & (~0x18)) | biaslevel << 3;
    } else {
		addacBuf[6][1] = (addacBuf[6][1] & (~0xc0)) | biaslevel << 6;
    }
	REG_WRITE_ARRAY(addacBuf, 1, *regWrites);
#undef N
}

#endif


#endif /* AH_SUPPORT_AR5416 */
