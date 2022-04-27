/*
 *
 * Copyright (c) 2004-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5212/ar5413.c#5 $
 * $File: //depot/sw/branches/sam_hal/ar5212/ar5413.c $
 * $Author: sam $
 * $DateTime: 2007/02/01 18:24:28 $
 * $Change: 232481 $
 *
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_5413

#include "ah.h"
#include "ah_internal.h"

#include "ar5212/ar5212.h"
#include "ar5212/ar5212reg.h"
#include "ar5212/ar5212phy.h"

/* Add static register initialization vectors */
#define AH_5212_5413
#include "ar5212/ar5212.ini"

#define	N(a)	(sizeof(a)/sizeof(a[0]))

typedef struct {
	u_int32_t Bank1Data[N(ar5212Bank1_5413)];
	u_int32_t Bank2Data[N(ar5212Bank2_5413)];
	u_int32_t Bank3Data[N(ar5212Bank3_5413)];
	u_int32_t Bank6Data[N(ar5212Bank6_5413)];
	u_int32_t Bank7Data[N(ar5212Bank7_5413)];
} AR5212_RF_BANKS_5413;


/*
 * WAR for bug 6773.  OS_DELAY() does a PIO READ on the PCI bus which allows
 * other cards' DMA reads to complete in the middle of our reset.
 */
#define WAR_6773(x) do {		\
	if ((++(x) % 64) == 0)		\
		OS_DELAY(1);		\
} while (0)

#define REG_WRITE_ARRAY(regArray, column, regWr) do {			\
	int r;								\
	for (r = 0; r < N(regArray); r++) {				\
		OS_REG_WRITE(ah, (regArray)[r][0], (regArray)[r][(column)]);\
		WAR_6773(regWr);					\
	}								\
} while (0)

#define REG_WRITE_RF_ARRAY(regArray, regData, regWr) do {               \
	int r;								\
	for (r = 0; r < N(regArray); r++) {				\
		OS_REG_WRITE(ah, (regArray)[r][0], (regData)[r]);	\
		WAR_6773(regWr);					\
	}								\
} while (0)

extern	void ar5212ModifyRfBuffer(u_int32_t *rfBuf, u_int32_t reg32,
		u_int32_t numBits, u_int32_t firstBit, u_int32_t column);

static void
ar5413WriteRegs(struct ath_hal *ah, u_int modesIndex, u_int freqIndex, int regWrites)
{
	REG_WRITE_ARRAY(ar5212Modes_5413, modesIndex, regWrites);
	REG_WRITE_ARRAY(ar5212Common_5413, 1, regWrites);
	REG_WRITE_ARRAY(ar5212BB_RfGain_5413, freqIndex, regWrites);
}

/*
 * Take the MHz channel value and set the Channel value
 *
 * ASSUMES: Writes enabled to analog bus
 */
static HAL_BOOL
ar5413SetChannel(struct ath_hal *ah,  HAL_CHANNEL_INTERNAL *chan)
{
	u_int32_t channelSel  = 0;
	u_int32_t bModeSynth  = 0;
	u_int32_t aModeRefSel = 0;
	u_int32_t reg32       = 0;
	u_int16_t freq;

	OS_MARK(ah, AH_MARK_SETCHANNEL, chan->channel);

	if (chan->channel < 4800) {
		u_int32_t txctl;

		if (((chan->channel - 2192) % 5) == 0) {
			channelSel = ((chan->channel - 672) * 2 - 3040)/10;
			bModeSynth = 0;
		} else if (((chan->channel - 2224) % 5) == 0) {
			channelSel = ((chan->channel - 704) * 2 - 3040) / 10;
			bModeSynth = 1;
		} else {
			HALDEBUG(ah, "%s: invalid channel %u MHz\n",
				__func__, chan->channel);
			return AH_FALSE;
		}

		channelSel = (channelSel << 2) & 0xff;
		channelSel = ath_hal_reverseBits(channelSel, 8);

		txctl = OS_REG_READ(ah, AR_PHY_CCK_TX_CTRL);
		if (chan->channel == 2484) {
			/* Enable channel spreading for channel 14 */
			OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
				txctl | AR_PHY_CCK_TX_CTRL_JAPAN);
		} else {
			OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
				txctl &~ AR_PHY_CCK_TX_CTRL_JAPAN);
		}
	} else if (((chan->channel % 5) == 2) && (chan->channel <= 5435)) {
		freq = chan->channel - 2; /* Align to even 5MHz raster */
		channelSel = ath_hal_reverseBits(
			(u_int32_t)(((freq - 4800)*10)/25 + 1), 8);
            	aModeRefSel = ath_hal_reverseBits(0, 2);
	} else if ((chan->channel % 20) == 0 && chan->channel >= 5120) {
		channelSel = ath_hal_reverseBits(
			((chan->channel - 4800) / 20 << 2), 8);
		aModeRefSel = ath_hal_reverseBits(1, 2);
	} else if ((chan->channel % 10) == 0) {
		channelSel = ath_hal_reverseBits(
			((chan->channel - 4800) / 10 << 1), 8);
		aModeRefSel = ath_hal_reverseBits(1, 2);
	} else if ((chan->channel % 5) == 0) {
		channelSel = ath_hal_reverseBits(
			(chan->channel - 4800) / 5, 8);
		aModeRefSel = ath_hal_reverseBits(1, 2);
	} else {
		HALDEBUG(ah, "%s: invalid channel %u MHz\n",
			__func__, chan->channel);
		return AH_FALSE;
	}

	reg32 = (channelSel << 4) | (aModeRefSel << 2) | (bModeSynth << 1) |
			(1 << 12) | 0x1;
	OS_REG_WRITE(ah, AR_PHY(0x27), reg32 & 0xff);

	reg32 >>= 8;
	OS_REG_WRITE(ah, AR_PHY(0x36), reg32 & 0x7f);

	AH_PRIVATE(ah)->ah_curchan = chan;
	return AH_TRUE;
}

/*
 * Reads EEPROM header info from device structure and programs
 * all rf registers
 *
 * REQUIRES: Access to the analog rf device
 */
static HAL_BOOL
ar5413SetRfRegs(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan, u_int16_t modesIndex, u_int16_t *rfXpdGain)
{
#define	RF_BANK_SETUP(_pb, _ix, _col) do {				    \
	int i;								    \
	for (i = 0; i < N(ar5212Bank##_ix##_5413); i++)			    \
		(_pb)->Bank##_ix##Data[i] = ar5212Bank##_ix##_5413[i][_col];\
} while (0)
	struct ath_hal_5212 *ahp = AH5212(ah);
	u_int16_t ob5GHz = 0, db5GHz = 0;	
	u_int16_t ob2GHz = 0, db2GHz = 0;
	AR5212_RF_BANKS_5413 *pRfBanks;
	int regWrites = 0;

	HALDEBUG(ah, "==>%s:chan 0x%x flag 0x%x modesIndex 0x%x\n", 
		 __func__, chan->channel, chan->channelFlags, modesIndex);

	pRfBanks = ahp->ah_analogBanks;

	HALASSERT(pRfBanks);

	/* Setup rf parameters */
	switch (chan->channelFlags & CHANNEL_ALL) {
	case CHANNEL_A:
	case CHANNEL_T:
		if (chan->channel > 4000 && chan->channel < 5260) {
			ob5GHz = ahp->ah_ob1;
			db5GHz = ahp->ah_db1;
		} else if (chan->channel >= 5260 && chan->channel < 5500) {
			ob5GHz = ahp->ah_ob2;
			db5GHz = ahp->ah_db2;
		} else if (chan->channel >= 5500 && chan->channel < 5725) {
			ob5GHz = ahp->ah_ob3;
			db5GHz = ahp->ah_db3;
		} else if (chan->channel >= 5725) {
			ob5GHz = ahp->ah_ob4;
			db5GHz = ahp->ah_db4;
		} else {
			/* XXX else */
		}
		break;
	case CHANNEL_B:
		ob2GHz = ahp->ah_obFor24;
		db2GHz = ahp->ah_dbFor24;
		break;
	case CHANNEL_G:
	case CHANNEL_108G:
		ob2GHz = ahp->ah_obFor24g;
		db2GHz = ahp->ah_dbFor24g;
		break;
	default:
		HALDEBUG(ah, "%s: invalid channel flags 0x%x\n",
			 __func__, chan->channelFlags);
		return AH_FALSE;
	}

	/* Bank 1 Write */
	RF_BANK_SETUP(pRfBanks, 1, 1);

	/* Bank 2 Write */
	RF_BANK_SETUP(pRfBanks, 2, modesIndex);

	/* Bank 3 Write */
	RF_BANK_SETUP(pRfBanks, 3, modesIndex);

	/* Bank 6 Write */
	RF_BANK_SETUP(pRfBanks, 6, modesIndex);

    	/* Only the 5 or 2 GHz OB/DB need to be set for a mode */
	if (IS_CHAN_2GHZ(chan)) {
        	ar5212ModifyRfBuffer(pRfBanks->Bank6Data, ob2GHz, 3, 241, 0);
        	ar5212ModifyRfBuffer(pRfBanks->Bank6Data, db2GHz, 3, 238, 0);

			/* TODO - only for Eagle 1.0 2GHz - remove for production */
			/* XXX: but without this bit G doesn't work. */
			ar5212ModifyRfBuffer(pRfBanks->Bank6Data, 1 , 1, 291, 2);

			/* Optimum value for rf_pwd_iclobuf2G for PCIe chips only */
			if (IS_PCIE(ah)) {
				ar5212ModifyRfBuffer(pRfBanks->Bank6Data, ath_hal_reverseBits(6, 3),
						 3, 131, 3);
			}
	} else {
        	ar5212ModifyRfBuffer(pRfBanks->Bank6Data, ob5GHz, 3, 247, 0);
        	ar5212ModifyRfBuffer(pRfBanks->Bank6Data, db5GHz, 3, 244, 0);

	}

	/* Bank 7 Setup */
	RF_BANK_SETUP(pRfBanks, 7, modesIndex);

	/* Write Analog registers */
	REG_WRITE_RF_ARRAY(ar5212Bank1_5413, pRfBanks->Bank1Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5212Bank2_5413, pRfBanks->Bank2Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5212Bank3_5413, pRfBanks->Bank3Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5212Bank6_5413, pRfBanks->Bank6Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5212Bank7_5413, pRfBanks->Bank7Data, regWrites);

	/* Now that we have reprogrammed rfgain value, clear the flag. */
	ahp->ah_rfgainState = HAL_RFGAIN_INACTIVE;

	HALDEBUG(ah, "<==%s\n", __func__);
	return AH_TRUE;
#undef	RF_BANK_SETUP
}

/*
 * Return a reference to the requested RF Bank.
 */
static u_int32_t *
ar5413GetRfBank(struct ath_hal *ah, int bank)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	AR5212_RF_BANKS_5413 *pRfBank5413 = ahp->ah_analogBanks;

	HALASSERT(ahp->ah_analogBanks != AH_NULL);
	switch (bank) {
	case 1: return pRfBank5413->Bank1Data;
	case 2: return pRfBank5413->Bank2Data;
	case 3: return pRfBank5413->Bank3Data;
	case 6: return pRfBank5413->Bank6Data;
	case 7: return pRfBank5413->Bank7Data;
	}
	HALDEBUG(ah, "%s: unknown RF Bank %d requested\n", __func__, bank);
	return AH_NULL;
}


/*
 * Fill the Vpdlist for indices Pmax-Pmin
 */
static HAL_BOOL
ar5413FillVpdTable(u_int32_t pdGainIdx, int16_t Pmin, int16_t  Pmax,
		   int16_t *pwrList, u_int16_t *VpdList, u_int16_t numIntercepts,
		   u_int16_t retVpdList[][64])
{
	u_int16_t ii, jj, kk;
	int16_t currPwr = (int16_t)(2*Pmin);
	/* since Pmin is pwr*2 and pwrList is 4*pwr */
	u_int32_t  idxL, idxR;

	ii = 0;
	jj = 0;

	if (numIntercepts < 2)
		return AH_FALSE;

	while (ii <= (u_int16_t)(Pmax - Pmin)) {
		ar5212GetLowerUpperIndex(currPwr, (u_int16_t *) pwrList,
				   numIntercepts, &(idxL), &(idxR));
		if (idxR < 1)
			idxR = 1;			/* extrapolate below */
		if (idxL == (u_int32_t)(numIntercepts - 1))
			idxL = numIntercepts - 2;	/* extrapolate above */
		if (pwrList[idxL] == pwrList[idxR])
			kk = VpdList[idxL];
		else
			kk = (u_int16_t)
				(((currPwr - pwrList[idxL])*VpdList[idxR]+ 
				  (pwrList[idxR] - currPwr)*VpdList[idxL])/
				 (pwrList[idxR] - pwrList[idxL]));
		retVpdList[pdGainIdx][ii] = kk;
		ii++;
		currPwr += 2;				/* half dB steps */
	}

	return AH_TRUE;
}


/*
 * Private state for reduced stack usage.
 */
struct ar5413state {
	/* filled out Vpd table for all pdGains (chanL) */
	u_int16_t vpdTable_L[MAX_NUM_PDGAINS_PER_CHANNEL]
			    [MAX_PWR_RANGE_IN_HALF_DB];
	/* filled out Vpd table for all pdGains (chanR) */
	u_int16_t vpdTable_R[MAX_NUM_PDGAINS_PER_CHANNEL]
			    [MAX_PWR_RANGE_IN_HALF_DB];
	/* filled out Vpd table for all pdGains (interpolated) */
	u_int16_t vpdTable_I[MAX_NUM_PDGAINS_PER_CHANNEL]
			    [MAX_PWR_RANGE_IN_HALF_DB];
};

/*
 * Uses the data points read from EEPROM to reconstruct the pdadc power table
 * Called by ar5413SetPowerTable()
 */
static int 
ar5413getGainBoundariesAndPdadcsForPowers(struct ath_hal *ah, u_int16_t channel,
		RAW_DATA_STRUCT_2413 *pRawDataset,  u_int16_t pdGainOverlap_t2, 
		int16_t  *pMinCalPower, u_int16_t pPdGainBoundaries[], 
		u_int16_t pPdGainValues[], u_int16_t pPDADCValues[]) 
{
	struct ar5413state *priv = AH5212(ah)->ah_rfHal.priv;
#define	VpdTable_L	priv->vpdTable_L
#define	VpdTable_R	priv->vpdTable_R
#define	VpdTable_I	priv->vpdTable_I
	u_int32_t ii, jj, kk;
	int32_t ss;/* potentially -ve index for taking care of pdGainOverlap */
	u_int32_t idxL, idxR;
	u_int32_t numPdGainsUsed = 0;
	/* 
	 * If desired to support -ve power levels in future, just
	 * change pwr_I_0 to signed 5-bits.
	 */
	int16_t Pmin_t2[MAX_NUM_PDGAINS_PER_CHANNEL];
	/* to accomodate -ve power levels later on. */
	int16_t Pmax_t2[MAX_NUM_PDGAINS_PER_CHANNEL];
	/* to accomodate -ve power levels later on */
	u_int16_t numVpd = 0;
	u_int16_t Vpd_step;
	int16_t tmpVal ; 
	u_int32_t sizeCurrVpdTable, maxIndex, tgtIndex;

	HALDEBUG(ah, "==>%s:\n", __func__);
    
	/* Get upper lower index */
	ar5212GetLowerUpperIndex(channel, pRawDataset->pChannels,
				 pRawDataset->numChannels, &(idxL), &(idxR));

	for (ii = 0; ii < MAX_NUM_PDGAINS_PER_CHANNEL; ii++) {
		jj = MAX_NUM_PDGAINS_PER_CHANNEL - ii - 1;
		/* work backwards 'cause highest pdGain for lowest power */
		numVpd = pRawDataset->pDataPerChannel[idxL].pDataPerPDGain[jj].numVpd;
		if (numVpd > 0) {
			pPdGainValues[numPdGainsUsed] = pRawDataset->pDataPerChannel[idxL].pDataPerPDGain[jj].pd_gain;
			Pmin_t2[numPdGainsUsed] = pRawDataset->pDataPerChannel[idxL].pDataPerPDGain[jj].pwr_t4[0];
			if (Pmin_t2[numPdGainsUsed] >pRawDataset->pDataPerChannel[idxR].pDataPerPDGain[jj].pwr_t4[0]) {
				Pmin_t2[numPdGainsUsed] = pRawDataset->pDataPerChannel[idxR].pDataPerPDGain[jj].pwr_t4[0];
			}
			Pmin_t2[numPdGainsUsed] = (int16_t)
				(Pmin_t2[numPdGainsUsed] / 2);
			Pmax_t2[numPdGainsUsed] = pRawDataset->pDataPerChannel[idxL].pDataPerPDGain[jj].pwr_t4[numVpd-1];
			if (Pmax_t2[numPdGainsUsed] > pRawDataset->pDataPerChannel[idxR].pDataPerPDGain[jj].pwr_t4[numVpd-1])
				Pmax_t2[numPdGainsUsed] = 
					pRawDataset->pDataPerChannel[idxR].pDataPerPDGain[jj].pwr_t4[numVpd-1];
			Pmax_t2[numPdGainsUsed] = (int16_t)(Pmax_t2[numPdGainsUsed] / 2);
			ar5413FillVpdTable(
					   numPdGainsUsed, Pmin_t2[numPdGainsUsed], Pmax_t2[numPdGainsUsed], 
					   &(pRawDataset->pDataPerChannel[idxL].pDataPerPDGain[jj].pwr_t4[0]), 
					   &(pRawDataset->pDataPerChannel[idxL].pDataPerPDGain[jj].Vpd[0]), numVpd, VpdTable_L
					   );
			ar5413FillVpdTable(
					   numPdGainsUsed, Pmin_t2[numPdGainsUsed], Pmax_t2[numPdGainsUsed], 
					   &(pRawDataset->pDataPerChannel[idxR].pDataPerPDGain[jj].pwr_t4[0]),
					   &(pRawDataset->pDataPerChannel[idxR].pDataPerPDGain[jj].Vpd[0]), numVpd, VpdTable_R
					   );
			for (kk = 0; kk < (u_int16_t)(Pmax_t2[numPdGainsUsed] - Pmin_t2[numPdGainsUsed]); kk++) {
				VpdTable_I[numPdGainsUsed][kk] = 
					interpolate_signed(
							   channel, pRawDataset->pChannels[idxL], pRawDataset->pChannels[idxR],
							   (int16_t)VpdTable_L[numPdGainsUsed][kk], (int16_t)VpdTable_R[numPdGainsUsed][kk]);
			}
			/* fill VpdTable_I for this pdGain */
			numPdGainsUsed++;
		}
		/* if this pdGain is used */
	}

	*pMinCalPower = Pmin_t2[0];
	kk = 0; /* index for the final table */
	for (ii = 0; ii < numPdGainsUsed; ii++) {
		if (ii == (numPdGainsUsed - 1))
			pPdGainBoundaries[ii] = Pmax_t2[ii] +
				PD_GAIN_BOUNDARY_STRETCH_IN_HALF_DB;
		else 
			pPdGainBoundaries[ii] = (u_int16_t)
				((Pmax_t2[ii] + Pmin_t2[ii+1]) / 2 );
		if (pPdGainBoundaries[ii] > 63) {
			HALDEBUG(ah, "%s: clamp pPdGainBoundaries[%d] %d\n",
			    __func__, ii, pPdGainBoundaries[ii]);/*XXX*/
			pPdGainBoundaries[ii] = 63;
		}

		/* Find starting index for this pdGain */
		if (ii == 0) 
			ss = 0; /* for the first pdGain, start from index 0 */
		else 
			ss = (pPdGainBoundaries[ii-1] - Pmin_t2[ii]) - 
				pdGainOverlap_t2;
		Vpd_step = (u_int16_t)(VpdTable_I[ii][1] - VpdTable_I[ii][0]);
		Vpd_step = (u_int16_t)((Vpd_step < 1) ? 1 : Vpd_step);
		/*
		 *-ve ss indicates need to extrapolate data below for this pdGain
		 */
		while (ss < 0) {
			tmpVal = (int16_t)(VpdTable_I[ii][0] + ss*Vpd_step);
			pPDADCValues[kk++] = (u_int16_t)((tmpVal < 0) ? 0 : tmpVal);
			ss++;
		}

		sizeCurrVpdTable = Pmax_t2[ii] - Pmin_t2[ii];
		tgtIndex = pPdGainBoundaries[ii] + pdGainOverlap_t2 - Pmin_t2[ii];
		maxIndex = (tgtIndex < sizeCurrVpdTable) ? tgtIndex : sizeCurrVpdTable;

		while (ss < (int16_t)maxIndex)
			pPDADCValues[kk++] = VpdTable_I[ii][ss++];

		Vpd_step = (u_int16_t)(VpdTable_I[ii][sizeCurrVpdTable-1] -
				       VpdTable_I[ii][sizeCurrVpdTable-2]);
		Vpd_step = (u_int16_t)((Vpd_step < 1) ? 1 : Vpd_step);           
		/*
		 * for last gain, pdGainBoundary == Pmax_t2, so will 
		 * have to extrapolate
		 */
		if (tgtIndex > maxIndex) {	/* need to extrapolate above */
			while(ss < (int16_t)tgtIndex) {
				tmpVal = (u_int16_t)
					(VpdTable_I[ii][sizeCurrVpdTable-1] + 
					 (ss-maxIndex)*Vpd_step);
				pPDADCValues[kk++] = (tmpVal > 127) ? 
					127 : tmpVal;
				ss++;
			}
		}				/* extrapolated above */
	}					/* for all pdGainUsed */

	while (ii < MAX_NUM_PDGAINS_PER_CHANNEL) {
		pPdGainBoundaries[ii] = pPdGainBoundaries[ii-1];
		ii++;
	}
	while (kk < 128) {
		pPDADCValues[kk] = pPDADCValues[kk-1];
		kk++;
	}

	HALDEBUG(ah, "<==%s\n", __func__);

	return numPdGainsUsed;
#undef VpdTable_L
#undef VpdTable_R
#undef VpdTable_I
}

static HAL_BOOL
ar5413SetPowerTable(struct ath_hal *ah,
	int16_t *minPower, int16_t *maxPower, HAL_CHANNEL_INTERNAL *chan, 
	u_int16_t *rfXpdGain)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	RAW_DATA_STRUCT_2413 *pRawDataset = AH_NULL;
	u_int16_t pdGainOverlap_t2;
	int16_t minCalPower5413_t2;
	u_int16_t *pdadcValues = ahp->ah_pcdacTable;
	u_int16_t gainBoundaries[4];
	u_int32_t i, reg32, regoffset;
	int numPdGainsUsed;

	HALDEBUG(ah, "%s:chan 0x%x flag 0x%x\n", __func__, chan->channel,chan->channelFlags);

	if (IS_CHAN_G(chan) || IS_CHAN_108G(chan))
		pRawDataset = &ahp->ah_rawDataset2413[headerInfo11G];
	else if (IS_CHAN_B(chan))
		pRawDataset = &ahp->ah_rawDataset2413[headerInfo11B];
	else {
		HALASSERT(IS_CHAN_5GHZ(chan));
		pRawDataset = &ahp->ah_rawDataset2413[headerInfo11A];
	}

	pdGainOverlap_t2 = (u_int16_t) SM(OS_REG_READ(ah, AR_PHY_TPCRG5),
					  AR_PHY_TPCRG5_PD_GAIN_OVERLAP);
    
	numPdGainsUsed = ar5413getGainBoundariesAndPdadcsForPowers(ah,
		chan->channel, pRawDataset, pdGainOverlap_t2,
		&minCalPower5413_t2,gainBoundaries, rfXpdGain, pdadcValues);

#ifdef AH_DEBUG
if (numPdGainsUsed != pRawDataset->pDataPerChannel[0].numPdGains || numPdGainsUsed > 2) ath_hal_printf(ah, "%s: numPdGainsUsed %d numPdGains %d\n", __func__, numPdGainsUsed, pRawDataset->pDataPerChannel[0].numPdGains);/*XXX*/
#endif
#if 0
	OS_REG_RMW_FIELD(ah, AR_PHY_TPCRG1, AR_PHY_TPCRG1_NUM_PD_GAIN, 
			 (pRawDataset->pDataPerChannel[0].numPdGains - 1));
#else
{ u_int32_t tpcrg1, newtpcrg1;
	tpcrg1 = OS_REG_READ(ah, AR_PHY_TPCRG1);
	newtpcrg1 = (tpcrg1 &~ (AR_PHY_TPCRG1_NUM_PD_GAIN|AR_PHY_TPCRG1_PDGAIN_SETTING1|AR_PHY_TPCRG1_PDGAIN_SETTING2|AR_PHY_TPCRG1_PDGAIN_SETTING3))
	       | SM(numPdGainsUsed-1, AR_PHY_TPCRG1_NUM_PD_GAIN)
	       | SM(rfXpdGain[0], AR_PHY_TPCRG1_PDGAIN_SETTING1)
	       | SM(rfXpdGain[1], AR_PHY_TPCRG1_PDGAIN_SETTING2)
	       | SM(rfXpdGain[2], AR_PHY_TPCRG1_PDGAIN_SETTING3)
	       ;
	OS_REG_WRITE(ah, AR_PHY_TPCRG1, newtpcrg1);
#ifdef AH_DEBUG
if (tpcrg1 != newtpcrg1) ath_hal_printf(ah, "%s: tpcrg1 0x%x => 0x%x\n", __func__, tpcrg1, newtpcrg1);
#endif
}
#endif

	/*
	 * Note the pdadc table may not start at 0 dBm power, could be
	 * negative or greater than 0.  Need to offset the power
	 * values by the amount of minPower for griffin
	 */
	if (minCalPower5413_t2 != 0)
		ahp->ah_txPowerIndexOffset = (int16_t)(0 - minCalPower5413_t2);
	else
		ahp->ah_txPowerIndexOffset = 0;

	/* Finally, write the power values into the baseband power table */
	regoffset = 0x9800 + (672 <<2); /* beginning of pdadc table in griffin */
	for (i = 0; i < 32; i++) {
		reg32 = ((pdadcValues[4*i + 0] & 0xFF) << 0)  | 
			((pdadcValues[4*i + 1] & 0xFF) << 8)  |
			((pdadcValues[4*i + 2] & 0xFF) << 16) |
			((pdadcValues[4*i + 3] & 0xFF) << 24) ;        
		OS_REG_WRITE(ah, regoffset, reg32);
		regoffset += 4;
	}

	OS_REG_WRITE(ah, AR_PHY_TPCRG5, 
		     SM(pdGainOverlap_t2, AR_PHY_TPCRG5_PD_GAIN_OVERLAP) | 
		     SM(gainBoundaries[0], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_1) |
		     SM(gainBoundaries[1], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_2) |
		     SM(gainBoundaries[2], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_3) |
		     SM(gainBoundaries[3], AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_4));

	return AH_TRUE;
}

/*
 * Free memory for analog bank scratch buffers
 */
static void
ar5413RfDetach(struct ath_hal *ah)
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
	if (ahp->ah_rfHal.priv != AH_NULL) {
		ath_hal_free(ahp->ah_rfHal.priv);
		ahp->ah_rfHal.priv = AH_NULL;
	}
}

static int16_t
ar5413GetMinPower(struct ath_hal *ah, RAW_DATA_PER_CHANNEL_2413 *data)
{
	u_int32_t ii,jj;
	u_int16_t Pmin=0,numVpd;

	for (ii = 0; ii < MAX_NUM_PDGAINS_PER_CHANNEL; ii++) {
		jj = MAX_NUM_PDGAINS_PER_CHANNEL - ii - 1;
		/* work backwards 'cause highest pdGain for lowest power */
		numVpd = data->pDataPerPDGain[jj].numVpd;
		if (numVpd > 0) {
			Pmin = data->pDataPerPDGain[jj].pwr_t4[0];
			return(Pmin);
		}
	}
	return(Pmin);
}

static int16_t
ar5413GetMaxPower(struct ath_hal *ah, RAW_DATA_PER_CHANNEL_2413 *data)
{
	u_int32_t ii;
	u_int16_t Pmax=0,numVpd;
	
	for (ii=0; ii< MAX_NUM_PDGAINS_PER_CHANNEL; ii++) {
		/* work forwards cuase lowest pdGain for highest power */
		numVpd = data->pDataPerPDGain[ii].numVpd;
		if (numVpd > 0) {
			Pmax = data->pDataPerPDGain[ii].pwr_t4[numVpd-1];
			return(Pmax);
		}
	}
	return(Pmax);
}

static
HAL_BOOL ar5413GetChannelMaxMinPower(struct ath_hal *ah, HAL_CHANNEL *chan,
				     int16_t *maxPow, int16_t *minPow)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	RAW_DATA_STRUCT_2413 *pRawDataset = AH_NULL;
	RAW_DATA_PER_CHANNEL_2413 *data=AH_NULL;
	u_int16_t numChannels;
	int totalD,totalF, totalMin,last, i;

	*maxPow = 0;

	if (IS_CHAN_G(chan) || IS_CHAN_108G(chan))
		pRawDataset = &ahp->ah_rawDataset2413[headerInfo11G];
	else if (IS_CHAN_B(chan))
		pRawDataset = &ahp->ah_rawDataset2413[headerInfo11B];
	else {
		HALASSERT(IS_CHAN_5GHZ(chan));
		pRawDataset = &ahp->ah_rawDataset2413[headerInfo11A];
	}

	numChannels = pRawDataset->numChannels;
	data = pRawDataset->pDataPerChannel;
	
	/* Make sure the channel is in the range of the TP values 
	 *  (freq piers)
	 */
	if (numChannels < 1)
		return(AH_FALSE);

	if ((chan->channel < data[0].channelValue) ||
	    (chan->channel > data[numChannels-1].channelValue)) {
		if (chan->channel < data[0].channelValue) {
			*maxPow = ar5413GetMaxPower(ah, &data[0]);
			*minPow = ar5413GetMinPower(ah, &data[0]);
			return(AH_TRUE);
		} else {
			*maxPow = ar5413GetMaxPower(ah, &data[numChannels - 1]);
			*minPow = ar5413GetMinPower(ah, &data[numChannels - 1]);
			return(AH_TRUE);
		}
	}

	/* Linearly interpolate the power value now */
	for (last=0,i=0; (i<numChannels) && (chan->channel > data[i].channelValue);
	     last = i++);
	totalD = data[i].channelValue - data[last].channelValue;
	if (totalD > 0) {
		totalF = ar5413GetMaxPower(ah, &data[i]) - ar5413GetMaxPower(ah, &data[last]);
		*maxPow = (int8_t) ((totalF*(chan->channel-data[last].channelValue) + 
				     ar5413GetMaxPower(ah, &data[last])*totalD)/totalD);
		totalMin = ar5413GetMinPower(ah, &data[i]) - ar5413GetMinPower(ah, &data[last]);
		*minPow = (int8_t) ((totalMin*(chan->channel-data[last].channelValue) +
				     ar5413GetMinPower(ah, &data[last])*totalD)/totalD);
		return(AH_TRUE);
	} else {
		if (chan->channel == data[i].channelValue) {
			*maxPow = ar5413GetMaxPower(ah, &data[i]);
			*minPow = ar5413GetMinPower(ah, &data[i]);
			return(AH_TRUE);
		} else
			return(AH_FALSE);
	}
}

static HAL_BOOL
ar5413GetChipPowerLimits(struct ath_hal *ah, HAL_CHANNEL *chans, u_int32_t nchans)
{
	HAL_BOOL retVal = AH_TRUE;
	int i;
	int16_t maxPow, minPow;

	for (i=0; i<nchans; i++) {
		if (ar5413GetChannelMaxMinPower(ah, &chans[i], &maxPow, &minPow)) {
			chans[i].maxTxPower = maxPow;
			chans[i].minTxPower = minPow;
		} else {
			HALDEBUG(ah, "Failed setting power table for nchans=%d\n", i);
			retVal = AH_FALSE;
		}
	}
#ifdef AH_DEBUG
	for (i=0; i<nchans; i++) {
		ath_hal_printf(ah,"Chan %d: MaxPow = %d MinPow = %d\n",
			 chans[i].channel,chans[i].maxTxPower, chans[i].minTxPower);
	}
#endif
	return (retVal);
}		

/*
 * Allocate memory for analog bank scratch buffers
 * Scratch Buffer will be reinitialized every reset so no need to zero now
 */
HAL_BOOL
ar5413RfAttach(struct ath_hal *ah, HAL_STATUS *status)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	HALASSERT(ahp->ah_analogBanks == AH_NULL);
	ahp->ah_analogBanks = ath_hal_malloc(sizeof(AR5212_RF_BANKS_5413));
	if (ahp->ah_analogBanks == AH_NULL) {
		HALDEBUG(ah, "%s: cannot allocate RF banks\n", __func__);
		*status = HAL_ENOMEM;		/* XXX */
		return AH_FALSE;
	}
	HALASSERT(ahp->ah_pcdacTable == AH_NULL);
	ahp->ah_pcdacTableSize = PWR_TABLE_SIZE_2413 * sizeof(u_int16_t);
	ahp->ah_pcdacTable = ath_hal_malloc(ahp->ah_pcdacTableSize);
	if (ahp->ah_pcdacTable == AH_NULL) {
		HALDEBUG(ah, "%s: cannot allocate PCDAC table\n", __func__);
		*status = HAL_ENOMEM;		/* XXX */
		return AH_FALSE;
	}
	HALASSERT(ahp->ah_rfHal.priv == AH_NULL);
	ahp->ah_rfHal.priv = ath_hal_malloc(sizeof(struct ar5413state));
	if (ahp->ah_rfHal.priv == AH_NULL) {
		HALDEBUG(ah, "%s: cannot allocate private state\n", __func__);
		*status = HAL_ENOMEM;		/* XXX */
		return AH_FALSE;
	}

	ahp->ah_rfHal.rfDetach		= ar5413RfDetach;
	ahp->ah_rfHal.writeRegs		= ar5413WriteRegs;
	ahp->ah_rfHal.getRfBank		= ar5413GetRfBank;
	ahp->ah_rfHal.setChannel	= ar5413SetChannel;
	ahp->ah_rfHal.setRfRegs		= ar5413SetRfRegs;
	ahp->ah_rfHal.setPowerTable	= ar5413SetPowerTable;
	ahp->ah_rfHal.getChipPowerLim	= ar5413GetChipPowerLimits;
	ahp->ah_rfHal.getNfAdjust	= ar5212GetNfAdjust;

	return AH_TRUE;
}
#endif /* AH_SUPPORT_5413 */
