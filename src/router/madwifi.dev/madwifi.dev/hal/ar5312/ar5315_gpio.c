/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5315/ar5315_gpio.c#1 $
 */
#include "opt_ah.h"

#if (AH_SUPPORT_2316 || AH_SUPPORT_2317)

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5312/ar5312phy.h"

#define	AR_NUM_GPIO	8		/* 8 GPIO pins */
#define	AR5315_GPIOD_MASK	0x000000FF	/* GPIO data reg r/w mask */

/*
 * Configure GPIO Output lines
 */
HAL_BOOL
ar5315GpioCfgOutput(struct ath_hal *ah, u_int32_t gpio)
{
	u_int32_t gpioDir = (AR5315_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5315_GPIODIR;
	u_int32_t reg;

	HALASSERT(gpio < AR_NUM_GPIO);

	reg = OS_REG_READ(ah, gpioDir);
	reg &= ~AR5315_GPIODIR_M(gpio);
	reg |= AR5315_GPIODIR_O(gpio);
	OS_REG_WRITE(ah, gpioDir, reg);
	(void) OS_REG_READ(ah, gpioDir); /* flush write */

	return AH_TRUE;
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL
ar5315GpioCfgInput(struct ath_hal *ah, u_int32_t gpio)
{
	u_int32_t gpioDir = (AR5315_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5315_GPIODIR;
	u_int32_t reg;

	HALASSERT(gpio < AR_NUM_GPIO);

	reg = OS_REG_READ(ah, gpioDir);
	reg &= ~AR5315_GPIODIR_M(gpio);
	reg |= AR5315_GPIODIR_I(gpio);
	OS_REG_WRITE(ah, gpioDir, reg);
	(void) OS_REG_READ(ah, gpioDir); /* flush write */

	return AH_TRUE;
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL
ar5315GpioSet(struct ath_hal *ah, u_int32_t gpio, u_int32_t val)
{
	u_int32_t gpioDo = (AR5315_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5315_GPIODO;
	u_int32_t reg;

	HALASSERT(gpio < AR_NUM_GPIO);

	reg =  OS_REG_READ(ah, gpioDo);
	reg &= ~(1 << gpio);
	reg |= (!!val) << gpio;
	OS_REG_WRITE(ah, gpioDo, reg);
	(void) OS_REG_READ(ah, gpioDo); /* flush write */

	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t
ar5315GpioGet(struct ath_hal *ah, u_int32_t gpio)
{
	u_int32_t gpioOffset = (AR5315_GPIO_BASE - ((u_int32_t) ah->ah_sh));

	if (gpio >= AR_NUM_GPIO)
		return 0xffffffff;

	u_int32_t val = OS_REG_READ(ah, gpioOffset+AR5315_GPIODI);
	val = ((val & AR5315_GPIOD_MASK) >> gpio) & 0x1;

	return val;
}

/*
 * Set the GPIO Interrupt
 */
void
ar5315GpioSetIntr(struct ath_hal *ah, u_int gpio, u_int32_t ilevel)
{
	u_int32_t gpioInt = (AR5315_GPIO_BASE - ((u_int32_t) ah->ah_sh)) + AR5315_GPIOINT;
	u_int32_t val;

	if (gpio >= AR_NUM_GPIO)
		return;

	val = OS_REG_READ(ah, gpioInt);
	val &= ~(AR5315_GPIOINT_M | AR5315_GPIOINTLVL_M);
	val |= gpio << AR5315_GPIOINT_S;
	if (ilevel)
		val |= 2 << AR5315_GPIOINTLVL_S;	/* interrupt on pin high */
	else
		val |= 1 << AR5315_GPIOINTLVL_S;	/* interrupt on pin low */

	/* Don't need to change anything for low level interrupt. */
	OS_REG_WRITE(ah, gpioInt, val);
	(void) OS_REG_READ(ah, gpioInt); /* flush write */

#ifdef wtf
	/* Change the interrupt mask. */
	(void) ar5212SetInterrupts(ah, AH5212(ah)->ah_maskReg | HAL_INT_GPIO);
#endif
}


#endif /* AH_SUPPORT_2316 || AH_SUPPORT_2317 */
