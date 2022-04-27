/*
 * Copyright (c) 2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar2133.c#5 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_2133

#define RFPART 1

#include "ah.h"
#include "ah_internal.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"

/* Add static register initialization vectors */
#define AH_5416_2133
#include "ar5416/ar5416.ini"
#ifdef AH_SUPPORT_AR9000
#include "ar5416/ar5416_sowl.ini"
#endif

#ifdef AH_ENABLE_FORCEBIAS
int	ath_hal_force_bias = 2;
#endif

#define N(a)    (sizeof(a)/sizeof(a[0]))

/*
 * WAR for bug 6773.  OS_DELAY() does a PIO READ on the PCI bus which allows
 * other cards' DMA reads to complete in the middle of our reset.
 */
#define WAR_6773(x) do {        \
	if ((++(x) % 64) == 0)      \
		OS_DELAY(1);        \
} while (0)

#define REG_WRITE_ARRAY(regArray, column, regWr) do {                   \
	int r;                              \
	for (r = 0; r < N(regArray); r++) {             \
		OS_REG_WRITE(ah, (regArray)[r][0], (regArray)[r][(column)]);\
		WAR_6773(regWr);                    \
	}                               \
} while (0)

#define REG_WRITE_RF_ARRAY(regArray, regData, regWr) do {               \
	int r;                              \
	for (r = 0; r < N(regArray); r++) {             \
		OS_REG_WRITE(ah, (regArray)[r][0], (regData)[r]);   \
		WAR_6773(regWr);                    \
	}                               \
} while (0)

#define	ar5416ModifyRfBuffer	ar5212ModifyRfBuffer	/*XXX*/

extern  void ar5416ModifyRfBuffer(u_int32_t *rfBuf, u_int32_t reg32,
	u_int32_t numBits, u_int32_t firstBit, u_int32_t column);
HAL_BOOL ar2133GetChipPowerLimits(struct ath_hal *ah, HAL_CHANNEL 
	*chans, u_int32_t nchans);
	
static void ar2133Detach(struct ath_hal *ah);
int16_t ar2133GetNfAdjust(struct ath_hal *ah, const HAL_CHANNEL_INTERNAL *c);

typedef struct {
	u_int32_t Bank0Data[N(ar5416Bank0)];
	u_int32_t Bank1Data[N(ar5416Bank1)];
	u_int32_t Bank2Data[N(ar5416Bank2)];
	u_int32_t Bank3Data[N(ar5416Bank3)];
	u_int32_t Bank6Data[N(ar5416Bank6)];
	u_int32_t Bank7Data[N(ar5416Bank7)];
} AR5416_RF_BANKS_2133;

static void
ar2133WriteRegs(struct ath_hal *ah, u_int modesIndex, u_int freqIndex, int regWrites)
{
#ifdef AH_SUPPORT_AR9000
	if (AR_SREV_SOWL_10_OR_LATER(ah)) 
		REG_WRITE_ARRAY(ar5416BB_RfGain_sowl, freqIndex, regWrites);
	else
#endif
		REG_WRITE_ARRAY(ar5416BB_RfGain, freqIndex, regWrites);
    
}


/*
 * Take the MHz channel value and set the Channel value
 *
 * ASSUMES: Writes enabled to analog bus
 *
 * Actual Expression,
 *
 * For 2GHz channel, 
 * Channel Frequency = (3/4) * freq_ref * (chansel[8:0] + chanfrac[16:0]/2^17) 
 * (freq_ref = 40MHz)
 *
 * For 5GHz channel,
 * Channel Frequency = (3/2) * freq_ref * (chansel[8:0] + chanfrac[16:0]/2^10)
 * (freq_ref = 40MHz/(24>>amodeRefSel))
 */
#ifdef AH_SUPPORT_AR9200
static HAL_BOOL
ar9280SetChannel(struct ath_hal *ah,  HAL_CHANNEL_INTERNAL *chan)
{
    u_int16_t bMode, fracMode, aModeRefSel = 0;
    u_int32_t freq, ndiv, channelSel = 0, channelFrac = 0, reg32 = 0;
    CHAN_CENTERS centers;
    u_int32_t refDivA = 24;

    OS_MARK(ah, AH_MARK_SETCHANNEL, chan->channel);
    ar5416GetChannelCenters(ah, chan, &centers);
    freq = centers.synth_center;
    
    reg32 = OS_REG_READ(ah, AR_PHY_SYNTH_CONTROL);
    reg32 &= 0xc0000000;

    if (freq < 4800) {     /* 2 GHz, fractional mode */
        u_int32_t txctl;

        bMode = 1;
        fracMode = 1;
        aModeRefSel = 0;       
        channelSel = (freq * 0x10000)/15;

        txctl = OS_REG_READ(ah, AR_PHY_CCK_TX_CTRL);
        if (freq == 2484) {
            /* Enable channel spreading for channel 14 */
            OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
                txctl | AR_PHY_CCK_TX_CTRL_JAPAN);
        } else {
            OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
                txctl &~ AR_PHY_CCK_TX_CTRL_JAPAN);
        }     
    } else {
        bMode = 0;
        fracMode = 0;

        if ((freq % 20) == 0) {
            aModeRefSel = 3;
        } else if ((freq % 10) == 0) {
            aModeRefSel = 2;
        } else {
            aModeRefSel = 0;
            /* Enable 2G (fractional) mode for channels which are 5MHz spaced */
            fracMode = 1;
            refDivA = 1;
            channelSel = (freq * 0x8000)/15;
            
            /* RefDivA setting */
            OS_REG_RMW_FIELD(ah, AR_AN_SYNTH9, AR_AN_SYNTH9_REFDIVA, refDivA);

        }
        if (!fracMode) {
            ndiv = (freq * (refDivA >> aModeRefSel))/60;
            channelSel =  ndiv & 0x1ff;         
            channelFrac = (ndiv & 0xfffffe00) * 2;
            channelSel = (channelSel << 17) | channelFrac;
        }
    }
    reg32 = reg32 | 
           (bMode << 29) |
           (fracMode << 28) |
           (aModeRefSel << 26) |
           (channelSel);



    OS_REG_WRITE(ah, AR_PHY_SYNTH_CONTROL, reg32);

    AH_PRIVATE(ah)->ah_curchan = chan;

#if 0
    if (chan->privFlags & CHANNEL_DFS) {
        struct ar5416RadarState *rs;
        u_int8_t index;

        rs = ar5416GetRadarChanState(ah, &index);
        if (rs != AH_NULL) {
            AH5416(ah)->ah_curchanRadIndex = (int16_t) index;
        } else {
           // HDPRINTF(ah, HAL_DBG_DFS, "%s: Couldn't find radar state information\n",
           //      __func__);
            return AH_FALSE;
        }
    } else
#endif

    return AH_TRUE;
}
#endif
/*
 * Take the MHz channel value and set the Channel value
 *
 * ASSUMES: Writes enabled to analog bus
 */
static HAL_BOOL
ar2133SetChannel(struct ath_hal *ah,  HAL_CHANNEL_INTERNAL *chan)
{
	u_int32_t channelSel  = 0;
	u_int32_t bModeSynth  = 0;
	u_int32_t aModeRefSel = 0;
	u_int32_t reg32       = 0;
	u_int16_t freq;
	CHAN_CENTERS centers;
    
	OS_MARK(ah, AH_MARK_SETCHANNEL, chan->channel);
    
	ar5416GetChannelCenters(ah,  chan, &centers);
	freq = centers.synth_center;

	if (freq < 4800) {
		u_int32_t txctl;

		if (((freq - 2192) % 5) == 0) {
			channelSel = ((freq - 672) * 2 - 3040)/10;
			bModeSynth = 0;
		} else if (((freq - 2224) % 5) == 0) {
			channelSel = ((freq - 704) * 2 - 3040) / 10;
			bModeSynth = 1;
		} else {
			HALDEBUG(ah, "%s: invalid channel %u MHz\n",
				__func__, freq);
			return AH_FALSE;
		}

		channelSel = (channelSel << 2) & 0xff;
		channelSel = ath_hal_reverseBits(channelSel, 8);

		txctl = OS_REG_READ(ah, AR_PHY_CCK_TX_CTRL);
		if (freq == 2484) {
			/* Enable channel spreading for channel 14 */
			OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
				txctl | AR_PHY_CCK_TX_CTRL_JAPAN);
		} else {
			OS_REG_WRITE(ah, AR_PHY_CCK_TX_CTRL,
 			txctl &~ AR_PHY_CCK_TX_CTRL_JAPAN);
		}
	} else if ((freq % 20) == 0 && freq >= 5120) {
		channelSel = ath_hal_reverseBits(((freq - 4800) / 20 << 2), 8);
		if (AR_SREV_HOWL(ah) || AR_SREV_SOWL_10_OR_LATER(ah))
		    aModeRefSel = ath_hal_reverseBits(3, 2);
		else
		    aModeRefSel = ath_hal_reverseBits(1, 2);
	} else if ((freq % 10) == 0) {
		channelSel = ath_hal_reverseBits(((freq - 4800) / 10 << 1), 8);
		if (AR_SREV_HOWL(ah) || AR_SREV_SOWL_10_OR_LATER(ah))
		    aModeRefSel = ath_hal_reverseBits(2, 2);
		else
		    aModeRefSel = ath_hal_reverseBits(1, 2);
	} else if ((freq % 5) == 0) {
		channelSel = ath_hal_reverseBits((freq - 4800) / 5, 8);
		aModeRefSel = ath_hal_reverseBits(1, 2);
	} else {
		HALDEBUG(ah, "%s: invalid channel %u MHz\n",
				__func__, freq);
		return AH_FALSE;
	}

	reg32 = (channelSel << 8) | (aModeRefSel << 2) | (bModeSynth << 1) |
		(1 << 5) | 0x1;

	OS_REG_WRITE(ah, AR_PHY(0x37), reg32);

	AH_PRIVATE(ah)->ah_curchan = chan;
#if 0
	if (chan->privFlags & CHANNEL_DFS) {
		struct ar5416RadarState *rs;
		u_int8_t index;

		rs = ar5416GetRadarChanState(ah, &index);
		if (rs != AH_NULL) {
			AH5416(ah)->ah_curchanRadIndex = (int16_t) index;
		} else {
			HALDEBUG(ah, "%s: Couldn't find radar state information\n",
				__func__);
			return AH_FALSE;
		}
	} else
		AH5416(ah)->ah_curchanRadIndex = -1;
#endif
	return AH_TRUE;

}

/*
 * Return a reference to the requested RF Bank.
 */
static u_int32_t *
ar2133GetRfBank(struct ath_hal *ah, int bank)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
	AR5416_RF_BANKS_2133 *pRfBank2133 = ahp->ah_analogBanks;

	HALASSERT(ahp->ah_analogBanks != AH_NULL);
	switch (bank) {
	case 1: return pRfBank2133->Bank1Data;
	case 2: return pRfBank2133->Bank2Data;
	case 3: return pRfBank2133->Bank3Data;
	case 6: return pRfBank2133->Bank6Data;
	case 7: return pRfBank2133->Bank7Data;
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
ar2133SetRfRegs(struct ath_hal *ah, HAL_CHANNEL_INTERNAL *chan,
                u_int16_t modesIndex, u_int16_t *rfXpdGain)
{
#define RF_BANK_SETUP(_pb, _ix, _col) do {                  \
	int i;                                  \
	for (i = 0; i < N(ar5416Bank##_ix); i++)             \
		(_pb)->Bank##_ix##Data[i] = ar5416Bank##_ix[i][_col];\
} while (0)
#define RF_BANK_SETUP_SOWL(_pb, _ix, _col) do {                  \
	int i;                                  \
	for (i = 0; i < N(ar5416Bank##_ix); i++)             \
		(_pb)->Bank##_ix##Data[i] = ar5416Bank##_ix##_sowl[i][_col];\
} while (0)
	struct ath_hal_5212 *ahp = AH5212(ah);
	AR5416_RF_BANKS_2133 *pRfBanks = ahp->ah_analogBanks;
	int regWrites = 0;
#ifdef AH_SUPPORT_AR9200

    if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
        /* Software does not need to program bank data for chips after Merlin */
        return AH_TRUE;
    }
#endif
	HALASSERT(pRfBanks);

#ifdef AH_SUPPORT_AR9000
	if (AR_SREV_SOWL_10_OR_LATER(ah)) {
	/* Setup Bank 0 Write */
	RF_BANK_SETUP_SOWL(pRfBanks, 0, 1);

	/* Setup Bank 1 Write */
	RF_BANK_SETUP_SOWL(pRfBanks, 1, 1);

	/* Setup Bank 2 Write */
	RF_BANK_SETUP_SOWL(pRfBanks, 2, 1);

	/* Setup Bank 3 Write */
	RF_BANK_SETUP_SOWL(pRfBanks, 3, modesIndex);

	/* Setup Bank 6 Write */
	RF_BANK_SETUP_SOWL(pRfBanks, 6, modesIndex);
	} else 
#endif
	{
	/* Setup Bank 0 Write */
	RF_BANK_SETUP(pRfBanks, 0, 1);

	/* Setup Bank 1 Write */
	RF_BANK_SETUP(pRfBanks, 1, 1);

	/* Setup Bank 2 Write */
	RF_BANK_SETUP(pRfBanks, 2, 1);

	/* Setup Bank 3 Write */
	RF_BANK_SETUP(pRfBanks, 3, modesIndex);

	/* Setup Bank 6 Write */
	RF_BANK_SETUP(pRfBanks, 6, modesIndex);
    }	
	
	
	/* Only the 5 or 2 GHz OB/DB need to be set for a mode */
	if (IS_CHAN_2GHZ(chan)) {
		ar5416ModifyRfBuffer(pRfBanks->Bank6Data, (u_int32_t) ahp->ah_obFor24, 3, 197, 0);
		ar5416ModifyRfBuffer(pRfBanks->Bank6Data, (u_int32_t) ahp->ah_dbFor24, 3, 194, 0);
	} else {
		ar5416ModifyRfBuffer(pRfBanks->Bank6Data, (u_int32_t) ahp->ah_ob1, 3, 203, 0);
		ar5416ModifyRfBuffer(pRfBanks->Bank6Data, (u_int32_t) ahp->ah_db1, 3, 200, 0);
	}
#ifdef AH_ENABLE_FORCEBIAS
    /*
     * Workaround FOWL orientation sensitivity bug by increasing rf_pwd_icsyndiv
     */
	if (IS_CHAN_2GHZ(chan) && ath_hal_force_bias) {
        /* DEBUG */
        HALDEBUG(ah, "Force rf_pwd_icsyndiv to %d\n", ath_hal_force_bias);
#if 0
        {
            int ii;
            HALDEBUG(ah, "DUMP BANK 6 in\n");
            for (ii = 0; ii < 33; ii++) {
                HALDEBUG(ah, " %8.8x\n", pRfBanks->Bank6Data[ii]);
            }
        }
#endif
    if (!AR_SREV_SOWL_10_OR_LATER(ah)) {
        /* swizzle rf_pwd_icsyndiv */
        ar5416ModifyRfBuffer(pRfBanks->Bank6Data,
                             ath_hal_force_bias & 7,
                             3, 181, 3);
	}
#if 0
        {
            int ii;
            HALDEBUG(ah, "DUMP BANK 6 out\n");
            for (ii = 0; ii < 33; ii++) {
                HALDEBUG(ah, " %8.8x\n", pRfBanks->Bank6Data[ii]);
            }
        }
#endif
    }
#endif /* ATH_FORCE_BIAS */  

	/* Write Analog registers */
#ifdef AH_SUPPORT_AR9000
    if (AR_SREV_SOWL_10_OR_LATER(ah)) {
		/* Setup Bank 7 Setup */
		RF_BANK_SETUP_SOWL(pRfBanks, 7, 1);
		/* Write Analog registers */
		REG_WRITE_RF_ARRAY(ar5416Bank0_sowl, pRfBanks->Bank0Data, regWrites);
		REG_WRITE_RF_ARRAY(ar5416Bank1_sowl, pRfBanks->Bank1Data, regWrites);
		REG_WRITE_RF_ARRAY(ar5416Bank2_sowl, pRfBanks->Bank2Data, regWrites);
		REG_WRITE_RF_ARRAY(ar5416Bank3_sowl, pRfBanks->Bank3Data, regWrites);
		REG_WRITE_RF_ARRAY(ar5416Bank6_sowl, pRfBanks->Bank6Data, regWrites);
		REG_WRITE_RF_ARRAY(ar5416Bank7_sowl, pRfBanks->Bank7Data, regWrites);
	} else
#endif
	{
	/* Setup Bank 7 Setup */
	RF_BANK_SETUP(pRfBanks, 7, 1);

	/* Write Analog registers */
	REG_WRITE_RF_ARRAY(ar5416Bank0, pRfBanks->Bank0Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5416Bank1, pRfBanks->Bank1Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5416Bank2, pRfBanks->Bank2Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5416Bank3, pRfBanks->Bank3Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5416Bank6, pRfBanks->Bank6Data, regWrites);
	REG_WRITE_RF_ARRAY(ar5416Bank7, pRfBanks->Bank7Data, regWrites);
    }
	return AH_TRUE;
#undef  RF_BANK_SETUP
}

/*
 * Read the transmit power levels from the structures taken from EEPROM
 * Interpolate read transmit power values for this channel
 * Organize the transmit power values into a table for writing into the hardware
 */

static HAL_BOOL
ar2133SetPowerTable(struct ath_hal *ah, int16_t *pPowerMin, int16_t *pPowerMax, 
	HAL_CHANNEL_INTERNAL *chan, u_int16_t *rfXpdGain)
{
	return AH_TRUE;
}

/*
 * Free memory for analog bank scratch buffers
 */
static void
ar2133Detach(struct ath_hal *ah)
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

#if 0
static int16_t
ar2133GetMinPower(struct ath_hal *ah, EXPN_DATA_PER_CHANNEL_5112 *data)
{
    int i, minIndex;
    int16_t minGain,minPwr,minPcdac,retVal;

    /* Assume NUM_POINTS_XPD0 > 0 */
    minGain = data->pDataPerXPD[0].xpd_gain;
    for (minIndex=0,i=1; i<NUM_XPD_PER_CHANNEL; i++) {
        if (data->pDataPerXPD[i].xpd_gain < minGain) {
            minIndex = i;
            minGain = data->pDataPerXPD[i].xpd_gain;
        }
    }
    minPwr = data->pDataPerXPD[minIndex].pwr_t4[0];
    minPcdac = data->pDataPerXPD[minIndex].pcdac[0];
    for (i=1; i<NUM_POINTS_XPD0; i++) {
        if (data->pDataPerXPD[minIndex].pwr_t4[i] < minPwr) {
            minPwr = data->pDataPerXPD[minIndex].pwr_t4[i];
            minPcdac = data->pDataPerXPD[minIndex].pcdac[i];
        }
    }
    retVal = minPwr - (minPcdac*2);
    return(retVal);
}
static HAL_BOOL
ar2133GetChannelMaxMinPower(struct ath_hal *ah, HAL_CHANNEL *chan, int16_t *maxPow,
                int16_t *minPow)
{
    struct ath_hal_5416 *ahp = (struct ath_hal_5416 *) ah;
    int numChannels=0,i,last;
    int totalD, totalF,totalMin;
    EXPN_DATA_PER_CHANNEL_5112 *data=AH_NULL;
    EEPROM_POWER_EXPN_5112 *powerArray=AH_NULL;

    *maxPow = 0;
    if (IS_CHAN_A(chan)) {
        powerArray = ahp->ah_modePowerArray5112;
        data = powerArray[headerInfo11A].pDataPerChannel;
        numChannels = powerArray[headerInfo11A].numChannels;
    } else if (IS_CHAN_G(chan) || IS_CHAN_108G(chan)) {
        /* XXX - is this correct? Should we also use the same power for turbo G? */
        powerArray = ahp->ah_modePowerArray5112;
        data = powerArray[headerInfo11G].pDataPerChannel;
        numChannels = powerArray[headerInfo11G].numChannels;
    } else if (IS_CHAN_B(chan)) {
        powerArray = ahp->ah_modePowerArray5112;
        data = powerArray[headerInfo11B].pDataPerChannel;
        numChannels = powerArray[headerInfo11B].numChannels;
    } else {
        return (AH_TRUE);
    }
    /* Make sure the channel is in the range of the TP values
     *  (freq piers)
     */
    if ((numChannels < 1) ||
        (chan->channel < data[0].channelValue) ||
        (chan->channel > data[numChannels-1].channelValue))
        return(AH_FALSE);

    /* Linearly interpolate the power value now */
    for (last=0,i=0;
         (i<numChannels) && (chan->channel > data[i].channelValue);
         last=i++);
    totalD = data[i].channelValue - data[last].channelValue;
    if (totalD > 0) {
        totalF = data[i].maxPower_t4 - data[last].maxPower_t4;
        *maxPow = (int8_t) ((totalF*(chan->channel-data[last].channelValue) + data[last].maxPower_t4*totalD)/totalD);

        totalMin = ar2133GetMinPower(ah,&data[i]) - ar2133GetMinPower(ah, &data[last]);
        *minPow = (int8_t) ((totalMin*(chan->channel-data[last].channelValue) + ar2133GetMinPower(ah, &data[last])*totalD)/totalD);
        return (AH_TRUE);
    } else {
        if (chan->channel == data[i].channelValue) {
            *maxPow = data[i].maxPower_t4;
            *minPow = ar2133GetMinPower(ah, &data[i]);
            return(AH_TRUE);
        } else
            return(AH_FALSE);
    }
}
#endif
HAL_BOOL
ar2133GetChipPowerLimits(struct ath_hal *ah, HAL_CHANNEL *chans, u_int32_t nchans)
{
    HAL_BOOL retVal = AH_TRUE;
    int i;
    //int16_t maxPow,minPow;

    for (i=0; i < nchans; i ++) {
        chans[i].maxTxPower = AR5416_MAX_RATE_POWER;
        chans[i].minTxPower = AR5416_MAX_RATE_POWER;
    }
    return (retVal);

#if 0
    for (i=0; i<nchans; i++) {
        if (ar2133GetChannelMaxMinPower(ah, &chans[i], &maxPow, &minPow)) {
            /* XXX -Need to adjust pcdac values to indicate dBm */
            chans[i].maxTxPower = maxPow;
            chans[i].minTxPower = minPow;
        } else {
            HALDEBUG(ah, "Failed setting power table for nchans=%d\n",i);
            retVal= AH_FALSE;
        }
    }
#ifdef AH_DEBUG
    for (i=0; i<nchans; i++) {
        HALDEBUG(ah,"Chan %d: MaxPow = %d MinPow = %d\n",
             chans[i].channel,chans[i].maxTxPower, chans[i].minTxPower);
    }
#endif
#endif
}

/*
 * Adjust NF based on statistical values for 5GHz frequencies.
 * Stubbed:Not used by Fowl
 */
int16_t
ar2133GetNfAdjust(struct ath_hal *ah, const HAL_CHANNEL_INTERNAL *c)
{
	return 0;
}
	
/*
 * Allocate memory for analog bank scratch buffers
 * Scratch Buffer will be reinitialized every reset so no need to zero now
 */
HAL_BOOL
ar2133RfAttach(struct ath_hal *ah, HAL_STATUS *status)
{
	struct ath_hal_5212 *ahp = AH5212(ah);

	HALASSERT(ahp->ah_analogBanks == AH_NULL);
	ahp->ah_analogBanks = ath_hal_malloc(sizeof(AR5416_RF_BANKS_2133));
	if (ahp->ah_analogBanks == AH_NULL) {
		HALDEBUG(ah, "%s: cannot allocate RF banks\n", __func__);
		*status = HAL_ENOMEM;       /* XXX */
		return AH_FALSE;
	}

	HALASSERT(ahp->ah_pcdacTable == AH_NULL);

	ahp->ah_pcdacTableSize = PWR_TABLE_SIZE * sizeof(u_int16_t);
	ahp->ah_pcdacTable = ath_hal_malloc(ahp->ah_pcdacTableSize);
	if (ahp->ah_pcdacTable == AH_NULL) {
		HALDEBUG(ah, "%s: cannot allocate PCDAC table\n", __func__);
		*status = HAL_ENOMEM;       /* XXX */
		return AH_FALSE;
	}

	ahp->ah_pcdacTableSize		= PWR_TABLE_SIZE;
	ahp->ah_rfHal.rfDetach		= ar2133Detach;
	ahp->ah_rfHal.writeRegs     	= ar2133WriteRegs;
	ahp->ah_rfHal.getRfBank     	= ar2133GetRfBank;
#ifdef AH_SUPPORT_AR9200
        if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
    		ahp->ah_rfHal.setChannel    = ar9280SetChannel;
        } else {
#else
	{
#endif
    		ahp->ah_rfHal.setChannel    = ar2133SetChannel; 
        }
	ahp->ah_rfHal.setRfRegs     	= ar2133SetRfRegs;
	ahp->ah_rfHal.setPowerTable 	= ar2133SetPowerTable;
	ahp->ah_rfHal.getChipPowerLim   = ar2133GetChipPowerLimits;
	ahp->ah_rfHal.getNfAdjust	= ar2133GetNfAdjust;

	return AH_TRUE;
}

#endif /* AH_SUPPORT_2133 */
