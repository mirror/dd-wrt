/*
 * Copyright (c) 2009 Sebastian Gottschall, NewMedia-NET GmbH
 * All rights reserved.
 *
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <ah.h>

HAL_BOOL ath_sys_gpiocfgoutput(struct ath_hal *ah, u_int32_t gpio)
{
	return AH_TRUE;
}

HAL_BOOL ath_sys_gpiocfginput(struct ath_hal *ah, u_int32_t gpio)
{
	return AH_TRUE;
}

extern int adm5120_gpio_direction_output(unsigned gpio, int value);
/*
 * Once configured for I/O - set output lines
 */
#define PIN_MASK 0x1f
HAL_BOOL ath_sys_gpioset(struct ath_hal *ah, u_int32_t gpio, u_int32_t val)
{
	adm5120_gpio_direction_output((unsigned int)gpio & PIN_MASK, val);
	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
u_int32_t ath_sys_gpioget(struct ath_hal *ah, u_int32_t gpio)
{
	return 0;
}
