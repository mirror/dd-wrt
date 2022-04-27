/*
 * Copyright (c) 2011 Sebastian Gottschall, NewMedia-NET GmbH
 * All rights reserved.
 *
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <ah.h>

extern void set_gpio(int gpio, int val);

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
	set_gpio(gpio, val);
	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t ath_sys_gpioget(struct ath_hal *ah, u_int32_t gpio)
{
	return 0;
}
