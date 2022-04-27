/*
 * Copyright (c) 2009 Sebastian Gottschall, NewMedia-NET GmbH
 * All rights reserved.
 *
 */
#ifndef _IF_ATH_GPIO_H_
#define _IF_ATH_GPIO_H_

HAL_BOOL ath_sys_gpiocfgoutput(struct ath_hal *ah, u_int32_t gpio);

HAL_BOOL ath_sys_gpiocfginput(struct ath_hal *ah, u_int32_t gpio);

HAL_BOOL ath_sys_gpioset(struct ath_hal *ah, u_int32_t gpio, u_int32_t val);

u_int32_t ath_sys_gpioget(struct ath_hal *ah, u_int32_t gpio);

#endif
