/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5312/ar5312_misc.c#2 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5312

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5312/ar5312phy.h"

#define	AR_NUM_GPIO	6		/* 6 GPIO pins */
#define	AR_GPIOD_MASK	0x0000002F	/* GPIO data reg r/w mask */

/*
 * Change the LED blinking pattern to correspond to the connectivity
 */
void
ar5312SetLedState(struct ath_hal *ah, HAL_LED_STATE state)
{
	u_int32_t val;
	u_int32_t resOffset = (AR5312_RSTIMER_BASE - ((u_int32_t) ah->ah_sh));
    if(IS_2316(ah)) return; /* not yet */
	val = SM(AR5312_PCICFG_LEDSEL0, AR5312_PCICFG_LEDSEL) |
		SM(AR5312_PCICFG_LEDMOD0, AR5312_PCICFG_LEDMODE) |
		2;
	OS_REG_WRITE(ah, resOffset+AR5312_PCICFG,
		(OS_REG_READ(ah, AR5312_PCICFG) &~
		 (AR5312_PCICFG_LEDSEL | AR5312_PCICFG_LEDMODE |
		  AR5312_PCICFG_LEDSBR))
		     | val);
}

/*
 * Detect if our wireless mac is present. 
 */
HAL_BOOL
ar5312DetectCardPresent(struct ath_hal *ah)
{
	u_int16_t macVersion, macRev;
	u_int32_t v;

	/*
	 * Read the Silicon Revision register and compare that
	 * to what we read at attach time.  If the same, we say
	 * a card/device is present.
	 */
#if (AH_SUPPORT_2316 || AH_SUPPORT_2317)
    if(IS_5315(ah))
    {
		v = (OS_REG_READ(ah,
                         (AR5315_RSTIMER_BASE-((u_int32_t) ah->ah_sh)) + AR5315_WREV))
			& AR_SREV_ID;
		macVersion = v >> AR_SREV_ID_S;
		macRev = v & AR_SREV_REVISION;
		return (AH_PRIVATE(ah)->ah_macVersion == macVersion &&
				AH_PRIVATE(ah)->ah_macRev == macRev);
    }
    else
#endif
    {
		v = (OS_REG_READ(ah,
                         (AR5312_RSTIMER_BASE-((u_int32_t) ah->ah_sh)) + AR5312_WREV))
			& AR_SREV_ID;
		macVersion = v >> AR_SREV_ID_S;
		macRev = v & AR_SREV_REVISION;
		return (AH_PRIVATE(ah)->ah_macVersion == macVersion &&
				AH_PRIVATE(ah)->ah_macRev == macRev);
    }
}

/*
 * If 32KHz clock exists, use it to lower power consumption during sleep
 *
 * Note: If clock is set to 32 KHz, delays on accessing certain
 *       baseband registers (27-31, 124-127) are required.
 */
void
ar5312SetupClock(struct ath_hal *ah, HAL_OPMODE opmode)
{
#ifdef notyet
	if (!ar5212Use32KHzclock(ah, opmode)) {
		ar5312RestoreClock(ah, opmode);
		return;
	}
	/*
	 * Enable clocks to be turned OFF in BB during sleep
	 * and also enable turning OFF 32MHz/40MHz Refclk
	 * from A2.
	 */
	OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_CONTROL, 0x1f);
	OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_LIMIT,   0x0d);
	OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,        0x0c);
	OS_REG_WRITE(ah, AR_PHY_M_SLEEP,           0x03);
	OS_REG_WRITE(ah, AR_PHY_REFCLKDLY,         0x05);
	OS_REG_WRITE(ah, AR_PHY_REFCLKPD, (IS_5112(ah) || IS_2413(ah)) ? 0x14 : 0x18);

	OS_REG_RMW_FIELD(ah, AR_USEC, AR_USEC_USEC32, 1);
	OS_REG_WRITE(ah, AR_TSF_PARM, 61);	/* 32 KHz TSF incr */
#else
	ar5312RestoreClock(ah, opmode);
#endif
}

/*
 * If 32KHz clock exists, turn it off and turn back on the 32Mhz
 */
void
ar5312RestoreClock(struct ath_hal *ah, HAL_OPMODE opmode)
{
	/* # Set sleep clock rate back to 32 MHz. */
	OS_REG_WRITE(ah, AR_TSF_PARM, 1);	/* 32 MHz TSF incr */

	/* 2315+ uses 40MHz refclock and 2413 based Rf */
	OS_REG_RMW_FIELD(ah, AR_USEC, AR_USEC_USEC32,
			(IS_5112(ah) || IS_2413(ah)) ? 39 : 31);

	/*
	 * Restore BB registers to power-on defaults
	 */
	OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_CONTROL, 0x1f);
	OS_REG_WRITE(ah, AR_PHY_SLEEP_CTR_LIMIT,   0x7f);

	if (IS_5312_2_X(ah))
		OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,	0x04);
	else {
		OS_REG_WRITE(ah, AR_PHY_SLEEP_SCAL,	0x0e);
		/* 
		 * WAR: Don't write the other phy registers on 5312 2.x -
		 * they alias to other areas
		 */
		OS_REG_WRITE(ah, AR_PHY_M_SLEEP,           0x0c);
		OS_REG_WRITE(ah, AR_PHY_REFCLKDLY,         0xff);
		OS_REG_WRITE(ah, AR_PHY_REFCLKPD, (IS_5112(ah) || IS_2413(ah)) ? 0x14 : 0x18);
	}
}

#endif /* AH_SUPPORT_AR5312 */
