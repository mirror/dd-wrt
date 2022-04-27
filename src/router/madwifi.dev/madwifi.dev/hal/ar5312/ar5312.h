/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5312/ar5312.h#3 $
 */
#ifndef _ATH_AR5312_H_
#define _ATH_AR5312_H_

#include "ah_soc.h"
#include "ar5212/ar5212.h"

#define AR5312_UNIT(_ah) \
	(((const struct ar531x_config *)((_ah)->ah_st))->unit)
#define AR5312_BOARDCONFIG(_ah) \
	(((const struct ar531x_config *)((_ah)->ah_st))->board)
#define AR5312_RADIOCONFIG(_ah) \
	(((const struct ar531x_config *)((_ah)->ah_st))->radio)

/*
#if defined(AH_SUPPORT_AR5312)
#define	IS_5312_2_X(ah) (1)
#else
#define	IS_5312_2_X(ah) (0)
#endif
*/
/*#if defined(AH_SUPPORT_2316) || defined(AH_SUPPORT_2315)
#define IS_5315(ah) (1)
#else
#define IS_5315(ah) (0)
#endif
*/



#define	IS_5312_2_X(ah) \
	(((AH_PRIVATE(ah)->ah_macVersion) == AR_SREV_VERSION_VENICE) && \
	 (((AH_PRIVATE(ah)->ah_macRev) == 2) || ((AH_PRIVATE(ah)->ah_macRev) == 7)))


#define IS_5315(ah) \
	((AH_PRIVATE(ah)->ah_devid == AR5212_AR2315_REV6) || \
	 (AH_PRIVATE(ah)->ah_devid == AR5212_AR2315_REV7) || \
	 (AH_PRIVATE(ah)->ah_devid == AR5212_AR2317_REV1) || \
	 (AH_PRIVATE(ah)->ah_devid == AR5212_AR2317_REV2))


extern	struct ath_hal * ar5312Attach(u_int16_t devid, HAL_SOFTC sc,
				      HAL_BUS_TAG st, HAL_BUS_HANDLE sh, HAL_STATUS *status);
extern  HAL_BOOL ar5312IsInterruptPending(struct ath_hal *ah);

/* AR5312 */
extern	HAL_BOOL ar5312GpioCfgOutput(struct ath_hal *, u_int32_t gpio);
extern	HAL_BOOL ar5312GpioCfgInput(struct ath_hal *, u_int32_t gpio);
extern	HAL_BOOL ar5312GpioSet(struct ath_hal *, u_int32_t gpio, u_int32_t val);
extern	u_int32_t ar5312GpioGet(struct ath_hal *ah, u_int32_t gpio);
extern	void ar5312GpioSetIntr(struct ath_hal *ah, u_int, u_int32_t ilevel);

/* AR2315+ */
extern	HAL_BOOL ar5315GpioCfgOutput(struct ath_hal *, u_int32_t gpio);
extern	HAL_BOOL ar5315GpioCfgInput(struct ath_hal *, u_int32_t gpio);
extern	HAL_BOOL ar5315GpioSet(struct ath_hal *, u_int32_t gpio, u_int32_t val);
extern	u_int32_t ar5315GpioGet(struct ath_hal *ah, u_int32_t gpio);
extern	void ar5315GpioSetIntr(struct ath_hal *ah, u_int, u_int32_t ilevel);

extern  void ar5312SetLedState(struct ath_hal *ah, HAL_LED_STATE state);
extern  HAL_BOOL ar5312DetectCardPresent(struct ath_hal *ah);
extern  void ar5312SetupClock(struct ath_hal *ah, HAL_OPMODE opmode);
extern  void ar5312RestoreClock(struct ath_hal *ah, HAL_OPMODE opmode);
extern  void ar5312DumpState(struct ath_hal *ah);
extern  HAL_BOOL ar5312Reset(struct ath_hal *ah, HAL_OPMODE opmode,
              HAL_CHANNEL *chan, HAL_RESET_TYPE resetType, HAL_STATUS *status);
extern  HAL_BOOL ar5312ChipReset(struct ath_hal *ah, HAL_CHANNEL *chan);
extern  HAL_BOOL ar5312SetPowerMode(struct ath_hal *ah, HAL_POWER_MODE mode,
                                    int setChip);
extern  HAL_BOOL ar5312PhyDisable(struct ath_hal *ah);
extern  HAL_BOOL ar5312Disable(struct ath_hal *ah);
extern  HAL_BOOL ar5312MacReset(struct ath_hal *ah, unsigned int RCMask);
extern  u_int32_t ar5312GetPowerMode(struct ath_hal *ah);
extern  HAL_BOOL ar5312GetPowerStatus(struct ath_hal *ah);

/* BSP functions */
extern	HAL_BOOL ar5312EepromRead(struct ath_hal *, u_int off, u_int16_t *data);
extern	HAL_BOOL ar5312EepromWrite(struct ath_hal *, u_int off, u_int16_t data);

#endif	/* _ATH_AR3212_H_ */
