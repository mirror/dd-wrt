/*
 * Copyright (c) 2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416.h#3 $
 */
#ifndef _ATH_AR5416_H_
#define _ATH_AR5416_H_

#include "ar5212/ar5212.h"
#include "ar5416/ar5416eeprom.h"

#define	AR5416_MAGIC	0x20065416

#define AR5416_PWR_TABLE_OFFSET  -5


typedef struct {
	u_int16_t	synth_center;
	u_int16_t	ctl_center;
	u_int16_t	ext_center;
} CHAN_CENTERS;

#ifdef AH_SUPPORT_IME
#define	AR5416_DEFAULT_RXCHAINMASK	5
#define	AR5416_DEFAULT_TXCHAINMASK	5
#else
#define	AR5416_DEFAULT_RXCHAINMASK	7
#define	AR5416_DEFAULT_TXCHAINMASK	7
#endif

#define	AR5416_MAX_RATE_POWER		63
#define	AR5416_KEYTABLE_SIZE		128

struct ath_hal_5416 {
	struct ath_hal_5212 ah_5212;

	struct ar5416eeprom ah_5416eeprom;
	u_int       	ah_globaltxtimeout;	/* global tx timeout */
	int		ah_clksel;
	u_int8_t	ah_keytype[AR5416_KEYTABLE_SIZE];
	/*
	 * Extension Channel Rx Clear State
	 */
	u_int32_t	ah_cycleCount;
	u_int32_t	ah_ctlBusy;
	u_int32_t	ah_extBusy;
	u_int32_t	ah_rx_chainmask;
	u_int32_t	ah_tx_chainmask;
};
#define	AH5416(_ah)	((struct ath_hal_5416 *)(_ah))

#define IS_5416_PCI(ah) ((AH_PRIVATE(ah)->ah_macVersion) == AR_SREV_VERSION_OWL_PCI)
#define IS_5416_PCIE(ah) ((AH_PRIVATE(ah)->ah_macVersion) == AR_SREV_VERSION_OWL_PCIE)
#undef IS_PCIE
#define IS_PCIE(ah) (IS_5416_PCIE(ah))

#define IS_5416(ah) (IS_5416_PCIE(ah) || IS_5416_PCI(ah))

extern	HAL_BOOL ar2133RfAttach(struct ath_hal *, HAL_STATUS *);

struct ath_hal;

extern	struct ath_hal * ar5416Attach(u_int16_t devid, HAL_SOFTC sc,
		HAL_BUS_TAG st, HAL_BUS_HANDLE sh, HAL_STATUS *status);
extern	void ar5416Detach(struct ath_hal *ah);

typedef enum {
	EEP_NFTHRESH_5,
	EEP_NFTHRESH_2,
	EEP_MAC_MSW,
	EEP_MAC_MID,
	EEP_MAC_LSW,
	EEP_REG_0,
	EEP_REG_1,
	EEP_OP_CAP,
	EEP_OP_MODE,
	EEP_RF_SILENT,
	EEP_OB_5,
	EEP_DB_5,
	EEP_OB_2,
	EEP_DB_2,
	EEP_MINOR_REV,
	EEP_TXMASK,
	EEP_RXMASK,
} EEPROM_PARAM;
extern	u_int32_t ar5416EepromGet(struct ath_hal *, EEPROM_PARAM param);

extern	HAL_BOOL ar5416AniAttach(struct ath_hal *ah);
extern	void ar5416AniDetach(struct ath_hal *ah);

extern	void ar5416SetBeaconTimers(struct ath_hal *, const HAL_BEACON_TIMERS *);
extern	void ar5416BeaconInit(struct ath_hal *ah,
		u_int32_t next_beacon, u_int32_t beacon_period);
extern	void ar5416ResetStaBeaconTimers(struct ath_hal *ah);
extern	void ar5416SetStaBeaconTimers(struct ath_hal *ah,
		const HAL_BEACON_STATE *);

extern	HAL_BOOL ar5416EepromRead(struct ath_hal *, u_int off, u_int16_t *data);
extern	HAL_BOOL ar5416EepromWrite(struct ath_hal *, u_int off, u_int16_t data);

extern	HAL_BOOL ar5416IsInterruptPending(struct ath_hal *ah);
extern	HAL_BOOL ar5416GetPendingInterrupts(struct ath_hal *, HAL_INT *masked);
extern	HAL_INT ar5416SetInterrupts(struct ath_hal *ah, HAL_INT ints);

extern	HAL_BOOL ar5416GpioCfgOutput(struct ath_hal *, u_int32_t gpio);
extern	HAL_BOOL ar5416GpioCfgInput(struct ath_hal *, u_int32_t gpio);
extern	HAL_BOOL ar5416GpioSet(struct ath_hal *, u_int32_t gpio, u_int32_t val);
extern	u_int32_t ar5416GpioGet(struct ath_hal *ah, u_int32_t gpio);
extern	void ar5416GpioSetIntr(struct ath_hal *ah, u_int, u_int32_t ilevel);

extern	u_int ar5416GetWirelessModes(struct ath_hal *ah);
extern	void ar5416SetLedState(struct ath_hal *ah, HAL_LED_STATE state);
extern	void ar5416ResetTsf(struct ath_hal *ah);
extern	HAL_BOOL ar5416SetAntennaSwitch(struct ath_hal *, HAL_ANT_SETTING);
extern	HAL_BOOL ar5416SetDecompMask(struct ath_hal *, u_int16_t, int);
extern	void ar5416SetCoverageClass(struct ath_hal *, u_int8_t, int);
extern	void ar5416GetCCAThreshold(struct ath_hal *, int8_t thresh62[2]);
extern	HAL_BOOL ar5416SetCCAThreshold(struct ath_hal *,
		const int8_t thresh62[2]);
extern	u_int32_t ar5416Get11nExtBusy(struct ath_hal *ah);
extern	void ar5416Set11nMac2040(struct ath_hal *ah, HAL_HT_MACMODE mode);
extern	HAL_HT_RXCLEAR ar5416Get11nRxClear(struct ath_hal *ah);
extern	void ar5416Set11nRxClear(struct ath_hal *ah, HAL_HT_RXCLEAR rxclear);
extern	HAL_STATUS ar5416GetCapability(struct ath_hal *, HAL_CAPABILITY_TYPE,
		u_int32_t, u_int32_t *);
extern	HAL_BOOL ar5416SetCapability(struct ath_hal *, HAL_CAPABILITY_TYPE,
		u_int32_t, u_int32_t, HAL_STATUS *);

extern	HAL_BOOL ar5416SetPowerMode(struct ath_hal *ah, HAL_POWER_MODE mode,
		int setChip);
extern	HAL_POWER_MODE ar5416GetPowerMode(struct ath_hal *ah);
extern	HAL_BOOL ar5416GetPowerStatus(struct ath_hal *ah);

extern	HAL_BOOL ar5416ResetKeyCacheEntry(struct ath_hal *ah, u_int16_t entry);
extern	HAL_BOOL ar5416SetKeyCacheEntry(struct ath_hal *ah, u_int16_t entry,
	       const HAL_KEYVAL *k, const u_int8_t *mac, int xorKey);

extern	void ar5416StartPcuReceive(struct ath_hal *ah);
extern	void ar5416StopPcuReceive(struct ath_hal *ah);
extern	HAL_BOOL ar5416SetupRxDesc(struct ath_hal *,
		struct ath_desc *, u_int32_t size, u_int flags);
extern	HAL_STATUS ar5416ProcRxDesc(struct ath_hal *ah, struct ath_desc *,
		u_int32_t, struct ath_desc *, u_int64_t,
		struct ath_rx_status *);

extern	HAL_BOOL ar5416Reset(struct ath_hal *ah, HAL_OPMODE opmode,
		HAL_CHANNEL *chan, HAL_RESET_TYPE resetType, HAL_STATUS *status);
extern	HAL_BOOL ar5416PhyDisable(struct ath_hal *ah);
extern	HAL_RFGAIN ar5416GetRfgain(struct ath_hal *ah);
extern	HAL_BOOL ar5416Disable(struct ath_hal *ah);
extern	HAL_BOOL ar5416ChipReset(struct ath_hal *ah, HAL_CHANNEL *);
extern	HAL_BOOL ar5416PerCalibration(struct ath_hal *,  HAL_CHANNEL *, HAL_BOOL longCal,
		HAL_BOOL *isIQdone);
extern	HAL_BOOL ar5416SetResetReg(struct ath_hal *, u_int32_t type);
extern	HAL_BOOL ar5416SetTxPowerLimit(struct ath_hal *ah, u_int32_t limit);
extern	HAL_BOOL ar5416GetChipPowerLimits(struct ath_hal *ah,
		HAL_CHANNEL *chans, u_int32_t nchans);
extern	void ar5416GetChannelCenters(struct ath_hal *,
		HAL_CHANNEL_INTERNAL *chan, CHAN_CENTERS *centers);

extern	HAL_BOOL ar5416SetupTxDesc(struct ath_hal *ah, struct ath_desc *ds,
		u_int pktLen, u_int hdrLen, HAL_PKT_TYPE type, u_int txPower,
		u_int txRate0, u_int txTries0,
		u_int keyIx, u_int antMode, u_int flags,
		u_int rtsctsRate, u_int rtsctsDuration,
		u_int compicvLen, u_int compivLen, u_int comp);
extern	HAL_BOOL ar5416SetupXTxDesc(struct ath_hal *, struct ath_desc *,
		u_int txRate1, u_int txRetries1,
		u_int txRate2, u_int txRetries2,
		u_int txRate3, u_int txRetries3);
extern	HAL_BOOL ar5416FillTxDesc(struct ath_hal *ah, struct ath_desc *ds,
		u_int segLen, HAL_BOOL firstSeg, HAL_BOOL lastSeg,
		const struct ath_desc *ds0);
extern	HAL_STATUS ar5416ProcTxDesc(struct ath_hal *ah,
		struct ath_desc *, struct ath_tx_status *);

extern	const HAL_RATE_TABLE *ar5416GetRateTable(struct ath_hal *, u_int mode);
extern HAL_BOOL ar5416DetectBbHang(struct ath_hal *ah, HAL_BOOL dfs_hang_check);

#endif	/* _ATH_AR5416_H_ */
