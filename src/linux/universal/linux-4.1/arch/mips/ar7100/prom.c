/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2007 Atheros 
 *  Copyright (C) 2007 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <linux/serial_reg.h>

#include "ar7100.h"

#define sysRegRead(phys)	\
	(*(volatile u32 *)KSEG1ADDR(phys))

#define UART_READ(r) \
	__raw_readl((void __iomem *)(KSEG1ADDR(AR7100_UART_BASE) + 4 * (r)))

#define UART_WRITE(r, v) \
	__raw_writel((v), (void __iomem *)(KSEG1ADDR(AR7100_UART_BASE) + 4*(r)))

void prom_putchar(unsigned char ch)
{
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0) ;
	UART_WRITE(UART_TX, ch);
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0) ;
}

int __ath_flash_size;

void __init prom_init(void)
{
#ifdef CONFIG_AR9100
	printk("flash_size passed from bootloader = %d\n", fw_arg3);
	__ath_flash_size = fw_arg3;
#endif
	/* 
	 * if user passes kernel args, ignore the default one 
	 */

	mips_machtype = MACH_ATHEROS_AP81;

}

void __init prom_free_prom_memory(void)
{
}
