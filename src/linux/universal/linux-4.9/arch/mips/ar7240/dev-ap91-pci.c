/*
 *  Atheros AP91 reference board PCI initialization
 *
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#ifndef CONFIG_MACH_HORNET
#include <linux/pci.h>
#include <linux/ath9k_platform.h>
#include <linux/delay.h>

#include <asm/mach-ar71xx/ar71xx.h>
#include <asm/mach-ar71xx/pci.h>

#include "dev-ap91-pci.h"
#include "pci-ath9k-fixup.h"

struct ath9k_platform_data wmac_data = {
	.led_pin = -1,
};

static char ap91_wmac_mac[6];

static int ap91_pci_plat_dev_init(struct pci_dev *dev)
{
	switch (PCI_SLOT(dev->devfn)) {
	case 0:
		dev->dev.platform_data = &wmac_data;
		break;
	}

	return 0;
}

void ap91_pci_setup_wmac_led_pin(int pin)
{
	wmac_data.led_pin = pin;
}

void ap91_pci_setup_wmac_gpio(u32 mask, u32 val)
{
	wmac_data.gpio_mask = mask;
	wmac_data.gpio_val = val;
}

void ap91_set_tx_gain_buffalo(void)
{
	wmac_data.tx_gain_buffalo = true;
}

void __init ap91_wmac_disable_2ghz(void)
{
	wmac_data.disable_2ghz = true;
}

void __init ap91_wmac_disable_5ghz(void)
{
	wmac_data.disable_5ghz = true;
}

void __init ap91_set_eeprom(void)
{
	wmac_data.use_eeprom = true;
}


int pcibios_init(void);

void ap91_pci_init(u8 *cal_data, u8 *mac_addr)
{
#ifndef CONFIG_ARCHERC7
#ifndef CONFIG_DIR859
#ifndef CONFIG_LIMA
#ifndef CONFIG_MMS344
#ifndef CONFIG_RAMBUTAN
	if (cal_data)
		memcpy(wmac_data.eeprom_data, cal_data, sizeof(wmac_data.eeprom_data));

	if (mac_addr) {
		memcpy(ap91_wmac_mac, mac_addr, sizeof(ap91_wmac_mac));
		wmac_data.macaddr = ap91_wmac_mac;
	}

	ar71xx_pci_plat_dev_init = ap91_pci_plat_dev_init;
	pci_enable_ath9k_fixup(0, wmac_data.eeprom_data);
#endif
#endif
#endif
#endif
#endif
	ar71xx_pci_init();
#ifdef CONFIG_MTD_NAND_ATH
	pcibios_init();
#endif
}
#endif
