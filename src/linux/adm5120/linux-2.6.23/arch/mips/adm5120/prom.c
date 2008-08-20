/*
 *  $Id: prom.c 9342 2007-10-18 07:40:01Z juhosg $
 *
 *  ADM5120 specific prom routines
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/io.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include <adm5120_info.h>
#include <adm5120_defs.h>
#include <adm5120_uart.h>

#include <prom/cfe.h>
#include <prom/generic.h>
#include <prom/routerboot.h>
#include <prom/myloader.h>
#include <prom/zynos.h>

unsigned int adm5120_prom_type	= ADM5120_PROM_GENERIC;


static void __init prom_detect_machtype(void)
{
/*	if (bootbase_present()) {
		adm5120_prom_type = ADM5120_PROM_BOOTBASE;
		mips_machtype = detect_machtype_bootbase();
		return;
	}

	if (cfe_present()) {
		adm5120_prom_type = ADM5120_PROM_CFE;
		mips_machtype = detect_machtype_cfe();
		return;
	}

	if (myloader_present()) {
		adm5120_prom_type = ADM5120_PROM_MYLOADER;
		mips_machtype = detect_machtype_myloader();
		return;
	}

	if (routerboot_present()) {
		adm5120_prom_type = ADM5120_PROM_ROUTERBOOT;
		mips_machtype = detect_machtype_routerboot();
		return;
	}

	if (generic_prom_present()) {
		adm5120_prom_type = ADM5120_PROM_GENERIC;
		mips_machtype = detect_machtype_generic();
		return;
	}
*/
	mips_machtype = MACH_ADM5120_GENERIC;
}

/* TODO: this is an ugly hack for RouterBOARDS */
extern char _image_cmdline;
static void __init prom_init_cmdline(void)
{
	char *cmd;

	/* init command line, register a default kernel command line */
	cmd = &_image_cmdline + 8;
	if (strlen(cmd) > 0)
		strcpy(arcs_cmdline, cmd);
	else
		strcpy(arcs_cmdline, CONFIG_CMDLINE);

}

#define UART_READ(r) \
	__raw_readl((void __iomem *)(KSEG1ADDR(ADM5120_UART0_BASE)+(r)))
#define UART_WRITE(r, v) \
	__raw_writel((v), (void __iomem *)(KSEG1ADDR(ADM5120_UART0_BASE)+(r)))

void __init prom_putchar(char ch)
{
	while ((UART_READ(UART_REG_FLAG) & UART_FLAG_TXFE) == 0);
	UART_WRITE(UART_REG_DATA, ch);
	while ((UART_READ(UART_REG_FLAG) & UART_FLAG_TXFE) == 0);
}

void __init prom_init(void)
{
	mips_machgroup = MACH_GROUP_ADM5120;
	prom_detect_machtype();

	prom_init_cmdline();
}

void __init prom_free_prom_memory(void)
{
	/* We do not have to prom memory to free */
}
