/*
 *  uncompress-ar531x.h
 *
 *  Copyright (C) 2003 Instant802 Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __UNCOMPRESS_H
#define __UNCOMPRESS_H

#include <linux/autoconf.h>

#ifndef CONFIG_DEFAULT_BAUDRATE
#define CONFIG_DEFAULT_BAUDRATE	115200
#endif

#define DEFAULT_DATA_FORMAT	UART16550_DATA_8BIT | \
				UART16550_PARITY_NONE | \
				UART16550_STOP_1BIT

#define UART			0xB1100003

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

#define AR5315_DSLBASE          0xB1000000	/* RESET CONTROL MMR */
#define AR5315_PLLC_CTL         (AR5315_DSLBASE + 0x0064)
#define AR5315_CPUCLK           (AR5315_DSLBASE + 0x006c)
#define AR5315_AMBACLK          (AR5315_DSLBASE + 0x0070)
#define AR5315_RESET            (AR5315_DSLBASE + 0x0004)
#define AR5315_RESET_UART0                 0x00000100	/* warm reset UART0 */

/* PLLc Control fields */
#define PLLC_REF_DIV_M              0x00000003
#define PLLC_REF_DIV_S              0
#define PLLC_FDBACK_DIV_M           0x0000007C
#define PLLC_FDBACK_DIV_S           2
#define PLLC_ADD_FDBACK_DIV_M       0x00000080
#define PLLC_ADD_FDBACK_DIV_S       7
#define PLLC_CLKC_DIV_M             0x0001c000
#define PLLC_CLKC_DIV_S             14
#define PLLC_CLKM_DIV_M             0x00700000
#define PLLC_CLKM_DIV_S             20

/* CPU CLK Control fields */
#define CPUCLK_CLK_SEL_M            0x00000003
#define CPUCLK_CLK_SEL_S            0
#define CPUCLK_CLK_DIV_M            0x0000000c
#define CPUCLK_CLK_DIV_S            2

static void putc(int c)
{
	while ((UART16550_READ(OFS_LINE_STATUS) & 0x20) == 0) ;
	UART16550_WRITE(OFS_SEND_BUFFER, c);
}

static void puts(const char *s)
{
	int c;

	while ((c = *s++)) {
		if (c == '\n')
			putc('\r');
		putc(c);
	}
}

static const int CLOCKCTL1_PREDIVIDE_TABLE[4] = {
	1,
	2,
	4,
	5
};

static const int PLLC_DIVIDE_TABLE[5] = {
	2,
	3,
	4,
	6,
	3
};

#define sysRegRead(phys)	\
	(*(volatile unsigned int *)(KSEG1|phys))

#define sysRegWrite(phys, val)	\
	((*(volatile unsigned int *)(KSEG1|phys)) = (val))

static unsigned int ar5315_sys_clk(unsigned int clockCtl)
{
	unsigned int pllcCtrl, cpuDiv;
	unsigned int pllcOut, refdiv, fdiv, divby2;
	unsigned int clkDiv;

	pllcCtrl = sysRegRead(AR5315_PLLC_CTL);
	refdiv = (pllcCtrl & PLLC_REF_DIV_M) >> PLLC_REF_DIV_S;
	refdiv = CLOCKCTL1_PREDIVIDE_TABLE[refdiv];
	fdiv = (pllcCtrl & PLLC_FDBACK_DIV_M) >> PLLC_FDBACK_DIV_S;
	divby2 = (pllcCtrl & PLLC_ADD_FDBACK_DIV_M) >> PLLC_ADD_FDBACK_DIV_S;
	divby2 += 1;
	pllcOut = (40000000 / refdiv) * (2 * divby2) * fdiv;

	/* clkm input selected */
	switch (clockCtl & CPUCLK_CLK_SEL_M) {
	case 0:
	case 1:
		clkDiv =
		    PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKM_DIV_M) >>
				      PLLC_CLKM_DIV_S];
		break;
	case 2:
		clkDiv =
		    PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKC_DIV_M) >>
				      PLLC_CLKC_DIV_S];
		break;
	default:
		pllcOut = 40000000;
		clkDiv = 1;
		break;
	}
	cpuDiv = (clockCtl & CPUCLK_CLK_DIV_M) >> CPUCLK_CLK_DIV_S;
	cpuDiv = cpuDiv * 2 ? : 1;
	return (pllcOut / (clkDiv * cpuDiv));
}

static unsigned int cpu_frequency(void)
{
	return ar5315_sys_clk(sysRegRead(AR5315_CPUCLK));
}

static unsigned int amba_frequency(void)
{
	return ar5315_sys_clk(sysRegRead(AR5315_AMBACLK));
}

static inline void arch_decomp_setup(void)
{
	/* Initialise the serial port here */

	sysRegWrite(AR5315_RESET,
		    sysRegRead(AR5315_RESET) & ~(AR5315_RESET_UART0));

	/* disable interrupts */
	UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	UART16550_WRITE(OFS_INTR_ENABLE, 0);

	/* set up buad rate */
	{
		u32 divisor;
		u32 uart_clock_rate = amba_frequency();
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
