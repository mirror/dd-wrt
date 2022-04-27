/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ar5416/ar5416_gpio.c#1 $
 */
#include "opt_ah.h"

#ifdef AH_SUPPORT_AR5416

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"
#ifdef AH_DEBUG
#include "ah_desc.h"			/* NB: for HAL_PHYERR* */
#endif

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"

#define	AR_NUM_GPIO	6		/* 6 GPIO pins */
#define AR_GPIO_BIT(_gpio)	(1 << _gpio)

/*
 * Configure GPIO Output lines
 */
HAL_BOOL
ar5416GpioCfgOutput(struct ath_hal *ah, u_int32_t gpio)
{
	HALASSERT(gpio < AR_NUM_GPIO);
	OS_REG_CLR_BIT(ah, AR_GPIO_INTR_OUT, AR_GPIO_BIT(gpio));
	return AH_TRUE;
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL
ar5416GpioCfgInput(struct ath_hal *ah, u_int32_t gpio)
{
	HALASSERT(gpio < AR_NUM_GPIO);
	OS_REG_SET_BIT(ah, AR_GPIO_INTR_OUT, AR_GPIO_BIT(gpio));
	return AH_TRUE;
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL
ar5416GpioSet(struct ath_hal *ah, u_int32_t gpio, u_int32_t val)
{
	u_int32_t reg;

	HALASSERT(gpio < AR_NUM_GPIO);
	reg = MS(OS_REG_READ(ah, AR_GPIO_INTR_OUT), AR_GPIO_OUT_VAL);
	if (val & 1)
		reg |= AR_GPIO_BIT(gpio);
	else 
		reg &= ~AR_GPIO_BIT(gpio);
		
	OS_REG_RMW_FIELD(ah, AR_GPIO_INTR_OUT, AR_GPIO_OUT_VAL, reg);	
	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t
ar5416GpioGet(struct ath_hal *ah, u_int32_t gpio)
{
	if (gpio >= AR_NUM_GPIO)
		return 0xffffffff;
    if (AR_SREV_MERLIN_10_OR_LATER(ah)) {
	return ((OS_REG_READ(ah, AR928X_GPIO_IN_VAL) & AR_GPIO_BIT(gpio)) >> gpio);
    } else {
	return ((OS_REG_READ(ah, AR_GPIO_IN) & AR_GPIO_BIT(gpio)) >> gpio);
    }
}

/*
 * Set the GPIO Interrupt
 */
void
ar5416GpioSetIntr(struct ath_hal *ah, u_int gpio, u_int32_t ilevel)
{
	u_int32_t val;
	
	HALASSERT(gpio < AR_NUM_GPIO);
	/* XXX bounds check gpio */
	val = MS(OS_REG_READ(ah, AR_GPIO_INTR_OUT), AR_GPIO_INTR_CTRL);
	if (ilevel)		/* 0 == interrupt on pin high */
		val &= ~AR_GPIO_BIT(gpio);
	else			/* 1 == interrupt on pin low */
		val |= AR_GPIO_BIT(gpio);
	OS_REG_RMW_FIELD(ah, AR_GPIO_INTR_OUT, AR_GPIO_INTR_CTRL, val);

	/* Change the interrupt mask. */
	val = MS(OS_REG_READ(ah, AR_INTR_ASYNC_ENABLE), AR_INTR_GPIO);
	val |= AR_GPIO_BIT(gpio);
	OS_REG_RMW_FIELD(ah, AR_INTR_ASYNC_ENABLE, AR_INTR_GPIO, val);

	val = MS(OS_REG_READ(ah, AR_INTR_ASYNC_MASK), AR_INTR_GPIO);
	val |= AR_GPIO_BIT(gpio);
	OS_REG_RMW_FIELD(ah, AR_INTR_ASYNC_MASK, AR_INTR_GPIO, val);	
}
#endif  /* AH_SUPPORT_AR5416 */
