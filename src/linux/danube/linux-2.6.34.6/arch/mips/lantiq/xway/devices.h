/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LQ_DEVICES_H__
#define _LQ_DEVICES_H__

#include <lantiq_platform.h>

extern void __init lq_register_gpio(void);
extern void __init lq_register_gpio_stp(void);
extern void __init lq_register_gpio_ebu(unsigned int value);
extern void __init lq_register_gpio_leds(struct gpio_led *leds, int cnt);
extern void __init lq_register_pci(struct lq_pci_data *data);
extern void __init lq_register_nor(struct physmap_flash_data *data);
extern void __init lq_register_wdt(void);
extern void __init lq_register_ethernet(struct lq_eth_data *eth);
extern void __init lq_register_asc(int port);
extern void __init lq_register_crypto(const char *name);

#endif
