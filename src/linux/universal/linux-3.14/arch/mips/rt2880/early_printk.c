/*
 *  Ralink RT305x SoC early printk support
 *
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/io.h>
#include <linux/serial_reg.h>

#include <asm/addrspace.h>

#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/rt_mmap.h>

#define UART_REG_RX	0
#define UART_REG_TX	1
#define UART_REG_IER	2
#define UART_REG_IIR	3
#define UART_REG_FCR	4
#define UART_REG_LCR	5
#define UART_REG_MCR	6
#define UART_REG_LSR	7


#define UART_READ(r) \
	__raw_readl((void __iomem *)(KSEG1ADDR(RALINK_UART_LITE_BASE) + 4 * (r)))

#define UART_WRITE(r, v) \
	__raw_writel((v), (void __iomem *)(KSEG1ADDR(RALINK_UART_LITE_BASE) + 4 * (r)))

void prom_putchar(unsigned char ch)
{
	while (((UART_READ(UART_REG_LSR)) & UART_LSR_THRE) == 0);
	UART_WRITE(UART_REG_TX, ch);
	while (((UART_READ(UART_REG_LSR)) & UART_LSR_THRE) == 0);
}
