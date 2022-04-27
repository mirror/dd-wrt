/*
 * Copyright (c) 2009 Sebastian Gottschall, NewMedia-NET GmbH
 * All rights reserved.
 *
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <ah.h>

#define	AR_NUM_GPIO	6	/* 6 GPIO pins */
#define	AR_GPIOD_MASK	0x0000002F	/* GPIO data reg r/w mask */

HAL_BOOL ath_sys_gpiocfgoutput(struct ath_hal *ah, u_int32_t gpio)
{
	return ah->ah_gpioCfgOutput(ah, gpio);
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL ath_sys_gpiocfginput(struct ath_hal *ah, u_int32_t gpio)
{
	return ah->ah_gpioCfgInput(ah, gpio);
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL ath_sys_gpioset(struct ath_hal *ah, u_int32_t gpio, u_int32_t val)
{
	return ah->ah_gpioSet(ah, gpio, val);
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t ath_sys_gpioget(struct ath_hal *ah, u_int32_t gpio)
{
	return ah->ah_gpioGet(ah, gpio);
}
