/*
 * Copyright (c) 2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 */

#ifndef _AR5416EEPROM_H_
#define _AR5416EEPROM_H_

/* reg_off = 4 * (eep_off) */
#define AR5416_EEPROM_S			2
#define AR5416_EEPROM_OFFSET		0x2000
#define AR5416_EEPROM_START_ADDR	0x503f1200
#define AR5416_EEPROM_MAX		0xae0 /* Ignore for the moment used only on the flash implementations */
#define AR5416_EEPROM_MAGIC		0xa55a
#define AR5416_EEPROM_MAGIC_OFFSET	0x0

#define owl_get_eep_ver(_eep)   \
    (((_eep)->baseEepHeader.version >> 12) & 0xF)
#define owl_get_eep_rev(_eep)   \
    (((_eep)->baseEepHeader.version) & 0xFFF)
#define owl_get_ntxchains(_txchainmask) \
    (((_txchainmask >> 2) & 1) + ((_txchainmask >> 1) & 1) + (_txchainmask & 1))

#ifdef __LINUX_ARM_ARCH__ /* AP71 */
#define owl_eep_start_loc		0
#else
#define owl_eep_start_loc		256
#endif

/* End temp defines */

#define AR5416_OPFLAGS_11A           0x01
#define AR5416_OPFLAGS_11G           0x02
#define AR5416_OPFLAGS_5G_HT40       0x04
#define AR5416_OPFLAGS_2G_HT40       0x08
#define AR5416_OPFLAGS_5G_HT20       0x10
#define AR5416_OPFLAGS_2G_HT20       0x20

/* RF silent fields in EEPROM */
#define EEP_RFSILENT_ENABLED        	1
#define EEP_RFSILENT_POLARITY       	0x0002
#define EEP_RFSILENT_POLARITY_S     	1
#define EEP_RFSILENT_GPIO_SEL       	0x001c
#define EEP_RFSILENT_GPIO_SEL_S     	2

#define AR5416_EEP_NO_BACK_VER       	0x1
#define AR5416_EEP_VER               	0xE
#define AR5416_EEP_VER_MINOR_MASK	0xFFF
#define AR5416_EEP_MINOR_VER_2		0x2  // Adds modal params txFrameToPaOn, txFrametoDataStart, ht40PowerInc
#define AR5416_EEP_MINOR_VER_3		0x3  // Adds modal params bswAtten, bswMargin, swSettle and base OpFlags for HT20/40 Disable
#define AR5416_EEP_MINOR_VER_7 	0x7
#define AR5416_EEP_MINOR_VER_9 	0x9

// 16-bit offset location start of calibration struct
#define AR5416_EEP_START_LOC         	256
#define AR5416_NUM_5G_CAL_PIERS      	8
#define AR5416_NUM_2G_CAL_PIERS      	4
#define AR5416_NUM_5G_20_TARGET_POWERS  8
#define AR5416_NUM_5G_40_TARGET_POWERS  8
#define AR5416_NUM_2G_CCK_TARGET_POWERS 3
#define AR5416_NUM_2G_20_TARGET_POWERS  4
#define AR5416_NUM_2G_40_TARGET_POWERS  4
#define AR5416_NUM_CTLS              	24
#define AR5416_NUM_BAND_EDGES        	8
#define AR5416_NUM_PD_GAINS          	4
#define AR5416_PD_GAINS_IN_MASK      	4
#define AR5416_PD_GAIN_ICEPTS        	5
#define AR5416_EEPROM_MODAL_SPURS    	5
#define AR5416_MAX_RATE_POWER        	63
#define AR5416_NUM_PDADC_VALUES      	128
#define AR5416_NUM_RATES             	16
#define AR5416_BCHAN_UNUSED          	0xFF
#define AR5416_MAX_PWR_RANGE_IN_HALF_DB 64
#define AR5416_EEPMISC_BIG_ENDIAN    	0x01
#define FREQ2FBIN(x,y) 			((y) ? ((x) - 2300) : (((x) - 4800) / 5))
#define AR5416_MAX_CHAINS            	3
#define AR5416_ANT_16S               	25

#define AR5416_NUM_ANT_CHAIN_FIELDS     7
#define AR5416_NUM_ANT_COMMON_FIELDS    4
#define AR5416_SIZE_ANT_CHAIN_FIELD     3
#define AR5416_SIZE_ANT_COMMON_FIELD    4
#define AR5416_ANT_CHAIN_MASK           0x7
#define AR5416_ANT_COMMON_MASK          0xf
#define AR5416_CHAIN_0_IDX              0
#define AR5416_CHAIN_1_IDX              1
#define AR5416_CHAIN_2_IDX              2

typedef struct BaseEepHeader {
	u_int16_t	length;
	u_int16_t	checksum;
	u_int16_t	version;
	u_int8_t	opCapFlags;
	u_int8_t	eepMisc;
	u_int16_t	regDmn[2];
	u_int8_t	macAddr[6];
	u_int8_t	rxMask;
	u_int8_t	txMask;
	u_int16_t	rfSilent;
	u_int16_t	blueToothOptions;
	u_int16_t	deviceCap;
	u_int32_t	binBuildNumber;
	u_int8_t	deviceType;
        u_int8_t        pwdclkind;
        u_int8_t        futureBase[32];
} __packed BASE_EEP_HEADER; // 64 B

typedef struct spurChanStruct {
	u_int16_t	spurChan;
	u_int8_t	spurRangeLow;
	u_int8_t	spurRangeHigh;
} __packed SPUR_CHAN;

typedef struct ModalEepHeader {
	u_int32_t	antCtrlChain[AR5416_MAX_CHAINS];	// 12
	u_int32_t	antCtrlCommon;				// 4
	int8_t		antennaGainCh[AR5416_MAX_CHAINS];	// 3
	u_int8_t	switchSettling;				// 1
	u_int8_t	txRxAttenCh[AR5416_MAX_CHAINS];		// 3
	u_int8_t	rxTxMarginCh[AR5416_MAX_CHAINS];	// 3
	u_int8_t	adcDesiredSize;				// 1
	int8_t		pgaDesiredSize;				// 1
	u_int8_t	xlnaGainCh[AR5416_MAX_CHAINS];		// 3
	u_int8_t	txEndToXpaOff;				// 1
	u_int8_t	txEndToRxOn;				// 1
	u_int8_t	txFrameToXpaOn;				// 1
	u_int8_t	thresh62;				// 1
	u_int8_t	noiseFloorThreshCh[AR5416_MAX_CHAINS];	// 3
	u_int8_t	xpdGain;				// 1
	u_int8_t	xpd;					// 1
	int8_t		iqCalICh[AR5416_MAX_CHAINS];		// 1
	int8_t		iqCalQCh[AR5416_MAX_CHAINS];		// 1
	u_int8_t	pdGainOverlap;				// 1
	u_int8_t	ob;					// 1
	u_int8_t	db;					// 1
	u_int8_t	xpaBiasLvl;				// 1
	u_int8_t	pwrDecreaseFor2Chain;			// 1
	u_int8_t	pwrDecreaseFor3Chain;			// 1 -> 48 B
	u_int8_t	txFrameToDataStart;			// 1
	u_int8_t	txFrameToPaOn;				// 1
	u_int8_t	ht40PowerIncForPdadc;			// 1
	u_int8_t	bswAtten[AR5416_MAX_CHAINS];		// 3
	u_int8_t	bswMargin[AR5416_MAX_CHAINS];		// 3
	u_int8_t	swSettleHt40;				// 1	
	u_int8_t   xatten2Db[AR5416_MAX_CHAINS];          // 3 -> New for AR9280 (0xa20c/b20c 11:6)
	u_int8_t   xatten2Margin[AR5416_MAX_CHAINS];      // 3 -> New for AR9280 (0xa20c/b20c 21:17)
	u_int8_t   ob_ch1;                                // 1 -> ob and db become chain specific from AR9280
	u_int8_t   db_ch1;                                // 1 
    u_int8_t   lna_cntl_sp          : 1,              // 1 Spare bit
               force_xpaon          : 1,              // Force XPA bit for 5G mode
               local_bias           : 1,              // enable local bias
               femBandSelectUsed    : 1,              //
               xlnabufin            : 1,              //
               xlnaisel             : 2,              //
               xlnabufmode          : 1;              //
	u_int8_t   futureModalMerlin;                     // 1
	u_int16_t  xpaBiasLvlFreq[3];                     // 3
	u_int8_t	futureModal[6];			// 30 B
	SPUR_CHAN spurChans[AR5416_EEPROM_MODAL_SPURS];		// 20 B
} __packed MODAL_EEP_HEADER;					// == 100 B    

typedef struct calDataPerFreq {
	u_int8_t	pwrPdg[AR5416_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
	u_int8_t	vpdPdg[AR5416_NUM_PD_GAINS][AR5416_PD_GAIN_ICEPTS];
} __packed CAL_DATA_PER_FREQ;

typedef struct CalTargetPowerLegacy {
	u_int8_t	bChannel;
	u_int8_t	tPow2x[4];
} __packed CAL_TARGET_POWER_LEG;

typedef struct CalTargetPowerHt {
	u_int8_t	bChannel;
	u_int8_t	tPow2x[8];
} __packed CAL_TARGET_POWER_HT;

#if AH_BYTE_ORDER == AH_BIG_ENDIAN
typedef struct CalCtlEdges {
	u_int8_t	bChannel;
	u_int8_t	flag   :2,
				tPower :6;
} __packed CAL_CTL_EDGES;
#else
typedef struct CalCtlEdges {
	u_int8_t  bChannel;
	u_int8_t  tPower :6,
		  flag   :2;
} __packed CAL_CTL_EDGES;
#endif

typedef struct CalCtlData {
	CAL_CTL_EDGES		ctlEdges[AR5416_MAX_CHAINS][AR5416_NUM_BAND_EDGES];
} __packed CAL_CTL_DATA;

struct ar5416eeprom {
	BASE_EEP_HEADER		baseEepHeader;         // 64 B
	u_int8_t			custData[64];          // 64 B
	MODAL_EEP_HEADER		modalHeader[2];        // 200 B
	u_int8_t			calFreqPier5G[AR5416_NUM_5G_CAL_PIERS];
	u_int8_t			calFreqPier2G[AR5416_NUM_2G_CAL_PIERS];
	CAL_DATA_PER_FREQ		calPierData5G[AR5416_MAX_CHAINS][AR5416_NUM_5G_CAL_PIERS];
	CAL_DATA_PER_FREQ		calPierData2G[AR5416_MAX_CHAINS][AR5416_NUM_2G_CAL_PIERS];
	CAL_TARGET_POWER_LEG	calTargetPower5G[AR5416_NUM_5G_20_TARGET_POWERS];
	CAL_TARGET_POWER_HT		calTargetPower5GHT20[AR5416_NUM_5G_20_TARGET_POWERS];
	CAL_TARGET_POWER_HT		calTargetPower5GHT40[AR5416_NUM_5G_40_TARGET_POWERS];
	CAL_TARGET_POWER_LEG	calTargetPowerCck[AR5416_NUM_2G_CCK_TARGET_POWERS];
	CAL_TARGET_POWER_LEG	calTargetPower2G[AR5416_NUM_2G_20_TARGET_POWERS];
	CAL_TARGET_POWER_HT		calTargetPower2GHT20[AR5416_NUM_2G_20_TARGET_POWERS];
	CAL_TARGET_POWER_HT		calTargetPower2GHT40[AR5416_NUM_2G_40_TARGET_POWERS];
	u_int8_t			ctlIndex[AR5416_NUM_CTLS];
	CAL_CTL_DATA		ctlData[AR5416_NUM_CTLS];
	u_int8_t			padding;			
} __packed;
#endif /* _AR_5416EEPROM_H_ */
