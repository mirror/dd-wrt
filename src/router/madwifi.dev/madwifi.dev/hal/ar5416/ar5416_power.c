/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_power.c#2 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_internal.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"

/*
 * Notify Power Mgt is enabled in self-generated frames.
 * If requested, force chip awake.
 *
 * Returns A_OK if chip is awake or successfully forced awake.
 *
 * WARNING WARNING WARNING
 * There is a problem with the chip where sometimes it will not wake up.
 */
static HAL_BOOL
ar5416SetPowerModeAwake(struct ath_hal *ah, int setChip)
{
#define	POWER_UP_TIME	200000
	u_int32_t val, val0;
	int i = 0;

	if (setChip) {
		/*
		 * Do a Power-On-Reset if OWL is shutdown
		 * the NetBSD driver  power-cycles the Cardbus slot
		 * as part of the reset procedure.
		 */
		if ((OS_REG_READ(ah, AR_RTC_STATUS) 
			& AR_RTC_PM_STATUS_M) == AR_RTC_STATUS_SHUTDOWN) {
			if (!ar5416SetResetReg(ah, HAL_RESET_POWER_ON))
				goto bad;			
		}

		OS_REG_SET_BIT(ah, AR_RTC_FORCE_WAKE, AR_RTC_FORCE_WAKE_EN);
		OS_DELAY(2000);   /* Give chip the chance to awake */

		for (i = POWER_UP_TIME / 20000; i != 0; i--) {
			val0 = OS_REG_READ(ah, AR_RTC_STATUS);
			val = OS_REG_READ(ah, AR_RTC_STATUS) & AR_RTC_STATUS_M;
			if (val == AR_RTC_STATUS_ON)
				break;
			OS_DELAY(20000);
			OS_REG_SET_BIT(ah, AR_RTC_FORCE_WAKE, AR_RTC_FORCE_WAKE_EN);
		}		
	bad:
		if (i == 0) {
#ifdef AH_DEBUG
			HALDEBUG(ah, "%s: Failed to wakeup in %ums\n",
				__func__, POWER_UP_TIME/20);
#endif
			return AH_FALSE;
		}
	} 

	OS_REG_SET_BIT(ah, AR_RTC_FORCE_WAKE, AR_RTC_FORCE_WAKE_EN|AR_RTC_FORCE_WAKE_ON_INT);
	OS_REG_CLR_BIT(ah, AR_STA_ID1, AR_STA_ID1_PWR_SAV);
	return AH_TRUE;
#undef POWER_UP_TIME
}

/*
 * Notify Power Mgt is disabled in self-generated frames.
 * If requested, force chip to sleep.
 */
static void
ar5416SetPowerModeSleep(struct ath_hal *ah, int setChip)
{
	OS_REG_SET_BIT(ah, AR_STA_ID1, AR_STA_ID1_PWR_SAV);
	if (setChip) {
		/* Clear the RTC force wake bit to allow the mac to sleep */
		OS_REG_CLR_BIT(ah, AR_RTC_FORCE_WAKE, AR_RTC_FORCE_WAKE_EN);
		/* XXX don't do this for howl */
//		if (!AR_SREV_HOWL(ah))
//			OS_REG_WRITE(ah, AR_RC, AR_RC_AHB|AR_RC_HOSTIF);
		/* Shutdown chip. Active low */
		OS_REG_CLR_BIT(ah, AR_RTC_RESET, AR_RTC_RESET_EN);
	}
}

/*
 * Notify Power Management is enabled in self-generating
 * fames.  If request, set power mode of chip to
 * auto/normal.  Duration in units of 128us (1/8 TU).
 */
static void
ar5416SetPowerModeNetworkSleep(struct ath_hal *ah, int setChip)
{
	OS_REG_SET_BIT(ah, AR_STA_ID1, AR_STA_ID1_PWR_SAV);
	
	if (setChip) {
		/* XXX same as ar5212SetPowerModeSleep() ??? */
		OS_REG_CLR_BIT(ah, AR_RTC_FORCE_WAKE, AR_RTC_FORCE_WAKE_EN);
		/* Allow 3ms for transition to sleep mode */ 
		OS_DELAY(3000); 	
	}
}

/*
 * Set power mgt to the requested mode, and conditionally set
 * the chip as well
 */
HAL_BOOL
ar5416SetPowerMode(struct ath_hal *ah, HAL_POWER_MODE mode, int setChip)
{
	struct ath_hal_5212 *ahp = AH5212(ah);
#ifdef AH_DEBUG
	static const char* modes[] = {
		"AWAKE",
		"FULL-SLEEP",
		"NETWORK SLEEP",
		"UNDEFINED"
	};
#endif
	int status = AH_TRUE;
	if (!setChip)
		return AH_TRUE;

#ifdef AH_DEBUG
	HALDEBUG(ah, "%s: %s -> %s (%s)\n", __func__,
		modes[ahp->ah_powerMode], modes[mode],
		setChip ? "set chip " : "");
		
#endif
	switch (mode) {
	case HAL_PM_AWAKE:
		status = ar5416SetPowerModeAwake(ah, setChip);
		break;
	case HAL_PM_FULL_SLEEP:
		ar5416SetPowerModeSleep(ah, setChip);
		break;
	case HAL_PM_NETWORK_SLEEP:
		ar5416SetPowerModeNetworkSleep(ah, setChip);
		break;
	default:
		HALDEBUG(ah, "%s: unknown power mode 0x%x\n", __func__, mode);
		return AH_FALSE;
	}
	ahp->ah_powerMode = mode;
	return status;
}

/*
 * Return the current sleep mode of the chip
 */
HAL_POWER_MODE
ar5416GetPowerMode(struct ath_hal *ah)
{
	int mode = OS_REG_READ(ah, AR_RTC_STATUS);
	switch (mode & AR_RTC_PM_STATUS_M) {
	case AR_RTC_STATUS_ON:
	case AR_RTC_STATUS_WAKEUP:
		return HAL_PM_AWAKE;
	case AR_RTC_STATUS_SLEEP:
		return HAL_PM_NETWORK_SLEEP;
	case AR_RTC_STATUS_SHUTDOWN:
		return HAL_PM_FULL_SLEEP;
	default:
		HALDEBUG(ah, "%s: unknown power mode, RTC_STAYUS 0x%x\n",
			__func__, mode);
		return HAL_PM_UNDEFINED;	
	}
}
#endif /* AH_SUPPORT_AR5416 */
