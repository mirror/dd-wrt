/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5212/ar5111.c#2 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_5111

#include "ah.h"
#include "ah_internal.h"

#include "ar5212/ar5212.h"
#include "ar5212/ar5212reg.h"
#include "ar5212/ar5212phy.h"

/* Add static register initialization vectors */
#define AH_5212_5111
#include "ar5212/ar5212.ini"

#define	N(a)	(sizeof(a)/sizeof(a[0]))

/*
 * WAR for bug 6773.  OS_DELAY() does a PIO READ on the PCI bus which allows
 * other cards' DMA reads to complete in the middle of our reset.
 */
#define WAR_6773(x) do {		\
	if ((++(x) % 64) == 0)		\
		OS_DELAY(1);		\
} while (0)

#define REG_WRITE_ARRAY(regArray, column, regWr) do {                  	\
	int r;								\
	for (r = 0; r < N(regArray); r++) {				\
		OS_REG_WRITE(ah, (regArray)[r][0], (regArray)[r][(column)]);\
		WAR_6773(regWr);					\
	}								\
} while (0)

static u_int16_t ar5212GetScaledPower(u_int16_t channel, u_int16_t pcdacValue,
		PCDACS_EEPROM *pSrcStruct);
static HAL_BOOL ar5212FindValueInList(u_int16_t channel, u_int16_t pcdacValue,
		PCDACS_EEPROM *pSrcStruct, u_int16_t *powerValue);
static void ar5212GetLowerUpperPcdacs(u_int16_t pcdac, u_int16_t channel,
		PCDACS_EEPROM *pSrcStruct,
		u_int16_t *pLowerPcdac, u_int16_t *pUpperPcdac);

extern void ar5212GetLowerUpperValues(u_int16_t value,
		u_int16_t *pList, u_int16_t listSize,
		u_int16_t *pLowerValue, u_int16_t *pUpperValue);
extern	void ar5212ModifyRfBuffer(u_int32_t *rfBuf, u_int32_t reg32,
		u_int32_t numBits, u_int32_t firstBit, u_int32_t column);

/* const globals for export to the attach */
typedef struct {
	u_int32_t Bank0Data[N(ar5212Bank0_5111)];
	u_int32_t Bank1Data[N(ar5212Bank1_5111)];
	u_int32_t Bank2Data[N(ar5212Bank2_5111)];
	u_int32_t Bank3Data[N(ar5212Bank3_5111)];
	u_int32_t Bank6Data[N(ar5212Bank6_5111)];
	u_int32_t Bank7Data[N(ar5212Bank7_5111)];
} AR5212_RF_BANKS_5111;

static void
ar5111WriteRegs(struct ath_hal *ah, u_int modesIndex, u_int freqIndex, int regWrites)
{
	REG_WRITE_ARRAY(ar5212Modes_5111, modesIndex, regWrites);
	REG_WRITE_ARRAY(ar5212Common_5111, 1, regWrites);
	REG_WRITE_ARRAY(ar5212BB_RfGain_5111, freqIndex, regWrites);
}

/*
 * Take the MHz channel value and set the Channel value
 *
 * ASSUMES: Writes enabled to analog bus
 */
static HAL_BOOL
ar5111SetChannel(struct ath_hal *ah,  HAL_CHANNEL_INTERNAL *chan)
{
#define CI_2GHZ_INDEX_CORRECTION 19
	u_int32_t refClk, reg32, data2111;
	int16_t chan5111, chanIEEE;

	/*
	 * Structure to hold 11b tuning information for 5111/2111
	 * 16 MHz mode, divider ratio = 198 = NP+S. N=16, S=4 or 6, P=12
	 */
	typedef struct {
		u_int32_t	refClkSel;	/* reference clock, 1 for 16 MHz */
		u_int32_t	channelSelect;	/* P[7:4]S[3:0] bits */
		u_int16_t	channel5111;	/* 11a channel for 5111 */
	} CHAN_INFO_2GHZ;

	const static CHAN_INFO_2GHZ chan2GHzData[] = {
		{ 1, 0x46, 96  },	/* 2312 -19 */
		{ 1, 0x46, 97  },	/* 2317 -18 */
		{ 1, 0x46, 98  },	/* 2322 -17 */
		{ 1, 0x46, 99  },	/* 2327 -16 */
		{ 1, 0x46, 100 },	/* 2332 -15 */
		{ 1, 0x46, 101 },	/* 2337 -14 */
		{ 1, 0x46, 102 },	/* 2342 -13 */
		{ 1, 0x46, 103 },	/* 2347 -12 */
		{ 1, 0x46, 104 },	/* 2352 -11 */
		{ 1, 0x46, 105 },	/* 2357 -10 */
		{ 1, 0x46, 106 },	/* 2362  -9 */
		{ 1, 0x46, 107 },	/* 2367  -8 */
		{ 1, 0x46, 108 },	/* 2372  -7 */
		/* index -6 to 0 are pad to make this a nolookup table */
		{ 1, 0x46, 116 },	/*       -6 */
		{ 1, 0x46, 116 },	/*       -5 */
		{ 1, 0x46, 116 },	/*       -4 */
		{ 1, 0x46, 116 },	/*       -3 */
		{ 1, 0x46, 116 },	/*       -2 */
		{ 1, 0x46, 116 },	/*       -1 */
		{ 1, 0x46, 116 },	/*        0 */
		{ 1, 0x46, 116 },	/* 2412   1 */
		{ 1, 0x46, 117 },	/* 2417   2 */
		{ 1, 0x46, 118 },	/* 2422   3 */
		{ 1, 0x46, 119 },	/* 2427   4 */
		{ 1, 0x46, 120 },	/* 2432   5 */
		{ 1, 0x46, 121 },	/* 2437   6 */
		{ 1, 0x46, 122 },	/* 2442   7 */
		{ 1, 0x46, 123 },	/* 2447   8 */
		{ 1, 0x46, 124 },	/* 2452   9 */
		{ 1, 0x46, 125 },	/* 2457  10 */
		{ 1, 0x46, 126 },	/* 2462  11 */
		{ 1, 0x46, 127 },	/* 2467  12 */
		{ 1, 0x46, 128 },	/* 2472  13 */
		{ 1, 0x44, 124 },	/* 2484  14 */
		{ 1, 0x46, 136 },	/* 2512  15 */
		{ 1, 0x46, 140 },	/* 2532  16 */
		{ 1, 0x46, 144 },	/* 2552  17 */
		{ 1, 0x46, 148 },	/* 2572  18 */
		{ 1, 0x46, 152 },	/* 2592  19 */
		{ 1, 0x46, 156 },	/* 2612  20 */
		{ 1, 0x46, 160 },	/* 2632  21 */
		{ 1, 0x46, 164 },	/* 2652  22 */
		{ 1, 0x46, 168 },	/* 2672  23 */
		{ 1, 0x46, 172 },	/* 2692  24 */
		{ 1, 0x46, 176 },	/* 2712  25 */
		{ 1, 0x46, 180 } 	/* 2732  26 */
	};

	OS_MARK(ah, AH_MARK_SETCHANNEL, chan->channel);

	chanIEEE = ath_hal_mhz2ieee(ah, chan->channel, chan->channelFlags);
	if (IS_CHAN_2GHZ(chan)) {
		const CHAN_INFO_2GHZ* ci =
			&chan2GHzData[chanIEEE + CI_2GHZ_INDEX_CORRECTION];
		u_int32_t txctl;

		data2111 = ((ath_hal_reverseBits(ci->channelSelect, 8) & 0xff)
				<< 5)
			 | (ci->refClkSel << 4);
		chan5111 = ci->channel5111;
		txctl = OS_REG_READ(ah, AR_PHY_CCK_TX_CTRL);
		if (chan->channel == 2484) {
			/* Enable channel spreading for channel 14 */
			OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
				txctl | AR_PHY_CCK_TX_CTRL_JAPAN);
		} else {
			OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
				txctl &~ AR_PHY_CCK_TX_CTRL_JAPAN);
		}
	} else {
		chan5111 = chanIEEE;	/* no conversion needed */
		data2111 = 0;
	}

	/* Rest of the code is common for 5 GHz and 2.4 GHz. */
	if (chan5111 >= 145 || (chan5111 & 0x1)) {
		reg32  = ath_hal_reverseBits(chan5111 - 24, 8) & 0xff;
		refClk = 1;
	} else {
		reg32  = ath_hal_reverseBits(((chan5111 - 24)/2), 8) & 0xff;
		refClk = 0;
	}

	reg32 = (reg32 << 2) | (refClk << 1) | (1 << 10) | 0x1;
	OS_REG_WRITE(ah, AR_PHY(0x27), ((data2111 & 0xff) << 8) | (reg32 & 0xff));
	reg32 >>= 8;
	OS_REG_WRITE(ah, AR_PHY(0x34), (data2111 & 0xff00) | (reg32 & 0xff));

	AH_PRIVATE(ah)->ah_curchan = chan;
	return AH_TRUE;
#undef CI_2GHZ_INDEX_CORRECTION
}

/*
 * Return a reference to the requested RF Bank.
 */
static u_int32_t *
ar5111GetRfBank(struct ath_hal *ah, int bank)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	AR5212_RF_BANKS_5111 *pRfBank5111 = ahp->ah_analogBanks;

	HALASSERT(ahp->ah_analogBanks != AH_NULL);
	switch (bank) {
	case 0: return pRfBank5111->Bank0Data;
	case 1: return pRfBank5111->Bank1Data;
	case 2: return pRfBank5111->Bank2Data;
	case 3: return pRfBank5111->Bank3Data;
	case 6: return pRfBank5111->Bank6Data;
	case 7: return pRfBank5111->Bank7Data;
	}
	HALDEBUG(ah, "%s: unknown RF Bank %d requested\n", __func__, bank);
	return AH_NULL;
}

/*
 * Reads EEPROM header info from device structure and programs
 * all rf registers
 *
 * REQUIRES: Access to the analog rf device
 */
static HAL_BOOL
ar5111SetRfRegs(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan,
	u_int16_t modesIndex, u_int16_t *rfXpdGain)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	u_int16_t rfXpdGainFixed, rfPloSel, rfPwdXpd, gainI;
	u_int16_t tempOB, tempDB;
	u_int32_t ob2GHz, db2GHz, rfReg[N(ar5212Bank6_5111)];
	int i, regWrites = 0;

	/* Setup rf parameters */
	switch (chan->channelFlags & CHANNEL_ALL) {
	case CHANNEL_A:
	case CHANNEL_T:
		if (4000 < chan->channel && chan->channel < 5260) {
			tempOB = ahp->ah_ob1;
			tempDB = ahp->ah_db1;
		} else if (5260 <= chan->channel && chan->channel < 5500) {
			tempOB = ahp->ah_ob2;
			tempDB = ahp->ah_db2;
		} else if (5500 <= chan->channel && chan->channel < 5725) {
			tempOB = ahp->ah_ob3;
			tempDB = ahp->ah_db3;
		} else if (chan->channel >= 5725) {
			tempOB = ahp->ah_ob4;
			tempDB = ahp->ah_db4;
		} else {
			/* XXX when does this happen??? */
			tempOB = tempDB = 0;
		}
		ob2GHz = db2GHz = 0;

		rfXpdGainFixed = ahp->ah_xgain[headerInfo11A];
		rfPloSel = ahp->ah_xpd[headerInfo11A];
		rfPwdXpd = !ahp->ah_xpd[headerInfo11A];
		gainI = ahp->ah_gainI[headerInfo11A];
		break;
	case CHANNEL_B:
		tempOB = ahp->ah_obFor24;
		tempDB = ahp->ah_dbFor24;
		ob2GHz = ahp->ah_ob2GHz[0];
		db2GHz = ahp->ah_db2GHz[0];

		rfXpdGainFixed = ahp->ah_xgain[headerInfo11B];
		rfPloSel = ahp->ah_xpd[headerInfo11B];
		rfPwdXpd = !ahp->ah_xpd[headerInfo11B];
		gainI = ahp->ah_gainI[headerInfo11B];
		break;
	case CHANNEL_G:
		tempOB = ahp->ah_obFor24g;
		tempDB = ahp->ah_dbFor24g;
		ob2GHz = ahp->ah_ob2GHz[1];
		db2GHz = ahp->ah_db2GHz[1];

		rfXpdGainFixed = ahp->ah_xgain[headerInfo11G];
		rfPloSel = ahp->ah_xpd[headerInfo11G];
		rfPwdXpd = !ahp->ah_xpd[headerInfo11G];
		gainI = ahp->ah_gainI[headerInfo11G];
		break;
	default:
		HALDEBUG(ah, "%s: invalid channel flags 0x%x\n",
			__func__, chan->channelFlags);
		return AH_FALSE;
	}

	HALASSERT(1 <= tempOB && tempOB <= 5);
	HALASSERT(1 <= tempDB && tempDB <= 5);

	/* Bank 0 Write */
	for (i = 0; i < N(ar5212Bank0_5111); i++)
		rfReg[i] = ar5212Bank0_5111[i][modesIndex];
	if (IS_CHAN_2GHZ(chan)) {
		ar5212ModifyRfBuffer(rfReg, ob2GHz, 3, 119, 0);
		ar5212ModifyRfBuffer(rfReg, db2GHz, 3, 122, 0);
	}
	for (i = 0; i < N(ar5212Bank0_5111); i++) {
		OS_REG_WRITE(ah, ar5212Bank0_5111[i][0], rfReg[i]);
		WAR_6773(regWrites);
	}

	/* Bank 1 Write */
	REG_WRITE_ARRAY(ar5212Bank1_5111, 1, regWrites);

	/* Bank 2 Write */
	REG_WRITE_ARRAY(ar5212Bank2_5111, modesIndex, regWrites);

	/* Bank 3 Write */
	REG_WRITE_ARRAY(ar5212Bank3_5111, modesIndex, regWrites);

	/* Bank 6 Write */
	for (i = 0; i < N(ar5212Bank6_5111); i++)
		rfReg[i] = ar5212Bank6_5111[i][modesIndex];
	if (IS_CHAN_A(chan)) {		/* NB: CHANNEL_A | CHANNEL_T */
		ar5212ModifyRfBuffer(rfReg, ahp->ah_cornerCal.pd84, 1, 51, 3);
		ar5212ModifyRfBuffer(rfReg, ahp->ah_cornerCal.pd90, 1, 45, 3);
	}
	ar5212ModifyRfBuffer(rfReg, rfPwdXpd, 1, 95, 0);
	ar5212ModifyRfBuffer(rfReg, rfXpdGainFixed, 4, 96, 0);
	/* Set 5212 OB & DB */
	ar5212ModifyRfBuffer(rfReg, tempOB, 3, 104, 0);
	ar5212ModifyRfBuffer(rfReg, tempDB, 3, 107, 0);
	for (i = 0; i < N(ar5212Bank6_5111); i++) {
		OS_REG_WRITE(ah, ar5212Bank6_5111[i][0], rfReg[i]);
		WAR_6773(regWrites);
	}

	/* Bank 7 Write */
	for (i = 0; i < N(ar5212Bank7_5111); i++)
		rfReg[i] = ar5212Bank7_5111[i][modesIndex];
	ar5212ModifyRfBuffer(rfReg, gainI, 6, 29, 0);   
	ar5212ModifyRfBuffer(rfReg, rfPloSel, 1, 4, 0);   

	if (IS_CHAN_QUARTER_RATE(chan) || IS_CHAN_HALF_RATE(chan) || IS_CHAN_SUBQUARTER_RATE(chan)) {
        	u_int32_t	rfWaitI, rfWaitS, rfMaxTime;

        	rfWaitS = 0x1f;
        	rfWaitI = (IS_CHAN_HALF_RATE(chan)) ?  0x10 : 0x1f;
        	rfMaxTime = 3;
        	ar5212ModifyRfBuffer(rfReg, rfWaitS, 5, 19, 0);
        	ar5212ModifyRfBuffer(rfReg, rfWaitI, 5, 24, 0);
        	ar5212ModifyRfBuffer(rfReg, rfMaxTime, 2, 49, 0);

	}

	for (i = 0; i < N(ar5212Bank7_5111); i++) {
		OS_REG_WRITE(ah, ar5212Bank7_5111[i][0], rfReg[i]);
		WAR_6773(regWrites);
	}

	/* Now that we have reprogrammed rfgain value, clear the flag. */
	ahp->ah_rfgainState = HAL_RFGAIN_INACTIVE;

	return AH_TRUE;
}


/*
 * Read the transmit power levels from the structures taken from EEPROM
 * Interpolate read transmit power values for this channel
 * Organize the transmit power values into a table for writing into the hardware
 */
static HAL_BOOL
ar5111SetPowerTable(struct ath_hal *ah,
	int16_t *pMinPower, int16_t *pMaxPower, HAL_CHANNEL_INTERNAL *chan,
	u_int16_t *rfXpdGain)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	FULL_PCDAC_STRUCT pcdacStruct;
	int i, j;

	u_int16_t     *pPcdacValues;
	int16_t      *pScaledUpDbm;
	int16_t      minScaledPwr;
	int16_t      maxScaledPwr;
	int16_t      pwr;
	u_int16_t     pcdacMin = 0;
	u_int16_t     pcdacMax = PCDAC_STOP;
	u_int16_t     pcdacTableIndex;
	u_int16_t     scaledPcdac;
	PCDACS_EEPROM *pSrcStruct;
	PCDACS_EEPROM eepromPcdacs;

	/* setup the pcdac struct to point to the correct info, based on mode */
	switch (chan->channelFlags & CHANNEL_ALL) {
	case CHANNEL_A:
	case CHANNEL_T:
	case CHANNEL_X:
		eepromPcdacs.numChannels     = ahp->ah_numChannels11a;
		eepromPcdacs.pChannelList    = ahp->ah_channels11a;
		eepromPcdacs.pDataPerChannel = ahp->ah_dataPerChannel11a;
		break;
	case CHANNEL_B:
		eepromPcdacs.numChannels     = ahp->ah_numChannels2_4;
		eepromPcdacs.pChannelList    = ahp->ah_channels11b;
		eepromPcdacs.pDataPerChannel = ahp->ah_dataPerChannel11b;
		break;
	case CHANNEL_G:
	case CHANNEL_108G:
		eepromPcdacs.numChannels     = ahp->ah_numChannels2_4;
		eepromPcdacs.pChannelList    = ahp->ah_channels11g;
		eepromPcdacs.pDataPerChannel = ahp->ah_dataPerChannel11g;
		break;
	default:
		HALDEBUG(ah, "%s: invalid channel flags 0x%x\n",
			__func__, chan->channelFlags);
		return AH_FALSE;
	}

	pSrcStruct = &eepromPcdacs;

	OS_MEMZERO(&pcdacStruct, sizeof(pcdacStruct));
	pPcdacValues = pcdacStruct.PcdacValues;
	pScaledUpDbm = pcdacStruct.PwrValues;

	/* Initialize the pcdacs to dBM structs pcdacs to be 1 to 63 */
	for (i = PCDAC_START, j = 0; i <= PCDAC_STOP; i+= PCDAC_STEP, j++)
		pPcdacValues[j] = i;

	pcdacStruct.numPcdacValues = j;
	pcdacStruct.pcdacMin = PCDAC_START;
	pcdacStruct.pcdacMax = PCDAC_STOP;

	/* Fill out the power values for this channel */
	for (j = 0; j < pcdacStruct.numPcdacValues; j++ )
		pScaledUpDbm[j] = ar5212GetScaledPower(chan->channel,
			pPcdacValues[j], pSrcStruct);

	/* Now scale the pcdac values to fit in the 64 entry power table */
	minScaledPwr = pScaledUpDbm[0];
	maxScaledPwr = pScaledUpDbm[pcdacStruct.numPcdacValues - 1];

	/* find minimum and make monotonic */
	for (j = 0; j < pcdacStruct.numPcdacValues; j++) {
		if (minScaledPwr >= pScaledUpDbm[j]) {
			minScaledPwr = pScaledUpDbm[j];
			pcdacMin = j;
		}
		/*
		 * Make the full_hsh monotonically increasing otherwise
		 * interpolation algorithm will get fooled gotta start
		 * working from the top, hence i = 63 - j.
		 */
		i = (u_int16_t)(pcdacStruct.numPcdacValues - 1 - j);
		if (i == 0)
			break;
		if (pScaledUpDbm[i-1] > pScaledUpDbm[i]) {
			/*
			 * It could be a glitch, so make the power for
			 * this pcdac the same as the power from the
			 * next highest pcdac.
			 */
			pScaledUpDbm[i - 1] = pScaledUpDbm[i];
		}
	}

	for (j = 0; j < pcdacStruct.numPcdacValues; j++)
		if (maxScaledPwr < pScaledUpDbm[j]) {
			maxScaledPwr = pScaledUpDbm[j];
			pcdacMax = j;
		}

	/* Find the first power level with a pcdac */
	pwr = (u_int16_t)(PWR_STEP *
		((minScaledPwr - PWR_MIN + PWR_STEP / 2) / PWR_STEP) + PWR_MIN);

	/* Write all the first pcdac entries based off the pcdacMin */
	pcdacTableIndex = 0;
	for (i = 0; i < (2 * (pwr - PWR_MIN) / EEP_SCALE + 1); i++) {
		HALASSERT(pcdacTableIndex < PWR_TABLE_SIZE);
		ahp->ah_pcdacTable[pcdacTableIndex++] = pcdacMin;
	}

	i = 0;
	while (pwr < pScaledUpDbm[pcdacStruct.numPcdacValues - 1] &&
	    pcdacTableIndex < PWR_TABLE_SIZE) {
		pwr += PWR_STEP;
		/* stop if dbM > max_power_possible */
		while (pwr < pScaledUpDbm[pcdacStruct.numPcdacValues - 1] &&
		       (pwr - pScaledUpDbm[i])*(pwr - pScaledUpDbm[i+1]) > 0)
			i++;
		/* scale by 2 and add 1 to enable round up or down as needed */
		scaledPcdac = (u_int16_t)(interpolate(pwr,
			pScaledUpDbm[i], pScaledUpDbm[i + 1],
			(u_int16_t)(pPcdacValues[i] * 2),
			(u_int16_t)(pPcdacValues[i + 1] * 2)) + 1);

		HALASSERT(pcdacTableIndex < PWR_TABLE_SIZE);
		ahp->ah_pcdacTable[pcdacTableIndex] = scaledPcdac / 2;
		if (ahp->ah_pcdacTable[pcdacTableIndex] > pcdacMax)
			ahp->ah_pcdacTable[pcdacTableIndex] = pcdacMax;
		pcdacTableIndex++;
	}

	/* Write all the last pcdac entries based off the last valid pcdac */
	while (pcdacTableIndex < PWR_TABLE_SIZE) {
		ahp->ah_pcdacTable[pcdacTableIndex] =
			ahp->ah_pcdacTable[pcdacTableIndex - 1];
		pcdacTableIndex++;
	}

	/* No power table adjustment for 5111 */
	ahp->ah_txPowerIndexOffset = 0;

	return AH_TRUE;
}

/*
 * Get or interpolate the pcdac value from the calibrated data.
 */
static u_int16_t
ar5212GetScaledPower(u_int16_t channel, u_int16_t pcdacValue, PCDACS_EEPROM *pSrcStruct)
{
	u_int16_t powerValue;
	u_int16_t lFreq, rFreq;		/* left and right frequency values */
	u_int16_t llPcdac, ulPcdac;	/* lower and upper left pcdac values */
	u_int16_t lrPcdac, urPcdac;	/* lower and upper right pcdac values */
	u_int16_t lPwr, uPwr;		/* lower and upper temp pwr values */
	u_int16_t lScaledPwr, rScaledPwr; /* left and right scaled power */

	if (ar5212FindValueInList(channel, pcdacValue, pSrcStruct, &powerValue)) {
		/* value was copied from srcStruct */
		return powerValue;
	}

	ar5212GetLowerUpperValues(channel,
		pSrcStruct->pChannelList, pSrcStruct->numChannels,
		&lFreq, &rFreq);
	ar5212GetLowerUpperPcdacs(pcdacValue,
		lFreq, pSrcStruct, &llPcdac, &ulPcdac);
	ar5212GetLowerUpperPcdacs(pcdacValue,
		rFreq, pSrcStruct, &lrPcdac, &urPcdac);

	/* get the power index for the pcdac value */
	ar5212FindValueInList(lFreq, llPcdac, pSrcStruct, &lPwr);
	ar5212FindValueInList(lFreq, ulPcdac, pSrcStruct, &uPwr);
	lScaledPwr = interpolate(pcdacValue, llPcdac, ulPcdac, lPwr, uPwr);

	ar5212FindValueInList(rFreq, lrPcdac, pSrcStruct, &lPwr);
	ar5212FindValueInList(rFreq, urPcdac, pSrcStruct, &uPwr);
	rScaledPwr = interpolate(pcdacValue, lrPcdac, urPcdac, lPwr, uPwr);

	return interpolate(channel, lFreq, rFreq, lScaledPwr, rScaledPwr);
}

/*
 * Find the value from the calibrated source data struct
 */
static HAL_BOOL
ar5212FindValueInList(u_int16_t channel, u_int16_t pcdacValue,
	PCDACS_EEPROM *pSrcStruct, u_int16_t *powerValue)
{
	DATA_PER_CHANNEL *pChannelData = pSrcStruct->pDataPerChannel;
	int i;

	for (i = 0; i < pSrcStruct->numChannels; i++ ) {
		if (pChannelData->channelValue == channel) {
			u_int16_t* pPcdac = pChannelData->PcdacValues;
			int j;

			for (j = 0; j < pChannelData->numPcdacValues; j++ ) {
				if (*pPcdac == pcdacValue) {
					*powerValue = pChannelData->PwrValues[j];
					return AH_TRUE;
				}
				pPcdac++;
			}
		}
		pChannelData++;
	}
	return AH_FALSE;
}

/*
 * Get the upper and lower pcdac given the channel and the pcdac
 * used in the search
 */
static void
ar5212GetLowerUpperPcdacs(u_int16_t pcdac, u_int16_t channel,
	PCDACS_EEPROM *pSrcStruct,
	u_int16_t *pLowerPcdac, u_int16_t *pUpperPcdac)
{
	DATA_PER_CHANNEL *pChannelData = pSrcStruct->pDataPerChannel;
	int i;

	/* Find the channel information */
	for (i = 0; i < pSrcStruct->numChannels; i++) {
		if (pChannelData->channelValue == channel)
			break;
		pChannelData++;
	}
	ar5212GetLowerUpperValues(pcdac, pChannelData->PcdacValues,
		      pChannelData->numPcdacValues,
		      pLowerPcdac, pUpperPcdac);
}

/*
 * Free memory for analog bank scratch buffers
 */
static void
ar5111Detach(struct ath_hal *ah)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	if (ahp->ah_pcdacTable != AH_NULL) {
		ath_hal_free(ahp->ah_pcdacTable);
		ahp->ah_pcdacTable = AH_NULL;
	}
	if (ahp->ah_analogBanks != AH_NULL) {
		ath_hal_free(ahp->ah_analogBanks);
		ahp->ah_analogBanks = AH_NULL;
	}
}

static HAL_BOOL
ar5111GetChipPowerLimits(struct ath_hal *ah, HAL_CHANNEL *chans, u_int32_t nchans)
{
	/* XXX - Get 5111 power limits! */
	return AH_TRUE;
}

/*
 * Adjust NF based on statistical values for 5GHz frequencies.
 */
static int16_t
ar5111GetNfAdjust(struct ath_hal *ah, const HAL_CHANNEL_INTERNAL *c)
{
	static const struct {
		u_int16_t freqLow;
		int16_t	  adjust;
	} adjust5111[] = {
		{ 5790,	6 },	/* NB: ordered high -> low */
		{ 5730, 4 },
		{ 5690, 3 },
		{ 5660, 2 },
		{ 5610, 1 },
		{ 5530, 0 },
		{ 5450, 0 },
		{ 5379, 1 },
		{ 5209, 3 },
		{ 3000, 5 },
		{    0, 0 },
	};
	int i;

	for (i = 0; c->channel <= adjust5111[i].freqLow; i++)
		;
	return adjust5111[i].adjust;
}

/*
 * Allocate memory for analog bank scratch buffers
 * Scratch Buffer will be reinitialized every reset so no need to zero now
 */
HAL_BOOL
ar5111RfAttach(struct ath_hal *ah, HAL_STATUS *status)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	struct ath_hal_private *ahpriv = AH_PRIVATE(ah);
	HAL_CAPABILITIES *pCap = &ahpriv->ah_caps;
	pCap->halLow2GhzChan = 2312;

	HALASSERT(ahp->ah_analogBanks == AH_NULL);
	ahp->ah_analogBanks = ath_hal_malloc(sizeof(AR5212_RF_BANKS_5111));
	if (ahp->ah_analogBanks == AH_NULL) {
		HALDEBUG(ah, "%s: cannot allocate RF banks\n", __func__);
		*status = HAL_ENOMEM;		/* XXX */
		return AH_FALSE;
	}
	HALASSERT(ahp->ah_pcdacTable == AH_NULL);
	ahp->ah_pcdacTableSize = PWR_TABLE_SIZE * sizeof(u_int16_t);
	ahp->ah_pcdacTable = ath_hal_malloc(ahp->ah_pcdacTableSize);
	if (ahp->ah_pcdacTable == AH_NULL) {
		HALDEBUG(ah, "%s: cannot allocate PCDAC table\n", __func__);
		*status = HAL_ENOMEM;		/* XXX */
		return AH_FALSE;
	}

	ahp->ah_rfHal.rfDetach		= ar5111Detach;
	ahp->ah_rfHal.writeRegs		= ar5111WriteRegs;
	ahp->ah_rfHal.getRfBank		= ar5111GetRfBank;
	ahp->ah_rfHal.setChannel	= ar5111SetChannel;
	ahp->ah_rfHal.setRfRegs		= ar5111SetRfRegs;
	ahp->ah_rfHal.setPowerTable	= ar5111SetPowerTable;
	ahp->ah_rfHal.getChipPowerLim	= ar5111GetChipPowerLimits;
	ahp->ah_rfHal.getNfAdjust	= ar5111GetNfAdjust;

	return AH_TRUE;
}
#endif /* AH_SUPPORT_5111 */
