/*
 * include/asm-mips/mach-ar7100/gpio.h
 *
 *  Copyright (C) 2007 Ubiquiti Networks
 *   Based on Atheros code
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef __ASM_MACH_AR7100_H
#define __ASM_MACH_AR7100_H

#include <asm/errno.h>
#include <asm/mach-ar7100/ar7100.h>

static inline int gpio_request(unsigned gpio, const char *label)
{
	/* TODO: implement */
	return 0;
}

static inline void gpio_free(unsigned gpio)
{
	/* TODO: implement */
}

static inline int gpio_direction_input(unsigned gpio)
{
	ar7100_gpio_config_input(gpio);

}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	ar7100_gpio_config_output(gpio);
	ar7100_gpio_out_val(gpio, value);
}

static inline int gpio_get_value(unsigned gpio)
{
	return ar7100_gpio_in_val(gpio);
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	ar7100_gpio_out_val(gpio, value);
}

#include <asm-generic/gpio.h>		/* cansleep wrappers */

static inline int gpio_to_irq(unsigned gpio)
{
	return AR7100_GPIO_IRQn(gpio);
}

static inline int irq_to_gpio(unsigned irq)
{
        return irq - AR7100_GPIO_IRQ_BASE;
}

#endif /* __ASM_MACH_AR7100_H */
