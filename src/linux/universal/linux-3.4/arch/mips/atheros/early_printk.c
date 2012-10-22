/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 */

#include <linux/mm.h>
#include <linux/io.h>
#include <linux/serial_reg.h>
#include <asm/delay.h>
#include <asm/addrspace.h>

#include <asm/mach-ar231x/ar2315_regs.h>
#include <asm/mach-ar231x/ar5312_regs.h>
#include "devices.h"

static inline void prom_uart_wr(void __iomem *base, unsigned reg,
				unsigned char ch)
{
	__raw_writeb(ch, base + 4 * reg);
}

static inline unsigned char prom_uart_rr(void __iomem *base, unsigned reg)
{
	return __raw_readb(base + 4 * reg);
}

void prom_putchar(unsigned char ch)
{
	static void __iomem *base;

	if (unlikely(base == NULL)) {
		if (is_2315())
			base = (void __iomem *)(KSEG1ADDR(AR2315_UART0));
		else
			base = (void __iomem *)(KSEG1ADDR(AR531X_UART0));
	}
	int cnt = 1000;
	while(cnt--) {
	if ((prom_uart_rr(base, UART_LSR) & UART_LSR_THRE))
	    break;
	udelay(1);
	}
	prom_uart_wr(base, UART_TX, ch);
	cnt = 1000;
	while(cnt--) {
	if ((prom_uart_rr(base, UART_LSR) & UART_LSR_THRE))
	    break;
	udelay(1);
	}
}

