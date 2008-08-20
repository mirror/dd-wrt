/*
 *  $Id: generic.c 9423 2007-10-24 08:19:16Z juhosg $
 *
 *  Generic ADM5120 based board
 *
 *  Copyright (C) 2007 OpenWrt.org
 *  Copyright (C) 2007 Gabor Juhos <juhosg at openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>

#include <asm/bootinfo.h>
#include <asm/gpio.h>

#include <adm5120_board.h>
#include <adm5120_platform.h>
#include <adm5120_irq.h>

static struct platform_device *generic_devices[] __initdata = {
	&adm5120_flash0_device,
	&adm5120_hcd_device,
};
static struct adm5120_pci_irq generic_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0)
};

/*--------------------------------------------------------------------------*/

ADM5120_BOARD_START(GENERIC, "ADM5120")
	.eth_num_ports	= 1,
	.num_devices	= ARRAY_SIZE(generic_devices),
	.devices	= generic_devices,
	.pci_nr_irqs	= ARRAY_SIZE(generic_pci_irqs),
	.pci_irq_map	= generic_pci_irqs,
	
ADM5120_BOARD_END
