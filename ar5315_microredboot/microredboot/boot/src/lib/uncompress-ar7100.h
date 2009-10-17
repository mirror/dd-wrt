/*
 *  uncompress-ar7100.h
 *
 *  Copyright (C) 2009 DD-WRT.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __UNCOMPRESS_H
#define __UNCOMPRESS_H

#include <linux/autoconf.h>

#define CYGARC_KSEG_MASK                               (0xE0000000)
#define CYGARC_KSEG_CACHED                             (0x80000000)
#define CYGARC_KSEG_UNCACHED                           (0xA0000000)
#define CYGARC_CACHED_ADDRESS(x)                       ((((x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_CACHED)
#define CYGARC_UNCACHED_ADDRESS(x)                     ((((x)) & ~CYGARC_KSEG_MASK) | CYGARC_KSEG_UNCACHED)

#include <ar7100_soc.h>
#include <ar7100_sio.h>
#ifndef CONFIG_DEFAULT_BAUDRATE
#define CONFIG_DEFAULT_BAUDRATE	115200
#endif

#define DEFAULT_DATA_FORMAT	UART16550_DATA_8BIT | \
				UART16550_PARITY_NONE | \
				UART16550_STOP_1BIT

#define UART			CYGARC_UNCACHED_ADDRESS(AR7100_UART_BASE)

#define REG_OFFSET		4
#define OFS_RCV_BUFFER          (0*REG_OFFSET)
#define OFS_TRANS_HOLD          (0*REG_OFFSET)
#define OFS_SEND_BUFFER         (0*REG_OFFSET)
#define OFS_INTR_ENABLE         (1*REG_OFFSET)
#define OFS_INTR_ID             (2*REG_OFFSET)
#define OFS_DATA_FORMAT         (3*REG_OFFSET)
#define OFS_LINE_CONTROL        (3*REG_OFFSET)
#define OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define OFS_LINE_STATUS         (5*REG_OFFSET)
#define OFS_MODEM_STATUS        (6*REG_OFFSET)
#define OFS_RS232_INPUT         (6*REG_OFFSET)
#define OFS_SCRATCH_PAD         (7*REG_OFFSET)
#define OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define OFS_DIVISOR_MSB         (1*REG_OFFSET)

#define UART16550_PARITY_NONE	0
#define UART16550_PARITY_ODD    0x08
#define UART16550_PARITY_EVEN   0x18
#define UART16550_PARITY_MARK   0x28
#define UART16550_PARITY_SPACE  0x38

#define UART16550_DATA_5BIT     0x0
#define UART16550_DATA_6BIT     0x1
#define UART16550_DATA_7BIT     0x2
#define UART16550_DATA_8BIT     0x3

#define UART16550_STOP_1BIT     0x0
#define UART16550_STOP_2BIT     0x4

#define UART16550_READ(p)	(*((volatile u8*)(UART + (p))))
#define UART16550_WRITE(p,v)	((*((volatile u8*)(UART + (p)))) = (v))

#define SYS_REG_WRITE(p,v)	(*((volatile unsigned long *) (p)) = (v))

static void putc(int c)
{
	while ((UART16550_READ(OFS_LINE_STATUS) & 0x20) == 0) ;
	UART16550_WRITE(OFS_SEND_BUFFER, c);
}

static void puts(const char *s)
{
	int c;

	while ((c = *s++)) {
		putc(c);
		if (c == '\n')
			putc('\r');
	}
}

static unsigned int cpu_frequency(void)
{
	u32 ar7100_cpu_freq, pll, pll_div, cpu_div, ahb_div, freq;
	static u32 ar7100_ahb_freq = 0;

	if (ar7100_ahb_freq)
		return ar7100_ahb_freq;

	pll = ar7100_reg_rd(AR7100_PLL_CONFIG);
	pll_div = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK) + 1;
	freq = pll_div * 40000000;
	cpu_div = ((pll >> CPU_DIV_SHIFT) & CPU_DIV_MASK) + 1;

	ar7100_cpu_freq = freq / cpu_div;

	return ar7100_cpu_freq;
}

static unsigned int sys_frequency(void)
{
	u32 ar7100_cpu_freq, pll, pll_div, cpu_div, ahb_div, freq;
	static u32 ar7100_ahb_freq = 0;

	if (ar7100_ahb_freq)
		return ar7100_ahb_freq;

	pll = ar7100_reg_rd(AR7100_PLL_CONFIG);

	pll_div = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK) + 1;
	freq = pll_div * 40000000;
	cpu_div = ((pll >> CPU_DIV_SHIFT) & CPU_DIV_MASK) + 1;
	ahb_div = (((pll >> AHB_DIV_SHIFT) & AHB_DIV_MASK) + 1) * 2;

	ar7100_cpu_freq = freq / cpu_div;
	ar7100_ahb_freq = ar7100_cpu_freq / ahb_div;

	return ar7100_ahb_freq;
}

static inline void arch_decomp_setup(void)
{
	/* Initialise the serial port here */

	ar7100_reg_wr(AR7100_GPIO_OE, 0xcff);
	ar7100_reg_wr(AR7100_GPIO_OUT, 0x3b);

	SYS_REG_WRITE(0xb8040028, 0x100);

	/* disable interrupts */
	UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	UART16550_WRITE(OFS_INTR_ENABLE, 0);

	/* set up buad rate */
	{
		u32 divisor;
		u32 uart_clock_rate = sys_frequency();
		u32 base_baud = uart_clock_rate / 16;

		/* set DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x80);

		/* set divisor */
		divisor = base_baud / CONFIG_DEFAULT_BAUDRATE;
		UART16550_WRITE(OFS_DIVISOR_LSB, divisor & 0xff);
		UART16550_WRITE(OFS_DIVISOR_MSB, (divisor & 0xff00) >> 8);

		/* clear DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	}

	/* set data format */
	UART16550_WRITE(OFS_DATA_FORMAT, DEFAULT_DATA_FORMAT);
}

#define arch_decomp_wdog()

#endif
