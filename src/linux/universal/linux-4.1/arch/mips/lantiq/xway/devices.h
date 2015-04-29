/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LTQ_DEVICES_XWAY_H__
#define _LTQ_DEVICES_XWAY_H__

#include "../devices.h"
#include <linux/phy.h>
#include <linux/spi/spi.h>
#include <linux/gpio_buttons.h>

extern void ltq_register_gpio(void);
extern void ltq_register_gpio_stp(void);
extern void ltq_register_ase_asc(void);
extern void ltq_register_etop(struct ltq_eth_data *eth);
extern void ltq_register_gpio_ebu(unsigned int value);
extern void ltq_register_spi(struct ltq_spi_platform_data *pdata,
	struct spi_board_info const *info, unsigned n);
extern void ltq_register_madwifi_eep(void *addr);
extern void ltq_register_gpio_buttons(struct gpio_button *buttons, int cnt);

#endif
