/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5312/ar5312_gpio.c#2 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5312

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5312/ar5312phy.h"

#define	AR_NUM_GPIO	8		/* 8 GPIO pins */
#define	AR5312_GPIOD_MASK	0x000000FF	/* GPIO data reg r/w mask */

/* FIXME: pins 8-11 aren't handled by this code yet - weren't handled by the old code either */

/*
 * Configure GPIO Output lines
 */
HAL_BOOL
ar5312GpioCfgOutput(struct ath_hal *ah, u_int32_t gpio)
{
	u_int32_t gpioCr = (AR5312_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5312_GPIOCR;

	HALASSERT(gpio < AR_NUM_GPIO);
	OS_REG_WRITE(ah, gpioCr, (OS_REG_READ(ah, gpioCr) &~ AR5312_GPIOCR_INPUT(gpio)));
	(void) OS_REG_READ(ah, gpioCr); /* flush register write */

	return AH_TRUE;
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL
ar5312GpioCfgInput(struct ath_hal *ah, u_int32_t gpio)
{
	u_int32_t gpioCr = (AR5312_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5312_GPIOCR;

	HALASSERT(gpio < AR_NUM_GPIO);
	OS_REG_WRITE(ah, gpioCr, (OS_REG_READ(ah, gpioCr) | AR5312_GPIOCR_INPUT(gpio)));
	(void) OS_REG_READ(ah, gpioCr); /* flush register write */

	return AH_TRUE;
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL
ar5312GpioSet(struct ath_hal *ah, u_int32_t gpio, u_int32_t val)
{
	u_int32_t gpioDo = (AR5312_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5312_GPIODO;
	u_int32_t reg;

	HALASSERT(gpio < AR_NUM_GPIO);

	reg = OS_REG_READ(ah, gpioDo);
	reg &= ~(1 << gpio);
	reg |= (!!val) << gpio;
	OS_REG_WRITE(ah, gpioDo, reg);
	(void) OS_REG_READ(ah, gpioDo); /* flush register write */

	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t
ar5312GpioGet(struct ath_hal *ah, u_int32_t gpio)
{
	u_int32_t gpioOffset = (AR5312_GPIO_BASE - ((u_int32_t) ah->ah_sh));

	if (gpio >= AR_NUM_GPIO)
		return 0xffffffff;

	u_int32_t val = OS_REG_READ(ah, gpioOffset+AR5312_GPIODI);
	val = ((val & AR5312_GPIOD_MASK) >> gpio) & 0x1;

	return val;
}

/*
 * Set the GPIO Interrupt
 */
void
ar5312GpioSetIntr(struct ath_hal *ah, u_int gpio, u_int32_t ilevel)
{
	u_int32_t gpioCr = (AR5312_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5312_GPIOCR;
	u_int32_t gpioInt = (AR5312_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5315_GPIOINT;
	u_int32_t val;

	if (gpio >= AR_NUM_GPIO)
		return;

	/* interrupt enable */
	val = OS_REG_READ(ah, gpioCr);
	val |= AR5312_GPIOCR_INT(gpio);
	OS_REG_WRITE(ah, gpioCr, val);
	(void) OS_REG_READ(ah, gpioCr); /* flush register write */

	/* interrupt level select */
	val = OS_REG_READ(ah, gpioInt);
	val &= ~(1 << gpio);
	val |= (!!val) << gpio;
	OS_REG_WRITE(ah, gpioInt, val);
	(void) OS_REG_READ(ah, gpioInt); /* flush register write */

#ifdef wtf
	/* Change the interrupt mask. */
	(void) ar5212SetInterrupts(ah, AH5212(ah)->ah_maskReg | HAL_INT_GPIO);
#endif
}


#endif /* AH_SUPPORT_AR5312 */
