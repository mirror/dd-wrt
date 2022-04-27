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

void ar7100_set_gpio(int gpio, int val);
int ar7100_get_gpio(int gpio);

HAL_BOOL ath_sys_gpiocfgoutput(struct ath_hal *ah, u_int32_t gpio)
{
	return AH_TRUE;
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL ath_sys_gpiocfginput(struct ath_hal *ah, u_int32_t gpio)
{
	return AH_TRUE;
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL ath_sys_gpioset(struct ath_hal *ah, u_int32_t gpio, u_int32_t val)
{
	ar7100_set_gpio(gpio, val);
	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t ath_sys_gpioget(struct ath_hal *ah, u_int32_t gpio)
{
	return ar7100_get_gpio(gpio);
}
